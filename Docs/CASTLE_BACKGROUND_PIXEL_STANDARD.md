# Demon Castle Background Pixel Standard

Status: **mandatory visual and export standard for A1–A5 backgrounds**. The current five runtime backgrounds use the standard canvas and are authored toward the 371 × 127 pixel-density benchmark.

## Canvas and pixel detail

- Runtime background canvas: **2172 × 724 px** (3:1 aspect ratio), matching the native castle background loader.
- Detail and pixel-density benchmark: **371 × 127 px**. This is a **visual quality reference**, not the required exported file size.
- Keep final runtime PNGs at 2172 × 724 px. Preserve the 3:1 composition; do not stretch artwork to force a 371 × 127 source image onto the runtime canvas.
- Use crisp, deliberate pixel clusters, a limited coherent palette, hard edges and nearest-neighbor sampling. Avoid blur, anti-aliased contours, smooth gradients and photographic texture.
- Judge detail at the intended game display size. Reduce excessive micro-detail until the scene reads with roughly the same pixel density and simplicity as a 371 × 127 reference image. Do not reduce the runtime file dimensions to achieve this look.
- Maintain useful silhouette contrast and enough grounded terrain detail to make traversal readable. Vary the ground, ledges, rubble, slopes and gaps where the area's layout calls for them; avoid an empty flat strip or repetitive decoration.

## Camera and composition

- Draw every background from a **2D side-on view** for the horizontal side-scrolling roguelike.
- Show only the portion of the environment around the player. A castle approach may show walls, gate fragments or distant towers, but must not reveal the entire Demon Castle.
- Keep the playable route and terrain legible. Do not paint characters, UI, collision debug marks or story text into the background.
- Compose each area as a distinct, hand-authored fixed scene, not a random or endlessly tiled map.

## Story and area direction

Use the approved visual direction in [Tutorial / Prologue Flow](TUTORIAL_PROLOGUE_FLOW.md): a dark, crisp pixel-art world, dialogue-free storytelling, readable silhouettes, and restrained atmosphere. Backgrounds support staging and anticipation through environmental shapes and lighting; they do not carry plot exposition as text.

| Area | Canonical name | Background intent |
| --- | --- | --- |
| A1 | Ashen Approach | The outer approach near the Demon Castle; communicate ash, an uneven route and the looming presence of the entrance without showing the whole castle. |
| A2 | Broken Gate | A damaged threshold with broken masonry and traversable ground that signals entry into the castle. |
| A3 | Void Corridor | An interior passage shaped by oppressive architecture and void influence; preserve readable ground and authored gaps. |
| A4 | Throne Stairway | A rising interior route that directs attention toward the castle core through stairs and vertical architecture. |
| A5 | Night Throne | A focused throne arena with strong framing and a clear combat floor; keep the Demon King staging readable. |

These descriptions guide art only. They do not assert that every area is activated or playable in runtime.

## File and review requirements

- Runtime path pattern: `Assets/Resources/Eclipse/demon_castle/areas/aN_<area_name>.png`; keep the existing canonical filenames.
- Verify the exported PNG is 2172 × 724 px and inspect it at native resolution after nearest-neighbor export.
- Preserve an editable source in `Art/` when the source is available; keep only reviewed game-ready backgrounds in `Assets/Resources/Eclipse/`.
- Record actual paths, dimensions and content hashes in the relevant asset notes when available.
- Art approval, collision alignment, runtime activation and owner-run gameplay validation are separate acceptance steps. Do not claim them based only on the file's presence.
- Do not run tests, builds or GitHub Actions as part of documenting or authoring these backgrounds unless the owner separately requests them; GitHub Actions also needs explicit approval for that specific run.

## Current asset status

- A1 `Assets/Resources/Eclipse/demon_castle/areas/a1_ashen_approach.png`: 2172 × 724 runtime PNG, authored to the 371 × 127 visual detail benchmark.
- A2 `Assets/Resources/Eclipse/demon_castle/areas/a2_broken_gate.png`: 2172 × 724 runtime PNG, authored to the 371 × 127 visual detail benchmark.
- A3 `Assets/Resources/Eclipse/demon_castle/areas/a3_void_corridor.png`: 2172 × 724 runtime PNG, authored to the 371 × 127 visual detail benchmark.
- A4 `Assets/Resources/Eclipse/demon_castle/areas/a4_throne_stairway.png`: 2172 × 724 runtime PNG, authored to the 371 × 127 visual detail benchmark.
- A5 `Assets/Resources/Eclipse/demon_castle/areas/a5_night_throne.png`: 2172 × 724 runtime PNG, authored to the 371 × 127 visual detail benchmark.

All five areas currently have authored route terrain and portal anchors. See `Assets/Resources/Eclipse/demon_castle/README.md` for the portal route. Art approval and owner-run gameplay validation remain separate acceptance steps.
