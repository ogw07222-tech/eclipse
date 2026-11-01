extends RefCounted
class_name EclipseEquipment

# Explicit slots and IDs keep save files independent of display names.
const SLOTS := ["weapon", "armor", "accessory"]
const ITEMS := {
	"rusty_sword": {"slot":"weapon","name":"낡은 장검","attack":0,"defense":0,"hp":0,"speed":1.0,"range":1.0},
	"blood_blade": {"slot":"weapon","name":"핏빛 검","attack":6,"defense":0,"hp":0,"speed":0.88,"range":1.16},
	"quick_blade": {"slot":"weapon","name":"질풍검","attack":-2,"defense":0,"hp":0,"speed":1.3,"range":1.08},
	"cloth_armor": {"slot":"armor","name":"천 갑옷","attack":0,"defense":0,"hp":0,"speed":1.0,"range":1.0},
	"iron_armor": {"slot":"armor","name":"철 갑옷","attack":0,"defense":4,"hp":30,"speed":0.9,"range":1.0},
	"scout_armor": {"slot":"armor","name":"정찰 갑옷","attack":0,"defense":1,"hp":10,"speed":1.1,"range":1.0},
	"empty_charm": {"slot":"accessory","name":"장신구 없음","attack":0,"defense":0,"hp":0,"speed":1.0,"range":1.0},
	"flame_charm": {"slot":"accessory","name":"불꽃 부적","attack":3,"defense":0,"hp":0,"speed":1.0,"range":1.0},
	"guardian_charm": {"slot":"accessory","name":"수호 부적","attack":0,"defense":2,"hp":15,"speed":1.0,"range":1.0}
}
const OPTIONS := {
	"weapon": ["rusty_sword","blood_blade","quick_blade"],
	"armor": ["cloth_armor","iron_armor","scout_armor"],
	"accessory": ["empty_charm","flame_charm","guardian_charm"]
}
var equipped := {"weapon":"rusty_sword","armor":"cloth_armor","accessory":"empty_charm"}

func equip(item_id: String) -> bool:
	if not ITEMS.has(item_id):
		return false
	var slot: String = ITEMS[item_id]["slot"]
	equipped[slot] = item_id
	return true

func cycle(slot: String) -> void:
	if not OPTIONS.has(slot):
		return
	var choices: Array = OPTIONS[slot]
	var index: int = choices.find(equipped[slot])
	equip(choices[(index + 1) % choices.size()])

func stat(key: String) -> float:
	var result := 1.0 if key == "speed" or key == "range" else 0.0
	for slot in SLOTS:
		var item: Dictionary = ITEMS[equipped[slot]]
		if key == "speed" or key == "range":
			result *= float(item.get(key, 1.0))
		else:
			result += float(item.get(key, 0))
	return result

func name_for(slot: String) -> String:
	return str(ITEMS[equipped[slot]]["name"])

func export_ids() -> Dictionary:
	return equipped.duplicate()

func restore_ids(ids: Dictionary) -> void:
	for slot in SLOTS:
		if ids.has(slot):
			var candidate: String = str(ids[slot])
			if ITEMS.has(candidate) and ITEMS[candidate]["slot"] == slot:
				equipped[slot] = candidate
