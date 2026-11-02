# Android native editor proof of concept

A **real native Android Jetpack Compose scaffold** on the `design/android-native-studio` PR.

- Portrait/landscape-capable Android Activity; landscape/tablet width recommended.
- High contrast rectangular controls with zero corner radius; scene list; object creation/selection; simple position changes.
- **Not yet a usable editor**: no persistence, asset import, scene canvas manipulation, SDL preview, project export or APK verification.
- Build via Android SDK + JDK17: `gradle :app:assembleDebug` inside this directory.
- This repository does **not** bundle Gradle wrapper. Android SDK/JDK/Gradle installation is required.
- Keep this independent from existing Godot and native SDL game runtime.
