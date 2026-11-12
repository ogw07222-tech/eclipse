# Demon Castle — fixed collision terrain integration (2026-11-12)

## Intended gameplay
- Hand-authored, **not random** maps A1–A5: A1 Ashen Approach, A2 Broken Gate, A3 Void Corridor, A4 Throne Stairway, A5 Night Throne.
- Single-image side-scrolling backdrops (not parallax tiles), with physical surfaces aligned to the authored art.
- Pit recovery: lose **ceil(maximum HP × 0.15)**, return to that area's left spawn, gain **0.65 seconds invulnerability**, snap camera to spawn. Lethal fall ends the run. Do not reset enemies or cooldowns.

## Current GitHub integration status
- **Source added:** `native-engine/src/castle_terrain.hpp` has authored floor segments, platforms, world widths, gap and spawn coordinates.
- **Physics updated:** `native-engine/src/combat.hpp` accepts `configureCastleArea(area)` and uses the terrain for collisions, enemy height and pit handling.
- **Camera updated:** `native-engine/src/main.cpp` snaps after pit recovery and uses active castle world width.
- **NOT ACTIVATED in default gameplay:** `castleArea` defaults to 0 deliberately. The old repository still uses `native-engine/src/demon_castle_entrance.hpp` five-layer art loader and lacks approved A1–A5 single-image PNG binaries. Calling `configureCastleArea` before replacing that loader/art would make graphics and collision disagree. Do not call this complete.
- **Pending prerequisite:** publish five original full-scene PNGs under `Assets/Resources/Eclipse/demon_castle/areas/`, migrate the art loader to 1 image/area, then select the same area for rendering and `Combat::configureCastleArea` atomically; delete obsolete entrance parallax layers only then.
- **Separate local state:** user's Termux checkout previously applied v31/v32/v33 and has unrelated worktree changes. This GitHub update does not automatically reconcile the user's local branch.
- **Tests / builds / GitHub Actions:** NOT RUN. Owner performs game/build/device acceptance.

## Physical coordinates
`CastleTerrain::groundAt` and `standingAt` use world-space foot Y values and must be measured against the approved A1–A5 images before activation. Authored A3 pit x = 1150–1220. Gravity and actor collision semantics beyond the authored patches are pending owner-run playtesting.

## Preserve
No character, monster, skill FX, UI, forest, or app-signing artifacts were intentionally changed as part of this commit series.
