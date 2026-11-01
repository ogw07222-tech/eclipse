extends RefCounted
class_name EclipsePixelTheme

# DESIGN_GUIDELINE_1: hard-edged, integer-grid UI with a limited palette.
const INK := Color("#101923")
const PANEL := Color("#1c2a34")
const LIGHT := Color("#d4e0d2")
const SHADE := Color("#080d16")
const GOLD := Color("#e3bd72")
const ATTACK := Color("#813a46")
const DASH := Color("#2b5970")
const SKILL := Color("#574879")
const SWAP := Color("#3d654d")

static func box(bg: Color, edge: Color = LIGHT, border: int = 3) -> StyleBoxFlat:
  var frame := StyleBoxFlat.new()
  frame.bg_color = bg
  frame.border_color = edge
  frame.set_border_width_all(border)
  frame.set_corner_radius_all(0)
  frame.anti_aliasing = false
  frame.content_margin_left = 8.0
  frame.content_margin_right = 8.0
  frame.content_margin_top = 6.0
  frame.content_margin_bottom = 6.0
  return frame

static func button(button_node: Button, role: String = "menu") -> void:
  var color := PANEL
  match role:
    "attack": color = ATTACK
    "dash": color = DASH
    "skill": color = SKILL
    "swap": color = SWAP
    "jump": color = DASH
  button_node.add_theme_stylebox_override("normal", box(color))
  button_node.add_theme_stylebox_override("hover", box(color.lightened(0.13), GOLD))
  button_node.add_theme_stylebox_override("pressed", box(color.darkened(0.3), GOLD, 4))
  button_node.add_theme_stylebox_override("focus", box(Color.TRANSPARENT, GOLD, 2))
  button_node.add_theme_stylebox_override("disabled", box(SHADE, Color("#65727b")))
  button_node.add_theme_color_override("font_color", LIGHT)
  button_node.add_theme_color_override("font_hover_color", Color.WHITE)
  button_node.add_theme_color_override("font_pressed_color", GOLD)
  button_node.add_theme_color_override("font_disabled_color", Color("#78878a"))
  button_node.add_theme_font_size_override("font_size", 18)
  button_node.focus_mode = Control.FOCUS_NONE

static func panel(panel_node: PanelContainer) -> void:
  panel_node.add_theme_stylebox_override("panel", box(PANEL, GOLD, 4))

static func label(label_node: Label) -> void:
  label_node.add_theme_color_override("font_shadow_color", SHADE)
  label_node.add_theme_constant_override("shadow_offset_x", 2)
  label_node.add_theme_constant_override("shadow_offset_y", 2)
