package dev.eclipse.studio

import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.activity.compose.BackHandler
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.viewinterop.AndroidView
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.net.HttpURLConnection
import java.net.URL

/**
 * GitHub-hosted web prototype: fetched on every Play launch so edits to play.html
 * do not require another Studio APK build. This is NOT the native SDL3 runtime.
 */
@Composable
internal fun GitHubLivePlay(onExit: () -> Unit) {
    val source = "https://raw.githubusercontent.com/ogw07222-tech/eclipse/main/native-engine/preview/play.html"
    var revision by remember { mutableIntStateOf(0) }
    var html by remember { mutableStateOf<String?>(null) }
    var error by remember { mutableStateOf<String?>(null) }
    BackHandler(onBack = onExit)
    LaunchedEffect(revision) {
        error = null
        html = null
        try {
            html = withContext(Dispatchers.IO) {
                val connection = URL("$source?v=${System.currentTimeMillis()}").openConnection() as HttpURLConnection
                connection.connectTimeout = 10000
                connection.readTimeout = 10000
                connection.useCaches = false
                try {
                    if (connection.responseCode != 200) throw IllegalStateException("GitHub HTTP ${connection.responseCode}")
                    val text = connection.inputStream.bufferedReader().use { it.readText() }
                    if (!text.contains("<canvas") || !text.contains("</html>")) throw IllegalStateException("잘못된 플레이 파일")
                    text
                } finally { connection.disconnect() }
            }
        } catch (e: Exception) {
            error = e.message ?: "네트워크 연결 실패"
        }
    }
    Column(Modifier.fillMaxSize().background(Color(0xFF101B24))) {
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = onExit, shape = RectangleShape) { Text("■ 편집으로") }
            Button(onClick = { revision++ }, shape = RectangleShape) { Text("↻ GitHub 새로고침") }
            Text("웹 플레이 · SDL과 별개", color = Color.LightGray)
        }
        when {
            error != null -> Column {
                Text("플레이 불러오기 실패: $error", color = Color(0xFFFFA9A9))
                Text("인터넷 연결 및 main 브랜치의 native-engine/preview/play.html을 확인해 줘.", color = Color.LightGray)
            }
            html == null -> CircularProgressIndicator()
            else -> {
                val current = html!!
                key(revision, current) {
                    AndroidView(
                        factory = { context ->
                            WebView(context).apply {
                                settings.javaScriptEnabled = true
                                settings.domStorageEnabled = false
                                settings.allowFileAccess = false
                                settings.allowContentAccess = false
                                webViewClient = WebViewClient()
                                setBackgroundColor(android.graphics.Color.BLACK)
                                loadDataWithBaseURL("https://raw.githubusercontent.com/ogw07222-tech/eclipse/main/native-engine/preview/", current, "text/html", "UTF-8", null)
                            }
                        },
                        modifier = Modifier.fillMaxSize(),
                        onRelease = { view -> view.stopLoading(); view.destroy() }
                    )
                }
            }
        }
    }
}
