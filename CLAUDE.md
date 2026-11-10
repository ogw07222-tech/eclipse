# ECLIPSE in Claude Code

@AGENTS.md

## Start here

- Read [README.md](README.md) and check the current branch and working tree. For “이어서”, find the matching task in [TASK_PROGRESS.md](TASK_PROGRESS.md) and reconcile it with the current files and Git diff before resuming. Preserve unrelated work.
- The production game is C++17 + SDL3. Start at [native-engine/README.md](native-engine/README.md) and use the smallest relevant part of the map below.
- Implement the requested change, inspect it against the acceptance criteria, fix any failures, then repeat the applicable checks. If blocked, record the failed check, exact changed paths, blocker and next action in `TASK_PROGRESS.md`.

## Repository map

| Task | Start with |
| --- | --- |
| Controls, gameplay and combat | `native-engine/src/main.cpp`, `core.hpp`, `combat.hpp` and `control_layout.hpp` under `native-engine/src/`. For skill behavior, read [combat design](Docs/COMBAT_SKILLS_AND_STATUS_DESIGN.md). |
| Rendering, sprite playback and packaging | `native-engine/src/presentation.hpp`, `native-engine/src/motion.hpp`, `native-engine/CMakeLists.txt` and `Assets/Resources/Eclipse/`. Keep the PNG, displayed-frame mapping, playback timing and packaging aligned. |
| Android | [Android setup](native-engine/android/README.md) and `native-engine/android/app/build.gradle`. Preserve the installed app's signing identity and data. |
| Pixel art and effects | [PNG workflow](Docs/PNG_GITHUB_LIBRARY_WORKFLOW.md), [character source guide](Art/character-library/README.md), and [local Pixelbolt/PixelFX commands](Docs/PIXEL_TOOL_INTEGRATION.md). The current pose counts and acceptance criteria come from `AGENTS.md`. |

## Verification and handoff

- Start with static inspection and `git diff --check`. Report static checks separately from builds, game tests and device playback. Existing desktop and Android build commands are in their respective READMEs. Run owner-controlled tests only when separately requested.
- Inspect the effective GitHub workflow triggers before publishing changes. Each Actions run requires explicit prior owner approval for that specific run, including any run mentioned in a README. The project permission prompts cover common direct `gh` commands. The approval rule also applies when using a browser, API or another tool.
- For sprite work, verify the actual source PNG before editing. Claude Code can inspect a Git checkout and run the local tools when installed. For ChatGPT Library operations, verify access and compare the actual bytes. If access is unavailable, record any pending mirror work and its exact paths.
- In the handoff, state what changed, what was actually verified, and why work ended. Keep unfinished work and its next step explicit.
