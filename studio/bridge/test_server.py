"""Run: python3 -m unittest discover -s studio/bridge -p 'test_*.py'."""
import importlib.util
import os
import tempfile
import unittest
from pathlib import Path

PATH = Path(__file__).with_name("server.py")
spec = importlib.util.spec_from_file_location("eclipse_bridge", PATH)
server = importlib.util.module_from_spec(spec)
spec.loader.exec_module(server)

class BridgeTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        server.DB = Path(self.tmp.name) / "test.sqlite3"
        server.DEVICE_TOKEN = "device-" + "x" * 32
        server.ADMIN_TOKEN = "admin-" + "y" * 32
        server.init()

    def tearDown(self):
        self.tmp.cleanup()

    def test_enqueue_delivery_result(self):
        queued = server.queue({"action": "move_object", "id": 1, "x": 120, "y": 80})
        self.assertEqual(queued["status"], "queued")
        cmd = server.next_command()
        self.assertEqual(cmd["id"], queued["command_id"])
        self.assertEqual(cmd["command"]["action"], "move_object")
        self.assertIsNone(server.next_command())
        server.record_result(cmd["id"], {"ok": True, "changed": True})
        self.assertEqual(server.status(cmd["id"])["status"], "done")
        self.assertTrue(server.status(cmd["id"])["result"]["ok"])

    def test_blocks_destructive_and_invalid(self):
        with self.assertRaises(ValueError):
            server.queue({"action": "delete_object", "id": 1})
        with self.assertRaises(ValueError):
            server.queue({"action": "move_object", "id": 1, "x": True, "y": 5})
        with self.assertRaises(ValueError):
            server.queue({"action": "set_property", "id": 1, "property": "uri", "value": "file:///"})
        with self.assertRaises(ValueError):
            server.queue({"action": "set_property", "id": 1, "property": "width", "value": -1})

    def test_read_only_list(self):
        queued = server.invoke("eclipse_list_objects", {})
        command = server.next_command()
        self.assertEqual(queued["command_id"], command["id"])
        self.assertEqual(command["command"], {"action": "list_objects"})

    def test_reject_unknown_results(self):
        with self.assertRaises(ValueError):
            server.record_result("0"*32, {"ok": True})

if __name__ == "__main__":
    unittest.main()
