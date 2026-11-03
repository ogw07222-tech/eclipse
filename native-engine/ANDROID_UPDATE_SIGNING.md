# ECLIPSE — Android APK updates without reinstalling

## Invariant

All distributed, updateable Android APKs must share:

1. The same `applicationId`: `com.eclipse.nativegame`.
2. The same original, permanent release signing key/certificate.
3. An equal or higher `versionCode` than the installed APK (the workflow assigns `100000 + github.run_number`).

Android **cannot** install a differently signed APK over an existing app, regardless of package name or version number. This includes installing a newly created release key over an app installed with a transient CI debug key. This is an Android security restriction; the repository cannot bypass it. Back up user data before migrating the one time.

## One-time keystore setup (do not commit or paste secrets into chat)

Generate a long-lived key on a trusted personal computer, or Termux with Java's `keytool`. Example:

```sh
# In Termux, install JDK if necessary: pkg install openjdk-17
mkdir -p ~/eclipse-signing
chmod 700 ~/eclipse-signing
keytool -genkeypair -v \
  -keystore ~/eclipse-signing/eclipse-release.jks \
  -storetype PKCS12 -alias eclipse-release \
  -keyalg RSA -keysize 3072 -validity 10000
chmod 600 ~/eclipse-signing/eclipse-release.jks
```

Record the keystore password (and key password if different) in a password manager. Retain the `.jks` **offline, encrypted backups** in more than one safe place. If it is lost, future APKs cannot update installations signed with it. Never put the keystore or its passwords in Git, an issue, a message, or ordinary cloud storage.

Open repository **Settings → Secrets and variables → Actions → New repository secret** and configure:

| Repository secret | Value |
|---|---|
| `ECLIPSE_KEYSTORE_BASE64` | Base64 of the **binary .jks file** (Termux: `base64 -w 0 ~/eclipse-signing/eclipse-release.jks `; copy directly into GitHub Secrets, don't paste it into chat) |
| `ECLIPSE_KEYSTORE_PASSWORD` | Keystore password from keytool |
| `ECLIPSE_KEY_ALIAS` | `eclipse-release` unless a different alias was selected |
| `ECLIPSE_KEY_PASSWORD` | Key password; typically same as keystore password for PKCS12 |

**Do not regenerate this key for subsequent builds.** When configuring an existing release signing key, use it instead of generating a new one.

## Building

Open Actions → **Native Android APK** → Run workflow → Branch `main`.
Keep the default `signed_release=true`, `run_emulator=false`. The artifact is named `ECLIPSE-Updateable-ARM64-<run number>` and contains `ECLIPSE-Native-Signed.apk`.

Debug builds are still available using `signed_release=false` for isolated developer testing, but **do not use them for user installations requiring updates**. GitHub-hosted runners can sign separate debug builds with different debug certificates.

The signed job fails early if any signing secret is missing. It must never generate a new temporary release keystore. The generated version code is `100000 + github.run_number`, avoiding lower-version upgrades from the older run-number-only scheme. It is not an auto-updater: users still need to install subsequent APKs, or distribute through an app store/managed update system.

## Diagnose previous install

If installing the signed APK over an installed ECLIPSE app gives `INSTALL_FAILED_UPDATE_INCOMPATIBLE` or "App not installed", the installed app is likely signed with a different certificate. Keep or export its data first; without access to the original signing key, one uninstall/reinstall is unavoidable.

If the message is `INSTALL_FAILED_VERSION_DOWNGRADE`, inspect existing/new `versionCode`. All default signed builds from this workflow advance by GitHub run number; do not manually reset `versionCode` or move to older builds.

APK signature checking and contents validation happen before any updateable artifact is published, but the signing certificate must be protected outside the repository.

## Safe upgrade diagnosis (preserve app data)

**Do not uninstall, clear app storage, or use `adb install -r -d` as an attempted signature fix.**
The signing certificate of the installed APK must match the candidate release APK.
GitHub cannot inspect the existing device installation.

1. Download a release artifact and read `APK-UPDATE-INFO.txt` (new artifacts). It contains the new APK's package name, version code and signing certificate SHA-256.
2. If Android platform-tools are available on a connected computer, use:
   ```sh
   adb shell dumpsys package com.eclipse.nativegame | grep -E 'versionCode=|SigningDetails|signatures'
   adb shell pm path com.eclipse.nativegame
   adb install -r ECLIPSE-Native-Signed.apk
   ```
   Do not run the install command until you have backed up important data if the signing key is unknown. The installer error often distinguishes `INSTALL_FAILED_UPDATE_INCOMPATIBLE` (certificates differ) from `INSTALL_FAILED_VERSION_DOWNGRADE` (version too old).
3. To get the existing signing fingerprint, pull the installed base APK when device permissions permit:
   ```sh
   adb shell pm path com.eclipse.nativegame
   # Copy the path printed after "package:" into the command below:
   adb pull /path/from/pm/base.apk existing-eclipse.apk
   apksigner verify --print-certs existing-eclipse.apk
   apksigner verify --print-certs ECLIPSE-Native-Signed.apk
   ```
   Compare `Signer #1 certificate SHA-256 digest` exactly. If the paths are inaccessible, use another trusted APK inspection tool or extract the original APK from your own build artifact.
4. If certificates match, ensure the new `versionCode` is not lower, then install over the app without uninstalling. If they differ, find and use the original signing keystore to rebuild the update. Android does not allow bypassing this security check. If the original key is lost, a properly verified data backup/migration is required before uninstall/reinstall; native private app data may not be exportable.

### Optional release certificate pin

Once you have independently verified the SHA-256 certificate of the installed production APK, add GitHub Actions secret `ECLIPSE_EXPECTED_CERT_SHA256` with that 64-hex digest (colons accepted). Release builds then **fail** if the keystore unexpectedly changes. Do not pin an unverified new certificate as a substitute for checking the existing installation. The workflow publishes the current certificate fingerprint and version code in both the Actions job summary and `APK-UPDATE-INFO.txt`.

The pin is optional to keep existing builds functional; without it, release jobs only verify that a valid certificate exists, not that it matches previous installs.
