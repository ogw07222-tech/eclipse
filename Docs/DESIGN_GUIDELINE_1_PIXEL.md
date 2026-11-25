# ECLIPSE DESIGN GUIDELINE 1 — 100% Pixel Identity

Status: **mandatory for new visual work**, adopted 2026-11-01.

## Visual rules
1. Everything the player sees—characters, enemies, forest, terrain, items, HUD, buttons, inventory, profile, damage numbers, popups, and combat effects—must follow a visibly pixelated, integer-grid visual language.
2. Avoid web/mobile-app rounded-rectangle styling, anti-aliased procedural lines, blurred glows, and vector gradients. Pixel-clustered circular controls are welcome when they read as game relics, not glossy app buttons.
   - Combat HUD buttons use clipped-square black-iron plates with restrained silver bevels. Keep each job’s skill glyphs distinct; use nearest-neighbour PNGs with transparent backgrounds. Circular silhouettes are welcome when they read as black-iron game relics, not glossy mobile-app buttons.
3. Author sprites on a consistent low-resolution pixel grid, render with nearest-neighbor filtering and integer scaling where possible. Aim for a 640×360 art grid under a 1280×720 design viewport; the current live project remains 1280×720 until a separate migration is tested.
4. Use a limited palette: dark ink #101923, dark panel #1c2a34, light border #d4e0d2, gold #e3bd72; functional accents are red #813a46, blue #2b5970, purple #574879, green #3d654d.
5. Use shared `scripts/pixel_theme.gd` to style *every* new Button, PanelContainer and Label. Control artwork must use hard corners and snapped integer coordinates.
6. Damage numbers should eventually use a licensed pixel typeface, pixel outlines and integer-position frame movement. No font redistribution until font licensing is verified; default system font fallback may not visually satisfy this requirement.
7. No imitation of third-party game artwork, icons or typography. Maintain distinct ECLIPSE shapes, silhouettes, palette and effects.
8. Accessibility: target practical touch sizes, preserve text contrast, and test simultaneous joystick + actions on Android landscape.

## Current implementation boundary
- Implemented: shared square-corner style for current HUD buttons, inventory/profile panels, retry control, label shadows; rectangular pixel joystick; bitmap pixel damage popups for regular/fire/bleed hits.
- Still pending: real Korean pixel font, complete inventory item UI, integer-scale viewport migration, and owner gameplay/device validation. The combat HUD now uses authored pixel PNG frames, job-specific skill glyphs, and pixel health-bar art.
- Do not describe pending features as shipped.

## QA
Open Android landscape project; verify menu states, press/release, overlap, touch multi-input, inventory and profile, retry. Inspect nearest filtering and no rounded corners. Check scripts parse in installed Godot before merging.
