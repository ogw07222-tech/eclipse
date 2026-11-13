# Flash Executioner reviewed motion and resource-based light effects v2

Six previously approved actor sheets are unchanged. Basic Attack now uses one fixed impact pose; five motion sheets are added below. All fifteen custom v1 effects have been rejected by the owner and removed from active bindings. Their source is historical only at `archive/rejected-custom-v1/`; old PNG bytes remain in Git history. Every active v2 effect comes from an existing finished [PixelFX resource](https://github.com/patch-gg/pixelfx/tree/5294518ce7c72e48b6100a744596e0acf57f2826/src/fx), pinned to commit5294518ce7c72e48b6100a744596e0acf57f2826.

| Control | Actor sheet | Gameplay duration / contact |
| --- | --- | --- |
| Attack (J/Z or Attack button) | actor_basic.png | One fixed pose for .46s / .184s gameplay contact; existing attack-speed scaling |
| Skill 1 (K/X) | actor_skill1.png | .50s / .125s |
| Skill 2 (U) | actor_skill2.png | .46s / .23s; sword lowered, opposite empty fist |
| Skill 3 (O) | actor_skill3.png | .36s / .15s; blank contact frame and arrival |
| Dash (Shift) | actor_dash.png | .15s |
| Swap (L/C/Tab), entering this class | actor_swap.png | .24s |
| Landing (joystick jump) | actor_land.png | .12s |
| Ultimate (P / VERDICT button) | actor_ultimate.png | 1.12s; 25 poses, first four repetitive wind-up frames trimmed |
| Idle (automatic) | actor_idle.png | 0.90s loop; 17 poses |
| Run (move input) | actor_run.png | 29-pose distance-driven loop; nominal .80s |
| Jump/Fall (jump and vertical velocity) | actor_jump.png / actor_fall.png | 29 / 9 poses; .80s / .60s nominal playback |

Ultimate, idle, run, jump and fall now use the owner-reviewed motion sheets. Idle, movement and airborne motions are selected automatically from the current character state; the P/VERDICT control selects Ultimate through the existing Verdict action. Basic Attack stays one fixed contact pose. Only repeated Ultimate preparation frames were trimmed; no physics, cooldown, damage, target, hitbox or twenty-lance behavior changed.

Actor sheets have192×192 cells, pivot70/133, scale1.4. `reviewed/manifest.json` records dimensions, frame counts, hashes and provenance. `review-approval.json` records the owner-reviewed additions and exact hashes. Clip rates are motion-specific: Idle 18.9 poses/s, Run 36.3 nominal poses/s, Jump 36.3 nominal poses/s, Fall 15 poses/s and Ultimate 22.3 poses/s. The runtime maps Ultimate contact to frame 16 and retains its unchanged .728s gameplay contact. All actor cells are 192×192 RGBA at pivot70/133 and scale1.4.

The v2 mappings and options are in `pixelfx/manifest.json`. Original effect recipes and dependencies are byte-identical to the pinned upstream; source leaf hashes, MIT license and verification evidence are retained under `pixelfx/source-original/`. Most variations use palette, exposed options, actual approved actor masks, uniform runtime scale, canvas registration and source-frame timing/selection. The owner-requested hand-pixel Lance/SkyLance silhouette edit is documented below; their original source recipes remain unchanged. Each `pixelfx/variants/<Role>/` preserves a same-fps original baseline and exact frame map.

Effects retain straight RGBA pixels. Export packing corrects upstream atlas double-premultiplication through direct RGBA byte copies; limited alpha-only entrance/exit fades and frame selections are documented. Owner-requested direct pixel edits now reshape all 48 Lance/SkyLance frames into a long, thin shaft and tapered spear point while keeping the projectile-tip pivot, runtime scale, timing and attack behavior. The upstream recipes remain preserved byte-for-byte as the baseline.

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
| Normal/sky lances | Complete original.8s loop until collision; long thin hand-edited spear pixels, both natively face right and rotate by actual projectile angle |

Effect frame selection uses metadata fps directly, including endpoint samples, instead of stretching N+1 frames across the nominal duration. Extremely fast basic attacks skip only unavailable source prelude so the original visual peak still registers to gameplay contact. Effects have independent visual lifetimes, rather than clipping their tails to the combat burst's .24s. A verdict collision's simultaneous generic impact is deduplicated visually into one large verdict hit without changing either combat record. Projectile effects continue after class swap. Charge/swap effects replace their old arrival decoration intentionally.

`native-engine/src/flash_executioner_assets.hpp` is generated only from approved metadata. `Tools/art/import_flash_executioner.py` requires explicit main approval, SHA256 matches, and sourceId/sourceCommit/sourceSha256 for every effect. It removes superseded PNGs from runtime and reviewed-source folders, preventing stale CMake/Android glob packaging. Roles without an approved actor sheet retain legacy art. Declared missing/wrong-dimension sheets fail via `loadWhiteNight` and the existing `ECLIPSE ASSET ERROR` modal; failed required loads never silently substitute candidates.

Static inspection confirms27 runtime PNG files (12 actor roles,15 effect roles), zero active v1 effect hashes. Six previously approved actor sheets remain byte-identical; Basic is now a one-pose sheet. Lance and SkyLance intentionally share the same source PNG pixels but use separate scales/descriptors.

GitHub main is the sole canonical project source.

Only static source/PNG/hash checks and main agent visual review were performed. Game tests, builds, APK, device playback and GitHub Actions were **not run**. Owner verification remains required for action timing, both facing directions, skill2 hands, teleport registration, effect occlusion and performance.
