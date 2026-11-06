#!/usr/bin/env python3
"""Layered MP4 preview from committed actor PNG and independent effects PNG."""
from pathlib import Path
from PIL import Image, ImageDraw
import argparse, math, shutil, subprocess

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--repo",type=Path,default=Path(__file__).resolve().parents[1])
    ap.add_argument("--out",type=Path,default=Path("knight_skill_mp4"))
    ap.add_argument("--fps",type=int,default=60)
    a=ap.parse_args()
    if not shutil.which("ffmpeg"):raise SystemExit("ffmpeg required")
    src=a.repo/"Assets/Resources/Eclipse"
    actor=Image.open(src/"knight_skill_motion.png").convert("RGBA")
    effects=Image.open(src/"knight_skill_fx_layer.png").convert("RGBA")
    if actor.size!=(448,846) or effects.size!=(448,564):
        raise SystemExit("ECLIPSE ASSET ERROR: actor 448x846, effects 448x564 required")
    a.out.mkdir(parents=True,exist_ok=True)
    for skill,(name,duration) in enumerate((("thrust",.48),("sweep",.415),("prepare",.4))):
        proc=subprocess.Popen(["ffmpeg","-hide_banner","-loglevel","error","-y",
            "-f","rawvideo","-pix_fmt","rgb24","-s","640x360","-r",str(a.fps),
            "-i","-","-c:v","libx264","-pix_fmt","yuv420p","-movflags","+faststart",
            str(a.out/(name+".mp4"))],stdin=subprocess.PIPE)
        for t in range(math.ceil(duration*a.fps)):
            pose=min(11,int(t/a.fps/duration*12))
            global_id=skill*12+pose
            art=actor.crop(((global_id%4)*112,(global_id//4)*94,
                            (global_id%4+1)*112,(global_id//4+1)*94))
            fxid=skill*4+pose//3
            fx=effects.crop(((fxid%4)*112,(fxid//4)*94,
                             (fxid%4+1)*112,(fxid//4+1)*94))
            screen=Image.new("RGBA",(640,360),(15,18,28,255))
            d=ImageDraw.Draw(screen)
            d.rectangle((0,310,640,359),fill=(28,31,40,255))
            x,y=320-44*3,310-88*3
            screen.alpha_composite(fx.resize((336,282),Image.Resampling.NEAREST),(x,y))
            screen.alpha_composite(art.resize((336,282),Image.Resampling.NEAREST),(x,y))
            d.text((18,16),f"ECLIPSE / KNIGHT / {name.upper()} / {pose+1}:12",fill=(216,224,226))
            proc.stdin.write(screen.convert("RGB").tobytes())
        proc.stdin.close()
        if proc.wait()!=0:raise SystemExit("ffmpeg failed: "+name)
        print(a.out/(name+".mp4"))
if __name__=="__main__":main()
