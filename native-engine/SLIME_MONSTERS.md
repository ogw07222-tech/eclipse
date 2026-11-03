# Slime monster sprites

The active SDL3 game now loads three transparent PNG sprite sheets from
`Assets/Resources/Eclipse/monsters/`. No procedural rectangle enemies remain.

| Tier | Sheet | HP | Damage | Speed | Bite duration | Cooldown |
| --- | --- | --- | --- | --- | --- | --- |
| Green, weakest | slime_green.png | 55 | 9 | 95 | 0.72 s | 1.30 s |
| Red | slime_red.png | 85 | 15 | 63 | 0.88 s | 1.80 s |
| Purple, strongest | slime_purple.png | 150 | 23 | 49 | 1.04 s | 2.20 s |

Each atlas is 512 x 512 pixels, a 4 x 4 grid of 128 x 128 cells. Frames 0–7
are movement, frames 8–15 are a nonlooping bite. Left-facing source art is
flipped for rightward attacks. Pivot is (64,116), with binary transparency,
shared 20-color palette per species and nearest-neighbor texture sampling.
Movement phase follows distance; idle does not skate. Attack direction locks
at anticipation; damage checks current position exactly once at frame 12,
halfway through the attack. Moving behind the jaw or out of range avoids damage.
The opening wave is green, red, green, purple.

Desktop CMake and Game Android asset packaging both include all three sheets.
The separately maintained Studio repository must package `monsters/**/*.png`
when it next syncs these game sources; this repository does not control its build.

## Visual review

`Art/Monsters/review/slimes_ingame.gif` shows 144 captured frames from the actual
SDL software renderer, including movement, biting and horizontal flips. It is a
deterministic animation review, not an Android device recording. To regenerate:

```sh
mkdir -p /tmp/slime-review
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software ./build/eclipse_engine --slime-review /tmp/slime-review
```

PNG artwork was generated from the three supplied monster references using the
built-in image generator, with these prompts: true limited-palette dark fantasy
pixel art; green low single-eyed acid slime, red tall crested fanged slime,
purple broad elite with trailing tendrils; four columns and four rows; first
eight poses a squash/stretch crawl loop, last eight anticipation/open jaw/
lunge/bite/recoil/recovery; left-facing, transparent background, no text or grid.
`Tools/art/pack_slimes.py SOURCE_DIRECTORY` normalizes generated sources named
`green.png`, `red.png`, `purple.png`, aligns feet, enforces a shared palette and
binary alpha, and packs the engine atlases. The original generated source PNGs
are separate authoring inputs, not runtime assets.

Validation: 40 core scenarios including tier order, delayed single contact,
escaping behind locked jaw, and frame bounds. Full SDL build, both CTest tests
and eight render smoke scenes passed. Android APK/hardware testing was not run.
