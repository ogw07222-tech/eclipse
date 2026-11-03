# Forest Entrance — authored moonlit pixel-art biome

Default world art: six standalone PNGs under `Assets/Resources/Eclipse/forest_entrance/`. Pixel-art assets are authored image layers, **not code-generated squares/trees**. Only camera parallax is calculated by SDL3/C++.

| Draw order | File | Size | Camera factor |
|---|---|---|---|
| sky, moon & gradient | sky/sky.png | 1280x720 | 0 |
| clouds | clouds/clouds.png | 1600x720 | 0.06 |
| mountain horizon | far/mountains.png | 1600x720 | 0.14 |
| trees (clear center) | mid/trees.png | 1600x720 | 0.40 |
| soil/grass | near/ground.png | 1600x720 | 1 |
| rocks & shrubs | near/props.png | 1600x720 | 1 |

Preserved gameplay geometry: viewport 1280×720; world width 1600; collision floor y585. Fixed future NPC placement corridors x260–425, 710–900, 1180–1360. All six assets are decorative and cannot move collision or NPC coordinates; density/seed does not shift them. The visual rocks/shrubs do not represent physical obstacles.

## Import the asset archive once

Download **eclipse_forest_entrance_v4.zip** from the related ChatGPT conversation onto the Android device. The GitHub connector currently updates code separately; this PNG archive must be imported to the repo to make CI include it.

```sh
cd ~/eclipse
git pull --ff-only origin main
unzip -o ~/storage/downloads/eclipse_forest_entrance_v4.zip -d ~/eclipse
git add Assets/Resources/Eclipse/forest_entrance
git commit -m "Add authored forest entrance pixel-art layers"
git push origin main
```

If `git commit` says nothing to commit, the assets may already have been pushed. Check `git status` and browse `Assets/Resources/Eclipse/forest_entrance` in the repository.

Preview:

```sh
cd ~/eclipse
python -m http.server 8765 --bind 127.0.0.1
# In a new Termux session:
termux-open-url "http://127.0.0.1:8765/native-engine/preview/forest.html?v=entrance-v4"
```

The status line must display **숲 입구 PNG 6/6**. If PNG assets are missing, it will fall back atomically to forest_v3 / forest_v2 / legacy.

## APK

Use GitHub Actions → Native Android APK → Run workflow on `main`, with `signed_release=true` for updates using the existing persistent signing key. **Do not replace or regenerate the signing key.** The build verifies that all checked-in PNGs are contained in the resulting APK. An Android signed APK has not been built/tested just by importing the PNG archive.
