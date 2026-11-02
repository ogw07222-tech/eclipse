extends Node2D
# Tiny 3x5 bitmap digits: no font dependency, no anti-aliasing.
const GLYPHS := {
  "0":["111","101","101","101","111"],
  "1":["010","110","010","010","111"],
  "2":["111","001","111","100","111"],
  "3":["111","001","111","001","111"],
  "4":["101","101","111","001","001"],
  "5":["111","100","111","001","111"],
  "6":["111","100","111","101","111"],
  "7":["111","001","010","010","010"],
  "8":["111","101","111","101","111"],
  "9":["111","101","111","001","111"],
  "-":["000","000","111","000","000"]
}
var damage := 0
var duration := 0.60
var elapsed := 0.0
var tint := Color("#f2d48c")
func setup(value: int, kind: String = "normal") -> void:
  damage = value
  if kind == "fire":
    tint = Color("#ff9155")
  elif kind == "bleed":
    tint = Color("#e76878")
func _process(delta: float) -> void:
  elapsed += delta
  if elapsed >= duration:
    queue_free()
    return
  position.y = roundf(position.y - 27.0 * delta)
  queue_redraw()
func _draw() -> void:
  var chars := str(damage)
  var scale_val := 3.0
  var width_val := float(chars.length()) * 4.0 * scale_val
  for i in range(chars.length()):
    var glyph: Array = GLYPHS.get(chars[i], GLYPHS["0"])
    for row in range(5):
      var pixels: String = glyph[row]
      for col in range(3):
        if pixels[col] == "1":
          var point := Vector2(roundf(float(i*4+col)*scale_val-width_val*0.5),float(row)*scale_val - 36.0)
          draw_rect(Rect2(point + Vector2(2,2),Vector2(scale_val,scale_val)),Color("#0b101b"))
          draw_rect(Rect2(point,Vector2(scale_val,scale_val)),tint)
