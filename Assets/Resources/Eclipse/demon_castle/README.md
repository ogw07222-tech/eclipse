# ECLIPSE — Demon castle fixed area artworks and portals

The current visual design uses one static side-view PNG per area (A1 through A5), not the retired entrance parallax pack.

| Area | File | Size |
|---|---|---|
| A1 Ashen Approach | `areas/a1_ashen_approach.png` | 2172x724 |
| A2 Broken Gate | `areas/a2_broken_gate.png` | 2172x724 |
| A3 Void Corridor | `areas/a3_void_corridor.png` | 2172x724 |
| A4 Throne Stairway | `areas/a4_throne_stairway.png` | 2172x724 |
| A5 Night Throne | `areas/a5_night_throne.png` | 2172x724 |

`native-engine/src/demon_castle_entrance.hpp` loads these textures and renders one at a time. Use `setArea(1..5)` for explicit transitions. The corresponding floor and portal coordinates are authored in `native-engine/src/castle_terrain.hpp`. No GitHub Actions were run.

Each area also draws the generated, transparent blue portal sprite at its authored left and right route endpoints. Portal dimensions are 96x128. While the player is grounded at a portal, the attack control shows `!`; attack or `E` interacts. Moving right goes A1→A2→A3→A4→A5, and moving left follows the reverse route. The route wraps A1-left↔A5-right to give every endpoint a paired destination. A2's left portal is placed by the broken gate. The game transition repositions the player beside the corresponding portal and carries current equipment and health across areas.

The updated collision floors and portal anchors are authored in `native-engine/src/castle_terrain.hpp` against the current art estimates. Review alignment and route behavior in the game before treating gameplay acceptance as complete; no build, test, or GitHub Actions run was performed for this update.
