#!/usr/bin/env python3
"""Validate the baked atlas and the physical animation constraints."""
from pathlib import Path
import importlib.util
import json
import math
import hashlib
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('bake',ROOT/'Tools/bake_knight_motion.py')
b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
stamp=json.loads((ROOT/'Art/knight/rig/build.json').read_text())
for section in stamp.values():
    for name,digest in section.items():
        assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest,('Rebake motion after changing',name)
results={}
for clip,stride in [('run',96),('walk',72)]:
    samples=[i/1000*.49 for i in range(1001)]
    xs=[b.foot(t,stride/6,4)[0]*3+stride*t for t in samples]
    drift=max(xs)-min(xs)
    assert drift<1e-6,(clip,'support foot slides')
    results[clip+'_analytic_support_drift_world_px']=round(drift,9)
    for i in range(1001):
        p=b.pose(clip,i/1000)
        for index,foot in enumerate(p['feet']):
            hip=(p['hip'][0]+(-1 if index==0 else 1),p['hip'][1])
            assert math.dist(hip,foot)<=12.01,(clip,i,'leg overextension')
for clip,start,count,duration in b.CLIPS:
    for i in range(count):
        p=b.pose(clip,i/(count if clip in ('idle','walk','run') else count-1))
        assert math.dist(p['shoulder'],p['near'])<=9.801,(clip,i,'right arm overextension')
        # The entire shared sword, including its guard, stays inside its cell.
        angle=math.radians(p['sword'])
        for sx,sy in [(-2,-2),(9,-2),(9,2),(-2,2)]:
            x=p['near'][0]+sx*math.cos(angle)-sy*math.sin(angle)
            y=p['near'][1]+sx*math.sin(angle)+sy*math.cos(angle)
            assert 0<=x<48 and 0<=y<48,(clip,i,'blade clipped')
im=Image.open(ROOT/'Assets/Resources/Eclipse/knight_motion.png').convert('RGBA')
assert im.size==(768,576),'Atlas dimensions'
assert set(im.getchannel('A').get_flattened_data())=={0,255},'Soft or missing transparency'
opaque_colors={pixel for pixel in im.get_flattened_data() if pixel[3]}
assert len(opaque_colors)<=12,'Pixel palette expanded'
for name,start,count,_ in b.CLIPS:
    for i in range(start,start+count):
        cell=im.crop((i%16*48,i//16*48,(i%16+1)*48,(i//16+1)*48))
        assert cell.getchannel('A').getbbox(),(name,i,'empty runtime frame')
results.update(atlas_pixels=[768,576],clips=12,frames=104,alpha=[0,255],opaque_palette_colors=len(opaque_colors),design_pixel_scale_world=3,shared_sword='one fixed 11x4 design-pixel PNG, right hand')
review=ROOT/'native-engine/review';review.mkdir(exist_ok=True)
(review/'metrics.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))
