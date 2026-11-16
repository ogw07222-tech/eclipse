# Flash Executioner motion atlas

The active class is **Flash Executioner**. Its current runtime actor sheets live in `Assets/Resources/Eclipse/flash_executioner/`; the retired `ui/white_night_motion.png` atlas is not loaded or packaged.

## Run sheet

- Asset: `Assets/Resources/Eclipse/flash_executioner/actor_run.png`.
- Grid: 960 × 1152 RGBA; 5 columns × 6 rows; 192 × 192 cells.
- Active frames: 0–28 (29 poses); unused final cell 29 is outside the frame count.
- Pivot: (70, 133); render scale: 1.4.
- Nominal cycle: 0.8 seconds, 36.25 poses/second. Runtime phase is distance-driven using the approved 240-world-unit stride.
- The run sheet uses the same Flash Executioner character design as the idle, jump, fall, attacks, skills, dash, landing and swap sheets.

## Other actor sheets

All current actor clips use 192 × 192 cells at pivot (70, 133) and scale 1.4. Exact frame counts, durations, contact poses and effect mappings are defined in `native-engine/src/flash_executioner_assets.hpp`. Do not infer a clip's frame count from the full atlas dimensions; some grids reserve trailing cells.

## Legacy atlas

`ui/white_night_motion.png` and its former frames 88–119 belonged to the previous class asset set. Those timings and source files are historical and do not describe the active Flash Executioner run sheet.

Runtime/device playback remains owner-verified. No build or gameplay test is claimed by this note.
