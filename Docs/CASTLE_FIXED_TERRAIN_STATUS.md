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
- **A1–A5 route and portal integration (2026-11-14):** all five current 2172×724 backgrounds are selected in runtime. A generated 96×128 blue portal is drawn at each area's authored endpoints. A1→A2→A3→A4→A5 is the rightward route; leftward travel follows the reverse route, with A1-left paired to A5-right to close the route. A2-left is anchored near the broken gate. Near a grounded portal, the attack control displays `!`; pressing attack or `E` transitions to the paired portal. Health and equipment carry across; active combat effects are cleared and a brief arrival invulnerability is granted.
- Collision floor and portal coordinates were adjusted to the current artwork estimates in `castle_terrain.hpp`. Exact alignment and gameplay acceptance still require the owner's in-game review; do not claim tested or approved playability.
- **Older status superseded:** the previous statement that the runtime still used five parallax layers is no longer accurate; the repository now has the five independent single-image loader. Old character sprite retirement may still cause startup failure until replacement character atlases are published.
- **Separate local state:** user's Termux checkout previously applied v31/v32/v33 and has unrelated worktree changes. This GitHub update does not automatically reconcile the user's local branch.
- **Tests / builds / GitHub Actions:** NOT RUN. Owner performs game/build/device acceptance.

## Physical coordinates
`CastleTerrain::groundAt` and `standingAt` use world-space foot Y values. Current authored floor/portal points should be checked against the five updated images during owner review. Authored A3 pit x = 1150–1220. Gravity and actor collision semantics beyond the authored patches are pending owner-run playtesting.

## Preserve
No character, monster, skill FX, UI, forest, or app-signing artifacts were intentionally changed as part of this commit series.
