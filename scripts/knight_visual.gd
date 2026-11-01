extends AnimatedSprite2D
# The knight is art, not geometry. SpriteFrames refer only to transparent PNG pixels.
const SHEET := "res://assets/knight_sheet_48x64_v2.png"
const FRAME := Vector2i(48, 64)
const COLUMNS := 8
const ANIMS := {"idle":6, "run":8, "jump":4, "fall":3, "attack1":6, "attack2":6, "attack3":8, "dash":5, "hurt":3, "death":6}
var actor: CharacterBody2D
var previous_hp := -1
var previous_attack := 0.0
var attack_time := 0.0
var hurt_time := 0.0
var combo := 1

func _ready() -> void:
	actor = get_parent() as CharacterBody2D
	if not ResourceLoader.exists(SHEET):
		push_warning("Missing new 48x64 knight image: " + SHEET)
		visible = false
		return
	var atlas: Texture2D = load(SHEET)
	var frames := SpriteFrames.new()
	for existing in frames.get_animation_names():
		frames.remove_animation(existing)
	var idx := 0
	for name_value in ANIMS.keys():
		var anim := str(name_value)
		frames.add_animation(anim)
		frames.set_animation_speed(anim, 12.0)
		frames.set_animation_loop(anim, anim == "idle" or anim == "run" or anim == "fall")
		for i in range(int(ANIMS[anim])):
			var tile := AtlasTexture.new()
			tile.atlas = atlas
			tile.region = Rect2((idx % COLUMNS) * FRAME.x, int(idx / COLUMNS) * FRAME.y, FRAME.x, FRAME.y)
			frames.add_frame(anim, tile)
			idx += 1
	sprite_frames = frames
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	scale = Vector2(2, 2)
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
		next_anim = "jump" if actor.velocity.y < 0 else "fall"
	elif absf(actor.velocity.x) > 20:
		next_anim = "run"
	if animation != next_anim:
		play(next_anim)
