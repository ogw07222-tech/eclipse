#!/usr/bin/env python3
"""Pack approved PNG poses into the existing 104-frame runtime layout."""
from pathlib import Path
import hashlib,json
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
MASTER=ROOT/'Art/knight/redesign/knight_64x94_master.png'
OUT=ROOT/'Assets/Resources/Eclipse/knight_motion.png'
CLIPS=[('idle',0,8,.96,7),('walk',16,8,.60,7),('run',32,16,.428571,14),('jump',48,6,.52,6),('fall',64,6,.65,6),('land',80,6,.12,6),('dash',96,8,.15,8),('weak_slash',112,8,.23,8),('strong_slash',128,10,.34,10),('thrust',144,10,.42,10),('hurt',160,6,.23,6),('death',176,12,.90,11)]
def bake_trails(metadata):
    paths=json.loads((ROOT/'Art/knight/redesign/blade_paths.json').read_text())
    frames=[(0,0,0)]*192;spans=[]
    windows={'weak_slash':(3,4),'strong_slash':(5,7),'thrust':(4,6)}
    for clip in metadata:
        name=clip['name']
        if name not in paths:continue
        begin,end=windows[name]
        for i in range(begin,end+1):
            src=clip['source_indices'][i];wrist=paths[name][src][:2];tip=paths[name][src][2:]
            prev=paths[name][clip['source_indices'][max(0,i-1)]][2:]
            mask=Image.new('L',(72,100));draw=ImageDraw.Draw(mask)
            if name=='thrust':
                draw.polygon([(tip[0]-3,tip[1]-1),(tip[0]+4,tip[1]),(tip[0]-3,tip[1]+1)],fill=2)
                draw.point((tip[0]+2,tip[1]),fill=3)
            else:
                inner=(round(wrist[0]+(tip[0]-wrist[0])*.65),round(wrist[1]+(tip[1]-wrist[1])*.65))
                draw.polygon([prev,tip,inner],fill=1)
                draw.line([prev,tip],fill=2,width=1);draw.point(tuple(tip),fill=3)
            start=len(spans)
            for y in range(mask.height):
                x=0
                while x<mask.width:
                    shade=mask.getpixel((x,y));right=x+1
                    while right<mask.width and mask.getpixel((right,y))==shade:right+=1
                    if shade:spans.append((x,y,right-x,shade))
                    x=right
            frames[clip['start']+i]=(start,len(spans)-start,255 if i==begin else 150)
    header='#pragma once\nnamespace eclipse {\nstruct SwordTrailSpan{int x,y,width,shade;};\nstruct SwordTrailFrame{int start,count,alpha;};\nconstexpr SwordTrailFrame swordTrailFrames[]={\n'
    header+=''.join('{%d,%d,%d},'%v for v in frames)+'};\nconstexpr SwordTrailSpan swordTrailSpans[]={\n'
    header+=''.join('{%d,%d,%d,%d},'%v for v in spans)+'};\n}\n'
    (ROOT/'native-engine/src/sword_trails.hpp').write_text(header)

def bake():
    source=Image.open(MASTER)
    if source.mode!='P':raise ValueError('Master must use the approved indexed pixel palette')
    if source.size!=(1024,1128):raise ValueError('Master must be 16x12 cells of 64x94')
    atlas=Image.new('P',source.size,source.getpixel((0,0)));atlas.putpalette(source.getpalette('RGBA'),rawmode='RGBA');atlas.info['transparency']=source.getpixel((0,0));metadata=[]
    review=ROOT/'native-engine/review';review.mkdir(parents=True,exist_ok=True)
    for name,start,count,duration,source_count in CLIPS:
        row=start//16;frames=[];indices=[];loop=name in ('idle','walk','run')
        for i in range(count):
            index=min(source_count-1,int(i*source_count/count)) if loop else round(i*(source_count-1)/(count-1))
            if name=='weak_slash':index=[0,1,2,4,3,5,6,7][i]
            cell=source.crop((index*64,row*94,(index+1)*64,(row+1)*94))
            if not cell.convert('RGBA').getchannel('A').getbbox():raise ValueError((name,index,'empty source'))
            atlas.paste(cell,(i*64,row*94));frames.append(cell.convert('RGBA'));indices.append(index)
        metadata.append(dict(name=name,start=start,count=count,duration=duration,loop=loop,source_count=source_count,source_indices=indices))
        previews=[]
        for frame in frames:
            bg=Image.new('RGBA',(64,94),(20,26,35,255));bg.alpha_composite(frame)
            previews.append(bg.convert('RGB').resize((256,376),Image.Resampling.NEAREST))
        previews[0].save(review/(name+'.gif'),save_all=True,append_images=previews[1:],duration=max(20,round(duration/count*1000)),loop=0)
    OUT.parent.mkdir(parents=True,exist_ok=True);atlas.save(OUT)
    (ROOT/'Art/knight/rig/clips.json').write_text(json.dumps(metadata,indent=2)+'\n')
    (review/'motion.json').write_text(json.dumps(metadata,indent=2)+'\n')
    bake_trails(metadata)
    inputs=[ROOT/'Art/knight/redesign/blade_paths.json',MASTER,Path(__file__).resolve(),ROOT/'Art/knight/rig/clips.json']
    outputs=[OUT,ROOT/'native-engine/src/motion.hpp',ROOT/'native-engine/src/sword_trails.hpp']
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    stamp={'inputs':{str(p.relative_to(ROOT)):digest(p) for p in inputs},'outputs':{str(p.relative_to(ROOT)):digest(p) for p in outputs}}
    (ROOT/'Art/knight/rig/build.json').write_text(json.dumps(stamp,indent=2)+'\n')
    print('Packed 104 playable frames from 99 authored PNG poses; 64x94 cells; 1024x1128 atlas.')
if __name__=='__main__':bake()
