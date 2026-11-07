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
