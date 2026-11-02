extends Control
# Action button artwork is loaded from committed PNG assets, never drawn pixel-by-pixel.
var action := "attack"
const ICONS := {
	"attack": "res://assets/ui/attack.png",
	"dash": "res://assets/ui/dash.png",
	"swap": "res://assets/ui/swap.png",
	"skill": "res://assets/ui/skill.png"
}
var icon_texture: Texture2D

func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	var path: String = ICONS.get(action, ICONS["attack"])
	if ResourceLoader.exists(path):
		icon_texture = load(path)
	else:
		push_warning("ECLIPSE action icon PNG missing: " + path)
	queue_redraw()

func _draw() -> void:
	if icon_texture == null:
		return
	var width := minf(size.x, size.y) * 0.64
	var origin := ((size - Vector2(width, width)) * 0.5).round()
	draw_set_transform(Vector2.ZERO)
	draw_texture_rect(icon_texture, Rect2(origin, Vector2(width, width)), false)
