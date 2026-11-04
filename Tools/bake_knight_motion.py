#!/usr/bin/env python3
"""Pack approved PNG poses into the existing 104-frame runtime layout."""
from pathlib import Path
import hashlib,json
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
MASTER=ROOT/'Art/knight/redesign/knight_64x94_master.png'
OUT=ROOT/'Assets/Resources/Eclipse/knight_motion.png'
CLIPS=[('idle',0,8,.96,8),('walk',16,8,.60,8),('run',32,16,.428571,16),('jump',48,6,.52,6),('fall',64,6,.65,6),('land',80,6,.12,6),('dash',96,8,.15,8),('weak_slash',112,8,.23,8),('strong_slash',128,10,.34,10),('thrust',144,10,.42,10),('hurt',160,6,.23,6),('death',176,12,.90,12),('air_slam',192,8,.38,8)]
TRAIL_WINDOWS={'weak_slash':(3,5),'strong_slash':(5,7),'thrust':(4,6),'air_slam':(2,6)}

# Give the effect room on every side of the actor cell, including above the helm.
FX_ORIGIN = (48, 40)
FX_SIZE = (176, 176)
FX_PALETTE = [(0, 0, 0, 0), (37, 83, 145, 110), (65, 153, 218, 205),
              (245, 255, 255, 255), (151, 230, 255, 235)]
FX_REACH = {'weak_slash': 1.72, 'strong_slash': 2.12,
            'thrust': 2.05, 'air_slam': 1.84}


def trail_mask(name, i, indices, paths):
    """Build an energy sweep around the steel path in padded cell coordinates."""
    import math
    mask = Image.new('L', FX_SIZE)
    window = TRAIL_WINDOWS.get(name)
    if not window or not window[0] <= i <= window[1]:
        return mask, 0

    draw = ImageDraw.Draw(mask)
    wx, wy, tx, ty = paths[name][indices[i]]
    ox, oy, px, py = paths[name][indices[i - 1]]
    length = math.hypot(tx - wx, ty - wy)
    reach = FX_REACH[name]
    def pixel(x, y):
        return round(x + FX_ORIGIN[0]), round(y + FX_ORIGIN[1])

    if name == 'thrust':
        dx, dy = (tx - wx) / length, (ty - wy) / length
        def point(along, side):
            return pixel(wx + length * (dx * along - dy * side),
                         wy + length * (dy * along + dx * side))

        # A tapered lance and two split wakes peel away from the forward thrust.
        for side in (-1, 1):
            draw.polygon([point(.32, side * .16), point(.65, side * .45),
                          point(1.28, side * .31), point(reach, 0),
                          point(1.23, side * .15)], fill=1)
            draw.line([point(.66, side * .43), point(1.05, side * .35),
                       point(1.61, side * .14)], fill=2, width=1)
            for along, drift in ((.62, .52), (1.12, .41), (1.67, .26)):
                draw.line([point(along, side * drift),
                           point(along + .15, side * drift * .82)], fill=4, width=1)
        draw.polygon([point(.32, 0), point(.95, -.22), point(reach, 0),
                      point(.95, .22)], fill=2)
        draw.polygon([point(.74, 0), point(1.14, -.08), point(reach - .04, 0),
                      point(1.14, .08)], fill=4)
        draw.line([point(.78, 0), point(reach - .1, 0)], fill=3, width=1)
    else:
        angle0 = math.atan2(py - oy, px - ox)
        angle1 = math.atan2(ty - wy, tx - wx)
        delta = (angle1 - angle0 + math.pi) % (2 * math.pi) - math.pi
        if name == 'strong_slash' and i > window[0]:
            # The sword has nearly stopped by contact; the overhead arc is still fading.
            delta = math.radians(72 if i == window[0] + 1 else 38)
            angle0 = angle1 - delta
        old_length = math.hypot(px - ox, py - oy)

        def sweep(t, radius, tangent=0):
            angle = angle0 + delta * t
            scale = old_length + (length - old_length) * t
            return pixel(ox + (wx - ox) * t + scale * radius * math.cos(angle) - tangent * math.sin(angle),
                         oy + (wy - oy) * t + scale * radius * math.sin(angle) + tangent * math.cos(angle))

        def outer(t):
            return 1 + (reach - 1) * (.30 + .70 * math.sin(t * math.pi / 2))

        def ribbon(start, stop, inner, edge, shade):
            times = [start + (stop - start) * k / 32 for k in range(33)]
            draw.polygon([sweep(t, edge(t)) for t in times] +
                         [sweep(t, inner(t)) for t in reversed(times)], fill=shade)

        # Tapered crescent, separated echo, icy rim and short white crest.
        # A translucent inner fan follows the whole blade behind the bright edge.
        ribbon(.06, 1, lambda t: .35 + .16 * (1 - t),
               lambda t: outer(t) - .25, 1)
        ribbon(0, 1, lambda t: outer(t) - .60 * t, outer, 1)
        ribbon(.11, 1, lambda t: outer(t) - .37 * t, outer, 2)
        ribbon(.28, 1, lambda t: outer(t) - .12 * t, outer, 4)
        draw.line([sweep(t / 32, outer(t / 32) - .02) for t in range(17, 33)],
                  fill=3, width=1)
        draw.line([sweep(t / 32, outer(t / 32) + .13) for t in range(5, 25)],
                  fill=2, width=1)
        # Connect the crescent to the current steel tip and throw off sparks.
        draw.line([sweep(1, .38), sweep(1, reach)], fill=4, width=2)
        for k, t in enumerate((.10, .23, .36, .53, .69, .84, .94)):
            radius = outer(t) + .20 + .08 * (k % 3)
            point = sweep(t, radius, 2 + k % 3)
            draw.line([point, sweep(t, radius + .11, 3 + k % 3)], fill=2, width=1)
            draw.point(point, fill=3 if k % 2 == 0 else 4)

    draw.line([pixel(tx - 1, ty), pixel(tx + 1, ty)], fill=3, width=1)
    fades = {'weak_slash': (255, 205, 100), 'strong_slash': (255, 220, 115),
             'thrust': (220, 255, 125), 'air_slam': (150, 215, 255, 255, 105)}
    return mask, fades[name][i - window[0]]

def bake_trails(metadata):
    paths=json.loads((ROOT/'Art/knight/redesign/blade_paths.json').read_text())
    frames=[(0,0,0)]*208;spans=[]
    width,height=FX_SIZE
    fx=Image.new('RGBA',(width*16,height*13))
    palette=FX_PALETTE
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
                    if shade:spans.append((x-FX_ORIGIN[0],y-FX_ORIGIN[1],right-x,shade))
                    x=right
            index=clip['start']+i;frames[index]=(start,len(spans)-start,alpha)
            rgba=Image.new('RGBA',mask.size)
            rgba.putdata([(*palette[v][:3],round(palette[v][3]*alpha/255)) for v in mask.get_flattened_data()])
            fx.paste(rgba,(index%16*width,index//16*height))
    header='#pragma once\nnamespace eclipse {\nstruct SwordTrailSpan{int x,y,width,shade;};\nstruct SwordTrailFrame{int start,count,alpha;};\nconstexpr SwordTrailFrame swordTrailFrames[]={\n'
    header+=''.join('{%d,%d,%d},'%v for v in frames)+'};\nconstexpr SwordTrailSpan swordTrailSpans[]={\n'
    header+='\n'.join('    '+''.join('{%d,%d,%d,%d},'%v for v in spans[i:i+8])
                      for i in range(0,len(spans),8))+'};\n'
    header+='constexpr unsigned swordTrailColors[]={'+','.join(
        f'0x{r:02x}{g:02x}{b:02x}' for r,g,b,a in palette)+'};\n'
    header+='constexpr int swordTrailAlpha[]={'+','.join(str(a) for r,g,b,a in palette)+'};\n}\n'
    (ROOT/'native-engine/src/sword_trails.hpp').write_text(header)
    fx.save(ROOT/'native-engine/review/sword_trails.png')


def bake():
    source=Image.open(MASTER)
    if source.mode!='P':raise ValueError('Master must use the approved indexed pixel palette')
    if source.size!=(1024,1222):raise ValueError('Master must be 16x13 cells of 64x94')
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
    print('Packed 112 playable frames from 112 authored PNG poses; 64x94 cells; 1024x1222 atlas.')
if __name__=='__main__':bake()

