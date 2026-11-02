package dev.eclipse.studio

import androidx.compose.foundation.*
import androidx.compose.ui.platform.LocalContext
import android.net.Uri
import android.util.Base64
import java.io.File
import java.net.URLEncoder
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.graphics.RectangleShape
import org.json.JSONArray
import org.json.JSONObject
import java.net.HttpURLConnection
import java.net.URL

private data class RepoFile(val name: String, val path: String, val directory: Boolean)
private val ghPattern = Regex("^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$")
private val pathSegments = Regex("^[A-Za-z0-9_./ -]*$")

private fun fetch(url: String): String {
    val connection = URL(url).openConnection() as HttpURLConnection
    connection.connectTimeout = 10000
    connection.readTimeout = 10000
    connection.setRequestProperty("Accept", "application/vnd.github+json")
    connection.setRequestProperty("X-GitHub-Api-Version", "2022-11-28")
    try {
        val status = connection.responseCode
        if (status !in 200..299) throw IllegalStateException("GitHub HTTP $status")
        return connection.inputStream.bufferedReader().use { it.readText() }
    } finally { connection.disconnect() }
}

@Composable
fun GitHubReader(onImportPng: (String) -> Unit = {}) {
    val context = LocalContext.current
    var branch by remember { mutableStateOf("main") }
    var repo by remember { mutableStateOf("ogw07222-tech/eclipse") }
    var folder by remember { mutableStateOf("") }
    var entries by remember { mutableStateOf(emptyList<RepoFile>()) }
    var preview by remember { mutableStateOf("") }
    var message by remember { mutableStateOf("공개 저장소만 읽을 수 있어요.") }
    var busy by remember { mutableStateOf(false) }

    fun load(target: String, file: String? = null) {
        if (!ghPattern.matches(repo) || !pathSegments.matches(target)) {
            message = "저장소는 owner/repo 형식으로 입력해 주세요."
            return
        }
        busy = true
        message = "GitHub에서 읽는 중..."
        val snapshot = repo
        Thread {
            try {
                val endpoint = "https://api.github.com/repos/$snapshot/contents/" +
                    target.split('/').filter { it.isNotEmpty() }.joinToString("/") {
                        URLEncoder.encode(it, "UTF-8")
                    } + "?ref=" + URLEncoder.encode(branch, "UTF-8")
                if (file == null) {
                    val data = JSONArray(fetch(endpoint))
                    val result = (0 until data.length()).map { i ->
                        val obj = data.getJSONObject(i)
                        RepoFile(obj.getString("name"),obj.getString("path"),obj.getString("type") == "dir")
                    }.sortedWith(compareByDescending<RepoFile> { it.directory }.thenBy { it.name })
                    entries = result
                    folder = target
                    preview = ""
                    message = "${result.size}개 항목 · 읽기 전용"
                } else {
                    if (file.endsWith(".png", ignoreCase = true)) {
                        val data = JSONObject(fetch(endpoint))
                        if (data.optLong("size", 0L) > 8_000_000) throw IllegalArgumentException("PNG 최대 크기는 8MB예요.")
                        val encoded = data.getString("content").replace("\\s".toRegex(), "")
                        val bytes = Base64.decode(encoded, Base64.DEFAULT)
                        if (bytes.size < 8 || !bytes.take(8).toByteArray().contentEquals(byteArrayOf(137.toByte(),80,78,71,13,10,26,10))) throw IllegalArgumentException("PNG 파일이 아니에요.")
                        val out = File(context.filesDir, "github_asset_" + System.nanoTime() + ".png")
                        out.writeBytes(bytes)
                        onImportPng(Uri.fromFile(out).toString())
                        message = "PNG를 씬으로 가져왔어요: $file"
                        return@Thread
                    }
                    // Text preview only; PNG is imported into the scene.
                    if (!file.matches(Regex(".*\\.(kt|kts|cs|cpp|c|h|hpp|json|md|txt|xml|gd|yml|yaml)$", RegexOption.IGNORE_CASE))) {
                        throw IllegalArgumentException("이 파일은 텍스트 미리보기를 지원하지 않아요.")
                    }
                    val data = JSONObject(fetch(endpoint))
                    if (data.optLong("size", 0L) > 256_000) throw IllegalArgumentException("미리보기는 256KB 이하 파일만 지원해요.")
                    val encoded = data.getString("content").replace("\\s".toRegex(),"")
                    val bytes = android.util.Base64.decode(encoded, android.util.Base64.DEFAULT)
                    preview = bytes.toString(Charsets.UTF_8)
                    message = "$file · 읽기 전용"
                }
            } catch(e: Exception) {
                message = "불러오기 실패: ${e.message}"
            } finally { busy = false }
        }.start()
    }

    Column(Modifier.fillMaxSize().padding(8.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Text("GitHub 파일 열기", fontWeight = FontWeight.Bold, color = Color.White)
        Text("공개 GitHub 프로젝트 탐색 · PNG를 누르면 씬으로 가져옵니다. 쓰기와 비공개 저장소는 아직 지원하지 않습니다.", color = Color.LightGray)
        OutlinedTextField(value=repo,onValueChange={repo=it},label={Text("저장소 (소유자/이름)")},singleLine=true,modifier=Modifier.fillMaxWidth())
        OutlinedTextField(value=branch,onValueChange={branch=it},label={Text("브랜치 (예: main)")},singleLine=true,modifier=Modifier.fillMaxWidth())
        Row(horizontalArrangement=Arrangement.spacedBy(8.dp)) {
            Button(onClick={ load("") },shape=RectangleShape,enabled=!busy) {Text("저장소 열기")}
            Button(onClick={ load(folder.substringBeforeLast('/',"")) },shape=RectangleShape,enabled=!busy) {Text("상위 폴더")}
        }
        Text(message, color = Color.LightGray)
        if(preview.isNotEmpty()) {
            Text("파일 내용",fontWeight=FontWeight.Bold,color=Color.White)
            Column(Modifier.weight(1f).verticalScroll(rememberScrollState())) {
                Text(preview, color=Color.White)
            }
        } else {
            LazyColumn(Modifier.weight(1f)) {
                items(entries) { entry ->
                    Text(
                        (if(entry.directory) "📁 " else if(entry.name.endsWith(".png", true)) "🖼 가져오기 · " else "📄 ") + entry.name,
                        color = Color.White,
                        modifier = Modifier.fillMaxWidth().clickable {
                            load(entry.path, if(entry.directory) null else entry.name)
                        }.padding(12.dp)
                    )
                    HorizontalDivider()
                }
            }
        }
    }
}
