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
