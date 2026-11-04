plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.compose")
}

val repositoryRoot = rootProject.projectDir.resolve("../..").canonicalFile
val nativeRoot = repositoryRoot.resolve("native-engine")
val sdlSource = nativeRoot.resolve("android/vendor/SDL")
val sdlJava = sdlSource.resolve("android-project/app/src/main/java")
if (!sdlJava.resolve("org/libsdl/app/SDLActivity.java").isFile) {
    throw GradleException("SDL3 Java bridge missing: run python3 native-engine/tools/prepare_android.py first")
}
val gameArt = repositoryRoot.resolve("Assets/Resources/Eclipse")
val packagedArt = layout.buildDirectory.dir("generated/eclipseAssets")
val packageEclipseAssets by tasks.registering(Sync::class) {
    from(gameArt) {
        include("knight_motion.png", "ui/attack_states.png", "ui/dash_states.png",
            "ui/swap_skill_states.png", "ui/joystick_arrows_new.png",
            "ui/settings_gear.png", "ui/settings_menu_panel.png", "ui/settings_window.png",
            "forest/**/*.png", "forest_v2/**/*.png", "forest_v3/**/*.png")
    }
    into(packagedArt)
}

android {
    namespace = "dev.eclipse.studio"
    compileSdk = 35
    ndkVersion = "27.2.12479018"
    defaultConfig {
        applicationId = "dev.eclipse.studio"
        minSdk = 26
        targetSdk = 35
        versionCode = 2
        versionName = "0.2-native-play"
        ndk { abiFilters += listOf("arm64-v8a") }
        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_static",
                    "-DECLIPSE_TESTS=OFF",
                    "-DECLIPSE_HEADLESS=OFF",
                    "-DFETCHCONTENT_SOURCE_DIR_SDL3=${sdlSource.absolutePath}",
                    "-DCMAKE_BUILD_TYPE=Release"
                )
                targets += listOf("main", "SDL3-shared", "SDL3_image-shared")
            }
        }
    }
    val storePath = System.getenv("ECLIPSE_STUDIO_KEYSTORE")
    val storePass = System.getenv("ECLIPSE_STUDIO_STORE_PASSWORD")
    val keyAlias = System.getenv("ECLIPSE_STUDIO_KEY_ALIAS")
    val keyPass = System.getenv("ECLIPSE_STUDIO_KEY_PASSWORD")
    val hasStableSigning = listOf(storePath, storePass, keyAlias, keyPass).all { !it.isNullOrBlank() }
    if (hasStableSigning) {
        signingConfigs {
            create("studioStable") {
                storeFile = file(storePath!!)
                storePassword = storePass
                this.keyAlias = keyAlias
                keyPassword = keyPass
            }
        }
    }
    buildTypes {
        getByName("debug") {
            if (hasStableSigning) signingConfig = signingConfigs.getByName("studioStable")
        }
    }
    sourceSets {
        getByName("main") {
            java.srcDir(sdlJava)
            assets.srcDir(packagedArt)
        }
    }
    externalNativeBuild {
        cmake {
            path = nativeRoot.resolve("CMakeLists.txt")
            version = "3.31.6"
        }
    }
    androidResources { noCompress += listOf("png") }
    buildFeatures { compose = true }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions { jvmTarget = "17" }
}
tasks.named("preBuild") { dependsOn(packageEclipseAssets) }

dependencies {
    implementation(platform("androidx.compose:compose-bom:2025.04.01"))
    implementation("androidx.activity:activity-compose:1.10.1")
    implementation("androidx.compose.ui:ui")
    implementation("androidx.compose.foundation:foundation")
    implementation("androidx.compose.material3:material3")
}
