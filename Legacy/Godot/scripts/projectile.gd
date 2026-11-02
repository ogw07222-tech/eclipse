extends Node2D
class_name EclipseProjectile

var direction := 1
var damage := 11
var empowered := false
var lifetime := 1.6
var speed := 540.0

func setup(origin: Vector2, facing: int, power: int, is_skill: bool) -> void:
	global_position = origin
	direction = facing
	damage = power
	empowered = is_skill
	speed = 440.0 if empowered else 540.0

func _physics_process(delta: float) -> void:
	position.x += direction * speed * delta
	lifetime -= delta
	if lifetime <= 0.0:
		queue_free()
		return
	for enemy in get_tree().get_nodes_in_group("enemies"):
		if not is_instance_valid(enemy) or enemy.is_dead:
			continue
		if global_position.distance_to(enemy.global_position + Vector2(0, -30)) < (34.0 if empowered else 25.0):
			enemy.receive_hit(damage, "fire")
			queue_free()
			return
	queue_redraw()

func _draw() -> void:
	draw_circle(Vector2.ZERO, 14 if empowered else 8, Color("ff8647"))
	draw_circle(Vector2.ZERO, 7 if empowered else 4, Color("ffe7a1"))
