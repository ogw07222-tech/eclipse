# ECLIPSE — Archived Unity combat prototype (Legacy)

Unity 6.3 LTS C# port of Godot `main` at **b47cc494f27258a1f1343aa0beeadee612b7b5cf** (PR #17).
This directory is a self-contained archived Unity project. The original Godot source and PNGs remain intact for comparison and rollback.

## Native engine development

ECLIPSE’s game-specific C++/SDL3 runtime is under [native-engine](../../native-engine/README.md). Its [scope](../../native-engine/SCOPE.md) is limited to the actual 2D game requirements. Native compilation/PNG smoke tests and [validation evidence](../../native-engine/VALIDATION.md) are separate from Unity. The native combat port and [native Android APK workflow](https://github.com/ogw07222-tech/eclipse/actions/workflows/native-android-apk.yml) use SDL/C++ without a Unity license. The historical Unity workflow is archived, not automatically runnable from GitHub Actions.

## Open and play

1. Install **Unity 6.3 LTS (6000.3.0f1)** through Unity Hub.
2. Hub → **Add project from disk** → select the `Legacy/Unity` folder (containing `Assets`, `Packages`, and `ProjectSettings`). Do not select `Assets` itself.
3. Let Unity import packages and assets.
4. Open **Assets/Scenes/Main.unity**, or use **ECLIPSE → Open Main Scene**.
5. Press **Play**. The scene constructs its own UI, forest, player and four enemies; no Inspector wiring is needed.

The 1280×720 view scales into the screen's safe area with black letterboxing. All source PNGs use point filtering, no mipmaps, no texture compression, and retain their exact original bytes.

## Controls

| Action | Keyboard | Touch |
| --- | --- | --- |
| Move | A/D or left/right | Left joystick |
| Jump | Space / W / up | Push joystick upward |
| Dash | Shift | Lightning button |
| Basic attack | J / Z | Sword button |
| Skill | K / X | Spark button |
| Swap form | L / C / Tab | Circular arrows |
| Equipment | I; 1/2/3 cycle slots | Tap HUD slots; tap equipment buttons |
| Retry | R | Result screen's retry button |

Sword three-hit combo, 0.8-second combo window, bleed finisher, mage projectiles, burn, +22 bleed/fire reaction, swap entry attacks, dash immunity, equipment stats, recoil, hit stop, damage popups, clear and defeat are ported from the baseline. Timings and pixel-space distances are retained. Touch controls have independent pointer ownership and release on focus loss/restart.

## Android tablet test

Build on a Windows/macOS/Linux computer; Unity Editor does not run on Android.

1. Add **Android Build Support**, **SDK & NDK Tools**, and **OpenJDK** to this Unity installation in Hub.
2. Open **File → Build Profiles**, choose **Android**, and switch to that platform.
3. Use **ECLIPSE → Build Android APK**. Output: `Builds/Android/ECLIPSE.apk`.
4. Copy the APK to the tablet and install it, or use Unity's Build And Run with USB debugging enabled.

Android is configured for landscape, IL2CPP, ARM64, package `com.eclipse.combat`, minimum API 26. The menu produces a **development APK**, not a signed store release. No APK has been produced in the current environment.

## Build APK with GitHub Actions

The former Unity APK workflow is archived at `workflows/android-apk.yml` and is no longer active. Re-enable it explicitly with updated project path only if needed. Download the APK from the successful run’s **Artifacts**. One-time Unity license Secrets are required; see [Docs/ANDROID_CI.md](Docs/ANDROID_CI.md). Missing configuration is reported before downloading Unity.

## Source map

- `Assets/Eclipse/Core/CombatSimulation.cs`: engine-independent fixed-step gameplay and equipment rules.
- `Assets/Eclipse/Runtime/EclipseGame.cs`: Unity scene composition, keyboard/touch adapters, atlas animation, viewport and round UI.
- `Assets/Eclipse/Runtime/PixelWorldView.cs`: native uGUI mesh drawing for the original procedural forest, enemies and effects.
- `Assets/Eclipse/Runtime/PixelHudView.cs`: original pixel hearts, cooldown meter and enemy indicator.
- `Assets/Eclipse/Runtime/TouchControl.cs`: independent pointer capture for joystick and action buttons.
- `Assets/Resources/Eclipse/`: byte-identical copies of all seven original PNGs.
- `Assets/Eclipse/Editor/EclipseBuild.cs`: desktop/Android build entry points.
- `Assets/Eclipse/Tests/EditMode/`: 15 shared gameplay scenarios and NUnit EditMode entry point.
- `Tools/CombatChecks/`: standalone .NET runner against the actual gameplay model.
- `Docs/AI/UnityProjectContext.md`: environment, architecture and evidence.
- `Docs/UNITY_MIGRATION.md`: parity mapping, deliberate fixes and remaining checks.

Original Godot entry: `../Godot/project.godot`; original instructions: [Legacy/Godot/README.md](../Godot/README.md). All original scripts, scenes, assets and tools are preserved there. This avoids case-only folder collisions on Windows.

## Validation

```sh
python Legacy/Unity/Tools/validate_unity_project.py
dotnet run --project Legacy/Unity/Tools/CombatChecks/CombatChecks.csproj --configuration Release
```

Both passed locally: 15/15 gameplay scenarios; GUIDs, scene references, resource paths and original PNG hashes checked. Historical GitHub Actions checks are archived under workflows/; native checks remain active.

In Unity: **Window → General → Test Runner → EditMode → Run All**.

**Unity Editor import/compilation, Play Mode, device multitouch, visual parity and Android/Windows builds remain unverified.** The environment had no Unity Editor/license/device. Standalone C# tests are not evidence of a successful Unity build. The Korean text uses OS font fallback because the baseline contains no licensed Korean pixel font.
