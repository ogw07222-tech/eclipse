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

### Current running-animation art targets — migrate to 60 FPS

- **All character and enemy actions:** produce a true **60 distinct sprite poses per second of animation playback** at the authored default timing, including idle, walk, run, jump, fall, dash, attacks, combos, skills, ultimate, recoil, death, and other animated actions.
- **Standard knight run:** previously approved 24 poses per approximately 0.7-second cycle; **new target is 42 distinct displayed poses per approximately 0.7-second cycle** (60 FPS). Preserve run duration and locomotion distance. Legacy 24-frame assets remain legacy until separately redrawn, integrated and reviewed; do not label them 60-FPS complete.
- **White Night Executioner run:** previous 32-pose distance-driven cycle is legacy. Derive the new count from the actual cycle duration at its reference/default movement speed: `round(cycle_seconds × 60)`; record that duration and updated frame count before art production. Preserve its original run speed/distance and do not assume 32 poses already constitute 60 FPS.
- In every pose, arms, shoulders, weapon, pelvis, legs, cape and hair must be coherent, and left/right contact must alternate. Do not use automatic upper-body shifts, image translations or duplicated sprites as a substitute for authored animation.
- Existing sprite sheets and runtime mappings are **not automatically converted** by this instruction update. Track each unfinished migration explicitly until the updated PNG, metadata and runtime mapping have been reviewed and committed.

## Sprite animation FPS and motion continuity — universal 60 FPS standard (mandatory)

**Every sprite-sheet action animation for ECLIPSE playable characters, enemies and bosses must target 60 distinct displayed poses per second at its authored playback duration.** This replaces all older 8–60 FPS or move-specific lower-FPS recommendations. Gameplay rendering also targets 60 FPS on a capable device, but render FPS and sprite animation FPS remain different metrics.

| Motion category | Target sprite animation FPS |
| --- | --- |
| Idle / breathing / ambient character motion | 60 FPS |
| Walking / running / jumping / falling | 60 FPS |
| Standard attacks / combos / recovery | 60 FPS |
| Dash / movement skills / special action | 60 FPS |
| Skills / ultimates / transformations | 60 FPS |
| Enemy / boss attacks, idle, transitions and death | 60 FPS |

1. **Frame planning:** `target distinct displayed frames = round(actual action duration in seconds × 60)`. Example: a 0.7-second running cycle requires 42 unique displayed poses. One 60-frame cycle is 60 FPS only if it plays in 1.0 second; 24 frames over 0.7 seconds are approximately 34.3 pose updates/second, **not** 60 FPS.
2. **Original art:** Draw/manual-correct real intermediate pixel poses using coherent hand-authored keys. Do not pad the count by duplicating frames, repeating earlier poses, copying/warping/scaling raster pieces or using automatic interpolation as final artwork. Source reference rigs/motion capture may guide manual pose drawing but are not proof of approved pixel art.
3. **Continuity:** Check feet and leg identity, grounded contact, center of mass, pelvis/torso/shoulder follow-through, attached hands/weapons, cape/hair drag, alpha/palette stability, and first-to-last loop closure at native scale and 60-FPS playback. Rapid attacks must keep anticipation, impact and recovery readable; use timing/pose design rather than changing gameplay mechanics.
4. **Runtime and atlas:** Keep PNG sheet dimensions, frame counts, row/column indexes, playback duration, renderer sprite selection, and any distance-driven locomotion timing consistent. A 60-FPS renderer that displays fewer unique images is **not** a 60-FPS sprite-sheet animation. Record authored pose count and effective pose rate per action.
5. **Gameplay invariance:** Do not alter movement speed, distance, jump physics, collision boxes, damage/hit windows, knockback, cooldowns or skill balance merely to satisfy 60 FPS. Preserve existing action durations unless a separate owner request approves changing one.
6. **Migration and acceptance:** Apply the rule to all **new and revised** animation sheets immediately, and migrate existing sheets individually. Mark older files as **pending 60-FPS conversion** rather than claiming they already meet the standard. Completion requires real approved art PNGs, matching game runtime metadata/mapping, visual motion review, and explicit owner/device verification. The owner performs tests. **Never run GitHub Actions without specific prior owner approval.**
7. **Exceptions:** Do not lower the target for stylistic reasons without the owner's explicit approval. If platform constraints require dropped or held poses at runtime, document the actual effective FPS as a performance exception, not as a 60-FPS authored animation.

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
