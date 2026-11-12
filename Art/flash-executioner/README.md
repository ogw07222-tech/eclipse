# Flash Executioner reviewed motion and resource-based light effects v2

The seven previously approved actor sheets are unchanged. All fifteen custom v1 effects have been rejected by the owner and removed from active bindings. Their source is historical only at `archive/rejected-custom-v1/`; old PNG bytes remain in Git history. Every active v2 effect comes from an existing finished [PixelFX resource](https://github.com/patch-gg/pixelfx/tree/5294518ce7c72e48b6100a744596e0acf57f2826/src/fx), pinned to commit5294518ce7c72e48b6100a744596e0acf57f2826.

| Control | Actor sheet | Gameplay duration / contact |
| --- | --- | --- |
| Attack (J/Z) | actor_basic.png | .46s / .184s, existing attack-speed scaling |
| Skill 1 (K/X) | actor_skill1.png | .50s / .125s |
| Skill 2 (U) | actor_skill2.png | .46s / .23s; sword lowered, opposite empty fist |
| Skill 3 (O) | actor_skill3.png | .36s / .15s; blank contact frame and arrival |
| Dash (Shift) | actor_dash.png | .15s |
| Swap (L/C/Tab), entering this class | actor_swap.png | .24s |
| Landing (joystick jump) | actor_land.png | .12s |

Ultimate/P, idle, run, jump and fall retain existing legacy actor art. Their new AI candidates did not pass the main review or current pose-rate target. This change does not modify physics, cooldowns, damage, target rules, marks, hitboxes or the ultimate's exactly20 lances. The previous seven actor PNG hashes are preserved.

Actor sheets have192×192 cells, pivot70/133, scale1.4. `reviewed/manifest.json` records dimensions, frame counts, hashes and provenance. `review-approval.json` records main agent static approval rather than owner device approval. Actor contact frames use a piecewise mapping registered to unchanged gameplay time; frame counts do not imply a uniform60fps cadence. Incomplete legacy motion migration remains explicitly pending.

The v2 mappings and options are in `pixelfx/manifest.json`. Original effect recipes and dependencies are byte-identical to the pinned upstream; source leaf hashes, MIT license and verification evidence are retained under `pixelfx/source-original/`. Variations use palette, exposed options, actual approved actor masks, uniform runtime scale, canvas registration and source-frame timing/selection. No new geometry recipes or hand-painted shapes were used. Each `pixelfx/variants/<Role>/` preserves a same-fps original baseline and exact frame map.

Effects retain straight RGBA pixels. Export packing corrects upstream atlas double-premultiplication through direct RGBA byte copies; limited alpha-only entrance/exit fades and frame selections are documented. No RGB painting, body pose synthesis or geometry replacement occurs.

| Event | v2 source-frame registration |
| --- | --- |
| Basic slash | Start1/30s before unchanged contact; account for basic-speed multiplier |
| Rush | Start.05s before unchanged contact |
| Command | Start.2s before unchanged contact; hand offset10,-25 |
| Blink departure | Start at action age1/30s; collapse aligns with teleport.15s |
| Blink arrival | Start at collision with source age2/15s |
| Generic impact | Start at collision with source age1/60s, centered on actual hit |
| Verdict impact | Smite source tail starts after original hit; register to target feet |
| Orbs | Complete original2.4s loop; focal offset0,+15.6 |
| Normal/sky lances | Complete original.8s loop until collision; both natively face right, rotate by actual projectile angle |

Effect frame selection uses metadata fps directly, including endpoint samples, instead of stretching N+1 frames across the nominal duration. Extremely fast basic attacks skip only unavailable source prelude so the original visual peak still registers to gameplay contact. Effects have independent visual lifetimes, rather than clipping their tails to the combat burst's .24s. A verdict collision's simultaneous generic impact is deduplicated visually into one large verdict hit without changing either combat record. Projectile effects continue after class swap. Charge/swap effects replace their old arrival decoration intentionally.

`native-engine/src/flash_executioner_assets.hpp` is generated only from approved metadata. `Tools/art/import_flash_executioner.py` requires explicit main approval, SHA256 matches, and sourceId/sourceCommit/sourceSha256 for every effect. It removes superseded PNGs from runtime and reviewed-source folders, preventing stale CMake/Android glob packaging. Unapproved actor roles intentionally retain legacy art. Declared missing/wrong-dimension sheets fail via `loadWhiteNight` and the existing `ECLIPSE ASSET ERROR` modal; failed required loads never silently substitute candidates.

Static inspection confirms22 runtime PNG files (7 actor roles,15 effect roles), zero active v1 effect hashes, and unchanged actor hashes. Lance and SkyLance intentionally share the same source PNG pixels but use separate scales/descriptors.

GitHub is canonical. ChatGPT Library capabilities are unavailable, so Library is **not synced**. A changed-files ZIP plus explicit deletion list is supplied for manual `/ECLIPSE/eclipse main/` reconciliation; extracting changed files alone does not remove old mirror paths.

Only static source/PNG/hash checks and main agent visual review were performed. Game tests, builds, APK, device playback and GitHub Actions were **not run**. Owner verification remains required for action timing, both facing directions, skill2 hands, teleport registration, effect occlusion and performance.
