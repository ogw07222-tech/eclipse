extends AnimatedSprite2D
# ECLIPSE knight: native 48x64 coarse-pixel frames. No external PNG dependency.
# Every animation is drawn at 48x64, then enlarged with nearest filtering.
const ART_SIZE := Vector2i(48, 64)
const ANIMS := {"idle": 6, "run": 8, "jump": 4, "fall": 3, "attack1": 6, "attack2": 6, "attack3": 8, "dash": 5, "hurt": 3, "death": 6}
const OUTLINE := Color("#0b101b")
const STEEL := Color("#ccd1d5")
const BRIGHT := Color("#f0f0e7")
const SHADOW := Color("#757e91")
const BLUE := Color("#1d4c9a")
const BLUE_LIGHT := Color("#3776c6")
const GOLD := Color("#e7b94c")
const LEATHER := Color("#764a2e")
var actor: CharacterBody2D
var previous_hp := -1
var previous_attack := 0.0
var attack_time := 0.0
var hurt_time := 0.0
var combo := 1

func _rect(img: Image, x: int, y: int, w: int, h: int, color: Color) -> void:
	img.fill_rect(Rect2i(x, y, w, h), color)

func _part(img: Image, x: int, y: int, w: int, h: int, color: Color) -> void:
	_rect(img, x - 1, y - 1, w + 2, h + 2, OUTLINE)
	_rect(img, x, y, w, h, color)

func _frame(anim: String, frame_index: int, total: int) -> ImageTexture:
	var img := Image.create(ART_SIZE.x, ART_SIZE.y, false, Image.FORMAT_RGBA8)
	img.fill(Color.TRANSPARENT)
	var bob := 0
	var stride := 0
	var lean := 0
	if anim == "idle":
		bob = 1 if frame_index >= 3 else 0
	elif anim == "run":
		bob = (frame_index % 4) / 2
		stride = [-4, -2, 2, 4, 4, 2, -2, -4][frame_index]
		lean = 2
	elif anim == "jump":
		bob = -3
		stride = -2
	elif anim == "fall":
		bob = 2
		stride = 2
	elif anim == "dash":
		lean = 5
		bob = 1
	elif anim == "hurt":
		lean = -3
		bob = 2
	elif anim == "death":
		bob = mini(frame_index * 3, 11)
		lean = -mini(frame_index * 2, 9)
	var y := bob
	# Cape silhouette extends to the right of the torso.
	_part(img, 26 + lean, 23 + y, 13, 25, BLUE)
	_rect(img, 31 + lean, 27 + y, 8, 17, BLUE_LIGHT)
	_rect(img, 34 + lean, 45 + y, 8, 3, GOLD)
	# Two distinctive, animated boots.
	_part(img, 16 + stride / 2 + lean, 51 + y, 9, 9, LEATHER)
	_part(img, 26 - stride / 2 + lean, 51 + y, 9, 9, LEATHER)
	_rect(img, 14 + stride / 2 + lean, 59 + y, 11, 3, OUTLINE)
	_rect(img, 26 - stride / 2 + lean, 59 + y, 11, 3, OUTLINE)
	# Wide armor shoulders, narrow waist, gold emblem.
	_part(img, 16 + lean, 26 + y, 18, 25, SHADOW)
	_rect(img, 19 + lean, 28 + y, 13, 18, STEEL)
	_rect(img, 19 + lean, 43 + y, 13, 5, BLUE)
	_rect(img, 22 + lean, 43 + y, 6, 4, GOLD)
	_part(img, 11 + lean, 26 + y, 9, 10, STEEL)
	_rect(img, 12 + lean, 27 + y, 6, 3, BRIGHT)
	_part(img, 29 + lean, 26 + y, 7, 9, STEEL)
	# Helmet must read clearly even at native 48x64.
	_part(img, 17 + lean, 7 + y, 17, 20, SHADOW)
	_rect(img, 19 + lean, 9 + y, 13, 7, BRIGHT)
	_rect(img, 20 + lean, 16 + y, 12, 9, STEEL)
	for slit in [22, 26, 30]:
		_rect(img, slit + lean, 17 + y, 2, 7, OUTLINE)
	_rect(img, 16 + lean, 11 + y, 4, 8, STEEL)
	_rect(img, 15 + lean, 13 + y, 3, 3, GOLD)
	_rect(img, 22 + lean, 5 + y, 5, 3, BLUE_LIGHT)
	# Front arm, hilt and oversized sword clearly separated from the body.
	var sword_dx := 0
	var sword_dy := 0
	if anim.begins_with("attack"):
		var step := float(frame_index) / float(maxi(total - 1, 1))
		sword_dx = int(roundf(sin(step * PI) * 16.0))
		sword_dy = -int(roundf(sin(step * PI) * 13.0))
	elif anim == "dash":
		sword_dx = 9
		sword_dy = -3
	elif anim == "jump":
		sword_dx = 2
		sword_dy = -6
	_part(img, 10 + lean, 35 + y, 8, 12, LEATHER)
	_rect(img, 12 + lean + sword_dx, 45 + y + sword_dy, 4, 6, GOLD)
	# Blade diagonal is a stepped pixel staircase, never anti-aliased.
	for p in range(12):
		var bx := 10 - int(p / 3) + sword_dx + lean
		var by := 47 + p + sword_dy + y
		_rect(img, bx, by, 4, 2, BRIGHT)
		_rect(img, bx + 3, by, 1, 2, SHADOW)
	if anim.begins_with("attack") and frame_index >= 2 and frame_index <= 4:
		for p in range(6):
			_rect(img, 36 + p + lean, 20 + p * 3 + y, 3, 3, BRIGHT)
	if anim == "hurt" and frame_index == 1:
		_rect(img, 15 + lean, 21 + y, 20, 3, BRIGHT)
	if anim == "death":
		# Collapse gradually while retaining the silhouette.
		img.resize(ART_SIZE.x, ART_SIZE.y, Image.INTERPOLATE_NEAREST)
	return ImageTexture.create_from_image(img)

func _ready() -> void:
	actor = get_parent() as CharacterBody2D
	var frames := SpriteFrames.new()
	for existing in frames.get_animation_names():
		frames.remove_animation(existing)
	for key in ANIMS.keys():
		var name := str(key)
		frames.add_animation(name)
		frames.set_animation_speed(name, 12.0)
		frames.set_animation_loop(name, name == "idle" or name == "run" or name == "fall")
		for i in range(int(ANIMS[name])):
			frames.add_frame(name, _frame(name, i, int(ANIMS[name])))
	sprite_frames = frames
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	# Render 48x64 art at 2x; 128px tall, with feet aligned to actor origin.
	scale = Vector2(2.0, 2.0)
	position = Vector2(0, -64)
	play("idle")
	previous_hp = int(actor.get("health"))

func _physics_process(delta: float) -> void:
	if not is_instance_valid(actor):
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
