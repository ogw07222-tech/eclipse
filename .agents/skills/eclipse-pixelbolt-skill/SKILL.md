---
name: eclipse-pixelbolt
description: Use Pixelbolt CLI or MCP for ECLIPSE pixel-art sprites, animation, precise per-pixel repair, effects, QA and PNG/atlas export. Trigger on requests to create, modify, inspect or export ECLIPSE pixel art. Preserve approved art direction and original source.
---

# ECLIPSE Pixelbolt Skill

## First: environment and source of truth

1. Read the target project's `AGENTS.md` and character specifications, and inspect the actual input files. Project instructions override defaults below.
2. Locate Pixelbolt: if `PIXELBOLT_CLI` points to a local `bin/pixelbolt.js`, use it; otherwise find `./Pixelbolt/bin/pixelbolt.js` or `./bin/pixelbolt.js`; if none exists, ask for an existing installation or use `npx -y pixelbolt@latest` **only when network access is permitted**. Node.js >=20 is required. Do not claim Pixelbolt ran when only documentation was read.
3. Use `scripts/pixelbolt.sh` as a safe CLI adapter. `PIXELBOLT_CLI=/absolute/path/Pixelbolt/bin/pixelbolt.js` ensures use of a pinned local checkout. CLI versions differ: confirm supported commands/options using `help` and `schema`.
4. Consult the upstream quick-start and exact schema from the pinned Pixelbolt version (see `references/UPSTREAM.md`) rather than guessing a JSON field.
5. Never invoke GitHub Actions or cloud builds without the user's explicit consent. Prefer local validation and manual tests.

## ECLIPSE art contract

- Pixel-perfect, no antialiasing, blurred resampling, smooth warping, indiscriminate upscale or palette drift. Output game assets at 1x native resolution.
- Keep the original anatomy, armor, palette, helmet, hair, sword hand, cape, lighting and dark-fantasy style. Treat existing approved PNG as a visual reference, not as a license to invent a new design.
- Default character action cell: **64x96 px**, but obey the actual file/AGENTS.md; e.g., older ordinary knight sheets may use 64x94. Never silently crop or stretch to force a default.
- Animation length/FPS come from the project: do not assume 24 or 32 frames unless the task specifies them.
- Existing multi-frame sprites: work from a source Pixelbolt JSON recipe where available. Pixelbolt's PNG import can turn an existing PNG into pixel data, but a flattened PNG does **not** recover separate joints, layers, motion intent or anatomy. Do not claim import magically rigs it.
- Rig/poses are optional authoring scaffolds, never final stickman art. For character animation, prefer deliberate, authored silhouettes and attached hands/props over mechanically translated limbs.
- Main weapon must stay in the intended hand; no disconnected hands, foot swapping artifacts, shortened limbs, immobile shoulders or cape, or frozen torso.

## Standard authoring loop

1. **Intake:** record file paths, asset dimensions and frame layout, current color palette, transparent background, intended origin/feet baseline, loop duration, frame count, chosen animation, and immutable reference. Copy original into `work/original/` or retain it in version control. Do not overwrite the original.
2. **Recipe:** select a close upstream example (`examples/hero.poses.json`, `examples/hero-sword.poses.json`, or a suitable `.fx.json` preset), or author a valid Pixelbolt JSON. Use exact fields of the installed version. Build silhouette and key poses first; then secondary motion and pixel refinements. Keep the `.json` editable.
3. **Validate:** `scripts/pixelbolt.sh validate path/to/asset.json`; fix schema/clipping errors and re-run. If source is a `.poses.json` or `.fx.json`, first confirm and run the version-supported `compile` workflow; validate the generated output as applicable.
4. **Inspect:** `scripts/pixelbolt.sh inspect path/to/asset.json --out work/review/contact.png --native --diagnostics`. Review every frame at 1x and enlarged nearest-neighbor, plus silhouette/grayscale/onion/motion views when available. Do not substitute a static contact sheet for actual looping playback.
5. **Repair:** read affected pixels with `inspect --grid --region x,y,w,h`; use `patch <recipe> --changes <changes.json> --out <new-recipe.json>` to make narrowly scoped, reviewable changes. Inspect adjacent frames and the cycle seam after each correction.
6. **Export:** `scripts/pixelbolt.sh render path/to/final.json --out work/export-v1 --expect '{"width":64,"height":96,"frames":24}'` **only if 64x96 and 24 frames are verified task requirements**. Use a fresh output directory, verify the returned paths/expectation checks, and optionally generate a playback GIF with `gif` if supported. Never use `--force` without an explicit, justified overwrite plan.
7. **Review:** pixel dimensions, grid boundaries, transparency, sheet order, alpha, loop seam, counts, timing, origin/foot contact, leg crossings, hands, sword and cape. Do visual QA, not just numeric tests. Export atlas metadata and source recipe together. Record verified output paths and failure modes.

## Run cycle: side-view knight

- Reference gait: stable head/torso volume; pelvis and shoulders counter-rotate; reciprocal arm/leg action; each leg reaches forward in alternating half-cycles; boot remains connected to shin and planted during stance; plausible bent knee in recovery.
- Sword follows near hand/forearm with coherent angle. No free-floating blade.
- Cape lags the torso with non-identical, continuous shapes, without slicing into the sword or popping across loop seam.
- Review 0%, 25%, 50%, 75% and wrap 100% key points, then every intervening frame. Track near/far leg identity using the project's shading convention.
- A 24-frame cycle played in 0.7 s is ~34.286 FPS, not 16 FPS. Preserve specified *duration* and playback timing unless the game code states otherwise.

## Mode selection

- **Create:** new assets from recipe/examples/presets; Pixelbolt renders and checks them.
- **Edit:** when a matching recipe exists, use Pixelbolt `inspect`/`patch`. When PNG only, import if supported and make limited pixel corrections; complex anatomy redraw requires authored frame work, not auto-warp.
- **QA:** use `inspect`, optional motion diagnostics and independent PNG frame comparison (Pillow allowed for checking outputs, not a substitute for Pixelbolt creation).
- **Existing visual:** do not claim Pixelbolt can infer the missing pose layers or guarantee premium character art from a flattened sheet.

## Failure and continuation

If Pixelbolt is missing, Node is too old, network is unavailable, input recipe is incompatible, or a tool command fails: stop asserting success. Document completed work, touched files, missing files, exact error and next steps in `TASK_PROGRESS.md` at the task working directory. Keep source and partial artifacts. Do not commit changes or claim GitHub updated unless it actually happened.

## Deliverables

Provide the recipe, PNG sheet (and frame directory/atlas metadata if created), preview, QA results and the list of source/target files. Mark any visual criterion that still needs inspection as `unverified`.
