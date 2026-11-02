extends Node2D
# Pure procedural pixel art; integer-sized rectangles and snapped parallax.
# No external asset dependency; repeatable deterministic forest.
const PIXEL := 6.0
const LAYERS := [
	{"factor": 0.12, "step": 124, "trunk": Color("#243c45"), "leaf": Color("#244b48"), "highlight": Color("#33575a"), "height": 172},
	{"factor": 0.28, "step": 110, "trunk": Color("#29433b"), "leaf": Color("#2c5c49"), "highlight": Color("#39705a"), "height": 210},
	{"factor": 0.48, "step": 104, "trunk": Color("#3b4934"), "leaf": Color("#3b744c"), "highlight": Color("#538859"), "height": 242}
]
var camera_ref: Camera2D
var frame_time := 0.0

func _ready() -> void:
	z_index = -20
	queue_redraw()

func _process(delta: float) -> void:
	frame_time += delta
	queue_redraw()

func _block(x: float, y: float, w: float, h: float, tint: Color) -> void:
	draw_rect(Rect2(roundf(x / PIXEL) * PIXEL, roundf(y / PIXEL) * PIXEL,
		maxf(PIXEL, roundf(w / PIXEL) * PIXEL), maxf(PIXEL, roundf(h / PIXEL) * PIXEL)), tint)

func _pine(cx: float, bottom: float, height: float, trunk_color: Color, leaf_color: Color, highlight: Color, seed: int) -> void:
	_block(cx - 9, bottom - height * 0.64, 18, height * 0.64, trunk_color)
	# Stepped tiers: no smooth circles, diagonals or polygons.
	for tier in range(4):
		var width := 38.0 + float(3 - tier) * 18.0
		var yy := bottom - height + float(tier) * height * 0.19
		for row in range(5):
			var rw := maxf(12.0, width - float(4 - row) * 12.0)
			_block(cx - rw * 0.5, yy + float(row) * PIXEL * 2.0, rw, PIXEL * 2.0, leaf_color)
		_block(cx - width * 0.23, yy + 24, width * 0.3, PIXEL, highlight)
	_block(cx - 5, bottom - height + 4, PIXEL * 2, PIXEL, highlight)
	if seed % 3 == 0:
		_block(cx + 16, bottom - height * 0.43, PIXEL, PIXEL, highlight)

func _draw() -> void:
	_block(0, 0, 1600, 720, Color("#152334"))
	# Pixel moon and small square stars.
	for row in range(11):
		var half := 2 + mini(row, 10 - row)
		_block(1190 - float(half) * PIXEL, 105 + float(row) * PIXEL, float(half * 2 + 1) * PIXEL, PIXEL, Color("#aebda3"))
	for i in range(62):
		var sx := float((i * 233 + 49) % 1600)
		var sy := float((i * 97 + 59) % 315)
		_block(sx, sy, PIXEL, PIXEL, Color("#718d8c"))
	for layer_index in range(LAYERS.size()):
		var layer: Dictionary = LAYERS[layer_index]
		var offset := 0.0
		if is_instance_valid(camera_ref):
			offset = snappedf((camera_ref.global_position.x - 640.0) * float(layer["factor"]) * 0.25, PIXEL)
		for i in range(22):
			var cx := float(i * int(layer["step"]) + layer_index * 47) - 100.0 - offset
			var variation := float((i * 19 + layer_index * 7) % 7) * PIXEL
			_pine(cx, 584.0, float(layer["height"]) + variation,
				layer["trunk"], layer["leaf"], layer["highlight"], i)
	# Foreground moss and individual grass pixels.
	_block(0, 578, 1600, 12, Color("#527b45"))
	for i in range(210):
		var gx := float((i * 47) % 1600)
		var breeze := float(int(sin(frame_time * 1.7 + float(i)) * 1.5)) * PIXEL
		_block(gx + breeze, 566 - float(i % 3) * PIXEL, PIXEL, 12, Color("#83a65a"))
