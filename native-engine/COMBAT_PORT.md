# Native combat + Android port

Baseline: main PR21 (`c50b4dd3a0499d55bed70622c64b2556e202a903`). Rules are ported from `Assets/Eclipse/Core/CombatSimulation.cs` and the preserved Godot scripts, not rebalanced for the old 480×270 placeholder.

## Completed source implementation

Combat values, arena/spawns, equipment stats, attack directions/heights, damage immunities, cooldowns, DoT, swap entries and +22 reaction mirror the existing C# port. Flat-floor manual collision and swept projectile collision remain the same approach. Full combat now owns the simulation; SDL handles input/platform/rendering.

`combat.hpp` owns gameplay; `core.hpp` owns fixed touch/keyboard input and press buffering; `presentation.hpp` owns PNG lifetimes, pixel drawing and UI; main.cpp is the platform loop/composition root. Android's Java Activity is the official SDL frontend subclass. Core and device APK build do not depend on Unity.

UI differences: the 128px joystick is positioned below the floor (35,586) to keep the starting knight visible; other action positions retain the original layout. English bitmap inventory labels (same item stats), extra readable form/GEAR/FOES/cooldown indicators. The separate sword-wave effect stays disabled, matching the baseline missing/removed effect. Effects and mage are pixel primitives, not new generated art. No source PNG pixels were edited.

## Executed local checks

- Production C++ core: **20/20 cases passed**. Covers wave/spawns, movement/walls, jump, dash immunity/cooldown, all combo hits, combo timeout, reaction consumed once, both DoTs, both swap entries, mage projectile, nine equipment stats, defeat immunity/freeze, clear, restart, directional/vertical range, independent touch/keyboard, fast tap buffering, joystick jump, inventory/equip/retry targets, animation/death bounds.
- Real SDL3/SDL3_image Release compilation succeeded. CTest **2/2 passed** (20-case core suite + graphics smoke).
- Graphics smoke loads all five PNGs and renders combat, mage/reaction, inventory, clear and defeat scenes with the actual software renderer. Local scene captures are used for visual review.
- New Actions YAML checked with actionlint. APK verifier checks real ELF64 machine type, three native libraries, original asset hashes, ZIP integrity and signature in CI.

## Android CI evidence

The Native Android APK workflow builds ARM64 and separately tests the application on an x86_64 Android emulator. Exact run/result evidence is added after the first workflow completes; do not infer APK success merely from these local Linux checks.

## Real-device QA still required

Install the ARM64 APK on the Galaxy Tab S10+ and verify simultaneous joystick+actions, Android16 safe areas/fullscreen, text/icon readability, background/resume, clear/defeat/retry, frame pacing, heat and memory. Android29 emulator results only cover that emulator/ABI. Full Android16/device performance is not established by a successful build.

The first Android execution exposed SDL's resizable-window orientation override: the Activity manifest alone did not keep landscape. Native startup now explicitly restricts SDL orientations and uses Android fullscreen; the emulator asserts landscape before mapping taps. Independent smoke scenarios restart the live stage to avoid unrelated enemy damage expiring the test character.

Android game-key dispatch uses a small JNI adapter that pushes scancodes into the same SDL event queue as desktop input, independently of virtual keyboard/native-focus translation. System keys keep their normal Android handling. The unchanged emulator test requires the real R-key restart event; it is not replaced by a model reset or test-only invulnerability.
