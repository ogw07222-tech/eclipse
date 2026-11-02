package dev.eclipse.studio

import android.graphics.BitmapFactory
import android.net.Uri
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.delay
import kotlin.math.roundToInt

/** First playable on-device preview. This is a self-contained prototype, not the SDL runtime. */
@Composable
internal fun PlayPreview(objects: List<VisualObject>, onExit: () -> Unit) {
    val context = LocalContext.current
    val pictures = remember(objects) {
        objects.mapNotNull { obj ->
            val bitmap = runCatching {
                context.contentResolver.openInputStream(Uri.parse(obj.uri))?.use { BitmapFactory.decodeStream(it) }
            }.getOrNull() ?: return@mapNotNull null
            obj.id to bitmap.asImageBitmap()
        }.toMap()
    }
    // The first picture is the controllable player; others remain scene decorations.
    val player = objects.firstOrNull()
    var x by remember(player?.id) { mutableFloatStateOf((player?.x ?: 70).toFloat()) }
    var y by remember(player?.id) { mutableFloatStateOf((player?.y ?: 80).toFloat()) }
    var vy by remember { mutableFloatStateOf(0f) }
    var grounded by remember { mutableStateOf(false) }
    var hits by remember { mutableIntStateOf(0) }
    var targetHealth by remember { mutableIntStateOf(3) }
    var faceRight by remember { mutableStateOf(true) }
    var holdLeft by remember { mutableStateOf(false) }
    var holdRight by remember { mutableStateOf(false) }
    val targetX = 340f
    val targetY = 190f
    val width = (player?.width ?: 32).coerceIn(1,160).toFloat()
    val height = (player?.height ?: 48).coerceIn(1,180).toFloat()
    val groundY = 240f
    LaunchedEffect(player?.id) {
        while (true) {
            delay(16)
            if (holdLeft) { x=(x-3.4f).coerceIn(0f,480f-width);faceRight=false }
            if (holdRight) { x=(x+3.4f).coerceIn(0f,480f-width);faceRight=true }
            vy = (vy + 0.44f).coerceAtMost(12f)
            y += vy
            val groundTop = groundY - height
            if (y >= groundTop) {
                y = groundTop
                vy = 0f
                grounded = true
            } else grounded = false
        }
    }
    Column(
        Modifier.fillMaxSize().background(Color(0xFF14181D)).padding(6.dp),
        verticalArrangement = Arrangement.spacedBy(6.dp)
    ) {
        Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = onExit, shape = RectangleShape) { Text("■ 편집으로 돌아가기") }
            Text("▶ 플레이 중 · 첫 번째 PNG = 조작 캐릭터", color = Color.White, fontSize = 12.sp)
            Text("명중: $hits · 연습 표적 HP: $targetHealth", color = Color.Cyan, fontSize = 12.sp)
        }
        BoxWithConstraints(
            Modifier.weight(1f).fillMaxWidth().border(1.dp,Color.Gray).background(Color(0xFF101B24))
        ) {
            val scale = minOf(constraints.maxWidth / 480f, constraints.maxHeight / 270f)
            val originX = (constraints.maxWidth - 480f * scale) / 2f
            val originY = (constraints.maxHeight - 270f * scale) / 2f
            val density = LocalDensity.current
            Canvas(Modifier.fillMaxSize()) {
                drawRect(Color(0xFF273541),Offset(originX,originY),Size(480f*scale,270f*scale))
                drawRect(Color(0xFF52624F),Offset(originX,originY+groundY*scale),Size(480f*scale,30f*scale))
                drawRect(Color.Gray,Offset(originX,originY),Size(480f*scale,270f*scale),style=Stroke(1.dp.toPx()))
                if(targetHealth > 0) {
                    drawRect(Color(0xFFD46C59), Offset(originX+targetX*scale,originY+targetY*scale),Size(26f*scale,50f*scale))
                    drawRect(Color(0xFF9FE1B0), Offset(originX+targetX*scale,originY+(targetY-7f)*scale),Size((26f*targetHealth/3f)*scale,4f*scale))
                }
            }
            objects.drop(1).forEach { obj ->
                val picture = pictures[obj.id]
                if (picture != null) {
                    Image(
                        bitmap=picture,contentDescription=obj.name,contentScale=ContentScale.FillBounds,
                        modifier=Modifier.offset { IntOffset((originX+obj.x*scale).roundToInt(),(originY+obj.y*scale).roundToInt()) }
                            .size(with(density){(obj.width*scale).toDp()},with(density){(obj.height*scale).toDp()})
                    )
                }
            }
            val sprite = player?.let { pictures[it.id] }
            if(sprite != null) {
                Image(bitmap=sprite,contentDescription="플레이어",contentScale=ContentScale.FillBounds,
                    modifier=Modifier.offset { IntOffset((originX+x*scale).roundToInt(),(originY+y*scale).roundToInt()) }
                        .size(with(density){(width*scale).toDp()},with(density){(height*scale).toDp()}))
            } else {
                Box(
                    Modifier.offset { IntOffset((originX+x*scale).roundToInt(),(originY+y*scale).roundToInt()) }
                        .size(with(density){(width*scale).toDp()},with(density){(height*scale).toDp()})
                        .background(Color(0xFF54B6EE))
                )
            }
        }
        Row(Modifier.fillMaxWidth(),horizontalArrangement=Arrangement.spacedBy(6.dp)) {
            HoldControl("◀", { holdLeft=it })
            HoldControl("▶", { holdRight=it })
            Control("점프") { if(grounded) { vy=-10f; grounded=false } }
            Control("공격") {
                val closeEnough = if(faceRight) x+width <= targetX+26f && targetX-(x+width) <= 55f
                    else x >= targetX && x-(targetX+26f) <= 55f
                val verticallyAligned = y+height >= targetY && y <= targetY+50f
                if(targetHealth > 0 && closeEnough && verticallyAligned) {
                    targetHealth--; hits++
                }
            }
            Control("재시작") { x=(player?.x ?: 70).toFloat();y=(player?.y ?: 80).toFloat();vy=0f;hits=0;targetHealth=3 }
        }
        Text("방향키는 길게 누르면 계속 이동 · 표적을 공격해서 HP 0 만들기",
            color=Color.LightGray,fontSize=11.sp)
    }
}

@Composable
private fun Control(label: String, action: () -> Unit) {
    Button(onClick=action, shape=RectangleShape, colors=ButtonDefaults.buttonColors(containerColor=Color(0xFF364651)),
        contentPadding=PaddingValues(horizontal=13.dp,vertical=12.dp)) { Text(label,fontSize=16.sp) }
}

@Composable
private fun HoldControl(label: String, onHold: (Boolean)->Unit) {
    Box(
        Modifier.border(1.dp,Color(0xFF80909D)).background(Color(0xFF364651))
            .pointerInput(Unit) {
                detectTapGestures(onPress = {
                    onHold(true)
                    try { tryAwaitRelease() } finally { onHold(false) }
                })
            }
            .padding(horizontal=23.dp,vertical=13.dp),
        contentAlignment=Alignment.Center
    ) { Text(label,color=Color.White,fontSize=16.sp) }
}
