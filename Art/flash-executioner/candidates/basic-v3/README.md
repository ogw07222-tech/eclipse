# Flash Executioner basic attack, v3 motion candidate (not approved, not runtime)

- `basic_v3_motion.png` + `basic_v3_motion.json`: 21-frame motion sheet, 192 px cells, 8 columns, 25 fps, pivot 70/133.
- Matches the last motion sequence used for the effect MP4 (`flash_basic_sweep_f10x4.mp4`), without the sweep effect.
- Contact frame: index 14 (JSON `contact_frame`).
- Frame 8 (approved sheet's 0-based frame 8, stray blade line) is replaced by a PixelLab candidate that has no line. The candidate has not been reviewed by the owner.
- Frames from the 22-frame motion removed: 1-based 12, 13, 21, 22. Cut frame 1 is repeated three times; the 10th frame is repeated four times in total.
- Not wired into `reviewed/`, `Assets/`, or `native-engine/`. Runtime mapping (contact frame, frame count) would need the importer, which requires explicit main approval.
- The sweep effect is not included here. It is a PixelFX remix (custom, not an upstream resource) and needs the owner's approval.
