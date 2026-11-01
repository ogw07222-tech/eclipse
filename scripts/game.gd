extends Node2D

const PlayerScript = preload("res://scripts/player.gd")
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
var overlay: ColorRect
var overlay_title: Label
var overlay_hint: Label
var foes_left := 0
var ended := false
var touch_buttons: Array[Button] = []

func _ready() -> void:
  _register_inputs()
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
  var slots := [Vector2(575, FLOOR_Y), Vector2(800, FLOOR_Y), Vector2(1020, FLOOR_Y), Vector2(1285, FLOOR_Y)]
  foes_left = slots.size()
  for i in range(slots.size()):
    var enemy := EnemyScript.new() as EclipseEnemy
    enemy.configure(0 if i % 2 == 0 else 1)
    enemy.position = slots[i]
    enemy.player = player
    add_child(enemy)
    enemy.killed.connect(_on_enemy_killed)
  overlay.visible = false
  _refresh_hud()

func _spawn_projectile(spawn_pos: Vector2, direction: int, power: int, is_skill: bool) -> void:
  var projectile := ProjectileScript.new() as EclipseProjectile
  add_child(projectile)
  projectile.add_to_group("bullets")
  projectile.setup(spawn_pos, direction, power, is_skill)

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
  overlay_title.text = "STAGE CLEAR!" if won else "GAME OVER"
  overlay_title.add_theme_color_override("font_color", Color("8be4b5") if won else Color("ff8d99"))
  overlay_hint.text = "R 키 또는 아래 버튼으로 다시 시작"

func _unhandled_input(event: InputEvent) -> void:
  if event.is_action_pressed("restart"):
    _start_round()

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

func _label(text_value: String, xy: Vector2, size: int, tint: Color) -> Label:
  var label := Label.new()
  label.text = text_value
  label.position = xy
  label.add_theme_font_size_override("font_size", size)
  label.add_theme_color_override("font_color", tint)
  return label

func _build_hud() -> void:
  hud = CanvasLayer.new()
  add_child(hud)
  var header := ColorRect.new()
  header.color = Color(0.035, 0.055, 0.1, 0.9)
  header.position = Vector2(0, 0)
  header.size = Vector2(1280, 115)
  header.mouse_filter = Control.MOUSE_FILTER_IGNORE
  hud.add_child(header)
  hud.add_child(_label("E C L I P S E   |   COMBAT PROTOTYPE 0.1", Vector2(24, 10), 18, Color("cfdaf9")))
  health_label = _label("", Vector2(26, 43), 22, Color("ff9ca9"))
  form_label = _label("", Vector2(250, 43), 20, Color("9ed7ff"))
  enemies_label = _label("", Vector2(755, 43), 20, Color("f5c58a"))
  cooldown_label = _label("", Vector2(28, 85), 15, Color("a7b6d2"))
  status_label = _label("", Vector2(555, 85), 15, Color("ffe19b"))
  for item in [health_label, form_label, enemies_label, cooldown_label, status_label]:
    hud.add_child(item)
  var controls := _label("A/D 이동   SPACE 점프   SHIFT 대시   J/Z 공격   K/X 스킬   L/C 교체   R 재시작", Vector2(26, 672), 17, Color("c7d7ee"))
  hud.add_child(controls)
  overlay = ColorRect.new()
  overlay.color = Color(0.025, 0.035, 0.065, 0.88)
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
  overlay.add_child(retry)
  # Mobile and tablet controls; toggle by adding ?touch=1 to launch parameters via code or show by default.
  var button_specs := [
    ["◀", "left", Vector2(25, 568), Vector2(70, 68)],
    ["▶", "right", Vector2(104, 568), Vector2(70, 68)],
    ["점프", "jump", Vector2(190, 568), Vector2(88, 68)],
    ["대시", "dash", Vector2(810, 568), Vector2(88, 68)],
    ["공격", "attack", Vector2(910, 568), Vector2(88, 68)],
    ["스킬", "skill", Vector2(1010, 568), Vector2(88, 68)],
    ["교체", "swap", Vector2(1110, 568), Vector2(88, 68)]
  ]
  for spec in button_specs:
    var button := Button.new()
    button.text = spec[0]
    button.position = spec[2]
    button.size = spec[3]
    button.modulate.a = 0.66
    button.button_down.connect(_on_touch_action.bind(spec[1], true))
    button.button_up.connect(_on_touch_action.bind(spec[1], false))
    hud.add_child(button)
    touch_buttons.append(button)

func _on_touch_action(action: String, state: bool) -> void:
  if is_instance_valid(player):
    player.set_virtual(action, state)

func _draw() -> void:
  draw_rect(Rect2(0, 0, WORLD_WIDTH, 720), Color("121b32"))
  for x in range(0, WORLD_WIDTH, 130):
    draw_rect(Rect2(x + 30, 210 + (x % 170), 46, 400), Color("1b2942"))
    draw_rect(Rect2(x + 15, 200 + (x % 170), 76, 15), Color("243550"))
  for x in range(0, WORLD_WIDTH, 175):
    draw_circle(Vector2(x + 55, 145 + x % 115), 3, Color("b5a6de"))
  draw_rect(Rect2(0, FLOOR_Y, WORLD_WIDTH, 135), Color("27364c"))
  draw_rect(Rect2(0, FLOOR_Y, WORLD_WIDTH, 8), Color("65718d"))
  for x in range(0, WORLD_WIDTH, 80):
    draw_line(Vector2(x, FLOOR_Y + 30), Vector2(x + 55, FLOOR_Y + 30), Color("334661"), 3)
