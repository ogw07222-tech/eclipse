extends CharacterBody2D
class_name EclipseEnemy

signal killed(enemy: EclipseEnemy)
signal damage_taken(position_value: Vector2, amount: int, kind: String)

var max_health := 55
var health := 55
var move_speed := 88.0
var attack_damage := 9
var attack_timer := 0.0
var bleed_timer := 0.0
var bleed_tick := 0.0
var reaction_flash := 0.0
var hit_flash := 0.0
var knockback_velocity := 0.0
var is_dead := false
var type_id := 0
var player: EclipsePlayer

func configure(kind: int) -> void:
  type_id = kind
  max_health = 55 if kind == 0 else 85
  health = max_health
  move_speed = 95.0 if kind == 0 else 63.0
  attack_damage = 9 if kind == 0 else 15

func _ready() -> void:
  add_to_group("enemies")
  collision_layer = 4
  collision_mask = 1
  var shape := CollisionShape2D.new()
  var rect := RectangleShape2D.new()
  rect.size = Vector2(36, 50) if type_id == 0 else Vector2(46, 64)
  shape.shape = rect
  shape.position = Vector2(0, -25 if type_id == 0 else -32)
  add_child(shape)
  queue_redraw()

func _physics_process(delta: float) -> void:
  if is_dead:
    return
  attack_timer = maxf(0.0, attack_timer - delta)
  hit_flash = maxf(0.0, hit_flash - delta)
  knockback_velocity = move_toward(knockback_velocity, 0.0, 650.0 * delta)
  reaction_flash = maxf(0.0, reaction_flash - delta)
  if bleed_timer > 0.0:
    bleed_timer = maxf(0.0, bleed_timer - delta)
    bleed_tick += delta
    if bleed_tick >= 0.75:
      bleed_tick = 0.0
      apply_damage(3)
  if is_dead:
    return
  if is_instance_valid(player) and player.health > 0:
    var dx := player.global_position.x - global_position.x
    if absf(dx) > 46.0:
      velocity.x = signf(dx) * move_speed
    else:
      velocity.x = 0.0
      if attack_timer <= 0.0 and absf(player.global_position.y - global_position.y) < 70.0:
        player.take_damage(attack_damage)
        attack_timer = 1.3 if type_id == 0 else 1.8
  velocity.x += knockback_velocity
  velocity.y += 1150.0 * delta
  move_and_slide()
  queue_redraw()

func receive_hit(amount: int, element: String, apply_bleed: bool = false) -> void:
  if is_dead:
    return
  var final_damage := amount
  if element == "fire" and bleed_timer > 0.0:
    final_damage += 22
    bleed_timer = 0.0
    reaction_flash = 0.5
  if apply_bleed:
    bleed_timer = 4.5
    bleed_tick = 0.0
  if is_instance_valid(player):
    knockback_velocity = (1.0 if global_position.x >= player.global_position.x else -1.0) * (210.0 if reaction_flash > 0.0 else 135.0)
  apply_damage(final_damage, "fire" if element == "fire" else "normal")

func apply_damage(amount: int, kind: String = "bleed") -> void:
  if is_dead:
    return
  damage_taken.emit(global_position, amount, kind)
  health = maxi(0, health - amount)
  hit_flash = 0.18
  if health <= 0:
    is_dead = true
    killed.emit(self)
    queue_free()
  else:
    queue_redraw()

func _draw() -> void:
  var body_color := Color("9d7abf") if type_id == 0 else Color("ba6c83")
  if hit_flash > 0.0:
    body_color = Color.WHITE
  var half_width := 16.0 if type_id == 0 else 21.0
  var h := 48.0 if type_id == 0 else 62.0
  draw_rect(Rect2(-half_width, -h, half_width * 2.0, h), body_color)
  draw_circle(Vector2(-7, -h + 16), 4, Color("ffed9c"))
  draw_circle(Vector2(7, -h + 16), 4, Color("ffed9c"))
  draw_rect(Rect2(-21, -h - 15, 42, 5), Color("262435"))
  draw_rect(Rect2(-21, -h - 15, 42 * float(health) / float(max_health), 5), Color("f47777"))
  if bleed_timer > 0.0:
    draw_circle(Vector2(0, -h - 26), 5, Color("ea4263"))
  if reaction_flash > 0.0:
    draw_arc(Vector2(0, -h * 0.5), 40.0, 0.0, TAU, 20, Color("ffb04b"), 5.0)
