extends Control
# Chunky pixel HUD. No top-left menu buttons, only health, slot silhouettes and foes.
var hp := 100
var max_hp := 100
var foes := 4
var skill_ratio := 1.0
func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	size = Vector2(1280, 125)
func _pixel(x: float, y: float, color: Color, mult: int = 1) -> void:
	draw_rect(Rect2(x, y, float(mult * 6), float(mult * 6)), color)
func _heart(at: Vector2, filled: bool) -> void:
	var rows := ["..XX.XX..",".XXXXXXX.","XXXXXXXXX","XXXXXXXXX",".XXXXXXX.","..XXXXX..","...XXX...","....X...."]
	for y in range(rows.size()):
		for x in range(rows[y].length()):
			if rows[y].substr(x, 1) == "X":
				_pixel(at.x + x * 5, at.y + y * 5, Color("#f56b78") if filled else Color("#364451"))
func _draw() -> void:
	var hearts := 7
	var full := float(hp) / float(maxi(max_hp,1))
	for i in range(hearts):
		_heart(Vector2(360 + i * 55, 18), full >= (float(i) + 0.5) / float(hearts))
	for i in range(5):
		var base := Vector2(365 + i * 48, 76)
		draw_rect(Rect2(base, Vector2(42, 16)), Color("#070e18"))
		draw_rect(Rect2(base + Vector2(4,4), Vector2(34,8)), Color("#f1bd6f") if skill_ratio > float(i) / 5.0 else Color("#353a43"))
	for i in range(4):
		var pos := Vector2(765 + i * 74, 21)
		draw_rect(Rect2(pos, Vector2(66, 61)), Color("#080d17"))
		draw_rect(Rect2(pos + Vector2(6,6), Vector2(54,49)), Color("#667482"))
		draw_rect(Rect2(pos + Vector2(12,12), Vector2(42,37)), Color("#222c3d"))
	# Enemy indicator, chunky skull + numeric count. Use built-in text only for count.
	for point in [Vector2(1130,30),Vector2(1148,30),Vector2(1124,48),Vector2(1136,60),Vector2(1148,60)]:
		_pixel(point.x, point.y, Color("#e8e7cd"), 2)
	draw_string(ThemeDB.fallback_font,Vector2(1174,67), "x%d" % foes, HORIZONTAL_ALIGNMENT_LEFT,-1,34,Color("#ece7d2"))
