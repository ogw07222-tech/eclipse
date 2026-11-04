#!/usr/bin/env python3
"""Check real APK structure, native libraries, packaged asset hashes and ELF ABI."""
from pathlib import Path
import argparse
import hashlib
import json
import struct
import zipfile

parser = argparse.ArgumentParser()
parser.add_argument('apk', type=Path)
parser.add_argument('--abi', default='arm64-v8a', choices=['arm64-v8a', 'x86_64'])
parser.add_argument('--all-assets', action='store_true',
                    help='For Studio APK: validate every PNG/JSON resource rather than legacy Game APK subset')
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
assets = ['knight_motion.png', 'mage_run.png', 'mage_motion.png', 'ui/attack_states.png', 'ui/dash_states.png',
          'ui/swap_skill_states.png', 'ui/joystick_arrows_new.png',
          'ui/settings_gear.png', 'ui/settings_menu_panel.png', 'ui/settings_window.png', 'ui/enemy_bar_frame.png', 'ui/execute_skull.png']
# Directory assets are packaged by the Android Gradle Sync task. Verify all actual
# source PNG files, including monsters, instead of assuming a fixed PNG baseline.
art_root = root / 'Assets/Resources/Eclipse'
folder_assets = {}
for folder in ('monsters', 'forest', 'forest_v2', 'forest_v3', 'forest_entrance'):
    folder_root = art_root / folder
    folder_assets[folder] = sorted(
        folder + '/' + path.relative_to(folder_root).as_posix()
        for path in folder_root.rglob('*.png')
    ) if folder_root.is_dir() else []
    assets.extend(folder_assets[folder])
forest_assets = sum(len(folder_assets[folder]) for folder in ('forest', 'forest_v2', 'forest_v3', 'forest_entrance'))
if args.all_assets:
    # Studio now bundles the complete resource tree. Keep the standalone Game
    # APK's original subset validation unchanged unless this flag is selected.
    assets = sorted(path.relative_to(art_root).as_posix()
                    for path in art_root.rglob('*')
                    if path.is_file() and path.suffix.lower() in ('.png', '.json'))
with zipfile.ZipFile(args.apk) as apk:
    if apk.testzip():
        raise SystemExit('APK ZIP integrity failed')
    assert 'AndroidManifest.xml' in apk.namelist(), 'Missing manifest'
    for name in ['libSDL3.so', 'libSDL3_image.so', 'libmain.so']:
        binary = apk.read(f'lib/{args.abi}/{name}')
        assert binary[:4] == b'\x7fELF', 'Not an ELF: ' + name
        assert binary[4] == 2, 'Not 64-bit ELF: ' + name
        machine = struct.unpack_from('<H', binary, 18)[0]
        assert machine == (183 if args.abi == 'arm64-v8a' else 62), 'Wrong ABI: ' + name
    for asset in assets:
        expected = (root / 'Assets/Resources/Eclipse' / asset).read_bytes()
        assert apk.read('assets/' + asset) == expected, 'Asset changed or missing: ' + asset
    asset_exts = ('.png', '.json') if args.all_assets else ('.png',)
    packaged = [n for n in apk.namelist() if n.startswith('assets/') and n.endswith(asset_exts)]
    expected_names = {'assets/' + name for name in assets}
    actual_names = set(packaged)
    assert actual_names == expected_names, (
        'APK asset set differs from source: '
        f'missing={sorted(expected_names - actual_names)}, '
        f'unexpected={sorted(actual_names - expected_names)}'
    )
digest = hashlib.sha256(args.apk.read_bytes()).hexdigest()
args.apk.with_suffix('.apk.sha256').write_text(f'{digest}  {args.apk.name}\n')
print(json.dumps({'apk': str(args.apk), 'bytes': args.apk.stat().st_size, 'abi': args.abi,
                  'pngs': len(assets), 'forest_pngs': forest_assets, 'monster_pngs': len(folder_assets['monsters']), 'sha256': digest}, indent=2))
