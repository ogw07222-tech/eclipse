# ECLIPSE — Combat Prototype v0.2

Godot 4.x / GDScript single-stage 2D roguelite combat vertical slice.

## Launch
1. Install Godot 4.3 or newer (4.x).
2. Import the folder by opening `project.godot` in Godot Project Manager.
3. Press F6/F5 (Play Project) or use the editor's Play button.

## Controls
- A/D or arrows: move
- SPACE / W / Up: jump
- SHIFT: dash (brief invulnerability)
- J or Z: basic attack
- K or X: character skill
- L or C or TAB: swap character
- R: restart stage
- Screen buttons: touch/tablet control support (landscape 16:9)

## Combat
- Sword basic attack now chains into a three-hit combo (0.8 s input window); third hit bleeds the enemy.
- Swapping forms automatically triggers a distinct entry attack with a short ring effect.
- Sword hits pause the attacker very briefly; enemies flash and recoil on hit.
- Sword form: short-range sword; skill causes bleed.
- Fire mage: ranged fireball; skill launches a stronger, larger fireball.
- Fire damage against a bleeding enemy consumes bleed and triggers +22 reaction damage.
- Defeat all four enemies to clear the stage; dying shows game over.

## Directory
- `scripts/game.gd`: scene bootstrap, arena, camera, HUD, wave, input mapping
- `scripts/player.gd`: character movement, forms, swapping, attack, skills
- `scripts/enemy.gd`: simple chasing AI, damage and status effects
- `scripts/projectile.gd`: ranged hit detection

## Next development milestones
1. Move input map to editor settings, use Input abstraction with remapping.
2. Build typed Resource definitions for character, weapon, skill and synergy; preserve stable IDs.
3. Add AnimationTree, sprite assets, sound, audiovisual hit feedback.
4. Introduce composition-based effects and Hitbox/Hurtbox nodes for combat.
5. Add curated room pool + seeded run generator + rewards and save data.
6. Write headless simulation tests, tune performance and exports.

## Status
This is a working-source prototype, not a professionally playtested build. Godot runtime was not installed in the packaging environment, so it was not executed locally. Verify Godot's first import and run on your device.

## v0.2 validation checklist
- In Godot, verify a three-hit sword combo within 0.8 s and combo reset after a pause.
- Verify swapping in either direction triggers one entry hit and observes cooldown.
- Verify enemies recoil from sword and fire hits, and that bleed + fire still works.
- Verify touch button swap/attack input and R restart.
- **Runtime testing is pending**: these changes were reviewed at source level but not executed in the Godot engine.
