# ECLIPSE — Native C++ / SDL3 game

**Active game:** [native-engine/](native-engine/README.md) — C++17 + SDL3, independent Android APK.  
**Android studio/editor:** [studio/android-app/](studio/android-app/README.md).

## Legacy versions

- [Legacy/Unity/](Legacy/Unity/README.md) — archived Unity 6.3 project; open this directory in Unity Hub.
- [Legacy/Godot/](Legacy/Godot/README.md) — archived Godot project.

The native engine continues to use shared pixel PNGs at `Assets/Resources/Eclipse/`; this is an asset library, **not an active Unity project**. The complete Unity asset snapshot (including PNG files and .meta files) is preserved inside `Legacy/Unity/Assets/`.

## Native build and checks

See [native-engine/README.md](native-engine/README.md), [native engine checks](.github/workflows/native-engine-checks.yml) and [native Android APK](.github/workflows/native-android-apk.yml). Unity-specific workflows are archived under `Legacy/Unity/workflows/` and do not run automatically.
