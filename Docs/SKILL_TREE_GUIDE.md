# ECLIPSE — Growing the job skill trees (guide)

Status 2026-11-24: the data model, special-effect engine and altar menu are written and syntax-checked only (`c++ -std=c++17 -fsyntax-only tests/core_tests.cpp`). Not built, not run, not playtested, not device-tested. `presentation.hpp` and `main.cpp` could not be compile-checked here (SDL3 missing).

The tree is **data**. Everything below happens in `native-engine/src/progression.hpp` unless noted.

## 1. Add a node
Append one line to the job's list in `masteryTree(job)` (job 1 Space Ruler, 2 Flash Executioner, 3 Blade). A node's index is its position in the list, and parents must come **earlier** in the list.
```cpp
masteryNode("NAME","TAG",raw,cost,x,y,{parents},need,{{Mod::X,value}},{masteryProc(...)},"ONE LINE DESCRIPTION")
```
| Field | Meaning |
| --- | --- |
| `name` | at most 11 characters (it must fit the 108 px node) |
| `tag` | value shown under the name, at most 16 characters |
| `raw` | `true` = flat stat node (square marker), `false` = skill-changing or special node (diamond marker) |
| `cost` | mastery points |
| `x`, `y` | free grid position, 1 unit = one node column/row, `x=0` is the centre; nodes may sit anywhere, the view scrolls |
| `{parents}`, `need` | up to 4 parents; `need` of them must be owned (0 for a root node). `need` 1 = "any one", `need` = count = "all" |
| mods | up to 4 `{Mod, value}` pairs |
| procs | up to 2 special effects (section 3) |
| description | at most 44 characters, shown in the detail box |

A test (`Mastery data stays valid however large the trees grow`) checks name/tag/description lengths, that parents exist and come earlier (no cycles), that `need` never exceeds the parents, that no two nodes share a spot and that every node does something. The per-job limit is `maxMasteryNodes` (64); raise it if a tree needs more.

## 2. Add a modifier (a new numeric effect)
A `Mod` changes a number the combat code reads. To add one: (1) add it to `enum class Mod`; (2) add the field to `MasteryBlock` and one `case` in `MasteryBlock::apply` (multipliers multiply, flags set, the rest add); (3) read the field where the skill is resolved in `combat.hpp` (search for `perks.` to see the existing ones). Raw stats (`RawDamage`, `RawHealth`, `RawDefense`, `RawCrit`, `RawMove`, `RawHaste`) are folded into the permanent stats by `Combat::refreshStats()`, so they need no extra hook.

## 3. Special effects (procs)
`masteryProc(trigger, action, value, chance, radius, cooldown, every)`:
| Trigger | Fires when |
| --- | --- |
| `OnKill` | a foe falls (once per foe) |
| `OnCrit` | a direct hit was a critical hit |
| `OnDash` | the player dashes |
| `OnDamaged` | the player is hurt and survives |
| `OnSkill` | the player starts a skill or ultimate (not a basic attack) |

| Action | Does |
| --- | --- |
| `Explode` | `value` damage to foes within `radius` of the target (of the player if there is none) |
| `Pulse` | `value` damage and a knockback to foes within `radius` of the player |
| `Bleed` | `value` bleed stacks on the target |
| `AddLance` | `value` stored light lances (Flash Executioner, capped) |
| `Heal` | `value` HP |
| `RefundCooldown` | `value` seconds off every skill and ultimate cooldown |

`chance` 0..1 per trigger, `cooldown` seconds before it can fire again, `every` N fires on every Nth trigger. A proc never triggers another proc (`inProc` guard), and blast/pulse damage is applied as damage-over-time style hits so it does not crit or scale with the Attack stat. To add a trigger: add it to `enum class Trigger` and call `fireProcs(Trigger::X, target)` at the right spot in `combat.hpp`. To add an action: add it to `enum class ProcAction` and one `case` in `Combat::runProc`. Visual cues for blasts and pulses are in `Combat::procCues`, drawn by `Presentation::drawProcCues` (code-drawn placeholder rings; real effect art is still to be made).

## 4. The altar menu with a big tree
`native-engine/src/memory_menu.hpp` holds the layout and the pointer logic (no SDL, so it is unit-tested): drag in the tree to scroll (clamped to the tree), tap a node to select it, tap it again or press BUY (or Enter) to buy it. The detail box shows the selected node's name, value, description, cost or the number of parents still missing. Linked parents are drawn as dotted lines that light up when owned; buyable nodes pulse.

## 5. What a run is
A run ends when the player dies or a new game starts; the whole tree, the mastery xp and the active special effects reset. Permanent stats and memories stay. Mastery xp: 1 per fallen foe (boss 5) for the job in use; point k costs `k^2 + 2k` xp in total.

## 6. Current trees
Each job has 13 nodes: ten in the pyramid (4 / 3 / 2 / 1, nodes 0-9) and three extras (10-12) with special effects. See `Docs/STAT_PROGRESSION.md` for every node. All numbers are static estimates.
