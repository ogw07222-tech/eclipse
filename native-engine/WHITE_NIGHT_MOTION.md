# 백야의 집행자 이동 아틀라스

- Asset: `Assets/Resources/Eclipse/ui/white_night_motion.png`.
- Required PNG dimensions: **512 × 1440**, RGBA, nearest-neighbor sampling.
- Cells: **64 × 96**, 8 columns × 15 rows. Indexing starts at zero.
- Run cycle: **88–119**, exactly **32** frames, ordered left to right, top to bottom.
- Walking is removed for this character: all grounded movement above the existing deadzone uses the run state, including slow analog input.
- Cycle phase is driven by distance traveled (**240 world units per cycle**), shared across movement speeds. At default280 units/s this is **0.8571 seconds / 37.33 pose updates per second**. This is a documented fast-run exception to16–32FPS: the new PNG planted-foot retreat is about52–54 world units over7/32 of a cycle, matching a240-unit stride. The prior280-unit stride made those planted feet slide forward. Movement velocity is unchanged. Device playback remains unverified.
- Frames 0–87 remain unchanged, including jump, attacks and skills. Frames 0–7 are legacy run poses retained for existing dash-frame references.
- The former walk cells 88–95 are replaced by the beginning of the new run cycle.
- Runtime source rectangle: `(index % 8 * 64, index / 8 * 96, 64, 96)`.
- Common destination foot anchor remains at local y=88.

## Art source

Created with the built-in image generator using the approved silver-armored run sheet as the character reference. Final prompt: preserve the silver armor, enclosed helmet, white hair, navy cape and right-hand sword; draw one 32-frame right-facing run in an 8×4 transparent grid, with alternating forward landing, support, rear push-off and leg recovery; constant scale and hip position, minimal vertical bob, complete sword and limbs in each cell, no walking, text or effects.

The selected source is sampled into native cells using one uniform scale and placement for all frames. Alpha is reduced to hard edges. No frame duplication or crossfading is used to inflate the count.

Builds, tests and device playback were not run; owner verification is required.

## Alternating-leg correction

The former run repeated a bright leading leg. The new32 PNG poses preserve identity by brightness: **bright = right leg**, **dark violet-gray = left leg**. At pose88 the bright right ankle is forward; at104 the dark left ankle is forward while the bright right ankle recovers behind. Crossing does not reassign brightness.

Only the lower-leg region of88–119 is replaced. The original RGBA pixels above local y66 and original sword pixels under the hip are copied from an immutable approved reference. The full non-run0–87 region remains byte-for-byte equal as decoded pixels. The atlas remains512×1440, cells64×96, pivot32/88; existing packaging/loader paths and runtime count/start stay the same.

See [art source and validation](../Art/white-night/README.md). The earlier source paragraph describes the superseded run. The current source is `Art/white-night/run_alternating_source.png`, generated from an explicit two-leg joint guide and packed with `Tools/pack_white_night_run.py`. It contains authored leg pixels, not duplicated old frames.

The previous animation audit/replay artifacts are historical for their recorded revisions; their old white-run FPS is superseded by this240-unit stride. No game/APK build, owner test suite, GitHub Actions, device play, or real-time video viewing is claimed.

### Folded-leg follow-up

The first alternating revision compressed the legs horizontally by0.84 during packing, making folded shins read shorter. That scaling is removed. A new15px thigh/15px shin guide with lower heel recovery was used to redraw the legs; these are guide targets, not measured proof of every PNG bone length. Original upper-body/sword/non-run pixels remain protected. Runtime phase/start/count/240-unit stride and all gameplay values are unchanged by this follow-up. Current comparison videos use the preceding alternating revision at the same cadence on both sides.

## Verdict visual-rate exception

- The current ultimate actor clip uses 20 separate PNG poses over 1.12s (17.86 slots/s).
- Its authored time-stop deliberately holds the off-hand-fist pose between 0.23s and 0.70s; this is a cinematic visual hold, not missing run/attack art. Because the ECLIPSE FPS targets are guidelines rather than immutable minima, **preserve the 20-frame Verdict clip until a separately approved re-authoring of the time-stop timeline** instead of adding fake duplicated frames.
- The executor's sprint remains 32 distinct PNG poses, with its approved movement-linked 240-unit stride. Sword slashes, golden lance textures and spatial rifts are independent PNG effects and must not be baked into the body sheet.
