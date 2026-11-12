# Flash Executioner PixelFX candidates
Original white-gold/cyan light motifs, not copied game sprites. Actor artwork never baked into runtime effect PNGs.
15 independent PNG atlases at 60fps with metadata, source JS/core (MIT), dark/light contacts and GIF previews.
manifest.json is the exact runtime mapping, pivot in cell pixel coordinates, world scale applied uniformly. Default facing right; mirror around pivot for left-facing.
All one-shots begin and end empty. Loop orb has a natural loop. Lance and sky-lance use one physical projectile image per existing gameplay projectile: no built-in fake 20-lance rain. Use existing runtime 20-projectile scheduling/targeting unchanged.
Source method: three draft/review/refine rounds plus main-requested fourth mass/brightness correction. check.mjs passes all 15 for metadata, deterministic randomness, bounds, loop and final cleanup; procedural render performance warnings may appear on large effects. Runtime should load baked PNGs, not execute JS. No game tests/build/GitHub Actions run.
Layer notes: verdict charge is behind actor. Other atlas combines its back/foreground drawing; runtime front preserves thin ring clarity. Character remains separate.
GIFs use 20ms approximate playback only. Runtime fps is 60 and metadata lifetime is authoritative; don't derive runtime speed from GIF.
Status: awaiting main agent approval. PNG bytes untouched after export except metadata/preview packing.

Projectile flight: use manifest flightFrameRange [3,8] inclusive, cycle with per-projectile visualFlightAge until collision; do not use globalclock or fade-to-empty endpoints. SkyLance native direction is +90 degrees/downward; runtime rotation is target angle - 90 degrees.
