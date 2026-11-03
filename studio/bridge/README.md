# ECLIPSE Studio remote control bridge (phase 2 prototype)

This introduces an **opt-in, foreground-only Android device client** and a **single-user, persistent command queue with a minimal MCP JSON-RPC endpoint**. It does **not** make the tablet publicly reachable, and does **not** complete ChatGPT's OAuth connection. The bridge must be hosted separately with HTTPS and proper authentication.

## How it works

```text
MCP client --(authenticated HTTP POST)--> bridge /mcp
                                          |
                                      SQLite queue
                                          |
Studio "화면 꾸미기" --(authenticated HTTPS polling)--> /device/next
Studio executes SceneCommands in foreground and POSTs --> /device/result
MCP client calls eclipse_command_result(command_id) to inspect result
```

- The app connects **outbound only**. It never opens an inbound Android port, never runs shell commands, and does not require Android NDK.
- Device polling works only when the `화면 꾸미기` screen remains active, the remote connection has been enabled, and play-preview is not running.
- The device token and HTTPS endpoint are held only in Compose memory for the open app session. Never share tokens in chat or commit them.
- The remote allowlist is `list_objects`, `move_object`, and `set_property`; no remote delete/clear/import, APK build, filesystem access, or code execution.
- Every accepted request is queued, **not immediately executed**. The MCP response provides a `command_id`; request `eclipse_command_result` to check completion.
- Requests are served serially; SQLite persists queue across bridge restarts. A lease can retry a request after 60 seconds; the running Android screen keeps a 100-item in-memory duplicate-response cache. Long offline/restart scenarios can still replay requests: commands are absolute-position/absolute-property operations, not increments.
- Android error responses are returned as structured JSON. Check `result.ok` before claiming any change succeeded. Modified scenes remain in memory until manually saved.

## Local server test (loopback only)

Python 3.10+ is required. Generate two independent random credentials locally:

```bash
export ECLIPSE_DEVICE_TOKEN="$(python3 -c 'import secrets; print(secrets.token_urlsafe(32))')"
export ECLIPSE_MCP_TOKEN="$(python3 -c 'import secrets; print(secrets.token_urlsafe(32))')"
python3 -m unittest discover -s studio/bridge -p 'test_*.py'
python3 studio/bridge/server.py
```

The server binds `127.0.0.1:8765` by default and uses `commands.sqlite3` in its working directory; never commit this database or credentials.

To smoke-test the local MCP JSON-RPC interface:

```bash
curl -s http://127.0.0.1:8765/mcp \
  -H "Authorization: Bearer $ECLIPSE_MCP_TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"jsonrpc":"2.0","id":1,"method":"tools/list"}'
```

To test end-to-end using a tablet, you need an HTTPS reverse proxy/tunnel terminating TLS at a trusted domain. Configure it to forward to the bridge running on loopback; **never publish port 8765 over plain HTTP**. Put the process on a persistent host with a persistent SQLite directory and backups. This prototype is strictly single-user; do not deploy as a multi-tenant service. Never embed the MCP/admin token in Android.

In the Studio app, open **화면 꾸미기 > 원격 연결**, paste the public HTTPS base URL and **device token**, then tap **연결**. The app shows poll status and performs queued edits while the editor is visible. Disable with **연결 해제**.

## MCP tools (prototype)

- `eclipse_list_objects` -> queue list request
- `eclipse_move_object` -> queue absolute move by object ID
- `eclipse_set_property` -> queue name/x/y/width/height change
- `eclipse_command_result` -> read queued / leased / done status and device reply

MCP endpoint: `POST /mcp`. Supported basics: `initialize`, `ping`, `tools/list`, `tools/call`, `notifications/initialized` (202); JSON-RPC responses are single application/json objects. This is a **protocol prototype**, not a fully certified implementation for all MCP versions/clients.

## Current ChatGPT integration limitation

ChatGPT custom MCP **write** connections may require a qualifying plan/workspace and OAuth authorization; the prototype's fixed admin bearer credential does **not** implement OAuth 2.1 or a ChatGPT plugin installation flow. The connection therefore is **not yet usable as a secure, directly installed ChatGPT write plugin**. Add a proper OAuth 2.1 protected resource and authorization flow (and verify your ChatGPT plan supports writing to custom apps) before installation.

Do **not** expose an unauthenticated MCP endpoint just to bypass OAuth. Keep the prototype private or limit usage to locally controlled MCP clients.

## Work remaining

- OAuth 2.1 / credential lifecycle and per-user/device pairing.
- Stronger remote command audit, persistent idempotency and automatic save with revision locking.
- Real-device validation; game preview and native SDL3 integration.
- HTTPS hosting, secure deployment, service health monitoring, safe disconnect/revoke controls.
