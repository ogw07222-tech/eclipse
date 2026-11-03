#!/usr/bin/env python3
"""Validate and import the 40 PNG ECLIPSE forest pack without requiring Pillow."""
from pathlib import Path
import argparse
import json
import struct
import zipfile

SCRIPT_REPO = Path(__file__).resolve().parents[2]
DEST = SCRIPT_REPO / 'Assets/Resources/Eclipse/forest'

def validate_png(data, name, expected):
    if not data.startswith(b'\x89PNG\r\n\x1a\n') or data[12:16] != b'IHDR':
        raise ValueError(f'Invalid PNG: {name}')
    width, height = struct.unpack_from('>II', data, 16)
    if (width, height) != (expected['width'], expected['height']):
        raise ValueError(f'Dimension mismatch: {name}')
    if not (0 < width <= 4096 and 0 < height <= 4096):
        raise ValueError(f'Invalid dimensions: {name}')

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('zip', type=Path, help='eclipse_forest_png_40.zip')
    parser.add_argument('--dest', type=Path, default=DEST)
    parser.add_argument('--check-only', action='store_true')
    args = parser.parse_args()
    with zipfile.ZipFile(args.zip) as archive:
        manifest = json.loads(archive.read('manifest.json'))
        assets = manifest['assets']
        if manifest.get('asset_count') != 40 or len(assets) != 40:
            raise ValueError('Expected exactly 40 forest assets')
        names = [entry['path'] for entry in assets]
        expected = (
            [f'background/forest_far_{i:02d}.png' for i in range(1, 4)] +
            [f'trees_large/tree_large_{i:02d}.png' for i in range(1, 6)] +
            [f'trees_medium/tree_mid_{i:02d}.png' for i in range(1, 5)] +
            [f'bushes/bush_{i:02d}.png' for i in range(1, 6)] +
            [f'grass/grass_{i:02d}.png' for i in range(1, 5)] +
            [f'rocks/rock_{i:02d}.png' for i in range(1, 4)] +
            [f'terrain/tile_ground_{i:02d}.png' for i in range(1, 5)] +
            ['details/fallen_log_01.png', 'details/flower_01.png',
             'details/hanging_vines_01.png', 'details/mushroom_01.png',
             'details/mushroom_02.png', 'details/stump_01.png'] +
            [f'plants/plant_{i:02d}.png' for i in range(1, 4)] +
            [f'effects/effect_{i:02d}.png' for i in range(1, 4)]
        )
        if set(names) != set(expected):
            raise ValueError('Asset names do not match the native renderer catalog')
        if len(set(names)) != 40:
            raise ValueError('Duplicate asset paths')
        valid_categories = {'background','trees_large','trees_medium','bushes',
                            'grass','rocks','terrain','details','plants','effects'}
        for entry in assets:
            relative = Path(entry['path'])
            if (relative.is_absolute() or '..' in relative.parts or
                    len(relative.parts) != 2 or relative.parts[0] not in valid_categories or
                    relative.suffix.lower() != '.png'):
                raise ValueError(f'Unsafe asset path: {relative}')
            data = archive.read(entry['path'])
            validate_png(data, entry['path'], entry)
            if not args.check_only:
                output = args.dest / relative
                output.parent.mkdir(parents=True, exist_ok=True)
                output.write_bytes(data)
        if not args.check_only:
            (args.dest / 'manifest.json').write_text(
                json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(f'PASS: {len(names)} forest PNGs validated' +
          (' (no files written)' if args.check_only else f' -> {args.dest}'))

if __name__ == '__main__':
    main()
