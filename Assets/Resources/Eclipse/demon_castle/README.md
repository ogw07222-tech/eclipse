# ECLIPSE — Demon Castle Entrance (tutorial art)
 
## Approved visual reference
Dark charcoal gothic stone, tall barred gate at right, hooded statues, muted worn red banners, orange braziers and an eclipsed moon. The approved user-supplied image is the source of truth; **do not use the older generated placeholder art**.

## Five PNG layer contract
```
Assets/Resources/Eclipse/demon_castle/entrance/
  sky_far.png             # 1280x720, 0.00 camera parallax
  far_castle.png           # 1280x720, 0.18
  mid_ruins.png            # 1600x720, 0.42
  near_gate.png            # 1600x720, 1.00
  floor_repeat_512.png     # 512x135, repeated horizontally starting y=585
```

The **floor is a single PNG** drawn as many times as needed, not five duplicate files. Do not include any old prototype `layered_preview.png`, `entrance_preview_full.png` or scene-baked duplicate floor images in game assets. Other biomes' PNG assets are unrelated and must not be deleted.

`native-engine/src/demon_castle_entrance.hpp` loads precisely these five images atomically (missing file = no scene override). It does not affect game physics or collision. The player collision floor is y=597 and the visible artwork surface begins y=585.

Desktop test-only preview: `./eclipse_engine --castle-preview`. This does **not** permanently change the forest arena. The tutorial scene transition still needs a scene-state hook when the tutorial gameplay is implemented.

Packaging support exists for desktop CMake and the normal Android Gradle asset Sync. Do not trigger GitHub Actions without the user's permission.

## Asset delivery status
The approved-reference layers have been generated in the ChatGPT working environment and delivered as a separate downloadable ZIP, but **the binary PNGs are not committed to this GitHub repository**. The code is ready to load them after they are placed at the exact paths above. Do not claim that the in-game scene works until assets are actually committed and a local rendering test is performed.
