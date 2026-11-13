# ECLIPSE — Mandatory Agent Instructions

**READ FIRST before inspecting, planning, editing, committing, opening/merging a PR, building, or testing.** This policy is set by the project owner and applies to ALL game work, including subagents and automated coding assistants.

## ABSOLUTE RULE: NO GITHUB ACTIONS WITHOUT EXPLICIT OWNER APPROVAL

- Do **not** invoke, dispatch, rerun, retry, schedule, enable, or otherwise trigger ANY GitHub Actions workflow without the owner's **explicit, prior approval for that specific run**.
- Do **not** enable or introduce automatic CI/build/test triggers, including `push`, `pull_request`, `pull_request_target`, `schedule`, `release`, `workflow_run`, `repository_dispatch`, and `workflow_call` chains. Prefer `workflow_dispatch`-only definitions, **but never invoke them without approval**.
- **Never assume** a request to implement code, fix a bug, commit, open/merge a pull request, or produce an APK constitutes approval to run GitHub Actions.
- The **owner conducts testing personally**. Do not run tests on their behalf without separately being asked. Even when testing is requested, GitHub Actions still requires explicit approval.
- Before **any push, commit, PR creation/update, or merge**, inspect the effective workflow triggers in `.github/workflows/` in `ogw07222-tech/eclipse`. If an automatic workflow could run, first disable its trigger without dispatching it; otherwise do not perform the triggering operation and explain the blocker.
- No claims of CI/test pass unless the owner supplies verified results. Static inspection and editing source files are allowed. Prefer manual/local build instructions where appropriate.
- Preserve existing signing identities and installed app data. Preserve the game's signing identity and installed app data.

## Safe workflow pattern

```yaml
on:
  workflow_dispatch:
```

This is a manually available workflow, **not permission for assistants to use it**.

## Development workflow

1. Read this file and repository-specific README before working.
2. Make requested code/assets/documentation changes only, respecting the Game repository boundaries.
3. Do not run GitHub Actions or user-owned tests.
4. Report what changed and explicitly mark tests/builds as **not run**.
5. Ask for approval only if a GitHub Actions run is actually necessary.

If any other instruction conflicts, follow this project owner policy and do not initiate GitHub Actions.

## Dialogue-free storytelling (mandatory creative rule)

ECLIPSE tells its story without character dialogue, voice lines, narration, subtitles, speech bubbles, or written plot exposition. Story-critical information must be understandable through pixel-art acting, character movement, staging, animation, camera, sound/music, visual motifs and environmental changes. No cutscene or tutorial should require reading prose to follow the narrative.

- Tutorials: contextual button glyphs, visual input demonstrations and direct feedback instead of instructional sentences. Keep controls understandable and accessible.
- Narrative: show the player the story through actions and consequences; use visual continuity to convey the Demon King's second phase, the protagonist's defeat, reincarnation and loss of the legendary classes.
- Interface exceptions: functional menus, accessibility labels, settings and explicit technical errors are permitted; this is **no story dialogue**, not a ban on safety/error feedback or necessary UI text.
- Review each major beat for legibility without text and without audio, then enrich with sound; never rely solely on color, audio, or opaque effects.
- Reference: `Docs/TUTORIAL_PROLOGUE_FLOW.md`.

## GitHub + ChatGPT Library PNG exchange (mandatory)

For every PNG or sprite edit/import/replacement, follow [`Docs/PNG_GITHUB_LIBRARY_WORKFLOW.md`](Docs/PNG_GITHUB_LIBRARY_WORKFLOW.md).

- **GitHub is the canonical versioned source of truth.** `Art/` holds editable originals and authored poses; `Assets/Resources/Eclipse/` holds only approved runtime assets at exact existing paths. The ChatGPT Library is an optional companion shelf for source references, drafts and approved exports, **not** an automatic GitHub mirror.
- Track stable asset identity, exact GitHub source/runtime paths, image dimensions, frame metadata, version/status and real content hashes/refs where available. Never fabricate Library IDs or assume a file named alike is identical.
- For “modify existing PNG”, retrieve and inspect the **actual original binary image** first. If current tools can only read GitHub text and cannot retrieve binary PNGs, state that limitation and seek accessible Library/uploaded source; never claim a newly generated approximation is a faithful edit.
- For “bring PNG from Library”, verify item bytes, compare with GitHub, keep original, and commit only the approved change with matching runtime/metadata/packaging updates. For “save PNG to Library”, explicitly upload and verify if supported; otherwise provide the file for manual import. **Never claim background or bidirectional sync.**
- Maintain a reviewable before/after with native-resolution visual checks. Do not silently overwrite canonical PNGs, alter silhouettes/hitboxes, or move runtime paths. Preserve pre-edit version via Git and report which of GitHub / Library / local draft actually changed.
- This rule does **not** authorize GitHub Actions, unrequested tests, or automatic builds. Observe the absolute owner-approval rule above.

## Asset loading: fail loudly (mandatory)

- Required game images (including the five demon-castle entrance layers and both title layers) must never silently fall back to another scene, placeholder, or missing animation when loading/decoding, dimension checks, or texture creation fail.
- Report an explicit `ECLIPSE ASSET ERROR` with the exact file path, expected/actual dimensions when relevant, and underlying SDL error when available. Stop the affected game's startup/load with nonzero status rather than showing an unrelated scene.
- Scene-selection policy is independent of asset integrity: only make scene-specific assets fatal when that scene is actually requested. Forest startup must not terminate just because the not-yet-entered castle preview pack is absent. Report castle preload errors in logs, and show a visible modal error and fail rather than falling back if the castle scene is requested.
- Startup failures for required current-scene assets must display a user-visible error dialog (e.g., SDL_ShowSimpleMessageBox) with the file/error reason before exiting. Logging alone and instantly closing the game is not acceptable.
- When introducing a new required asset, update packaging/import rules and document its expected paths and dimensions. Do not claim runtime success without owner-run testing.

## Pixel animation authoring — hand-authored key poses first (mandatory)

For all ECLIPSE playable characters and enemies, animation **artwork** must be authored primarily as deliberate pixel-art key poses, not generated by moving/scaling existing raster regions with Python.

1. **Author the poses:** Establish clear hand-drawn pixel-art key poses for anticipation, contact, passing, recovery, and loop closure; draw the upper and lower body in the same pose, maintaining character identity, perspective, proportions, hands gripping weapons, armor overlaps, cape and hair.
2. **Fill intermediate frames:** Draw and revise actual intermediate pixel poses from the approved keys to meet the move-specific frame count and timing. Never claim duplicated, warped, translated, or automatically interpolated frames are newly hand-authored drawings.
3. **Use scripts for tooling and QA:** Python/Pillow and other scripts may slice/pack PNG atlases, validate dimensions/alpha/palette, map runtime frames, track ankles and contact, assess segment length/foot sliding, compare frame differences and loop boundaries, and produce contact sheets/GIF/MP4. A procedural draft or rig-generated motion can be used as a **reference**, but it is not the final pixel artwork without manual pixel-level art review and corrections.
4. **Review at real playback speed:** Inspect the entire sequence, including the last-to-first loop, at native pixel resolution and actual in-game timing. Check left/right leg identity and alternating contacts, knee folds, waist-to-shoulder follow-through, hand-to-sword connection, trailing cape/hair, and jitter/flicker/teleporting limbs. A checksum, frame count, rendered MP4 file, or script success is **not** proof that the motion looks natural.
5. **Source and integration:** Retain editable authored frames and key poses in `Art/`; place only approved, packed game-ready PNG atlases in `Assets/Resources/Eclipse/`. Keep displayed-frame mappings and timing synchronized with the actual atlas. Distinguish authored key poses, intermediate drawings, and automated QA artifacts in the documentation.
6. **Acceptance:** Work is complete only after actual PNG assets have been reviewed for continuity and committed with their matching metadata/runtime references; otherwise report exactly what remains. Owner performs final device/game tests; do not run owner-owned tests or GitHub Actions without separately granted approval.

### Animation frame-rate policy — motion-specific, no universal 60 FPS requirement

- **No fixed sprite animation FPS requirement:** ECLIPSE does not require 60 distinct poses per second for every character, enemy, boss or action. Choose authored frame counts and playback cadence based on the movement, readability, visual style and production cost. Slow, restrained hovering and deliberate holds may use fewer unique poses; fast attacks may use more.
- **Rendering versus animation:** The gameplay renderer may target smooth 60 FPS on capable devices, but this is not a requirement for 60 unique sprite poses each second. Keep render FPS and authored pose rate distinct.
- **Timing:** Define each clip's duration, unique pose count, hold frames, contact/impact timing, loop behavior and effective pose rate in its metadata. Do not add artificial unique frames solely to reach a numerical FPS target.
- **Art quality:** Preserve intentional hand-authored key poses, coherent anatomy and perspective, weapon/book attachment, cape/cloth continuity, leg identity where relevant, stable palette/alpha, and clean loop closure. Avoid automatic raster translations or duplicated frames disguised as newly drawn poses; deliberate held poses are allowed when documented.
- **Existing assets:** Previously approved 24-pose knight and 32-pose White Night Executioner run cycles are not automatically deficient for being below 60 unique poses per second. Retain their approved timings and mappings unless independently revised and reviewed.
- **Gameplay invariants:** Do not change movement speed, collision, hit windows, damage, cooldowns or other mechanics merely to alter animation FPS. Keep PNG atlases, frame indices and runtime timing consistent with approved artwork.
- **Acceptance:** Inspect actual motion at intended playback speed and native pixel resolution, including loop boundaries and effects synchronization. Owner performs final game/device tests. Never run GitHub Actions without explicit approval.

## Character art: GitHub + ChatGPT Library working-copy bridge

- The **GitHub game repository main** is the authoritative runtime/source version; ChatGPT **Library `/ECLIPSE/characters/`** is a separately uploaded, versioned working copy for directly inspecting/editing PNGs.
- Read `Art/character-library/README.md` and `Art/character-library/character-asset-manifest.json` at the start of every character/sprite task.
- Retrieve matching production PNG files from Library when present, but check their SHA-256 against the intended GitHub revision first. Never substitute a similar-looking concept image or silently assume the Library mirror is synchronized.
- After approved manual pixel-art edits and PNG inspection, update the GitHub runtime PNG **and** its Library working copy with identical bytes and record the verified source revision. Keep original/editable `Art/` sources. Do not claim completion if the actual PNG or mirror is still missing.
- Library does not auto-sync with GitHub; use an explicit snapshot and integrity check. Creating an index or folders is not copying runtime PNGs. Preserve all prior prohibitions on unapproved GitHub Actions and owner-owned tests.

## Full main branch ↔ ChatGPT Library mirror (mandatory)

- The canonical ChatGPT Library mirror is **`/ECLIPSE/eclipse main/`**, with the **same relative filenames and directory layout** as `ogw07222-tech/eclipse` `main`. This supersedes the previous character-only `/ECLIPSE/characters/` workflow and the dated `/ECLIPSE/main-snapshots/...` location as the active working mirror. Historical archives may remain separately.
- **Whenever changing GitHub `main`, update the corresponding Library files within that same work session**, including additions, changes, renames and deletions. This covers code, PNGs, art sources, configuration, documentation and `AGENTS.md`, not only character assets. Prefer a single reviewed source revision; never mix arbitrary commits in the active mirror.
- Before a GitHub write, read the current GitHub `AGENTS.md`, inspect `.github/workflows/` for automatic triggers, and compare the target file against the Library counterpart. For text or image work, create the actual modified file, not a description or a claimed future sync.
- After GitHub changes, upload the **actual matching file bytes** into `/ECLIPSE/eclipse main/<repository-relative-path>` (or update/remove the corresponding Library file for renames and deletions). Verify SHA-256 where accessible and record the updated commit SHA and any unverified files. Preserve original image/source files as appropriate.
- **A GitHub commit without its Library update is a partial result**, not a synchronized completion. If file size, tool limitations, permissions or an interrupted upload block updating Library, clearly report the divergence and required manual step; never silently declare both sides synchronized.
- GitHub is authoritative for released/game assets; Library is a manually maintained mirror, not a live link. `main` commits made outside this ChatGPT work session cannot automatically change Library. At the next relevant interaction, detect the changed GitHub commit and reconcile explicitly.
- Do **not** trigger GitHub Actions or user-owned tests just to synchronize. This policy does not grant approval for GitHub Actions runs.

## Mandatory autonomous quality gate — repeat until accepted or blocked

For ECLIPSE game assets, sprites, animation, and other implementation deliverables:

1. Translate the request into concrete, verifiable acceptance criteria before production. For sprite work, include original character identity, actual transparent PNG frames, intended cell dimensions, true distinct authored poses at 60 poses per playback second, animation continuity, weapon/limb/foot contact, atlas metadata, and runtime integration where requested.
2. Produce the actual requested deliverable, not a concept sheet, illustration of a sprite sheet, placeholder, or code-drawn character substituted for authored art. Use scripts only for packing, format conversion, metadata, and static checks, never to fabricate hand-authored animation frames.
3. Inspect the result against every criterion. A generated image, successful script, frame count, or file existence is not proof of visual quality or runtime readiness.
4. On any failed criterion, revise the defective frames or rebuild the asset and verify again **without asking the owner to say “retry”**. Repeat while tools and execution budget permit. Do not present a failing candidate as final or silently lower the acceptance threshold.
5. Only label work complete after the applicable quality gates genuinely pass. Clearly distinguish static checks from owner-only runtime/device testing. Never claim a test or approval that did not occur.
6. If tool availability, access, or execution limits prevent completion, update `TASK_PROGRESS.md` with completed steps, exact modified files, failed checks, remaining work, blockers, and precise next actions so a subsequent “이어서” resumes without duplicating work.
7. Do not run GitHub Actions or owner-owned tests without the separately required explicit approval. Follow all GitHub/Library synchronization rules above.
