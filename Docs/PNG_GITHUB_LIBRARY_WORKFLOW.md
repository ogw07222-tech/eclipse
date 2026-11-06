# ECLIPSE — GitHub + ChatGPT Library PNG Asset Workflow

Status: mandatory asset-handling procedure documented for **future** work. This document does **not** claim that the ChatGPT Library is currently synced to GitHub.

## Responsibility and source of truth
- **GitHub `ogw07222-tech/eclipse`** is the canonical, versioned repository. Exact production paths and runtime mappings live here.
- **ChatGPT Library** is a convenient user-managed asset shelf for reusable image references, approved PNG exports, previews, and comparison sheets. It is **not** a Git repository, runtime asset server, automatic mirror, or authoritative replacement for the GitHub production PNG.
- **ChatGPT conversation attachments** and Library assets may be read or exported when accessible with available Files/connector tools. Never assume an uploaded image is already present in GitHub or automatically imported into the game.
- No automatic two-way sync should be claimed. The assistant must explicitly upload/store a Library copy when supported and requested; otherwise provide the asset with a clear manual Library-import step. Never overwrite a Library file or GitHub production asset based solely on similar filenames.

## Existing project paths (preserve these)
- `Art/`: editable source artwork, independent authored key poses, variants, contact sheets and review assets; preserve originals.
- `Assets/Resources/Eclipse/`: approved runtime PNGs, including existing root atlases, `monsters/`, `ui/`, `title/`, `demon_castle/`, `forest*` and other established resource paths. **Do not relocate or rename existing active runtime files for tidiness.**
- `Docs/`: specifications and asset tracking records.
- Avoid placing game-specific tooling or assets in ECLIPSE Studio; it is a separate repository.

## Stable identity and manifest
For each actively edited image, track the following fields in a corresponding `Art/<subject>/asset-record.md` or `Art/<subject>/manifest.json` (choose existing local conventions before creating new files):
- `asset_id`: stable identifier, e.g. `enemy/castle/gargoyle/idle` (not a display title alone).
- `source_path`: exact GitHub path under `Art/`.
- `runtime_path`: exact target GitHub PNG path, if approved/implemented; otherwise null.
- `library_name`: human-friendly, stable Library filename/title; **not** a guaranteed Library ID.
- `library_file_id`: only when actually returned by a Library tool; do not fabricate.
- `github_ref` (branch/commit), source Git blob SHA or SHA-256 when available, `version`, `status` (`concept` / `draft` / `reviewed` / `approved` / `integrated`), `last_updated`.
- Pixel data: width, height, frame cell size, frame count, rows/columns, orientation, pivot, playback FPS/timings, alpha/color constraints, attack contact frame, and dependent runtime/metadata/collision files where relevant.
- Record whether the asset is a concept image, authored key pose, actual frames, packed spritesheet, or QA preview. Do not interchange these labels.

Recommended human filename for Library exports: `ECLIPSE__<asset-id-with-dashes>__vNN__<type>.png`. Keep **production filenames unchanged** where code expects specific paths.

## A. Fetch a PNG for editing
1. Identify the precise asset by name, identity, current gameplay scene, and GitHub path; inspect its associated atlas metadata and render usage first. If ambiguous, enumerate candidate paths and compare image dimensions/content; avoid guessing.
2. Fetch the **actual bytes** using a connected GitHub file retrieval/download path when available; GitHub text-only tools do **not** provide binary PNG bytes. If binary fetch is unavailable, use a verified user-supplied PNG/Library file or explain the limitation. **Do not create a visually guessed replacement and call it an edit of the original.**
3. Optionally retrieve the corresponding Library source/working copy. Compare against the GitHub canonical version using dimensions, content hash (if available), and visual inspection. If they differ, do not silently overwrite either.
4. Preserve the source original before editing; work on a new named draft/version. Respect image editing tools for actual visual changes, not merely prompt descriptions or code-only placeholders.
5. Check native-resolution pixel integrity, alpha transparency, nearest-neighbor, design consistency, frame layout, foot/weapon/cape continuity, and intended in-game timing. Follow the authored-frame rules in `AGENTS.md`.

## B. Approve and integrate a corrected PNG
1. Show a visible before/after comparison plus native resolution and enlarged nearest-neighbor preview when possible; note exact differences and uncertainty. Wait for explicit approval where the change is only a design proposal.
2. Save editable source in `Art/`; when approved, pack production atlas at its exact existing `Assets/Resources/Eclipse/` path. Preserve originals via Git history and separate drafts.
3. Update animation/runtime mappings, contact frames, collision silhouettes, atlas metadata, asset packaging/import rules, and dimension checks **together** whenever the artwork requires it.
4. Update the manifest status/version/hash and document what actually changed. Commit to the game repository only after checking effective `.github/workflows/` triggers per `AGENTS.md`; **never run GitHub Actions without explicit run-specific approval**. Never infer that commit permission equals permission to build/test.
5. If the ChatGPT Library is accessible and user wants the updated copy stored, upload the approved image there and verify the resulting record. If not accessible, provide a real downloadable PNG/ZIP and say the user must add it manually. Explicitly distinguish GitHub updated / Library updated / only local preview.
6. User runs in-game tests; do not claim visual gameplay validation from static checks.

## C. Library → GitHub and GitHub → Library
- **Library → GitHub:** locate the exact Library item, fetch/materialize it if supported; inspect/version-check; use as edit source; commit approved PNG to the verified runtime path with dependencies; explicitly report changed destinations. A Library URL/title alone is not proof of bytes or identical content.
- **GitHub → Library:** fetch actual GitHub PNG bytes with a binary-capable path; optionally create a versioned Library reference/preview; verify upload result when supported. GitHub file links alone do not import assets into the Library.
- **Conflicts:** do not assume “newest filename” wins. Compare content/hash and author intent; keep both versions (e.g. `v03`, `v04`) until reviewed.
- **Mass import:** batch by scene/asset identity; generate a manifest with expected file counts and checksums. Detect missing, duplicated and wrong-size PNGs. Do not silently skip failed imports.
- **Failures:** always surface missing file, wrong path, unsupported binary download, hash/dimension mismatch or absent permissions. Preserve previous working game assets. Current-scene required game resources must follow the explicit `ECLIPSE ASSET ERROR` rule.

## Response checklist for every PNG task
Report concise status for: **canonical GitHub path / Library item (or not synced) / draft or integrated / dimensions and frame mapping / visual review performed or not / game tests not run**.

### Example record (illustrative, NOT an existing synced asset)
```json
{
  "asset_id": "enemy/castle/gargoyle/idle",
  "source_path": "Art/Monsters/castle/gargoyle/idle/",
  "runtime_path": null,
  "library_name": "ECLIPSE__enemy-castle-gargoyle-idle__v01__concept.png",
  "library_file_id": null,
  "github_ref": null,
  "version": 1,
  "status": "concept",
  "frame_cell_px": null,
  "frames": null
}
```
