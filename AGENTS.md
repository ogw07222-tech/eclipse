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

### Current running-animation art targets

- **Standard knight:** 24 distinct manually authored or manually corrected displayed poses per approximately 0.7-second cycle, using roughly 8 foundational key poses.
- **White Night Executioner:** 32 distinct manually authored or manually corrected displayed poses per current distance-driven run cycle, using roughly 8 foundational key poses.
- In both, arms, shoulders, weapon, pelvis, legs, cape and hair must be pose-coherent. No automatic upper-body shift script counts as a substitute for authored torso/arm/weapon animation.
- This authoring rule supersedes the earlier practice of treating `Tools/add_upper_body_followthrough.py` or other pixel-translation/procedural-bake scripts as final animation art. Existing assets are not silently deleted or changed by this policy; replace them in separately reviewed art commits.

## Animation FPS and motion continuity — ECLIPSE development principle (mandatory)

These are **sprite animation update rates**, not renderer FPS. Target gameplay rendering at 60 FPS or the device's supported stable rate; do not equate sprite pose count with screen refresh rate.

The project's baseline motion FPS guidelines are twice the previous guidelines:

| Motion category | Target sprite animation FPS |
| --- | --- |
| Idle / breathing | 8–16 FPS |
| Walking / running | 16–32 FPS |
| Standard attacks / combos | 24–48 FPS |
| Dash / fast special action | 32–60 FPS |

- These ranges are **production targets**, not absolute limits: choose per move based on its actual duration, distance, playback speed, readability, and visual continuity.
- Calculate desired distinct frames from `motion duration (seconds) × sprite FPS`; do **not** confuse the number of frames in one animation cycle with FPS.
- Current approved **standard knight run** target: **24 distinct displayed poses per approximately 0.7-second cycle at the default run speed**, equivalent to **about 34.3 pose updates/second**. This explicitly approved move-specific target takes precedence over the general 16–32 FPS running guideline. Other movement speeds may alter its real-time cycle because locomotion animation is distance-driven.
- Keep consistent character registration, foot contact and stance/swing timing, body proportions, helm/armor and sword silhouette, transparent pixels, cape continuity, and loop closure. Never increase frame count by simply duplicating/interpolating unstable silhouettes or letting limbs teleport/flicker.
- When changing motion rates or frame counts, update both the runtime's animation mapping and corresponding atlas-generation/asset metadata. Do not claim a new 24-frame PNG has been authored when runtime is only selecting 24 poses out of an existing atlas.
- Do not change gameplay movement speed, hitboxes, damage windows or cooldowns merely to achieve a sprite FPS target without specific approval.
- Document any justified exceptions beside the motion. The owner verifies rendering and tests; never run GitHub Actions without their specific prior approval.

## Character art: GitHub + ChatGPT Library working-copy bridge

- The **GitHub game repository main** is the authoritative runtime/source version; ChatGPT **Library `/ECLIPSE/characters/`** is a separately uploaded, versioned working copy for directly inspecting/editing PNGs.
- Read `Art/character-library/README.md` and `Art/character-library/character-asset-manifest.json` at the start of every character/sprite task.
- Retrieve matching production PNG files from Library when present, but check their SHA-256 against the intended GitHub revision first. Never substitute a similar-looking concept image or silently assume the Library mirror is synchronized.
- After approved manual pixel-art edits and PNG inspection, update the GitHub runtime PNG **and** its Library working copy with identical bytes and record the verified source revision. Keep original/editable `Art/` sources. Do not claim completion if the actual PNG or mirror is still missing.
- Library does not auto-sync with GitHub; use an explicit snapshot and integrity check. Creating an index or folders is not copying runtime PNGs. Preserve all prior prohibitions on unapproved GitHub Actions and owner-owned tests.
