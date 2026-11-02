extends Node2D

const PlayerScript = preload("res://scripts/player.gd")
const ForestScript = preload("res://scripts/forest_background.gd")
const JoystickScript = preload("res://scripts/touch_joystick.gd")
const PixelTheme = preload("res://scripts/pixel_theme.gd")
const PixelIcon = preload("res://scripts/pixel_action_icon.gd")
const PixelCombatHud = preload("res://scripts/pixel_combat_hud.gd")
const DamageNumber = preload("res://scripts/pixel_damage_number.gd")
const EnemyScript = preload("res://scripts/enemy.gd")
const ProjectileScript = preload("res://scripts/projectile.gd")
const WORLD_WIDTH := 1600
const FLOOR_Y := 585.0

var player: EclipsePlayer
var camera: Camera2D
var hud: CanvasLayer
var health_label: Label
var form_label: Label
var enemies_label: Label
var cooldown_label: Label
var status_label: Label
var equipment_label: Label
var inventory_panel: PanelContainer
var inventory_label: Label
var overlay: ColorRect
var overlay_title: Label
var overlay_hint: Label
var foes_left := 0
var ended := false
var touch_buttons: Array[Button] = []
var mobile_controls: Control
var joystick_jump := false
var profile_panel: PanelContainer
var pixel_hud: Control

func _ready() -> void:
	_register_inputs()
	var forest := ForestScript.new()
	forest.name = "ForestBackground"
	add_child(forest)
	_build_arena()
	_build_hud()
	_start_round()

func _register_inputs() -> void:
	var bindings := {
		"left": [KEY_A, KEY_LEFT],
		"right": [KEY_D, KEY_RIGHT],
		"jump": [KEY_SPACE, KEY_UP, KEY_W],
		"attack": [KEY_J, KEY_Z],
		"skill": [KEY_K, KEY_X],
		"swap": [KEY_L, KEY_C, KEY_TAB],
		"dash": [KEY_SHIFT],
		"restart": [KEY_R]
	}
	for action in bindings:
		if not InputMap.has_action(action):
			InputMap.add_action(action)
		for keycode in bindings[action]:
			var evt := InputEventKey.new()
			evt.keycode = keycode
			InputMap.action_add_event(action, evt)

func _build_arena() -> void:
	var floor_body := StaticBody2D.new()
	floor_body.collision_layer = 1
	floor_body.collision_mask = 0
	var floor_shape := CollisionShape2D.new()
	var rect := RectangleShape2D.new()
	rect.size = Vector2(WORLD_WIDTH + 200, 120)
	floor_shape.shape = rect
	floor_shape.position = Vector2(WORLD_WIDTH * 0.5, FLOOR_Y + 60)
	floor_body.add_child(floor_shape)
	add_child(floor_body)
	for x in [-50.0, float(WORLD_WIDTH) + 50.0]:
		var wall := StaticBody2D.new()
		wall.collision_layer = 1
		var wall_shape := CollisionShape2D.new()
		var wall_rect := RectangleShape2D.new()
		wall_rect.size = Vector2(100, 720)
		wall_shape.shape = wall_rect
		wall_shape.position = Vector2(x, 270)
		wall.add_child(wall_shape)
		add_child(wall)
	queue_redraw()

func _start_round() -> void:
	ended = false
	if is_instance_valid(player):
		player.queue_free()
		player = null
	for enemy in get_tree().get_nodes_in_group("enemies"):
		enemy.queue_free()
	for bullet in get_tree().get_nodes_in_group("bullets"):
		bullet.queue_free()
	player = PlayerScript.new() as EclipsePlayer
	player.position = Vector2(140, FLOOR_Y)
	add_child(player)
	player.health_changed.connect(_update_health)
	player.form_changed.connect(_update_form)
	player.equipment_changed.connect(_refresh_hud)
	player.projectile_requested.connect(_spawn_projectile)
	player.defeated.connect(_on_defeated)
	if not is_instance_valid(camera):
		camera = Camera2D.new()
		camera.position = Vector2(520, 350)
		camera.position_smoothing_enabled = true
		camera.position_smoothing_speed = 5.0
		camera.limit_left = 0
		camera.limit_right = WORLD_WIDTH
		camera.limit_top = 0
		camera.limit_bottom = 720
		add_child(camera)
		camera.make_current()
	$ForestBackground.camera_ref = camera
	var slots := [Vector2(575, FLOOR_Y), Vector2(800, FLOOR_Y), Vector2(1020, FLOOR_Y), Vector2(1285, FLOOR_Y)]
	foes_left = slots.size()
	for i in range(slots.size()):
		var enemy := EnemyScript.new() as EclipseEnemy
		enemy.configure(0 if i % 2 == 0 else 1)
		enemy.position = slots[i]
		enemy.player = player
		add_child(enemy)
		enemy.killed.connect(_on_enemy_killed)
		enemy.damage_taken.connect(_on_enemy_damage)
	overlay.visible = false
	if is_instance_valid(mobile_controls):
		mobile_controls.visible = true
	_refresh_hud()

func _spawn_projectile(spawn_pos: Vector2, direction: int, power: int, is_skill: bool) -> void:
	var projectile := ProjectileScript.new() as EclipseProjectile
	add_child(projectile)
	projectile.add_to_group("bullets")
	projectile.setup(spawn_pos, direction, power, is_skill)

func _on_enemy_damage(world_pos: Vector2, amount: int, kind: String) -> void:
	var popup := DamageNumber.new()
	popup.position = world_pos + Vector2(0, -50)
	popup.setup(amount, kind)
	add_child(popup)

func _on_enemy_killed(_enemy: EclipseEnemy) -> void:
	foes_left -= 1
	_refresh_hud()
	if foes_left <= 0 and not ended:
		_end_round(true)

func _on_defeated() -> void:
	_end_round(false)

func _end_round(won: bool) -> void:
	ended = true
	if is_instance_valid(player):
		player.controls_active = false
	overlay.visible = true
	if is_instance_valid(mobile_controls):
		mobile_controls.visible = false
	overlay_title.text = "STAGE CLEAR!" if won else "GAME OVER"
	overlay_title.add_theme_color_override("font_color", Color("8be4b5") if won else Color("ff8d99"))
	overlay_hint.text = "R 키 또는 아래 버튼으로 다시 시작"

func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("restart"):
		_start_round()
	if is_instance_valid(player) and not ended and event is InputEventKey and event.pressed and not event.echo:
		match event.keycode:
			KEY_1: player.cycle_equipment("weapon")
			KEY_2: player.cycle_equipment("armor")
			KEY_3: player.cycle_equipment("accessory")
			KEY_I: _toggle_inventory()

func _physics_process(_delta: float) -> void:
	if is_instance_valid(player):
		camera.global_position.x = player.global_position.x
		_refresh_hud()

func _update_health(_current: int, _maximum: int) -> void:
	if is_instance_valid(health_label):
		_refresh_hud()

func _update_form(_form: int) -> void:
	if is_instance_valid(form_label):
		_refresh_hud()

func _refresh_hud() -> void:
	if not is_instance_valid(player) or not is_instance_valid(health_label):
		return
	pixel_hud.hp = player.health
	pixel_hud.max_hp = player.max_health
	pixel_hud.foes = maxi(foes_left, 0)
	pixel_hud.skill_ratio = clampf(1.0 - player.skill_cooldown / 4.0, 0.0, 1.0)
	pixel_hud.queue_redraw()
	health_label.text = "HP  %d / %d" % [player.health, player.max_health]
	form_label.text = "FORM  %s" % ("검사 / 출혈 베기" if player.form == 0 else "화염술사 / 화염 폭발")
	enemies_label.text = "ENEMIES  %d" % maxi(foes_left, 0)
	cooldown_label.text = "SKILL %.1fs   SWAP %.1fs   DASH %.1fs" % [player.skill_cooldown, player.swap_cooldown, player.dash_cooldown]
	var bleeding := false
	for enemy in get_tree().get_nodes_in_group("enemies"):
		if is_instance_valid(enemy) and enemy.bleed_timer > 0:
			bleeding = true
			break
	status_label.text = "출혈 대상 있음! 화염 공격으로 폭발 가능" if bleeding else "검사 스킬로 출혈 → 화염술사로 교체 → 화염 공격"
	equipment_label.text = "1 무기: %s   2 방어구: %s   3 장신구: %s" % [player.equipment.name_for("weapon"), player.equipment.name_for("armor"), player.equipment.name_for("accessory")]
	if is_instance_valid(inventory_label):
		inventory_label.text = "장비 현황\n\n무기: %s\n방어구: %s\n장신구: %s\n\n공격력 %+d | 방어력 %d\n최대 체력 %d | 속도 %.2fx\n\n1 / 2 / 3 키 또는 화면 버튼으로 교체" % [player.equipment.name_for("weapon"),player.equipment.name_for("armor"),player.equipment.name_for("accessory"),int(player.equipment.stat("attack")),int(player.equipment.stat("defense")),player.max_health,player.equipment.stat("speed")]

func _label(text_value: String, xy: Vector2, size: int, tint: Color) -> Label:
	var label := Label.new()
	label.text = text_value
	label.position = xy
	label.add_theme_font_size_override("font_size", size)
	label.add_theme_color_override("font_color", tint)
	PixelTheme.label(label)
	return label

func _build_hud() -> void:
	hud = CanvasLayer.new()
	add_child(hud)
	var header := ColorRect.new()
	header.color = Color.TRANSPARENT
	header.position = Vector2(0, 0)
	header.size = Vector2(1280, 85)
	header.mouse_filter = Control.MOUSE_FILTER_IGNORE
	hud.add_child(header)
	# Reference layout deliberately has no top-left menu buttons or title.
	health_label = _label("", Vector2(235, 39), 20, Color("ff9ca9"))
	form_label = _label("", Vector2(448, 43), 18, Color("9ed7ff"))
	enemies_label = _label("", Vector2(1050, 43), 18, Color("f5c58a"))
	cooldown_label = _label("", Vector2(750, 14), 14, Color("a7b6d2"))
	status_label = _label("", Vector2(235, 68), 12, Color("ffe19b"))
	for item in [health_label, form_label, enemies_label, cooldown_label, status_label]:
		hud.add_child(item)
	for label_node in [health_label, form_label, enemies_label, cooldown_label, status_label]:
		label_node.visible = false
	equipment_label = _label("", Vector2(235, 91), 14, Color("d6e8b8"))
	hud.add_child(equipment_label)
	equipment_label.visible = false
	pixel_hud = PixelCombatHud.new()
	hud.add_child(pixel_hud)
	inventory_panel = PanelContainer.new()
	inventory_panel.position = Vector2(46,135)
	inventory_panel.custom_minimum_size = Vector2(450,310)
	inventory_panel.visible = false
	PixelTheme.panel(inventory_panel)
	hud.add_child(inventory_panel)
	inventory_label = _label("",Vector2(20,16),20,Color("#e4eacd"))
	inventory_label.custom_minimum_size = Vector2(400,270)
	inventory_panel.add_child(inventory_label)
	overlay = ColorRect.new()
	overlay.color = Color("#111a28", 0.94)
	overlay.position = Vector2(0, 0)
	overlay.size = Vector2(1280, 720)
	overlay.visible = false
	hud.add_child(overlay)
	overlay_title = _label("STAGE CLEAR!", Vector2(430, 250), 54, Color("8be4b5"))
	overlay.add_child(overlay_title)
	overlay_hint = _label("R 키로 다시 시작", Vector2(460, 340), 23, Color("f4f3e9"))
	overlay.add_child(overlay_hint)
	var retry := Button.new()
	retry.text = "다시 플레이"
	retry.position = Vector2(510, 412)
	retry.size = Vector2(260, 65)
	retry.pressed.connect(_start_round)
	PixelTheme.button(retry)
	overlay.add_child(retry)
	# Sketch-based Android landscape layout. HUD lives in 1280x720 canvas coordinates.
	mobile_controls = Control.new()
	mobile_controls.name = "MobileControls"
	mobile_controls.mouse_filter = Control.MOUSE_FILTER_IGNORE
	mobile_controls.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	hud.add_child(mobile_controls)
	var joystick: Control = JoystickScript.new()
	joystick.position = Vector2(35, 518)
	joystick.size = Vector2(160, 160)
	joystick.direction_changed.connect(_on_joystick)
	mobile_controls.add_child(joystick)
	_touch_action("공격", "attack", Vector2(1110, 540), Vector2(116, 116))
	_touch_action("돌진", "dash", Vector2(985, 560), Vector2(100, 100))
	_touch_action("교체", "swap", Vector2(995, 445), Vector2(90, 90))
	# Jump is triggered by pushing the left joystick upward; no separate jump button.
	# Keep the existing skill behavior, only switch its placeholder button to icon-only.
	_touch_action("스킬", "skill", Vector2(1115, 420), Vector2(94, 94))

func _touch_action(caption: String, action: String, xy: Vector2, dimensions: Vector2) -> void:
	var button := Button.new()
	button.text = ""
	button.tooltip_text = caption
	button.position = xy
	button.size = dimensions
	PixelTheme.action_button(button)
	button.button_down.connect(_on_touch_action.bind(action, true))
	button.button_up.connect(_on_touch_action.bind(action, false))
	mobile_controls.add_child(button)
	var icon_node: Control = PixelIcon.new()
	icon_node.action = action
	icon_node.position = Vector2.ZERO
	icon_node.size = dimensions
	icon_node.mouse_filter = Control.MOUSE_FILTER_IGNORE
	button.add_child(icon_node)
	touch_buttons.append(button)

func _on_joystick(horizontal: float, jump_held: bool) -> void:
	if not is_instance_valid(player):
		return
	player.set_virtual("left", horizontal < -0.23)
	player.set_virtual("right", horizontal > 0.23)
	# Jump uses press-edge buffering in the existing controller.
	if joystick_jump != jump_held:
		player.set_virtual("jump", jump_held)
	joystick_jump = jump_held

func _toggle_profile() -> void:
	if is_instance_valid(profile_panel):
		profile_panel.visible = not profile_panel.visible

func _toggle_inventory() -> void:
	if is_instance_valid(inventory_panel):
		inventory_panel.visible = not inventory_panel.visible

func _on_equip_button(slot: String) -> void:
	if is_instance_valid(player) and not ended:
		player.cycle_equipment(slot)

func _on_touch_action(action: String, state: bool) -> void:
	if is_instance_valid(player):
		player.set_virtual(action, state)

func _draw() -> void:
	# ForestBackground node renders the sky, trees and distant foliage.
	# Keep stage decorations here, without hiding the forest with an opaque rectangle.
	for x in range(0, WORLD_WIDTH, 175):
		draw_circle(Vector2(x + 55, 145 + x % 115), 2, Color("#d6f2b2", 0.5))
	draw_rect(Rect2(0, FLOOR_Y, WORLD_WIDTH, 135), Color("#2b4034"))
	draw_rect(Rect2(0, FLOOR_Y, WORLD_WIDTH, 8), Color("#73995d"))
	for x in range(0, WORLD_WIDTH, 80):
		draw_line(Vector2(x, FLOOR_Y + 30), Vector2(x + 55, FLOOR_Y + 30), Color("334661"), 3)
