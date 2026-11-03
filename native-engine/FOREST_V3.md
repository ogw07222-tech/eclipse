# Forest v3 — six-role pixel PNG scene (NPC-safe)

ECLIPSE now supports a **six-layer composed background** with independent art assets. New mode is preferred when *all six* exact-size PNGs are present; otherwise the renderer automatically falls back to the four-layer v2 scene and ultimately the original object forest.

## Art assets and draw order

| Layer | Path under `Assets/Resources/Eclipse/forest_v3/` | Dimensions | Camera parallax | Rules |
| --- | --- | --- | ---: | --- |
| 0. Sky + moon | `sky/sky_gradient_moon_01.png` | 1280×720 | 0.00 | Opaque, fixed, moon+gradient only |
| 1. Clouds | `clouds/clouds_01.png` | 1600×720 | 0.06 | Independent transparent pixel cloud layer |
| 2. Mountains | `far/mountain_far_01.png` | 1600×720 | 0.14 | Weathered ridges, seed affects only restrained brightness |
| 3. Trees | `mid/forest_mid_01.png` | 1600×720 | 0.40 | Complete mid-distance forest clusters, fixed positioning |
| 4. Ground | `near/ground_fixed_01.png` | 1600×720 | 1.00 | Soil and grass, fixed collision floor y585 |
| 5. Foreground | `near/near_props_fixed_01.png` | 1600×720 | 1.00 | Rocks and shrubs, fixed positions outside NPC zones |

All files are PNG. Cloud, mountain, tree, ground and foreground layers are independently alpha-composited. Sky fully covers the viewport and cannot create rectangular alpha seams. The limited 1600px stage does not infinitely tile; keep non-sky files at least 1600px wide.

**Unchanged gameplay contract:** world width 1600; ground y585; all enemy, player, NPC spawn coordinates, and existing collision shapes remain unchanged. The v3 loader never randomizes near/mid layers. No foreground decor is placed inside the three reserved NPC intervals x260–425, x710–900, x1180–1360, plus 60px clearance. Foreground decor is non-collidable background art, not an obstacle system.

## Installation on Termux

Download `eclipse_forest_v3_layers.zip` from this conversation. It contains six PNGs and `scene.json` in the correct repository paths.

```sh
cd ~/eclipse
git pull --ff-only origin main
unzip -o ~/storage/downloads/eclipse_forest_v3_layers.zip -d ~/eclipse
git status --short Assets/Resources/Eclipse/forest_v3
git add Assets/Resources/Eclipse/forest_v3
git commit -m "Import NPC-safe six-layer forest v3 PNG assets"
git push origin main
```

**Preview without an APK:**

```sh
cd ~/eclipse
python -m http.server 8765 --bind 127.0.0.1
```

Open `http://127.0.0.1:8765/native-engine/preview/forest.html?v=forest-v3`, or from a separate Termux session run:

```sh
termux-open-url "http://127.0.0.1:8765/native-engine/preview/forest.html?v=forest-v3"
```

Look for `forest_v3 · PNG 6/6` at the bottom, not the old `6e6a466` message. Enable `NPC 고정 구역 보기` to verify reserved corridors. If six-layer mode is not active, inspect download extraction and missing files. The preview's moon, clouds and layer assets are real PNG images rather than simple block shapes.

## Verification status

The six-file image package was checked for PNG decoding, dimensions, sky opacity, independent alpha layers, JSON configuration and clear zones. The C++ v3 interface, renderer selection, Android packaging, desktop CMake and pure C++ unit assertions have been updated. End-to-end native SDL and Android device rendering has **not** been run; do not claim platform validation from HTML alone.
