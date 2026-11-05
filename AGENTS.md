# ECLIPSE — Mandatory Agent Instructions

**READ FIRST before inspecting, planning, editing, committing, opening/merging a PR, building, or testing.** This policy is set by the project owner and applies to ALL game and Studio work, including subagents and automated coding assistants.

## ABSOLUTE RULE: NO GITHUB ACTIONS WITHOUT EXPLICIT OWNER APPROVAL

- Do **not** invoke, dispatch, rerun, retry, schedule, enable, or otherwise trigger ANY GitHub Actions workflow without the owner's **explicit, prior approval for that specific run**.
- Do **not** enable or introduce automatic CI/build/test triggers, including `push`, `pull_request`, `pull_request_target`, `schedule`, `release`, `workflow_run`, `repository_dispatch`, and `workflow_call` chains. Prefer `workflow_dispatch`-only definitions, **but never invoke them without approval**.
- **Never assume** a request to implement code, fix a bug, commit, open/merge a pull request, or produce an APK constitutes approval to run GitHub Actions.
- The **owner conducts testing personally**. Do not run tests on their behalf without separately being asked. Even when testing is requested, GitHub Actions still requires explicit approval.
- Before **any push, commit, PR creation/update, or merge**, inspect the effective workflow triggers in `.github/workflows/` in both `ogw07222-tech/eclipse` and `ogw07222-tech/eclipse-studio` as applicable. If an automatic workflow could run, first disable its trigger without dispatching it; otherwise do not perform the triggering operation and explain the blocker.
- No claims of CI/test pass unless the owner supplies verified results. Static inspection and editing source files are allowed. Prefer manual/local build instructions where appropriate.
- Preserve existing signing identities and installed app data. Game and Studio are separate applications and repositories.

## Safe workflow pattern

```yaml
on:
  workflow_dispatch:
```

This is a manually available workflow, **not permission for assistants to use it**.

## Development workflow

1. Read this file and repository-specific README before working.
2. Make requested code/assets/documentation changes only, respecting separate Game and Studio boundaries.
3. Do not run GitHub Actions or user-owned tests.
4. Report what changed and explicitly mark tests/builds as **not run**.
5. Ask for approval only if a GitHub Actions run is actually necessary.

If any other instruction conflicts, follow this project owner policy and do not initiate GitHub Actions.

## Asset loading: fail loudly (mandatory)

- Required game images (including the five demon-castle entrance layers and both title layers) must never silently fall back to another scene, placeholder, or missing animation when loading/decoding, dimension checks, or texture creation fail.
- Report an explicit `ECLIPSE ASSET ERROR` with the exact file path, expected/actual dimensions when relevant, and underlying SDL error when available. Stop the affected game's startup/load with nonzero status rather than showing an unrelated scene.
- Scene-selection policy is independent of asset integrity: only make scene-specific assets fatal when that scene is actually requested. Forest startup must not terminate just because the not-yet-entered castle preview pack is absent. Report castle preload errors in logs, and show a visible modal error and fail rather than falling back if the castle scene is requested.
- Startup failures for required current-scene assets must display a user-visible error dialog (e.g., SDL_ShowSimpleMessageBox) with the file/error reason before exiting. Logging alone and instantly closing the game is not acceptable.
- When introducing a new required asset, update packaging/import rules and document its expected paths and dimensions. Do not claim runtime success without owner-run testing.

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
