# Local SDL3 Play — P1 in progress

Priority order: (1) playable without GitHub Actions/APK rebuild per C++ change, (2) startup diagnostics, (3) editor **deferred**.

## Architecture

- Installed Studio APK contains SDL3 + SDL3_image + stable `libmain.so` bootstrap.
- Original ECLIPSE gameplay C++ (`native-engine/src/main.cpp`) is compiled as a separate `libeclipse_game.so`, also packaged as a reliable fallback.
- The bootstrap loads an app-private `files/game_updates/libeclipse_game.so` if the user explicitly imports one. Otherwise it loads the packaged game module.
- The game Activity uses its own Android process (`:game`) so reopening it may load a new version of the game module.
- This is **the actual C++ SDL3 engine**, not the earlier HTML5 prototype. Network-fetched native code is never executed automatically.

## One-time installation

Build Studio once after merging this branch, using the **manual** `ECLIPSE Studio Android APK` workflow or a local Android SDK/NDK build. Install its APK on the tablet. The existing Studio's signature must match for an in-place update; otherwise back up data and reinstall.

## Normal local development (Termux)

1. Install Termux compiler tools: `pkg install clang binutils unzip git`
2. Make the freshly built Studio APK accessible to Termux, e.g. in Downloads. The compiler links against the exact SDL3 and SDL3_image libraries that shipped in this installed Studio APK.
3. `cd ~/eclipse && git fetch origin && git switch main && git pull --ff-only` (after integration) and run:

```sh
bash studio-play.sh ~/storage/downloads/app-debug.apk
```

4. Choose `~/eclipse/studio/local-build/libeclipse_game.so` from **Studio → Termux .so 가져오기**, then **▶ 플레이 시작**. On some Android document pickers Termux home is not exposed, so copy only the built .so to `~/storage/downloads/` before selecting it:

```sh
cp ~/eclipse/studio/local-build/libeclipse_game.so ~/storage/downloads/
```

5. After changing C++, rerun `bash studio-play.sh ...` and reimport the new module. **No GitHub Actions or APK rebuild per revision** once the loader is verified.

## Error detection

- Termux compilation: Clang prints C++ filename, line, column and compiler diagnostic. An unsuccessful build does **not** replace the previously imported module.
- Studio import: validates max 40 MB and basic Android ARM64 ELF header; reports import errors.
- Native bootstrap: writes `files/studio_last_launch.txt` with `STARTED`, `RUNNING`, `ERROR`, or `EXIT <code>`. Home screen reads this after returning from the isolated game process. Linker errors include the failing shared object.
- Unexpected death may leave a stale `RUNNING` marker; this is **only a possible crash indicator**, not a reliable symbolicated stack trace.
- Native C++ line numbers on crash and automatic recovery still require Android tombstone/logcat collection and symbols; **not implemented yet**.
- Importing a native library is executing trusted local code with app privileges; **only import your own compiled modules**. The import checker is not a security sandbox or signature verifier.

## Known open QA gates

- The Termux clang toolchain and `-static-libstdc++` flag may differ by Termux release. Non-system dependencies can fail at link or load time. Do not mark as working until a real ARM64 tablet build passes.
- Verify Android linker namespace permits loading the app-private imported module on the tablet, and compare bundled module vs imported module behavior.
- Verify freshly built `libeclipse_game.so` exports `SDL_main` and needs only available Android/SDL libraries.
- Verify Compose `NativePlayActivity`, SDL lifecycle and return-to-home on real Galaxy Tab hardware.
- No native C++ code execution through Work or GitHub Actions is needed after the one-time Studio APK.
