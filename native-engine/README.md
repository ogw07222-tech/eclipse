# ECLIPSE native 2D engine — first prototype

Experimental **SDL3 + C++** engine alongside the original Godot project. This does **not** replace Godot or Unity, and should not be merged as a complete Android build solution.

## Current features
- SDL3 renderer using 480×270 logical game resolution
- Existing knight PNG sprite-sheet loading via SDL3_image (48×64 cells, first row only)
- Nearest-neighbor pixel art scaling
- Basic left/right, jump and attack animation indicator
- Keyboard A/D, Space, J; rudimentary touch controls

## Desktop prototype
```bash
cmake -S native-engine -B native-engine/build
cmake --build native-engine/build -j4
./native-engine/build/eclipse_engine
```
CMake downloads SDL3 and SDL3_image source, so Internet access is required on first configuration.

## Android next steps
Official SDL3 Android guidance uses SDL's android-project glue, Android SDK, NDK, JDK and Gradle; **this folder is not yet an APK-ready Android project**.
Follow https://wiki.libsdl.org/SDL3/README-android to configure a shared `main` target with SDL3 Java frontend, include the game's assets, then build/debug the APK.

## Known limits
- First-row placeholder animation, not correct run/combo sprite mapping
- No game objects, enemy AI, save system, physics solver or complete combat
- Touch handling is single-pointer prototype, not proper multitouch
- Not tested on Android or compiled on CI yet
- Existing knight atlas may contain alpha defects or embedded slash effects; this prototype does not repair PNG assets

## Project boundary
Existing Godot sources and assets are untouched. Build dependencies and scripts are isolated in `native-engine/`.
