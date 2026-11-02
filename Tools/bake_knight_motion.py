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
CLIPS=[('idle',0,8,.96),('walk',16,8,.60),('run',32,16,.428571),('jump',48,6,.52),('fall',64,6,.65),('land',80,6,.12),('dash',96,8,.15),('weak_slash',112,8,.23),('strong_slash',128,10,.25),('thrust',144,10,.42),('hurt',160,6,.23),('death',176,12,.90)]
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

def foot(phase,stride,lift,contact=.5):
    phase%=1
    if phase<contact:return (24+stride*(.5-phase/contact),43)
    t=(phase-contact)/(1-contact)
    return (24+lerp(-stride/2,stride/2,smooth(t)),43-lift*math.sin(math.pi*t))

def pose(name,t):
    hip=[24.,30.];lean=5.;twist=0.;wind=0.;sword=12.;near=(30.,27.);far=(20.,28.)
    feet=[(21.,43.),(27.,43.)]
    if name=='idle':hip[1]+=.25*math.sin(t*math.tau);wind=.5*math.sin(t*math.tau)
    elif name in ('run','walk'):
        run=name=='run';stride=16 if run else 12;contact=.4 if run else .5
        feet=[foot(t+.5,stride,6 if run else 2,contact),foot(t,stride,6 if run else 2,contact)]
        hip[1]=(31.7 if run else 30.8)+.35*math.cos(t*math.tau*2)
        if run:hip[0]+=.6+1.5*math.sin(t*math.tau*2)
        lean=(28 if run else 9)+2*math.sin(t*math.tau*2);twist=.65*math.sin(t*math.tau);wind=6 if run else 1
        near=(29+3*math.sin(t*math.tau),28.5-2*math.cos(t*math.tau));far=(20-3*math.sin(t*math.tau),27+3*math.cos(t*math.tau));sword=-8+18*math.sin(t*math.tau)
    elif name in ('jump','fall'):
        u=t if name=='jump' else 1-t
        feet=[(21-3*u,43-9*u),(27+4*u,43-8*u)];lean=lerp(12,26,u);wind=3+u*3;near=(32,26-u*2);far=(19,28-u*2);sword=-30
    elif name=='land':
        squash=math.sin(math.pi*t);hip[1]+=4*squash;lean=5+28*squash;wind=3*(1-t);near=(30,27+2*squash)
        feet=[(21-2*squash,43),(27+3*squash,43)];sword=12+12*squash
    elif name=='dash':
        drive=sample([(0,.75),(.16,1),(.72,1),(1,.3)],t)
        hip=[24-drive,30+5*drive];lean=78*drive;wind=11*drive
        feet=[(lerp(21,13,drive),42),(lerp(27,32,drive),43)];near=(37,34);far=(20,33);sword=-5
    elif name in ('weak_slash','strong_slash','thrust'):
        if name=='weak_slash':
            weight=sample([(0,0),(.2,1),(.65,1),(1,0)],t)
            hip[0]+=sample([(0,0),(.18,-1),(.43,3),(.68,2),(1,0)],t)
            hip[1]+=sample([(0,0),(.18,2),(.43,4),(.68,3),(1,0)],t)
            lean=sample([(0,5),(.18,-25),(.43,42),(.68,27),(1,5)],t)
            twist=sample([(0,0),(.18,-1),(.43,1.1),(.68,.6),(1,0)],t)
            near=(sample([(0,30),(.18,25),(.43,43),(.68,37),(1,30)],t),sample([(0,27),(.18,22),(.43,30),(.68,36),(1,27)],t))
            sword=sample([(0,12),(.18,-145),(.43,55),(.68,35),(1,12)],t)
            feet=[(lerp(21,17,weight),43),(lerp(27,32,weight),43)]
        elif name=='strong_slash':
            weight=sample([(0,0),(.22,1),(.8,1),(1,0)],t)
            hip[0]+=sample([(0,0),(.30,-1),(.61,4),(.8,3),(1,0)],t)
            hip[1]+=sample([(0,0),(.30,3),(.61,5),(.8,4),(1,0)],t)
            lean=sample([(0,5),(.30,-28),(.42,-26),(.61,55),(.8,35),(1,5)],t)
            twist=sample([(0,0),(.30,-1.1),(.61,1.2),(.8,.8),(1,0)],t)
            near=(sample([(0,30),(.30,23),(.42,23),(.61,44),(.8,39),(1,30)],t),sample([(0,27),(.30,17),(.42,17),(.61,34),(.8,36),(1,27)],t))
            sword=sample([(0,12),(.30,-145),(.42,-150),(.61,70),(.8,50),(1,12)],t)
            feet=[(lerp(21,16,weight),43),(lerp(27,33,weight),43)]
        else:
            drive=sample([(0,0),(.24,0),(.52,1),(.72,1),(1,0)],t)
            hip[0]+=sample([(0,0),(.24,-1),(.52,3),(.72,3),(1,0)],t)
            hip[1]+=5*drive;lean=5+41*drive;twist=.7*drive
            near=(sample([(0,30),(.24,25),(.52,44.5),(.72,44.5),(1,30)],t),sample([(0,27),(.24,31),(.52,30),(.72,30),(1,27)],t));sword=0
            feet=[(lerp(21,15,drive),43),(lerp(27,35,drive),43)]
        wind=3+3*max(0,twist);far=(hip[0]-4,hip[1]-5)
    elif name=='hurt':
        recoil=math.sin(math.pi*t);hip[0]-=3*recoil;lean=5-28*recoil;near=(30-3*recoil,27);sword=12-45*recoil
    elif name=='death':
        d=smooth(t);hip=[24-3*d,30+11*d];lean=90*d;feet=[(lerp(21,10,d),43),(lerp(27,17,d),43)]
        near=(lerp(30,37,d),lerp(27,42,d));far=(lerp(20,32,d),lerp(28,42,d));wind=1-d;sword=12*(1-d)
    angle=math.radians(lean);chest=(hip[0]+math.sin(angle)*8+twist*.4,hip[1]-math.cos(angle)*8)
    head=(chest[0]+math.sin(angle)*3,chest[1]-math.cos(angle)*4)
    shoulder=(chest[0]+1+twist*1.3,chest[1]+1-abs(twist)*.4)
    dx,dy=near[0]-shoulder[0],near[1]-shoulder[1];d=math.hypot(dx,dy)
    if d>9.8:near=(shoulder[0]+dx/d*9.8,shoulder[1]+dy/d*9.8)
    far_shoulder=(chest[0]-2,chest[1]+1)
    dx,dy=far[0]-far_shoulder[0],far[1]-far_shoulder[1];d=math.hypot(dx,dy)
    if d>9.8:far=(far_shoulder[0]+dx/d*9.8,far_shoulder[1]+dy/d*9.8)
    return dict(hip=tuple(hip),chest=chest,head=head,shoulder=shoulder,near=near,far=far,feet=feet,lean=lean,twist=twist,wind=wind,sword=sword,head_roll=lean*(.85 if name=='death' else .3),phase=t)

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
        hip=(h[0]+(-1 if index==0 else 1),h[1]);ankle=p['feet'][index];knee=ik(hip,ankle,7,7)
        return joint(hip)+segment('forearm',hip,knee,3)+joint(knee)+segment('shin',knee,ankle,3)+image('boot',ankle[0]-2,ankle[1]-1,5,2)
    parts.append(leg(0));fs=(c[0]-2,c[1]+1);elbow=ik(fs,p['far'],5,5,-1)
    parts.append(segment('upper_arm',fs,elbow,2.8)+segment('forearm',elbow,p['far'],2.8))
    parts.append(segment('torso',c,h,6.5+p['twist']*1.2));parts.append(f'<line x1="{c[0]}" y1="{c[1]}" x2="{hd[0]}" y2="{hd[1]}" stroke="#23272b" stroke-width="3"/>');parts.append(image('helmet',hd[0]-3,hd[1]-3,6,6,p['head_roll']))
    parts.append(leg(1));elbow=ik(p['shoulder'],p['near'],5,5,-1)
    parts.append(joint(p['shoulder'])+segment('upper_arm',p['shoulder'],elbow,3)+joint(elbow)+segment('forearm',elbow,p['near'],3))
    x,y=p['near'];parts.append(f'<g transform="translate({x:.4f} {y:.4f}) rotate({p["sword"]:.4f})">'+image('sword',-2,-2,11,4)+'</g>')
    parts.append(f'<rect x="{x-1:.3f}" y="{y-1:.3f}" width="2" height="2" fill="#737b80"/>')
    return '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="64" height="48" viewBox="0 0 64 48">'+''.join(parts).replace('href=','xlink:href=')+'</svg>'

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
                run(temp/'raster.png','-filter','point','-resize','64x48!','-channel','A','-threshold','50%','+channel',temp/'opaque.png')
                run(temp/'opaque.png','-alpha','extract',temp/'alpha.png')
                run(temp/'opaque.png','+dither','-remap',temp/'palette.png',temp/'alpha.png','-alpha','off','-compose','CopyOpacity','-composite','-compose','Over',png)
                frames[start+i]=png
            metadata.append(dict(name=name,start=start,count=count,duration=duration,loop=loop,poses=poses))
        run('-size','64x48','xc:none',temp/'blank.png')
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
    header='#pragma once\nnamespace eclipse {\nstruct MotionClip{int start,count;float duration,contact,stride;};\nconstexpr MotionClip motionClips[]={\n'
    header+=''.join('    {%d,%d,%.6ff,%.6ff,%.6ff},\n'%(*mapping[n][1:],{'weak_slash':.43,'strong_slash':.61,'thrust':.52}.get(n,.5),{'run':120,'walk':72}.get(n,0)) for n in states)
    header+='};\nconstexpr int motionWidth=64,motionHeight=48,motionColumns=16,motionRows=12,motionPivotX=24,motionPivotY=44;\n}\n'
    (ROOT/'native-engine/src/motion.hpp').write_text(header)
    inputs=[Path(__file__).resolve(),*sorted(ART.glob('*.png'))]
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    stamp={'inputs':{str(p.relative_to(ROOT)):digest(p) for p in inputs},'outputs':{str(p.relative_to(ROOT)):digest(p) for p in [OUT,ROOT/'native-engine/src/motion.hpp']}}
    (ART/'build.json').write_text(json.dumps(stamp,indent=2)+'\n')
    print('Baked 104 motion frames, 12 clips, 1024x576 atlas.')

if __name__=='__main__':bake()
