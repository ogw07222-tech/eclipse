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
for clip,stride in [('run',120),('walk',72)]:
    contact=.4 if clip=='run' else .5
    samples=[i/1000*(contact-.01) for i in range(1001)]
    xs=[b.foot(t,stride/3*contact,4,contact)[0]*3+stride*t for t in samples]
    drift=max(xs)-min(xs)
    assert drift<1e-6,(clip,'support foot slides')
    results[clip+'_analytic_support_drift_world_px']=round(drift,9)
    for i in range(1001):
        p=b.pose(clip,i/1000)
        for index,foot in enumerate(p['feet']):
            hip=(p['hip'][0]+(-1 if index==0 else 1),p['hip'][1])
            assert math.dist(hip,foot)<=14.01,(clip,i,'leg overextension')
for clip,start,count,duration in b.CLIPS:
    for i in range(count):
        p=b.pose(clip,i/(count if clip in ('idle','walk','run') else count-1))
        assert math.dist(p['shoulder'],p['near'])<=9.801,(clip,i,'right arm overextension')
        assert math.dist((p['chest'][0]-2,p['chest'][1]+1),p['far'])<=9.801,(clip,i,'off-hand arm overextension')
        # The entire shared sword, including its guard, stays inside its cell.
        angle=math.radians(p['sword'])
        for sx,sy in [(-2,-2),(9,-2),(9,2),(-2,2)]:
            x=p['near'][0]+sx*math.cos(angle)-sy*math.sin(angle)
            y=p['near'][1]+sx*math.sin(angle)+sy*math.cos(angle)
            assert 0<=x<64 and 0<=y<48,(clip,i,'blade clipped')
samples=[b.pose('run',i/1000) for i in range(1000)]
flight=sum(all(foot[1]<42.9 for foot in p['feet']) for p in samples)/len(samples)
assert flight>=.12,'Sprint has no airborne push-off phase'
heavy=[b.pose('strong_slash',i/1000) for i in range(1001)]
assert max(p['hip'][0] for p in heavy)-min(p['hip'][0] for p in heavy)>=4,'Heavy cut has no weight transfer'
thrust=[b.pose('thrust',i/1000) for i in range(1001)]
assert max(p['feet'][1][0]-p['feet'][0][0] for p in thrust)>=19,'Thrust lacks a deep lunge'
assert max(math.dist(p['shoulder'],p['near']) for p in thrust)>=9.5,'Thrust arm never extends'
assert max(b.pose('dash',i/1000)['lean'] for i in range(1001))>=70,'Dash remains upright'
results['sprint_airborne_fraction']=round(flight,3)
results['heavy_cut_weight_transfer_design_px']=round(max(p['hip'][0] for p in heavy)-min(p['hip'][0] for p in heavy),3)
results['thrust_stance_design_px']=round(max(p['feet'][1][0]-p['feet'][0][0] for p in thrust),3)
im=Image.open(ROOT/'Assets/Resources/Eclipse/knight_motion.png').convert('RGBA')
assert im.size==(1024,576),'Atlas dimensions'
assert set(im.getchannel('A').get_flattened_data())=={0,255},'Soft or missing transparency'
opaque_colors={pixel for pixel in im.get_flattened_data() if pixel[3]}
assert len(opaque_colors)<=12,'Pixel palette expanded'
for name,start,count,_ in b.CLIPS:
    for i in range(start,start+count):
        cell=im.crop((i%16*64,i//16*48,(i%16+1)*64,(i//16+1)*48))
        assert cell.getchannel('A').getbbox(),(name,i,'empty runtime frame')
results.update(atlas_pixels=[1024,576],clips=12,frames=104,alpha=[0,255],opaque_palette_colors=len(opaque_colors),design_pixel_scale_world=3,shared_sword='one fixed 11x4 design-pixel PNG, right hand')
review=ROOT/'native-engine/review';review.mkdir(exist_ok=True)
(review/'metrics.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))
