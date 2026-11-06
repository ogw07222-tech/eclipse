#!/usr/bin/env python3
"""Offline, source-PNG layered preview. No gameplay code or GitHub Actions."""
from pathlib import Path
from PIL import Image, ImageDraw
import argparse, math, re, shutil, subprocess

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--repo",type=Path,default=Path(__file__).resolve().parents[1])
    p.add_argument("--out",type=Path,default=Path("knight_skill_mp4"))
    p.add_argument("--fps",type=int,default=60)
    a=p.parse_args()
    if not shutil.which("ffmpeg"): raise SystemExit("Install ffmpeg before rendering")
    source=a.repo/"Assets/Resources/Eclipse/knight_skill_motion.png"
    blade=a.repo/"native-engine/src/knight_skill_blades.hpp"
    if not source.exists(): raise SystemExit("Missing source PNG: "+str(source))
    sprite=Image.open(source).convert("RGBA")
    if sprite.size!=(448,564): raise SystemExit("Unexpected atlas dimensions: "+str(sprite.size))
    data=blade.read_text().split("skillBlades[]=",1)[1].split("};",1)[0]
    marks=[tuple(map(int,x)) for x in re.findall(r"\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\}",data)]
    if len(marks)!=12: raise SystemExit("Missing sword annotations")
    a.out.mkdir(parents=True,exist_ok=True)
    for name,start,duration in (("thrust",0,.48),("sweep",4,.415),("prepare",8,.40)):
        out=a.out/(name+".mp4")
        proc=subprocess.Popen(["ffmpeg","-hide_banner","-loglevel","error","-y","-f","rawvideo","-pix_fmt","rgb24","-s","720x480","-r",str(a.fps),"-i","-","-c:v","libx264","-pix_fmt","yuv420p","-movflags","+faststart",str(out)],stdin=subprocess.PIPE)
        for tick in range(math.ceil(duration*a.fps)):
            k=min(3,int((tick/a.fps)/duration*4)); ix=start+k
            actor=sprite.crop(((ix%4)*112,(ix//4)*94,(ix%4+1)*112,(ix//4+1)*94))
            if not actor.getchannel("A").getbbox():raise SystemExit("Empty source cell "+str(ix))
            frame=Image.new("RGBA",(720,480),(13,17,28,255))
            draw=ImageDraw.Draw(frame)
            draw.rectangle((0,353,720,480),fill=(27,32,42,255))
            draw.line((0,353,720,353),fill=(70,79,92,255),width=3)
            fx=Image.new("RGBA",(224,144))
            d=ImageDraw.Draw(fx)
            hx,hy,tx,ty=marks[ix];hx+=56;hy+=20;tx+=56;ty+=20
            dx=tx-hx;dy=ty-hy;rad=max(1,math.hypot(dx,dy))
            if k in (1,2):
                if name=="thrust":
                    d.polygon([(hx+dx*.4,hy+dy*.4),(tx-dy*.35,ty+dx*.35),(hx+dx*1.9,hy+dy*1.9),(tx+dy*.35,ty-dx*.35)],fill=(255,204,122,185))
                    d.line((tx,ty,hx+dx*1.8,hy+dy*1.8),fill=(255,248,218,255),width=2)
                elif name=="sweep":
                    angle=math.atan2(dy,dx)
                    arc=[(round(hx+math.cos(angle-.9+i/24*1.7)*rad*1.35),round(hy+math.sin(angle-.9+i/24*1.7)*rad*1.35)) for i in range(25)]
                    d.line(arc,fill=(255,204,122,195),width=6)
                    d.line(arc[12:],fill=(255,248,218,255),width=2)
                else:
                    d.line((hx,hy,tx,ty),fill=(255,204,122,190),width=3)
                    for i in range(12):
                        angle=i*math.pi/6
                        px=round(tx+math.cos(angle)*(12+k*4));py=round(ty+math.sin(angle)*(12+k*4))
                        d.rectangle((px,py,px+2,py+2),fill=(255,248,218,220))
            left=360-44*3;top=353-88*3
            frame.alpha_composite(fx.resize((672,432),Image.Resampling.NEAREST),(left-168,top-60))
            frame.alpha_composite(actor.resize((336,282),Image.Resampling.NEAREST),(left,top))
            draw=ImageDraw.Draw(frame)
            draw.text((20,18),"ECLIPSE / KNIGHT / "+name.upper(),fill=(240,231,208))
            draw.text((20,42),"SOURCE PNG + SEPARATE EFFECT / POSE "+str(k+1)+"/4",fill=(171,182,192))
            proc.stdin.write(frame.convert("RGB").tobytes())
        proc.stdin.close()
        if proc.wait():raise SystemExit("ffmpeg failed for "+name)
        print(out)
if __name__=="__main__":main()
