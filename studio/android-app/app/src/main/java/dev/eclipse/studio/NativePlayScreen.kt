package dev.eclipse.studio

import android.content.Intent
import android.content.pm.ActivityInfo
import android.os.Bundle
import android.system.Os
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
import java.io.File
import java.security.MessageDigest

private const val MAX_GAME_BYTES = 40L * 1024 * 1024

private fun readStatus(activity: ComponentActivity): String {
    val file = File(activity.filesDir, "studio_last_launch.txt")
    return if (file.exists()) file.readText().take(5000) else "아직 게임 실행 기록이 없어."
}

private fun installGame(activity: ComponentActivity, uri: android.net.Uri): String {
    val root = File(activity.filesDir, "game_updates")
    if (!root.exists() && !root.mkdirs()) error("게임 저장 폴더 생성 실패")
    val tmp = File(root, "game_import.tmp")
    val target = File(root, "libeclipse_game.so")
    try {
        var bytes = 0L
        activity.contentResolver.openInputStream(uri)?.use { input ->
            tmp.outputStream().use { output ->
                val buffer = ByteArray(65536)
                while (true) {
                    val count = input.read(buffer)
                    if (count < 0) break
                    bytes += count
                    if (bytes > MAX_GAME_BYTES) error("라이브러리 크기 제한 초과 (40MB)")
                    output.write(buffer, 0, count)
                }
            }
        } ?: error("선택한 파일을 열 수 없어")
        val header = ByteArray(20)
        tmp.inputStream().use { input ->
            if (input.read(header) != header.size) error("ELF 헤더가 손상됨")
        }
        if (header[0] != 0x7f.toByte() || header[1] != 'E'.code.toByte() ||
            header[2] != 'L'.code.toByte() || header[3] != 'F'.code.toByte()) error("ELF 파일이 아님")
        if (header[4] != 2.toByte() || header[5] != 1.toByte() ||
            header[18] != 183.toByte() || header[19] != 0.toByte()) {
            error("ARM64 Android ELF 라이브러리만 사용할 수 있어")
        }
        // Atomic rename prevents partially written code from being selected by the next process.
        if (target.exists() && !target.delete()) error("기존 모듈을 교체할 수 없어")
        if (!tmp.renameTo(target)) error("라이브러리 저장 실패")
        val digest = MessageDigest.getInstance("SHA-256").digest(target.readBytes())
            .joinToString("") { "%02x".format(it) }
        return "가져오기 완료: ${bytes}B / SHA256 ${digest.take(12)}…"
    } finally {
        if (tmp.exists()) tmp.delete()
    }
}

class NativePlayActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE
        setContent { NativePlayHome(this) }
    }
}

@Composable
private fun NativePlayHome(activity: ComponentActivity) {
    val context = LocalContext.current
    val lifecycle = LocalLifecycleOwner.current
    var status by remember { mutableStateOf(readStatus(activity)) }
    var operation by remember { mutableStateOf("") }
    val installed = File(activity.filesDir, "game_updates/libeclipse_game.so")
    var installedRevision by remember { mutableIntStateOf(0) }
    val picker = rememberLauncherForActivityResult(ActivityResultContracts.OpenDocument()) { uri ->
        if (uri != null) {
            operation = runCatching { installGame(activity, uri) }
                .fold(onSuccess = { it }, onFailure = { "가져오기 오류: ${it.message}" })
            installedRevision++
        }
    }
    DisposableEffect(lifecycle) {
        val observer = LifecycleEventObserver { _, event ->
            if (event == Lifecycle.Event.ON_RESUME) status = readStatus(activity)
        }
        lifecycle.lifecycle.addObserver(observer)
        onDispose { lifecycle.lifecycle.removeObserver(observer) }
    }
    MaterialTheme(colorScheme = darkColorScheme()) {
        Column(
            Modifier.fillMaxSize().background(Color(0xFF101822)).padding(20.dp)
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(10.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text("ECLIPSE · SDL3 Native Play", style = MaterialTheme.typography.headlineMedium)
            Text("원본 C++ 엔진 · APK 재설치 없는 로컬 게임 코드 교체")
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(
                    onClick = { startActivity(Intent(context, GameActivity::class.java)) },
                    shape = RectangleShape
                ) { Text("▶ 플레이 시작") }
                Button(
                    onClick = { picker.launch(arrayOf("application/octet-stream", "application/x-sharedlib", "*/*")) },
                    shape = RectangleShape
                ) { Text("Termux .so 가져오기") }
                Button(
                    onClick = {
                        operation = if (installed.delete()) "기본 내장 게임으로 전환했어."
                        else "가져온 모듈이 없거나 삭제에 실패했어."
                        installedRevision++
                    },
                    shape = RectangleShape
                ) { Text("기본 게임 복원") }
            }
            Text(
                if (installed.exists()) "실행 모듈: 가져온 게임 라이브러리"
                else "실행 모듈: 내장 원본 게임",
                color = Color(0xFFAED2F8)
            )
            if (operation.isNotBlank()) Text(operation, color = Color(0xFFD5E9AC))
            HorizontalDivider()
            Text("실행 진단 (게임에서 돌아오면 자동 갱신)")
            Text(
                when {
                    status.startsWith("ERROR") -> "실행 오류\n$status"
                    status.startsWith("STARTED") || status.startsWith("RUNNING") ->
                        "마지막 실행이 정상 종료되지 않았을 수 있어. 프로세스 종료/크래시 확인 필요.\n$status"
                    else -> status
                },
                color = if (status.startsWith("ERROR")) Color(0xFFFF9D9D) else Color(0xFFC3D5E4)
            )
            Text("참고: C++ 파일명·줄 번호는 별도 심볼/크래시 로그가 있어야 표시할 수 있어.",
                style = MaterialTheme.typography.bodySmall)
        }
    }
}
