# ECLIPSE — Demon castle fixed area artworks (v30)

The current visual design uses one static side-view PNG per area (A1 through A5), not the retired entrance parallax pack.

| Area | File | Size |
|---|---|---|
| A1 Ashen Approach | `areas/a1_ashen_approach.png` | 900x360 |
| A2 Broken Gate | `areas/a2_broken_gate.png` | 2172x724 |
| A3 Void Corridor | `areas/a3_void_corridor.png` | 2172x724 |
| A4 Throne Stairway | `areas/a4_throne_stairway.png` | 2172x724 |
| A5 Night Throne | `areas/a5_night_throne.png` | 2172x724 |

`native-engine/src/demon_castle_entrance.hpp` loads these textures and renders one at a time. Use `setArea(1..5)` for explicit transitions. Collision is unchanged; the fixed side-scrolling level collision map must be authored and reviewed separately. No GitHub Actions were run.
