#!/usr/bin/env python3
"""ECLIPSE Studio single-user command relay (development prototype).

No third-party packages. Put behind HTTPS and an authentication gateway before
remote use. ChatGPT plugin OAuth integration is NOT implemented in this relay.
"""
import hmac
import json
import os
import secrets
import sqlite3
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit

DB = Path(os.environ.get("ECLIPSE_BRIDGE_DB", "commands.sqlite3"))
DEVICE_TOKEN = os.environ.get("ECLIPSE_DEVICE_TOKEN", "")
ADMIN_TOKEN = os.environ.get("ECLIPSE_MCP_TOKEN", "")
BIND_HOST = os.environ.get("ECLIPSE_BRIDGE_HOST", "127.0.0.1")
BIND_PORT = int(os.environ.get("ECLIPSE_BRIDGE_PORT", "8765"))
ALLOWED_ORIGIN = os.environ.get("ECLIPSE_ALLOWED_ORIGIN", "")
LOCK = threading.RLock()

COMMANDS = {"list_objects", "move_object", "set_property"}
MAX_PENDING = 100
LEASE_SECONDS = 60

def db():
    connection = sqlite3.connect(str(DB), timeout=10)
    connection.row_factory = sqlite3.Row
    return connection

def init():
    if not DEVICE_TOKEN or not ADMIN_TOKEN or hmac.compare_digest(DEVICE_TOKEN, ADMIN_TOKEN):
        raise SystemExit("Set two different, nonempty ECLIPSE_DEVICE_TOKEN and ECLIPSE_MCP_TOKEN.")
    DB.parent.mkdir(parents=True, exist_ok=True)
    with db() as conn:
        conn.execute("""CREATE TABLE IF NOT EXISTS commands (
            id TEXT PRIMARY KEY, request_json TEXT NOT NULL,
            status TEXT NOT NULL, result_json TEXT,
            created_at REAL NOT NULL, updated_at REAL NOT NULL,
            lease_until REAL NOT NULL DEFAULT 0)""")

def validate(request):
    if not isinstance(request, dict):
        raise ValueError("Command must be a JSON object")
    action = request.get("action")
    if action not in COMMANDS:
        raise ValueError("Remote command not permitted")
    if action == "list_objects":
        if set(request) != {"action"}:
            raise ValueError("Unknown fields")
        return request
    if action == "move_object":
        if set(request) != {"action", "id", "x", "y"}:
            raise ValueError("Expected action, id, x, y")
        for key in ("id", "x", "y"):
            val = request[key]
            if type(val) is not int or not (1 <= val <= 2147483647 if key == "id" else -100000 <= val <= 100000):
                raise ValueError("Invalid " + key)
        return request
    if set(request) != {"action", "id", "property", "value"}:
        raise ValueError("Expected action, id, property, value")
    if type(request["id"]) is not int or request["id"] < 1:
        raise ValueError("Invalid object ID")
    prop = request["property"]
    value = request["value"]
    if prop == "name":
        if type(value) is not str or not 1 <= len(value.strip()) <= 120:
            raise ValueError("Invalid name")
    elif prop in ("x", "y", "width", "height"):
        limits = (-100000, 100000) if prop in ("x", "y") else (1, 8192)
        if type(value) is not int or not limits[0] <= value <= limits[1]:
            raise ValueError("Invalid property value")
    else:
        raise ValueError("Unsupported property")
    return request

def queue(request):
    request = validate(request)
    with LOCK, db() as conn:
        count = conn.execute("SELECT COUNT(*) FROM commands WHERE status IN ('queued','leased')").fetchone()[0]
        if count >= MAX_PENDING:
            raise ValueError("Command queue is full")
        command_id = secrets.token_hex(16)
        now = time.time()
        conn.execute("INSERT INTO commands (id,request_json,status,created_at,updated_at) VALUES (?,?,?,?,?)",
                     (command_id, json.dumps(request), "queued", now, now))
    return {"command_id": command_id, "status": "queued", "note": "Not yet executed on tablet"}

def next_command():
    with LOCK, db() as conn:
        now = time.time()
        row = conn.execute("""SELECT * FROM commands
            WHERE status='queued' OR (status='leased' AND lease_until<?)
            ORDER BY created_at ASC LIMIT 1""", (now,)).fetchone()
        if row is None:
            return None
        conn.execute("UPDATE commands SET status='leased', updated_at=?, lease_until=? WHERE id=?",
                     (now, now + LEASE_SECONDS, row["id"]))
        return {"id": row["id"], "command": json.loads(row["request_json"])}

def record_result(command_id, result):
    if type(command_id) is not str or len(command_id) != 32 or not isinstance(result, dict):
        raise ValueError("Invalid result payload")
    if len(json.dumps(result)) > 128000:
        raise ValueError("Result is too large")
    with LOCK, db() as conn:
        row = conn.execute("SELECT status FROM commands WHERE id=?", (command_id,)).fetchone()
        if row is None:
            raise ValueError("Unknown command ID")
        if row["status"] == "done":
            return {"status": "already_done"}
        now = time.time()
        conn.execute("UPDATE commands SET status='done',result_json=?,updated_at=?,lease_until=0 WHERE id=?",
                     (json.dumps(result), now, command_id))
    return {"status": "done"}

def status(command_id):
    with db() as conn:
        row = conn.execute("SELECT status,result_json,created_at,updated_at FROM commands WHERE id=?",
                           (command_id,)).fetchone()
    if row is None:
        raise ValueError("Unknown command ID")
    return {"command_id": command_id, "status": row["status"],
            "result": json.loads(row["result_json"]) if row["result_json"] else None}

TOOLS = [
    {"name": "eclipse_list_objects", "description": "Queue a request for objects in the currently open ECLIPSE Studio scene.",
     "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False}},
    {"name": "eclipse_move_object", "description": "Queue movement of an object to absolute scene coordinates (does not execute immediately).",
     "inputSchema": {"type": "object", "properties": {
         "id": {"type": "integer", "minimum": 1}, "x": {"type": "integer"},
         "y": {"type": "integer"}}, "required": ["id", "x", "y"], "additionalProperties": False}},
    {"name": "eclipse_set_property", "description": "Queue changing one object's name, position or dimensions.",
     "inputSchema": {"type": "object", "properties": {
         "id": {"type": "integer", "minimum": 1},
         "property": {"type": "string", "enum": ["name", "x", "y", "width", "height"]},
         "value": {"oneOf": [{"type": "integer"}, {"type": "string"}]}},
         "required": ["id", "property", "value"], "additionalProperties": False}},
    {"name": "eclipse_command_result", "description": "Check the status and result of a previously queued command.",
     "inputSchema": {"type": "object", "properties": {
         "command_id": {"type": "string", "minLength": 32, "maxLength": 32}},
         "required": ["command_id"], "additionalProperties": False}}
]

def invoke(name, args):
    if not isinstance(args, dict):
        raise ValueError("Invalid tool arguments")
    if name == "eclipse_command_result":
        if set(args) != {"command_id"}:
            raise ValueError("Unknown fields")
        return status(args["command_id"])
    actions = {
        "eclipse_list_objects": "list_objects",
        "eclipse_move_object": "move_object",
        "eclipse_set_property": "set_property"
    }
    if name not in actions:
        raise ValueError("Unknown tool")
    return queue({"action": actions[name], **args})

class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        # Do not print bearer credentials or raw request data
        print("%s %s" % (self.address_string(), fmt % args))

    def response(self, code, payload=None):
        data = json.dumps(payload, ensure_ascii=False).encode("utf-8") if payload is not None else b""
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        if data:
            self.wfile.write(data)

    def authorized(self, device=False):
        expected = DEVICE_TOKEN if device else ADMIN_TOKEN
        supplied = self.headers.get("X-Device-Token", "") if device else self.headers.get("Authorization", "")
        if not device:
            supplied = supplied.removeprefix("Bearer ")
        return bool(supplied) and hmac.compare_digest(supplied, expected)

    def allowed(self):
        origin = self.headers.get("Origin")
        # No browser origin means non-browser/native client.
        return not origin or (bool(ALLOWED_ORIGIN) and origin == ALLOWED_ORIGIN)

    def do_GET(self):
        if not self.allowed():
            return self.response(403, {"error": "Origin denied"})
        path = urlsplit(self.path).path
        if path == "/health":
            return self.response(200, {"status": "ready"})
        if path == "/device/next":
            if not self.authorized(device=True):
                return self.response(401, {"error": "Unauthorized"})
            item = next_command()
            return self.response(200, item) if item else self.response(204)
        return self.response(404, {"error": "Not found"})

    def do_POST(self):
        if not self.allowed():
            return self.response(403, {"error": "Origin denied"})
        path = urlsplit(self.path).path
        if path not in ("/device/result", "/mcp"):
            return self.response(404, {"error": "Not found"})
        if not self.authorized(device=path == "/device/result"):
            return self.response(401, {"error": "Unauthorized"})
        try:
            size = int(self.headers.get("Content-Length", "-1"))
            if size < 0 or size > 128000:
                return self.response(413, {"error": "Invalid request size"})
            request = json.loads(self.rfile.read(size))
            if path == "/device/result":
                return self.response(200, record_result(request["id"], request["result"]))
            self.mcp(request)
        except (ValueError, TypeError, KeyError, json.JSONDecodeError) as exc:
            return self.response(400, {"error": str(exc)})

    def mcp(self, request):
        if not isinstance(request, dict) or request.get("jsonrpc") != "2.0":
            raise ValueError("Invalid JSON-RPC request")
        method = request.get("method", "")
        req_id = request.get("id")
        # Notifications have no JSON-RPC response.
        if req_id is None and method.startswith("notifications/"):
            return self.response(202)
        try:
            if method == "initialize":
                result = {"protocolVersion": "2025-06-18", "capabilities": {"tools": {}},
                          "serverInfo": {"name": "eclipse-studio-bridge", "version": "0.1.0"}}
            elif method == "ping":
                result = {}
            elif method == "tools/list":
                result = {"tools": TOOLS}
            elif method == "tools/call":
                params = request.get("params", {})
                output = invoke(params["name"], params.get("arguments", {}))
                result = {"content": [{"type": "text", "text": json.dumps(output, ensure_ascii=False)}]}
            else:
                return self.response(200, {"jsonrpc": "2.0", "id": req_id,
                                           "error": {"code": -32601, "message": "Method not found"}})
            self.response(200, {"jsonrpc": "2.0", "id": req_id, "result": result})
        except (ValueError, KeyError) as exc:
            self.response(200, {"jsonrpc": "2.0", "id": req_id,
                                "result": {"content": [{"type": "text", "text": str(exc)}],
                                           "isError": True}})

if __name__ == "__main__":
    init()
    print("ECLIPSE bridge on %s:%d (HTTP local only; use HTTPS reverse proxy)." %
          (BIND_HOST, BIND_PORT))
    ThreadingHTTPServer((BIND_HOST, BIND_PORT), Handler).serve_forever()
