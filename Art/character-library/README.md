# ECLIPSE Character Asset Bridge — GitHub ↔ ChatGPT Library

## Purpose
GitHub `ogw07222-tech/eclipse` `main` is the authoritative game asset repository. ChatGPT Library `/ECLIPSE/characters/` is a **versioned working copy**, intended for direct image inspection and editing inside ChatGPT. It is **not** a Git remote or an automatic two-way sync.

## Layout
| Character | GitHub runtime PNG | Library mirror |
|---|---|---|
| Knight | `Assets/Resources/Eclipse/knight_motion.png` | `/ECLIPSE/characters/knight/knight_motion.png` |
| White Night Executioner | `Assets/Resources/Eclipse/ui/white_night_motion.png` | `/ECLIPSE/characters/white-night/white_night_motion.png` |
| Mage | `Assets/Resources/Eclipse/mage_motion.png` | `/ECLIPSE/characters/mage/mage_motion.png` |

Keep authored pixel-key poses, source references, palettes, motion metadata and revisions beside each character under the GitHub `Art/` tree. A similar structure may be used in Library. Store contact sheets and playback reviews separately from production atlases.

## Mandatory editing workflow
1. Read `AGENTS.md` before any character task. Check GitHub `main` and its current runtime animation mapping.
2. Find exact corresponding **Library** file (not same-looking artwork) and compare SHA-256 with the GitHub version. Upload missing production PNGs from the actual Git checkout; don't substitute unrelated older concept images.
3. If hashes differ, identify the source commit. Never silently overwrite newer revisions. Missing Library file means **NOT SYNCED**, not that an image was inspected.
4. Review/author coherent pixel-art key poses and intermediate frames (not raster body shifts as final art), preserving character identity and connections of shoulder/arm/weapon/cape/hair.
5. Repack *actual* PNGs, inspect native resolution and cycle seam, record dimensions/frames/source SHA-256. Owner tests runtime on device.
6. Commit approved PNGs, editable source frames and metadata to GitHub; update the Library mirror from the same committed binary. Verify the two copies' hashes, and record the commit ID.
7. Avoid claims of successful integration or sync until both copies are present and their bytes verified.

## Governance
- Never trigger GitHub Actions, automated tests, or APK workflows without prior, run-specific owner permission.
- Before a GitHub write, inspect `.github/workflows/` for automatic triggers per `AGENTS.md`.
- Do not relocate runtime PNGs out of `Assets/Resources/Eclipse/`; the Library is an additional mirror, not an asset loader.
- Add new characters to `character-asset-manifest.json` and create a matching Library subfolder.
- This README/manifest deliberately do not claim the production PNGs have been mirrored yet.
