# Demon Castle Entrance — tutorial background prototype

The opening tutorial's dark-gothic castle entrance is intentionally separate from the existing forest and from the later throne-room boss arena. The art direction uses charcoal masonry, muted red banners, small warm braziers and minimal violet emission.

## Layer contract (SDL3, 1280 × 720 logical viewport)
Put the following PNGs under `Assets/Resources/Eclipse/demon_castle/entrance/`:

| PNG | Pixel size | Parallax | Usage |
| --- | --- | --- | --- |
| sky_far.png | 1280×720 | fixed | opaque sky |
| far_castle.png | 1280×720 | 0.18 | distant silhouettes |
| mid_ruins.png | 1600×720 | 0.42 | collapsed exterior arches |
| near_gate.png | 1600×720 | 1.0 | unique castle gate |
| floor_repeat_512.png | 512×135 | 1.0 | exactly one horizontally repeated texture |

All layers use nearest-neighbor, no interpolation. The floor illustration begins at y=585, while existing engine collision is at y=597; do not infer collision from PNG pixels.

`native-engine/src/demon_castle_entrance.hpp` loads the five textures as an **optional atomic pack**. Missing or incorrect dimensions leave the original forest intact; `Presentation::setDemonCastleEntrance(true)` enables this art when loaded, and false restores forest rendering. The controller currently does not select scenes automatically, since the tutorial scene/state transition is not implemented. It must be called from the tutorial's entry transition only, not at the global startup of the forest combat prototype.

Build pipelines package the folder whenever PNGs exist. **No GitHub Actions workflow or automatic testing is requested or run.**

## Art status
The initial prototype exports and reference previews were generated separately from the repository. A game-ready transparent-seam art pass is still needed before enabling this scene in production. 
