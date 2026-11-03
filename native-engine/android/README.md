# ECLIPSE Native Android

The native APK uses C++17/SDL3, not Unity. No Unity account or license is needed.

## Download

Open [Native Android APK](https://github.com/ogw07222-tech/eclipse/actions/workflows/native-android-apk.yml), choose **Run workflow** on main, then download `ECLIPSE-Native-ARM64-<run>` from a successful run's Artifacts. Extract the ZIP and install `ECLIPSE-Native.apk` on an ARM64 Android 8+ device. App ID is `com.eclipse.nativegame`, separate from the legacy Unity build.

This is a debug-signed development APK, not a Play Store release. Ephemeral CI runners may use different debug signing keys; if Android reports an incompatible signature when replacing an earlier test build, remove that earlier development app first. This prototype has no saved progress to migrate.

## Build locally

Prerequisites: JDK17, Gradle8.11.1, Android SDK35/build-tools35.0.0, NDK27.2.12479018 and SDK CMake3.31.6. Set ANDROID_HOME to the SDK directory.

```sh
sdkmanager 'platforms;android-35' 'build-tools;35.0.0' 'ndk;27.2.12479018' 'cmake;3.31.6'
python3 native-engine/tools/prepare_android.py
gradle -p native-engine/android --no-daemon assembleDebug
python3 native-engine/tools/verify_apk.py native-engine/android/app/build/outputs/apk/debug/app-debug.apk
```

Default ABI: arm64-v8a. To build an emulator-only x86_64 APK, pass `-PeclipseAbis=x86_64`. Only the ARM64 build is uploaded as the installable device artifact. VersionCode can be set using `-PeclipseVersionCode=<integer>`.

The preparation script fetches SDL release-3.2.22's official Java bridge and C source to ignored `vendor/SDL`. CMake uses that source for the native SDL library too, so the Java/native versions match. SDL3_image3.2.4 is fetched by CMake with PNG decoding only.

Only the five referenced original PNGs are packaged. SDL3.2 Android loads assets through AssetManager with relative filenames; desktop uses executable-relative `assets/`. The application requests no network, camera, storage or recording permissions.

## CI checks

- Compile and run 20 combat/input/animation cases against the actual C++ core.
- Build the ARM64 APK; check ZIP integrity, manifest, ELF architecture, all three native libraries, exact PNG hashes and APK signature. Upload APK + SHA-256 for 14 days.
- Build a separate x86_64 emulator APK; run on an Android29 emulator and exercise startup, textures, inventory touch, equipment cycling, mage swap, background/foreground and restart. Preserve logs and synthetic app screenshots for 7 days.

Emulator checks do not replace tablet testing. Verify simultaneous joystick + attack/skill/dash, cutouts/navigation bars, Android16 lifecycle, pacing and heat on the actual device.


## Persistent signing key for release APKs

The default workflow still builds a debug-signed test APK. To build a release APK that keeps the same signing identity across builds, first create your own private PKCS12 keystore on a trusted device:

~~~sh
keytool -genkeypair -v -keystore eclipse-release.jks -storetype PKCS12 -alias eclipse-release -keyalg RSA -keysize 3072 -validity 10000
~~~

Keep the JKS file and the password in secure separate backups. Do not upload the keystore or passwords to source control or chat. In GitHub Settings > Secrets and variables > Actions, set four repository secrets: `ECLIPSE_KEYSTORE_BASE64` (the base64-encoded keystore file), `ECLIPSE_KEYSTORE_PASSWORD` (store password), `ECLIPSE_KEY_ALIAS` (`eclipse-release`), and `ECLIPSE_KEY_PASSWORD` (key password). Encode locally with `base64 -w 0 eclipse-release.jks`; transfer the output directly into GitHub Secrets, not via chat.

Choose Actions > Native Android APK > Run workflow, branch `main`, and enable `signed_release`. It builds an APK with the persistent key and validates its signer certificate. The key file is created only in an ephemeral runner directory and deleted after the signing step. Debug-signed installs cannot be upgraded directly to release-signed APKs under the same package name; uninstall the old debug app first after saving any needed data. For Google Play App Signing, distinguish your upload key from Google's managed app-signing key.
