package dev.eclipse.studio

import org.json.JSONArray
import org.json.JSONObject

/**
 * One deterministic edit path for touch UI and future authenticated AI/MCP adapters.
 * No network or Android access: commands only transform an in-memory scene.
 */
internal object SceneCommands {
    private const val MAX_OBJECTS = 500
    private const val MAX_SIZE = 8192
    private const val MAX_COORD = 100000

    internal data class Result(
        val objects: List<VisualObject>,
        val changed: Boolean,
        val message: String,
        val selectedId: Int? = null,
        val payload: JSONObject? = null
    ) {
        fun response(): JSONObject = JSONObject().apply {
            put("ok", true)
            put("changed", changed)
            put("message", message)
            put("selectedId", selectedId ?: JSONObject.NULL)
            if (payload != null) put("data", payload)
        }
    }

    internal fun execute(before: List<VisualObject>, request: JSONObject): Result {
        val action = request.optString("action")
        fun id(): Int = request.requiredInt("id", 1, Int.MAX_VALUE)
        fun existing(): VisualObject = before.firstOrNull { it.id == id() }
            ?: throw IllegalArgumentException("Object not found")
        fun updated(obj: VisualObject, next: VisualObject): Result =
            Result(before.map { if (it.id == obj.id) next else it },
                obj != next, "Object updated", obj.id)

        return when (action) {
            "list_objects" -> Result(before, false, "Listed objects", payload = JSONObject()
                .put("objects", JSONArray().apply { before.forEach { put(toJson(it)) } }))
            "add_object" -> {
                require(before.size < MAX_OBJECTS) { "Scene object limit reached" }
                val uri = request.requiredString("uri", 4096)
                require(uri.startsWith("content://") || uri.startsWith("file://")) {
                    "Only local content/file asset URIs are supported"
                }
                val newId = (before.maxOfOrNull { it.id } ?: 0) + 1
                val obj = VisualObject(newId, request.requiredString("name", 120),
                    uri, request.requiredInt("x", -MAX_COORD, MAX_COORD),
                    request.requiredInt("y", -MAX_COORD, MAX_COORD),
                    request.requiredInt("width", 1, MAX_SIZE),
                    request.requiredInt("height", 1, MAX_SIZE))
                Result(before + obj, true, "Object created", newId)
            }
            "move_object" -> {
                val obj = existing()
                updated(obj, obj.copy(
                    x = request.requiredInt("x", -MAX_COORD, MAX_COORD),
                    y = request.requiredInt("y", -MAX_COORD, MAX_COORD)))
            }
            "set_property" -> {
                val obj = existing()
                val key = request.requiredString("property", 40)
                val next = when (key) {
                    "name" -> obj.copy(name = request.requiredString("value", 120))
                    "x" -> obj.copy(x = request.requiredInt("value", -MAX_COORD, MAX_COORD))
                    "y" -> obj.copy(y = request.requiredInt("value", -MAX_COORD, MAX_COORD))
                    "width" -> obj.copy(width = request.requiredInt("value", 1, MAX_SIZE))
                    "height" -> obj.copy(height = request.requiredInt("value", 1, MAX_SIZE))
                    else -> throw IllegalArgumentException("Unsupported property: $key")
                }
                updated(obj, next)
            }
            "delete_object" -> {
                val obj = existing()
                Result(before.filterNot { it.id == obj.id }, true, "Object deleted")
            }
            "clear_scene" -> Result(emptyList(), before.isNotEmpty(), "Scene cleared")
            else -> throw IllegalArgumentException("Unsupported action: $action")
        }
    }

    private fun JSONObject.requiredString(key: String, maxLength: Int): String {
        require(has(key) && !isNull(key)) { "Missing field: $key" }
        val value = opt(key)
        require(value is String && value.isNotBlank() && value.length <= maxLength) {
            "Invalid string: $key"
        }
        return value
    }

    private fun JSONObject.requiredInt(key: String, min: Int, max: Int): Int {
        require(has(key) && !isNull(key)) { "Missing field: $key" }
        val raw = opt(key)
        require(raw is Number && raw.toDouble().isFinite() &&
            raw.toDouble() % 1.0 == 0.0 && raw.toDouble() in min.toDouble()..max.toDouble()) {
            "Invalid integer: $key"
        }
        return raw.toInt()
    }

    private fun toJson(o: VisualObject): JSONObject = JSONObject()
        .put("id", o.id).put("name", o.name).put("uri", o.uri)
        .put("x", o.x).put("y", o.y)
        .put("width", o.width).put("height", o.height)
}
