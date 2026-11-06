# Knight skill motion — source-preserving 12-frame pass

- Original GitHub actor atlas: `Art/knight/skill/original_4x6.png` (448×564).
- Active actor atlas: `Assets/Resources/Eclipse/knight_skill_motion.png` (448×846; 4×9 cells of 112×94).
- Actor: 12 visual poses per skill. Thrust: 0.48s / 25 FPS; sweep: 0.415s / 28.92 FPS; preparation: 0.40s / 30 FPS.
- Special thrust and sweep retain **their original distinctive key poses** instead of borrowing normal attack frames. New intermediate poses use nearest-neighbor source-pixel row deformation of existing authored body, cape and sword, with a fixed foot region. These are source-derived animated variants, not 36 freshly hand-drawn human poses.
- Key actor rows are retained in `original_4x6.png`; the new visual order is 12 contiguous frames per skill at indices 0–11, 12–23 and 24–35.
- Skill sword FX remain in **independent** `knight_skill_fx_layer.png` (448×564, original 4-phase mapping); no light slash, beam or particle is baked into the actor atlas.
- Runtime and packaging read the updated actor atlas dimensions. Attack damage windows, cooldowns, collision and movement speed are unchanged.
- Rebuild manually: `python3 Tools/bake_knight_skill_12.py`.
- Layered 60Hz preview manually: `python3 Tools/render_knight_skill_mp4.py` (requires Pillow and FFmpeg).
- **Not verified in SDL game, APK or device. GitHub Actions and owner tests not run.** Review especially pose order, grounded foot contact, attached sword, and transitions.

This is a limited source-based motion pass, not a claim that every animation in the game is finalized.

## Hand-authored art acceptance hold (2026-11-06)

**Status: BLOCKED — the current game PNG is not approved artwork.**

The 12-pose `knight_skill_motion.png` was produced partly by
`Tools/bake_knight_skill_12.py`, which moves existing raster rows. Under
the current `AGENTS.md` pixel-animation rule, translated/warped art is
a **reference draft only**; it cannot be counted as newly hand-drawn
intermediate poses or declared production-ready without pixel-level redraw
and actual continuous animation review.

Prior read-only PNG inspection found the same corrupted, almost opaque
background in local frames **3, 6, 9, 10, 11** of *each* 12-frame skill
(15/36 in total). Normal cells are about 11–16% opaque; broken frames are
about 98.6–99.1% opaque. This blocks animation acceptance and hides the
independent sword-effect PNG beneath the actor.

### Approved source for manual redrawing

- Original character key poses: `Art/knight/skill/original_4x6.png`.
- Separate FX layer: `Assets/Resources/Eclipse/knight_skill_fx_layer.png`.
- Actor target: 12 *individually reviewed* poses for each of the three skills,
  4 columns × 9 rows, 112×94 pixels/cell, pivot (44,88); transparent background.
- FPS targets at existing durations: thrust **25 FPS / 0.48s**,
  sweep **28.9 FPS / 0.415s**, prep **30 FPS / 0.40s**.
- Preserve the exact hit contact times, gameplay speed, collision, damage,
  cooldowns and character design. No light beam/slash is baked onto the body.

| Frame | Special thrust (different from basic stab) | Heavy sweep (different from basic slash) | Preparation |
| --- | --- | --- | --- |
| 1 | Guard compressed, back heel bears weight | Low braced guard, sword coiled behind | Neutral |
| 2–3 | Advance the lead foot, rear-leg drive | Turn hips and shoulders against swing | Begin sword raise |
| 4–5 | Sword hand and chest launch forward | Sword clears high guard, rotate through core | Strong symmetrical brace |
| 6 | **Thrust contact**: deep lunge, blade fixed to right palm | Release begins | Sword centerline charge |
| 7 | Extension settles; cape trails the torso | **Sweep contact**: broad rotational cut | Charge held |
| 8–9 | Retract sword, front knee absorbs force | Shoulder follow-through, cape delayed | Remain braced without flicker |
| 10–11 | Rear foot recovers, shoulders unwind | Return to balanced stance | Lower sword gradually |
| 12 | Stable recovery pose | Stable recovery pose | Stable recovery pose |

Only code/utility tools may slice and pack reviewed hand-authored frames,
inventory alpha/palette, compare contact/loop timing or create GIF/MP4 for
**actual playback inspection**. No automatic frame deformation is an acceptable
shortcut for signed-off art. Run the non-destructive static check manually with
`python Tools/qa_knight_skill_art.py` when the artwork is ready; it does not
verify naturalness and must not be equated with game/device testing.

**Safety:** Existing runtime PNG is intentionally preserved by this documentation
update; this does not silently revert or fix it. No owner tests, local game build,
APK build, or GitHub Actions have been executed. Owner device acceptance remains
required.
