# Flash Executioner — upstream PixelFX resource variants v2
All 15 roles use existing finished resources from https://github.com/patch-gg/pixelfx commit5294518ce7c72e48b6100a744596e0acf57f2826.
No old executor custom drawKind or _lib.js recipe reused. Every effect draw/create and upstream helper/core file is byte-identical to its source. source-verification.json verifies leaf hashes against git show; source-original/ preserves the actual originals, dependencies and MIT license.

Changes are palette, existing options(radius/reach/height/length/stone), supplied actual actor masks, canvas registration, uniform runtime scale, and timing/frame selection. Exception requested by the owner: `Lance` and `SkyLance` atlases are additionally edited pixel-by-pixel in `manual-corrections/lance/` to replace the broad sword silhouette with a long, thin shaft and tapered point. No AI or effect re-render was used; the exact source recipe and pre-edit atlas remain preserved.
originals/ preserves canonical30fps original exports. Each variants/<Role>/original-60fps.png and before-after.png uses same60fps/source seed as variant, with same actual actor input for fair comparison. Canonical original packing artifact remains preserved there as historical export.

Packing correction: upstream spriteSheet calls canvas.paste which multiplies RGB by alpha onto transparent atlas and retains original alpha. That produces double-premultiplied halos under standard straight-alpha blending. render-executor-v2.mjs now copies existing r.frames.clear straight RGBA bytes directly into the atlas. Before RGBA25,23,17,25 becomes correctly straight RGBA255,235,173,25. No effect source modified. packing-correction/ contains before/after light/dark comparison evidence.

Command, Dash and Jump finite clips have alpha-only edge fade; exact per-frame mapping/envelope is frame-map.json. No actor pose interpolation or fake frame duplication. The lance silhouette change is literal RGBA pixel painting recorded with guarded source hash and coordinates; other PNG frames remain re-packed samples of original resources. Each atlas manifest records actual count/fps/unique pixels and terminal transparency. Loop Orb/Lance/SkyLance keep complete original loop, not old v1 flight-frame ranges.

Actor inputs: BlinkOut uses approved skill3 frame10 tuck; BlinkIn uses frame12 braced arrival; Dash uses approved dash frame2. Actual pixel crops/masks are supplied through original o.actor API. Approved feet pivot70/133. Role inputJSON records source sheet hash, index, columns and bbox; source-pose-PNGs are preserved. Character artwork itself is not redrawn.

Alignment: actor effects at actor feet plus offset. Command offset10,-25 puts source hand focal15,-22 at actual fist42,-71 world pixels. Impact uses exposed source height0, so center maps directly to hit center. LanceHit smite sourcewindow starts.55s(first60fps sample on/after source HIT.54), so peak0 anchors target feet at actual collision. Orb source focal0,-52 requires offset0,+15.6 at scale.3 to center its light on world orbit point. Both Lance/SkyLance native direction0(right): rotate to actual projectile angle, no extra -90. Sky is shown90degree in actor-preview only.

Visual peaks are snapped to exported frame-map samples, not estimated continuous source time. Slash triggerLead1/30s, Rush.05s, Command.2s: start effect that far before unchanged actor contact. BlinkIn starts at spawnAge2/15s on arrival collision; Impact spawnAge1/60s. BlinkOut triggerStartAge.0333333s aligns source collapse sample with teleport.15s. Verdict is charge attached at action start, not damage timing. Existing gameplay/20-lance targeting/physics remain unchanged.

Each variants/<Role>/ contains exact PNG+JSON, source-frame map, same-fps original PNG, original-vs-variant contact, dark/light contact and approximate GIF. GIF20ms timing is inspection only; atlasfps60 and descriptor lifetime are authoritative.

QA: all15 source leaf hashes match pinned upstream, allPNG hashes match manifest, no nontransparent edge pixels, all12 one-shots end transparent, 3 full loops remain visible. Upstream check reports13pass0fail0warnings in upstream-check.txt; expanded registered variant PNGs have no border pixels. No game/device tests, builds, GitHub Actions or GitHub writes performed by this authoring agent. Main agent must review before runtime import.


## Reproduce the exporter
The render script is packing/export tooling, not a new effect recipe. Existing draw/create recipes come from the pinned upstream checkout. Its relative ./lib imports resolve when it is placed under that checkout's tools directory.

From the ECLIPSE repository root, choose separate writable output/check-out paths:

```sh
git clone https://github.com/patch-gg/pixelfx /path/to/pixelfx-source
git -C /path/to/pixelfx-source checkout 5294518ce7c72e48b6100a744596e0acf57f2826
cp Art/flash-executioner/pixelfx/render-executor-v2.mjs /path/to/pixelfx-source/tools/render-executor-v2.mjs
mkdir -p /path/to/fx-export
cp Art/flash-executioner/pixelfx/actor-input*.json /path/to/fx-export/
PIXELFX_EXPORT_DIR=/path/to/fx-export node /path/to/pixelfx-source/tools/render-executor-v2.mjs
```

PIXELFX_EXPORT_DIR overrides only output/registration file location. It does not change any drawing function, source palette/options, timing selection or PNG pixels. Source role masks and input JSON are preserved beside this script; use those actual inputs instead of a stock character. The reviewed PNGs remain authoritative until a re-export is statically and visually reviewed again. These instructions are supplied for reproduction; no export, game build or test was rerun during native integration.
