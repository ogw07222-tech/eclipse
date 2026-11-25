# Flash Executioner reviewed motion and resource-based light effects v2

Six previously approved actor sheets remain unchanged. Basic Attack now uses the approved 28-pose sequence with contact mapped to pose 6. The active package retains the five reviewed motion sheets below. All fifteen custom v1 effects have been rejected by the owner and removed from active bindings. Their source is historical only at `archive/rejected-custom-v1/`; old PNG bytes remain in Git history. Every active v2 effect comes from an existing finished [PixelFX resource](https://github.com/patch-gg/pixelfx/tree/5294518ce7c72e48b6100a744596e0acf57f2826/src/fx), pinned to commit5294518ce7c72e48b6100a744596e0acf57f2826.

| Control | Actor sheet | Gameplay duration / contact |
| --- | --- | --- |
| Attack (J/Z or Attack button) | actor_basic.png | 28 poses over .46s / .184s gameplay contact; contact at pose 6; existing attack-speed scaling |
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

Ultimate, idle, run, jump and fall use the current reviewed motion sheets. Idle, movement and airborne motions are selected automatically from the current character state; the P/VERDICT control selects Ultimate through the existing Verdict action. Basic Attack uses the current 28-pose reviewed sequence, with its existing .184s contact timing preserved. Only repeated Ultimate preparation frames were trimmed; no physics, cooldown, damage, target, hitbox or twenty-lance behavior changed.

Actor sheets have192×192 cells, pivot70/133, scale1.4. `reviewed/manifest.json` records dimensions, frame counts, hashes and provenance. `review-approval.json` records the owner-reviewed additions and exact hashes. Clip rates are motion-specific: Idle 18.9 poses/s, Run 36.3 nominal poses/s, Jump 36.3 nominal poses/s, Fall 15 poses/s and Ultimate 22.3 poses/s. The runtime maps Ultimate contact to frame 16 and retains its unchanged .728s gameplay contact. All actor cells are 192×192 RGBA at pivot70/133 and scale1.4.

The v2 mappings and options are in `pixelfx/manifest.json`. Original effect recipes and dependencies are byte-identical to the pinned upstream; source leaf hashes, MIT license and verification evidence are retained under `pixelfx/source-original/`. Most variations use palette, exposed options, actual approved actor masks, uniform runtime scale, canvas registration and source-frame timing/selection. The owner-requested hand-pixel Lance/SkyLance silhouette edit is documented below; their original source recipes remain unchanged. Each `pixelfx/variants/<Role>/` preserves a same-fps original baseline and exact frame map.

Effects retain straight RGBA pixels. Export packing corrects upstream atlas double-premultiplication through direct RGBA byte copies; limited alpha-only entrance/exit fades and frame selections are documented. Owner-requested direct pixel edits now reshape all 48 Lance/SkyLance frames into a long, thin shaft and tapered spear point while keeping the projectile-tip pivot, runtime scale, timing and attack behavior. The upstream recipes remain preserved byte-for-byte as the baseline.

| Event | v2 source-frame registration |
| --- | --- |
| Basic slash | 19 visible frames plus transparent release frame; starts 1/30s before unchanged contact and scales its lifetime with Basic attack speed |
| Rush | Start.05s before unchanged contact |
| Command | Start.2s before unchanged contact; hand offset10,-25 |
| Blink departure | Start at action age1/30s; collapse aligns with teleport.15s |
| Blink arrival | Start at collision with source age2/15s |
| Generic impact | Start at collision with source age1/60s, centered on actual hit |
| Verdict impact | Smite source tail starts after original hit; register to target feet |
| Orbs | Complete original2.4s loop; focal offset0,+15.6 |
| Normal/sky lances | Complete original.8s loop until collision; long thin hand-edited spear pixels, both natively face right and rotate by actual projectile angle |

Effect frame selection uses metadata fps directly, including endpoint samples, instead of stretching N+1 frames across the nominal duration. Extremely fast basic attacks skip only unavailable source prelude so the original visual peak still registers to gameplay contact. Effects have independent visual lifetimes, rather than clipping their tails to the combat burst's .24s. A verdict collision's simultaneous generic impact is deduplicated visually into one large verdict hit without changing either combat record. Projectile effects continue after class swap. Charge/swap effects replace their old arrival decoration intentionally.

`native-engine/src/flash_executioner_assets.hpp` is generated only from approved metadata. `Tools/art/import_flash_executioner.py` requires explicit main approval, SHA256 matches, and sourceId/sourceCommit/sourceSha256 for every effect. It removes superseded PNGs from runtime and reviewed-source folders, preventing stale CMake/Android glob packaging. Roles without an approved actor sheet retain legacy art. Declared missing/wrong-dimension sheets fail via `loadFlashExecutioner` and the existing `ECLIPSE ASSET ERROR` modal; failed required loads never silently substitute candidates.

Static inspection confirms27 runtime PNG files (12 actor roles,15 effect roles), zero active v1 effect hashes. The approved 28-pose Basic and current Slash/Lance/SkyLance sheets match the accompanying ECLIPSE source manifest. Jump and Land effect sheets now play on takeoff and landing. Lance and SkyLance intentionally share the same source PNG pixels but use separate scales/descriptors.

GitHub main is the sole canonical project source.

Only static source/PNG/hash checks and main agent visual review were performed. Game tests, builds, APK, device playback and GitHub Actions were **not run**. Owner verification remains required for action timing, both facing directions, skill2 hands, teleport registration, effect occlusion and performance.

## Mid-frame and light-crescent update (2026-11-22)

- Basic, Skill1 and Dash actor sheets gained AI-generated mid-frames to reduce large pose jumps; Skill2, Skill3, Swap and Land are unchanged on purpose (Skill3 position jumps are intentional blink frames).
- Basic attack keeps its gameplay duration (0.46s) and contact time (0.184s); the inserted frames play inside the same wind-up. `contactFrame` moved to the matching contact pose.
- Dash `contactFrame` stays -1 (no contact frame).
- Mid-frames were generated with PixelLab `animate_image` with the original neighbouring frames pinned. The redrawn Basic blade frame (0-based 8) was edited in the PixelLab pixel workbench. Nothing is hand-authored, and no effects are baked into any actor sheet.
- Slash effect is now the custom `melee/blade-light-crescent` (33 frames, 5x7 grid of 320x240 cells, fps 30, duration 1.1s, pivot 120,140, scale 1.4, offset 6,-13, triggerLeadSeconds 0.159, layer front). It is remixed from PixelFX recipes `melee/sword-slash`, `melee/scythe-reap` and `holy/divine-shield`, so it is NOT an unmodified upstream resource. This deviates from the earlier "unmodified PixelFX resource only" rule by the owner's explicit decision; the policy exception is recorded here. Source: `pixelfx/blade-light-crescent.js` and `pixelfx/blade-light-crescent-sheet.json`.
- Engine: for `FlashMove::Slash`, `playbackDuration = fxSheet.duration / speed` (no longer stretched to the attack clip's remaining time) in `native-engine/src/presentation.hpp`.
- `Tools/art/import_flash_executioner.py` was run on `review-approval.json`, accepting all 27 sheets; `native-engine/src/flash_executioner_assets.hpp` was regenerated (only the Basic, Skill1, Dash and Slash lines changed).
- Backups of the previous sheets, approval JSON and header are in `pre-midframe-originals/`. Generation totals: 17 PixelLab generations, 11 of which are in shipped assets (Basic 5, Dash 3, Skill1 3).

| Asset | Frames (old -> new) | contactFrame (old -> new) | Source |
| --- | --- | --- | --- |
| `reviewed/actor_basic.png` (6x6, 192px cells, 1152x1152) | 28 -> 35 (7 inserted after original F6, 0-based 6-12) | 6 -> 13 | PixelLab `animate_image`, F6 and F7 pinned, seed 31, 5 generations; blade of 0-based frame 8 redrawn in pixel workbench |
| `reviewed/actor_dash.png` (4x4, 192px cells, 768x768) | 9 -> 13 (4 inserted after F1) | -1 -> -1 | PixelLab `animate_image`, F1 and F2 pinned, seed 7, 3 generations |
| `reviewed/actor_skill1.png` (6x6, 192px cells) | 30 -> 33 (3 inserted after original F6) | 6 -> 9 | PixelLab `animate_image`, F6 and F7 pinned, seed 41, 3 generations; a 4th near-duplicate of F7 was dropped Burst/arrowhead shapes the AI drew on mid-frames 2 and 3 were removed and replaced with partial original F7 blade pixels (no baked effects). |
| `reviewed/effect_slash.png` (5x7, 320x240 cells) | replaced by 33-frame custom effect (1.1 s) | n/a | Custom `melee/blade-light-crescent` remixed from `melee/sword-slash`, `melee/scythe-reap`, `holy/divine-shield` (owner-approved exception) |

Runtime copies in `Assets/Resources/Eclipse/flash_executioner/` match the reviewed files pixel for pixel.

Not built, tested or device-checked. No GitHub Actions were run. Owner device verification is pending for the mid-frame timing, the Slash playback duration and the custom crescent effect.

### Mid-air burst effects (2026-11-22)

The Impact and LanceHit hit effects and the Slash light crescent now burst in mid-air instead of assuming a floor. The upstream PixelFX recipes are floor-based (crit-hit kicks sparks upward with bouncing gravity shards; smite is a floor shockwave with stone chips and dust), so these are custom effects, not unmodified upstream resources. This is the owner's explicit decision (2026-11-22), recorded in `review-approval.json`.

| Effect | Old source | New source | Grid / frames | Anchor |
| --- | --- | --- | --- | --- |
| Impact (`effect_impact.png`) | `impacts/crit-hit` | `impacts/air-hit` (`pixelfx/air-hit.js`) | 5x4, 240x240 cells, 16 frames, pivot 120,120, scale 0.7, fps 30, 0.53 s | impact centre |
| LanceHit (`effect_lancehit.png`) | `holy/smite` | `holy/air-burst` (`pixelfx/air-burst.js`) | 6x4, 320x320 cells, 23 frames, pivot 160,160, scale 0.85, fps 30, 0.77 s | impact centre (targetFeet 0) |
| Slash (`effect_slash.png`) | `melee/blade-light-crescent` with gravity-based particles | same sheet; sparks and motes scatter with drag, no gravity or floor (`pixelfx/blade-light-crescent.js`) | 5x7, 320x240 cells, 33 frames, scale 1.4, 1.1 s, layer front (drawn in front of the fighter) | unchanged |

- Impact and LanceHit are symmetric. Impact is a burst of light in the gold-white palette: a soft white bloom flash, one luminous ring, a lens cross, a few wide soft rays, glints and drifting light motes, all spreading in every direction with no spark streaks, no gravity and no floor. LanceHit uses the same light-burst language, larger: a bloom flash, two luminous rings, rays, glints and motes in every direction.
- 2026-11-22 revision: rebuilt as soft light bursts (no spark streaks) and reduced in size at the owner's request; current scales are Impact 0.7 and LanceHit 0.85 (earlier versions used 1.7 and 1.5).
- Previous sheets are kept in `pre-midframe-originals/` (`effect_impact.png`, `effect_lancehit.png`, `effect_slash_crescent_v1.png`).
- `Tools/art/import_flash_executioner.py` was re-run and accepted all 27 sheets. `native-engine/src/flash_executioner_assets.hpp` now points Impact and LanceHit at the new sheets and dimensions.
- 2026-11-22 scale update: the basic attack Impact is scale 0.7 and LanceHit is scale 0.85. Command and Rush hits use the new ImpactLance and ImpactRush sheets (see Hit effect variants).
- Jump and Land (`impacts/heavy-landing`) are ground effects by nature and are unchanged. Rush, Command, BlinkOut/BlinkIn, Swap, Dash, Orb, Lance and SkyLance were not changed and were not re-checked for floor assumptions.

Not built, tested or device-checked. No GitHub Actions were run. Owner device verification is pending.

### Hit effect variants (2026-11-22)

The hit effect is fixed per action: the same action always shows the same effect, different actions show different effects, and stronger actions look stronger (bigger and heavier). All are custom mid-air bursts of light in the gold-white palette, with no floor, no gravity and no spark streaks, derived from `impacts/crit-hit` and `holy/smite`. They are not unmodified upstream PixelFX resources, as recorded in `review-approval.json`. The earlier rotating variant set was replaced by this design.

| Tier (weakest to strongest) | Action | File | Source | Frames / grid | Scale | Duration | Look |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Basic attack | Impact | `effect_impact.png` | `impacts/air-hit` (`pixelfx/air-hit.js`) | 16 / 5x4, 240x240, pivot 120,120 | 0.7 | 0.53 s | Small round bloom flash, one ring, lens cross, soft rays, glints |
| Command skill (stored light lances) | ImpactLance | `effect_impactlance.png` | `impacts/air-hit-cross` (`pixelfx/air-hit-cross.js`) | 12 / 5x3, 240x240, pivot 120,120 | 0.8 | 0.40 s | Two crossing blades of light |
| Rush skill | ImpactRush | `effect_impactrush.png` | `impacts/air-hit-rush` (`pixelfx/air-hit-rush.js`) | 16 / 5x4, 280x280, pivot 140,140 | 0.8 | 0.53 s | Heavy long eight-point star, big bloom, two shock rings, ring of glints |
| Verdict (ultimate) lance | LanceHit | `effect_lancehit.png` | `holy/air-burst` (`pixelfx/air-burst.js`) | 23 / 6x4, 320x320, pivot 160,160 | 0.85 | 0.77 s | Tails cut (27 -> 23 frames) and scale reduced from 1.0 to 0.85 (2026-11-22 readability pass); 20 lances each burst, plus the Verdict pillar |

- Engine, `native-engine/src/combat.hpp`: `enum LightFx` gained `SlashHit` and `RushHit` at the end. The basic attack publishes `LightFx::SlashHit` for each enemy hit when `player.flashExecutioner` is set (before this change the basic attack published no hit burst). The Rush skill hit publishes `LightFx::RushHit` instead of `LightFx::Impact`. Lance landings still publish `LightFx::Impact`.
- Mapping, `native-engine/src/presentation.hpp`: Impact -> `FlashFx::ImpactLance`, SlashHit -> `FlashFx::Impact`, RushHit -> `FlashFx::ImpactRush`, Verdict -> `FlashFx::LanceHit`. The plain Impact is still suppressed when a verdict pair exists. The legacy "replaced" check was extended for the new kinds so the old procedural burst is not drawn on top.
- Importer: `Tools/art/import_flash_executioner.py` EFFECTS list and FlashFx header template end with `..., Land, ImpactLance, ImpactRush`, in that order. The tool accepted 29 sheets and `native-engine/src/flash_executioner_assets.hpp` was regenerated. The obsolete variant PNGs were removed from Art and runtime by the tool.
- Packaging: CMake and Android Gradle package `flash_executioner/*.png` by glob, so no build-file change was needed.
- Not compiled: the C++ edits are static only. No build or test was run, and no GitHub Actions were run. Owner device verification is pending.
- Open: Command lances and Verdict lances share the Impact burst kind (the Verdict case is handled by the pair rule). Blink arrival and Dash, Swap, Jump and Land effects were not reviewed for this tiering.

### Readability check (2026-11-22)

Measured with `occlusion_metric.py` on the same takes, rendered with and without effects. Only the four hit effects (Impact, ImpactLance, ImpactRush, LanceHit) changed; the Slash crescent is unchanged.

| Take | Player silhouette covered, peak (before -> after) | Screen coverage, peak (before -> after) |
| --- | --- | --- |
| Basic attack x3 (3.55 s) | 28% -> 28% | not recorded (1.95 s of 3.55 s with any effect on screen) |
| Rush | 77% -> 58% | 0.9% -> 0.5% |
| Command | 10% -> 10% | not recorded -> 1.0% |
| Verdict | 0% -> 0% | 3.2% -> 2.9% |

- What changed: the nearly transparent tails of the four hit effects were cut (Impact 18 -> 16 frames, ImpactLance 18 -> 12, ImpactRush 21 -> 16, LanceHit 27 -> 23) and their scales were lowered (ImpactLance 0.9 -> 0.8, ImpactRush 1.2 -> 0.8, LanceHit 1.0 -> 0.85; Impact stays at 0.7). The Slash crescent (33 frames, 1.1 s, scale 1.4, in front of the fighter) was not changed. A shortened, rear-layer Slash was tried and reverted at the owner's request.
- Measuring tool: `Art/ui/review-before-after/motion-review/occlusion_metric.py` renders a take with and without effects and reports screen coverage, player silhouette coverage and seconds with an effect on screen. The seconds value did not change for Rush, Command and Verdict, because it is dominated by non-hit effects (orbs, pillar, rush trail).
- Remaining rush overlap: the fighter rushes through the target, so the Rush hit still covers up to 58% of the player for about 0.1 s at the contact moment. Owner device verification is still pending; nothing was built or tested.
