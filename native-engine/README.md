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
| Restart | R | Retry button after clear/defeat |

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

## Seed-based forest placement engine

`src/forest_generator.hpp` is the pure C++17 deterministic layout subsystem; `Presentation::setForestSettings` changes seed, density (0–1), size (0.6–1.5), variation (0–1), and micro-motion (0–0.2). The scene is generated once at initialization or when settings change, not regenerated every frame. Spatial slots and minimum separation prevent unconstrained overlap, and layer-specific parallax is camera-relative. Trees, shrubs, and grasses do **not** sway; effects alone may move less than ~2px.

The PNG catalog expects up to 40 optional files under `Assets/Resources/Eclipse/forest/`, preserving the `background/`, `trees_large/`, `trees_medium/`, `bushes/`, `grass/`, `rocks/`, `terrain/`, `details/`, `plants/`, and `effects/` subdirectories. Sprite PNGs load once at startup with nearest-neighbor sampling; missing images fall back to simple procedural shapes. Place the independently produced asset package there to enable sprite rendering. Gameplay collision and combat are not modified. Headless tests check deterministic seeds, density, layer order, motion, and parallax.

### Importing the 40 PNG concept pack

Download `eclipse_forest_png_40.zip` from the ECLIPSE asset delivery and run:

```sh
python3 native-engine/tools/import_forest_pack.py eclipse_forest_png_40.zip --check-only
python3 native-engine/tools/import_forest_pack.py eclipse_forest_png_40.zip
git add Assets/Resources/Eclipse/forest
git commit -m "Add ECLIPSE forest sprites"
git push
```

The importer validates exact sprite filenames, PNG signatures and dimensions against the packaged manifest. The current optional loader has procedural fallback until those files are added. **Known v1 limitations:** three far-background images are opaque and intentionally use procedural fallback to avoid rectangular sky seams; four ground tile samples are reference assets and are not tiled because their borders do not yet form a seamless tileset. Some decorative sprites have clipped edges from the concept-sheet extraction. Mainline gameplay does not depend on these assets until import.

## APK-free pixel forest visual preview

A lightweight local browser page at `native-engine/preview/forest.html` displays the **real forest PNG sprites** with a JavaScript copy of the C++ seeded placement rules. It is intended for layout/art tuning, not a gameplay or frame-performance replacement for the native renderer. It uses the same defaults (seed 20261103, 74% density, 100% size, 18% variation), main tree asset `tree_large_01.png`, secondary tree asset `tree_mid_01.png`, and the intentionally restrained accessory selection. Tree crown sizes were increased; static props do not sway. Large opaque far-background sprites remain disabled and procedural silhouettes are used instead.

From the repository root on Termux (requires Python):

```sh
cd ~/eclipse
git pull --ff-only origin main
python -m http.server 8765 --bind 127.0.0.1
```

Then open `http://127.0.0.1:8765/native-engine/preview/forest.html` in Samsung Internet or Chrome on the same tablet. Change seed, density, size, variation or camera without rebuilding anything. The PNG save button exports the currently displayed 1280×720 backdrop. Stop the server with Ctrl+C. No GitHub Actions, Android SDK, APK, or Unity is needed. The browser preview mirrors C++ seeded placement but cannot certify native SDL render quality or run combat.

## Forest visual cleanup

The low-fidelity procedural distant trees have been removed from both the native renderer and APK-free browser preview. Missing PNG sprites are skipped rather than replaced with rectangular trees. The floor now includes reproducible, subdued ground/shadow patches and short uneven grass-edge marks. Keep the current two large PNG tree species and sparse foreground props. Reload the browser page after pulling the latest commit; no APK rebuild is necessary for the preview.

## Cropped forest sprite repair

A revised image patch, `eclipse_forest_cropping_fix.zip`, re-extracts the **main tree**, **secondary tree**, and **two bushes** from wider original source-sheet regions, leaving transparent margins and updating the manifest dimensions. Extract its `Assets/Resources/Eclipse/forest/` paths into the repository and commit the changed PNGs plus manifest. The native SDL renderer and browser preview normalize the sprites' visual height to the original footprint so restored source boundaries do not make objects unexpectedly bigger; trees are constrained not to clip against the top of the viewport. PNG edges cannot be fixed by changing camera culling alone.

## Ground art and PNG audit (November 2026)

The playable surface remains y=585 and collision is unchanged. The original continuous green ground slab has been replaced by layered dark brown soil, strata, low-contrast rocky pixels, deterministic clods and a sparse, thin irregular grass fringe. Both the SDL renderer and browser preview use the same coordinate-based hash and color palette, and the ground remains static during scroll.

The original concept-sheet pack includes several edge-touching art fragments and seven opaque backgrounds/terrain tiles that are **not seamless**. Four active foreground sprites (two trees and two bushes) have a separately prepared wider re-extraction pack. Until that pack is imported, the repository still contains the older crops. Non-active reference sprites remain in the archive for future art production, not active gameplay. Do not infer a fully restored sprite solely from added alpha margins.
