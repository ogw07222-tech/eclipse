# ECLIPSE — SDL3 Dark Fantasy Action Roguelite

ECLIPSE is a dark-fantasy 2D action roguelite built with **C++17 and SDL3**. This repository contains the production game, native engine, Android project, and game art.

## Contents

1. [Game and engine](#game-and-engine)
2. [Characters, skills, and combat](#characters-skills-and-combat)
3. [Art, animation, and levels](#art-animation-and-levels)
4. [Android and development](#android-and-development)
5. [Mandatory project rules](#mandatory-project-rules)

## Game and engine

- [Native game engine](native-engine/README.md) — combat, gameplay, SDL3 runtime, desktop build, and previews.
- [Android game](native-engine/android/README.md) — `com.eclipse.nativegame` APK project and local build instructions.

## Characters, skills, and combat

- **[Character and skill documentation index](Docs/CHARACTER_SKILL_INDEX.md)** — start here for each character's skills, design status, and art/runtime references.
- [Current class design](Docs/CURRENT_CLASS_DESIGN.md) — Void Lord / Void Mage V3 proposal (not implemented according to that document).
- [Combat skills and status design](Docs/COMBAT_SKILLS_AND_STATUS_DESIGN.md) — knight skills, bleed, and elemental/special reactions.
- [Flash Executioner motion and FX](Art/flash-executioner/README.md) — approved sheets and mapping notes.
- [Flash Executioner FX pack audit](Docs/EXECUTOR_INCANDESCENT_FX_BUTTON_AUDIT.md) — separate pack, documented but not imported.

## Art, animation, and levels

- `Assets/Resources/Eclipse/` — production game art and runtime pixel assets.
- `Art/` — editable sources and reviewed artwork.
- [Art pipeline](Docs/ART_PIPELINE_V2.md) and [pixel art guidelines](Docs/DESIGN_GUIDELINE_1_PIXEL.md).
- [Castle background pixel standard](Docs/CASTLE_BACKGROUND_PIXEL_STANDARD.md) — A1–A5 canvas, pixel-density, camera and composition rules.
- [Castle terrain status](Docs/CASTLE_FIXED_TERRAIN_STATUS.md) and [tutorial/prologue flow](Docs/TUTORIAL_PROLOGUE_FLOW.md).

## Android and development

- [Android build instructions](native-engine/android/README.md).
- [Pixel tool integration](Docs/PIXEL_TOOL_INTEGRATION.md).
- [Task progress](TASK_PROGRESS.md) — historical and ongoing task notes; verify dates before treating entries as current.

## Mandatory project rules

Read **[AGENTS.md](AGENTS.md)** before making changes. **Never run GitHub Actions without explicit approval for that particular run.** The owner performs device tests.

Keep sprite atlases, runtime animation mappings, and game resources consistent. Required assets must report explicit errors rather than silently falling back to unrelated art. Do not conflate design proposals, approved PNG assets, runtime integration, and owner-verified gameplay.
