plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.compose")
}
android {
    namespace = "dev.eclipse.studio"
    compileSdk = 35
    defaultConfig {
        applicationId = "dev.eclipse.studio"
        minSdk = 26
        targetSdk = 35
        versionCode = 1
        versionName = "0.1"
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
            if (hasStableSigning) {
                signingConfig = signingConfigs.getByName("studioStable")
            }
        }
    }
    buildFeatures { compose = true }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions { jvmTarget = "17" }
}
dependencies {
    implementation(platform("androidx.compose:compose-bom:2025.04.01"))
    implementation("androidx.activity:activity-compose:1.10.1")
    implementation("androidx.compose.ui:ui")
    implementation("androidx.compose.foundation:foundation")
    implementation("androidx.compose.material3:material3")
}
