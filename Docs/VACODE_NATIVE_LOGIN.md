# ECLIPSE — VACODE native Google login (Android)

Status: **first-stage integration, not device-validated**. This change is limited to the normal Game APK; ECLIPSE Studio is a separate application/repository.

## What is implemented

- The normal Android Game `MainActivity` (SDL3 host) displays a small **VACODE 로그인** button over the running native game while a title-screen menu is not yet implemented.
- Tapping it invokes Android **Credential Manager** and Google's native sign-in experience, using a fresh cryptographically random nonce. The SHA-256 nonce goes to Google; its raw version goes to Supabase.
- The returned Google ID token is sent over HTTPS to VACODE's existing Supabase Auth project using `POST /auth/v1/token?grant_type=id_token`, `provider=google`, `id_token`, and `nonce`. A local identity hint or unsigned token alone never authorizes an account.
- Only when Supabase returns a user UUID and issued access/refresh tokens does the app show **VACODE 연결됨 · 로그아웃**. Pressing the same button signs out locally.
- The Supabase URL, public publishable key and web client ID use NOCT's existing **public** VACODE identity configuration. The Gradle properties `vacodeSupabaseUrl`, `vacodePublishableKey`, and `vacodeGoogleWebClientId` may override them. Never add `service_role` or any private signing key to this app.

## Important limitations

1. **Session tokens are kept in memory only** and cleared on Activity teardown/sign-out. The user signs in again after app restart. Durable secure session storage and token refresh are not implemented.
2. **There is no cloud-save backend or account-based game-state synchronization yet.** Existing local save, progression, purchases and Studio are unchanged. Do not claim the login alone enables cross-device saves.
3. The button is a temporary native overlay. When the ECLIPSE pixel-art title menu ships, trigger this Android flow from its login button and remove the overlay, without copying VACODE credentials into C++ or displaying them in logs.
4. Google sign-in success on an actual Android device requires configuration of appropriate app signing fingerprints / client registration on the Google/VACODE side. The preexisting NOCT web client ID alone does not prove the ECLIPSE Android package and signing certificate have been authorized.
5. The Google provider must be enabled on the VACODE Supabase project, and its accepted audiences must include the configured web client.
6. A real Android build requires access to the Maven dependencies and the SDL vendor setup. **No build, test, APK creation, or GitHub Actions was run by the assistant.**

## Manual validation (owner only)

- Verify the installed app package is `com.eclipse.nativegame` and the app is update-signed with the existing key.
- Launch the app, press **VACODE 로그인**, choose a Google account, then verify the button changes to connected only after server confirmation.
- Cancel the account picker; verify the app remains logged out.
- Test airplane mode/offline; verify authentication fails without incorrectly asserting a valid session.
- Press **로그아웃**, then reopen and confirm this first-stage implementation does not persist sessions.
- Use a separate Android account, verify only the selected account is shown after server verification. Never inspect or print ID/access/refresh tokens in logs.
- Verify local gameplay and any preexisting saves remain unaffected.

## Next implementation milestones

1. Move login UX into the finished ECLIPSE title UI (pixel-art button) and provide a guest path.
2. Add encrypted session persistence, refresh rotation, and safe logout/revocation.
3. Add authenticated cloud save with explicit conflict policy and separated ECLIPSE game data using the VACODE user UUID.
4. Independently verify free/premium app purchase entitlement through the distribution platform, not via VACODE login alone.
