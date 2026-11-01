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
var burn_timer := 0.0
var burn_tick := 0.0
var vfx_clock := 0.0
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
  vfx_clock += delta
  attack_timer = maxf(0.0, attack_timer - delta)
  hit_flash = maxf(0.0, hit_flash - delta)
  knockback_velocity = move_toward(knockback_velocity, 0.0, 650.0 * delta)
  reaction_flash = maxf(0.0, reaction_flash - delta)
  if bleed_timer > 0.0:
    bleed_timer = maxf(0.0, bleed_timer - delta)
    bleed_tick += delta
    if bleed_tick >= 0.75:
      bleed_tick = 0.0
      apply_damage(3, true, "bleed")
  if burn_timer > 0.0:
    burn_timer = maxf(0.0, burn_timer - delta)
    burn_tick += delta
    if burn_tick >= 0.6:
      burn_tick -= 0.6
      apply_damage(2, true, "fire")
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
  if element == "fire":
    burn_timer = 3.0
    burn_tick = 0.0
  if element == "fire" and bleed_timer > 0.0:
    final_damage += 22
    bleed_timer = 0.0
    reaction_flash = 0.5
  if apply_bleed:
    bleed_timer = 4.5
    bleed_tick = 0.0
  if is_instance_valid(player):
    knockback_velocity = (1.0 if global_position.x >= player.global_position.x else -1.0) * (210.0 if reaction_flash > 0.0 else 135.0)
  apply_damage(final_damage, false, "fire" if element == "fire" else "normal")

func apply_damage(amount: int, is_dot: bool = false, kind: String = "normal") -> void:
  if is_dead:
    return
  damage_taken.emit(global_position, amount, kind)
  health = maxi(0, health - amount)
  if not is_dot:
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
  # Integer-grid pixel particles attached to the body, not smooth circles.
  # Bounded fixed counts: no per-frame allocations or new nodes.
  if bleed_timer > 0.0:
    for i in range(6):
      var phase := fposmod(vfx_clock * (17.0 + float(i % 3) * 4.0) + float(i * 11), h)
      var px := snappedf(float((i * 13) % 29 - 14), 3.0)
      var py := snappedf(-h + phase, 3.0)
      draw_rect(Rect2(Vector2(px, py), Vector2(3, 6 if i % 2 == 0 else 3)), Color("#e34359") if i % 2 == 0 else Color("#8f183a"))
  if burn_timer > 0.0:
    for i in range(9):
      var rise := fposmod(vfx_clock * (29.0 + float(i % 4) * 6.0) + float(i * 9), h + 18.0)
      var px := snappedf(float((i * 17) % 35 - 17) + sin(vfx_clock * 5.0 + float(i)) * 3.0, 3.0)
      var py := snappedf(4.0 - rise, 3.0)
      var shade := Color("#ffb74b") if i % 3 == 0 else (Color("#f16b32") if i % 3 == 1 else Color("#bb3839"))
      draw_rect(Rect2(Vector2(px, py), Vector2(6 if i % 3 == 0 else 3, 9 if i % 2 == 0 else 6)), shade)
  if reaction_flash > 0.0:
    for i in range(8):
      var x := snappedf(float((i % 4) * 17 - 26), 3.0)
      var y := snappedf(-h * 0.5 + float(int(i / 4) * 18 - 9), 3.0)
      draw_rect(Rect2(Vector2(x,y),Vector2(9,6)),Color("#ffbc57"))
