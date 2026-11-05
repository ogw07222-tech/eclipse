# 백야의 집행자 이동 아틀라스

- Asset: `Assets/Resources/Eclipse/ui/white_night_motion.png`.
- Required PNG dimensions: **512 × 1440**, RGBA, nearest-neighbor sampling.
- Cells: **64 × 96**, 8 columns × 15 rows. Indexing starts at zero.
- Run cycle: **88–119**, exactly **32** frames, ordered left to right, top to bottom.
- Walking is removed for this character: all grounded movement above the existing deadzone uses the run state, including slow analog input.
- Cycle phase is driven by distance traveled (120 world units per cycle), shared across movement speeds. It is not restarted when crossing the former walk/run speed threshold.
- Frames 0–87 remain unchanged, including jump, attacks and skills. Frames 0–7 are legacy run poses retained for existing dash-frame references.
- The former walk cells 88–95 are replaced by the beginning of the new run cycle.
- Runtime source rectangle: `(index % 8 * 64, index / 8 * 96, 64, 96)`.
- Common destination foot anchor remains at local y=88.

## Art source

Created with the built-in image generator using the approved silver-armored run sheet as the character reference. Final prompt: preserve the silver armor, enclosed helmet, white hair, navy cape and right-hand sword; draw one 32-frame right-facing run in an 8×4 transparent grid, with alternating forward landing, support, rear push-off and leg recovery; constant scale and hip position, minimal vertical bob, complete sword and limbs in each cell, no walking, text or effects.

The selected source is sampled into native cells using one uniform scale and placement for all frames. Alpha is reduced to hard edges. No frame duplication or crossfading is used to inflate the count.

Builds, tests and device playback were not run; owner verification is required.
