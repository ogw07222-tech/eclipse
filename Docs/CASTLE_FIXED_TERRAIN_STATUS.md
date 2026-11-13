# Demon Castle — fixed collision terrain and portal integration (2026-11-14)

## Intended gameplay
- Hand-authored, **not random** maps A1–A5: A1 Ashen Approach, A2 Broken Gate, A3 Void Corridor, A4 Throne Stairway, A5 Night Throne.
- Single-image side-scrolling backdrops (not parallax tiles), with physical surfaces aligned to the authored art.
- Pit recovery: lose **ceil(maximum HP × 0.15)**, return to that area's left spawn, gain **0.65 seconds invulnerability**, snap camera to spawn. Lethal fall ends the run. Do not reset enemies or cooldowns.

## Current GitHub integration status
- **Source added:** `native-engine/src/castle_terrain.hpp` has authored floor segments, platforms, world widths, gap and spawn coordinates.
- **Physics updated:** `native-engine/src/combat.hpp` accepts `configureCastleArea(area)` and uses the terrain for collisions, enemy height and pit handling.
- **Camera updated:** `native-engine/src/main.cpp` snaps after pit recovery and uses active castle world width.
- **A1 integration (2026-11-12):** the approved new entrance image is present at `Assets/Resources/Eclipse/demon_castle/areas/a1_ashen_approach.png`. The renderer expects 2172×724 for all five area PNGs; A1 world width is 2160 at 1280×720 virtual resolution. A1 ground follows the visible mostly-flat walkway, rising toward the right gate; player and enemy spawns use the same surface. `main.cpp` selects `configureCastleArea(1)` when the main castle scene is entered. GitHub's binary connector supplies the A1 blob SHA, not PNG bytes, so exact source-image byte comparison against the ChatGPT draft was **not** independently verified.
- **A1–A5 one-way route and portal integration (2026-11-14):** all five current 2172×724 backgrounds are selected in runtime. The generated 96×128 blue portal appears only at the right-side exits of A1–A4; entry sides show no portal and A5 has no outgoing exit. Near a grounded exit, the attack control displays `!`; pressing attack or `E` advances A1→A2→A3→A4→A5. Health and equipment carry across; active combat effects are cleared and a brief arrival invulnerability is granted. Reverse travel is not supported.
- **Floor alignment review (2026-11-14):** the authored ground polylines were overlaid on each exact 2160×720 in-game background composition. A1/A2 floors were raised to the illustrated walkway; A3's raised span follows its steps while preserving the pit; A4's slope now follows the stair surface; A5's throne approach is a continuous rising floor rather than a detached floating platform. This was a static renderer-scale visual review, not a live SDL gameplay session. The current environment has no built game binary or connected Android target, so owner in-game validation remains pending.
- **Older status superseded:** the previous statement that the runtime still used five parallax layers is no longer accurate; the repository now has the five independent single-image loader. Old character sprite retirement may still cause startup failure until replacement character atlases are published.
- **Separate local state:** user's Termux checkout previously applied v31/v32/v33 and has unrelated worktree changes. This GitHub update does not automatically reconcile the user's local branch.
- **Tests / builds / GitHub Actions:** NOT RUN. Owner performs game/build/device acceptance.

## Physical coordinates
`CastleTerrain::groundAt` and `standingAt` use world-space foot Y values. Current authored floor/portal points should be checked against the five updated images during owner review. Authored A3 pit x = 1150–1220. Gravity and actor collision semantics beyond the authored patches are pending owner-run playtesting.

## Preserve
No character, monster, skill FX, UI, forest, or app-signing artifacts were intentionally changed as part of this commit series.
