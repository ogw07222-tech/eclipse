#!/usr/bin/env python3
"""Read-only static PNG QA. Does not run gameplay/owner tests or approve art."""
import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
W, H = 112, 94
ASSET = ROOT / 'Assets/Resources/Eclipse'
SOURCE = ROOT / 'Art/knight/skill/original_4x6.png'
CLIPS = [('thrust', .48), ('sweep', .415), ('prepare', .40)]

def clip(atlas, n):
    x, y = (n % 4) * W, (n // 4) * H
    return atlas.crop((x, y, x+W, y+H))

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--json', type=Path, default=None)
    args = p.parse_args()
    result = {'scope':'read-only static alpha and frame inventory; not motion acceptance',
              'failures': [], 'warnings': [], 'skills': []}
    try:
        actor = Image.open(ASSET/'knight_skill_motion.png').convert('RGBA')
        fx = Image.open(ASSET/'knight_skill_fx_layer.png').convert('RGBA')
        source = Image.open(SOURCE).convert('RGBA')
    except (OSError, ValueError) as e:
        result['failures'].append('ECLIPSE ASSET ERROR: '+str(e))
        print(json.dumps(result, ensure_ascii=False, indent=2))
        return 1
    for label, image, size in [('actor',actor,(448,846)),('fx',fx,(448,564)),('source',source,(448,564))]:
        if image.size != size:
            result['failures'].append(f'ECLIPSE ASSET ERROR: {label} expected={size}, actual={image.size}')
    if not result['failures']:
        for skill, (name,duration) in enumerate(CLIPS):
            seen=set()
            detail={'name':name,'frames':[],'fps':round(12/duration,2)}
            for local in range(12):
                image=clip(actor,skill*12+local)
                alpha=image.getchannel('A')
                pixels=list(alpha.getdata())
                occupied=sum(x==255 for x in pixels)/(W*H)
                stray=sum(x not in (0,255) for x in pixels)
                seen.add(hashlib.sha256(image.tobytes()).hexdigest())
                detail['frames'].append({'index':local+1,'occupied':round(occupied,4),'soft_alpha':stray})
                if occupied>.55:
                    result['failures'].append(f'{name} {local+1}: {occupied:.1%} opaque — likely black rectangle')
                if stray:
                    result['failures'].append(f'{name} {local+1}: unexpected alpha {stray} pixels')
                if not alpha.getbbox():
                    result['failures'].append(f'{name} {local+1}: empty frame')
            if len(seen)<12:
                result['failures'].append(f'{name}: only {len(seen)}/12 unique RGBA frames')
            for old_i,new_i in enumerate((0,3,6,11)):
                if clip(actor,skill*12+new_i).tobytes()!=clip(source,skill*4+old_i).tobytes():
                    result['warnings'].append(f'{name}: original key {old_i+1} differs in frame {new_i+1}')
            result['skills'].append(detail)
    result['warnings'].append('Static inspection is NOT evidence of natural animation, gripping, anatomy or video playback.')
    result['status']='FAIL' if result['failures'] else 'STATIC_ONLY'
    result['production_approved']=False
    if args.json:
        args.json.parent.mkdir(parents=True,exist_ok=True)
        args.json.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(result,ensure_ascii=False,indent=2))
    return bool(result['failures'])

if __name__=='__main__':
    raise SystemExit(main())
