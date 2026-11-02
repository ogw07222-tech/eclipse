package dev.eclipse.studio

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.*
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.*
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.*
import androidx.compose.ui.text.font.FontWeight

private val bg = Color(0xFF14181D)
private val panel = Color(0xFF20262D)
private val edge = Color(0xFF657380)
private val fg = Color(0xFFF2F5F7)

data class SceneItem(val id: Int, val name: String, val x: Int, val y: Int)

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent { StudioScreen() }
    }
}

@Composable
private fun SquareButton(label: String, enabled: Boolean = true, onClick: () -> Unit) {
    Button(
        onClick = onClick, enabled = enabled,
        shape = androidx.compose.ui.graphics.RectangleShape,
        contentPadding = PaddingValues(horizontal = 12.dp, vertical = 6.dp),
        colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF35424E), contentColor = fg)
    ) { Text(label, fontSize = 13.sp) }
}

@Composable
private fun StudioScreen() {
    var project by remember { mutableStateOf("Untitled") }
    val objects = remember { mutableStateListOf(SceneItem(1, "Camera", 0, 0), SceneItem(2, "Player", 120, 80)) }
    var selection by remember { mutableIntStateOf(2) }
    var tab by remember { mutableStateOf("화면 꾸미기") }
    var status by remember { mutableStateOf("1. 왼쪽에서 선택 → 2. 오른쪽에서 수정 · 저장 기능 개발 중") }
    var nextId by remember { mutableIntStateOf(3) }
    MaterialTheme(colorScheme = darkColorScheme(background = bg, surface = panel, onSurface = fg)) {
        Column(Modifier.fillMaxSize().background(bg).padding(8.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Text("ECLIPSE STUDIO", fontWeight = FontWeight.Bold, color = fg)
                Text(project, color = Color.LightGray, fontSize = 12.sp)
                Spacer(Modifier.weight(1f))
                SquareButton("처음부터 만들기") {
                    project = "Untitled"
                    objects.clear()
                    objects.add(SceneItem(1, "Camera", 0, 0))
                    nextId = 2
                    selection = 1
                    status = "새 임시 프로젝트 생성"
                }
                SquareButton("저장 · 준비 중", enabled = false) {}
            }
            Spacer(Modifier.height(8.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                listOf("화면 꾸미기", "그림 관리", "움직임 만들기", "공격 설정", "GitHub").forEach { name ->
                    SquareButton(name) { tab = name }
                }
            }
            Text("시작하기: 항목을 추가하고 → 목록에서 선택하고 → 위치를 바꿔보세요", color = Color(0xFFBBDDEA), fontSize = 12.sp)
            Spacer(Modifier.height(8.dp))
            if (tab == "GitHub") {
                Box(Modifier.weight(1f)) { GitHubReader() }
            } else if (tab == "화면 꾸미기") {
                Row(Modifier.weight(1f), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    Column(Modifier.width(160.dp).fillMaxHeight().border(1.dp, edge).background(panel).padding(8.dp)) {
                        Text("화면에 있는 것", fontWeight = FontWeight.Bold, fontSize = 12.sp)
                        Spacer(Modifier.height(10.dp))
                        LazyColumn(Modifier.weight(1f)) {
                            items(objects, key = { it.id }) { item ->
                                Text(
                                    if (selection == item.id) "> ${item.name}" else "  ${item.name}",
                                    color = if (selection == item.id) Color.Cyan else fg,
                                    modifier = Modifier.fillMaxWidth().clickable { selection = item.id }.padding(vertical = 8.dp)
                                )
                            }
                        }
                        SquareButton("+ 그림/물체 추가") {
                            objects.add(SceneItem(nextId, "Sprite ${nextId}", 0, 0))
                            selection = nextId
                            nextId++
                            status = "새 항목을 추가했어요. 오른쪽에서 위치를 변경할 수 있어요."
                        }
                    }
                    Box(Modifier.weight(1f).fillMaxHeight().border(1.dp, edge).background(Color(0xFF101B24)), contentAlignment = Alignment.Center) {
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            Text("게임 화면 미리보기 · 480 × 270", fontSize = 14.sp)
                            Text("현재는 목록에서 선택하고 오른쪽에서 위치를 조정해요", color = Color.LightGray, fontSize = 12.sp)
                        }
                    }
                    Column(Modifier.width(165.dp).fillMaxHeight().border(1.dp, edge).background(panel).padding(8.dp)) {
                        Text("선택한 항목", fontWeight = FontWeight.Bold, fontSize = 12.sp)
                        Spacer(Modifier.height(12.dp))
                        val item = objects.find { it.id == selection }
                        Text(item?.name ?: "선택 없음")
                        Text("X: ${item?.x ?: 0}", fontSize = 12.sp)
                        Text("Y: ${item?.y ?: 0}", fontSize = 12.sp)
                        Spacer(Modifier.height(10.dp))
                        SquareButton("오른쪽으로 10", enabled = item != null) {
                            val index = objects.indexOfFirst { it.id == selection }
                            if (index >= 0) objects[index] = objects[index].copy(x = objects[index].x + 10)
                        }
                        SquareButton("아래로 10", enabled = item != null) {
                            val index = objects.indexOfFirst { it.id == selection }
                            if (index >= 0) objects[index] = objects[index].copy(y = objects[index].y + 10)
                        }
                    }
                }
            } else {
                Box(Modifier.weight(1f).fillMaxWidth().border(1.dp, edge).background(panel), contentAlignment = Alignment.Center) {
                    Text("${tab} 기능은 개발 중이에요", color = fg)
                }
            }
            Spacer(Modifier.height(6.dp))
            Text(status, color = Color.LightGray, fontSize = 11.sp)
        }
    }
}
