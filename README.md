# ECLIPSE — SDL3 Dark Fantasy Action Roguelite

This repository contains the production **ECLIPSE Game**, developed with **C++17 and SDL3**.

## Game source and assets

- [Native game engine](native-engine/README.md) — gameplay, combat, rendering, and SDL3 runtime.
- [Android game](native-engine/android/README.md) — `com.eclipse.nativegame` APK project.
- `Assets/Resources/Eclipse/` — production game art and pixel assets.
- [Combat skills and status design](Docs/COMBAT_SKILLS_AND_STATUS_DESIGN.md).

## Development rules

Read [AGENTS.md](AGENTS.md) before changes. **Never run GitHub Actions without explicit approval for that particular run.** The owner tests on their own device.

Keep sprite atlases, runtime animation mappings, and game resources consistent. Required game assets must fail with an explicit error rather than silently falling back to unrelated art.
