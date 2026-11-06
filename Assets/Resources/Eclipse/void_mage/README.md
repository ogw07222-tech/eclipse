# Void Lord V3 — 64×96 layered key-pose draft

Reference: the user-provided Void Mage / Gravity Mage motion concept sheet (2026-11-06). Class rules: [CURRENT_CLASS_DESIGN.md](../../../../Docs/CURRENT_CLASS_DESIGN.md).

## Assets actually committed
- `character.png`: 27 transparent, indexed-color, native 64×96 pixel key poses; 8 columns, 4 rows.
- `grimoire.png`: 27 matching, separate floating grimoire poses, **not carried in a hand**.
- `effects.png`: 27 matching transparent 96×96 FX cells, including orb, flight particles, gravity field, spikes, radial repulsion and singularity.
- `manifest.json`: frame mapping, durations and pivots.

## Pose indices
| Clip | Frames | Key poses | Proposed duration |
|---|---|---:|---:|
| idle | 0–2 | 3 | 0.9s |
| run | 3–6 | 4 | 0.7s |
| attack | 7–10 | 4 | 0.44s |
| flight | 11–13 | 3 | 0.65s |
| gravity_field | 14–16 | 3 | 0.5s |
| dimensional_spikes | 17–19 | 3 | 0.55s |
| repulsion | 20–22 | 3 | 0.42s |
| singularity | 23–26 | 4 | 2s |

## Integration status — NOT COMPLETE
These are **first-pass visual key poses**, not a production-ready continuous 24–48 FPS animated character and **not yet registered in the SDL3 renderer or combat state**. Distinct intermediate frames, silhouette review, pivot/motion continuity, reliable playback, skill hit synchronization, flight gauge, enemy pull / push, moving gravity aura and 2-second ultimate state flow remain to be implemented and owner-reviewed.

Keep `mage_motion.png` and existing fire-mage skills unchanged. When the class is implemented, route the separate PNG layers through the game loader with exact 64×96 / 96×96 checks, nearest-neighbor filtering and explicit `ECLIPSE ASSET ERROR` on failures. The concept's push/pull must use `Enemy::knockbackResistance()` and must not cancel enemy attacks. The field follows the player, not a fixed spot on the floor. No Actions or owner tests were run.
