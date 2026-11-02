# ECLIPSE Studio for Android — architecture v0.1

Status: architecture proposal, **not yet an implementation or runnable APK**.
Audience: Android-first **2D game developers**, with room to expand to Android application development in a later phase.

## Product principles
1. Android-native editor UI (Kotlin + Jetpack Compose), optimized for tablets, touch, pen, keyboard and mouse.
2. Offline-first: projects and asset files belong to the developer; no account or cloud build requirement to edit or run.
3. ECLIPSE Engine (C++17 / SDL3) runs the game preview via JNI on-device.
4. Deterministic, JSON-serialized scene/animation/combat data so both humans and AI can edit through one validated API.
5. Explicit consent before AI/tool commands write or delete project content. Undo/redo and versioned backups are mandatory.
6. Do not assume Android ARM64 supports the standard desktop Android SDK/Gradle toolchain. Build/export is a separate validated milestone.

## Technology and module boundaries
- `studio/android-app`: native Kotlin/Compose UI, editor navigation, touch/S Pen interactions.
- `studio/editor-core`: pure Kotlin project model, commands, validation, undo/redo, serialization.
- `studio/scene-editor`: tile/object layout, selection, snapping, zoom and pan.
- `studio/animation-editor`: PNG atlas slicing and timeline, hitbox and animation events.
- `studio/assets`: Storage Access Framework document tree access, content URI mapping, image preview, cache.
- `studio/preview`: Android Surface + JNI bridge to SDL3 native renderer.
- `studio/project-format`: versioned project.json and relative asset paths.
- `studio/build`: later Android export pipeline; fail loudly when required host tools are unavailable.
- `studio/ai-bridge`: optional MCP API with access policy, no unauthenticated write endpoints.
- `native-engine/`: reusable SDL3 runtime independent of the editor.

## First usable MVP
- Create/Open/Save project from Android document tree (without unrestricted storage permission)
- Import actual PNGs, inspect transparent pixels, frame-grid slicing with nearest-neighbor
- Place sprite entities onto a 2D canvas; edit position, layer, scale and collider
- Basic timeline for idle/run/attack and synchronized independent slash PNG
- Save/loading `project.json` and scene data
- Preview in the app (no APK packaging for every test)
- Touch controls for camera pan/zoom and object drag, optional stylus accuracy
- Undo/redo, autosave snapshots and dirty state
- Instrumented smoke test plus offline reopen test

## Out of scope for MVP
Full Godot-style scripting/editor debugger, 3D, Android app layout designer, Android APK local compilation, asset store, online collaboration, arbitrary plugin execution.

## On-device export evaluation
Option A: use an embedded prebuilt engine runtime and package user assets into an Android APK via an Android-compatible packager and signer; requires experimental proof on arm64 Android.
Option B: native builds on CI/cloud where SDK/NDK/Gradle work reliably.
The studio must be useful for edit/preview without requiring either export method.

## Versioned scene format (example)
```json
{
  "schemaVersion": 1,
  "projectId": "demo",
  "canvas": {"width": 480, "height": 270, "pixelPerfect": true},
  "entities": [
    {"id": "knight", "type": "sprite", "asset": "assets/knight.png", "x": 120, "y": 128, "layer": 1}
  ]
}
```

## Work sequence
P0 project model and SAF file access → P1 Compose scene editor → P2 PNG timeline → P3 embedded SDL3 preview → P4 AI MCP control surface → P5 APK export feasibility/test.
Never call any stage complete until it compiles and runs on a real Android device.
