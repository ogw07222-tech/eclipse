# ECLIPSE Art Pipeline v2 — headless authoring and review

Status: **architecture + asset recipes, NOT a verified game-art generator**. Updated 2026-11-08.

## Scope and authority
- Game repository `main` is canonical; read `AGENTS.md`, `README.md`, `Docs/PNG_GITHUB_LIBRARY_WORKFLOW.md` before editing.
- No 3D mesh, 3D rigging, mandatory GUI, automatic production-asset overwrite, or GitHub Actions.
- Character **final** sprites are individually authored/revised pixel poses. A projected BVH skeleton, Pixelbolt procedural frames, and raster translations are **reference/draft only**, never automatically approved character artwork.
- Preserve existing paths, character silhouettes, palettes, animation mapping, weapon-hand connection, hitbox and runtime semantics.
- Separate game assets (`Assets/Resources/Eclipse/`) from authored working sources (`Art/`) and temporary previews (`Art/tool-previews/`).

## The five deliberately limited components

| Role | Implementation | Inputs | Outputs | Status |
| --- | --- | --- | --- | --- |
| Motion references | `ogw07222-tech/eclipse-motion-library` `tools/motion_query.py` | reviewed or unreviewed projected CMU BVH JSON | side-view joint positions with original left/right identity | separate existing repo; no implicit sync |
| Effect recipe rendering | `tools/pixel-tools.sh` (Pixelbolt; PixelFX) | native tool recipes / explicit effect ID | draft PNG/contact/sheet; tool-owned paths | wrapper exists; device execution unverified |
| Pixel editing | Pillow and optionally **LibreSprite CLI** | native-resolution PNG, pixel masks, individually authored poses | revisioned draft images and per-frame corrections | Pillow existing; LibreSprite not installed/verified |
| Atlas diagnostics | `native-engine/tools/eclipse_sprite_tool.py` | unchanged production PNG or draft PNG + dimensions/index | JSON metrics/contact/GIF/optional MP4 in review dir | existing, not executed in this change |
| Export and approvals | manually reviewed manifests, explicit packaging and SDL3 mapping | approved authored PNG + metadata | exact original runtime PNG path on a later approved integration | **not automated** |

**Important:** Use `tools/pixel-tools.sh` for installed Pixelbolt/PixelFX wrappers; do not invent a cross-tool JSON API. The JSON in `Art/pipeline/recipes/` is **ECLIPSE orchestration metadata**, NOT Pixelbolt's or PixelFX's native syntax. A converter for these recipes is NOT implemented. Sprite Fusion TypeScript is a possible later optional adapter, not installed nor integrated. Aseprite is likewise optional; don't require paid tooling.

## Data flow

1. **Locate and fingerprint** the exact canonical PNG and required runtime mappings. Get actual binary bytes (GitHub text API alone is insufficient), check SHA-256, atlas cell geometry and index semantics. Do not treat Library as automatically synced.
2. **Create working revision** under `Art/<subject>/` with a `manifest.json`: source-ref/SHA, cell size, frame indices, pose provenance, pivot, timing and `status: draft`. Preserve original.
3. **Character lane:** choose 6–8 coherent authored key poses and create/revise *distinct* in-between pixel poses. Use side-projected Motion Library joint coordinates to audit only; track left/right contacts and cycle seam. For knight run use 24 poses at ~0.7 seconds; White Night run use 32 distinct displayed poses with the **runtime distance-driven cycle**, not a fabricated fixed FPS.
4. **Effects lane:** generate Pixelbolt/PixelFX drafts via their supported native input/tool IDs; optional follow-up Sprite Fusion adapter after licensing and CLI investigation. Review alpha-only effects separated from demo characters, intrinsic frame durations, origin/pivot, blend and alpha.
5. **Revision lane:** apply *local frame-level, hand-reviewed pixel edits* in an image editor or explicitly authored pixel patches. Pillow may handle masks, exact palette replacement and lossless copy/packing, never pass limb-translation/warping off as new authored pose art.
6. **Inspect read-only:** use `eclipse_sprite_tool.py` with explicit `--start` and `--end` (end **exclusive**), native cell dimensions, `--strict` and review output path. Inspect native size + nearest-neighbor contact sheet at intended playback speed; automatic bbox/pixel-diff metrics cannot decide motion quality.
7. **Approve:** record `reviewed` only after visual review, `approved` only after owner approval, `integrated` only after committed runtime PNG AND corresponding animation mapping, package manifest and metadata have been changed and checked. Runtime game/device testing is owner-controlled and **not run here**.
8. **Mirror:** manually upload exact approved bytes to `/ECLIPSE/eclipse main/<relative-path>` and verify SHA-256; otherwise report `not synced` and record divergence.

## CLI entrypoints already present

```bash
# no automatic setup, installation or rendering during this design change
bash tools/pixel-tools.sh versions
bash tools/pixel-tools.sh bolt-inspect /absolute/path/to/native.pixel.json
bash tools/pixel-tools.sh fx-check melee/sword-slash

# read-only atlas review, example for WHITE NIGHT (88..119 inclusive):
python3 native-engine/tools/eclipse_sprite_tool.py \
  Assets/Resources/Eclipse/ui/white_night_motion.png \
  --width 64 --height 96 --start 88 --end 120 \
  --fps 24 --format report --strict \
  --output Art/tool-previews/review-white-night

# Knight run indices 432..455 inclusive:
python3 native-engine/tools/eclipse_sprite_tool.py \
  Assets/Resources/Eclipse/knight_motion.png \
  --width 64 --height 94 --start 432 --end 456 \
  --fps 34.285714 --format report --strict \
  --output Art/tool-previews/review-knight
```

Note: `--fps 24` for White Night is a **preview sampling value**, not validated in-game timing. Existing display rate is distance-driven. The tool writes reports and contact images; nothing is imported automatically.

## CLI roadmap (not implemented)
- `tools/art_pipeline.py inspect --recipe ...`: read immutable source and check metadata/source hashes.
- `tools/art_pipeline.py extract --recipe ...`: exact indexed PNG cutouts into versioned draft dir.
- `tools/art_pipeline.py compare --before ... --after ...`: changed-pixel masks and annotations, preserving unchanged cells.
- `tools/art_pipeline.py stage --recipe ...`: copies candidate PNG into `Art/`, never into runtime path.
- `tools/art_pipeline.py review --recipe ...`: aggregate size/alpha/contact/loop reports and human acceptance checklist.
- `tools/art_pipeline.py integrate --recipe ...`: intentionally gated future action requiring approved art and verified dependency/mirror rules, not implemented.

## Acceptance gates
- **G0 provenance:** known source revision + real PNG SHA-256, exact existing production path.
- **G1 raster:** correct RGBA/cell/atlas dimensions, native pixel grid, no unintended frames changed.
- **G2 motion:** independently reviewed hands/sword, torso, alternating feet, knee folding, silhouette, loop closure, cloth follow-through; do not assert based on numerical checks alone.
- **G3 effects:** transparent backgrounds, no embedded demo actors, stable color/edge/pivot/timing.
- **G4 integration:** matching SDL3 frame mapping, packaging, native resolution, required asset error path, author approval.
- **G5 device:** owner verifies actual in-game playback; no assistant-triggered tests/Actions.

## Known limitations
- Tool availability is environment-specific. Cloud/connector access cannot directly execute Node CLIs installed only in Termux.
- No CMU-to-Pixelbolt adapter, guaranteed LibreSprite CLI install, or Sprite Fusion adapter exists as part of this design.
- All recipes are descriptive metadata, not a renderer; no new production character/FX artwork or gameplay change is claimed.
- Third-party source and licenses must be checked and pinned before packaging; no automatic network install.
- `.github/workflows/native-engine-checks.yml` and `native-android-apk.yml` were read and each uses manual `workflow_dispatch` only; never dispatch either.
