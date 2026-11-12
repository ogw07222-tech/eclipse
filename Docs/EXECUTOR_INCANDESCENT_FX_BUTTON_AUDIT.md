# Flash Executioner IncandescentFX pack — button and asset audit

Source: user-supplied `ECLIPSE_Executor_IncandescentFX_Pack.zip` (2026-11-12). **Audit only; PNG assets are NOT yet imported and runtime bindings are NOT changed.**

| Gameplay input | Runtime move | Pack actor clip | FX tracks | Status |
| --- | --- | --- | --- | --- |
| Basic attack / Attack | `WhiteMove::Slash` | `actors/executor.basic.png` | `slash`, `impact` | Match; not integrated |
| Skill 1 / Skill | `WhiteMove::Rush` | `actors/executor.skill1.png` | `rush`, `rush_hit` | Match; not integrated |
| Skill 2 / Sweep | `WhiteMove::Command` | `actors/executor.skill2.png` | `command`, `lance`, `lance_hit` | Match; not integrated |
| Skill 3 / Buff | `WhiteMove::Blink` | `actors/executor.skill3.png` | `blink_in`, `blink_out` | Match; not integrated |
| Ultimate | `WhiteMove::Verdict` | `actors/executor.ultimate.png` | `verdict`, `verdict_lance`, `verdict_hit` | Match; not integrated |

Additional pack content: `actor_light/`, `composites/`, `atlas.json`, `actors/atlas.json`, `bindings.json`, preview MP4. `bindings.json` has tick-indexed compositing data at 60 FPS; actor and FX layers must be kept separate. The pack has no general-purpose idle, walk/run, jump/fall, normal dash, or class-swap actor clips. Preserve existing assets and controls for those actions.

Runtime controls in `native-engine/src/control_layout.hpp`: `Stick`, `Attack`, `Dash`, `Swap`, `Skill`, `Sweep`, `Buff`, `Ultimate`. Keyboard mapping in `native-engine/src/main.cpp`: J/Z attack, K/X skill 1, U skill 2, O skill 3, P ultimate, Shift dash, L/C/Tab swap. `Sweep` and `Buff` are historical control identifiers, not attack types for this class.

## Integration prerequisites

1. Import all required original PNG pages plus JSON metadata without recompression/resampling, under an approved resource path; verify SHA-256 and image bounds.
2. Add resource packaging entries in `native-engine/CMakeLists.txt` for Android/desktop.
3. Add a PNG atlas and JSON timeline loader or generate a static native table from the supplied metadata. Render actor, masked actor-light, back FX, and front FX in the pack's prescribed order.
4. Trigger animations on the existing `WhiteMove` state transitions, respecting each clip's timeline and hit events; do not alter combat damage, cooldowns, or hitboxes without separate approval.
5. Verify all five buttons and all non-skill controls on-device, 60 FPS timeline, both facing directions, transparency, pivot alignment, and multi-target projectile behavior.
6. Do not run GitHub Actions without explicit permission.

**Status: mapping documented; asset upload and runtime integration remain pending.**
