package dev.eclipse.studio

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.net.HttpURLConnection
import java.net.URL

/** Foreground-only HTTPS client; tokens are never persisted on disk. */
internal object StudioRemoteBridge {
    private fun endpoint(baseUrl: String, suffix: String): URL {
        val url = URL(baseUrl.trim().trimEnd('/') + suffix)
        require(url.protocol == "https" && url.userInfo == null && url.host.isNotBlank()) {
            "공개 서버의 HTTPS 주소만 사용할 수 있어요."
        }
        return url
    }

    suspend fun fetchNext(baseUrl: String, token: String): JSONObject? = withContext(Dispatchers.IO) {
        val connection = endpoint(baseUrl, "/device/next").openConnection() as HttpURLConnection
        try {
            connection.requestMethod = "GET"
            connection.setRequestProperty("X-Device-Token", token)
            connection.connectTimeout = 5000
            connection.readTimeout = 5000
            connection.useCaches = false
            when (val status = connection.responseCode) {
                204 -> null
                200 -> connection.inputStream.bufferedReader().use { JSONObject(it.readText()) }
                else -> throw IllegalStateException("서버 응답 오류: HTTP $status")
            }
        } finally {
            connection.disconnect()
        }
    }

    suspend fun sendResult(baseUrl: String, token: String, id: String, result: JSONObject) =
        withContext(Dispatchers.IO) {
            val connection = endpoint(baseUrl, "/device/result").openConnection() as HttpURLConnection
            try {
                connection.requestMethod = "POST"
                connection.setRequestProperty("Content-Type", "application/json; charset=utf-8")
                connection.setRequestProperty("X-Device-Token", token)
                connection.connectTimeout = 5000
                connection.readTimeout = 5000
                connection.doOutput = true
                val payload = JSONObject().put("id", id).put("result", result)
                    .toString().toByteArray(Charsets.UTF_8)
                connection.outputStream.use { it.write(payload) }
                val status = connection.responseCode
                if (status != 200) throw IllegalStateException("명령 응답 전송 실패: HTTP $status")
                connection.inputStream.close()
            } finally {
                connection.disconnect()
            }
        }
}
