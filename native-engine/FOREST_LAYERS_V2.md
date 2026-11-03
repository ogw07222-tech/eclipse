# Forest v2 — fixed-NPC PNG layer pilot

This experiment replaces scattered randomized trees/props with a composed background consisting of four **PNG layers**. Gameplay coordinates do **not** come from the art.

## Art package

Download `eclipse_forest_layers_v1.zip` from the corresponding chat and extract it **from the repository root**:

```sh
cd ~/eclipse
git pull --ff-only origin main
unzip -o ~/storage/downloads/eclipse_forest_layers_v1.zip -d .
git add Assets/Resources/Eclipse/forest_v2
git commit -m "Import fixed NPC-safe layered forest PNGs"
git push origin main
```

Expected paths:

```
Assets/Resources/Eclipse/forest_v2/
    sky/sky_01.png           1280x720 (screen-locked, parallax 0)
    far/forest_far_01.png    1600x720 (parallax 0.12)
    mid/forest_mid_01.png    1600x720 (parallax 0.40)
    near/ground_fixed_01.png 1600x720 (world-locked, parallax 1)
    scene.json
```

These are starter art prototypes, not seamless infinitely-repeatable tiles. Current stage bounds are fixed to 1600 world pixels and the image doesn't tile. A master reference image was prepared in the chat as inspiration. The separated PNGs are a first iterative art pass, not exact multilayer extraction of that reference. Future sprite-quality work should replace these layers without changing game coordinates.

## Critical NPC guarantees

- Physics floor **y=585**, world width **1600** and existing combat/enemy/player positions stay untouched.
- NPC reserved intervals, **x=260–425**, **710–900**, and **1180–1360**, are defined in `src/forest_layers.hpp`.
- **No randomized world obstacles or props** are rendered on top of composed layers. Decorative mid-ground artwork can appear behind an NPC but does not block it.
- Near/ground art is a single fixed PNG; no scene-scale, flip, horizontal jitter, or spawn coordinate randomization.
- Only far-layer brightness varies by seed, at about **94–100%** intensity. Mid, near and sky remain visually stable.
- All four PNG files must be loadable and at exact expected dimensions to enable composed mode. If any is absent or corrupt, native engine and browser preview render the previous forest instead.

## Preview

```sh
cd ~/eclipse
python -m http.server 8765 --bind 127.0.0.1
```

Open `http://127.0.0.1:8765/native-engine/preview/forest.html?v=layers-v1`.

Enable **NPC 고정 구역 보기** to visualize reserved world intervals. In composed mode, changing density, size and variation has no effect, by design. Camera scroll, seed-based distant tint and zone visualization still work. Browser preview is **not** a native engine render/Android performance test.

## Verification

`c++ -std=c++17 native-engine/tests/core_tests.cpp -o /tmp/eclipse-tests && /tmp/eclipse-tests`

The pure C++ unit tests assert fixed ground, world bounds, NPC corridors, layers and brightness cap. SDL / Android integration still needs on-device render inspection after importing the four PNG files. Build actions have not been triggered.
