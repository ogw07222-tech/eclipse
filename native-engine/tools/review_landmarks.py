#!/usr/bin/env python3
"""Diagnostic marker overlay/trajectory plots; never modifies source PNGs.

Manual landmarks have uncertainty. Feet A/B are local track IDs, NOT anatomical
left/right labels. Null means unknown; never bridge gaps. No automatic quality
score or pose interpolation. Requires Pillow/matplotlib; MP4 needs FFmpeg.
"""
import csv
import hashlib
import json
import math
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from PIL import Image, ImageDraw
import eclipse_sprite_tool as player

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'native-engine/animation-audit/landmarks'
DATA=json.loads((OUT/'landmarks.json').read_text())
COLORS=DATA['colors']
TRACKS={t['name']:t for t in DATA['tracks']}

def source(t,i):
    w,h=t['cell'];im=Image.open(ROOT/'Assets/Resources/Eclipse'/t['asset']).convert('RGBA')
    return im.crop((i%t['columns']*w,i//t['columns']*h,(i%t['columns']+1)*w,(i//t['columns']+1)*h))

def points(t,i):return t['points'][t['indices'].index(i)]

def main():
    hashes={t['asset']:hashlib.sha256((ROOT/'Assets/Resources/Eclipse'/t['asset']).read_bytes()).hexdigest() for t in DATA['tracks']}
    white=TRACKS['white'];speed,stride,scale=280.,280.,1.85
    rows=list(csv.DictReader((ROOT/'native-engine/review/changed/trace.csv').open()))
    first_tick={i:next(int(r['tick']) for r in rows if r['clip']=='white' and int(r['source'])==i) for i in white['indices']}
    contact=[]
    for indices in ((89,90,91),(112,113,114)):
        toe=[]
        for i in indices:
            im=source(white,i);xs=[x for x in range(im.width) if im.getpixel((x,88))[3]>=128]
            if not xs:raise ValueError(f'No visible contact pixels at row88: {i}')
            toe.append(max(xs))
        times=[(first_tick[i]-first_tick[indices[0]])/60 for i in indices]
        world=[speed*t+(x-toe[0])*scale for t,x in zip(times,toe)]
        contact.append(dict(indices=indices,toe_x=toe,times=times,relative_world_x=world,
                            net_world_drift=world[-1],assumption='Rightmost opaque outline pixel at ground row88 approximates toe contact. Boot deformation may move this edge. Geometry diagnostic, not a foot-ground physics measurement.'))
    fig=plt.figure(figsize=(12,8),layout='constrained');grid=fig.add_gridspec(2,4)
    for col,i in enumerate((89,91,112,114)):
        ax=fig.add_subplot(grid[0,col]);ax.set_facecolor('#18202b');ax.imshow(source(white,i),interpolation='nearest')
        for key,xy in points(white,i).items():
            if xy:ax.scatter(*xy,c=COLORS[key],s=44,edgecolors='black',linewidths=.7)
        ax.axhline(88,color='#d5d5d5',lw=.7);ax.set_title(f'White run / atlas {i}');ax.set_xlim(0,64);ax.set_ylim(94,25);ax.set_xticks([0,32,64]);ax.set_yticks([32,64,88])
    ax=fig.add_subplot(grid[1,:2])
    for group,color in zip(contact,('#ce633a','#408bc0')):
        ax.plot([t*1000 for t in group['times']],group['relative_world_x'],'o-',c=color,label=f"{group['indices'][0]} -> {group['indices'][-1]}: {group['net_world_drift']:+.2f}px")
    ax.axhline(0,c='gray',ls='--',label='Stationary toe reference');ax.set_xlabel('Elapsed time (ms)');ax.set_ylabel('World toe-edge drift (px)');ax.set_title('Ground-row toe outline: opposite drift directions');ax.legend(fontsize=8);ax.grid(alpha=.2)
    ax=fig.add_subplot(grid[1,2:])
    for key in ('foot_a','foot_b','weapon_wrist'):
        xs=[float('nan') if p[key] is None else p[key][0] for p in white['points']]
        ax.plot([first_tick[i]/60 for i in white['indices']],xs,'o-',ms=3,color=COLORS[key],markeredgecolor='black',lw=1,label=key)
    ax.set_facecolor('#27303d');ax.set_xlabel('60Hz sample time (s)');ax.set_ylabel('Local landmark X (source px)');ax.set_title('Manual local tracks; gaps are unresolved');ax.legend(fontsize=8);ax.grid(alpha=.2)
    fig.suptitle('ECLIPSE motion review / diagnostic markers only\nFoot A/B are provisional IDs; hidden landmarks omitted. Sources unchanged.',fontsize=13)
    fig.savefig(OUT/'white_landmark_review.png',dpi=125);plt.close(fig)
    counts={}
    for name,t in TRACKS.items():
        samples=[r for r in rows if r['clip']==name];frames=[]
        for tick,r in enumerate(samples):
            i=int(r['source']);im=source(t,i);frame=Image.new('RGBA',(384,440),(24,28,36,255));frame.alpha_composite(im.resize((im.width*4,im.height*4),Image.Resampling.NEAREST),(64,32));draw=ImageDraw.Draw(frame)
            if name=='white':
                # Camera follows root; ground moves left at the actual base speed.
                shift=speed*tick/60*4/scale
                for x in range(-128,640,128):
                    gx=int(x-shift%128);draw.line((gx,385,gx+54,385),fill='#b2b6be',width=2)
            for key,xy in points(t,i).items():
                if xy:
                    x,y=64+xy[0]*4,32+xy[1]*4
                    draw.ellipse((x-5,y-5,x+5,y+5),fill=COLORS[key],outline='black',width=1)
            draw.text((8,5),f'{name}  atlas={i}  t={tick/60:.3f}s',fill='white')
            draw.text((8,402),'RED other wrist / BLUE weapon wrist',fill='white')
            draw.text((8,419),'GREEN foot A / YELLOW foot B (side unresolved)',fill='white')
            frames.append(frame)
        if player.shutil.which('ffmpeg'):player.write_mp4(frames,OUT/f'{name}_marked.mp4',1,60,3,(24,28,36))
        counts[name]={key:sum(p[key] is not None for p in t['points']) for key in COLORS}
    result=dict(scope='Manual landmark/ground-row outline diagnostic, not an animation quality pass',source_sha256=hashes,source_uncertainty_px=2,speed=speed,stride=stride,motion_scale=scale,contact_segments=contact,visible_marker_counts=counts,limits=DATA['sides'])
    (OUT/'measurements.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
    for path,sha in hashes.items():
        if hashlib.sha256((ROOT/'Assets/Resources/Eclipse'/path).read_bytes()).hexdigest()!=sha:raise RuntimeError('Source changed: '+path)
    print(json.dumps({'contact_segments':contact,'visible_marker_counts':counts},ensure_ascii=False))

if __name__=='__main__':main()
