#!/usr/bin/env python3
"""Author transparent knight skill effect PNG from the game's exact 12 sword annotations.

This is an additive visual layer. It does not edit the actor atlas, gameplay state,
blade collisions, cooldowns, damage, or Github Actions.
"""
from pathlib import Path
import math, re
from PIL import Image, ImageDraw

ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/"native-engine/src/knight_skill_blades.hpp"
OUTPUT=ROOT/"Assets/Resources/Eclipse/knight_skill_fx_layer.png"
CW,CH=112,94

def main():
    data=SRC.read_text().split("skillBlades[]=",1)[1].split("};",1)[0]
    marks=[tuple(map(int,x)) for x in re.findall(r"\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\}",data)]
    if len(marks)!=12: raise ValueError("Expected exactly 12 original authored skill blade paths")
    sheet=Image.new("RGBA",(CW*4,CH*6))
    for i,(hx,hy,tx,ty) in enumerate(marks):
        row,k=divmod(i,4)
        cell=Image.new("RGBA",(CW,CH))
        d=ImageDraw.Draw(cell)
        dx,dy=tx-hx,ty-hy
        distance=max(1,math.hypot(dx,dy))
        nx,ny=-dy/distance,dx/distance
        def point(t,side=0):
            return (round(hx+dx*t+nx*side),round(hy+dy*t+ny*side))
        if k in (1,2):
            if row==0:
                far=min(1.3,1+(110-tx)/max(1,distance))
                d.polygon([point(.28,-3),point(.9,-6),point(far),point(.9,6),point(.28,3)],fill=(238,179,85,125))
                d.line([point(.45),point(far)],fill=(255,247,210,240),width=2)
                for j in range(3):
                    d.line([point(.5+j*.18,(-1)**j*7),point(.68+j*.16,(-1)**j*8)],fill=(255,197,99,175))
            elif row==1:
                angle=math.atan2(dy,dx)
                arc=[(round(hx+math.cos(angle-.75+j/20*1.5)*distance*.98),round(hy+math.sin(angle-.75+j/20*1.5)*distance*.98)) for j in range(21)]
                d.line(arc,fill=(240,182,102,150),width=5)
                d.line(arc[7:],fill=(255,241,195,235),width=2)
            else:
                d.line([point(0),point(1)],fill=(255,225,151,190),width=2)
                for j in range(9):
                    theta=j*math.tau/9
                    radius=9+3*k
                    x=round(tx+math.cos(theta)*radius)
                    y=round(ty+math.sin(theta)*radius)
                    if 0<=x<CW-2 and 0<=y<CH-2:
                        d.rectangle((x,y,x+1,y+1),fill=(255,244,207,195))
        sheet.alpha_composite(cell,(k*CW,row*CH))
    OUTPUT.parent.mkdir(parents=True,exist_ok=True)
    sheet.save(OUTPUT,optimize=True)
    print("Wrote",OUTPUT,"448x564 RGBA; 12 source-aligned effect cells")
if __name__=="__main__":
    main()
