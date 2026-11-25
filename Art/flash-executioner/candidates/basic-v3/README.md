# Flash Executioner basic attack, v3 motion candidate (not approved, not runtime)

- `basic_v3_motion.png` + `basic_v3_motion.json`: motion sheet, 192 px cells, 8 columns, 25 fps, pivot 70/133.
- Matches the last motion sequence used for the effect MP4 (`flash_basic_sweep_f10x4.mp4`), without the sweep effect.
- Contact frame: see JSON `contact_frame`.
- No AI candidate frame is included. The approved frames are used as they are.
- Frames from the 22-frame motion removed: 1-based 12, 13, 21, 22. Cut frame 1 is repeated three times; the 10th frame is repeated four times in total.
- Not wired into `reviewed/`, `Assets/`, or `native-engine/`. Runtime mapping (contact frame, frame count) would need the importer, which requires explicit main approval.
- The sweep effect is not included here. It is a PixelFX remix (custom, not an upstream resource) and needs the owner's approval.

## Effect (candidate)

- `effect_flash_sweep.png` + `effect_flash_sweep.json`: PixelFX `melee/flash-sweep` sheet, 35 frames, 208x160 cells, 30 fps, one-shot, origin [72,128]. Source recipe: `pixelfx/src/fx/melee/flash-sweep.js` (a copy of `melee/sword-slash`, adapted; not an upstream resource).
- Composite used for the MP4: scale 1.5, offset +20 px X and +11 px Y from the contact-relative origin, effect starts 0.14 s before contact plus one frame, contact at index 14 of the motion sheet.
- Not wired into runtime. The owner must approve the effect as a custom remix before it is used.
