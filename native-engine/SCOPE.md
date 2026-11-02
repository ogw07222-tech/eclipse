# ECLIPSE-specific engine scope

Decision: implement only the runtime capabilities consumed by ECLIPSE. Add a dependency or subsystem when an actual game feature uses it. Keep game rules separate from platform/rendering code without building a general engine framework.

## Capability boundary

| Capability | Foundation now | Remaining ECLIPSE work |
| --- | --- | --- |
| Pixel rendering | One logical viewport, PNG texture caching, atlas cells, flip, 2D primitives | Forest layers, original HUD, effects and camera follow |
| Movement/collision | Fixed-step movement, jump, gravity, flat floor and horizontal limits | Match original arena tuning; add only collision shapes used by actual levels |
| Input | Keyboard, pointer IDs, combined move+attack, letterbox conversion, focus release | Original joystick/up-jump layout and dash/skill/swap buttons |
| Combat | Basic attack animation | Four enemies, hitboxes, three-hit combo, damage, bleed/fire, swapping, clear/retry |
| Equipment/UI | Original source data remains in Unity/Godot | Port nine items, three slots, health/cooldown display and inventory |
| Platform/build | Native desktop target, C++ tests, PNG render smoke test | Android SDL bootstrap, Gradle/NDK build, APK artifact and device testing |

The engine owns frame timing, input, asset lifetime, drawing and collision helpers. Game code owns damage, cooldowns, enemies, equipment and stage completion. Prefer compact concrete code until a second real consumer justifies an abstraction.

## Size and resource policy

- Runtime image support: PNG decode only, no image encoding. No font engine until text assets require it.
- Copy referenced assets, not the complete art archive. Current runtime uses one original 384×448 RGBA atlas, approximately 672 KiB decoded pixel payload; GPU/driver/decoder allocations are additional.
- Assets load at startup, not per frame. Core input uses a fixed array of eight pointers. Render with SDL's 2D API; no scene graph or per-frame heap object construction is introduced.
- Simulation: fixed 60 Hz; stop simulation and rendering in background. Frame cap targets about 60 fps, not a measured device performance guarantee.
- Current unused SDL subsystems are disabled in CMake. Enable small audio playback only when ECLIPSE ships sound effects/music.
- Build mode defaults to Release. Compare stripped native binaries, dependencies, packaged assets and eventual APK size separately.
- Do not promise an APK size, RAM ceiling, startup time or frame-rate result before producing and measuring a real Android build.

## Delivery order

1. Prove the native foundation compiles, reads its existing PNG, handles independent inputs and cleans up.
2. Package this minimal build as an Android APK and verify real tablet multitouch/background handling.
3. Port existing ECLIPSE combat and UI feature by feature, retaining gameplay constants and regression checks.
4. Measure APK size/RAM/frame time on the tablet; optimize the observed bottleneck.

Foundation tests are source/runtime checks only. Complete game parity and Android readiness remain pending.

## Combat/Android update

The combat port now implements the previously remaining gameplay/UI and SDL Android packaging. See COMBAT_PORT.md for current evidence and android/README.md for the independent native APK workflow. Device performance, sound assets, Korean typography and actual tablet QA remain separate work. The delivery-order table above records the original foundation plan.
