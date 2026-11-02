# ECLIPSE Unity project context

Analyzed 2026-11-02. Source baseline: Godot main `b47cc494f27258a1f1343aa0beeadee612b7b5cf`.

## Confirmed environment

- Root is a new Unity project: `Assets`, `Packages/manifest.json`, `ProjectSettings/ProjectVersion.txt`.
- Editor target: 6000.3.0f1 (Unity 6.3 LTS). Built-in pipeline; all game rendering is native uGUI pixel meshes/RawImages, with an empty solid-black Camera for letterboxing. No SRP asset, physics engine or third-party runtime is required.
- Legacy Input Manager, keyboard + EventSystem/StandaloneInputModule multitouch. `ProjectSettings/InputManager.asset` contains UI navigation axes.
- UGUI 2.0.0; Unity Test Framework 1.4.6; built-in UI, inputlegacy, imageconversion, imgui, textrendering, audio modules. Package resolution in Editor not yet observed.
- Local Unity Editor and Unity MCP: unavailable. Original Godot runtime baseline was not available either.

## Boundaries and startup

- `Eclipse.Core`: no Unity references, authoritative player/enemy/projectile/equipment state. Fixed 60 Hz pixel-coordinate model matches the original flat arena.
- `Eclipse.Runtime`: references Core and Unity.ugui. `EclipseGame` is the composition root and sole MonoBehaviour in `Assets/Scenes/Main.unity`; it owns the simulation, builds child presentation and adapts input. No scene searches, static game state, service locator, networking or persistence.
- `Eclipse.Editor`: editor-only build menu/CLI functions.
- `Eclipse.Tests`: editor-only NUnit tests; `Eclipse.PlayModeTests`: bootstrap/restart and pointer ownership smoke tests (not run); shared scenarios are also compiled by standalone .NET runner.
- Main scene is the only enabled Build Settings scene. All dynamically created presentation belongs to its bootstrap object; no DontDestroyOnLoad.
- Source Godot scripts/scenes/assets are preserved under `Legacy/Godot`, avoiding case-only path conflicts on Windows. They are archival input, not imported Unity scripts. `README.md` now documents Unity first.

## Rendering and art

- 1280×720 logical coordinates, top-left origin, floor y=585, world width=1600. Camera follows horizontally, clamped to [640,960]. Root UI fits safe area. Fractional viewport scaling is allowed; integer art vertices and point-filtered textures preserve pixel edges, but device raster parity still needs visual QA.
- Original 384×448 knight atlas: 48×64 frames, 8 columns, 55 used frames at 12 fps; displayed at 2×. Counts idle/run/jump/fall/attack1/attack2/attack3/dash/hurt/death = 6/8/4/3/6/6/8/5/3/6.
- All seven PNGs copied byte-for-byte under Resources/Eclipse, including unused effect sheets. No image regeneration or alpha repair performed.
- Actual committed button state atlases are used. Source main referenced nonexistent flattened button/effect paths; see migration notes.

## Settings and validation

New ProjectSettings: version, EditorSettings (2D + text serialization), enabled Main scene, InputManager, PlayerSettings (legacy input, 1280×720 window, landscape ARM64 IL2CPP Android, API 26).
Build menus support Windows and development Android APK. No generated caches/build artifacts are committed.
15/15 standalone production-model checks passed; structural script passed. Unity import/compiler/Console, Play Mode, build, device inputs and performance are unknown. Use `Docs/UNITY_MIGRATION.md` acceptance checklist before declaring runtime parity.

Inspected: all source gameplay/visual/input scripts, README, project.godot, main.tscn, design guideline, source PNG dimensions, new manifests/asmdefs/scene/settings/tests.
