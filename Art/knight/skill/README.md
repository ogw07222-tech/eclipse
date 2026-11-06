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
