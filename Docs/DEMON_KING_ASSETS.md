# Demon King (final boss) — assets and runtime contract

Status 2026-11-23: **art generated and packed; runtime code written; NOT owner-verified on a device.** No GitHub Actions were run. Only the SDL-free core tests were run locally (see "Verification").

Design reference: `Art/demon-king/concept_full.png` (owner's concept sheet). Phase 1 = heavy armored knight (gold-trimmed black/violet plate, spiked pauldrons, crown helm, huge glowing greatsword). Phase 2 = armor shed, lean agile swordsman-mage (silver-lilac hair, crown horns, long thin violet sword, open left hand). The concept's oversized capes were shortened on request.

## Runtime assets (required; the castle scene fails loudly with `ECLIPSE ENEMY ASSET ERROR` if one is missing or the wrong size)

Folder: `Assets/Resources/Eclipse/monsters/demon_castle/<folder>/` (packaged by the existing `monsters/demon_castle/**/*.png` globs in `CMakeLists.txt` and `android/app/build.gradle`).

| Kind | Folder | `이동_sheet.png` | `공격_sheet.png` | `공격_vfx_layer.png` |
|---|---|---|---|---|
| `DemonKingArmored` (11) | `마왕_갑주` | 2048 x 256 (8 cells of 256) | 3584 x 256 (14 cells) | 5824 x 256 (14 x 416, fully transparent) |
| `DemonKingTrue` (12) | `마왕_본체` | 2048 x 256 | 3584 x 256 | 5824 x 256 (fully transparent) |

* Boss cells are **256 px** (`enemyCell()` in `castle_enemy.hpp`); every other enemy stays 192 px. Pivot = (128, 236), feet planted on row 236.
* The vfx layer is intentionally empty: all king effects (armor shards, void-burst telegraph, shake, flash) are drawn in code by `Presentation`, never baked into the actor art.
* Scale 2.0 (armored) and 1.8 (true form) on screen; the figures are about 100 px tall in the cell, so the pixel density matches the other castle enemies before scaling. The larger scale is a deliberate "colossal" choice and gives 2x pixels against 1.3-1.4x on the other enemies. Revisit if it looks mixed.

## How the sheets were made (honest provenance)

Characters: Pixellab `create_character` **pro** mode, 128 px, side view, from the owner's concept crops plus an existing castle enemy as style anchor. East-facing rotation only.

Motions follow the key-poses-first rule (owner request 2026-11-23): 4 key poses are generated first, reviewed, then in-betweens are generated between key pairs (start/end frames), then whole frames are selected. Prompts avoid effect words, because "no effect" phrasing made the model draw slash arcs.

* Authored/generated **key poses**, **in-betweens** and **holds** are listed per frame in `Art/demon-king/p1_armored_frames.json` and `p2_true_frames.json`. A hold repeats a whole frame deliberately (wind-up peak, impact, coil, full lunge).
* `native-engine/tools/assemble_castle_boss.py` only **selects and aligns** frames and prints the pixel change between neighbours. It draws, warps and interpolates nothing. Alignment uses the leg centroid for x and the **registered foot row** for y: in-between animations are registered to a key frame by mask cross-correlation (mismatch 0-3 %), and every frame's feet land on row 236. An earlier version used the lowest opaque pixel as the foot row, which lifted every frame whose sword tip hung below the feet (the P1 impact key by 28 source px, 56 px on screen), so the boss visibly floated; fixed 2026-11-23 and verified on all attack frames of both forms.
* Editable originals: `Art/demon-king/pixellab-p1-armored/` and `pixellab-p2-true/` (full Pixellab exports including unused tries). Concepts: `concept_*.png`.
* Unused on purpose: `inbetween_strike:1` (P1; slash arc baked in), `inbetween_lunge:*` (P2; collapsed into duplicates of the lunge key), the first one-shot 16-frame P1 attack and 8-frame P1 walk (effects baked in / barely moving).

Motion summary (pose change between neighbouring frames, from the assembler):

| Clip | Frames | Notes |
|---|---|---|
| P1 move | 4 keys + 4 in-betweens | heavy stride, 17-38 % change per step |
| P1 attack | 14 | guard, wind-up in-between, 3 keys, hold, 2 strike in-betweens (one 90 % snap into the impact), impact + hold, 4 recovery in-betweens. Contact timing .57 |
| P2 move | 4 keys + 4 in-betweens | low stalking stride, 32-49 % change |
| P2 attack | 14 | guard, 3 coil in-betweens, coil key + hold, 3 lunge keys + hold, 4 recovery in-betweens. Contact timing .55 |

Known art caveats (owner review needed): the 90 % snap frame in the P1 strike; P2 cloak is shorter than the concept but still large; recovery in-betweens show 60-70 % change because the sword swings across the body; the P1 strike has no dedicated mid-swing frame (the slash-arc frame was rejected).

## Combat contract (`combat.hpp`, `castle_enemy.hpp`)

* Both forms run on a dedicated move engine (`Combat::bossAi`, data and picker in `castle_enemy.hpp`: `BossMove`, `BossAction`, `bossStepDef`, `bossPickMove`). Each move is a list of steps; a step is one action played from `animFrom` of the 14-frame attack sheet, so one sheet serves every move. The next step starts the instant the previous one ends. The picker is a fixed rotation bent by distance, never repeating a move.
* Reaching 0 HP in phase 1 does **not** kill it: `Enemy::phaseShift` (2.8 s) starts, health holds at 1, damage and AI are ignored, hazards are cleared, then `morphKing()` turns the same body into `DemonKingTrue` with fresh health (`kingShiftSerial` / `kingPhase2Serial` let Presentation and the director react). In the true form damage stops at a 35 % floor (`kingUnbeatable`).
* New enemies get `wake` (arrival: rise out of the floor, immune and inert while waking).

### Phase 1 (`DemonKingArmored`): one heavy blow at a time, long punish windows

| Move | What it does | How to beat it | Recovery |
|---|---|---|---|
| Smash | slow overhead smash in front, x1.5 damage | step out of the crimson zone | 2.2 s |
| Quake | slam that sends two ground waves (520 px/s, 70 px high) left and right | jump the waves | 2.4 s |
| Leap slam | marks the player's spot, leaps there, area blast radius 170 | move away from the ring | 2.4 s |
| Judgement | slam while three void pillars erupt (at the player and 200 px either side, 1.25 / 1.5 / 1.75 s) | leave the marked circles | 2.6 s |
| Iron sweep | low sweep 300 px wide in front | jump it | 2.0 s |
| Juggernaut rush | brace, then a 640 px charge that hits anything in its path | dash/jump aside; he is exposed after | 3.2 s |

### Phase 2 (`DemonKingTrue`): fast chained combos, short recovery

| Combo | Steps | Notes |
|---|---|---|
| Dash combo | 3 chasing dash-slashes (last one heavier) | stays on you |
| Blink strike | teleports behind the player, 2 slashes, dash-slash | flash at both points |
| Pillar chain | backstep, 3 tracking pillars (0.6 s delay), dash-slash | magic + sword |
| Slash burst | slash, dash-slash, void burst (radius 300) | burst has a ground ring telegraph |
| Wave volley | ground-wave slashes with a backstep between | jump the waves |
| Void rain | backstep, five spread pillars, long dash-slash | wide area denial |
| Aerial dive | backflip away, leap high and dive onto the player's mark (radius 150), lunge | the dive hangs in the air before it falls |
| Execution combo | twist cut, rising cut, thrust, blink, heavy void burst | only below 60 % health (every 4th pick) |

Recoveries are 1.4-2.0 s. Hazards (`Combat::hazards`) are pillars (visible delay, then a column; hit radius 90-130, ignored above 240 px) and waves (pass under a player higher than 70 px). Damage multipliers and timings live in `bossStepDef`; every number is a placeholder for owner tuning.

Presentation (code-drawn, nothing baked into the art): crimson ground zones for melee reach, violet rings/lines for magic, pillar columns, wave crests, blink flashes, screen shake (`Combat::shake`). The Juggernaut rush and leap reuse the existing attack frames (the strike pose is held while he slides), so they look like the same sword blow travelling; dedicated rush/leap art is a future upgrade.
* Combat hooks used by the story: `scriptedSpawns`, `endOnClear`, `suppressGameOver`, `defeated`, `playerHealthFloor`, `enemyPower`, `enemyDamageScale`, `kingPower`, `kingDamageScale`, `handicap`, `spawn()`, `defeatPlayer()`, `enterForest()`. Defaults leave the plain castle combat playtest unchanged.

## Per-move body motions and separate effect layers (2026-11-23)

Every move plays its own body motion (a **20-frame** `모션_<name>_sheet.png`, 256 px cells, same pivot/feet row as the other sheets; only the legacy `공격_sheet.png` keeps 14 frames) and its own **pixelfx effect layer**. Effects are never baked into the actor art; a motion whose generated frames contained effects had those frames dropped.

| Move | Body motion (sheet) | pixelfx layer | Notes |
|---|---|---|---|
| Smash | `attack` (overhead) | `Hammer` (melee/hammer-smash) + stack above | anchored on the sword tip |
| Quake | `stab` (sword driven into floor) | `Spike` + `Spikes` (nature/earth-spike, ice/ice-spikes chained along the floor as the waves travel) | |
| Leap slam | `leap` (crouch, airborne, dive, land) | `Slam` (impacts/ground-slam) at the marked spot | he really leaves the floor |
| Judgement | `raise` (sword to the sky, pointing arm) | `Circle` (arcane/magic-circle-cast) per mark, impact frame timed to the eruption | |
| Iron sweep | `sweep` (wind-up, turn, level sweep) | `Reap` (melee/scythe-reap) | |
| Juggernaut rush | `charge` (crouch, sprint strides, skid) | `Landing` dust along the run + `Slam` crash | |
| Dash combo | `twist` -> `rise` -> `attack` (thrust) | `Spin`, `CutUp`, `Thrust` | three different body motions in a row |
| Blink strike | `twist`, `rise`, `attack` | `BlinkOut` / `BlinkIn` (the king is hidden during the first 0.16 s) | |
| Pillar chain | `backflip`, `cast` x3, `attack` | `Lightning` (lightning/lightning-strike) per pillar | |
| Slash burst | `twist`, `attack`, `burst` (gather, arms open) | `Spin`, `Thrust`, `Burst` (arcane/mana-burst) | |
| Wave volley | `twist`, `rise`, `backflip`, `twist` | `Spin`/`CutUp`, `VoidBolt` waves | |
| Void rain | `backflip`, `sky` (arms to the sky), `attack` | `Stars` (arcane/star-fall) per pillar | |
| Aerial dive | `backflip`, `dive`, `attack` | `Landing`, `Dirt` (explosions/dirt-explosion) | |
| Execution combo | `twist`, `rise`, `attack`, blink, `burst` | as above | |

### Effect stacks (round 3: effects were too thin next to the new motions)

Each move now stacks 5-14 separate layers, scheduled in `Combat::bossStartStep` so every layer's impact frame lands on the move's contact moment: **wind-up** (looping `SpellCharge` energy, `RuneCircle` under the feet, `HexSigil` above the head, `VoidOrb`), **strike** (the move's own cut layer plus a sword-tip ribbon), **shock** (`Thunderclap` ring, `DarkBlast`, `Spike`, `Dirt`, `Slam`, `HolyPillar`, `Lightning`, `Stars`), **dust and debris** (`Landing` dust ring), and a short **screen-wide `ImpactFrame`** plus a white flash (`Combat::flash`) on the heaviest hits. Floor and behind-the-king layers draw before the sprite, strikes after.

| Move | Layers (distinct pixelfx sheets) |
|---|---|
| Smash | SpellCharge, Hammer, Thunderclap, Landing, ImpactFrame |
| Quake | RuneCircle, Spike, DarkBlast, Thunderclap, Landing, ImpactFrame, Spikes (waves) |
| Leap slam | Landing, RuneCircle (landing mark), Slam, DarkBlast, Thunderclap, ImpactFrame |
| Judgement | HexSigil, RuneCircle, SpellCharge, Circle, HolyPillar, Thunderclap |
| Iron sweep | RuneCircle, Reap, Flurry, Thunderclap, Landing |
| Juggernaut rush | RuneCircle, Landing, Slam, DarkBlast, Thunderclap, ImpactFrame |
| Dash combo | Landing, Spin, Flurry, CutUp, Thrust |
| Blink strike | BlinkOut, BlinkIn, Portal, Landing, Spin, Flurry, CutUp, Thrust |
| Pillar chain | SpellCharge, Lightning, Thunderclap, Landing, Thrust |
| Slash burst | VoidOrb, SpellCharge, Burst, DarkBlast, Thunderclap, Landing, ImpactFrame, Spin, Flurry, Thrust |
| Wave volley | Flurry, Landing, VoidBolt |
| Void rain | HexSigil, SpellCharge, Stars, Thunderclap, Landing, Thrust |
| Aerial dive | Portal, Landing, RuneCircle, Dirt, DarkBlast, Thunderclap, ImpactFrame, Thrust |
| Execution combo | all of the above that its steps use (14 layers) |

Sword ribbon (`Presentation::drawSwordTrail`): reads `bossTips` (the tip pixel of every motion frame) and draws a fading three-tone streak behind the blade. It is code-drawn and separate from the actor art. Not used: pixelfx `spawn/poof` (reads as cartoon grape clouds on this palette) and `status/haste` (draws a human after-image).

Pipeline (all in `native-engine/tools/`): Pixellab key poses -> in-betweens -> `assemble_castle_boss.py` (selects and aligns frames, writes `motion_tips.json`) -> `export_boss_assets.py` (copies the pixelfx sheets to `Assets/Resources/Eclipse/boss_fx/`, writes `boss_fx_assets.hpp` and `boss_motion_data.hpp`). `boss_trace.cpp` runs the real `Combat` logic headless and dumps every frame (motion, sheet frame, effect schedule, hazards) so a preview can be rendered from the engine's own decisions.

Engine side: `BossStepDef::motion` names the sheet, `Enemy::motion` carries it to `Presentation`, `Combat::bossFx` schedules each layer so its brightest frame lands on the contact moment (`emitFx`), hazards carry their effect kind (`BossHazard::fx`). Required sheets and their sizes are checked at load and fail with `ECLIPSE ENEMY ASSET ERROR` / `ECLIPSE ASSET ERROR`.

In-between audit (2026-11-23, round 2): measuring neighbouring frames showed many held (repeated) frames (sweep 6, twist 6, cast 8) and 45-92 % pose jumps in almost every motion, which read as missing in-betweens. Fix: the motion sheets went from 14 to 20 frames (`bossMotionFrames` in `castle_enemy.hpp`; the sword-tip table, loader size check and frame mapping follow) and 30 extra in-between sets were generated for the largest jumps. Frames that came back with baked effects or a registration mismatch were not used. Every motion now has few or no exact holds, but short motions still stretch fewer distinct poses over 20 frames (distinct frames per motion: raise 11, cast 11, burst 12, sky 14, twist 16, dive 16, backflip 17, stab 19, sweep 19, rise 19, leap 20, charge 20; the rest are even holds). The biggest remaining pose jump is 90 % in the leap/dive fall: the generator will not bridge it without baking in an effect.

Motions that were generated and rejected (kept in `Art/demon-king/pixellab-*` for reference, listed in the frame specs): P1 stomp and P2 palm (poses too close to existing motions), P2 horizontal/upward cut keys (slash arcs baked in, replaced by `twist`/`rise`), several in-between frames with baked arcs or streaks.

Known motion caveats for owner review: `cast` has only four distinct poses and many holds (subtle gesture); `leap` and `dive` have a big pose jump at the start of the fall; `rise` ends airborne and snaps back to guard; small debris pixels are drawn at the sword tip in the `stab` frames; the P1 `charge` frames are all one forward-lean family. Sword-tip anchoring uses the rightmost opaque pixel of each frame, so hair or cloak that extends further can shift an effect.

## Generation budget

Owner approved 100, then +15, then +70 (185 total). Spent **182**: about 20 on the first abandoned attempts (128 px characters, effect-baked attacks), 50 on the two pro characters, the rest on key poses (4 frames each) and in-betweens (including a second round of 30 in-between sets aimed at the biggest pose jumps). About 3 remain unused.

## Verification

* Run locally: `native-engine/tests/core_tests.cpp` (SDL-free). The 9 new story/king tests pass. Of the older tests, 32 fail; 29 of those fail identically at git `HEAD`, and 3 more (`Fast tap`, `Multitouch`, `Settings vertical navigation`) fail only against the working tree. Those three exercise `ControlLayout`/`Input`, which this work did not edit; the working tree carries uncommitted owner changes to `control_layout.hpp`, but I did not bisect to confirm that is the cause.
* New engine tests (8 more this round) cover motion ownership, tip-table coverage, effect scheduling, the dive combo and motion variety; all pass. Total locally: 56/88 (the same 32 older tests fail).
* **Not run:** desktop/Android builds (SDL3 is not installed in this shell, so `main.cpp` and `presentation.hpp` changes were proofread by reading only and may contain compile errors), device play, GitHub Actions.
