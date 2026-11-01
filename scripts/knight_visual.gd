extends AnimatedSprite2D
# The animations are PNG assets; this script only slices frames and selects states.
# Copy assets/knight_sheet_48x64.png into the Godot project to enable the new art.
const SHEET := "res://assets/knight_sheet_48x64.png"
const FRAME := Vector2i(48, 64)
const COLS := 8
const COUNTS := {"idle":6, "run":8, "jump":4, "fall":3, "attack1":6, "attack2":6, "attack3":8, "dash":5, "hurt":3, "death":6}
var actor: CharacterBody2D
var previous_hp := -1
var previous_attack := 0.0
var attack_time := 0.0
var hurt_time := 0.0
var combo := 1

func _ready() -> void:
	actor = get_parent() as CharacterBody2D
	if not ResourceLoader.exists(SHEET):
		push_warning("Missing 48x64 knight PNG: " + SHEET)
		visible = false
		return
	var atlas: Texture2D = load(SHEET)
	var frames := SpriteFrames.new()
	for existing in frames.get_animation_names():
		frames.remove_animation(existing)
	var idx := 0
	for key in COUNTS.keys():
		var name := str(key)
		frames.add_animation(name)
		frames.set_animation_speed(name, 12.0)
		frames.set_animation_loop(name, name == "idle" or name == "run" or name == "fall")
		for i in range(int(COUNTS[name])):
			var tile := AtlasTexture.new()
			tile.atlas = atlas
			tile.region = Rect2((idx % COLS) * FRAME.x, int(idx / COLS) * FRAME.y, FRAME.x, FRAME.y)
			frames.add_frame(name, tile)
			idx += 1
	sprite_frames = frames
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	# Keep approximately the same on-screen size; source atlas has real 48x64 art.
	scale = Vector2(2.0, 2.0)
	position = Vector2(0, -64)
	play("idle")
	previous_hp = int(actor.get("health"))

func _physics_process(delta: float) -> void:
	if not is_instance_valid(actor) or sprite_frames == null:
		return
	var hp := int(actor.get("health"))
	if hp < previous_hp:
		hurt_time = 0.23
	previous_hp = hp
	hurt_time = maxf(0.0, hurt_time - delta)
	var atk := float(actor.get("attack_flash"))
	if atk > 0.0 and previous_attack <= 0.0:
		attack_time = 0.36
		combo = clampi(int(actor.get("combo_index")), 1, 3)
	previous_attack = atk
	attack_time = maxf(0.0, attack_time - delta)
	visible = int(actor.get("form")) == 0
	flip_h = int(actor.get("facing")) < 0
	var next_anim := "idle"
	if hp <= 0:
		next_anim = "death"
	elif hurt_time > 0:
		next_anim = "hurt"
	elif attack_time > 0:
		next_anim = "attack%d" % combo
	elif float(actor.get("dash_left")) > 0.0:
		next_anim = "dash"
	elif not actor.is_on_floor():
		next_anim = "jump" if actor.velocity.y < 0.0 else "fall"
	elif absf(actor.velocity.x) > 20.0:
		next_anim = "run"
	if animation != next_anim:
		play(next_anim)
