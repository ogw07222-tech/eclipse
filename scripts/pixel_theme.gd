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

# 48x48 sprite frame, nine-sliced so corners stay exactly 12x12 pixels.
# Image pixels are drawn without AA; no web-style flat gradient/button border.
static func _sprite_frame(base_color: Color, active: bool = false) -> StyleBoxTexture:
  var img := Image.create(48, 48, false, Image.FORMAT_RGBA8)
  img.fill(Color.TRANSPARENT)
  var dark := base_color.darkened(0.52)
  var mid := base_color.darkened(0.16)
  var lit := base_color.lightened(0.30)
  var center := base_color.lightened(0.08)
  if active:
    center = base_color.darkened(0.19)
    lit = GOLD
  img.fill_rect(Rect2i(8, 0, 32, 4), SHADE)
  img.fill_rect(Rect2i(4, 4, 40, 4), SHADE)
  img.fill_rect(Rect2i(0, 8, 48, 32), SHADE)
  img.fill_rect(Rect2i(4, 40, 40, 4), SHADE)
  img.fill_rect(Rect2i(8, 44, 32, 4), SHADE)
  img.fill_rect(Rect2i(8, 4, 32, 4), lit)
  img.fill_rect(Rect2i(4, 8, 40, 32), dark)
  img.fill_rect(Rect2i(8, 8, 32, 32), mid)
  img.fill_rect(Rect2i(12, 12, 24, 24), center)
  img.fill_rect(Rect2i(8, 8, 32, 3), lit)
  img.fill_rect(Rect2i(8, 11, 3, 25), lit)
  img.fill_rect(Rect2i(8, 36, 32, 4), base_color.darkened(0.43))
  img.fill_rect(Rect2i(37, 11, 3, 25), base_color.darkened(0.43))
  # stepped metal corner rivets and carved pixel highlights
  for xx in [9, 35]:
    for yy in [9, 35]:
      img.fill_rect(Rect2i(xx, yy, 4, 4), GOLD if active else LIGHT)
      img.set_pixel(xx + 1, yy + 1, SHADE)
  var texture := ImageTexture.create_from_image(img)
  var style := StyleBoxTexture.new()
  style.texture = texture
  style.set_texture_margin_all(12.0)
  style.set_content_margin_all(7.0)
  return style

static func box(bg: Color, edge: Color = LIGHT, border: int = 3) -> StyleBoxFlat:
  var frame := StyleBoxFlat.new()
  frame.bg_color = bg
  frame.border_color = edge
  frame.set_border_width_all(border)
  frame.set_corner_radius_all(0)
  frame.anti_aliasing = false
  frame.set_content_margin_all(8.0)
  return frame

static func button(button_node: Button, role: String = "menu") -> void:
  var color := PANEL
  match role:
    "attack": color = ATTACK
    "dash": color = DASH
    "skill": color = SKILL
    "swap": color = SWAP
    "jump": color = DASH
  button_node.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
  button_node.add_theme_stylebox_override("normal", _sprite_frame(color))
  button_node.add_theme_stylebox_override("hover", _sprite_frame(color.lightened(0.10)))
  button_node.add_theme_stylebox_override("pressed", _sprite_frame(color, true))
  button_node.add_theme_stylebox_override("disabled", _sprite_frame(SHADE))
  button_node.add_theme_stylebox_override("focus", box(Color.TRANSPARENT, GOLD, 2))
  button_node.add_theme_color_override("font_color", LIGHT)
  button_node.add_theme_color_override("font_hover_color", Color.WHITE)
  button_node.add_theme_color_override("font_pressed_color", GOLD)
  button_node.add_theme_color_override("font_disabled_color", Color("#78878a"))
  button_node.add_theme_font_size_override("font_size", 18)
  button_node.add_theme_constant_override("outline_size", 2)
  button_node.add_theme_color_override("font_outline_color", SHADE)
  button_node.focus_mode = Control.FOCUS_NONE

static func panel(panel_node: PanelContainer) -> void:
  panel_node.add_theme_stylebox_override("panel", box(PANEL, GOLD, 4))

static func label(label_node: Label) -> void:
  label_node.add_theme_color_override("font_shadow_color", SHADE)
  label_node.add_theme_constant_override("shadow_offset_x", 2)
  label_node.add_theme_constant_override("shadow_offset_y", 2)
