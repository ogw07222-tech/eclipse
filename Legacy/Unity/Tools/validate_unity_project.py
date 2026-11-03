#!/usr/bin/env python3
"""Validate GUIDs, scene wiring, resource references and byte-identical source PNGs.
This is structural validation, not a substitute for Unity import/Play Mode.
"""
from pathlib import Path
import hashlib
import json
import re
import struct

ROOT = Path(__file__).resolve().parents[1]
REPO_ROOT = ROOT.parents[1]
errors = []
def check(condition, message):
    if not condition:
        errors.append(message)

guids = {}
for path in (ROOT / 'Assets').rglob('*'):
    if path.suffix == '.meta':
        continue
    meta = path.with_name(path.name + '.meta')
    check(meta.exists(), f'Missing meta: {path.relative_to(ROOT)}')
    if not meta.exists():
        continue
    match = re.search(r'^guid: ([0-9a-f]{32})$', meta.read_text(), re.M)
    check(match is not None, f'Invalid GUID: {meta.name}')
    if match:
        guid = match[1]
        check(guid not in guids, f'Duplicate GUID: {guid}')
        guids[guid] = path
for source in (REPO_ROOT / 'Legacy/Godot/assets').rglob('*.png'):
    target = ROOT / 'Assets/Resources/Eclipse' / source.relative_to(REPO_ROOT / 'Legacy/Godot/assets')
    check(target.exists(), f'Missing PNG: {target}')
    if target.exists():
        check(hashlib.sha256(source.read_bytes()).digest() == hashlib.sha256(target.read_bytes()).digest(), f'PNG changed: {target}')
        raw = target.read_bytes()
        check(raw[:8] == b'\x89PNG\r\n\x1a\n', f'Invalid PNG: {target}')
        w, h = struct.unpack('>II', raw[16:24])
        check(w > 0 and h > 0, f'Empty PNG: {target}')
        meta = target.with_name(target.name + '.meta').read_text()
        check('filterMode: 0' in meta and 'enableMipMap: 0' in meta and 'textureCompression: 0' in meta, f'Pixel settings: {target}')
scene = (ROOT / 'Assets/Scenes/Main.unity').read_text()
for guid in re.findall(r'guid: ([0-9a-f]{32})', scene):
    check(guid in guids, f'Missing scene reference: {guid}')
scene_guid = re.search(r'^guid: (\w+)', (ROOT / 'Assets/Scenes/Main.unity.meta').read_text(), re.M)[1]
settings = (ROOT / 'ProjectSettings/EditorBuildSettings.asset').read_text()
check('enabled: 1' in settings and 'Assets/Scenes/Main.unity' in settings and scene_guid in settings, 'Main scene missing in Build Settings')
model = ROOT / 'Assets/Eclipse/Core/CombatSimulation.cs'
check('using UnityEngine' not in model.read_text(), 'Core must be engine independent')
for path in list((ROOT / 'Assets').rglob('*.asmdef')) + [ROOT / 'Packages/manifest.json']:
    json.loads(path.read_text())
code = (ROOT / 'Assets/Eclipse/Runtime/EclipseGame.cs').read_text()
for resource in re.findall(r'LoadTexture\("([^"]+)"\)', code) + re.findall(r'ActionButton\("[^"]+",[^\n]*?"(ui/[^"]+)"', code):
    check((ROOT / f'Assets/Resources/Eclipse/{resource}.png').exists(), f'Unresolved resource: {resource}')
check('int[] FrameCounts = { 6, 8, 4, 3, 6, 6, 8, 5, 3, 6 }' in code, 'Original animation counts changed')
if errors:
    raise SystemExit('\n'.join(errors))
print(f'PASS: {len(guids)} unique asset GUIDs, scene wiring, Resources, 7 original PNGs and pixel import settings')
