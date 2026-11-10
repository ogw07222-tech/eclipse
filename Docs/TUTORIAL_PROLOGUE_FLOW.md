# ECLIPSE — Tutorial / Prologue Flow (9 stages)
Status: **approved narrative flow; design specification, NOT implemented**.
Scope: ECLIPSE game only. Do not conflate with ECLIPSE Studio.
No GitHub Actions or unattended tests/builds. Owner performs runtime validation.


## Nonverbal storytelling — binding creative direction

**ECLIPSE is a dialogue-free game.** Convey story, emotion, character motives, danger, power loss, and reincarnation through acting, pose/silhouette, visual staging, animation, camera, sound, music, lighting, and interactive gameplay. Never depend on spoken dialogue, subtitles, narration, speech bubbles, written exposition, or tutorial instruction sentences for the player to understand the plot.

### Beat-by-beat nonverbal communication
1. **Controls without dialogue:** brief contextual controller/button glyphs, directional arrows, ghosted input demonstrations, and immediate visual feedback. No tutorial text paragraphs. Let player try each action and then unlock next demonstration. Both legendary classes visibly differ in attack silhouette/effects; class change understood by visual transformation.
2. **Castle threshold:** camera reveals vast looming entrance, gate reacts to approach, protagonist briefly pauses/looks up; path lights draw attention inward.
3. **First monsters:** wake-up/arrival animation, hostile targeting, highly legible anticipation and reaction; flying and ground threat differentiated visually.
4. **Interior + mid-boss:** foes react fearfully or brace; hit-stop, knockback, breakable armor and cinematic recovery reinforce legendary power without character dialogue.
5. **Castle core:** framing, scale, throne placement, silhouettes and enemy withdrawal imply the final opponent's authority.
6. **Phase 1 victory:** boss staggers, falls or kneels; attacks cease; brief quiet lets player believe victory.
7. **Phase 2:** unnatural stillness, armored cracks, energy leaking, changed posture, throne/arena reaction, subdued transformation. Escalating attacks visually communicate an unwinnable disparity without explaining it.
8. **Death + regression:** legendary class silhouettes/effects flicker and extinguish; time reverses using repeated environmental fragments or animations; no explanatory text. Contrast previously dominant protagonist with powerless figure.
9. **Forest awakening:** matching pose/visual motif from death to rebirth, missing signature weapons/effects, weak tentative movement; an accessible visual cue introduces progression through reclaimed power.

### Readability safeguards
- Never gate progression on reading language. UI may retain essential accessibility/settings labels, save/error messages and optional control bindings; these are **interface affordances, not story dialogue**. Prefer universal glyphs and allow remapping/accessibility alternatives where needed.
- Each key narrative beat must be understandable in silhouette-only/blocking review, then reinforced by animation/sound (audio is supportive, never the only channel).
- Favor restraint and meaningful pauses over constant particle overload. Maintain the current dark, crisp, pixel-art direction.
- Do not confuse **zero dialogue** with **zero user feedback**: input, telegraphs, damage, scene transition and failures remain clearly communicated.
- This is a design specification; no cutscene, scene, UI, or art implementation is asserted by this document.

## Story and core intent
The tutorial is a playable prologue showing the protagonist's legendary peak using **섬광의 집행자 (Flash Executioner)** and **공간의 지배자 (Ruler of Space)**. The player experiences unusually powerful attacks and satisfying hit feedback, defeats Demon King phase 1, then inevitably loses in phase 2. The defeat is the first shown reincarnation, cutting to a **starting forest** where a weakened protagonist must recover lost classes and powers. The initial castle enemies are tutorial-weakened variants of intimidating **endgame** enemies, never early-game slimes.

## Canonical progression
| # | Scene | Gameplay | Exit / transition |
|---|---|---|---|
| 1 | Control practice, **no monsters** | Move, jump, dash, basic attack, skills, ultimate and swap/experience both legendary classes without combat pressure. | All essential icon/animation demonstrations experienced; allow replay/skip of visual guidance. |
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


## Flash Executioner — confirmed combat and animation specification (2026-11-10)

This section is the owner's approved design specification. It supersedes any earlier speculative names or slash-based interpretations for this class. **Design documentation only**: implementation, balancing, sprite completion, and device validation are not asserted.

| Input | Combat behavior | Required actor animation |
| --- | --- | --- |
| Basic attack / passive | Each successful basic-attack activation grants exactly one orbiting light orb, even when the same attack hits multiple enemies. Each orb increases attack speed, maximum five. No orb on a miss. | Distinct basic-attack strikes; orbiting orbs are separate visual layers, not baked into the actor. |
| Skill 1 — Rush | Rapid forward charge applying the Flash mark to struck enemies. | Sword thrust straight ahead while charging; legs trail behind with one leg bent in a conventional lunging/dashing pose, **not** a Superman flight pose. Do not finish with a sword swing. |
| Skill 2 — Command | Transform available light orbs into light lances and launch them together at randomly selected enemies. | Raise the **non-sword hand** and clench its fist to command simultaneous release. Sword does not swing. Lances are separate effect/projectile layers. |
| Skill 3 — Blink | Teleport to the **farthest Flash-marked enemy**. | Distinct departure/disappearance and arrival/recovery poses; not a slash attack. |
| Ultimate — Verdict | Drop exactly **20** light lances onto Flash-marked enemies, distributing counts as evenly as possible among eligible targets. | Raise the sword skyward; time freezes. First lances fall one by one, then a dense final rain. Preserve the raised-sword hold; do not replace it with a generic slash. |

### Deterministic implementation notes / unresolved values

- Orb gain is per successful basic-attack activation, not per target or damage tick. The precise attack-speed increment per orb, orb lifetime, and skill-2 consumption behavior beyond transformation remain to be specified.
- For 20 ultimate lances across N eligible enemies, assign each target either floor(20/N) or ceil(20/N) when N <= 20; if N > 20, choose 20 targets by a documented deterministic rule. Exact target ordering and zero-eligible-target behavior remain unresolved.
- Exact skill damage, cooldowns, Flash mark duration, invulnerability windows, and attack-frame timing must be checked against gameplay code before changing runtime behavior.
- Actor sprites, light orbs, lances, teleport visuals and impact effects remain separate assets. Author each skill from distinct key poses, not from a shared sword-swing template.
- Existing runtime clip identifiers `Slash`, `Rush`, `Command`, `Blink`, `Verdict` are implementation labels, **not** evidence that `Slash` is a separately named skill called '섬광 베기'.
- Do not run GitHub Actions or cloud builds without explicit owner permission.
