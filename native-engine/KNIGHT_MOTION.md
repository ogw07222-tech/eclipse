# Coarse knight animation

The runtime now uses `Assets/Resources/Eclipse/knight_motion.png`: 768×576 pixels, 16 columns × 12 rows, 48×48 cells. Each design pixel renders as a 3×3 world-pixel block. There are 104 playable frames; unused cells are transparent. The legacy Unity/Godot atlas is preserved.

| Row | Clip | Frames |
| --- | --- | --- |
| 0 | Idle | 8 |
| 1 | Walk | 8 |
| 2 | Run | 16 |
| 3 | Jump | 6 |
| 4 | Fall | 6 |
| 5 | Land | 6 |
| 6 | Dash | 8 |
| 7 | Weak slash | 8 |
| 8 | Strong overhead slash | 10 |
| 9 | Thrust | 10 |
| 10 | Hurt | 6 |
| 11 | Death | 12 |

The approved coarse concept supplied the helmet, torso, limb and boot pixels in `Art/knight/rig`. An offline rig assembles these parts with connected dark under-armor, two-bone joint constraints, eased attack poses and a secondary cape shape. One fixed sword PNG is attached to the foreground right hand in every pose, including death. There are no baked sword waves. Palette export retains transparent backgrounds and opaque dark cloth/armor.

Runtime run/walk phase uses actual player travel (96/72 world pixels per cycle), including equipment speed. It stops advancing at walls. Rising, falling and landing have separate clips. Attack clips fit the existing cooldowns and finish all frames; confirmed hit-stop holds the contact pose. Combat damage, range, gear stats and cooldown values are unchanged.

## Development and review

The game loads the committed PNG; no art tools or extra runtime framework are required.

To edit and rebuild the atlas, install ImageMagick and `python -m pip install -r Tools/knight-art-requirements.txt`, then run `python Tools/bake_knight_motion.py`. The build copies art changes even when C++ sources are unchanged.

`python native-engine/tools/check_motion.py` checks frame occupancy, alpha, palette, blade bounds, joint reach and analytical support-foot drift. The mathematical contact model has zero sliding; coarse rasterization still quantizes foot positions to design pixels.

For an actual SDL renderer replay:

```sh
mkdir -p /tmp/eclipse-review
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software native-engine/build/eclipse_engine --motion-review /tmp/eclipse-review
ffmpeg -framerate 12 -i /tmp/eclipse-review/frame-%04d.bmp -c:v libx264 -pix_fmt yuv420p ECLIPSE-motion-review.mp4
```

This renders all clips using the production Animation adapter, then runs a live Combat replay with movement, jump, dash, attacks, skill, swap, defeat and restart. The Actions review artifact contains the video, atlas and constraint metrics. Native Android CI additionally tests real touch run, attack, dash and jump on its emulator.
