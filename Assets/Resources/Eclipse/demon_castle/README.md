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

The transparent 96x128 blue portal sprite appears only at the outgoing right edge of A1–A4. There are no visible or interactable portals at the entry side of A2–A5, and A5 has no outgoing portal. While grounded near an exit portal, the attack control shows `!`; attack or `E` advances one way: A1→A2→A3→A4→A5. A2's exit portal sits along the connected castle wall. Transitions place the player inside the next area and carry current equipment and health across.

The updated collision floors and portal anchors are authored in `native-engine/src/castle_terrain.hpp` against the current art estimates. Review alignment and route behavior in the game before treating gameplay acceptance as complete; no build, test, or GitHub Actions run was performed for this update.
