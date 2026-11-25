# ECLIPSE — Tutorial / Prologue Flow (9 stages)
Status: **approved narrative flow. Runtime code for all 9 stages was written on 2026-11-23 (see "Implementation status" at the end); it is statically reviewed and headless-core-tested only. NOT owner-verified on a device; Demon King art is pending integration.**
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
8. **Death + infinite regression (owner direction 2026-11-24):** legendary class silhouettes/effects flicker and extinguish; then the same time-reversal plays again and again, faster each time, one tally mark added per loop. Nothing is explained in text. In the last, slow loop the world thins out and **one memory of the earlier regressions stays lit** while everything else fades.
9. **Forest awakening + memory tutorial:** matching pose/visual motif from death to rebirth, missing signature weapons/effects, weak tentative movement. The surviving memory is eaten at the **memory altar on the awakening spot** (altar glow, interact button pulse, the attack row pulses in the menu); then light leads to the next foe, whose fall leaves another memory flowing into the protagonist. That implies the protagonist grows stronger by finding memories, and the tutorial ends.

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
| 8 | Death and infinite regression | Death, silence/blackout, class/power loss, then the **same regression loop repeats** (A5..A1 rewind, faster each time, tally marks), the last slow loop keeps **one memory**. | Last loop ends in a whiteout. |
| 9 | Starting forest + memory tutorial | New game loop starts with the weakened baseline protagonist. **Upgrade tutorial:** the memory that survived is eaten at the memory altar (first stat upgrade), then the next foe's memory is hinted; beating it also returns the first skill. | Tutorial ends once a memory was eaten and a foe beaten; later new games start here without it. |

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


## Canonical class-based motion asset naming (2026-11-11)

Use the character's canonical English class ID, not the animation's source name, as the asset prefix. For the Flash Executioner, the approved mapping is:

| Role | Canonical asset filename | Legacy clip |
| --- | --- | --- |
| Basic attack | `executor.basic.mp4` | `slash.mp4` |
| Skill 1 | `executor.skill1.mp4` | `rush.mp4` |
| Skill 2 | `executor.skill2.mp4` | `command.mp4` |
| Skill 3 | `executor.skill3.mp4` | `blink.mp4` |
| Ultimate | `executor.ultimate.mp4` | `verdict.mp4` |

Use the same stem for motion descriptions and skill descriptions. Preserve extension and class spelling consistently. Do not number basic attacks or ultimates as normal skills. This is an authoring/export convention; existing runtime identifiers, binary PNG paths, CMake packaging, and game logic are not renamed without an atomic migration of all references and verification. No GitHub Actions were run.


## Implementation status (2026-11-23)

Honest split between design, art, code and verification (do not conflate them):

| # | Stage | Code | Notes |
|---|---|---|---|
| 1 | Combat tutorial | `story_director.hpp` `Practice` | **Rewritten 2026-11-25: one continuous fight, no resets, no text.** Opens and ends as the Flash Executioner. 16 beats (`StoryDirector::beats`): walk to a beacon, jump, **dodge** (a soldier's wind-up stops time, chevrons show the dash path through the blow, only Dash resumes it), strike until 3 orbs, Command on reinforcements + a caster (lances mark everyone), **Blink** (the caster starts casting: time stops, a dotted line points at the farthest mark), finish the caster, **Rush** (the soldiers charge: time stops, the charge path is shown, the charge kills them), mark a wave from both sides, **Verdict** (time stops, 20 lances clear the wave), **Swap** when a gatekeeper knight rises, then as the Ruler: bolt, spikes, **repulsion** when surrounded (reach ring), field, **singularity** finale (reach ring, clears the field). Soft beats pulse the button while the fight goes on; reaction beats freeze the world (`Combat::timeScale=0`, tinted and framed) until the prompted button, which fires straight into the moment. Foes are real AI with a 30 % health floor (`Enemy::dummy`+`fights`) until the beat lets them fall; a failed reaction (hit anyway, nothing reached) brings the moment back. Skip and R (replay this beat) remain. **Not playtested.** |
| 2 | Castle threshold | `Threshold` | Path-light chevrons, a 1.8 s pause + camera look-ahead near x=1100, combat trigger at x=1480. Uses existing A1 art (the practice also runs on A1; there is no separate practice background). |
| 3 | First foes | `FirstFoes` (A1, A2) | A1: 4 infantry + 2 gargoyles. A2: 3 infantry + 4 hounds. Enemies rise from the floor (`Enemy::wake`). Weakened presets via `Combat::enemyPower/enemyDamageScale`. |
| 4 | Interior + mid-boss | `Interior` (A3, A4) | A3: 2 knights, 3 priests, 5 infantry. A4: Executioner flagged `boss` + 4 infantry. |
| 5 | Core | `Core` (A5) | Gargoyle + fodder, then the king intro (slow wake, camera reveal, 2.4 s input pause). |
| 6 | King phase 1 | `KingPhase1` | New enemy kind `DemonKingArmored` with six telegraphed one-shot moves (smash, quake waves, leap slam, judgement pillars, iron sweep, juggernaut rush). Genuinely beatable. Player has a 1 HP safety floor until phase 2. |
| 7 | King phase 2 | `KingPhase2` | At 0 HP the armor breaks (`phaseShift` 2.8 s, shards, shake, flash) and the same body becomes `DemonKingTrue` (agile; eight chained combos with their own body motions and pixelfx layers, see `Docs/DEMON_KING_ASSETS.md`). Damage stops at a 35 % health floor (`kingUnbeatable`). Safety floor is lifted; defeat is guaranteed by lethal damage, 38 s timer, or 6 s at the floor (no deadlock for evasion). |
| 8 | Death + infinite regression | `Death` | Scripted `Combat::defeatPlayer()` (never GAME OVER): death pose, class echoes flicker out with desaturation, blackout, then **5 regression loops** of the A5..A1 rewind (`StoryDirector::loopRoomSeconds` .6/.4/.25/.15 s per room, then a slow .8 s loop; ~11 s total), a tally mark per loop (`view.loopPips`), a flash and shake when a loop snaps back, and in the last loop the world dims while one memory stays lit (`view.memoryGlint`); whiteout, then forest. R does not skip it. |
| 9 | Starting forest + memory tutorial | `Forest` / `Free` | Open-air forest, blade baseline only (`Combat::handicap`: no swap, no dash, 0 skills, 50 % damage, 72 % speed), 45 HP, protagonist rises by playing the death clip in reverse. **Memory tutorial** (first reincarnation only, `memoryTutorialDone` saved in `story.cfg` v2): one memory is granted, the altar glows and the interact button pulses (`view.altarGlow`, `view.highlight`), the menu pulses the attack row (`view.menuRow`), the menu closes after the first upgrade, then a glint marks the nearest foe (`view.hintX`). The tutorial ends (`Free`) once a memory was eaten and a foe beaten; the first kill also returns Skill 1 (ring cue). `story.cfg` (`ECLIPSE_STORY 2 reincarnated 1 memory 1`; v1 files count as already played) makes later new games start in the forest. Details: `Docs/STAT_PROGRESSION.md`. |

2026-11-24 flow change: the death is now an infinite regression and the forest teaches eating memories at the altar (written, statically syntax-checked only; not built, run, playtested or device-tested; the loop timings are untuned and the visuals are code-drawn placeholders).

2026-11-24 review: R can no longer revive the scripted death, R in the forest keeps the weakened baseline, finishing practice returns the player to the start, and the tutorial gatekeeper knight is eased (x0.65 health). Balance figures are static estimates; not playtested or build-tested.

Not done / open: separate practice background, training dummy for Flash marks, replay-practice control (only skip + R), real forest layout beyond the existing forest art, skill 2-4 recovery progression, audio cues, tuning of every number, and all owner device tests. R inside the tutorial replays only the current beat.

Art: the Demon King is a new pair of sheets (see Docs/DEMON_KING_ASSETS.md). Until they are imported the loader reports `ECLIPSE ENEMY ASSET ERROR` if the king is requested; Presentation currently needs the sheets in `Assets/Resources/Eclipse/monsters/demon_castle/마왕_갑주` and `마왕_본체`.
