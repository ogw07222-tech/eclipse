extends Control
# 16x16 bitmap glyphs, rendered as crisp 4px blocks.
var action := "attack"
const PIXELS := {
  "attack": ["...........W....","..........WWW...",".........WWW....","........WWW.....",".......WWW......","......WWW.......",".....WWW........","....WWW.........","...WWW..........","..WWW...........","...K............","....KK..........",".....KK.........","......K.........","................","................"],
  "dash": ["................","................","..W.............","..WWW...........","..WWWWW.........","..WWWWWWW.......","..WWWWWWWWW.....","..WWWWWWWWWWW...","..WWWWWWWWW.....","..WWWWWWW.......","..WWWWW.........","..WWW...........","..W.............","................","................","................"],
  "skill": ["......O.........",".....OOO........","....OOOO........",".....OOOO.......","...OOOOOO.......","....OOOOOO......","...OOOOOOOO.....","....OOOOOO......",".....OOOO.......","......OO........","......OO........",".....OOOO.......","....OOOOOO......","................","................","................"],
  "swap": ["................","....GGGGGG......","...G......GG....","..G.........G...","..G.............","..G.............","..G.............","..GGGG......G...","...........GG...","...........G....","..GG.......G....","...GG......G....","....GGGGGGG.....","................","................","................"],
  "jump": [".......B........","......BBB.......",".....BBBBB......","....BBBBBBB.....",".......B........",".......B........",".......B........",".......B........",".......B........",".......B........",".......B........",".......B........","................","................","................","................"]
}
func _ready() -> void:
  mouse_filter = Control.MOUSE_FILTER_IGNORE
  queue_redraw()
func _draw() -> void:
  var icon_rows: Array = PIXELS.get(action, PIXELS["attack"])
  var unit := 3.0
  var offset := (size - Vector2(16, 16) * unit) * 0.5
  for y in range(icon_rows.size()):
    var line: String = icon_rows[y]
    for x in range(line.length()):
      var symbol := line.substr(x, 1)
      if symbol == ".":
        continue
      var tint := Color("#e9e4c7")
      match symbol:
        "W": tint = Color("#e5eef3")
        "K": tint = Color("#a2b1c1")
        "O": tint = Color("#f4a357")
        "G": tint = Color("#9fdbac")
        "B": tint = Color("#91cfe6")
      draw_rect(Rect2(offset + Vector2(x, y) * unit, Vector2(unit, unit)), tint)
