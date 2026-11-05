# Unified 64x94 knight artwork

All thirteen animations now use the same attack-reference knight identity and detail level: silver full plate, closed helmet, navy cape and a longer right-hand sword. The old small multi-row locomotion extraction has been replaced with newly authored poses of the same quality as the attacks.

The atlas remains **1024x1222**, with 16 columns, 13 rows, **64x94 cells**, pivot (28,88) and 1.85 world scale. Ready poses share a 53-pixel silhouette height and all motions share one 96-color indexed palette with binary alpha and nearest sampling. The extended blade remains visible in every pose. Measured attack blade annotations are about 20-30% longer on average than the previous artwork; apparent length still varies with pose projection.

| Row | Animation | Authored/runtime frames |
| --- | --- | --- |
| 0 | Idle | 8 |
| 1 | Walk | 8 |
| 2 | Run | 16 |
| 3 | Jump | 6 |
| 4 | Fall | 6 |
| 5 | Land | 6 |
| 6 | Dash | 8 |
| 7 | Weak slash | 8 |
| 8 | Overhead slash | 10 |
| 9 | Thrust | 10 |
| 10 | Hurt | 6 |
| 11 | Death | 12 |
| 12 | Air slam | 8 |

There are **112 distinct authored poses**. No temporal duplicate frames are needed. Sprite cells reserve room around the silhouette and preserve the longer blade inside the 64-pixel frame width. Pose groups use a common normalization, with limited width fitting where a long blade and fully extended cape/lunge would exceed the fixed cell. Source normalization records are in `Art/knight/redesign/unified_style.json`.

## Air attack

Sword attacks started in the air use a separate downward slam: a brief raised-sword wind-up, accelerated descent held until actual floor contact, then a planted-blade impact and recovery. Landing resolves damage once against monster outlines, freezes the contact pose briefly, and emits a ground shockwave from the steel tip. Dash, swap, skills, damage and restart cancel pending slams. Mage air attacks retain projectiles. The previous 104 poses remain pixel-identical.

## Sword effects

Wrist/tip coordinates in `blade_paths.json` refer to the current master PNG. The leading effect edge ends at the current visible steel tip. Cuts form a tapered blade sweep; overhead cuts have a broader arc; thrust uses a compact forward wake. All effects share the actor's pivot, 1.85 scale and horizontal reflection. Weak/strong/thrust effects appear on frames 3-5 / 5-7 / 4-6 respectively. Contact has an effect; wind-up and late recovery do not. Hit-stop holds the same actor/effect frame. White cores, two blue sweep layers and pixel glints strengthen all three ground attacks. The air slam uses frames 2-6, a vertical trail, contact flash, expanding ground arcs and debris. The character PNG contains no baked sword trails.

## Rebuild and validation

Edit the indexed master PNG, preserving the transparent palette index recorded in the PNG (currently 3). Update the blade coordinates if a pose changes. Run:

```sh
python Tools/bake_knight_motion.py
python native-engine/tools/check_motion.py
```

Validation checks actual PNG dimensions, all 112 distinct occupied cells, transparent unused cells, binary alpha, shared palette, ready-pose height, uncut frame bounds, source mappings, steel-tip alignment, effect windows and proportional effect reach. It does not use the retired coarse rig's anatomical calculations as evidence for this PNG artwork.

The prompt direction for every new motion was the same attack reference, matching helmet/armor/body detail, realistic adult proportions, navy cape and a blade about 40% of standing height. All approved poses were produced using built-in image generation and then normalized into exact game cells. The committed master is the reproducible authoring source; the runtime loads the packed PNG directly.

Previews: `Art/knight/redesign/unified_motion_preview.gif` and `sword_fx_preview.gif`. Local validation: 57 core tests, real SDL compilation, PNG render smoke and left/right attack rendering. Android installation is not covered by these art checks.

## Standard knight sprint update —24 authored poses

The current sprint override uses **24 newly baked distinct PNG poses**, not24 selected from54. They occupy432–455, preserving the existing1024×2914/64×94/16-column atlas contract and original28/88 pivot. At base speed280, the196-unit distance cycle remains0.7s /34.29 pose FPS (the approved exception). Internal16-slot run phase metadata and the legacy source row remain separate from actual display metadata.

The existing editable sprint renderer now uses196 units of contact travel per cycle,14px thigh/shin lengths and lower heel recovery. Original opaque upper-body/weapon pixels and all non-sprint0–431 decoded pixels are preserved. No gameplay parameters or other motions are changed. See [sprint source and review](../Art/knight/sprint/README.md). Full game/device playback and owner tests were not run; no Actions workflow was dispatched.
