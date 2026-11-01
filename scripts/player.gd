extends CharacterBody2D
class_name EclipsePlayer

signal health_changed(current: int, maximum: int)
signal form_changed(form: int)
signal projectile_requested(position_value: Vector2, direction_value: int, damage: int, skill: bool)
signal equipment_changed
signal defeated

const EquipmentScript = preload("res://scripts/equipment.gd")
const KNIGHT_VISUAL_PATH := "res://scripts/knight_visual.gd"
var equipment = EquipmentScript.new()
const MOVE_SPEED := 280.0
const JUMP_SPEED := -650.0 # approx 169 px apex (3x 56 px collider) at 1250 gravity
const GRAVITY := 1250.0
const DASH_SPEED := 780.0
const DASH_DURATION := 0.15
const COYOTE_TIME := 0.11
const JUMP_BUFFER := 0.13

var max_health := 100
var health := 100
var form := 0 # 0 = sword / 1 = fire mage
var facing := 1
var dash_left := 0.0
var dash_cooldown := 0.0
var attack_cooldown := 0.0
var skill_cooldown := 0.0
var swap_cooldown := 0.0
var invulnerable := 0.0
var coyote := 0.0
var jump_buffer := 0.0
var attack_flash := 0.0
var combo_index := 0
var combo_window := 0.0
var hit_stop := 0.0
var swap_attack_timer := 0.0
var virtual_input := {"left": false, "right": false, "jump": false, "attack": false, "skill": false, "swap": false, "dash": false}
var previous_virtual := virtual_input.duplicate()
var controls_active := true

func _ready() -> void:
	add_to_group("player")
	var knight_script: Script = load(KNIGHT_VISUAL_PATH)
	if knight_script != null:
		var appearance: Node = knight_script.new()
		appearance.name = "KnightVisual"
		add_child(appearance)
	else:
		push_warning("Knight visual failed to load; procedural player remains usable.")
	collision_layer = 2
	collision_mask = 1
	var shape := CollisionShape2D.new()
	var rect := RectangleShape2D.new()
	rect.size = Vector2(30, 56)
	shape.shape = rect
	shape.position = Vector2(0, -28)
	add_child(shape)
	refresh_equipment_health()
	health_changed.emit(health, max_health)
	form_changed.emit(form)
	equipment_changed.emit()

func pressed(action: String) -> bool:
	return Input.is_action_pressed(action) or virtual_input.get(action, false)

func just_pressed(action: String) -> bool:
	return Input.is_action_just_pressed(action) or (virtual_input.get(action, false) and not previous_virtual.get(action, false))

func set_virtual(action: String, active: bool) -> void:
	virtual_input[action] = active

func _physics_process(delta: float) -> void:
	if not controls_active:
		return
	dash_left = maxf(0.0, dash_left - delta)
	dash_cooldown = maxf(0.0, dash_cooldown - delta)
	attack_cooldown = maxf(0.0, attack_cooldown - delta)
	skill_cooldown = maxf(0.0, skill_cooldown - delta)
	swap_cooldown = maxf(0.0, swap_cooldown - delta)
	invulnerable = maxf(0.0, invulnerable - delta)
	attack_flash = maxf(0.0, attack_flash - delta)
	combo_window = maxf(0.0, combo_window - delta)
	swap_attack_timer = maxf(0.0, swap_attack_timer - delta)
	if hit_stop > 0.0:
		hit_stop = maxf(0.0, hit_stop - delta)
		previous_virtual = virtual_input.duplicate()
		queue_redraw()
		return
	if combo_window <= 0.0:
		combo_index = 0
	coyote = COYOTE_TIME if is_on_floor() else maxf(0.0, coyote - delta)
	if just_pressed("jump"):
		jump_buffer = JUMP_BUFFER
	else:
		jump_buffer = maxf(0.0, jump_buffer - delta)
	var axis := float(pressed("right")) - float(pressed("left"))
	if absf(axis) > 0.1:
		facing = 1 if axis > 0 else -1
	if just_pressed("swap") and swap_cooldown <= 0.0:
		form = 1 - form
		swap_cooldown = 1.0
		attack_flash = 0.22
		combo_index = 0
		combo_window = 0.0
		swap_attack_timer = 0.24
		# A swap triggers its own distinct entry attack, independent of basic attack cooldown.
		if form == 0:
			melee_hit(13, false, 112.0)
		else:
			projectile_requested.emit(global_position + Vector2(facing * 27, -33), facing, 14, false)
		form_changed.emit(form)
	if just_pressed("dash") and dash_cooldown <= 0.0:
		dash_left = DASH_DURATION
		dash_cooldown = 0.85
		invulnerable = maxf(invulnerable, DASH_DURATION)
	if dash_left > 0.0:
		velocity.x = DASH_SPEED * facing * equipment.stat("speed")
		velocity.y = 0.0
	else:
		velocity.x = axis * MOVE_SPEED * equipment.stat("speed")
		velocity.y += GRAVITY * delta
		if jump_buffer > 0.0 and coyote > 0.0:
			velocity.y = JUMP_SPEED
			coyote = 0.0
			jump_buffer = 0.0
	if just_pressed("attack") and attack_cooldown <= 0.0:
		basic_attack()
	if just_pressed("skill") and skill_cooldown <= 0.0:
		use_skill()
	move_and_slide()
	position.x = clampf(position.x, 25.0, 1550.0)
	previous_virtual = virtual_input.duplicate()
	queue_redraw()

func basic_attack() -> void:
	attack_flash = 0.16
	if form == 0:
		combo_index = (combo_index % 3) + 1 if combo_window > 0.0 else 1
		combo_window = 0.8
		attack_cooldown = [0.0, 0.23, 0.25, 0.42][combo_index] / equipment.stat("speed")
		var power: int = [0, 12, 15, 23][combo_index]
		melee_hit(power + int(equipment.stat("attack")), combo_index == 3, (115.0 if combo_index == 3 else 96.0) * equipment.stat("range"))
	else:
		combo_index = 0
		attack_cooldown = 0.47
		projectile_requested.emit(global_position + Vector2(facing * 27, -33), facing, 11 + int(equipment.stat("attack")), false)

func use_skill() -> void:
	skill_cooldown = 3.4 if form == 0 else 4.1
	attack_flash = 0.28
	if form == 0:
		melee_hit(23 + int(equipment.stat("attack")), true, 96.0 * equipment.stat("range"))
	else:
		projectile_requested.emit(global_position + Vector2(facing * 27, -33), facing, 25 + int(equipment.stat("attack")), true)

func melee_hit(damage: int, inflict_bleed: bool, reach: float = 96.0) -> void:
	for enemy in get_tree().get_nodes_in_group("enemies"):
		if not is_instance_valid(enemy) or enemy.is_dead:
			continue
		var displacement: Vector2 = enemy.global_position - global_position
		if displacement.x * facing >= -9.0 and displacement.x * facing < reach and absf(displacement.y) < 67.0:
			enemy.receive_hit(damage, "blade", inflict_bleed)
			hit_stop = maxf(hit_stop, 0.045 if damage < 20 else 0.075)

func take_damage(amount: int) -> void:
	if invulnerable > 0.0 or health <= 0:
		return
	health = maxi(health - maxi(1, amount - int(equipment.stat("defense"))), 0)
	invulnerable = 0.7
	attack_flash = 0.28
	combo_index = 0
	combo_window = 0.0
	health_changed.emit(health, max_health)
	if health == 0:
		controls_active = false
		defeated.emit()
	queue_redraw()

func refresh_equipment_health() -> void:
	var old_max := max_health
	max_health = 100 + int(equipment.stat("hp"))
	health = clampi(health + max_health - old_max, 1, max_health)

func cycle_equipment(slot: String) -> void:
	equipment.cycle(slot)
	refresh_equipment_health()
	health_changed.emit(health, max_health)
	equipment_changed.emit()

func reset_character() -> void:
	max_health = 100 + int(equipment.stat("hp"))
	health = max_health
	form = 0
	facing = 1
	velocity = Vector2.ZERO
	dash_left = 0.0
	dash_cooldown = 0.0
	attack_cooldown = 0.0
	skill_cooldown = 0.0
	swap_cooldown = 0.0
	invulnerable = 0.0
	combo_index = 0
	combo_window = 0.0
	hit_stop = 0.0
	swap_attack_timer = 0.0
	controls_active = true
	health_changed.emit(health, max_health)
	form_changed.emit(form)

func _draw() -> void:
	if form == 0 and has_node("KnightVisual") and $KnightVisual is AnimatedSprite2D and $KnightVisual.sprite_frames != null and $KnightVisual.visible:
		return
	var core_color := Color("80d6ff") if form == 0 else Color("f69a5c")
	var shadow := Color("19314d") if form == 0 else Color("5a2837")
	if invulnerable > 0.0 and Engine.get_physics_frames() % 6 < 3:
		core_color.a = 0.4
	draw_circle(Vector2(0, 0), 20, Color(0.03, 0.04, 0.08, 0.22))
	draw_rect(Rect2(-15, -55, 30, 40), shadow)
	draw_rect(Rect2(-12, -52, 24, 30), core_color)
	draw_circle(Vector2(0, -60), 15, Color("f6d3b3"))
	draw_rect(Rect2(-12, -70, 24, 10), shadow)
	draw_rect(Rect2(facing * 7 - 2, -62, 4, 4), Color("151b2c"))
	draw_rect(Rect2(-12, -15, 10, 15), shadow)
	draw_rect(Rect2(2, -15, 10, 15), shadow)
	if form == 0:
		draw_line(Vector2(facing * 12, -37), Vector2(facing * 35, -49), Color("dbe9f8"), 6)
	else:
		draw_line(Vector2(facing * 15, -35), Vector2(facing * 22, -61), Color("8f6ad4"), 6)
		draw_circle(Vector2(facing * 22, -64), 7, Color("ffad5b"))
	if swap_attack_timer > 0.0:
		draw_arc(Vector2(0, -36), 44.0, 0.0, TAU, 24, Color("fff2b6"), 4.0)
	if attack_flash > 0.0:
		var effect_color := Color("a9f0ff") if form == 0 else Color("ff9d51")
		draw_arc(Vector2(facing * 16, -36), 55, -1.1 if facing == 1 else 2.0, 1.15 if facing == 1 else 4.3, 18, effect_color, 6.0)
