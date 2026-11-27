# ECLIPSE title opening

The 5.65-second opening cuts from a gauntlet on a greatsword to a knight on an obsidian altar under an eclipse. The blade catches the light in the close-up. In the wide shot, the knight straightens from the waist as the camera pulls back and the cape lifts in the wind. The dark face of the eclipse fills the frame before the existing title emerges. Tap, left click, Enter, or Space to open the menu during the sequence.

## Source artwork

The three source PNGs were made with the built-in ImageGen tool for this cinematic. The source direction for each image was as follows.

- `eclipse_gauntlet.png` (1672 × 941). Extreme side-on close-up of a segmented black-iron gauntlet gripping a vertical greatsword on an obsidian altar. Use a nearly black plum silhouette, icy violet blade reflection, and restrained pixel-painted highlights.
- `eclipse_altar.png` (1672 × 941). Wide view of a cracked circular obsidian altar over an abyss, framed by gothic arches with a large broken violet eclipse overhead. Keep the altar clear for the knight and light its edge with the corona.
- `eclipse_character_cels.png` (1536 × 1024 RGBA). One transparent cel of a closed-helmet knight seen from behind with a downturned blade, and a separate transparent cel of an indigo cape with a right-side shoulder attachment. Keep a clear gap between the pieces and preserve transparency through the cloth tears and silhouette.

## Runtime assets

Run `python Art/title/pack_opening_knight.py` with Pillow from the repository root to regenerate the three runtime PNGs under `Assets/Resources/Eclipse/title/`.

| Runtime PNG | Dimensions | SHA-256 |
| --- | --- | --- |
| `opening_gauntlet.png` | 1280 × 720 RGB | `fd0b6a0d5b848539216b462a9af9a8e8066ddff17ce0bb5f5c45d45c422d915d` |
| `opening_altar.png` | 1280 × 720 RGB | `c47eb170ff0dc971cc9af63675f93a3d7ae2507defe6940b9bfff3c6b17dffba` |
| `opening_knight.png` | 1536 × 1408 RGBA | `05c96020fae3ec80478f1c3a15254936c89da15b883616fc95e533c6a923a8c0` |

| Cel | Atlas x, y, width, height | Source x, y |
| --- | --- | --- |
| Upper body and held sword | 0, 0, 586, 846 | 28, 8 |
| Cape | 620, 0, 870, 872 | 638, 124 |
| Lower body | 0, 884, 398, 504 | 80, 450 |

The upper body and legs overlap under the belt. The leg cel stays planted while the upper body rotates around source point (280, 508). The cape follows source point (268, 145) at the shoulder and ripples toward its free edge. The original transparent silhouettes are preserved except for negligible export noise below alpha 5.

Desktop CMake and Android Gradle package all three PNGs. The Android APK verifier checks the packaged copies by default. Missing, malformed, or incorrectly sized assets surface through the startup title error dialog with the resolved file path.
