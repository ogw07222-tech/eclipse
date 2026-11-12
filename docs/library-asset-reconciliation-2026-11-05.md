# Library-to-repository asset reconciliation (2026-11-05)

This register records confirmed runtime paths on **main** and the newer assets identified in the ECLIPSE ChatGPT Library. Do not automatically replace production PNGs with concept sheets: source images can be collages rather than engine-ready atlases.

## Present in GitHub (confirmed by file existence and source references)
- Mage: `Assets/Resources/Eclipse/mage_run.png`, `mage_motion.png`; loaded by `native-engine/src/presentation.hpp`.
- Knight: `knight_motion.png` and `knight_skill_motion.png`.
- White Night: five `ui/white_fx_*.png` atlases and lance PNG.
- Title composition: `title/bg_clean.png`, `title/knight_fg.png`, `title/cape.png`; loaded by `loadTitle()`.
- Castle entrance: `sky_far.png`, `far_castle.png`, `mid_ruins.png`, `near_gate.png`, `floor_repeat_512.png`; referenced by `demon_castle_entrance.hpp`.
- First playable scene explicitly selects the castle entrance in `native-engine/src/main.cpp`.

## Newer Library packages detected, but NOT imported
The latest Library ZIP bytes could not be materialized in the current session, so replacing any game PNG with their contents would be speculative:
- `demon_castle_clean_no_fake_masks.zip` — likely newer castle cutout/layer revision.
- `demon_castle_fixed_assets.zip` — another castle correction; compare versions before replacing.
- `eclipse_mage_ingame_pack.zip` — verify actual frame mapping against current mage atlases.
- `eclipse_title_layers_corrected.zip` — confirm alpha masks against shipped title layers.
- The various named Flash Knight/White Night concept motion sheets — require per-frame consistency, alpha masking, dimensions, and pose-to-clip mapping validation before use.

## Code integration in this branch
- Missing corrected title layers no longer silently revert to the old composite.
- Missing/malformed mage and knight skill motion PNGs are now explicit load errors, rather than unnoticed legacy art.
- Any subsequent import must pass a startup PNG dimension check and an Android Studio runtime/playback test.

## Unverified
- Whether the *pixel contents* of the current GitHub PNGs match newer Library ZIPs.
- Whether all animation loops are smooth in actual gameplay.
- Whether Studio's external asset pack includes every new file.

Do not claim that the ZIP asset revisions were imported until their original bytes become accessible and are reviewed.
