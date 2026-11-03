# ECLIPSE Studio — original SDL3 gameplay first

The Studio APK now packages **the same** `native-engine/src/main.cpp` C++ entry point, SDL3 3.2.22, SDL3_image 3.2.4, and native PNG assets as the standalone native Android game. The temporary launcher has one **▶ 플레이** control, which starts `GameActivity : SDLActivity`. The previous Compose scene editor source is preserved but is **not launched**; editing is postponed.

## Build once for the tablet

From repository root, with JDK 17, Android SDK 35, Android NDK 27.2.12479018, CMake 3.31.6 and Gradle 8.11.1 installed:

```sh
python3 native-engine/tools/prepare_android.py
gradle -p studio/android-app --no-daemon :app:assembleDebug
python3 native-engine/tools/verify_apk.py studio/android-app/app/build/outputs/apk/debug/app-debug.apk
```

Or manually trigger **Actions → ECLIPSE Studio Android APK → Run workflow**, choosing the branch containing this change. This workflow is manual-only, to avoid spending Actions minutes whenever source files change.

Install the built Studio APK once. Launch **ECLIPSE Studio → ▶ 플레이**. The actual C++ SDL game runs in a fullscreen landscape Activity. Android Back/closing SDL returns to the launcher (device behavior still requires hands-on QA).

## Scope / limitations

- **Original C++ SDL3 gameplay** (not the HTML5 proxy). PNG assets in the repository are bundled into the APK.
- This initial change does **not** implement hot-reload of C++ or GitHub-hosted native code. Any change to C++ or bundled PNG currently requires rebuilding/reinstalling the *Studio* APK. Future asset-only sync can avoid APK rebuilds after changing the asset loader.
- GitHub is connected as the code source for builds; the app does **not** dynamically execute arbitrary C++ from the network.
- This PR needs Android CI compilation and real tablet testing before it is considered verified.
- Signing configuration `ECLIPSE_STUDIO_*` is retained. If the installed Studio APK uses a different certificate, Android requires reinstall rather than an in-place update.
