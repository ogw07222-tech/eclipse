#!/usr/bin/env bash
# Replace only the five ECLIPSE demon castle entrance images.
# Run from anywhere: bash ~/eclipse/scripts/install-castle-entrance.sh
# Never touches ECLIPSE Studio files, other assets, or GitHub Actions.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ "$(git -C "$ROOT" remote get-url origin 2>/dev/null)" != *"ogw07222-tech/eclipse.git"* ]]; then
  echo "ERROR: run this script from a clone of ogw07222-tech/eclipse (not eclipse-studio)" >&2
  exit 2
fi
ZIP="${1:-$HOME/storage/downloads/demon_castle_fixed_assets.zip}"
if [[ ! -f "$ZIP" ]]; then
  echo "ERROR: ZIP not found: $ZIP" >&2
  exit 3
fi
# Validate file paths, dimensions and PNG headers without external Python modules.
python3 - "$ZIP" "$ROOT" <<'PY'
import os,sys,zipfile,struct,shutil,datetime
archive,root=sys.argv[1:]
directory='Assets/Resources/Eclipse/demon_castle/entrance/'
spec={'sky_far.png':(1280,720),'far_castle.png':(1280,720),
      'mid_ruins.png':(1600,720),'near_gate.png':(1600,720),
      'floor_repeat_512.png':(512,135)}
with zipfile.ZipFile(archive) as z:
    names=set(z.namelist())
    for name,dimensions in spec.items():
        path=directory+name
        if path not in names:raise SystemExit('Missing ZIP entry: '+path)
        blob=z.read(path)
        if blob[:8]!=b'\x89PNG\r\n\x1a\n':
            raise SystemExit('Invalid PNG: '+name)
        if struct.unpack('>II',blob[16:24])!=dimensions:
            raise SystemExit('Unexpected dimensions: '+name)
    dest=os.path.join(root,directory)
    backup=os.path.join(os.path.expanduser('~'),'eclipse-castle-backups',datetime.datetime.now().strftime('%Y%m%d-%H%M%S'))
    os.makedirs(backup,exist_ok=True)
    os.makedirs(dest,exist_ok=True)
    for name in spec:
        old=os.path.join(dest,name)
        if os.path.isfile(old):shutil.copy2(old,os.path.join(backup,name))
    for name in spec:
        source=directory+name
        target=os.path.join(dest,name)
        temp=target+'.tmp'
        with z.open(source) as inp,open(temp,'wb') as out:
            shutil.copyfileobj(inp,out)
        os.replace(temp,target)
    print('OK: installed five castle PNGs in game repo')
    print('Backups (if prior files existed):',backup)
PY
echo "Updated game assets; no build, commit or push performed."
echo "Next: cd '$ROOT' && git status --short Assets/Resources/Eclipse/demon_castle/entrance"
