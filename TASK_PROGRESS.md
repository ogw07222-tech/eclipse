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
