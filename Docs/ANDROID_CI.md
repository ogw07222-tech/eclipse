# GitHub Actions Android APK

Workflow: [Build Android APK](https://github.com/ogw07222-tech/eclipse/actions/workflows/android-apk.yml).

Runs on manual **Run workflow** and on main pushes that change Assets, Packages, ProjectSettings or the APK workflow. It uses Unity 6000.3.0f1 from ProjectVersion.txt and GameCI's Android editor container (SDK/NDK/JDK included). No local Android SDK is required to trigger it from a tablet.

## One-time Unity license setup

Open [repository Actions secrets](https://github.com/ogw07222-tech/eclipse/settings/secrets/actions) → **New repository secret**.

| Secret | Personal license | Pro license |
| --- | --- | --- |
| UNITY_EMAIL | Unity account email | Unity account email |
| UNITY_PASSWORD | Unity account password | Unity account password |
| UNITY_LICENSE | Full contents of the activated `.ulf` license | Leave unset |
| UNITY_SERIAL | Leave unset | Unity Pro serial |

For Personal, activate a license in Unity Hub on a supported desktop and follow the current [GameCI activation guide](https://game.ci/docs/github/activation/). Typical license paths are `C:\ProgramData\Unity\Unity_lic.ulf` on Windows, `/Library/Application Support/Unity/Unity_lic.ulf` on macOS, and `~/.local/share/unity3d/Unity/Unity_lic.ulf` on Linux. Availability and validity depend on your Unity license. Unity Personal manual `.alf` activation is not assumed to work; Unity's current guidance is to activate Personal through Hub.

Add credentials only to repository Secrets. Do not commit the `.ulf`, password or serial. A green configuration check means values exist, not that Unity activation has succeeded. Choose the matching Personal or Pro method and avoid configuring both UNITY_LICENSE and UNITY_SERIAL.

If configuration is missing, the workflow stops before pulling the large Unity image, reports missing **names only**, and does not produce an APK. Secrets cannot be generated from repository code.

## Build and download from Android

1. Open **Actions → Build Android APK → Run workflow**. Select `main` and run it.
2. Wait for **Unity 6.3 - Android ARM64 APK** to finish. The first import/build can take substantially longer than later cached builds.
3. Open the successful run. Under **Artifacts**, download `ECLIPSE-Android-APK-<run number>` (retained for 14 days).
4. Extract its ZIP. Install `ECLIPSE.apk` on the tablet. Android may request permission for that app to install unknown apps.
5. Test landscape joystick/jump + attack/skill/swap, inventory, clear/defeat and retry. A successful build does not prove gameplay or multitouch correctness.

The artifact also contains `ECLIPSE.apk.sha256`. The workflow checks ZIP integrity, AndroidManifest.xml, and ARM64 libraries before upload. App ID remains `com.eclipse.combat`; target is ARM64/IL2CPP, minimum API 26. This is a development APK using Unity's development signing defaults, not a store release/AAB.

## Failure diagnosis

- **Check Unity license configuration**: add missing repository Secrets, then rerun.
- **Activation failure**: check the current Unity license and the GameCI activation guide; code cannot supply a license.
- **Disk space**: runner cleanup removes preinstalled .NET/Android/GHC toolchains because the Unity Docker container supplies the build toolchains.
- **Unity compilation/import**: inspect the Build development APK log. This is the first real engine compilation path, so source-model tests do not rule out Unity-specific failures.
- **Missing APK / missing ARM64**: build output validation deliberately fails; no empty/fake APK is uploaded.

Unity logs are preserved as a separate short-lived artifact when the action creates log files; the Actions console remains the authoritative build/activation log.

Implementation: `.github/workflows/android-apk.yml` invokes existing `Eclipse.Editor.EclipseBuild.AndroidApk`, preserving the same scene and development build options as the local Unity menu. Output is exactly `Builds/Android/ECLIPSE.apk`. Caches contain Library imports only. No publish/release upload occurs.

References: [GameCI Builder](https://game.ci/docs/github/builder/), [GameCI Activation](https://game.ci/docs/github/activation/), [Unity manual activation](https://docs.unity3d.com/6000.3/Documentation/Manual/ManualActivationGuide.html).
