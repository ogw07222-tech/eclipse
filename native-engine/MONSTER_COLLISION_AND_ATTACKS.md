# Monster outline collision and distinct attacks

Monster hits now use the visible body silhouette instead of distance from the sprite origin. `Tools/art/bake_slime_shapes.py` finds the largest connected opaque component in each of the 48 atlas poses and records left/right bounds for every source pixel row. Detached droplets are decoration. The generated `slime_shapes.hpp` stays registered with the exact renderer cell (128), pivot (64,116), species scale and facing reflection.

Melee overlaps the body rows. Moving projectiles sweep their visible-radius disc against those rows, so fast shots do not tunnel and body edges can be hit before a projectile reaches the monster center. Empty corners outside the outline stay empty. Airborne hit regions follow the red monster's actual Y coordinate.

Approach/attack engagement is measured from the monster's visible front edge to the player's body edge. Damage is tested at each species' contact time, against its current front body plus a short outward reach. Health/damage and the green < red < purple order are preserved.

| Species | Attack | Duration / contact | Movement |
| --- | --- | --- | --- |
| Green | Fast low jaw snap | 0.72s / 50% | Short 18px lunge, large jaw stretch |
| Red | Forward belly leap and slam | 1.10s / 68% | Telegraph, 120px-high physical arc, land once |
| Purple | Heavy rising maw and crush | 1.16s / 61% | Tall wind-up, 30px weight transfer, broad crush |

The red slime chooses a landing point at attack start and locks direction. It does not home onto later player movement. Flight runs between 22% and 68% of the attack, then the flattened landing pose and low impact region apply damage once. Moving away or jumping above the landing region avoids it. Its recovery returns to the ground before normal pursuit resumes.

Movement pose layouts are retained. All three species have new, larger-deformation eight-frame attack PNGs; the red has crouch, takeoff, airborne, descent, squash and rebound poses. Attack metadata is in `Assets/Resources/Eclipse/monsters/slimes.json`.

```sh
python Tools/art/bake_slime_shapes.py
python Tools/art/bake_slime_shapes.py --check
```

The second command compares the generated body rows with the committed header. Core regressions cover all 48 pose outlines in both directions, empty outline corners, melee hits with centers beyond legacy reach, swept projectile hits, outer-edge engagement, red flight height and target lock, landing-only damage, jumping over the slam, one damage event, and airborne hit-region registration. Total current core tests: 48/48.

`--slime-review DIRECTORY` now simulates real AI and physics before rendering; it does not fake the red leap by merely changing a frame index. The approved in-game preview is `Art/Monsters/review/slimes_polish.gif`.
