# ECLIPSE Studio — stable APK signing

Android will not update an installed app when the new APK has a different signing certificate. CI previously used automatically generated debug keystores on fresh GitHub runners, which are not persistent.

## One-time GitHub repository setup
1. Generate a **new** private JKS keystore on a trusted computer (or Termux with a compatible JDK):
   ```sh
   keytool -genkeypair -v -keystore eclipse-studio.jks -alias eclipse-studio -keyalg RSA -keysize 3072 -validity 10000
   ```
2. Back up the JKS and both passwords **offline**. Losing the keystore prevents future in-place updates.
3. Base64-encode the keystore without printing the content publicly:
   ```sh
   base64 -w 0 eclipse-studio.jks > eclipse-studio.jks.base64
   ```
   (On macOS, use `base64 < eclipse-studio.jks | tr -d '\\n'`.)
4. GitHub repository → Settings → Secrets and variables → Actions → New repository secret. Configure:
   - `ECLIPSE_STUDIO_KEYSTORE_BASE64`: entire base64 keystore content
   - `ECLIPSE_STUDIO_STORE_PASSWORD`: keystore password
   - `ECLIPSE_STUDIO_KEY_ALIAS`: `eclipse-studio` (or selected alias)
   - `ECLIPSE_STUDIO_KEY_PASSWORD`: key password
5. Re-run **ECLIPSE Studio Android APK** on the main branch after merging this PR.

## Safety and limitations
- **Never** commit the keystore, passwords, or base64 data to GitHub.
- Pull request builds use ephemeral Android debug signing solely to test compilation; **do not use their APKs for regular updates**.
- Only `main`/manual builds with all four secrets configured use persistent signing.
- Existing APKs signed by an older ephemeral debug key **cannot update in place** to this new signing key. Export any important data first, then uninstall the old app *once* and install the new stable-signed APK. Future builds signed with the same JKS can update in place provided package name stays the same and versionCode does not decrease.
- Current Android editor has local JSON scenes; uninstalling the app can delete them. The app lacks a verified export/backup workflow; do not uninstall if you need to preserve those scenes.
- The workflow will deliberately fail on main if stable signing secrets are absent instead of silently creating yet another incompatible APK.
