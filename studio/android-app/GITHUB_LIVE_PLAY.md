# GitHub Live Play (experimental)

The **▶ 게임 플레이** Studio tab fetches `native-engine/preview/play.html` from the **main** branch of public `ogw07222-tech/eclipse` each time it opens or when **GitHub 새로고침** is tapped. Studio's WebView renders the HTML5 mini-game and supports on-screen move, jump, attack, skill and dash. No Android game APK build or GitHub Actions is needed to iterate on this **web prototype** after the updated Studio APK is installed once.

## Important boundaries

- This is **not the SDL3 C++ game**. The combat, sprite pipeline, map and settings are a separate browser prototype; it does not prove parity with the native build.
- Changes to `native-engine/src/` or its C++ rendering are **not** reflected automatically. Only edits to the HTML web implementation appear when refreshed.
- GitHub Raw access is public-only and requires an internet connection. Offline rendering and private repository authentication are unsupported.
- This feature requires **one Studio APK rebuild/install** because its entry point is native Kotlin; subsequent HTML/JS/CSS changes need no APK rebuild.
- Once the PR is merged to `main`, open Studio → **▶ 게임 플레이**. Retry with **GitHub 새로고침** after modifying HTML.
- The existing **▶ 플레이** in **화면 꾸미기** remains an independent scene preview for PNGs imported through the GitHub tab.
- To play or QA the **actual SDL3 game** after native C++ changes, rebuild its APK; an Android application cannot interpret updated C++ directly from GitHub without compiling native binaries.

## QA checklist

1. Build Studio APK once; install it over the current Studio build if signing certificates match.
2. Tap **▶ 게임 플레이**, verify movement, jump, attack, skill, dash, restart and **편집으로**.
3. Make a visible HTML change on main and tap **GitHub 새로고침**; check it appears without rebuilding APK.
4. Disable network and verify the loading failure is clear; re-enable and retry.
5. Independently check Android WebView compilation and real device touch controls. These checks have **not** been run by this PR.
