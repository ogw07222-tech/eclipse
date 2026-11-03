package dev.eclipse.studio

import android.content.Intent
import android.content.pm.ActivityInfo
import androidx.activity.ComponentActivity
import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.unit.dp

/** Temporary game-first home. Editor code is retained but not started. */
class NativePlayActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE
        setContent {
            MaterialTheme(colorScheme = darkColorScheme()) {
                Column(
                    modifier = Modifier.fillMaxSize().background(Color(0xFF101822)).padding(24.dp),
                    verticalArrangement = Arrangement.Center,
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Text("ECLIPSE · SDL3 원본", style = MaterialTheme.typography.headlineMedium)
                    Spacer(Modifier.height(12.dp))
                    Text("원본 C++ 전투 엔진 · Android 네이티브 실행")
                    Spacer(Modifier.height(24.dp))
                    Button(
                        onClick = { startActivity(Intent(this@NativePlayActivity, GameActivity::class.java)) },
                        shape = RectangleShape
                    ) { Text("▶ 플레이") }
                    Spacer(Modifier.height(12.dp))
                    Text("게임 종료 또는 Android 뒤로가기로 이 화면에 복귀")
                }
            }
        }
    }
}
