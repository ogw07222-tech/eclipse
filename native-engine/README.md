# ECLIPSE Native — small game-specific runtime

C++17 + SDL3 3.2.22 + SDL3_image 3.2.4, scoped to ECLIPSE's 2D pixel combat. Current stage: native runtime foundation, not a full gameplay port or Android APK.

## Current implementation

- 480×270 logical viewport and nearest-filtered original 48×64 PNG atlas.
- Fixed 60 Hz simulation; render pacing capped near 60 fps, with VSync where supported. The scene is redrawn only when visible actor state changes or the window needs repainting. Background/focus loss clears controls and suspends simulation/rendering.
- Walking, jump/gravity/floor/walls, basic attack animation, correct sequential idle/run/jump/fall/attack1 atlas ranges.
- Keyboard plus eight fixed touch slots; movement and attack can be held by different pointers. Jump/attack use press edges.
- Letterbox/high-DPI-aware pointer conversion, texture loaded once at startup, executable-relative assets, explicit failure if the required image is missing.
- PNG-only image decoder build, no image encoders/sample programs. Unused SDL audio, camera, sensors, haptics, joystick and GPU API modules are disabled. SDL's 2D renderer remains enabled.
- Core input/movement checks and a dummy-video PNG decode/render smoke test.

The authoritative scope and next steps are in [SCOPE.md](SCOPE.md). Existing Unity/Godot projects stay available; this branch does not remove them.

## Build on desktop

Install CMake >=3.24, a C++17 compiler and SDL's normal platform prerequisites, then:

```sh
cmake -S native-engine -B native-engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build native-engine/build --parallel 4
./native-engine/build/eclipse_engine
```

First configure downloads the pinned SDL sources. On Windows use the Release executable under the selected build configuration. Runtime SDL shared libraries must remain available beside the app or on the loader path; the build tree supplies a development loader path on Linux. Desktop distribution packaging is not implemented.

CMake copies **only the knight PNG currently used** from `Assets/Resources/Eclipse` to the executable's `assets` directory. The same source image is reused without modifying its bytes. Start the executable from any working directory.

## Headless checks

```sh
cmake -S native-engine -B native-engine/build -DECLIPSE_HEADLESS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build native-engine/build --parallel 4
ctest --test-dir native-engine/build --output-on-failure
```

Headless mode disables desktop X11/Wayland dependencies and uses SDL's software renderer/dummy video for the smoke test. It is only for CI; use the normal build for an interactive window. The smoke test opens the packaged PNG and actually renders one frame; it is not device/visual QA.

The native CI workflow compiles the C++ runtime and runs both tests. Tests can also compile without SDL:

```sh
c++ -std=c++17 native-engine/tests/core_tests.cpp -o /tmp/eclipse-core-tests
/tmp/eclipse-core-tests
```

## Current limits and validation boundary

Enemy AI, three-hit combo, bleed/fire reaction, mage/swap, equipment/HUD and full forest visuals still require porting from ECLIPSE. Sprite alpha defects are not repaired by the engine. Korean font/text UI and audio can be added when real game assets require them.

**Android bootstrap/Gradle/NDK packaging and an APK have not been implemented or tested.** Existing `.github/workflows/android-apk.yml` builds the Unity project; it does not build this native runtime. The native runtime does not require a Unity license, but the Unity workflow still does.

Native Android packaging should use [SDL's official Android project](https://wiki.libsdl.org/SDL3/README-android), the existing shared `main` CMake target, and only the game's referenced PNG assets. That is the next platform task, separate from completing game parity.
