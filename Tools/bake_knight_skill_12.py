#!/usr/bin/env python3
"""Bake the regular knight's 12-frame skill clips from the preserved GitHub original.
No procedurally drawn substitute character; original steel/cape pixels are reused.
No gameplay tests, GitHub Actions, or APK builds are invoked.
"""
from pathlib import Path
import hashlib
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]
MASTER=ROOT/"Art/knight/skill/original_4x6.png"
OUT=ROOT/"Assets/Resources/Eclipse/knight_skill_motion.png"
W,H=112,94
AMPLITUDES=((2,3,-3,-2),(-2,3,-3,2),(2,-2,2,-2))

def warp_upper(image, amount):
    # Foot anchor is fixed; warp the upper torso, cape and hand-held steel
    # together using nearest-neighbor integer row shifts and binary alpha.
    image=image.convert("RGBA")
    output=Image.new("RGBA",(W,H))
    for y in range(H):
        vertical=min(1.,max(0.,((H-1-y)/H-.15)/.5))
        dx=round(amount*vertical)
        output.paste(image.crop((0,y,W,y+1)),(dx,y))
    return output

def bake():
    original=Image.open(MASTER).convert("RGBA")
    if original.size!=(W*4,H*6):
        raise ValueError("ECLIPSE source atlas must be 448x564")
    def cell(row,col):
        return original.crop((col*W,row*H,(col+1)*W,(row+1)*H))
    frames=[]
    for skill in range(3):
        for phase in range(4):
            key=cell(skill,phase)
            midpoint=cell(skill+3,phase)
            if phase<3:
                frames.extend((key,midpoint,warp_upper(midpoint,AMPLITUDES[skill][phase])))
            else:
                # The legacy last midpoint duplicates the key. Author two
                # genuinely different recovery offsets, then settle on key.
                frames.extend((warp_upper(key,-3),warp_upper(key,3),key))
    result=Image.new("RGBA",(W*4,H*9))
    for n,frame in enumerate(frames):
        result.paste(frame,((n%4)*W,(n//4)*H))
    for skill in range(3):
        clip=frames[skill*12:(skill+1)*12]
        if any(not x.getchannel("A").getbbox() for x in clip):
            raise ValueError("Empty frame in skill "+str(skill))
        if len({hashlib.sha256(x.tobytes()).digest() for x in clip})!=12:
            raise ValueError("Repeated pose in skill "+str(skill))
    alpha=set(result.getchannel("A").getdata())
    if not alpha.issubset({0,255}):
        raise ValueError("Unexpected antialiasing or alpha")
    OUT.parent.mkdir(parents=True,exist_ok=True)
    result.save(OUT)
    print("Wrote 448x846, 4 columns, 9 rows, 12 poses per skill:",OUT)

if __name__=="__main__":
    bake()
