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
assert im.size==(1024,1222)
assert set(im.getchannel('A').get_flattened_data())=={0,255},'Alpha must be binary'
clips=json.loads((ROOT/'Art/knight/rig/clips.json').read_text())
assert [c['count'] for c in clips]==[8,8,16,6,6,6,8,8,10,10,6,12,8]
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
for i in range(208):
    if i not in occupied:
        assert not im.crop((i%16*64,i//16*94,(i%16+1)*64,(i//16+1)*94)).getchannel('A').getbbox(),('unused cell',i)
header=(ROOT/'native-engine/src/motion.hpp').read_text()
for key,value in [('motionWidth',64),('motionHeight',94),('motionColumns',16),('motionRows',13),('motionPivotX',28),('motionPivotY',88)]:
    assert re.search(r'\b'+key+r'\s*=\s*'+str(value)+r'\b',header),key
assert len(occupied)==112 and len(unique)==112
assert all(c['source_count']==c['count'] for c in clips),'Every motion must retain authored detail at every runtime frame'
assert len({p for p in im.get_flattened_data() if p[3]})<=95,'Motions must share the same pixel palette'
for start in (0,16,96,112,128,144,160,176):
    box=im.crop((0,start//16*94,64,(start//16+1)*94)).getchannel('A').getbbox()
    assert 50<=box[3]-box[1]<=56,('Inconsistent ready-pose body height',start)
results=dict(atlas_pixels=list(im.size),cell_pixels=[64,94],clips=13,frames=len(occupied),unique_poses=len(unique),alpha=[0,255],pivot=[28,88],world_scale=1.85,source='112 authored PNG poses; shared 96-color palette and ready-pose scale')
# Blade annotations are checked against the ACTUAL steel pixels, not a retired rig.
import importlib.util,math
spec=importlib.util.spec_from_file_location('knight_bake',ROOT/'Tools/bake_knight_motion.py')
bake=importlib.util.module_from_spec(spec);spec.loader.exec_module(bake)
paths=json.loads((ROOT/'Art/knight/redesign/blade_paths.json').read_text())
trail_samples=0;maximum_tip_error=0.
for clip in clips:
    name=clip['name'];row=clip['start']//16
    if name in paths:
        assert len(paths[name])==clip['source_count']
        for i,(wx,wy,tx,ty) in enumerate(paths[name]):
            assert 5<=math.dist((wx,wy),(tx,ty))<=(30 if name=='air_slam' else 23),(name,i,'blade size outside reference proportions')
            distances=[]
            for y in range(max(0,ty-2),min(94,ty+3)):
                for x in range(max(0,tx-2),min(64,tx+3)):
                    r,g,b,a=master.getpixel((i*64+x,row*94+y))
                    if a and min(r,g,b)>95:distances.append(math.dist((tx,ty),(x,y)))
            error=min(distances,default=999)
            assert error<=2,(name,i,'annotated tip detached from visible blade')
            maximum_tip_error=max(maximum_tip_error,error)
    for i in range(clip['count']):
        mask,alpha=bake.trail_mask(name,i,clip['source_indices'],paths)
        window=bake.TRAIL_WINDOWS.get(name)
        active=window and window[0]<=i<=window[1]
        if not active:
            assert not alpha and not mask.getbbox(),(name,i,'effect outside strike')
            continue
        assert alpha>0 and mask.getbbox(),(name,i,'missing strike effect')
        current=paths[name][clip['source_indices'][i]]
        previous=paths[name][clip['source_indices'][i-1]]
        assert mask.getpixel(tuple(v+o for v,o in zip(current[2:],bake.FX_ORIGIN)))==3,(name,i,'leading edge misses current sword tip')
        # The energy extends past the steel while following the moving wrist.
        ax,ay=previous[:2];bx,by=current[:2];dx,dy=bx-ax,by-ay
        max_radius=max(math.dist(previous[:2],previous[2:]),math.dist(current[:2],current[2:]))*(bake.FX_REACH[name]+.4)+3
        box=mask.getbbox();assert box[0]>0 and box[1]>0 and box[2]<mask.width and box[3]<mask.height
        furthest=0.0
        for y in range(box[1],box[3]):
            for x in range(box[0],box[2]):
                if not mask.getpixel((x,y)):continue
                px,py=x-bake.FX_ORIGIN[0],y-bake.FX_ORIGIN[1]
                t=max(0,min(1,((px-ax)*dx+(py-ay)*dy)/max(.001,dx*dx+dy*dy)))
                distance=math.dist((px,py),(ax+dx*t,ay+dy*t))
                furthest=max(furthest,distance)
                assert distance<=max_radius,(name,i,'oversized sword effect')
        assert furthest>math.dist(current[:2],current[2:])*1.25,(name,i,'energy does not extend beyond steel')
        trail_samples+=1
    if name in paths:
        contact=int(clip['count']*float(re.search(r'\{'+str(clip['start'])+r','+str(clip['count'])+r',[\d.]+f,([\d.]+)f',header).group(1)))
        assert bake.trail_mask(name,contact,clip['source_indices'],paths)[1]>0,(name,'contact has no effect')
assert re.search(r'slamContactTipX='+str(paths['air_slam'][5][2])+r',slamContactTipY='+str(paths['air_slam'][5][3]),header),'Landing flash origin differs from sword tip'
results.update(sword_effect_frames=trail_samples,maximum_blade_tip_annotation_error_px=maximum_tip_error,sword_effect_scale='extended from the steel path; padded effect coordinates preserve actor registration')

review=ROOT/'native-engine/review';review.mkdir(exist_ok=True)
(review/'metrics.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

