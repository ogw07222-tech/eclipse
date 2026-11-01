extends Button
# Pixel cooldown: the colored region fills from bottom to top.
var actor: Node
var slot := 0
var is_ultimate := false
var previous_remaining := -1.0
var ready_flash := 0.0

func _process(delta: float) -> void:
  if not is_instance_valid(actor):
    return
  var remaining := _remaining()
  if previous_remaining > 0.0 and remaining <= 0.0:
    ready_flash = 0.42
  previous_remaining = remaining
  ready_flash = maxf(0.0, ready_flash - delta)
  queue_redraw()

func _remaining() -> float:
  if not is_instance_valid(actor):
    return 0.0
  if is_ultimate:
    return float(actor.ultimate_cooldown)
  return float(actor.skill_cooldowns[slot])

func _draw() -> void:
  var duration := float(actor.ULTIMATE_DURATION) if is_ultimate and is_instance_valid(actor) else 1.0
  if is_instance_valid(actor) and not is_ultimate:
    duration = float(actor.SKILL_DURATIONS[slot])
  var remaining := _remaining()
  var fill_fraction := clampf(1.0 - remaining / maxf(0.01, duration), 0.0, 1.0)
  var fill_height := floorf((size.y - 8.0) * fill_fraction / 4.0) * 4.0
  if remaining > 0.0:
    draw_rect(Rect2(4.0, size.y - 4.0 - fill_height, size.x - 8.0, fill_height), Color("#71b99c", 0.55))
  if ready_flash > 0.0:
    var frame_color := Color("#ffe6a0") if int(ready_flash * 16.0) % 2 == 0 else Color("#fffaf0")
    draw_rect(Rect2(2.0, 2.0, size.x - 4.0, size.y - 4.0), frame_color, false, 4.0, false)
