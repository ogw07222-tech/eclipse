extends Node2D
# Drawn in Godot: no external texture required. Parallax is computed from camera position.
const LAYERS = [
  {"factor": 0.15, "trunk": Color("#273e48"), "crown": Color("#244b47"), "size": 0.65},
  {"factor": 0.35, "trunk": Color("#29413a"), "crown": Color("#286247"), "size": 0.9},
  {"factor": 0.60, "trunk": Color("#33452c"), "crown": Color("#39734b"), "size": 1.15}
]
var camera_ref: Camera2D
var frame_time := 0.0
func _ready() -> void:
  z_index = -20
  mouse_filter_fallback()
func mouse_filter_fallback() -> void:
  pass
func _process(delta: float) -> void:
  frame_time += delta
  queue_redraw()
func _draw() -> void:
  draw_rect(Rect2(0, 0, 1600, 720), Color("#152334"))
  draw_circle(Vector2(1190, 130), 73, Color("#c5d4ad", 0.24))
  for i in range(70):
    var sx := float((i * 221 + 73) % 1600)
    var sy := float((i * 97 + 67) % 320)
    draw_circle(Vector2(sx, sy), 1.5, Color("#c9e4ca", 0.45))
  for layer_index in range(LAYERS.size()):
    var data: Dictionary = LAYERS[layer_index]
    var fac: float = data["factor"]
    for i in range(25):
      var base_x := float(i * 83 + (layer_index * 37)) - 90.0
      var offset := 0.0
      if is_instance_valid(camera_ref):
        offset = (camera_ref.global_position.x - 640.0) * fac * 0.18
      var x := base_x - offset
      var size_val: float = data["size"]
      var variation := float((i * 17 + layer_index * 11) % 13)
      var ground_y := 590.0 + variation
      var top_y := ground_y - (220.0 + variation * 7.0) * size_val
      var w := (14.0 + variation * 0.5) * size_val
      draw_rect(Rect2(x - w * 0.5, top_y, w, ground_y - top_y), data["trunk"])
      for n in range(3):
        var h := 80.0 * size_val
        var crown_y := top_y + n * 46.0 * size_val
        draw_colored_polygon(PackedVector2Array([Vector2(x, crown_y - h), Vector2(x - 57.0 * size_val, crown_y + 22.0 * size_val), Vector2(x + 57.0 * size_val, crown_y + 22.0 * size_val)]), data["crown"])
  draw_rect(Rect2(0, 580, 1600, 14), Color("#466d42"))
  for i in range(140):
    var x := float((i * 53) % 1600)
    var sway := sin(frame_time * 1.4 + float(i)) * 2.0
    draw_line(Vector2(x, 585), Vector2(x + sway, 575 - float(i % 5)), Color("#72a55e"), 2.0)
