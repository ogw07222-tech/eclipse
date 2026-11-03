# ECLIPSE Native — game-specific 2D runtime

C++17 + SDL3 3.2.22 + SDL3_image 3.2.4. The native runtime now contains ECLIPSE's combat and an independent Android APK project.

## Play on Android

[Native Android APK workflow](https://github.com/ogw07222-tech/eclipse/actions/workflows/native-android-apk.yml) → **Run workflow** → successful run → **Artifacts → ECLIPSE-Native-ARM64**. Extract and install `ECLIPSE-Native.apk`. No Unity license is used. See [android/README.md](android/README.md) for setup and CI details.

## Gameplay port

- Original 1280×720 logical viewport, 1600px arena, floor y585, player x140 and original four enemy spawns/types.
- Sword three-hit combo12/15/23, 0.8s combo window, third-hit bleed; sword skill; mage basic/empowered projectiles, burn, +22 reaction consuming bleed.
- Both form-swap entry attacks, dash immunity/cooldowns, hit stop, recoil, floating damage and pixel status effects.
- Nine original equipment stat sets across weapon/armor/charm; max-health adjustment; inventory, clear, defeat and retry.
- Original knight atlas with correct idle/run/jump/fall/three attacks/dash/hurt/death ranges; procedural pixel forest, mage, enemies, HUD and original PNG button artwork.
- Keyboard and independently owned touch pointers; upward joystick jump, simultaneous actions, fast press-edge buffering, focus release and restart cleanup.

| Action | Keyboard | Touch |
| --- | --- | --- |
| Move | A/D, arrows | Joystick |
| Jump | Space/W/up | Push joystick upward |
| Attack | J/Z | Sword |
| Skill | K/X | Spark |
| Dash | Shift | Lightning |
| Swap | L/C/Tab | Circular arrows |
| Inventory/equipment | I, 1/2/3 | HUD GEAR slots, equipment buttons |
| Restart | R | Retry button after clear/defeat or Settings → Restart |
| Settings / pause | Escape | Pixel gear in the upper right |

Inventory item labels are English bitmap text to keep the runtime free of a font engine; original item stat values/slot identities are preserved. Korean pixel typography remains future UI work. No sound assets were present in the baseline, so audio remains disabled.

## Desktop build and tests

```sh
cmake -S native-engine -B native-engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build native-engine/build --parallel 4
./native-engine/build/eclipse_engine
```

Requires CMake3.24+, C++17 compiler and SDL platform prerequisites. For CI software rendering without X11/Wayland:

```sh
cmake -S native-engine -B native-engine/build -DECLIPSE_HEADLESS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build native-engine/build --parallel 2
ctest --test-dir native-engine/build --output-on-failure
```

Core checks also run without SDL: `c++ -std=c++17 native-engine/tests/core_tests.cpp -o /tmp/eclipse-tests && /tmp/eclipse-tests`.

Desktop CMake packages exactly five original PNGs beside the executable. Source image bytes are untouched. UI sheets are reduced once at startup to 128px per state for compact GPU textures. SDL shared libraries must remain available for a standalone desktop distribution.

## Resource and validation boundary

PNG decoding only, no image encoders; fixed-size touch storage; fixed60Hz simulation, approximately60fps presentation cap; suspend in background. During active combat the procedural forest, enemies, projectiles and UI require redraws. This extends the earlier foundation's static-scene redraw rule deliberately.

[Combat port validation](COMBAT_PORT.md) documents model and render checks, APK/Android results and remaining real-device QA. [SCOPE.md](SCOPE.md) keeps engine responsibilities limited to actual game requirements. The prior foundation size snapshot in VALIDATION.md is historical and must not be used as the new APK size.

## Coarse knight motion update

The current native knight uses the 104-frame coarse motion sheet described in [KNIGHT_MOTION.md](KNIGHT_MOTION.md), replacing the original knight atlas in native packaging. Run/walk phase follows distance, attacks have separate full clips, and Actions uploads a renderer replay video. Earlier original-atlas descriptions above document the previous port baseline.

## Pixel controls and combat update

- Hold the joystick upward to jump again on the landing simulation step; release or center it to stop repeating. Keyboard jump uses the same held behavior, with the existing quick-tap buffer preserved.
- Dash leaves up to six short, fading snapshots of the current knight/mage pose. Both forms flash white for 0.18 seconds when damage is accepted; invulnerable hits do not flash.
- Sword collisions follow each active frame's trail cells and the blade swept from its grip to those cells. Rendering and collision share the same pivot, facing and equipment range transform. Each enemy is hit once per swing; dash, damage and form changes cancel pending and active sword hits. Sword skill and blade-form entry now use a visible swing with wind-up.
- The HUD displays a pixel health bar plus `HP current / maximum`, including armor/charm bonuses and a low-health color. The upper-right pixel gear opens a paused settings panel with working screen shake, damage number and dash afterimage switches, Resume and Restart. Preferences survive restart and are saved in SDL's per-app preference directory.
- All new UI, typography and effects use integer pixel blocks or nearest-neighbor sprite sampling. Existing game assets and Legacy engines are unchanged.

Validation: 34 dependency-free regression checks pass with `-Wall -Wextra -Werror`; the SDL desktop renderer compiles with the same warnings and passes twelve software-rendered smoke scenes, including settings, hit flash, dash and low health. Android APK/real-device validation remains a separate Actions/device check.
