package dev.eclipse.studio

import android.content.Context
import android.graphics.BitmapFactory
import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.Image
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import org.json.JSONArray
import org.json.JSONObject
import kotlin.math.roundToInt

internal data class VisualObject(val id: Int, val name: String, val uri: String, val x: Int, val y: Int, val width: Int, val height: Int)
private const val PROJECT_FILE = "eclipse_scene_v1.json"
private val bg = Color(0xFF101B24)
private val borderColor = Color(0xFF778899)

private fun saveScene(context: Context, objects: List<VisualObject>) {
    val root = JSONObject().put("schemaVersion", 1)
    val arr = JSONArray()
    objects.forEach { obj ->
        arr.put(JSONObject().put("id",obj.id).put("name",obj.name).put("uri",obj.uri)
            .put("x",obj.x).put("y",obj.y).put("width",obj.width).put("height",obj.height))
    }
    root.put("objects",arr)
    context.openFileOutput(PROJECT_FILE,Context.MODE_PRIVATE).bufferedWriter().use { it.write(root.toString(2)) }
}
private fun loadScene(context: Context): List<VisualObject> {
    if (!context.getFileStreamPath(PROJECT_FILE).exists()) return emptyList()
    val arr=JSONObject(context.openFileInput(PROJECT_FILE).bufferedReader().use { it.readText() }).getJSONArray("objects")
    return (0 until arr.length()).map { i ->
        val o=arr.getJSONObject(i)
        VisualObject(o.getInt("id"),o.getString("name"),o.getString("uri"),o.getInt("x"),o.getInt("y"),o.getInt("width"),o.getInt("height"))
    }
}
@Composable
private fun Sharp(label: String, onClick: () -> Unit, enabled: Boolean = true) {
    Button(onClick=onClick, enabled=enabled, shape=RectangleShape,
        contentPadding=PaddingValues(horizontal=10.dp,vertical=4.dp),
        colors=ButtonDefaults.buttonColors(containerColor=Color(0xFF34424F))) {
        Text(label,fontSize=12.sp)
    }
}

@Composable
internal fun VisualSceneEditor(githubPng: String? = null, onImported: () -> Unit = {}) {
    val context= LocalContext.current
    val objects=remember { mutableStateListOf<VisualObject>() }
    var selected by remember { mutableIntStateOf(-1) }
    var status by remember { mutableStateOf("PNG를 가져와서 화면에 배치해 보세요.") }
    val undoStack = remember { mutableStateListOf<List<VisualObject>>() }
    var showCommands by remember { mutableStateOf(false) }
    var commandText by remember { mutableStateOf("""{"action":"list_objects"}""") }
    var commandReply by remember { mutableStateOf("") }
    var remotePanel by remember { mutableStateOf(false) }
    var remoteUrl by remember { mutableStateOf("") }
    var remoteToken by remember { mutableStateOf("") }
    var remoteConnected by remember { mutableStateOf(false) }
    var remoteStatus by remember { mutableStateOf("연결되지 않음") }
    val completedRemoteIds = remember { mutableStateMapOf<String, JSONObject>() }
    fun applyCommand(command: JSONObject, recordUndo: Boolean = true): JSONObject {
        try {
            val previous = objects.toList()
            val result = SceneCommands.execute(previous, command)
            if (result.changed) {
                if (recordUndo) {
                    if (undoStack.size >= 40) undoStack.removeAt(0)
                    undoStack.add(previous)
                }
                objects.clear()
                objects.addAll(result.objects)
                result.selectedId?.let { selected = it }
                if (selected >= 0 && objects.none { it.id == selected }) selected = -1
            }
            commandReply = result.response().toString()
            status = result.message
            return result.response()
        } catch (e: Exception) {
            val error = JSONObject().put("ok", false).put("error", e.message ?: "Unknown error")
            commandReply = error.toString()
            status = "명령 오류: ${e.message}"
            return error
        }
    }
    fun addImage(name: String, uri: String, width: Int, height: Int) {
        applyCommand(JSONObject().put("action", "add_object").put("name", name)
            .put("uri", uri).put("x", 100).put("y", 80)
            .put("width", width).put("height", height))
    }
    var nextId by remember { mutableIntStateOf(1) }
    var zoom by remember { mutableFloatStateOf(1f) }
    var playing by remember { mutableStateOf(false) }
    LaunchedEffect(remoteConnected, remoteUrl, remoteToken, playing) {
        if (!remoteConnected || playing) return@LaunchedEffect
        remoteStatus = "서버 명령 대기 중 (앱 화면이 열려 있는 동안)"
        while (isActive) {
            try {
                val incoming = StudioRemoteBridge.fetchNext(remoteUrl, remoteToken)
                if (incoming != null) {
                    val id = incoming.getString("id")
                    val command = incoming.getJSONObject("command")
                    val action = command.optString("action")
                    val result = completedRemoteIds[id] ?: run {
                        // The server applies an allowlist too; re-check on device.
                        val reply = if (action in setOf("list_objects", "move_object", "set_property")) {
                            applyCommand(command)
                        } else {
                            JSONObject().put("ok", false).put("error", "원격 실행이 허용되지 않은 명령")
                        }
                        if (completedRemoteIds.size >= 100) completedRemoteIds.remove(completedRemoteIds.keys.first())
                        completedRemoteIds[id] = reply
                        reply
                    }
                    StudioRemoteBridge.sendResult(remoteUrl, remoteToken, id, result)
                    remoteStatus = "최근 처리: $action (#${id.take(8)})"
                }
            } catch (e: Exception) {
                remoteStatus = "연결 오류: ${e.message}"
            }
            delay(3000)
        }
    }
    val picker=rememberLauncherForActivityResult(ActivityResultContracts.OpenDocument()) { uri: Uri? ->
        if(uri!=null) {
            try {
                context.contentResolver.takePersistableUriPermission(uri, android.content.Intent.FLAG_GRANT_READ_URI_PERMISSION)
                val bounds=BitmapFactory.Options().apply { inJustDecodeBounds=true }
                context.contentResolver.openInputStream(uri)?.use { BitmapFactory.decodeStream(it,null,bounds) }
                val width=bounds.outWidth.coerceIn(1,480)
                val height=bounds.outHeight.coerceIn(1,270)
                addImage("그림 $nextId", uri.toString(), width, height)
                nextId = (objects.maxOfOrNull { it.id } ?: 0) + 1
                status="PNG가 추가됐어요. 그림을 드래그해서 이동해 보세요."
            } catch(e:Exception) { status="파일 불러오기 실패: ${e.message}" }
        }
    }
    LaunchedEffect(githubPng) {
        if (githubPng != null) {
            val bounds = BitmapFactory.Options().apply { inJustDecodeBounds = true }
            context.contentResolver.openInputStream(Uri.parse(githubPng))?.use { BitmapFactory.decodeStream(it, null, bounds) }
            addImage("GitHub PNG $nextId", githubPng,
                bounds.outWidth.coerceIn(1,480), bounds.outHeight.coerceIn(1,270))
            nextId = (objects.maxOfOrNull { it.id } ?: 0) + 1
            status = "GitHub에서 그림을 가져왔어요. 화면에서 이동해 보세요."
            onImported()
        }
    }
    LaunchedEffect(Unit) {
        try {
            objects.clear()
            objects.addAll(loadScene(context))
            undoStack.clear()
            nextId=(objects.maxOfOrNull { it.id } ?: 0)+1
            selected=objects.firstOrNull()?.id ?: -1
        } catch(e: Exception) { status="저장된 씬을 읽지 못했어요: ${e.message}" }
    }
    val images = remember { mutableStateMapOf<String, androidx.compose.ui.graphics.ImageBitmap?>() }
    LaunchedEffect(objects.map { it.uri }) {
        objects.forEach { obj ->
            if(!images.containsKey(obj.uri)) {
                val bitmap=runCatching {
                    context.contentResolver.openInputStream(Uri.parse(obj.uri))?.use { BitmapFactory.decodeStream(it) }?.asImageBitmap()
                }.getOrNull()
                images[obj.uri]=bitmap
            }
        }
    }
    if (playing) {
        PlayPreview(objects.toList(), onExit = { playing = false })
        return
    }
    Column(Modifier.fillMaxSize().background(Color(0xFF14181D)).padding(6.dp)) {
        Row(horizontalArrangement=Arrangement.spacedBy(6.dp),verticalAlignment=Alignment.CenterVertically) {
            Sharp("▶ 플레이", { playing = true })
            Sharp("+ PNG 추가",{picker.launch(arrayOf("image/png"))})
            Sharp("저장",{
                try { saveScene(context,objects.toList());status="앱 내부에 씬을 저장했어요." }
                catch(e:Exception) { status="저장 실패: ${e.message}" }
            })
            Sharp("초기화", { applyCommand(JSONObject().put("action", "clear_scene"));nextId=1 })
            Sharp("실행 취소", {
                if (undoStack.isNotEmpty()) {
                    val restored = undoStack.removeAt(undoStack.lastIndex)
                    objects.clear()
                    objects.addAll(restored)
                    selected = objects.firstOrNull()?.id ?: -1
                    nextId = (objects.maxOfOrNull { it.id } ?: 0) + 1
                    status = "이전 편집 상태로 복원했어요. 저장하면 반영됩니다."
                }
            }, undoStack.isNotEmpty())
            Sharp("AI 명령", { showCommands = !showCommands })
            Sharp("원격 연결", { remotePanel = !remotePanel })
            Text("확대: ${(zoom*100).roundToInt()}%",color=Color.White,fontSize=12.sp)
        }
        if (remotePanel) {
            Text("외부 HTTPS 브리지 연결 · 이 화면에서만 명령 처리", color = Color.White, fontSize = 11.sp)
            OutlinedTextField(
                value = remoteUrl, onValueChange = {
                    if (!remoteConnected) remoteUrl = it
                }, enabled = !remoteConnected,
                label = { Text("브리지 HTTPS 주소") },
                singleLine = true, modifier = Modifier.fillMaxWidth()
            )
            OutlinedTextField(
                value = remoteToken, onValueChange = {
                    if (!remoteConnected) remoteToken = it
                }, enabled = !remoteConnected,
                label = { Text("기기 인증 토큰 (저장되지 않음)") },
                visualTransformation = androidx.compose.ui.text.input.PasswordVisualTransformation(),
                singleLine = true, modifier = Modifier.fillMaxWidth()
            )
            Row(horizontalArrangement = Arrangement.spacedBy(6.dp),
                verticalAlignment = Alignment.CenterVertically) {
                Sharp(if (remoteConnected) "연결 해제" else "연결", {
                    if (remoteConnected) {
                        remoteConnected = false
                        remoteStatus = "연결 해제됨"
                    } else if (!remoteUrl.startsWith("https://") || remoteToken.length < 24) {
                        remoteStatus = "HTTPS 주소와 24자 이상 기기 토큰을 입력하세요."
                    } else {
                        remoteConnected = true
                    }
                })
                Text(remoteStatus, color = Color(0xFFBFDDE8), fontSize = 10.sp, maxLines = 2)
            }
        }
        if (showCommands) {
            OutlinedTextField(
                value = commandText,
                onValueChange = { commandText = it },
                modifier = Modifier.fillMaxWidth(),
                label = { Text("JSON 편집 명령 (로컬 테스트)") },
                maxLines = 3,
                singleLine = false
            )
            Row(horizontalArrangement = Arrangement.spacedBy(6.dp),
                verticalAlignment = Alignment.CenterVertically) {
                Sharp("명령 실행", {
                    try { applyCommand(JSONObject(commandText)) }
                    catch (e: Exception) {
                        commandReply = JSONObject().put("ok", false)
                            .put("error", e.message ?: "Invalid JSON").toString()
                    }
                })
                Text(commandReply, color = Color(0xFFBFDDE8),
                    fontSize = 10.sp, maxLines = 2)
            }
        }
        Row(Modifier.weight(1f),horizontalArrangement=Arrangement.spacedBy(6.dp)) {
            Column(Modifier.width(128.dp).fillMaxHeight().border(1.dp,borderColor).padding(5.dp)) {
                Text("화면에 있는 그림",color=Color.White,fontSize=12.sp)
                LazyColumn {
                    items(objects,key={it.id}) { item ->
                        Text((if(item.id==selected) "▶ " else "")+item.name,
                            color=if(item.id==selected) Color.Cyan else Color.White,
                            fontSize=12.sp,modifier=Modifier.fillMaxWidth().padding(vertical=10.dp)
                                .then(Modifier.pointerInput(item.id){ detectTapGestures(onTap={selected=item.id}) }))
                    }
                }
            }
            BoxWithConstraints(Modifier.weight(1f).fillMaxHeight().border(1.dp,borderColor).background(bg)) {
                val density=androidx.compose.ui.platform.LocalDensity.current
                val canvasWidth=constraints.maxWidth.toFloat()
                val canvasHeight=constraints.maxHeight.toFloat()
                val scale = minOf(canvasWidth/480f,canvasHeight/270f)*zoom
                val offsetX=(canvasWidth-480f*scale)/2f
                val offsetY=(canvasHeight-270f*scale)/2f
                Box(Modifier.fillMaxSize().pointerInput(objects.toList(),scale,offsetX,offsetY) {
                    detectDragGestures(onDragStart={p->
                        if (undoStack.size >= 40) undoStack.removeAt(0)
                        undoStack.add(objects.toList())
                        selected=objects.lastOrNull { obj ->
                            val px=offsetX+obj.x*scale
                            val py=offsetY+obj.y*scale
                            p.x>=px && p.x<=px+obj.width*scale && p.y>=py && p.y<=py+obj.height*scale
                        }?.id ?: -1
                    },onDrag={change,amount->
                        change.consume()
                        val index=objects.indexOfFirst { it.id==selected }
                        if(index>=0) {
                            val item=objects[index]
                            applyCommand(JSONObject().put("action", "move_object")
                                .put("id", item.id)
                                .put("x", (item.x + amount.x / scale).roundToInt())
                                .put("y", (item.y + amount.y / scale).roundToInt()),
                                recordUndo = false)
                        }
                    })
                }.pointerInput(Unit){
                    detectTransformGestures { _,_,change,_ -> zoom=(zoom*change).coerceIn(0.5f,4f) }
                }) {
                    Canvas(Modifier.fillMaxSize()) {
                        drawRect(Color(0xFF273541),topLeft=Offset(offsetX,offsetY),
                            size=androidx.compose.ui.geometry.Size(480f*scale,270f*scale))
                        drawRect(Color(0xFF6C8798),topLeft=Offset(offsetX,offsetY),
                            size=androidx.compose.ui.geometry.Size(480f*scale,270f*scale),style=Stroke(1.dp.toPx()))
                    }
                    objects.forEach { obj ->
                        val img=images[obj.uri]
                        Box(Modifier.offset { androidx.compose.ui.unit.IntOffset((offsetX+obj.x*scale).roundToInt(),(offsetY+obj.y*scale).roundToInt()) }
                            .size(with(density){(obj.width*scale).toDp()},with(density){(obj.height*scale).toDp()})
                            .border(if(obj.id==selected) 2.dp else 0.dp,if(obj.id==selected) Color.Cyan else Color.Transparent)) {
                            if(img!=null) Image(bitmap=img, contentDescription=obj.name,contentScale=ContentScale.FillBounds,modifier=Modifier.fillMaxSize())
                            else Text("PNG 없음",color=Color.Red,fontSize=10.sp)
                        }
                    }
                }
            }
            Column(Modifier.width(125.dp).fillMaxHeight().border(1.dp,borderColor).padding(5.dp),
                verticalArrangement=Arrangement.spacedBy(5.dp)) {
                val i=objects.indexOfFirst { it.id==selected }
                val item=objects.getOrNull(i)
                Text("선택한 그림",color=Color.White,fontSize=12.sp)
                Text(item?.name ?: "선택 없음",color=Color.Cyan,fontSize=12.sp)
                Text("X: ${item?.x ?: "-"}",color=Color.White,fontSize=12.sp)
                Text("Y: ${item?.y ?: "-"}",color=Color.White,fontSize=12.sp)
                Sharp("오른쪽 +10",{
                    item?.let { applyCommand(JSONObject().put("action", "move_object")
                        .put("id", it.id).put("x", it.x + 10).put("y", it.y)) }
                },i>=0)
                Sharp("아래쪽 +10",{
                    item?.let { applyCommand(JSONObject().put("action", "move_object")
                        .put("id", it.id).put("x", it.x).put("y", it.y + 10)) }
                },i>=0)
                Sharp("선택 삭제",{
                    item?.let { applyCommand(JSONObject().put("action", "delete_object").put("id", it.id)) }
                },i>=0)
            }
        }
        Text(status,color=Color(0xFFBFDDE8),fontSize=11.sp)
    }
}
