#!/usr/bin/env python3
"""Offline pixel-rig baker. Uses fixed PNG parts cut from the approved coarse knight.
Requires ImageMagick 6/7 and CairoSVG (offline development tooling only). No image/network generation occurs during game builds.
"""
from pathlib import Path
import base64
import json
import hashlib
import math
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'Art/knight/rig'
OUT=ROOT/'Assets/Resources/Eclipse/knight_motion.png'
CLIPS=[('idle',0,8,.96),('walk',16,8,.60),('run',32,16,.343),('jump',48,6,.52),('fall',64,6,.65),('land',80,6,.12),('dash',96,8,.15),('weak_slash',112,8,.23),('strong_slash',128,10,.25),('thrust',144,10,.42),('hurt',160,6,.23),('death',176,12,.90)]
PALETTE=['#101318','#23272b','#444a4e','#737b80','#a8afb0','#d0d2c8','#111b30','#20314a','#32465c','#49301d','#75512c','#a17b40']
def lerp(a,b,t):return a+(b-a)*t
def smooth(t):return t*t*(3-2*t)
def sample(keys,t):
    for i in range(len(keys)-1):
        a,va=keys[i];b,vb=keys[i+1]
        if t<=b:return lerp(va,vb,smooth(max(0,(t-a)/(b-a))))
    return keys[-1][1]
def ik(a,b,l1,l2,bend=1):
    dx,dy=b[0]-a[0],b[1]-a[1];d=max(.001,min(math.hypot(dx,dy),l1+l2-.001));raw=max(.001,math.hypot(dx,dy));ux,uy=dx/raw,dy/raw
    along=(l1*l1-l2*l2+d*d)/(2*d);h=math.sqrt(max(0,l1*l1-along*along))
    return a[0]+ux*along+bend*uy*h,a[1]+uy*along-bend*ux*h

def foot(phase,stride,lift):
    phase%=1
    if phase<.5:return (24+stride*(.5-2*phase),43)
    t=(phase-.5)*2
    return (24+lerp(-stride/2,stride/2,smooth(t)),43-lift*math.sin(math.pi*t))

def pose(name,t):
    hip=[24.,32.];lean=0.;wind=0.;sword=18.;near=(29.,30.);far=(20.,29.)
    feet=[(21.,43.),(27.,43.)]
    if name=='idle':hip[1]+=.25*math.sin(t*math.tau);wind=.5*math.sin(t*math.tau)
    elif name in ('run','walk'):
        run=name=='run';stride=16 if run else 12
        feet=[foot(t+.5,stride,4 if run else 2),foot(t,stride,4 if run else 2)]
        hip[1]+=(2.7 if run else 1)+.35*math.cos(t*math.tau*2);lean=18 if run else 7;wind=4 if run else 1
        near=(29+1.5*math.sin(t*math.tau),30+1.5*math.cos(t*math.tau));far=(20-2*math.sin(t*math.tau),29-2*math.cos(t*math.tau));sword=8+10*math.sin(t*math.tau)
    elif name in ('jump','fall'):
        u=t if name=='jump' else 1-t
        feet=[(21-2*u,43-6*u),(28+2*u,43-5*u)];lean=lerp(5,12,u);wind=2+u*2;near=(30,28-u*2);far=(20,29-u);sword=-25
    elif name=='land':
        crouch=math.sin(math.pi*t)*2.5;hip[1]+=crouch;lean=15*math.sin(math.pi*t);wind=2*(1-t);near=(30,31);sword=20
    elif name=='dash':
        drive=sample([(0,.3),(.16,1),(.72,1),(1,.25)],t);hip[1]+=3*drive;lean=55*drive;wind=9*drive
        feet=[(17,42),(29,43)];near=(34,32);far=(22,32);sword=0
    elif name in ('weak_slash','strong_slash','thrust'):
        if name=='weak_slash':
            lean=sample([(0,-8),(.2,-12),(.50,28),(.75,15),(1,0)],t)
            near=(sample([(0,28),(.2,27),(.5,34),(.75,32),(1,29)],t),sample([(0,29),(.2,22),(.5,29),(.75,32),(1,30)],t))
            sword=sample([(0,18),(.2,-80),(.55,55),(.75,30),(1,18)],t)
        elif name=='strong_slash':
            lean=sample([(0,-10),(.3,-18),(.60,35),(.8,24),(1,0)],t);hip[1]+=sample([(0,0),(.3,-1),(.65,3),(1,0)],t)
            near=(sample([(0,27),(.3,27),(.65,34),(1,29)],t),sample([(0,23),(.3,18),(.65,32),(1,30)],t))
            sword=sample([(0,-90),(.3,-135),(.65,50),(1,18)],t)
        else:
            drive=sample([(0,0),(.25,0),(.53,1),(.72,1),(1,0)],t);hip[0]+=2*drive;lean=30*drive
            near=(sample([(0,27),(.25,26),(.53,37),(.72,37),(1,29)],t),sample([(0,31),(.25,30),(.53,28),(.72,28),(1,30)],t));sword=0
        feet=[(19,43),(30,43)];wind=3;far=(21,29)
    elif name=='hurt':
        recoil=math.sin(math.pi*t);hip[0]-=2*recoil;lean=-20*recoil;near=(29-2*recoil,28);sword=-35*recoil
    elif name=='death':
        d=smooth(t);hip=[24-2*d,32+9*d];lean=90*d;feet=[(lerp(21,11,d),43),(lerp(27,17,d),43)]
        near=(lerp(29,36,d),lerp(30,42,d));far=(lerp(20,32,d),lerp(29,42,d));wind=1-d;sword=18*(1-d)
    angle=math.radians(lean);chest=(hip[0]+math.sin(angle)*8,hip[1]-math.cos(angle)*8)
    head=(chest[0]+math.sin(angle)*3,chest[1]-math.cos(angle)*4)
    shoulder=(chest[0]+1,chest[1]+1)
    # Reach targets are clamped to the fixed arm length, never stretched.
    dx,dy=near[0]-shoulder[0],near[1]-shoulder[1];d=math.hypot(dx,dy)
    if d>9.8:near=(shoulder[0]+dx/d*9.8,shoulder[1]+dy/d*9.8)
    return dict(hip=tuple(hip),chest=chest,head=head,shoulder=shoulder,near=near,far=far,feet=feet,lean=lean,wind=wind,sword=sword,phase=t)

PARTS={}
def image(name,x,y,w,h,rotation=0):
    href=PARTS[name];cx=x+w/2;cy=y+h/2
    return f'<image href="{href}" x="{x:.4f}" y="{y:.4f}" width="{w}" height="{h}" transform="rotate({rotation:.3f} {cx:.4f} {cy:.4f})" image-rendering="optimizeSpeed"/>'
def segment(name,a,b,width):
    dx,dy=b[0]-a[0],b[1]-a[1];length=math.hypot(dx,dy);angle=math.degrees(math.atan2(dy,dx))-90
    return f'<line x1="{a[0]:.4f}" y1="{a[1]:.4f}" x2="{b[0]:.4f}" y2="{b[1]:.4f}" stroke="#23272b" stroke-width="{width}"/>'+f'<g transform="translate({a[0]:.4f} {a[1]:.4f}) rotate({angle:.4f})">'+image(name,-width/2,-.6,width,length+1.2)+'</g>'
def joint(p):return f'<rect x="{p[0]-1:.3f}" y="{p[1]-1:.3f}" width="2" height="2" fill="#23272b"/>'
def svg_frame(p):
    h=p['hip'];c=p['chest'];hd=p['head'];parts=[]
    tailx=c[0]-8-p['wind'];taily=min(43,h[1]+3+math.sin(p['phase']*math.tau-1)*.7)
    points=[(c[0]-2,c[1]),(c[0]-5,c[1]+2),(tailx-3,taily-2),(tailx,taily-3),(tailx+1,taily),(tailx+4,taily-2),(h[0]-2,h[1]+1)]
    parts.append('<polygon points="'+' '.join(f'{x:.2f},{y:.2f}' for x,y in points)+'" fill="#111b30" stroke="#101318" stroke-width="1"/>')
    parts.append(f'<path d="M{c[0]-4:.2f},{c[1]+2:.2f} L{tailx:.2f},{taily-4:.2f} L{tailx+4:.2f},{taily-3:.2f} L{c[0]-2:.2f},{c[1]+1:.2f} Z" fill="#20314a"/>')
    def leg(index):
        hip=(h[0]+(-1 if index==0 else 1),h[1]);ankle=p['feet'][index];knee=ik(hip,ankle,6,6)
        return joint(hip)+segment('forearm',hip,knee,3)+joint(knee)+segment('shin',knee,ankle,3)+image('boot',ankle[0]-2,ankle[1]-1,5,2)
    parts.append(leg(0));fs=(c[0]-2,c[1]+1);elbow=ik(fs,p['far'],5,5,-1)
    parts.append(segment('upper_arm',fs,elbow,2.8)+segment('forearm',elbow,p['far'],2.8))
    parts.append(segment('torso',c,h,6.5));parts.append(f'<line x1="{c[0]}" y1="{c[1]}" x2="{hd[0]}" y2="{hd[1]}" stroke="#23272b" stroke-width="3"/>');parts.append(image('helmet',hd[0]-3,hd[1]-3,6,6,p['lean']*.3))
    parts.append(leg(1));elbow=ik(p['shoulder'],p['near'],5,5,-1)
    parts.append(joint(p['shoulder'])+segment('upper_arm',p['shoulder'],elbow,3)+joint(elbow)+segment('forearm',elbow,p['near'],3))
    x,y=p['near'];parts.append(f'<g transform="translate({x:.4f} {y:.4f}) rotate({p["sword"]:.4f})">'+image('sword',-2,-1.5,11,3)+'</g>')
    parts.append(f'<rect x="{x-1:.3f}" y="{y-1:.3f}" width="2" height="2" fill="#737b80"/>')
    return '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="48" height="48" viewBox="0 0 48 48">'+''.join(parts).replace('href=','xlink:href=')+'</svg>'

def run(*args):subprocess.run(['convert',*map(str,args)],check=True,stdout=subprocess.DEVNULL)
def bake():
    import cairosvg
    for name in ('helmet','torso','upper_arm','forearm','shin','boot','sword'):
        PARTS[name]='data:image/png;base64,'+base64.b64encode((ART/(name+'.png')).read_bytes()).decode()
    review=ROOT/'native-engine/review';review.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='eclipse-rig-') as directory:
        temp=Path(directory)
        run(*sum((['-size','1x1','xc:'+c] for c in PALETTE),[]),'+append',temp/'palette.png')
        frames={};metadata=[]
        for name,start,count,duration in CLIPS:
            loop=name in ('idle','walk','run');poses=[]
            for i in range(count):
                t=i/(count if loop else count-1);p=pose(name,t);poses.append(p)
                svg=temp/'frame.svg';svg.write_text(svg_frame(p));png=temp/f'{start+i}.png'
                cairosvg.svg2png(bytestring=svg.read_bytes(),write_to=str(temp/'raster.png'))
                run(temp/'raster.png','-filter','point','-resize','48x48!','-channel','A','-threshold','50%','+channel',temp/'opaque.png')
                run(temp/'opaque.png','-alpha','extract',temp/'alpha.png')
                run(temp/'opaque.png','+dither','-remap',temp/'palette.png',temp/'alpha.png','-alpha','off','-compose','CopyOpacity','-composite','-compose','Over',png)
                frames[start+i]=png
            metadata.append(dict(name=name,start=start,count=count,duration=duration,loop=loop,poses=poses))
        run('-size','48x48','xc:none',temp/'blank.png')
        for row in range(12):run(*[frames.get(row*16+c,temp/'blank.png') for c in range(16)],'+append',temp/f'row-{row}.png')
        run(*[temp/f'row-{row}.png' for row in range(12)],'-append',OUT)
        for clip in metadata:
            indices=range(clip['start'],clip['start']+clip['count'])
            run(*[frames[i] for i in indices],'+append',review/(clip['name']+'.png'))
            run('-delay',str(round(clip['duration']/clip['count']*100)),'-dispose','background',*[frames[i] for i in indices],'-loop','0',review/(clip['name']+'.gif'))
        (review/'motion.json').write_text(json.dumps(metadata,indent=2)+'\n')
    (ART/'clips.json').write_text(json.dumps([{k:v for k,v in c.items() if k!='poses'} for c in metadata],indent=2)+'\n')
    states=['idle','run','jump','fall','weak_slash','strong_slash','thrust','dash','hurt','death','land','walk']
    mapping={c[0]:c for c in CLIPS}
    header='#pragma once\nnamespace eclipse {\nstruct MotionClip{int start,count;float duration;};\nconstexpr MotionClip motionClips[]={\n'
    header+=''.join('    {%d,%d,%.6ff},\n'%mapping[n][1:] for n in states)
    header+='};\nconstexpr int motionCell=48,motionColumns=16,motionRows=12;\n}\n'
    (ROOT/'native-engine/src/motion.hpp').write_text(header)
    inputs=[Path(__file__).resolve(),*sorted(ART.glob('*.png'))]
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    stamp={'inputs':{str(p.relative_to(ROOT)):digest(p) for p in inputs},'outputs':{str(p.relative_to(ROOT)):digest(p) for p in [OUT,ROOT/'native-engine/src/motion.hpp']}}
    (ART/'build.json').write_text(json.dumps(stamp,indent=2)+'\n')
    print('Baked 104 motion frames, 12 clips, 768x576 atlas.')

if __name__=='__main__':bake()
