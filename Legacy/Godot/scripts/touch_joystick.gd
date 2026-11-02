extends Control
# One-finger virtual thumbstick. Upward deflection triggers jump.
signal direction_changed(horizontal: float, jump_held: bool)
const DEAD_ZONE := 0.23
const MAX_RADIUS := 63.0
const ARROW_PNG := "res://assets/ui/move_arrows.png"
var arrow_texture: Texture2D
var finger := -1
var stick := Vector2.ZERO
func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_STOP
	custom_minimum_size = Vector2(160, 160)
	if ResourceLoader.exists(ARROW_PNG):
		arrow_texture = load(ARROW_PNG)
	queue_redraw()
func _gui_input(event: InputEvent) -> void:
	if event is InputEventScreenTouch:
		if event.pressed and finger < 0:
			finger = event.index
			_move(event.position)
			accept_event()
		elif not event.pressed and event.index == finger:
			finger = -1
			stick = Vector2.ZERO
			direction_changed.emit(0.0, false)
			queue_redraw()
			accept_event()
	elif event is InputEventScreenDrag and event.index == finger:
		_move(event.position)
		accept_event()
	elif event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_LEFT:
			if event.pressed:
				finger = -2
				_move(event.position)
			elif finger == -2:
				finger = -1
				stick = Vector2.ZERO
				direction_changed.emit(0.0, false)
				queue_redraw()
	elif event is InputEventMouseMotion and finger == -2:
		_move(event.position)
func _move(local_position: Vector2) -> void:
	var center := size * 0.5
	stick = (local_position - center).limit_length(MAX_RADIUS) / MAX_RADIUS
	var x := 0.0 if absf(stick.x) < DEAD_ZONE else stick.x
	direction_changed.emit(x, stick.y < -0.52)
	queue_redraw()
func _pixel_disk(center: Vector2, radius: float, color: Color) -> void:
	var cell := 8.0
	for yy in range(-10, 11):
		for xx in range(-10, 11):
			var p := Vector2(float(xx) * cell, float(yy) * cell)
			if p.length_squared() <= radius * radius:
				draw_rect(Rect2((center + p).round(), Vector2(cell, cell)), color)

func _draw() -> void:
	var center := (size * 0.5).round()
	_pixel_disk(center, 73.0, Color("#101b20", 0.82))
	_pixel_disk(center, 65.0, Color("#84928b", 0.9))
	_pixel_disk(center, 57.0, Color("#213137", 0.9))
	var knob := (center + stick * MAX_RADIUS).snapped(Vector2(8, 8))
	if arrow_texture != null:
		draw_texture_rect(arrow_texture, Rect2(center - Vector2(64, 64), Vector2(128, 128)), false)
	_pixel_disk(knob, 30.0, Color("#0d171d"))
	_pixel_disk(knob, 23.0, Color("#c2c4b8"))
	_pixel_disk(knob, 15.0, Color("#576d70"))
