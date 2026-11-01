extends RefCounted
class_name EclipsePixelTheme

# DESIGN_GUIDELINE_1: hard-edged, integer-grid UI with a limited palette.
const INK := Color("#101923")
const PANEL := Color("#1c2a34")
const LIGHT := Color("#d4e0d2")
const SHADE := Color("#080d16")
const GOLD := Color("#e3bd72")
const ATTACK := Color("#3d4244")
const DASH := Color("#343f45")
const SKILL := Color("#45424a")
const SWAP := Color("#3b4541")

# 48x48 sprite frame, nine-sliced so corners stay exactly 12x12 pixels.
# Image pixels are drawn without AA; no web-style flat gradient/button border.
static func _sprite_frame(base_color: Color, active: bool = false) -> StyleBoxTexture:
	# 8x8 logical pixels, expanded x6 to exactly 48x48 physical pixels.
	var image := Image.create(48, 48, false, Image.FORMAT_RGBA8)
	image.fill(Color.TRANSPARENT)
	var palette := [SHADE, Color("#656a6b"), Color("#c0c3bb"), base_color, base_color.darkened(0.22)]
	if active:
		palette[2] = Color("#e1dfd0")
		palette[3] = base_color.darkened(0.25)
	var pattern := [
		"00111100",
		"01222210",
		"12333321",
		"12333321",
		"12333321",
		"12333321",
		"01444410",
		"00111100"
	]
	for y in range(8):
		for x in range(8):
			var index := int(pattern[y].substr(x, 1))
			image.fill_rect(Rect2i(x * 6, y * 6, 6, 6), palette[index])
	var texture := ImageTexture.create_from_image(image)
	var frame := StyleBoxTexture.new()
	frame.texture = texture
	frame.set_texture_margin_all(18.0)
	frame.set_content_margin_all(6.0)
	return frame

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
