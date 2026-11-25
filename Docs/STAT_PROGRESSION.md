# ECLIPSE — Stat progression: permanent stats + run upgrades (design + implementation status)

Status 2026-11-24: **core logic, save file, combat hookups, the memory altar and its menu are written. Static syntax check of the core only (`c++ -std=c++17 -fsyntax-only` on `core_tests.cpp`); not built, not run, not playtested, not device-tested. `main.cpp` and `presentation.hpp` could not be compile-checked here (SDL3 missing) and were only re-read.** Every number is a static estimate, not a tuned value.

## Concept
The protagonist grows by **eating memories**. Every fallen enemy leaves a memory; it streams into the protagonist as a mote of light. The memories are consumed at the **memory altar, the exact spot where the protagonist awoke after the reincarnation** (the forest start, x=140). It fits the story: after dying, the protagonist is empty and rebuilds power from what the world leaves behind. Dialogue-free: only light, motes, rings, numbers and glyph keys; no prose.

## Tutorial (first reincarnation)
After the infinite regression (see `Docs/TUTORIAL_PROLOGUE_FLOW.md`, stages 8-9) one memory survives. In the forest the protagonist eats it at the altar as the first upgrade (guided by light and glyphs), then follows a glint to the next foe, whose fall leaves another memory: memories make the protagonist stronger, little by little. The tutorial then ends; later new games skip it.

## Two layers (owner direction 2026-11-24)
| | Permanent (영구 강화) | Run (회차 강화) |
| --- | --- | --- |
| Character | broad, general-purpose stats | a **skill tree per job**, narrow and job-specific |
| Currency | **memories** | **job mastery**: points earned by playing that job |
| Lasts | forever, saved in `progress.cfg` | until the run is lost; **resets on death** and on a new game; never saved |
| Buy at | the memory altar, tab FOREVER | the memory altar, tab THIS RUN (shows the tree of the active job) |

A run ends when the player dies (GAME OVER or the scripted death) or a new game starts; a plain R restart keeps it. Menu: left/right switch tabs; keys 1-7 (or a tap) pick a permanent stat, a tap selects a tree node, a second tap or BUY (or Enter) buys it, and dragging scrolls the tree.

### Mastery
Every fallen foe gives the job in use **1 mastery xp (boss 5)**. Mastery points come from xp: point k needs `k^2 + 2k` total xp (3, 8, 15, 24, 35 ...). Jobs the player does not use earn nothing.

### The tree: real prerequisites and special effects (owner direction 2026-11-24)
The trees are **data-driven and meant to grow** (see `Docs/SKILL_TREE_GUIDE.md`): a node has a free grid position, a list of parents and how many of them are needed, a cost, numeric modifiers and special-effect procs. The first ten nodes of each job form a pyramid; nodes 10-12 add special effects. Below, the pyramid core:
Ten pyramid nodes per job in four tiers (the same shape for every job; only the meaning of the nodes differs):

```
tier 1 (1 pt)     [0]   [1]   [2]   [3]          always open
                    \  /  \  /  \  /
tier 2 (2 pts)      [4]    [5]    [6]            each needs BOTH of its two parents
                      \    |    /
tier 3 (3 pts)        [7]     [8]                each needs ANY TWO of [4] [5] [6]
                        \    /
tier 4 (5 pts)           [9]                     needs BOTH [7] and [8]
```
Parents: [4]=0+1, [5]=1+2, [6]=2+3, [7] and [8]=two of 4/5/6, [9]=7+8. The pyramid costs 21 points, the three extra nodes 12 more (33 in all, about 1,100 xp). The rule is data (`parents` and `need` per node), so a node can ask for one parent, three, or any other set.

Every tree **mixes two kinds of node**: **signature** nodes change what the job's own skills do (diamond marker in the menu) and **raw** nodes are flat stat boosts that differ per job (square marker). Raw nodes join the permanent stats only while that job is active and vanish with the run.

| Node | Space Ruler (raw: crit, move, health, haste) | Flash Executioner (raw: damage, move, haste, health) | Blade (raw: damage, defense, health, defense) |
| --- | --- | --- | --- |
| 0 | DENSE CORE: bolt damage +25 % | KEEN EDGE (raw): damage +10 % | HEAVY HAND: combo damage +25 % |
| 1 | CLEAR MIND (raw): crit +6 % | ORB SPRINT: +4 % attack speed per stored lance | BRUTE (raw): damage +10 % |
| 2 | DEEP WELL: gravity field lasts 4.5 s | SHARP LANCE: lance damage +30 % | PIERCING: thrust damage +40 % |
| 3 | LIGHT STEP (raw): move +10 % | FLEET (raw): move +12 % | THICK SKIN (raw): defense +2 |
| 4 | TWIN BOLT: a second bolt | FLOW (raw): cooldown recovery +10 % | BLOODY EDGE: third hit bleeds one stack more |
| 5 | SPIKE STORM: spikes damage +40 % | OVERFLOW: stores 2 more lances (cap 5 -> 7) | IRON BODY (raw): +40 max HP |
| 6 | DARK BODY (raw): +30 max HP | LONG RUSH: rush damage +40 % | WIDE SWEEP: sweep damage +50 %, reach +15 % |
| 7 | WIDE PULSE: repulsion radius +25 % | SKY RAIN: Verdict cooldown -33 % | SECOND WIND: heal 3 HP per kill |
| 8 | FOCUS (raw): cooldown recovery +10 % | LIGHT BODY (raw): +30 max HP | BULWARK (raw): defense +3 |
| 9 | HUNGRY DARK: singularity +50 % damage, cooldown -25 % | AFTERFLASH: blink invulnerability 0.6 s, cooldown -33 % | LAST STAND: +30 % damage below 40 % health |

Special-effect nodes (10-12, all signature nodes):

| Node | Space Ruler | Flash Executioner | Blade |
| --- | --- | --- | --- |
| 10 (needs 5 and 6, cost 3) | VOID SEED: 30 % of kills burst on nearby foes (45 damage, radius 130) | LIGHT BURST: every critical hit stores a lance | RUPTURE: critical hits apply 2 bleed stacks |
| 11 (needs 7 OR 8, cost 3) | GRAV SNAP: when hurt, push foes away (20 damage, radius 170, 8 s cooldown) | AFTERIMAGE: dashing leaves a blast (30 damage, radius 110, 1 s cooldown) | COUNTER: 35 % when hurt, a blast around you (60 damage, radius 140, 3 s cooldown) |
| 12 (needs 9 and 10, cost 6) | COLLAPSE: every 3rd skill refunds 1.5 s of cooldown | RADIANT END: kills store a lance and heal 2 | BLOODLUST: kills heal 5 and refund 1 s of cooldown |

Each node shows its name, a one-line value (`masteryNodeTag`) and its cost; a locked node shows how many parents are still missing. Links between parents and children are drawn and light up when a parent is owned. Overflow also lets the extra lances count for the +8 % attack speed per stored lance.

Code: `masteryTree()`, `MasteryNodeDef`, `Mod`, `ProcDef`, `MasteryState` and `MasteryBlock` in `progression.hpp`; the altar layout and pointer logic in `memory_menu.hpp`; the proc engine (`fireProcs`/`runProc`) in `combat.hpp`; `Combat::mastery`, `perks`, `runLost`, `beginRun()`, `buyMasteryNode()`, `menuChoose()`, `memoryTab`; the node effects are read from `perks` in the kill reward, `damageEnemy`, `damagePlayer`, `basic()`, `swordSkill()`, the Flash move code, the light lances and `stepSpace`. `StoryDirector` starts a fresh run on a new game and on the regression into the forest. Tests: 11 (the tree rules, data integrity, the special-effect engine, effect nodes through the real triggers, the menu controller, blocks per job, combat integration and reset, blade effects, raw stats per job, tabs). Raw nodes are merged into `Combat::stats` by `refreshStats()`; the signature numbers stay in `Combat::perks`. The first six run perks that were here earlier (Bleed Edge, Quick Dash, ...) were replaced by these trees.

## Where and how
- **Altar:** `memoryAltarX=140`, reach 100 units, forest scene only (`castleArea==0`), player grounded. Drawn as code-made rings, a hovering light seed and rising motes (**a placeholder cue; authored art is still to be made**). A white ring shows when the protagonist is in reach; the attack button turns into the interact "!" prompt (same mechanism as the castle exit portals); key E also works.
- **Menu:** interact opens a centred modal and **freezes the world**; interact (or I) closes it. Keys 1-7 or a tap on a row eat memories for ATK VIT GRD CRT CDM SPD HST of the active job; a tap on X or outside closes. A restart also closes it. Taps only swallow press events, so touch releases still reach the input layer.
- **Upgrading is only possible at the altar.** The inventory screen (I) is back to equipment only.

## Rules
- Seven stats, **levelled separately for each job group** (job 1 Space Ruler, 2 Flash Executioner, 3 blade). Max level 20. Swapping jobs swaps the active stat block (the menu edits the active job).
- Currency: **memories**, left by every kill: `max(1, maxHealth/40)`, bosses x5, paid once per enemy.
- Cost of level L -> L+1: `round(10 * 1.28^L)` (10, 13, 16, 21 ... ~1100 at 19 -> 20); ~4,950 memories per stat to max, ~34,600 for all seven stats of one job.
- **Prologue:** no memories (the protagonist is at their legendary peak). Drops turn on at the starting forest (`StoryDirector::beginForest`). Levels persist across game starts.
- Persistence: `progress.cfg` in the SDL pref path (`ECLIPSE_PROGRESS 1 memories N` + one `job` line each). Files written before the rename (`shards`) still load. Saved at most every 2 s while it changes and on exit; a damaged file is ignored.

## Stats (value at level 20)
| Key | Stat | Effect per level | At 20 |
| --- | --- | --- | --- |
| 1 | Attack (ATK) | +2.5 % damage on direct hits | +50 % |
| 2 | Vitality (VIT) | +5 max health | +100 |
| 3 | Guard (GRD) | flat damage reduction `(L+1)/2` | 10 |
| 4 | Precision (CRT) | +1.5 % critical chance, cap 30 % | 30 % |
| 5 | Ferocity (CDM) | critical damage 150 % + 4 % | 230 % |
| 6 | Swiftness (SPD) | +0.75 % movement speed | +15 % |
| 7 | Haste (HST) | +1 % cooldown recovery, skills and ultimates only | +20 % |

Attack and crit apply to direct hits only (not bleed/burn ticks, gravity-field ticks or the bleed execute finisher). Haste never touches dash or swap. Guard adds to equipment defense. Vitality raises current health by the same amount.

## Code map
`native-engine/src/progression.hpp` (pure logic, save/load) · `combat.hpp` (`progression`, `stats`, `dropsEnabled`, `memoryMenu`, `nearMemoryAltar()`, `memoryMotes`, `memoryFlash`, `refreshStats()`, `upgradeStat()`, hooks in `damageEnemy`/`damagePlayer`/cooldowns/swap/`restart`) · `story_director.hpp` (drops off in the prologue, on in the forest) · `main.cpp` (load/save, interact prompt, menu keys and taps) · `presentation.hpp` (altar, motes, absorb pulse, menu, `memoryMenuHit`) · `tests/core_tests.cpp` (11 progression/altar tests).

## Not done / open
Tree nodes can only be bought at the altar for now (the forest start), so a long run means walking back; buying inside the field, authored art/icons for nodes, xp and node numbers (all static estimates), whether the Space Ruler bolt needs a second twin-bolt offset, and what mastery means for a fourth job.
Authored art for the altar and memory motes (currently code-drawn), sound for eating, a controller/keyboard cursor in the menu beyond keys 1-7, cost and drop tuning against real play, what happens to the levels on a later reincarnation, the gravity-field slow (documented but not implemented), and every build, playtest and device check.
