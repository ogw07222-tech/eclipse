extends Control
# One-finger virtual thumbstick. Upward deflection triggers jump.
signal direction_changed(horizontal: float, jump_held: bool)
const DEAD_ZONE := 0.23
const MAX_RADIUS := 63.0
var finger := -1
var stick := Vector2.ZERO
func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_STOP
	custom_minimum_size = Vector2(160, 160)
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
func _draw() -> void:
	var center := (size * 0.5).round()
	var base := Rect2(center - Vector2(70, 70), Vector2(140, 140))
	draw_rect(base, Color("#0c1924", 0.84))
	draw_rect(base, Color("#d4e0d2"), false, 4.0, false)
	draw_rect(Rect2(center - Vector2(44, 44), Vector2(88, 88)), Color("#263d4c"), false, 4.0, false)
	var knob := (center + stick * MAX_RADIUS).round()
	draw_rect(Rect2(knob - Vector2(25, 25), Vector2(50, 50)), Color("#447593"))
	draw_rect(Rect2(knob - Vector2(25, 25), Vector2(50, 50)), Color("#e2edf3"), false, 4.0, false)
	draw_rect(Rect2(knob - Vector2(17, 17), Vector2(34, 6)), Color("#b3d6e8"))
