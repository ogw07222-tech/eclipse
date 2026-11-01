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
  var center := size * 0.5
  draw_circle(center, 73.0, Color("#091728", 0.49))
  draw_arc(center, 69.0, 0.0, TAU, 44, Color("#d5e4dd", 0.64), 3.0)
  draw_circle(center + stick * MAX_RADIUS, 28.0, Color("#47739a", 0.8))
  draw_arc(center + stick * MAX_RADIUS, 28.0, 0.0, TAU, 32, Color("#e1f1ff", 0.8), 2.0)
