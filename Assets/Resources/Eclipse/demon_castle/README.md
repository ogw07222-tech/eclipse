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

`native-engine/src/demon_castle_entrance.hpp` loads precisely these five images atomically. Missing, invalid-sized, or corrupt files now produce an explicit `ECLIPSE ASSET ERROR` and fail the game load rather than silently showing the forest. It does not affect game physics or collision. The player collision floor is y=597 and the visible artwork surface begins y=585.

Desktop test-only preview: `./eclipse_engine --castle-preview`. This does **not** permanently change the forest arena. The tutorial scene transition still needs a scene-state hook when the tutorial gameplay is implemented.

Packaging support exists for desktop CMake and the normal Android Gradle asset Sync. Do not trigger GitHub Actions without the user's permission.

## Asset delivery status
Five castle PNG layers were committed to `main` in commit `8e1b9a2`. Their existence in Git does not prove Studio import or runtime decoding succeeded. Any missing/invalid required layer now causes a logged startup failure; owner-run runtime validation is still required.
