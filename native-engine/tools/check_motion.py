#!/usr/bin/env python3
"""Validate actual PNG art and runtime layout, replacing retired rig assertions."""
from pathlib import Path
import hashlib,json,re
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]
stamp=json.loads((ROOT/'Art/knight/rig/build.json').read_text())
for section in stamp.values():
    for name,digest in section.items():
        assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest,('Repack motion after changing',name)
im=Image.open(ROOT/'Assets/Resources/Eclipse/knight_motion.png').convert('RGBA')
master=Image.open(ROOT/'Art/knight/redesign/knight_64x94_master.png').convert('RGBA')
assert im.size==(1024,1128)
assert set(im.getchannel('A').get_flattened_data())=={0,255},'Alpha must be binary'
clips=json.loads((ROOT/'Art/knight/rig/clips.json').read_text())
assert [c['count'] for c in clips]==[8,8,16,6,6,6,8,8,10,10,6,12]
occupied=set();unique=set()
for clip in clips:
    row=clip['start']//16
    assert len(clip['source_indices'])==clip['count']
    for i,index in enumerate(clip['source_indices']):
        cell=im.crop((i*64,row*94,(i+1)*64,(row+1)*94))
        expected=master.crop((index*64,row*94,(index+1)*64,(row+1)*94))
        assert cell.tobytes()==expected.tobytes(),(clip['name'],i,'packing mismatch')
        box=cell.getchannel('A').getbbox();assert box,(clip['name'],i,'blank frame')
        assert box[0]>=1 and box[2]<=63 and box[1]>=1 and box[3]<=89,(clip['name'],i,'clipped silhouette')
        occupied.add(clip['start']+i);unique.add(cell.tobytes())
for i in range(192):
    if i not in occupied:
        assert not im.crop((i%16*64,i//16*94,(i%16+1)*64,(i//16+1)*94)).getchannel('A').getbbox(),('unused cell',i)
header=(ROOT/'native-engine/src/motion.hpp').read_text()
for key,value in [('motionWidth',64),('motionHeight',94),('motionColumns',16),('motionRows',12),('motionPivotX',28),('motionPivotY',88)]:
    assert re.search(r'\b'+key+r'\s*=\s*'+str(value)+r'\b',header),key
assert len(occupied)==104 and len(unique)>=90
results=dict(atlas_pixels=list(im.size),cell_pixels=[64,94],clips=12,frames=len(occupied),unique_poses=len(unique),alpha=[0,255],pivot=[28,88],world_scale=1.85,source='PNG art; nearest-frame temporal resampling')
review=ROOT/'native-engine/review';review.mkdir(exist_ok=True)
(review/'metrics.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))
