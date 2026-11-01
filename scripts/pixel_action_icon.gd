extends Control
# Design Guideline 1: 8x8 coarse pixel glyphs at 6 screen pixels per art pixel.
var action := "attack"
const PIXELS := {
  "attack": ["......WW",".....WWW","....WWW.","...WWW..","..WW....",".WWKK...","KKK.....","K......."],
  "dash": ["........","...WW...","..WWWW..","WWWWWWWW","..WWWW..","...WW...","........","........"],
  "swap": ["..WWWW..",".WW..WW.",".....WW.","....WW..","...WW...","WW......",".WW..WW.","..WWWW.."],
  "jump": ["...WW...","..WWWW..",".WWWWWW.","...WW...","...WW...","...WW...","...WW...","........"]
}
func _ready() -> void:
  mouse_filter = Control.MOUSE_FILTER_IGNORE
  queue_redraw()

func _draw() -> void:
  var rows: Array = PIXELS.get(action, PIXELS["attack"])
  var block := 6.0
  var offset := ((size - Vector2(48.0, 48.0)) * 0.5).round()
  for y in range(8):
    var row: String = rows[y]
    for x in range(8):
      var symbol := row.substr(x, 1)
      if symbol == ".":
        continue
      draw_rect(Rect2(offset + Vector2(x, y) * block, Vector2(block, block)),
        Color("#c6c7bd") if symbol == "W" else Color("#727a7b"))
