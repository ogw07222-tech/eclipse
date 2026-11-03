# Reference knight: 64x94 PNG animation

The reference silver-armored, closed-helmet knight with a navy cape replaces the coarse procedural rig. The runtime keeps its existing 12 clip slots and 104 playable frames. Each cell is now **64x94**, in a 16-column by 12-row atlas (1024x1128). Registration is (28,88), mirrored around the same point when facing left. World scale is 1.85, so the taller canvas does not triple the character's on-screen height.

| Row | Clip | Runtime frames | Authored PNG poses |
| --- | --- | --- | --- |
| 0 | Idle | 8 | 7 |
| 1 | Walk | 8 | 7 |
| 2 | Run | 16 | 14 |
| 3 | Jump | 6 | 6 |
| 4 | Fall | 6 | 6 |
| 5 | Land | 6 | 6 |
| 6 | Dash | 8 | 8 |
| 7 | Weak slash | 8 | 8 |
| 8 | Strong overhead slash | 10 | 10 |
| 9 | Thrust | 10 | 10 |
| 10 | Hurt | 6 | 6 |
| 11 | Death | 12 | 11 |

The source has 99 distinct poses. Five repeated temporal samples preserve the existing 104-frame interface; they are not new interpolated drawings. All three attacks have distinct authored poses at every runtime frame. The weak cut's sequence was reordered so the high guard precedes contact and the down-right follow-through follows contact.

The right-hand sword is visible in locomotion, aerial, dash, hurt and death poses, as well as attacks. The three attack sequences were regenerated separately to correct missing blades and broken swings. Armor and cloth retain opaque interiors. The final PNG uses an indexed muted palette, binary transparency, no filtering, and an empty margin around each cell. Sword effects remain separate runtime geometry, registered from the new blade paths; they are not baked into the actor PNG.

## Edit and validate

Edit `Art/knight/redesign/knight_64x94_master.png` in its indexed palette, keeping the transparent color at index zero. Update `blade_paths.json` when attack wrist/tip positions change. Run:

```sh
python Tools/bake_knight_motion.py
python native-engine/tools/check_motion.py
```

The baker packs the approved PNG poses and regenerates separate trail spans. It no longer rebuilds the character from the retired 64x48 body-part rig. Old rig PNGs remain historical assets, not current animation inputs. Checks validate source/output hashes, actual pixel dimensions, all populated cells, unused transparent cells, binary alpha, uncut silhouettes, clip counts and source-frame mappings. They do not claim to prove anatomical correctness or zero foot sliding from an unrelated analytical rig.

The existing 36 core tests and the real SDL PNG renderer smoke test pass locally. The full SDL replay was also captured. APK/device installation has not been tested for this art change. Movement, attack duration, contact timing, cooldowns and damage are retained.

## Art direction

Reference-driven image generation, followed by exact pixel sizing and PNG packing: adult silver armor, closed pointed helmet, navy ragged cape, one short right-hand sword; visible sword in every pose. Weak cut: ready, high guard, hip-driven forward cut, down-right follow-through, recovery. Strong cut: raised overhead guard, loaded knees, diagonal downward cut, recovery. Thrust: chamber at hip, step/lunge, extend the short blade without stretching it, retract, ready. No character-baked slash effects or gray background matte.
