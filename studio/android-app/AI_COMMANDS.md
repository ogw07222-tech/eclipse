# ECLIPSE Studio: AI-first scene command protocol (phase 1)

## Architecture

Human touch controls and the **local JSON command panel** both call the same `SceneCommands.execute` function. The command core transforms in-memory objects and returns structured results. The UI owns persistence, rendering, the undo stack, and content URI permissions.

**No remote access is enabled in this phase.** ChatGPT cannot control a tablet by itself yet; a future authenticated MCP adapter plus a device-side command receiver will be required.

## Local verification

1. Open Studio, select **화면 꾸미기**, import a PNG.
2. Tap **AI 명령**, paste one JSON request, and tap **명령 실행**.
3. Inspect the object list and response; use **실행 취소** to restore the previous scene.
4. Save and reopen the scene to confirm persistence.

Examples:

```json
{"action":"list_objects"}
```

```json
{"action":"move_object","id":1,"x":140,"y":90}
```

```json
{"action":"set_property","id":1,"property":"width","value":64}
```

```json
{"action":"delete_object","id":1}
```

`add_object` accepts `name`, `uri`, `x`, `y`, `width`, `height`; `uri` must be a local `content://` or `file://` URI. Images must first be imported using the Android picker or GitHub importer, so the app has access to them. `clear_scene` clears the active scene (undo supported).

Responses include `ok`, `changed`, `message`, and optional `selectedId` / `data`. Invalid requests report `{"ok":false,"error":"..."}`, leaving the scene untouched. Object count, dimensions, coordinates, and editable property names are constrained.

## Next phases

- Extract project schema into a shared versioned format consumable by the SDL3 runtime.
- Introduce a restricted, authenticated Android-side command receiver and HTTPS MCP service; never expose arbitrary file system or shell operations.
- Add durable command audit logs, atomic save, revisions, transaction undo, confirmation for destructive operations, and real runtime test results.
- Verify the scene editor and command panel on a physical Android tablet.

No remote connector, APK installation, real-device test, or native SDL integration is claimed by this change.
