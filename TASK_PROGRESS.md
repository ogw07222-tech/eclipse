# TASK_PROGRESS — ECLIPSE Pixelbolt + PixelFX integration

Updated: 2026-11-08
Status: PARTIAL — local setup and in-game integration NOT verified.

## Completed
- Read `AGENTS.md`, `README.md`, PNG workflow and inspected the two effective workflows (manual `workflow_dispatch` only). No GitHub Actions run.
- Confirmed official upstream projects and licenses: `skulitom/Pixelbolt` (MIT), `patch-gg/pixelfx` (MIT code/effects, font exception).
- Added `tools/pixel-tools.sh`: opt-in clone/install, version reporting, Pixelbolt preview/inspect/render, PixelFX gallery/sheet/check.
- Added `Docs/PIXEL_TOOL_INTEGRATION.md`: Termux commands, output handling, manual art/QA and runtime gates.
- Verified both new files can be fetched from `main` after commits. No game assets touched.

## Files changed
- `tools/pixel-tools.sh` — added at dc0c2ce35ee0838d02624330fb886cfb4861030b.
- `Docs/PIXEL_TOOL_INTEGRATION.md` — added at cad787e35e1a7053b29936c257c1937bb558db8f.
- `TASK_PROGRESS.md` — this record.

## Blockers / not completed
- Termux is on the owner's Galaxy Tab, not available in this execution environment. The local wrapper has **not** been installed or exercised on the device. Do not claim it works there.
- Neither production sprites nor SDL3 combat effect integration have been generated.
- The CMU Motion Library JSON/BVH -> Pixelbolt pose JSON converter is not built.
- Existing gameplay art must be manually reviewed/approved; automated generated/rigged frames are not final artwork under `AGENTS.md`.
- No tests/builds run, including no GitHub Actions.

## Next steps on '이어서'
1. User can run `bash tools/pixel-tools.sh install` inside their existing `~/eclipse` Termux checkout, then `bash tools/pixel-tools.sh versions`. Capture exact errors if any.
2. Run Pixelbolt sample render and PixelFX sample sheet in local shell if owner grants permission for local test; adjust wrapper for environment-specific issues.
3. Create a small versioned test recipe in `Art/`, inspect native-size preview + frame metadata.
4. Implement Motion Library JSON mapping using an actual sample schema, not guessed coordinates.
5. Stage drafts only; integrate game-runtime PNGs and mappings after reviewed approvals. Owner runs game tests.

No requested account credentials, tokens, signing keys or GitHub Actions permissions are needed to proceed.

## 2026-11-08 — Pixelbolt live render attempt
- User requested actual Pixelbolt generation from Library `/ECLIPSE/characters/white-night/white_night_motion.png`.
- Library original materialized: 512x1440 PNG; frames 88–95 are 64x96.
- Node v22.16.0 was available in local container, but Pixelbolt was not installed. `git ls-remote https://github.com/skulitom/Pixelbolt.git HEAD` failed: `Could not resolve host: github.com`; npm cache had no Pixelbolt package. Therefore **Pixelbolt was not executed**.
- Created local downloadable `/mnt/data/white_night_pixelbolt_reference.pixel.json` by exact per-pixel transfer of original frames 88–95 into Pixelbolt recipe `frames[].pixels` and an 8-frame animation. This is a conversion input, **not** a Pixelbolt-generated novel animation, and remains unvalidated by Pixelbolt CLI. No game source, runtime PNG or GitHub Actions modified.
- Resume: Run `node bin/pixelbolt.js validate <recipe>` then `inspect` and `render` in a Node environment with Pixelbolt installed, check image bytes/atlas. Afterwards develop genuine poses-based variant and visually inspect all frames. Do not claim actual tool execution until achieved.

## 2026-11-08 — 2D Pixel Art Pipeline architecture v2 (GitHub design)

Status: PARTIAL — documentation and metadata committed; exporter/render adapters and runtime art integration NOT implemented.

### Completed
- Re-read game `AGENTS.md`, `README.md`, `Docs/PNG_GITHUB_LIBRARY_WORKFLOW.md` and Motion Library instructions.
- Inspected `.github/workflows/native-engine-checks.yml` and `.github/workflows/native-android-apk.yml`: both manual `workflow_dispatch` only. No GitHub Actions dispatched.
- Reused existing `tools/pixel-tools.sh` (Pixelbolt/PixelFX) and `native-engine/tools/eclipse_sprite_tool.py` (Pillow). Did not claim these executed.
- Created `Docs/ART_PIPELINE_V2.md` at commit `0da783e4735a954a3d2926f86f49650df37158c6`.
- Created `Art/pipeline/recipes/white_night_run.json` at commit `8bebd2e6deec0f5847c3a1e661c9df63668f84d7`.
- Re-fetched both new files from main, confirming their presence.
- No existing PNGs, animations, engine sources, signing material or workflows changed.

### Not completed / blockers
- The JSON is orchestration metadata, not native Pixelbolt/PixelFX schema; rendering, extract/compare/stage CLI, Sprite Fusion adapter and LibreSprite installation are not implemented.
- No original PNG bytes or hashes inspected in this task; no art generated, visually reviewed or integrated.
- Library mirror at `/ECLIPSE/eclipse main/` for the two new files is NOT verified/synchronized. GitHub main is ahead until an explicit byte-matching Library upload occurs.
- No tests, builds or device playback performed (owner-controlled).

### Next steps on '이어서'
1. Retrieve the actual source PNG bytes and verify original dimensions/hashes against the manifest, without overwriting the runtime asset.
2. Implement a minimal read-only Python `inspect/extract/compare` adapter reusing existing Pillow analysis and stable recipe schema, with staged output only in `Art/`.
3. Add an effect recipe and native Pixelbolt/PixelFX adapter only after verifying tool schema and installation in a suitable shell.
4. Produce before/after frame contacts for human review; no auto-approval of character artwork.
5. Explicitly sync committed docs/recipe with Library and compare bytes, or record mirror divergence.

## 2026-11-08 — ChatGPT execution environment probe and Pillow prototype

Status: PARTIAL — local tooling verified; Pixelbolt unavailable; production source not edited.

### Completed
- ChatGPT container: Python 3.13.5, Pillow and NumPy available; Node v22.16.0, npm and git present.
- Tested Pixelbolt network access: `git ls-remote https://github.com/skulitom/Pixelbolt.git HEAD` failed DNS resolution for github.com. `npm view pixelbolt version` failed `EAI_AGAIN` for registry.npmjs.org. Pixelbolt NOT installed or executed.
- Authored and locally executed `/mnt/data/eclipse-art-toolkit/pixel_patch.py` as a non-destructive Pillow prototype. Requires source SHA-256, precise row-major frame+pixel coordinates, optional expected old RGBA, distinct nonexisting output. Reports changed pixels/frames and sha256, status DRAFT_NOT_REVIEWED.
- Synthetic 2-frame PNG verified: only one pixel in frame #1 changed; frame #0 and original PNG unchanged; accidental overwrite rejected. Python bytecode compilation succeeded. NOT an ECLIPSE game asset test.
- Local downloadable prototype bundle: `/mnt/data/eclipse-art-toolkit.zip` (conversation artifact, not yet committed to GitHub).
- No GitHub Actions, APK build, game testing or production asset overwrite.

### Outstanding
1. GitHub source file for `pixel_patch.py` was not committed; only local exported ZIP exists. Copy after code review and verify source bytes.
2. Retrieve exact existing production PNG bytes (and hashes); this synthetic fixture is not the actual character sprite.
3. Install Pixelbolt in an environment with working npm/GitHub DNS or use a verified offline source archive, then validate its native schema and CLI. Do not infer runtime success.
4. Implement full review compare/contact sheet and approval workflow; integrate only after visual review and owner approval.
5. Check Library mirror separately; no automatic sync.

## 2026-11-08 — Uploaded Pixelbolt v0.13.0 source: actual offline execution VERIFIED

Status: Pixelbolt CLI sample rendering SUCCESS in ChatGPT container. ECLIPSE art integration remains NOT DONE.

### Completed
- Received user-provided `Pixelbolt-main.zip` (~7.3 MB). Inspected archive for traversal and symlinks before extraction. Package metadata: `pixelbolt` v0.13.0, Node >=20, zero runtime dependencies (as declared).
- Extracted safely into the ChatGPT container. Node v22.16.0.
- Executed `node bin/pixelbolt.js validate examples/forest-spirit.json`: `ok:true`, 24×24, 6 source frames, idle/bob/blink animations, zero validation warnings.
- Executed `node bin/pixelbolt.js render examples/forest-spirit.json --out /mnt/data/pixelbolt_render_demo`: `ok:true`, produced actual 156×26 RGBA sprite atlas, individual animation PNGs, `forest-spirit.atlas.json`, preview HTML and other output files.
- Opened resulting images with Pillow: source atlas 156×26 RGBA; idle/bob/blink PNG 24×24 RGBA each. Downloadable result bundle: `/mnt/data/pixelbolt_verified_output.zip`.
- The prior Pixelbolt DNS/network blocker is now BYPASSED for this uploaded, dependency-free source checkout; network install is unnecessary for this copy.
- Read effective two game workflows again, both manual dispatch-only. No GitHub Actions, build, or game tests executed.

### Limitations / next steps
1. The uploaded ZIP is an extracted local tool, NOT a permanent global plugin or persistent installation across new execution environments. Retain ZIP for reuse.
2. Only packaged sample recipe rendered, not a novel ECLIPSE sprite. Existing game asset PNGs untouched.
3. Need a validated native Pixelbolt recipe for a specific ECLIPSE effect, then native-size alpha/frame/contact review and user art approval. Never treat procedural character frames as hand-authored final poses.
4. Production integration, external Library synchronization, Termux device execution, CMU adapter and Sprite Fusion remain unverified/not implemented.


## 2026-11-13 — Flash Executioner reviewed asset/button integration

- Main agent statically reviewed and approved 7 actor sheets (Basic, Rush/Skill1, opposite-hand Command/Skill2, Blink/Skill3, Dash, entering-class Swap, Landing) and 15 independent PixelFX light effects.
- Source PNG/JS/prompt/receipt/license and verified hashes: Art/flash-executioner/. Exact runtime PNG copies: Assets/Resources/Eclipse/flash_executioner/.
- Native descriptors, strict dimension-checked loading, nearest-neighbor anchored/mirrored rendering, desktop/Android packaging, contact-timed visual event pool and per-projectile nonblank flight cycles are prepared.
- Combat changes are presentation-only serial/flight clock fields. Damage, physics, hitboxes, cooldowns, target rules and exactly20 ultimate lances are unchanged.
- Ultimate, Idle, Run, Jump/Fall use existing legacy actor art; rejected candidates were not imported into runtime. True60-pose/sec migration remains incomplete for those roles.
- Static diff/PNG/hash/source checks only. Builds, tests, device playback and GitHub Actions NOT RUN.
- Library mirror is NOT SYNCED: no ChatGPT Library capability. Matching-byte changed-file ZIP supplied separately for manual /ECLIPSE/eclipse main/ import. GitHub commit without Library upload is partial sync, not a complete mirror.
- Remaining owner validation: device timing and both facing directions; skill2 hands; teleport frame/target position; FX front/back occlusion/performance. Remaining art: full qualified Ultimate/Idle/Run/Jump/Fall re-authoring and review.


## 2026-11-13 — Owner-requested character archive purge

- Preserve all today's Flash Executioner source, actor and effect sheets, original_192.png and production sources; preserve backgrounds, enemies and UI. Static comparison confirmed 184 existing main asset/source files retain identical Git blob IDs.
- Purge 226 historical paths: obsolete playable character sprite sheets, associated art sources/reviews, legacy archived game trees and obsolete character art scripts. Rewrite all 78 branch histories to remove these paths. Existing game implementation is otherwise retained.
- Historical branch workflow definitions changed to manual workflow_dispatch only before publication. No Actions, builds or owner tests run.
- Library unavailable: /ECLIPSE/eclipse main/ and historical Library archives are NOT synchronized or purged. GitHub PR refs/cached commits, forks and other clones may retain old bytes; branch history cleanup cannot guarantee physical server erasure.
- Existing eclipse-integration working directory has owner changes and was not overwritten. It must not be pushed to restore old history.

## 2026-11-13 — Replace rejected custom FX with existing PixelFX resources v2

- Owner rejected all15 custom v1 effects. Their source is marked historical/rejected in Art/flash-executioner/archive/rejected-custom-v1; none is active or packaged.
- Main reviewed all15 existing-resource variants, pinned upstream recipes at5294518ce7c72e48b6100a744596e0acf57f2826, original leaf hashes, side-by-side variants and light/dark actor registration.
-7 approved actor PNGs unchanged.15 active effect roles now bind reviewed upstream-based variants; runtime has exactly22 PNG files and no v1 effect hash.
- Added metadata-driven source peak lead/spawn ages, absolute Blink departure age, focal offsets, direct fps selection and native projectile direction. Game contact/damage/physics values are unchanged.
- Full source loops replace v1 flight-frame-range samples. Importer cleans superseded PNGs and requires actual upstream effect identity.
- Exact sources, MIT license, prompts/actor sources, options, frame maps and baseline PNGs retained in Art. Library remains NOT SYNCED; supply matching-byte delta ZIP plus deletion list.
- Static validation/main visual review only. Game tests, builds, APK/device checks and GitHub Actions NOT RUN.


## 2026-11-13 — Flash Executioner manual pixel correction (partial)

Status: PARTIAL — corrected one visible Ultimate artifact; no runtime asset was approved or replaced.

- Kept the existing Basic actor sheet and its single fixed attack pose unchanged.
- Recolored 150 individual pixels on Ultimate frame 8 around the sword blade using exact source-coordinate RGBA assignments; no generation, AI image edit, interpolation or raster copy was used.
- Preserved the edited frame, updated contact pages and pixel audit under `Art/flash-executioner/pixellab/manual-corrections/ultimate/`.
- Visual check confirms the saturated violet blade block is now a silver/lilac blade.
- Did not import this animation into `Assets/Resources/` or change runtime descriptors: selected candidate is 29 frames at 100 ms/frame (10 authored poses/sec), below the repository requirement of 60 distinct poses/sec. Idle/run/jump/fall/ultimate motion as a whole also has not passed continuity and contact review.
- No GitHub main write or Library upload was performed. Library mirror capability is unavailable in this session; main requires matching Library files in the same work session.
- No builds, tests, device playback or GitHub Actions were run.

Next steps: continue direct pixel authoring of the deficient non-Basic motions to 60 distinct coherent poses/sec, inspect all frames for continuity, update atlas metadata, and only then prepare runtime import plus the corresponding Library mirror update.


## 2026-11-13 — Owner-approved Flash Executioner motion integration

Status: IMPLEMENTED — actor sheets and existing state/button bindings are connected in the latest main-based worktree.

- Owner clarified that motion-specific pose rates are acceptable, asked for shorter waits/preparation and faster playback, and approved the current generated motions after trying them. Latest `AGENTS.md` now explicitly removes a universal 60-pose/sec target.
- Added Idle (17 poses/.90s loop), Run (29-pose distance-driven loop/.80s nominal), Jump (29/.80s nominal), Fall (9/.60s nominal), and Ultimate (25/1.12s). Ultimate opening repetitive prep frames 1–4 are trimmed; contact stays frame 16 at the existing .728s action contact.
- Basic Attack is now one fixed impact pose for the full .46s action. Original 28-pose sheet is preserved at `Art/flash-executioner/pixellab/manual-corrections/basic/actor_basic_28_pose_backup.png`.
- Existing presentation mapping connects Slash/Basic, Rush/Skill1, Command/Skill2, Blink/Skill3, Verdict/Ultimate; Idle, Run, Jump and Fall are selected from movement state and vertical velocity. P/VERDICT and the existing J/Z/Attack controls are unchanged.
- Added 5 runtime actor PNGs and updated Basic PNG, approved metadata, generated descriptors, source frames, atlases, owner review notes and documentation. Manual Ultimate blade recolor is included in runtime frame 5. No VFX was added.
- `Tools/art/import_flash_executioner.py` now safely skips copying an input file onto itself, so approved existing source sheets can be imported from `Art/flash-executioner/reviewed/`.
- Static PNG dimension, alpha mode, runtime/source byte hash and manifest-count checks passed: 27 runtime sheets (12 actors + 15 effects).
- No tests, builds, game/device playback or GitHub Actions were run in this integration turn. Owner previously reported trying the motions and finding playback acceptable; both-facing/device verification remains outstanding.
- GitHub main head at integration start: `74a224e4f832075a19a9dca14a2f001e55166ea2`.


## 2026-11-13 — Owner-requested light spear effect revision

Status: IMPLEMENTED — all existing effect roles remain in runtime; Lance/SkyLance now use the manually revised spear silhouette.

- Verified all15 active Flash Executioner effect roles already had runtime PNGs and `FlashFx` descriptors from the preceding main update; no extra unbound effect candidates were present.
- Manually edited each of the48 frames in both Lance and SkyLance atlases. Removed the old blade/guard pixels, softened the round tip halo, painted a3px-long shaft and a tapered point, and kept the existing tip pivot, scale, loop, angle and combat timing. Both roles share the same edited PNG bytes.
- Preserved pre-edit atlases and created a reproducible guarded pixel-edit script at `Art/flash-executioner/pixelfx/manual-corrections/lance/`. Upstream JS recipes and original source hashes remain unchanged.
- Updated runtime PNGs, reviewed sources, approval hashes, PixelFX manifest, project documentation and audit record.
- Static review at 1x and 4x confirms the long shaft and pointed tip; all15 effects remain active. The script reproduces the runtime atlas SHA exactly.
- No tests, builds, game/device playback or GitHub Actions were run.

## 2026-11-13 — Space Ruler Download pack runtime integration

Implemented from main 5b82db1 and rebased onto e04c4a7 (preserving obsolete White Night dependency cleanup): imported 12 actor sheets and 6 Pixelfx sheets with source/runtime hashes, generated `space_ruler_assets.hpp`, connected ranged job hover/flight/fuel, casting and combat effects, numeric skill controls, renderer and Android/desktop asset packaging. Sources: native-engine/src/combat.hpp, presentation.hpp, main.cpp, new space_ruler_assets.hpp, native-engine/CMakeLists.txt, native-engine/android/app/build.gradle, scripts/import_space_ruler.py, Art/space-ruler, Assets/Resources/Eclipse/space_ruler. Details and owner acceptance steps: Docs/SPACE_RULER_INTEGRATION.md.

Static asset metadata/hash inspection and source diff inspection completed. No builds/tests/device checks/Actions run. Owner device acceptance remains outstanding; no runtime success claimed.


## 2026-11-13 — Space Ruler effect enlargement and movement follow-up

Status: PARTIAL — enlarged effects and grounded anchoring updated; a suitable PixelLab walk animation could not be produced in this pass.

### Completed
- Increased all six Pixelfx render scales in `native-engine/src/space_ruler_assets.hpp` and synchronized `Art/space-ruler/manifest.json` and `scripts/import_space_ruler.py`: void bolt 1.0x, gravity field 2.6x, dimensional spikes 1.8x, repulsion 1.75x, singularity 2.4x, flight trail 2.2x.
- Removed the idle bob while the actor is grounded; air-only bob remains. Existing role selection chooses flight rise/hold/descend only while not grounded. Up input and flight fuel behavior were inspected in `native-engine/src/combat.hpp`.
- Generated and visually reviewed PixelLab candidates. Image-animation jobs `ac12c1a6-40bb-4176-a219-ba867c4c7705` and `07b83921-6716-4d80-9bb2-64fe6e2506c8` had insufficient alternating leg poses and foot contacts. A character-template job `5d8f92a7-dde0-485b-9cdf-f09a8ba4fce8` had a more upright gait but changed the character identity and omitted the glowing book. None was integrated or mislabeled as a walk. Review-only outputs are in the workspace root as `space-ruler-walk-*.png`, `space-ruler-walk2-*.png`, and `space-ruler-pixellab-walk-*.png`.
- Re-read effective `.github/workflows/`: only `workflow_dispatch` triggers are configured. No workflow was run.

### Files changed
- `native-engine/src/space_ruler_assets.hpp`
- `native-engine/src/presentation.hpp`
- `Art/space-ruler/manifest.json`
- `scripts/import_space_ruler.py`
- `Docs/SPACE_RULER_INTEGRATION.md`
- `TASK_PROGRESS.md`

### Remaining / precise next actions
1. Generate from the exact game source character (retain the purple mage, book, robe silhouette) and achieve clearly alternating legs, visible foot lifts and planted contacts, and a closed walk loop; do not reuse rejected candidates as a final atlas.
2. Pack the approved frames into `Art/space-ruler/walk.png` and `Assets/Resources/Eclipse/space_ruler/walk.png`, update manifest/hash/geometry and map the grounded movement role in `native-engine/src/space_ruler_assets.hpp`; preserve `hover_move.png` in Git history.
3. Inspect all walk frames at native scale and playback timing. Owner runs game/device tests; no builds/tests were run in this pass.
4. Review oversized effect clipping and on-screen balance in owner-run gameplay.


## 2026-11-14 — Space Ruler original-based grounded walk

Status: IMPLEMENTED LOCALLY — original-reference PixelLab walk is integrated; owner gameplay review pending.

- Replaced the incorrect walk atlas that used another character template. PixelLab job `79a752b1-834f-4452-8438-767fac4c5661` generated eight east-facing 96x96 poses from the exact original `space_ruler_96_30colors.png` input. The animation keeps the purple mage, robe/hair silhouette, and glowing open book, with a clear alternating stride and planted/lifted foot phases.
- Packed each source frame at native 96x96 pixel scale, offset `(48,48)` in a 192x192 cell. Atlas is 1536x192, eight frames, pivot `(96,132)`, 0.72-second loop; identical atlas bytes are installed in Art and runtime Resources.
- Retained all source poses and the original reference image under `Art/space-ruler/walk-source/pixellab-skeleton-east/`. Updated manifest and importer metadata to remove the old wrong-character IDs and point to the original-reference PixelLab job. Updated integration documentation.
- Visually reviewed the native-size frames and loop/contact sheet. Source animation used 27 colors from the original 29-color palette; source alpha bounds remain stable around `(6,16)-(92,85)`, keeping the floor anchor consistent.
- No tests, builds, or gameplay checks were run. Owner should verify the grounded gait in-game, including west-facing flip and transition to flight.

## 2026-11-16 — Demon Castle no-railing backgrounds and foot baselines

Status: IMPLEMENTED IN SOURCE; runtime/device acceptance pending owner.

- Replaced runtime backgrounds at `Assets/Resources/Eclipse/demon_castle/areas/a1_ashen_approach.png` through `a5_night_throne.png` with the no-railing ImageGen revisions (2172×724 each).
- Preserved editable revisions as `Art/demon_castle/areas/a1_ashen_approach-no-railings-v2.png` through `a5_night_throne-no-railings-v2.png`; existing `*-source.png` assets remain unchanged.
- Updated `Art/demon_castle/asset-manifest.json` with v2 source/runtime records and SHA-256 values; documented integration in `Assets/Resources/Eclipse/demon_castle/README.md` and `Docs/CASTLE_FIXED_TERRAIN_STATUS.md`.
- Revised all A1–A5 floor/stair foot baselines in `native-engine/src/castle_terrain.hpp` to sit 3 world pixels below the visible surface edge, and aligned start spawns to the interpolated floor under their spawn X.
- Reviewed a static overlay of the terrain polylines on the new backgrounds. No builds, tests, GitHub Actions, or owner gameplay/device tests were run. Owner should check walking contact, stairs, jumps/landings and area transitions in-game.

## 2026-11-16 — Flash Executioner skill buttons use shared frame + individual icons

Status: IMPLEMENTED IN SOURCE; owner gameplay/device acceptance pending.

- Added a 64×64 circular metal skill-button frame and separate 32×32 pixel icons for Rush, Command/Release, Blink and Verdict under `Art/ui/` and `Assets/Resources/Eclipse/ui/`; the same deliverables are in `Download/ECLIPSE/UI/`.
- Updated the Flash Executioner HUD to draw the shared frame, its corresponding individual icon, slot number and cooldown-ready bar. Existing other-class skill-button renderers remain unchanged for their later migration to this standard.
- Added required texture loading with size/error reporting; updated CMake desktop packaging and Android Gradle asset inclusion.
- Recorded the frame-plus-icon rule in `Docs/DESIGN_GUIDELINE_1_PIXEL.md` and documented paths, dimensions and hashes in `Art/ui/skill-button-manifest.json`.
- Replaced the initial hand-authored button graphics with the ImageGen originals retained in `Art/ui/imagegen/`. Python was used only to threshold/crop transparent margins and nearest-neighbor export to the required pixel sizes; it was not used to draw the art. The manifest records this workflow and source hashes.
- Renamed the runtime and shared-storage individual icon files to `skill1.png` through `skill4.png`; updated the loader, CMake asset list, and manifest paths accordingly.
- Revised the four glyphs for clearer meaning using ImageGen: a forward sword rush, three launched lances, teleporting through a portal, and falling lances. Kept the existing frame and blue-silver style; retained each generated original under `Art/ui/imagegen/skillN-imagegen-v2.png` and replaced the 32×32 exports in Art, runtime Resources, and shared storage.
- Reworked those icons again after owner feedback and reread the approved combat spec: Skill 1 shows the specified forward sword-thrust charge; Skill 2 shows the non-sword hand transforming stored light orbs into lances toward a marked enemy; Skill 3 shows paired portals and teleport to a distant marked target; Verdict shows many falling lances striking marked targets. This separates the skills by their actual mechanics. Source originals are `skillN-imagegen-v3.png`; 32×32 Art/runtime/shared-storage files and manifest hashes are updated.
- Simplified again to the explicit keywords using the free PixelLab pixel-art workbench: Skill 1 = dash arrow + speed marks; Skill 2 = one embedded light lance; Skill 3 = portal ring + light flash; Skill 4 = cloud + three falling lances. Replaced all four 32×32 icons, preserved 64×64 PixelLab exports in `Art/ui/pixellab/`, updated hashes and shared-storage copies. No Python drawing; nearest-neighbor reduction only.
- Reviewed a contact sheet against the existing circular Flash Executioner attack button reference. No builds, tests, GitHub Actions, or owner gameplay/device tests were run.

## 2026-11-17 — Mobile combat HUD touch-layout revision

Status: IMPLEMENTED IN SOURCE; automated checks added; owner gameplay/device acceptance pending.

- Repositioned default touch controls into a separated lower-right action cluster on the 1280×720 logical viewport. The four skills use a 68px 2×2 cluster beginning at y=456; attack is the largest target (112px), dodge is 76px, swap is 68px, and the joystick is 126px. All targets remain inside the viewport and the default rectangles do not overlap.
- Added control-layout schema v5. Existing unmodified legacy defaults migrate to the new positions; layouts with any user customization retain their positions. Audio and accessibility settings remain preserved during migration.
- Added a core regression test for minimum touch-target size, safe bounds, non-overlapping default hitboxes, and the skill-cluster vertical boundary.
- Reused the existing nearest-filtered pixel PNG HUD kit: shared 64×64 metal frame, 32×32 skill glyphs, and the attack/dash/swap/joystick state sheets. No mechanics or touch action mappings changed.
- Files changed for this HUD pass: `native-engine/src/control_layout.hpp`, `native-engine/tests/core_tests.cpp`, and this record.
- Static inspection and PNG dimension checks completed. Tests, builds, gameplay playback, GitHub Actions, and device checks were not run; the owner performs final gameplay/device testing.


## 2026-11-17 — Dark-fantasy combat HUD art and integration

Status: IMPLEMENTED IN SOURCE; static PNG/code checks complete; owner gameplay/device acceptance pending.

- Replaced combat controls with PixelLab pixel-workbench PNG art: black-iron attack (idle/pressed), directional dodge, swap state sheet, semi-transparent joystick bezel, shared skill plate, distinct Space Ruler and Blade icon atlases, player health frame, and separate enemy/boss frames. Existing Flash Executioner individual skill glyphs remain in use.
- Preserved prior UI PNGs under `Art/ui/source-original/`; the former player HP frame was code-rendered and had no source PNG. Added an asset manifest with dimensions and hashes, plus `Art/ui/review-before-after/hud-assets-before-after.png` (asset component sheet; not a gameplay capture).
- Hooked the new PNGs into the existing render and asset loading paths with exact-dimension checks and nearest-neighbour filtering. Updated desktop CMake and Android Gradle package lists. Skill cooldown, pressed, unavailable, health damage-trail, and touch mappings remain driven by existing gameplay state.
- Iterated the joystick after static review: its center was initially opaque and could hide the scene; PixelLab flood-fill changed the center to alpha 0 while keeping the iron bezel opaque. All new PNGs are RGBA, have binary alpha, match their manifest dimensions, and their Art/runtime copies have identical bytes.
- `git diff --check` and static PNG metadata/hash checks passed. No build, automated tests, GitHub Actions, gameplay capture, or device testing were run, per project policy; owner must perform final in-game visibility and touch-response validation.


## 2026-11-17 — Circular ImageGen + PixelLab Workbench HUD revision

Status: new art generated, edited and wired in source; static PNG and packaging checks passed; owner gameplay/device review required.

- Replaced the manually authored square-button visual direction with cohesive circular black-iron controls and full circular skill buttons, following the owner's preference. ImageGen created one 8-frame control sheet and one 12-frame class-skill sheet; original high-resolution sources remain under `Art/ui/imagegen/`.
- Used PixelLab `pixelart_workbench` to split, repair stray pixels and darken only the pressed attack/dodge/swap cells. Source frames are 256px and use nearest filtering at the existing hitbox sizes. Joystick idle/active and all 12 ability icons share a consistent circular rim and material.
- Runtime atlases are `Assets/Resources/Eclipse/ui/circular_control_states.png` (768×768, 8 cells in a 3×3 sheet) and `circular_skill_buttons.png` (1024×768, 12 cells in a 4×3 sheet). Existing mechanics/cooldowns, class mapping, hitboxes and input layout are unchanged. Health-bar PNGs from the preceding revision remain.
- Updated rendering to select the new cells by existing input state and class, retaining cooldown bars, disabled tint, slot numbers and the health overlays. Added both atlases to CMake and Android asset packaging.
- Static checks passed for exact atlas dimensions/frame counts, binary transparent alpha, palette size, manifest hashes, Art/runtime byte equality, and CMake/Android asset references. Delivered the latest PNGs, ImageGen sources, PixelLab outputs, manifest and comparison sheet under `/storage/emulated/0/Download/ECLIPSE/UI/`, with the former loose skill PNGs preserved in `Skills/PreviousSet/`. No build, automated tests, GitHub Actions or device/gameplay tests were run; owner must validate touch states and button legibility in real combat.


## 2026-11-17 — Combat icon legibility revision

Status: intuitive icon revision implemented in source/runtime PNGs; static checks passed; owner gameplay/device validation required.

- Replaced abstract glyphs with conventional gameplay silhouettes: sword strike, lunging boot, two-character exchange, four-way joystick, sword thrust/sweep/shield/crossed swords, gravity pull-in, ground spikes, falling lances, running rush, orb-to-lance, portal passage, and lance rain.
- ImageGen produced the cohesive revisions; PixelLab Workbench split each atlas, repaired stray pixels, and retained the 8 control-state and 12 skill-cell sprite sheets. The previous high-resolution ImageGen source sheets and v1 assets remain preserved.
- `Art/ui/review-before-after/intuitive-icons-native-size-final.png` compares the actual in-game target dimensions for controls and all class skills.
- Existing renderer mapping, control hitboxes, class actions, cooldown behavior, and touch areas were not changed in this legibility-only pass.
- Static checks passed for atlas dimensions, 8+12 frame counts, binary alpha, hashes, Art/runtime equality, native-size review images and existing control/class index mapping. No build, tests, GitHub Actions, or device gameplay were run.

## Combat skill glyphs — verified against gameplay FX (2026-11-17)

- Rechecked the actual `stepPlayer`/`stepSpace`/`stepFlashMove` combat paths, the Space Ruler integration notes, the Flash Executioner approved spec, and the runtime FX sheets before revising glyph meaning.
- Replaced the misleading glyphs in `circular_skill_buttons.png`: Space Ruler cells now show persistent caster-centered gravity field, ground spikes, outward repulsion, and inward singularity collapse; Flash Executioner cells show Rush, orb-to-lance volley, blink to a marked target, and marked-target lance rain. Blade preparation now reads as an attack-speed boost rather than defense. Circular frame, 4x3 atlas order, crop rectangle, button state/cooldown rendering, touch bounds, and input mapping remain unchanged.
- Art process: ImageGen atlas; PixelLab palette reduction (12 frames together, 64 colors), PixelLab Workbench 4x3 split / join / repair / 2x nearest scale; alpha-only normalization for runtime output. Preserved the prior atlas. New before/after comparison uses the actual 224px runtime crop rendered at 64px, not a game capture.
- Static validation: runtime PNG is RGBA 1024x768, 12 equal 256x256 cells, alpha values exactly 0 or 255; manifest hash and Art/runtime bytes match; Android Gradle and desktop CMake still load/package the same path. Latest PNG, ImageGen source, Workbench output, comparison sheet, manifest, and prior atlas are organized in `Download/ECLIPSE/UI/`.
- No build, owner tests, gameplay/device check, or GitHub Actions run. Native combat readability and touch-state legibility still need owner device acceptance.

## 2026-11-22 — Flash Executioner mid-frame and light-crescent slash update

Status: ASSETS AND RUNTIME MAPPINGS UPDATED IN SOURCE; static checks passed; owner device verification pending. Nothing was built, tested, or run on GitHub Actions.

- Basic actor sheet: 28 -> 35 frames (7 AI mid-frames inserted after original F6); `contactFrame` 6 -> 13; gameplay duration 0.46s and contact 0.184s unchanged. Blade of 0-based frame 8 redrawn in the PixelLab pixel workbench.
- Skill1 actor sheet: 30 -> 33 frames (3 mid-frames after F6); `contactFrame` 6 -> 9. The generated white burst/arrowhead shapes on the 2nd and 3rd mid-frames were removed and replaced with partial original F7 blade pixels, and floating tick fragments were deleted, so no effect is baked into the sheet.
- Dash actor sheet: 9 -> 13 frames (4 mid-frames after F1); `contactFrame` stays -1.
- Skill2, Skill3, Swap and Land intentionally unchanged (Skill3 blink jumps are intended and use blank frames).
- Slash effect replaced by custom `melee/blade-light-crescent` (33 frames, 5x7 grid of 320x240, fps 30, duration 1.1s, pivot 120,140, scale 1.4, offset 6,-13, triggerLeadSeconds 0.159). Remixed from PixelFX `melee/sword-slash`, `melee/scythe-reap` and `holy/divine-shield`, so it is not an unmodified upstream resource. Owner explicitly approved this deviation from the earlier rule (2026-11-22).
- `native-engine/src/presentation.hpp`: for `FlashMove::Slash`, `playbackDuration = fxSheet.duration / speed` (no longer stretched to the attack clip's remaining time).
- 17 PixelLab generations used in total, 11 of them in shipped assets (Basic 5, Dash 3, Skill1 3). Backups of previous sheets, approval JSON and header are in `Art/flash-executioner/pre-midframe-originals/`.

Files modified:
- `Art/flash-executioner/reviewed/actor_basic.png`, `actor_skill1.png`, `actor_dash.png`, `effect_slash.png`
- `Assets/Resources/Eclipse/flash_executioner/actor_basic.png`, `actor_skill1.png`, `actor_dash.png`, `effect_slash.png` (matching runtime copies)
- `Art/flash-executioner/review-approval.json`
- `Art/flash-executioner/reviewed/manifest.json`
- `Art/flash-executioner/pixelfx/blade-light-crescent.js`, `Art/flash-executioner/pixelfx/blade-light-crescent-sheet.json`
- `native-engine/src/flash_executioner_assets.hpp` (regenerated by `Tools/art/import_flash_executioner.py`; only Basic, Skill1, Dash and Slash lines changed)
- `native-engine/src/presentation.hpp`
- `Art/flash-executioner/README.md` (new section appended)

Checks done (static only):
- `import_flash_executioner.py` accepted all 27 sheets.
- Read-only validation script: every runtime flash sheet matches its reviewed PNG pixel for pixel, dimensions match cell grid, count fits the grid, and `contactFrame` is valid. Fully transparent cells were reported only where the README documents them as fades, release or blank frames.

Remaining work:
- Owner device/game test: mid-frame timing and continuity, Slash playback duration, crescent effect legibility, both facing directions, and performance.
- Skill2, Skill3, Swap and Land untouched.
- The custom crescent effect needs the owner's policy exception recorded in the project policy docs (the README records it; AGENTS.md still states the PixelFX-only rule).
- Not committed or pushed.

NOT RUN: builds, automated or core tests, headless tests, APK packaging, GitHub Actions, and device or gameplay checks. Owner verification is still required.

## 2026-11-22 - Mid-air burst effects

Status: custom mid-air effects in source and runtime; static checks passed; owner device verification pending. Nothing was built, tested, or run on GitHub Actions.

- Impact (`effect_impact.png`): replaced `impacts/crit-hit` (upward spark kick, bouncing gravity shards) with custom `impacts/air-hit` (derived from `impacts/crit-hit`): 18 frames, 5x4 grid of 240x240 cells, pivot 120,120, scale 1.0, fps 30, 0.6 s. Burst of light in the gold-white palette: soft white bloom flash, one luminous ring, lens cross, wide soft rays, glints and light motes in every direction; no spark streaks, no gravity, no floor.
- LanceHit (`effect_lancehit.png`): replaced `holy/smite` (floor shockwave, stone chips, cracks, dust) with custom `holy/air-burst` (derived from `holy/smite`): 27 frames, 6x5 grid of 320x320 cells, pivot 160,160, scale 0.9, fps 30, 0.9 s. Same light-burst language, bigger: bloom flash, two luminous rings, rays, glints and motes in every direction. Anchored at the impact centre (targetFeet 0) instead of the target's feet.
- Slash (`effect_slash.png`): sparks and light motes now scatter in all directions with drag and no gravity or floor (previously gravity-based `Particles`).
- 2026-11-22 revision: rebuilt as soft light bursts (no spark streaks) and reduced in size at the owner's request; scales are now Impact 1.0 and LanceHit 0.9 (were 1.7 and 1.5 for the earlier versions).
- 2026-11-22 scale update: the basic attack Impact is now scale 0.7 and LanceHit is scale 1.0. Command and Rush hits use the new ImpactLance and ImpactRush sheets (see "Hit effect variants").
- All three are custom effects, not unmodified PixelFX resources. This is the owner's explicit deviation (2026-11-22), recorded in `review-approval.json`.

Files modified:
- `Art/flash-executioner/reviewed/effect_impact.png`, `effect_lancehit.png`, `effect_slash.png`
- `Assets/Resources/Eclipse/flash_executioner/effect_impact.png`, `effect_lancehit.png`, `effect_slash.png` (matching runtime copies)
- `Art/flash-executioner/review-approval.json`
- `Art/flash-executioner/reviewed/manifest.json`
- `Art/flash-executioner/pixelfx/air-hit.js`, `air-hit-sheet.json`, `air-burst.js`, `air-burst-sheet.json`, `blade-light-crescent.js`, `blade-light-crescent-sheet.json`
- `native-engine/src/flash_executioner_assets.hpp` (Impact and LanceHit lines)

Checks done (static only):
- `Tools/art/import_flash_executioner.py` re-run; accepted all 27 sheets.
- Read-only validation script: 27 sheets checked, 0 errors. Runtime PNGs match `reviewed/` pixel for pixel, cell grids and frame counts fit, and `contactFrame` values are valid. Fully transparent cells (fades and tails) were listed, not counted as errors, including Impact 22-24, LanceHit 28-29 and Slash 29-32.

Remaining work:
- Owner device check of the Impact, LanceHit and Slash bursts: timing, legibility, occlusion, both facing directions, performance.
- Rush, Command, Blink, Swap, Dash, Orb, Lance and SkyLance floor assumptions not reviewed. Jump and Land are ground effects by design and were left unchanged.
- Not committed or pushed.

NOT RUN: builds, tests, headless tests, APK packaging, GitHub Actions, and device or gameplay checks.

## 2026-11-22 - Hit effect variants

Status: fixed per-action hit effects in source and runtime; the earlier rotating variant set was replaced by this design. Static checks only; owner device verification pending. Nothing was built, tested, or run on GitHub Actions.

- Design: the hit effect is fixed per action. The same action always shows the same effect, different actions show different effects, and stronger actions look stronger (bigger, heavier). All are custom mid-air bursts of light in the gold-white palette, with no floor, no gravity and no spark streaks, derived from `impacts/crit-hit` and `holy/smite`. Not unmodified PixelFX resources, as recorded in `review-approval.json`.
- Impact (`effect_impact.png`, basic attack, weakest tier): `impacts/air-hit`. 18 frames, 5x4 grid of 240x240 cells, pivot 120,120, scale 0.7, 0.6 s.
- ImpactLance (`effect_impactlance.png`, Command skill stored light lances): `impacts/air-hit-cross`, two crossing blades of light. 18 frames, 5x4 grid of 240x240 cells, scale 0.9, 0.6 s.
- ImpactRush (`effect_impactrush.png`, Rush skill): `impacts/air-hit-rush`, heavy long eight-point star, big bloom, two shock rings, ring of glints. 21 frames, 5x5 grid of 280x280 cells, pivot 140,140, scale 1.2, 0.7 s.
- LanceHit (`effect_lancehit.png`, Verdict lance, strongest tier): art unchanged, scale raised 0.9 -> 1.0; 20 lances each burst, plus the Verdict pillar.
- `native-engine/src/combat.hpp`: `enum LightFx` gained `SlashHit` and `RushHit` at the end. Basic attack publishes `SlashHit` for each enemy hit when `player.flashExecutioner` is set (previously no hit burst at all). Rush hit publishes `RushHit` instead of `Impact`. Lance landings still publish `Impact`.
- `native-engine/src/presentation.hpp`: Impact -> `ImpactLance`, SlashHit -> `Impact`, RushHit -> `ImpactRush`, Verdict -> `LanceHit`. Plain Impact is still suppressed when a verdict pair exists. The legacy "replaced" check was extended for the new kinds so the old procedural burst is not drawn on top.
- `Tools/art/import_flash_executioner.py`: EFFECTS list and FlashFx header template end with `..., Land, ImpactLance, ImpactRush`. The obsolete variant PNGs were removed from Art and runtime by the tool.

Files modified:
- `Art/flash-executioner/reviewed/effect_impactlance.png`, `effect_impactrush.png`
- `Assets/Resources/Eclipse/flash_executioner/effect_impactlance.png`, `effect_impactrush.png` (matching runtime copies)
- `Art/flash-executioner/pixelfx/air-hit.js`, `air-hit-cross.js`, `air-hit-rush.js`, `air-burst.js` (+ `*-sheet.json`); petal and star sources removed
- `Art/flash-executioner/review-approval.json`
- `Tools/art/import_flash_executioner.py`
- `native-engine/src/flash_executioner_assets.hpp` (regenerated; 29 sheets accepted)
- `native-engine/src/combat.hpp`, `native-engine/src/presentation.hpp`
- `Art/flash-executioner/README.md`

Checks done (static only):
- `Tools/art/import_flash_executioner.py` accepted 29 sheets.
- CMake and Android Gradle package `flash_executioner/*.png` by glob, so no build-file change was needed.

Remaining work:
- Owner device check: hit variety per action, timing, legibility, occlusion, both facing directions, performance.
- The C++ edits in `combat.hpp` and `presentation.hpp` are not compiled.
- Command lances and Verdict lances share the Impact burst kind (the Verdict case is handled by the pair rule).
- Blink arrival and Dash, Swap, Jump and Land effects were not reviewed for this tiering.
- Not committed or pushed.

NOT RUN: builds, tests, headless tests, APK packaging, GitHub Actions, and device or gameplay checks. Owner verification is still required.

## 2026-11-22 - Effect readability

Status: the four hit effects were made less intrusive; the Slash crescent was not changed. Static checks and measurements only; owner device verification pending. Nothing was built, tested, or run on GitHub Actions.

- Hit effects (tails cut and scales lowered; these four only):
  - Impact (`effect_impact.png`): 18 -> 16 frames, 5x4 grid, 0.53 s; scale unchanged at 0.7.
  - ImpactLance (`effect_impactlance.png`): 18 -> 12 frames, 5x3 grid, 0.40 s; scale 0.9 -> 0.8.
  - ImpactRush (`effect_impactrush.png`): 21 -> 16 frames, 5x4 grid, 0.53 s; scale 1.2 -> 0.8.
  - LanceHit (`effect_lancehit.png`): 27 -> 23 frames, 6x4 grid, 0.77 s; scale 1.0 -> 0.85.
- Slash (`effect_slash.png`) unchanged: 33 frames, 5x7 grid of 320x240 cells, 1.1 s, scale 1.4, drawn in front of the fighter. A shortened, rear-layer version was tried and reverted at the owner's request.
- Measurements with `occlusion_metric.py` (before -> after): Basic x3 player silhouette peak 28% (unchanged; effect on screen 1.95 s of 3.55 s); Rush player peak 77% -> 58%, screen peak 0.9% -> 0.5%; Command player peak 10% (unchanged), screen peak 1.0%; Verdict player 0% (unchanged), screen peak 3.2% -> 2.9%. Seconds with any effect on screen did not change for Rush, Command and Verdict (dominated by non-hit effects).

Files modified:
- `Art/flash-executioner/reviewed/effect_impact.png`, `effect_impactlance.png`, `effect_impactrush.png`, `effect_lancehit.png`
- `Assets/Resources/Eclipse/flash_executioner/effect_impact.png`, `effect_impactlance.png`, `effect_impactrush.png`, `effect_lancehit.png` (matching runtime copies)
- `Art/flash-executioner/review-approval.json`
- `Art/flash-executioner/reviewed/manifest.json`
- `native-engine/src/flash_executioner_assets.hpp` (regenerated; no engine C++ change in this step)
- `Art/ui/review-before-after/motion-review/occlusion_metric.py` (measuring tool)
- `Art/flash-executioner/README.md` (values updated; "Readability check (2026-11-22)" subsection rewritten)

Checks done (static and measurement only):
- `Tools/art/import_flash_executioner.py` accepted all 29 sheets.
- Occlusion measurements listed above (take renders with and without effects).

Remaining work:
- Owner device check: hit legibility at real speed, both facing directions, performance.
- Rush hit still covers up to 58% of the player for about 0.1 s at contact, because the fighter rushes through the target.
- Not committed or pushed.

NOT RUN: builds, tests, headless tests, APK packaging, GitHub Actions, and device or gameplay checks. Owner verification is still required.

## 2026-11-23 — Tutorial / prologue (9 stages) + Demon King implementation

Status: code written for all 9 stages and the boss; boss art generated and packed. NOT built, NOT device-tested, NOT committed or pushed.

Files added:
- `native-engine/src/story_director.hpp` (stage state machine, practice prompts, waves, king flow, death sequence, forest, `story.cfg` persistence)
- `native-engine/tools/assemble_castle_boss.py` (frame selection and alignment only)
- `Art/demon-king/` (concepts, Pixellab exports, per-frame specs `p1_armored_frames.json` / `p2_true_frames.json`)
- `Assets/Resources/Eclipse/monsters/demon_castle/마왕_갑주/` and `마왕_본체/` (move 2048x256, attack 3584x256, empty vfx 5824x256)
- `Docs/DEMON_KING_ASSETS.md`

Files modified (all also carry the owner's earlier uncommitted edits, which were left alone): `native-engine/src/combat.hpp`, `castle_enemy.hpp`, `presentation.hpp`, `main.cpp`, `native-engine/tests/core_tests.cpp`, `Docs/TUTORIAL_PROLOGUE_FLOW.md`.

Checks done: core tests compiled and run locally (SDL-free); 9 new story/king tests pass. 32 older tests fail (29 identical at git HEAD, 3 only against the working tree). `main.cpp`/`presentation.hpp` were proofread only: SDL3 is not installed here, so compile errors are possible.

Remaining work:
- Compile and play-test on desktop/device (owner). Check that the king scale 2.0 / 1.8 does not look mixed against other enemies.
- Review the P1 strike snap frame, recovery in-betweens, and the missing mid-swing frame (see DEMON_KING_ASSETS.md).
- Not implemented: separate practice background, Flash Blink/Verdict training dummy, replay-practice control, audio, forest layout beyond existing art, skill 2-4 recovery, all number tuning.
- Pixellab budget: 110 of 115 approved generations spent.

NOT RUN: desktop/Android builds, GitHub Actions, device or gameplay checks.

### Addendum 2026-11-23 — Demon King move set
Added a dedicated boss move engine: phase 1 has six telegraphed one-shot moves (smash, quake waves, leap slam, judgement pillars, iron sweep, juggernaut rush); phase 2 has six chained combos plus an enraged finisher (dash combo, blink strike, pillar chain, slash burst, wave volley, void rain, execution combo). Files: `native-engine/src/castle_enemy.hpp` (move data + picker), `combat.hpp` (`bossAi`, hazards), `monster_collision.hpp` (`animFrom` frame mapping), `presentation.hpp` (telegraphs, pillars, waves, blink flash, shake), `native-engine/tests/core_tests.cpp` (8 new tests, all passing locally). Core tests now 52/84 (the same 32 older failures as before). NOT run: builds, device play, GitHub Actions. All timings/damage numbers are untuned placeholders; the rush and leap reuse the existing attack frames.

### Addendum 2026-11-23 — floating-feet fix
`assemble_castle_boss.py` anchored frames on the lowest opaque pixel, so frames with a sword tip below the feet floated. Now anchors on the registered foot row (key frames share a canvas; in-betweens are registered by cross-correlation). Both boss sheets (`마왕_갑주`, `마왕_본체`) were rebuilt and checked frame by frame against a baseline. Preview video v2 (`Docs`-adjacent: `~/storage/downloads/ECLIPSE/TUTORIAL_WORK/boss_move_preview/demon_king_moves_v2.mp4`) uses the fixed sheets. NOT run: builds, device play, GitHub Actions.

### Addendum 2026-11-23 — per-move motions + pixelfx layers
Each Demon King move now plays its own body motion: 11 new sheets (`모션_*_sheet.png`: P1 stab/leap/raise/sweep/charge, P2 twist/rise/burst/sky/cast/backflip/dive) plus a new P2 combo (aerial dive). 17 pixelfx effect sheets live in `Assets/Resources/Eclipse/boss_fx/` and are scheduled by `Combat::bossFx` / `BossHazard::fx`; `Presentation` loads and draws them (not compiled here). Generated headers: `boss_fx_assets.hpp`, `boss_motion_data.hpp`. New tools: `export_boss_assets.py`, `boss_trace.cpp`. CMake and Android packaging include `boss_fx/`. Local core tests: 56/88 (32 older failures unchanged). Pixellab: 152 of 185 spent. NOT run: desktop/Android builds (SDL3 missing here), device play, GitHub Actions. Remaining: owner review of every motion, compile check of `presentation.hpp`/`main.cpp`, tuning, real-game footage.

### Addendum 2026-11-23 — in-between round 2 (20-frame motion sheets)
Owner feedback: too few in-between frames. Audit found many held frames and 45-92 % pose jumps. Generated 30 more in-between sets (Pixellab spent 182 of 185), motion sheets now 20 frames (legacy attack sheet stays 14), all 12 motion sheets, tip tables and engine constants (`bossMotionFrames`, `bossSheetFrames`, `enemyFrame(...,frames)`) rebuilt; step timings re-derived from each motion's impact frame. Local core tests 56/88 (the same 32 older failures). NOT run: desktop/Android builds, device play, GitHub Actions. Still open: owner review of every motion; leap/dive fall (90 % jump), short motions (raise, cast, burst) with few distinct poses.

### Addendum 2026-11-23 — effect stacks (round 3)
Owner feedback: effects too thin next to the richer motions. Each Demon King move now stacks 5-14 pixelfx layers (28 sheets in `Assets/Resources/Eclipse/boss_fx/`, 17 MB): wind-up energy/runes/sigils, strike layers, shock rings/blasts, dust, a short screen-wide impact frame and a flash, plus a code-drawn sword-tip ribbon from the per-frame tip table. `Presentation` draws floor/behind layers before the sprite and strikes after (not compiled here). New test: every move stacks several different layers. Local core tests 57/89 (the same 32 older failures). NOT run: desktop/Android builds, device play, GitHub Actions. Open: tune scale/alpha of layers, check APK size impact of boss_fx (17 MB).


## Addendum: ruined black castle backgrounds (A1-A5)
- Replaced all five runtime backgrounds (Assets/Resources/Eclipse/demon_castle/areas/a1..a5, 2172x724) with new Pixellab pixen art (724x240, 3x nearest upscale), black/grey ruined-castle theme, no purple.
- Floors composited along castle_terrain.hpp polylines: A3 raised ledges + pit (x 1150-1220), A4 stone staircase, A5 shifted up 58 px. Terrain code unchanged.
- Sources: Art/demon_castle/areas/*-ruined-black-source.png. asset-manifest.json hashes NOT yet updated (old entries describe the purple set).
- Generations used: 5 (about 187 of the approved 215). Not run: builds/tests (owner). Open: boss/effect palette is violet and should be re-checked against the new look.

- REVISION: first set (perspective floors, bright windows) rejected as unfit for a side-scroller. Regenerated with flat orthogonal side-view prompts (low contrast, one horizontal floor strip); A3 ledges/pit, A4 slope and A5 floor slab are drawn from the terrain polylines. A1 desaturated. Sources overwritten at Art/demon_castle/areas/*-ruined-black-source.png. 10 generations spent in total on this set (about 192 of 215). Builds/tests not run.
- A1 replaced by a create_image_pro result (human-scale gate, visible floor band, blue-black moonlit style); source Art/demon_castle/areas/a1_ashen_approach-ruined-pro-source.png. Pixellab: 75 of the 100 approved generations spent on pro (3 x 25). A2-A5 still the lower-quality pixen set; regenerating them in the A1 style costs 100 more and needs approval.

### Addendum 2026-11-24 — tutorial flow review fixes + balance
Static review of `story_director.hpp` against `combat.hpp`/`main.cpp` (all hooks exist; call order correct). Fixed: (1) R during the death stage no longer clears `defeated`/revives the player (`restartStage` returns early in `Death`); (2) R in the forest now re-applies the weakened forest foes and the 45 HP start (new `Combat::weakenForestFoes()`, `enterForest()` uses it); (3) `finishPractice` returns the player to the A1 spawn so the threshold pause/trigger cannot be skipped by running ahead. Balance: new `tutorialPower()` in `story_director.hpp` + `Combat::spawn(..., powerMul)` eases only the gatekeeper knight (x0.65, ~218 -> ~142 HP at A3, about 13 -> 8 basic hits; every other kind unchanged; default powerMul=1 keeps all other callers identical). The figures come from static arithmetic (avg basic combo ~17 dmg), not playtesting. Files: `native-engine/src/combat.hpp`, `native-engine/src/story_director.hpp`, `native-engine/tests/core_tests.cpp` (4 new tests, written not run). NOT run: builds, core tests, device play, GitHub Actions. Left open by owner decision: death blackout pops from 1.0 to 0.35 at 4.4 s; A1 wave spawns x 1880-2120 sit beyond the exit portal (x 1810).

### Addendum 2026-11-24 — class coefficient balance (Space Ruler)
Static single-target DPS from `combat.hpp` constants: Flash Executioner ~80 (basic 15/0.46s with +8 %/stored lance, rush 18/3s, command up to 5x10, verdict 20x8/9s), blade ~56 (12/15/23 combo + thrust/sweep), Space Ruler 34 (bolt 11/0.55s, field 6x6/7s, spikes 24/7s, repulsion 12/5s, ultimate 65/20s). Raised only the values the design leaves open: bolt 11->15, spikes 24->36, singularity 65->105 (constants `spaceBoltDamage`, `spaceSpikesDamage`, `spaceSingularityDamage` in `combat.hpp`); everything the design fixes (field 6/tick, repulsion 12, cooldowns, radii, timings) is unchanged. Result: Space Ruler ~45 single-target, with its range/area advantage kept; one singularity one-shots every tutorial soldier. Flash Executioner and blade untouched. Gap found: the gravity field's documented 30 % slow is not implemented. Files: `native-engine/src/combat.hpp`, `native-engine/tests/core_tests.cpp` (1 new test), `Docs/SPACE_RULER_INTEGRATION.md`. Estimates only: no playtest. NOT run: builds, core tests, device play, GitHub Actions.

### Addendum 2026-11-24 — stat progression (per-job stats, shard drops, save)
Owner request: stats that can be upgraded. Choices: 7 stats (Attack, Vitality, Guard + Precision, Ferocity, Swiftness, Haste), levelled per job group, enemy shard drops, core logic + save + minimal inventory screen. New `native-engine/src/progression.hpp`; hooks in `combat.hpp` (`damageEnemy`, `damagePlayer`, cooldown recovery, `Character::maxHealth/movementMultiplier`, swap, `restart`), `story_director.hpp` (drops off in the prologue, on in the forest), `main.cpp` (load/save `progress.cfg`, inventory keys 4 5 6 7 8 9 0), `presentation.hpp` (STATS panel), `tests/core_tests.cpp` (7 new tests), `Docs/STAT_PROGRESSION.md`, `Docs/CHARACTER_SKILL_INDEX.md`. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only: no binary, no test run). NOT verified: `main.cpp` and `presentation.hpp` (SDL3 missing here, edits re-read only), any build, core test run, playtest, device, GitHub Actions. Open: touch upgrade controls, pickup/upgrade cues, cost/drop tuning, reincarnation reset policy, gravity-field slow.

### Addendum 2026-11-24 — memory concept + altar at the start
Owner direction: upgrades belong at the starting point, and the concept is growing by eating memories. Renamed shards -> memories (`Progression::memories/addMemories/memoryDrop`, save header `memories`, old `shards` header still loads). Upgrading is now only possible at the memory altar at the forest start (`memoryAltarX=140`): interact (attack button turns into the "!" prompt, or key E) opens a world-freezing menu (keys 1-7 or taps; X/outside closes); the inventory panel is equipment-only again. Fallen enemies send a memory mote into the protagonist; eating pulses a ring (all code-drawn placeholder visuals, no new art). Files: `progression.hpp`, `combat.hpp` (`memoryMenu`, `nearMemoryAltar()`, `memoryMotes`, `memoryFlash`, step/restart hooks), `main.cpp`, `presentation.hpp`, `tests/core_tests.cpp` (+5 tests, renamed old ones), `Docs/STAT_PROGRESSION.md`, `Docs/CHARACTER_SKILL_INDEX.md`. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only; no binary, no test run). NOT verified: `main.cpp`/`presentation.hpp` compile (SDL3 missing here, re-read only), builds, tests, playtest, device, GitHub Actions. Open: authored altar/mote art, eating sound, reincarnation policy for levels.

### Addendum 2026-11-24 — tutorial flow: infinite regression + memory upgrade tutorial
Owner flow: dying to the boss starts an infinite regression loop; in this regression some memory of earlier ones remains; that memory drives the upgrade tutorial; finding memories makes the protagonist gradually stronger, which ends the tutorial. Implemented in `story_director.hpp`: stage 8 plays 5 regression loops (faster each time, then one slow loop; tally marks, snap-back flash, a surviving memory glint); stage 9 adds the memory tutorial (gift of the first memory, altar glow, interact prompt, attack-row pulse in the altar menu, auto-close, glint on the next foe; ends when a memory was eaten and a foe beaten; `story.cfg` v2 stores `memory`, v1 counts as already played). `presentation.hpp` draws tally pips, the glint, altar glow, foe hint and menu-row pulse (code-drawn placeholders). Tests: death timeline updated, +3 tests. Files: `story_director.hpp`, `presentation.hpp`, `tests/core_tests.cpp`, `Docs/TUTORIAL_PROLOGUE_FLOW.md`, `Docs/STAT_PROGRESSION.md`. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only; no binary, no test run). NOT verified: `presentation.hpp`/`main.cpp` compile (SDL3 missing), builds, tests, playtest, device, GitHub Actions. Open: loop timings, authored art/sound for the loop and the memory, replay of the tutorial, what the regression shows beyond rewinding the same rooms.

### Addendum 2026-11-24 — permanent vs run upgrades
Owner direction: split permanent upgrades (more general-purpose) from run upgrades (more specific); run upgrades reset each run. Permanent layer unchanged (7 broad stats, memories, saved). New run layer: `RunPerk`/`RunState` in `progression.hpp` (6 niche perks, max level 5, cost 3/5/7/9/11 echoes; Gravity Reach = Space Ruler only, Lance Surge = Flash Executioner only), echoes dropped per kill, lost on death (`Combat::runLost` -> `beginRun()` in `restart()`), fresh on a new game and on the regression into the forest. Altar menu got two tabs (FOREVER / THIS RUN; left/right or tap; keys 1-7 via `Combat::menuChoose`). Files: `progression.hpp`, `combat.hpp`, `story_director.hpp`, `main.cpp`, `presentation.hpp`, `tests/core_tests.cpp` (+5 tests), `Docs/STAT_PROGRESSION.md`. Assumptions I made (not confirmed by the owner): what a run is (until death), echoes as the run currency, the six perks. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only). NOT verified: `main.cpp`/`presentation.hpp` compile (SDL3 missing), builds, test run, playtest, device, GitHub Actions. Open: buying run perks in the field, perk numbers, art/sound.

### Addendum 2026-11-24 — run upgrades are now a skill tree per job (job mastery)
Owner direction: run upgrades as a skill tree per job; mastery is earned by playing the job; spend it on the tree; death resets everything. Replaced the six shared run perks (`RunPerk`/`RunState`) with `MasteryState`/`MasteryBlock` in `progression.hpp`: 9 nodes per job (3 branches x 3 tiers, cost 1/2/3 points, tier needs the tier above), xp 1 per kill (boss 5) to the job in use, point k at k^2+2k xp. Effects wired into `combat.hpp` through `Combat::perks` (rebuilt by `refreshStats`), reset by `beginRun()` on death or a new game. Altar THIS RUN tab now draws the active job's tree (keys 1-9 or tap on a node; `memoryMenuHit(x,y,tab)`), header shows mastery points. Files: `progression.hpp`, `combat.hpp`, `main.cpp`, `presentation.hpp`, `story_director.hpp` (unchanged calls), `tests/core_tests.cpp` (5 tests replaced), `Docs/STAT_PROGRESSION.md`. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only). NOT verified: `main.cpp`/`presentation.hpp` compile (SDL3 missing), builds, test run, playtest, device, GitHub Actions. Assumptions mine, not owner-confirmed: the 27 node effects and numbers, xp per kill, tree shape.

### Addendum 2026-11-24 — trees mix signature and raw job-stat nodes
Owner direction: each job tree should mix job-signature nodes (e.g. more stored light lances) with the job's own flat stat boosts. Reworked the 27 nodes: 6 signature + 3 raw per job (Space Ruler raw: crit +6 %, +30 HP, move +10 %; Flash raw: damage +10 %, haste +10 %, move +12 %; Blade raw: damage +10 %, +40 HP, +2 defense). Raw nodes live in new `MasteryBlock::raw*` fields and are merged into `Combat::stats` in `refreshStats()` (so they stack with permanent stats, only for the active job, and reset with the run). Dropped nodes and their dead hooks: SWIFT CAST, CRUSH TICK, COOL ORBIT, DAWN EDGE slash multiplier, CHAIN MARK, SHORT CUT, WHIRL, RECOVERY. Menu nodes now show a one-line value and a corner marker (square = raw, diamond = signature). Files: `progression.hpp`, `combat.hpp`, `presentation.hpp`, `tests/core_tests.cpp` (blade test rewritten, +1 raw-stat test), `Docs/STAT_PROGRESSION.md`. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only). NOT verified: `presentation.hpp` compile (SDL3 missing), builds, test run, playtest, device, GitHub Actions. Node numbers remain static estimates.

### Addendum 2026-11-24 — the tree is now a real pyramid
Owner direction: tier 2 opens only after two tier-1 nodes, like a real tree, not necessarily always two. Replaced the 3 branches x 3 tiers chain with a data-driven pyramid (`masteryShape`: tier, parents, need) of 10 nodes per job: 4 / 3 / 2 / 1 per tier, tier 2 needs both of its two parents, tier 3 any two tier-2 nodes, tier 4 both tier-3 nodes; costs 1/2/3/5 (21 total). Still 6 signature + 4 raw nodes per job (raw nodes are additive: Space Ruler crit/move/health/haste, Flash damage/move/haste/health, Blade damage/defense x2/health). UI: a pyramid with parent links, buyable nodes pulse, locked nodes show the number of missing parents, keys 1-9 and 0 (0 = top node), `memoryTreeNode()` gives each node's rectangle for drawing and touch. Files: `progression.hpp`, `presentation.hpp`, `main.cpp`, `tests/core_tests.cpp` (tree rules, blocks, blade and raw-stat tests rewritten), `Docs/STAT_PROGRESSION.md`. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only). NOT verified: `presentation.hpp`/`main.cpp` compile (SDL3 missing here), builds, test run, playtest, device, GitHub Actions. Layout sizes (node 112x70) are untested on a real screen.

### Addendum 2026-11-24 — scalable skill trees + special effects
Owner: the trees will grow very complex and should get special effects. Reworked the mastery trees into data: `MasteryNodeDef` (free grid position, parents + need, cost, up to 4 `Mod` modifiers, up to 2 `ProcDef` special effects, name/tag/description), `masteryTree(job)` lists (13 nodes per job: the 10-node pyramid plus 3 special-effect nodes), `MasteryBlock::apply(Mod)` as the single place that knows what a modifier does, up to `maxMasteryNodes` (64). Special-effect engine in `combat.hpp`: triggers OnKill/OnCrit/OnDash/OnDamaged/OnSkill, actions Explode/Pulse/Bleed/AddLance/Heal/RefundCooldown, with chance, cooldown, every-Nth and a no-chaining guard; visual cues in `procCues` (code-drawn placeholder rings). New `memory_menu.hpp` (pure C++): layout, drag-to-scroll, tap-to-select, second tap/BUY/Enter to buy; `presentation.hpp` draws the scrolling tree with links and a detail box; `main.cpp` forwards pointers (releases still reach the input layer). Guide for growing the trees: `Docs/SKILL_TREE_GUIDE.md`. Tests: tree rules, data integrity (lengths, cycles, duplicate spots), proc engine, effect nodes via real triggers, controller; older mastery tests adapted. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only). NOT verified: `presentation.hpp`/`main.cpp` compile (SDL3 missing), builds, test run, playtest, device, GitHub Actions. Open: real effect art/sound for procs, node icons, tuning every number, keyboard navigation inside the tree (mouse/touch/Enter only).

### Addendum 2026-11-25 — local build with ECLIPSE Studio
Owner asked for a build with ECLIPSE Studio. Ran `bash ~/eclipse-studio/studio-play.sh` (Termux Clang, no GitHub Actions): it compiles the `~/eclipse` working tree (uncommitted changes included) into `~/eclipse-studio/studio/local-build/libeclipse_game.so` and copies it to `~/storage/downloads/libeclipse_game.so`. The first attempt FAILED: `native-engine/src/main.cpp` had a duplicated 91-line block (an earlier edit of mine matched the title-screen ESC handler instead of the in-game one), giving "'continue' not in loop" and an extra brace; removed the duplicate and the obsolete inventory key handler. Second attempt succeeded: AArch64 ELF, SDL_main exported, no forbidden NEEDED libs. This is also the first real compile of `main.cpp`/`presentation.hpp`/`memory_menu.hpp` with SDL3 headers. NOT done: importing into Studio, running on the tablet, the core test run, GitHub Actions, the Game APK. Android 16 loader behaviour is still unverified.

### Addendum 2026-11-25 — art/resources for the Studio build
The `.so` build does not carry art. Ran `check-game-asset-references.py` (OK) and `pack-game-assets.py`: new `~/eclipse-studio/studio/local-build/eclipse-assets.zip` (262 PNG/JSON, 42 MB, sha256 de705b9f...) replacing the Nov 14 one (kept as `eclipse-assets.before-nov25.zip`; it was missing 67 files, among them every Demon King sheet, and 19 had changed: castle backgrounds, Flash Executioner sheets/effects). Copied to `~/storage/downloads/eclipse-assets.zip`. Nothing was sent to the Studio app and the game was not launched. No new art was added for the stat/tree/altar UI (it is code-drawn). The Space Ruler v2 motion/effect sheets made earlier are NOT part of the game resources (not integrated, by the approval rule). NOT verified: the import and run on the tablet.

### Addendum 2026-11-25 — goal-based control practice, Flash Executioner first
Owner: the practice only said "press this button" with no goal; show what a skill does (e.g. freeze, show how to get orbs) and start as the Flash Executioner. Practice (stage 1) now: opens as the Flash Executioner (`equippedJobs` forced to {2,1}, handed back as Flash at the end); every step has a visible goal (beacon to walk to, 3 orbs to collect, Rush through a target, Command marks targets, Blink to the farthest mark, Verdict on 3 marked targets, then the Ruler's attack/skills); each combat step starts with a ghost run (scripted button press, letterbox bars) before the player tries. New passive practice targets: `Enemy::dummy/dummyHits`, `Combat::stepDummy`. Files: `combat.hpp`, `story_director.hpp`, `presentation.hpp` (`StoryView::demo/goalX/goalCount/goalDone/targetsHint` drawn in `drawStory`), `tests/core_tests.cpp` (practice test replaced by three), `Docs/TUTORIAL_PROLOGUE_FLOW.md`. Verified: `c++ -std=c++17 -fsyntax-only -Wall tests/core_tests.cpp` passes (syntax only). NOT verified: test run, `presentation.hpp` compile (SDL3), build, playtest, device. Open/guesses: ghost-run timings, Ruler skill goals (Sweep/Buff/Ultimate fall back to a timeout because their effect on a target was not checked), the bot test's timing, Flash Rush marks nothing in code (Command lances mark), so Rush is taught as a charge only.

### Addendum 2026-11-25 (b) — practice foes now fight back
Owner: dummies are not real combat; teach dodging an attack. Added `Enemy::fights`: a practice foe with normal AI that never falls. Step 2 is now a dodge lesson (dash through a real attack unhurt; ghost run dashes at attackAge .40-.48 s, covering the .56 s contact frame with the .15 s dash i-frames); steps 3, 4, 9 use live foes after their ghost run. Rush has no i-frames in code, so it is taught as a charge, not a dodge. Verified: `-fsyntax-only` core tests only. NOT run: tests, build, playtest.

### Addendum 2026-11-25 (c) — tutorial rewritten as one continuous fight with time stops
Owner: the tutorial read like a user manual. Replaced the reset-per-step practice and ghost runs with 16 beats in one fight (see the stage 1 row in `Docs/TUTORIAL_PROLOGUE_FLOW.md`). New: `Combat::timeScale` (0 = story time stop; `main.cpp` freezes the player animation too), `StoryView::freeze/previewR` (tint, frame, reach ring, dotted target line in `drawStory`), practice foes keep a 30 % health floor instead of re-healing. Tests: tutorial tests replaced (opening, full bot run, time-stop dodge). Verified: `-fsyntax-only` core tests; Studio `.so` build. NOT run: core tests, playtest, device. Guesses: every trigger distance/time, foe placements, hp values for the kill beats.

### Addendum 2026-11-25 (d) — tutorial input lockout
Owner: remove variables; during a tutorial beat only that beat's button works. `StoryDirector::practiceInput` masks dash/attack/skills/swap (and jump outside the Stick beats) except the beat's own control; walking always works. Core tests run on request: 88/122 pass; the 34 failures are legacy/unrelated (29 already fail at HEAD). New lockout test passes.

### Addendum 2026-11-25 (e) — dark stacks and light x dark reactions (eclipse / solar burst)
Owner: put dark stacks on Space Ruler skills, let the Ruler interact with its own stacks, and let light and dark stacks react. Implemented in `combat.hpp` (Enemy dark stacks, `darkHit`, `solarCheck`, `startEclipse`, `stepEclipse`, `eclipseFx`), drawn in `presentation.hpp` (violet pips under the health bar; placeholder rings/disc for the eclipse and the solar burst). Spec and numbers: `Docs/COMBAT_SKILLS_AND_STATUS_DESIGN.md` (last section). Tests: 6 new core tests, all pass; full suite 94/128 with no new failures versus before. NOT done: the tutorial beat that teaches it (design doc beat 7, beats 6-7 still unbuilt), authored VFX art, sound, tuning, playtest, device.

### Addendum 2026-11-25 (f) — reactions are element-based, not class-based
Owner: other jobs will also stack light/dark; make it easy to apply broadly. Refactored `combat.hpp`: `Element`/`ElementTag`/`namespace tags`, `stacksOf/addStacks/consumeStacks`, `elementHit/elementReact`, `startEclipse/startSolar`; light stacks are now counted (`lightStacks`), shared stack cap/duration. No job names remain in the reaction code; how to add a job: Docs/COMBAT_SKILLS_AND_STATUS_DESIGN.md (last section). Tests: reaction tests updated + 2 new (any-tag reacts; light stacks count/fade); suite 96/130, no new failures. NOT done: the interesting effects (re-emitted lances, multi-hit solar, orbit well, 3/5-stack thresholds), art, tutorial beats 6-7, playtest.

### Addendum 2026-11-25 (g) — void stack effects
Owner: spikes execute by void stacks (not bosses), repulsion scales with void stacks, 3 stacks slow, 5 stacks erode (stop all movement + DOT). Implemented in `combat.hpp` (voidExecute, knockback scaling, slow, erosion), tinted in `presentation.hpp`, 3 new core tests (suite 99/133, no new failures). Boss is not frozen by erosion (my call, DOT only). Numbers are estimates; no playtest.
