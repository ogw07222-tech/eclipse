# ECLIPSE — Pixelbolt + PixelFX (local authoring tools)

## Purpose and limits

- **Pixelbolt:** JSON-based pixel sprites, rigged motion drafts, animation exports and frame inspection.
- **PixelFX:** code-based effects, sprite sheets and Aseprite-style frame timing JSON.
- Both are third-party MIT projects. Review their LICENSE/NOTICE files before redistribution; PixelFX's Patch Pixel font is excluded from its code/effects license.
- These tools are **not** installed as ChatGPT plugins. A shell-capable agent or developer must execute the CLI in a local environment. GitHub file access alone cannot run these tools.
- Generated motion is **reference or draft art**, not approved character animation. Follow `AGENTS.md`: manual key poses, continuous anatomy, 24 standard knight / 32 White Night Executioner run poses, pixel-level correction and owner acceptance.
- **Never** start GitHub Actions. This adapter neither adds workflows nor triggers them.
- No release sprites or original character PNGs are replaced automatically.

## Prerequisites

Local shell with git, npm and Node.js 20+. For Termux: `pkg install git nodejs-lts -y`. Setup downloads third-party source and package dependencies; network access is needed **only for setup**. The cloned git commits are retained locally without automatic updates.

From the ECLIPSE repository:

```bash
bash tools/pixel-tools.sh install
bash tools/pixel-tools.sh versions
bash tools/pixel-tools.sh bolt-preview
bash tools/pixel-tools.sh fx-gallery
```

To use your own Pixelbolt JSON:

```bash
bash tools/pixel-tools.sh bolt-inspect /absolute/path/to/motion.pixel.json
bash tools/pixel-tools.sh bolt-render /absolute/path/to/motion.pixel.json draft-001
```

Draft output is under `Art/tool-previews/pixelbolt/`; do not copy to production without art review. Pixelbolt has built-in run, walk, jump and attack pose tools, but is not a CMU BVH importer. The ECLIPSE Motion Library BVH/JSON to Pixelbolt schema converter is **not implemented**.

Render an effect:

```bash
bash tools/pixel-tools.sh fx-sheet melee/sword-slash
bash tools/pixel-tools.sh fx-check melee/sword-slash
```

PixelFX writes its sheet and frame metadata inside its own local checkout (see command output). Do not assume that output frame sizes, loop timing, pivot or blend modes match ECLIPSE's SDL3 renderer. Effects drawn on the demo dummy must be separated from the dummy before integration.

## Review gates

1. Inspect effect frames, alpha, pivot, palette and duration; inspect rigged motion at native size and actual playback timing, including loop closure.
2. Compare left/right foot identity, ankle plant/sliding, shoulder/elbow/sword grip, torso movement and cape/hair coherence.
3. Preserve and version originals in `Art/` with an asset manifest per `Docs/PNG_GITHUB_LIBRARY_WORKFLOW.md`.
4. Only after approval, pack and place PNG into its **existing exact** `Assets/Resources/Eclipse/` target. Update SDL3 atlas metadata, contact frame alignment and packaging rules together.
5. Owner tests on device. Neither local builds nor GitHub Actions are run by this integration.

## Known unfinished items

- Local `install` and sample rendering are **not yet executed or validated** on the owner's Galaxy Tab/Termux.
- No verified 64×96 character PNGs, combat effects, runtime integration or Motion Library converter exist from this adapter.
- Effects and motions need review and a stable manifest, especially when importing third-party source revisions.
- This file is the setup guide, not a claim of functional in-game integration.
