extends AnimatedSprite2D
# Optional real sprite atlas: assets/knight_sheet.png from the ECLIPSE animation pack.
# If the atlas is missing, the original procedural player remains visible.
const SHEET := "res://assets/knight_sheet.png"
const COUNTS := {"idle":6,"run":8,"jump":4,"fall":3,"attack1":6,"attack2":6,"attack3":8,"dash":5,"hurt":3,"death":6}
const FRAME := Vector2i(96,128)
var actor: CharacterBody2D
var previous_hp := -1
var previous_attack := 0.0
var attack_time := 0.0
var hurt_time := 0.0
var combo := 1
func _ready() -> void:
  actor = get_parent() as CharacterBody2D
  if not ResourceLoader.exists(SHEET):
    visible = false
    return
  var texture: Texture2D = load(SHEET)
  var frames := SpriteFrames.new()
  for key in frames.get_animation_names():
    frames.remove_animation(key)
  var idx := 0
  for animation_key in COUNTS.keys():
    var anim: String = str(animation_key)
    frames.add_animation(anim)
    frames.set_animation_speed(anim, 11.0)
    frames.set_animation_loop(anim, anim == "idle" or anim == "run" or anim == "fall")
    for n in range(int(COUNTS[anim])):
      var region := AtlasTexture.new()
      region.atlas = texture
      region.region = Rect2((idx % 8) * FRAME.x, (idx / 8) * FRAME.y, FRAME.x, FRAME.y)
      frames.add_frame(anim, region)
      idx += 1
  sprite_frames = frames
  position = Vector2(0,-58)
  play("idle")
  previous_hp = int(actor.get("health"))
func _physics_process(delta: float) -> void:
  if not is_instance_valid(actor) or sprite_frames == null:
    return
  var hp := int(actor.get("health"))
  if hp < previous_hp:
    hurt_time = 0.23
  previous_hp = hp
  hurt_time = maxf(0.0,hurt_time - delta)
  var atk := float(actor.get("attack_flash"))
  if atk > 0.0 and previous_attack <= 0.0:
    attack_time = 0.36
    combo = clampi(int(actor.get("combo_index")),1,3)
  previous_attack = atk
  attack_time = maxf(0.0,attack_time - delta)
  visible = int(actor.get("form")) == 0
  flip_h = int(actor.get("facing")) < 0
  var next_animation: String = "idle"
  if hp <= 0:
    next_animation = "death"
  elif hurt_time > 0.0:
    next_animation = "hurt"
  elif attack_time > 0.0:
    next_animation = "attack%d" % combo
  elif float(actor.get("dash_left")) > 0:
    next_animation = "dash"
  elif not actor.is_on_floor():
    next_animation = "jump" if actor.velocity.y < 0 else "fall"
  elif absf(actor.velocity.x) > 20:
    next_animation = "run"
  if animation != next_animation:
    play(next_animation)
