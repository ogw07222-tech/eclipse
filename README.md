# ECLIPSE — SDL3 game and separate developer Studio

The production ECLIPSE Game remains in this repository. **ECLIPSE Studio has been extracted to its own repository:** [ogw07222-tech/eclipse-studio](https://github.com/ogw07222-tech/eclipse-studio). The legacy `studio/` directory remains temporarily for migration safety and should not be treated as the active Studio source.

| | ECLIPSE Game | ECLIPSE Studio |
| --- | --- | --- |
| Purpose | Normal playable game for players | Developer-only launcher, local testing and diagnostics |
| Android application ID | `com.eclipse.nativegame` | `dev.eclipse.studio` |
| Project | [native-engine/android/](native-engine/android/README.md) | [eclipse-studio](https://github.com/ogw07222-tech/eclipse-studio) |
| Game implementation | [native-engine/](native-engine/README.md) (C++17 / SDL3) | Reuses the original game source, but Studio's test runner is separately built |
| APK | `ECLIPSE-Native.apk` | `ECLIPSE-Studio.apk` |
| Workflow | [Native Android APK](.github/workflows/native-android-apk.yml) | [Studio APK (separate repository)](https://github.com/ogw07222-tech/eclipse-studio/actions/workflows/studio-android-apk.yml) |
| Local game-code testing | Not applicable to the public app | Termux compilation / native module import (experimental, PR #40) |

## Development boundaries

1. **Game changes**: modify `native-engine/src/`, `native-engine/CMakeLists.txt`, or `Assets/Resources/Eclipse/`. These are the **shared original game**, and can affect both APKs at their next build. Always test normal Game independently before releasing.
2. **Studio changes**: modify `studio/android-app/`, the Studio-only loader `native-engine/src/studio_loader.cpp` (when present), and the Studio build workflow. Never add Studio's developer UI, local module import, or debugging tools to the regular Game APK.
3. **Release boundary**: keep separate application IDs, version codes, signing keys and artifacts. Updating one app must not replace or change the installed version of the other.
4. **CI boundary**: do not use a Studio APK artifact as a substitute for a Game APK, or vice versa. Build each app explicitly and independently.
5. **Studio's Termux feature** is an experimental development feature, not part of the normal Game installation. It is pending the Studio-only PR sequence [#39](https://github.com/ogw07222-tech/eclipse/pull/39) → [#40](https://github.com/ogw07222-tech/eclipse/pull/40), plus Android hardware QA. Do not assume those unmerged changes already work.
6. **Editor is deferred**. Studio priority: (P1) run original SDL3 gameplay without GitHub Actions for each local code change, (P2) show startup/linker errors, (P3) editor.

A shared repository is **not a shared application**. If independent repository access and history become necessary, repository splitting is a separate migration; do not silently duplicate game C++ sources and allow gameplay forks to diverge.

## Game design

- [기사 스킬과 상태이상 스택 설계](Docs/COMBAT_SKILLS_AND_STATUS_DESIGN.md)

## Archived versions

- [Legacy/Unity/](Legacy/Unity/README.md) — Unity 6.3 snapshot
- [Legacy/Godot/](Legacy/Godot/README.md) — archived Godot game

The currently active game is SDL3, not the archived Unity or Godot versions.

## Migration note (2026-11-04)

Studio development belongs in [eclipse-studio](https://github.com/ogw07222-tech/eclipse-studio). The standalone Game remains `com.eclipse.nativegame` and its code and game build are unchanged. Studio Android signing secrets must be manually recreated in the new repository; encrypted GitHub Actions secrets cannot be exported by the API. The old `studio/` directory is retained until the new repository's Studio build and APK update signing are verified, then it can be removed through a separate reviewed change.
