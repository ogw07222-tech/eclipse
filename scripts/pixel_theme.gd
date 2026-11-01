extends RefCounted
class_name EclipsePixelTheme

# DESIGN_GUIDELINE_1: hard-edged, integer-grid UI with a limited palette.
const INK := Color("#101923")
const PANEL := Color("#1c2a34")
const LIGHT := Color("#c6d0c5")
const SHADE := Color("#080d16")
const GOLD := Color("#e3bd72")
const ATTACK := Color("#283d3c")
const DASH := Color("#283b40")
const SKILL := Color("#34383d")
const SWAP := Color("#304039")

# 48x48 sprite frame, nine-sliced so corners stay exactly 12x12 pixels.
# Image pixels are drawn without AA; no web-style flat gradient/button border.
static func _sprite_frame(base_color: Color, active: bool = false) -> StyleBoxTexture:
	# 8x8 logical pixels, expanded x6 to exactly 48x48 physical pixels.
	var image := Image.create(48, 48, false, Image.FORMAT_RGBA8)
	image.fill(Color.TRANSPARENT)
	var palette := [Color.TRANSPARENT, Color("#142627", 0.83), Color("#89978b", 0.82), Color(base_color.r, base_color.g, base_color.b, 0.75), Color("#182a2b", 0.84)]
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

# Pixel-round 16x16 buttons. Nearest-neighbor enlargement makes every pixel visible.
static func _round_frame(active: bool = false) -> StyleBoxTexture:
	var img := Image.create(32, 32, false, Image.FORMAT_RGBA8)
	img.fill(Color.TRANSPARENT)
	var outer := Color("#0e191d", 0.93)
	var rim := Color("#92a09b", 0.94) if not active else Color("#e0e0d2")
	var fill := Color("#26363b", 0.82) if not active else Color("#425157", 0.96)
	var low := Color("#53615c", 0.92)
	for y in range(32):
		for x in range(32):
			var dx := float(x) - 15.5
			var dy := float(y) - 15.5
			var d2 := dx * dx + dy * dy
			var color := Color.TRANSPARENT
			if d2 <= 245.0:
				color = outer
			if d2 <= 203.0:
				color = rim
			if d2 <= 167.0:
				color = fill
			if d2 <= 203.0 and y >= 24:
				color = low
			if d2 <= 167.0 and y >= 24:
				color = fill
			img.set_pixel(x, y, color)
	var frame := StyleBoxTexture.new()
	frame.texture = ImageTexture.create_from_image(img)
	frame.set_content_margin_all(0.0)
	return frame

static func action_button(button_node: Button) -> void:
	button_node.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	button_node.add_theme_stylebox_override("normal", _round_frame(false))
	button_node.add_theme_stylebox_override("hover", _round_frame(false))
	button_node.add_theme_stylebox_override("pressed", _round_frame(true))
	button_node.add_theme_stylebox_override("disabled", _round_frame(false))
	button_node.add_theme_stylebox_override("focus", box(Color.TRANSPARENT, Color.TRANSPARENT, 0))
	button_node.focus_mode = Control.FOCUS_NONE

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
