# ECLIPSE — Tutorial / Prologue Flow (9 stages)
Status: **approved narrative flow; design specification, NOT implemented**.
Scope: ECLIPSE game only. Do not conflate with ECLIPSE Studio.
No GitHub Actions or unattended tests/builds. Owner performs runtime validation.

## Story and core intent
The tutorial is a playable prologue showing the protagonist's legendary peak using **섬광의 집행자 (Flash Executioner)** and **공간의 지배자 (Ruler of Space)**. The player experiences unusually powerful attacks and satisfying hit feedback, defeats Demon King phase 1, then inevitably loses in phase 2. The defeat is the first shown reincarnation, cutting to a **starting forest** where a weakened protagonist must recover lost classes and powers. The initial castle enemies are tutorial-weakened variants of intimidating **endgame** enemies, never early-game slimes.

## Canonical progression
| # | Scene | Gameplay | Exit / transition |
|---|---|---|---|
| 1 | Control practice, **no monsters** | Move, jump, dash, basic attack, skills, ultimate and swap/experience both legendary classes without combat pressure. | All essential prompts demonstrated; permit skipping/replaying help after initial prompts. |
| 2 | Demon Castle Entrance (the **currently used starting screen**) | Enter the castle; atmosphere and anticipation. This is the entrance scene, not the eventual starting forest. | Enter combat trigger. |
| 3 | Castle entrance: first enemies | Actual combat against castle fodder including ground and flying targets; easy power fantasy. | Clear encounter. |
| 4 | Castle interior | Fodder waves and a **mid-boss**; intentionally extremely easy, prioritize animation, hit-stop, impact, launches and area clearing. | Mid-boss defeated. |
| 5 | Castle core / throne approach | More fodder and final boss intro/encounter. Establish throne staging. | Boss fight begins. |
| 6 | Demon King Phase 1 | **Genuinely beatable** boss combat. Player actually depletes phase-1 HP. | Phase-1 defeat triggers phase-2 reveal. |
| 7 | Demon King Phase 2 | Playable confrontation and overwhelming transformation; **narratively unavoidable defeat**, even if player is highly skilled. Keep agency until transition; avoid softlocks if no damage is taken. | Scripted loss / death into story sequence, **not normal Game Over**. |
| 8 | Death and reincarnation | Death, silence/blackout, time-reversal motif, class/power loss. Show that death triggers a loop; brief visual echoes of both legendary classes can dissolve. | Reincarnation sequence completes. |
| 9 | Starting forest (**not yet created**) | New game loop starts with weakened baseline protagonist, class/power recovery and progression systems. | Transition into main game. |

## Encounter roster (design candidates, not yet authored/runtime integrated)
- **흑철 보병**: ground sword/shield soldier, baseline crowd target.
- **마왕군 창병**: ground reach/dash soldier, encourages repositioning.
- **성벽 가고일**: flying gargoyle, swoop then exposed landing.
- **공허 망령**: floating ranged spirit, volleys/telegraph and low health.
- **심연의 사냥개**: fast ground hound, leap and chase.
- **살아있는 갑주**: heavy armored construct, guard break and weighty stagger.
- **심연의 처형자**: elite heavy sword executioner, big readable windup; suitable mid-boss candidate.
- **타락한 천공기사**: elite flying knight, air sword techniques and ground transition.

Suggested first build set: infantry, gargoyle, hound, living armor. Remaining roster design-first. Proposed encounters, subject to balance testing:
1. Entrance: 4 infantry + 2 gargoyles.
2. Gate breakthrough: 3 spearmen + 4 hounds.
3. Interior: 2 living armor + 3 wraiths + 5 infantry.
4. Mid-boss: executioner + 4 infantry.
5. Core: flying knight + mixed fodder, then Demon King.
These numbers are placeholders and must account for simultaneous enemy/performance limits.

## Implementation constraints / acceptance
1. Keep tutorial easy through stage 6 without making endgame versions weak. Use explicit tutorial stat presets independent of production roster.
2. Flying units have bounded altitude and readable telegraphs; all playable classes need fair counterplay, particularly melee jumping/anti-air attacks.
3. Attack contact timing must line up with drawn motion and effects. Enemies retain silhouette-consistent collision. Maintain pixel-art PNG look, nearest-neighbor and coherent authored frames.
4. Preserve existing castle entrance assets; stage 9 requires a **new starting forest** scene. Current castle entrance is stage 2, not the eventual post-reincarnation spawn.
5. No silent missing-asset fallback. Follow AGENTS.md fail-loud asset loading and documentation rules when integrating new PNGs.
6. Runtime flow needs checkpoints/state transitions so scripted defeat never accidentally opens normal Game Over or restarts at stage 2; persistent story progress starts at the forest after the first reincarnation.
7. Distinguish **design mockups** from authored sprite sheets: concept sheets alone are not game-ready animation or runtime assets.
8. Phase-2 scripting must avoid deadlock for players who evade indefinitely; trigger a narratively coherent unavoidable ending without removing control immediately.
9. The project owner performs gameplay/build checks; **tests and GitHub Actions not run** as part of documenting this plan.

## Open design decisions
Exact enemy numbers, HP and cooldown tuning, mid-boss identity, stage duration, cinematic timings, player starting level/skills after reincarnation, and exact forest layout remain to be finalized.
