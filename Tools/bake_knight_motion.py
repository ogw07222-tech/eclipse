#!/usr/bin/env python3
"""Pack approved PNG poses into the existing 104-frame runtime layout."""
from pathlib import Path
import hashlib,json
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
MASTER=ROOT/'Art/knight/redesign/knight_64x94_master.png'
OUT=ROOT/'Assets/Resources/Eclipse/knight_motion.png'
CLIPS=[('idle',0,8,.96,7),('walk',16,8,.60,7),('run',32,16,.428571,14),('jump',48,6,.52,6),('fall',64,6,.65,6),('land',80,6,.12,6),('dash',96,8,.15,8),('weak_slash',112,8,.23,8),('strong_slash',128,10,.34,10),('thrust',144,10,.42,10),('hurt',160,6,.23,6),('death',176,12,.90,11)]
TRAIL_WINDOWS={'weak_slash':(3,5),'strong_slash':(5,7),'thrust':(4,6)}

def trail_mask(name,i,indices,paths):
    """Sweep the actual blade segment in cell coordinates, behind the actor."""
    import math
    mask=Image.new('L',(96,128));draw=ImageDraw.Draw(mask)
    if name not in TRAIL_WINDOWS:return mask,0
    begin,end=TRAIL_WINDOWS[name]
    if not begin<=i<=end:return mask,0
    w=paths[name][indices[i]][:2];tip=paths[name][indices[i]][2:]
    old=paths[name][indices[i-1]]
    length=math.dist(w,tip)
    if name=='thrust':
        # Compact forward wake, proportional to the visible short blade.
        dx,dy=(tip[0]-w[0])/length,(tip[1]-w[1])/length
        nx,ny=-dy,dx;reach=min(6,length*.4);half=max(1,length*.10)
        point=lambda along,side:(round(tip[0]+dx*along+nx*side),round(tip[1]+dy*along+ny*side))
        draw.polygon([point(-length*.3,-half),point(reach,0),point(-length*.3,half)],fill=1)
        draw.line([point(-1,0),point(reach-1,0)],fill=2,width=1)
        draw.point(tuple(tip),fill=3)
    else:
        angle0=math.atan2(old[3]-old[1],old[2]-old[0])
        angle1=math.atan2(tip[1]-w[1],tip[0]-w[0])
        delta=(angle1-angle0+math.pi)%(2*math.pi)-math.pi
        old_length=math.dist(old[:2],old[2:])
        inner=.62 if name=='weak_slash' else .46
        previous=None
        for sample in range(49):
            t=sample/48;angle=angle0+delta*t
            cx=old[0]+(w[0]-old[0])*t;cy=old[1]+(w[1]-old[1])*t
            radius=old_length+(length-old_length)*t
            # Fade the old portion to a narrow tip. Never inflate blade reach.
            taper=.22+.78*t
            inside=radius*(1-(1-inner)*taper);outside=radius+1.0*taper
            point=lambda r:(round(cx+math.cos(angle)*r),round(cy+math.sin(angle)*r))
            pair=(point(inside),point(outside))
            if previous:
                shade=1 if t<.55 else 2
                draw.polygon([previous[0],previous[1],pair[1],pair[0]],fill=shade)
            previous=pair
        # The leading edge ends on the CURRENT blade, not the preceding pose.
        edge=(round(w[0]+(tip[0]-w[0])*.7),round(w[1]+(tip[1]-w[1])*.7))
        draw.line([edge,tuple(tip)],fill=3,width=1)
    fades=(255,210,110) if name!='thrust' else (190,255,150)
    return mask,fades[i-begin]

def bake_trails(metadata):
    paths=json.loads((ROOT/'Art/knight/redesign/blade_paths.json').read_text())
    frames=[(0,0,0)]*192;spans=[]
    fx=Image.new('RGBA',(96*16,128*12))
    palette=[(0,0,0,0),(54,93,112,95),(123,164,173,165),(220,231,219,235)]
    for clip in metadata:
        for i in range(clip['count']):
            mask,alpha=trail_mask(clip['name'],i,clip['source_indices'],paths)
            if not alpha:continue
            start=len(spans)
            for y in range(mask.height):
                x=0
                while x<mask.width:
                    shade=mask.getpixel((x,y));right=x+1
                    while right<mask.width and mask.getpixel((right,y))==shade:right+=1
                    if shade:spans.append((x,y,right-x,shade))
                    x=right
            index=clip['start']+i;frames[index]=(start,len(spans)-start,alpha)
            rgba=Image.new('RGBA',mask.size)
            rgba.putdata([(*palette[v][:3],round(palette[v][3]*alpha/255)) for v in mask.get_flattened_data()])
            fx.paste(rgba,(index%16*96,index//16*128))
    header='#pragma once\nnamespace eclipse {\nstruct SwordTrailSpan{int x,y,width,shade;};\nstruct SwordTrailFrame{int start,count,alpha;};\nconstexpr SwordTrailFrame swordTrailFrames[]={\n'
    header+=''.join('{%d,%d,%d},'%v for v in frames)+'};\nconstexpr SwordTrailSpan swordTrailSpans[]={\n'
    header+=''.join('{%d,%d,%d,%d},'%v for v in spans)+'};\n}\n'
    (ROOT/'native-engine/src/sword_trails.hpp').write_text(header)
    fx.save(ROOT/'native-engine/review/sword_trails.png')


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

