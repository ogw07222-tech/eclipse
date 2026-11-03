# ECLIPSE forest PNG audit — 2026-11-03

## Scope and method

Reviewed **all 40 sprites** from `eclipse_forest_repo_ready.zip`, substituting four newer candidates from `eclipse_forest_cropping_fix_v2.zip` for the comparison. The original repository still contains older crop versions until the patch is imported. PNG file decoding, dimensions, alpha channel extent, and count of nontransparent border pixels on each side were checked. **Border-touching pixels are a warning to compare with the source illustration, not proof that art was lost.** This is an automated edge audit, not manual proof of complete artwork.

| Finding | Count | Interpretation |
| --- | ---: | --- |
| Transparent padding on all four sides in the proposed v2 patch | 4 | Main tree, secondary tree, bush 01 and bush 02. Safe extraction margins, but visual content still needs in-game review. |
| Alpha border pixels touch one or more PNG edges | 29 | Source crop may be truncated, or a legitimate ground contact/tile edge. Inspect individually before enabling additional species. |
| Fully opaque, non-seamless sampled art | 7 | 3 backgrounds and 4 ground tiles. **Do not directly repeat/tile** as rendered backdrops. |

## Asset risk groups

- **Current visual baseline (4 candidate fixes):** `trees_large/tree_large_01.png`, `trees_medium/tree_mid_01.png`, `bushes/bush_01.png`, `bushes/bush_02.png` have re-extracted candidates with alpha margins. The ZIP needs to be applied to the repository.
- **Unused tree variations requiring source review (7):** `trees_large/tree_large_02..05` and `trees_medium/tree_mid_02..04`. Keep out of the active two-species distribution.
- **Unused bush variations (3):** `bush_03..05`. `bush_03` has significant contact on both left and right; likely extraction risk.
- **Detail/vegetation/effects edge review (19):** `details/` six, `effects/` three, `grass/` four, `plants/` three, `rocks/` three. A few grass and rock sprites intentionally contact their bottom edge; others such as hanging vines touch the entire right edge and need wider source cropping.
- **Opaque references (7):** `background/forest_far_01..03` and `terrain/tile_ground_01..04`. Not suitable for seamless rendering without re-authoring.

## Render usage / decisions

- Keep the **two PNG tree types only**, bush01/02, grass01/02, rock01, and two subtle details.
- Do not bring back the procedural low-quality trees. The old `Far` layer currently has no generated instances.
- New soil uses brown layered earth, a thin static grass fringe, deterministic dithered strata and scattered stone pixels. This does **not** rely on the four unusable terrain tiles.
- Avoid treating alpha-padding alone as a fix for missing plant geometry: truly cut foliage must be redrawn or re-extracted from a wider source.
- Re-check screenshot quality at camera X=640 and 960 and size=60%, 100%, 180%; do not infer SDL final performance from the browser preview.

## Apply newer four-sprite patch

Download `eclipse_forest_cropping_fix_v2.zip` from the related chat and from repository root:

```sh
cd ~/eclipse
git pull --ff-only origin main
unzip -o ~/storage/downloads/eclipse_forest_cropping_fix_v2.zip -d .
git add Assets/Resources/Eclipse/forest
git commit -m "Apply wider source crops for active forest sprites"
git push origin main
```

The browser preview works without any Android build:
`python -m http.server 8765 --bind 127.0.0.1` → `http://127.0.0.1:8765/native-engine/preview/forest.html`.
