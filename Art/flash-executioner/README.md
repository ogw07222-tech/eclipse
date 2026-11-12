# Flash Executioner reviewed motion and light effects

This change binds only the main reviewer's approved sheets. Character appearance follows the user's `original_192.png` reference. Actor sheets and effects remain separate transparent PNGs; FX are original gold/white pixel effects authored with PixelFX, not copied game art.

| Control | Actor sheet | Gameplay duration / contact |
| --- | --- | --- |
| Attack (J/Z) | actor_basic.png | .46s / .184s, existing attack-speed scaling |
| Skill 1 (K/X) | actor_skill1.png | .50s / .125s |
| Skill 2 (U) | actor_skill2.png | .46s / .23s; sword lowered, opposite empty fist |
| Skill 3 (O) | actor_skill3.png | .36s / .15s; blank contact frame and arrival |
| Dash (Shift) | actor_dash.png | .15s |
| Swap (L/C/Tab), entering this class | actor_swap.png | .24s |
| Landing (joystick jump) | actor_land.png | .12s |

Ultimate/P, idle, run, jump and fall retain existing legacy actor art. Their new AI candidates did not pass the main review or current pose-rate target. They are **not** silently declared converted. In particular, a default jump rises for .52s (-650 velocity, 1250 gravity) and needs about31 poses; the rejected17-pose candidate is insufficient. This change does not modify physics, cooldowns, damage, target rules, marks, hitboxes or the ultimate's exactly20 lances.

All new actors have192×192 cells, pivot70/133, scale1.4. Source and runtime copies have identical PNG bytes. `reviewed/manifest.json` records dimensions, frame counts, hashes and provenance. `review-approval.json` is the main agent's static art/sequence approval, not owner device approval. Contact frames use a piecewise mapping registered to the unchanged gameplay contact time; frame counts alone do not imply a uniform60fps cadence. Movement-linked legacy art is still pending migration.

Effects include slash, rushing wake, command release, teleport departure/arrival, verdict charge, impacts, orbiting light orbs, ordinary and sky lances, dash, jump, landing and swap. Effect one-shots begin at the real action/contact event and retain their own visual lifetime instead of being truncated to the combat burst's .24s. Existing projectile count, targeting, staggered delays and collisions remain authoritative. Verdict contact/swap use their dedicated charge/swap effects rather than replaying the legacy arrival burst. A verdict collision publishes both generic and verdict burst records; the reviewed visual path deduplicates them into one large verdict impact, preserving both combat records. Generic impact focal points register to the collision center (+30 source-origin offset); ground-authored verdict impacts register to the struck target’s feet. Flight sheets cycle only the main-reviewed nonblank range3–8 at60fps until actual collision. Sky lances face down natively and use angle minus90°.

`native-engine/src/flash_executioner_assets.hpp` is generated from approved metadata by `Tools/art/import_flash_executioner.py`. The importer requires explicit approval and matching SHA256; it never packs arbitrary candidate files into runtime. Desktop CMake and Android asset packaging include `flash_executioner/*.png`. Missing/wrong-dimension declared sheets fail through `loadWhiteNight` and the existing `ECLIPSE ASSET ERROR` modal at class/game entry; unapproved roles intentionally retain their existing assets.

Editable PixelFX JavaScript/core and MIT license are in `pixelfx/source/`. Actual PixelLab source sheets, prompts/job receipts and packing script are in `pixellab/`. These are production sources, not additional runtime sheets. No duplicated/warped poses were added to inflate actor frame counts.

GitHub is the canonical versioned source. ChatGPT Library capabilities are unavailable in this session, so the Library mirror is **not synced**. A matching-byte changed-files ZIP is provided for manual import into `/ECLIPSE/eclipse main/`; do not claim bidirectional sync.

Static source/PNG metadata inspection and main agent visual review only. Game tests, build, APK, device playback and GitHub Actions were **not run**. Owner verification remains required for timing, both facing directions, skill2 hands, teleport anchoring, effect occlusion and performance.
