# Unity migration and validation

Baseline: `main` commit `b47cc494f27258a1f1343aa0beeadee612b7b5cf` (PR #17), 2026-11-02.

## Behavior contract

| Godot source | Unity owner | Preserved behavior |
| --- | --- | --- |
| `scripts/game.gd` | `EclipseGame`, `CombatSimulation` | 1600×720 arena, floor y=585, spawn x=140, enemy x=575/800/1020/1285, horizontal camera limits, clear/defeat/retry |
| `scripts/player.gd` | `CombatSimulation` | move 280, jump -650/gravity1250, dash780/0.15s/0.85s cooldown, 0.11s coyote, 0.13s jump buffer |
| Sword combat | `CombatSimulation` | combo12/15/23, cooldown0.23/0.25/0.42, 0.8s combo window, third hit bleed, skill23/3.4s cooldown |
| Mage combat | `CombatSimulation` | basic11/0.47s, skill25/4.1s, projectiles540/440, lifetime1.6s, burn3s |
| Swap | `CombatSimulation` | 1s cooldown, entry sword13/112reach or fireball14, entry ring |
| `scripts/enemy.gd` | `CombatSimulation`, `PixelWorldView` | types55/85HP, speed95/63, damage9/15, attacks1.3/1.8s, range46, recoil135/210, bleed3 per0.75s, burn2 per0.6s, bleed consumed by fire+22 |
| `scripts/equipment.gd` | `Equipment`, `PlayerState` | same nine IDs, names and stat values, three slots, max-health adjustment on equip |
| `scripts/knight_visual.gd` | `EclipseGame.AnimateKnight` | original PNG, 48×64/8-column sequential atlas, original animation counts and priorities, 12fps, facing flip, 2× size |
| `scripts/forest_background.gd` | `PixelWorldView` | original deterministic moon/stars/3-layer trees, palette, snapped parallax and grass |
| HUD/damage/controls | `PixelHudView`, `PixelWorldView`, `TouchControl` | seven hearts, skill meter, four slots, foes, bitmap damage, same action positions, upward joystick jump |

All seven source PNGs are copied with identical SHA-256. `Tools/validate_unity_project.py` checks this every run. Original Godot code and assets remain intact under `Legacy/Godot`. The historical PNG repair workflow is archived there and replaced by read-only Unity source checks. There is no automatic dependency on Godot in the Unity version.

## Necessary adapter differences and fixes

- The source flat-floor `CharacterBody2D` behavior is implemented as a deterministic C# floor/wall solver; no general-purpose platform physics were added. This matches the current arena, not future slopes/platforms.
- `main` references absent files `assets/ui/{attack,dash,swap,skill,move_arrows}.png` and `assets/effects/sword_slash.png`. Unity directly slices the actually committed `attack_states`, `dash_states`, `swap_skill_states`, and `joystick_arrows_new` atlases. The separate sword-wave effect stays inactive, matching baseline's missing effect. Available effect PNGs remain preserved but unused.
- Attack animation receives an explicit attack serial instead of detecting only attack_flash rising from zero, so successive valid combo inputs restart their own animation reliably.
- Projectile collision sweeps its short movement segment to avoid tunnelling. All original damage/range parameters are retained.
- Ended rounds freeze combat. In the Godot source, surviving enemies could continue running behind the result overlay.
- Inventory can also open by tapping the existing HUD slots; its three equipment cycle buttons work on touch. Original keyboard mappings remain.
- Safe-area fitting, independent pointer IDs, focus/pause/restart release prevent stuck touch inputs.
- Pixel circles/lines in the mage/enemy source are represented by hard pixel blocks to follow `DESIGN_GUIDELINE_1_PIXEL.md`.
- Korean text keeps an OS-font fallback; no Korean pixel font is claimed. Final Korean typography remains pending as in baseline.

## Executed checks

- `python Tools/validate_unity_project.py`: PASS (35 unique GUIDs, Main script reference, enabled build scene, Resources paths, seven PNG hashes, point/no-mip/no-compression metadata).
- .NET SDK 8.0.427, `dotnet run --project Tools/CombatChecks/CombatChecks.csproj --configuration Release`: **15/15 PASS**, using actual `CombatSimulation.cs`, not a translated test model.
- Cases: initial wave, movement/walls, jump/landing, dash immunity/cooldown, three-hit combo/bleed, combo timeout, bleed-fire reaction, DoT, both swap entries/cooldown, mage skill/projectile, equipment/HP, damage/defeat, clear, restart, facing/vertical range.
- Temporary C# API-surface compilation is only a syntax/type-consistency aid, not Unity compilation evidence; no mock engine files are shipped.
- Original Godot code, scene, project config and PNG bytes have not been edited; files moved together to `Legacy/Godot` so the original project remains importable without Windows path-case collisions.

Two PlayMode smoke tests are provided for scene bootstrap/restart and pointer ownership/disable cleanup; they have not been executed.

## Required Unity/device checks — NOT RUN

Overall runtime/build validation status: **Blocked by unavailable Unity Editor/license/device**. Structural/model checks passed; engine parity is not yet proven.

1. Open Unity 6000.3.0f1, finish package resolution/import, inspect Console and compile all assemblies.
2. Open Main scene and Play: forest, moon, terrain, exact knight atlas alignment/transparency, left/right flip, idle/run/jump/fall/3 attacks/dash/hurt/death.
3. Exercise sword skill → swap → fire reaction; all four enemies; win, lose, R and retry button; restart repeatedly without duplicate UI/EventSystems/projectiles.
4. Test joystick+attack+swap simultaneous pointers on Android landscape; release outside buttons; background/foreground; rotate/resize; safe areas and long aspect ratios.
5. Open inventory and cycle all three slots by touch and keyboard. Verify Korean glyphs, contrast and clipping.
6. Run EditMode tests inside Unity, then build/install Android ARM64 development APK and inspect device logs/performance.
7. Build Windows and test keyboard controls.

No APK/EXE, Unity screenshot, Editor Console capture or device result exists for this migration yet.
