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
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
assets = ['knight_motion.png', 'ui/attack_states.png', 'ui/dash_states.png',
          'ui/swap_skill_states.png', 'ui/joystick_arrows_new.png']
# Forest assets are packaged by the Android Gradle Sync task. Verify all actual
# source PNG files instead of assuming the historical five-PNG baseline.
art_root = root / 'Assets/Resources/Eclipse'
forest_assets = []
for forest_folder in ('forest', 'forest_v2', 'forest_v3', 'forest_entrance'):
    forest_root = art_root / forest_folder
    if forest_root.is_dir():
        forest_assets.extend(sorted(
            forest_folder + '/' + path.relative_to(forest_root).as_posix()
            for path in forest_root.rglob('*.png')
        ))
assets += forest_assets
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
    packaged = [n for n in apk.namelist() if n.startswith('assets/') and n.endswith('.png')]
    expected_names = {'assets/' + name for name in assets}
    actual_names = set(packaged)
    assert actual_names == expected_names, (
        'APK PNG set differs from source: '
        f'missing={sorted(expected_names - actual_names)}, '
        f'unexpected={sorted(actual_names - expected_names)}'
    )
digest = hashlib.sha256(args.apk.read_bytes()).hexdigest()
args.apk.with_suffix('.apk.sha256').write_text(f'{digest}  {args.apk.name}\n')
print(json.dumps({'apk': str(args.apk), 'bytes': args.apk.stat().st_size, 'abi': args.abi,
                  'pngs': len(assets), 'forest_pngs': len(forest_assets), 'sha256': digest}, indent=2))
