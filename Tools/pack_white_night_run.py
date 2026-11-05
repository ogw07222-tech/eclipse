#!/usr/bin/env python3
"""Pack image-authored alternating legs; preserve original torso/weapon pixels.
Only run88..119 lower-leg pixels are eligible to change. No pose duplication,
procedural limb drawing, interpolation, or gameplay edits occur here.
"""
from pathlib import Path
import hashlib,json
from collections import deque
from PIL import Image
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
ASSET=ROOT/'Assets/Resources/Eclipse/ui/white_night_motion.png'
ART=ROOT/'Art/white-night'

def leg_mask(reference_cell):
 m=np.zeros((96,64),dtype=bool);m[66:]=True
 # Preserve the complete original right-hand sword, including its blade below
 # the hip line. Coordinates are in the original immutable reference cells.
 for y in range(66,74):
  for x in range(40,64):
   if abs(y-(63+(x-35)*.30))<=2 and reference_cell[y,x,3]>0:m[y,x]=False
 return m

def main():
 atlas=Image.open(ASSET).convert('RGBA');reference=Image.open(ART/'run_upper_reference.png').convert('RGBA')
 authored=Image.open(ART/'run_alternating_source.png').convert('RGBA')
 if atlas.size!=(512,1440) or reference.size!=(512,384):raise ValueError('Unexpected native atlas/reference dimensions')
 authored=authored.resize(reference.size,Image.Resampling.NEAREST)
 auth=np.array(authored);auth[:,:,3]=np.where(auth[:,:,3]>=128,255,0)
 auth[auth[:,:,3]==0]=0
 before=np.array(atlas).copy();out=np.array(atlas).copy();ref=np.array(reference);counts=[]
 for i in range(32):
  x,y=i%8*64,i//8*96;old=ref[y:y+96,x:x+64].copy();mask=leg_mask(old)
  # Recover toes spilling across the generated grid boundary, without shrinking
  # any limb. Use the native grid at1:1 below; no per-pose recentering,
  # bounding-box sizing, directional scaling or limb-path interpolation.
  roi=np.zeros((96,80,4),dtype=np.uint8)
  lo,hi=max(0,x-8),min(512,x+72)
  roi[:,lo-(x-8):hi-(x-8)]=auth[y:y+96,lo:hi]
  roi[:66]=0
  for yy in range(66,74):
   for xx in range(40,72):
    if abs(yy-(63+(xx-35)*.30))<=3:roi[yy,xx+8]=0
  seeds=[(xx,yy) for yy in range(66,94) for xx in range(16,72) if roi[yy,xx,3]>0]
  todo=deque(seeds);seen=set(seeds)
  while todo:
   xx,yy=todo.popleft()
   for dx in (-1,0,1):
    for dy in (-1,0,1):
     q=(xx+dx,yy+dy)
     if 0<=q[0]<80 and 66<=q[1]<96 and q not in seen and roi[q[1],q[0],3]>0:seen.add(q);todo.append(q)
  isolated=np.zeros_like(roi)
  for xx,yy in seen:isolated[yy,xx]=roi[yy,xx]
  new=np.zeros((96,64,4),dtype=np.uint8)
  for xx in range(64):
   sx=xx+8
   if 0<=sx<80:new[:,xx]=isolated[:,sx]
  if new[74:,0,3].any() or new[74:,-1,3].any() or new[-1,:,3].any():raise ValueError(('Leg clipping',i))
  old[mask]=new[mask]
  out[1056+y:1056+y+96,x:x+64]=old
  counts.append(int(np.any(old!=ref[y:y+96,x:x+64],axis=2).sum()))
 if np.any(before[:1056]!=out[:1056]):raise ValueError('Non-run frames changed')
 for i in range(32):
  x,y=i%8*64,i//8*96
  mask=leg_mask(ref[y:y+96,x:x+64])
  if np.any(out[1056+y:1056+y+96,x:x+64][~mask]!=ref[y:y+96,x:x+64][~mask]):raise ValueError('Torso or sword changed')
 frames=[out[1056+i//8*96:1056+(i//8+1)*96,i%8*64:(i%8+1)*64] for i in range(32)]
 unique=len({hashlib.sha256(f.tobytes()).hexdigest() for f in frames})
 leg_hashes=[]
 for i,f in enumerate(frames):
  x,y=i%8*64,i//8*96;m=leg_mask(ref[y:y+96,x:x+64]);layer=np.zeros_like(f);layer[m]=f[m];leg_hashes.append(hashlib.sha256(layer.tobytes()).hexdigest())
 leg_unique=len(set(leg_hashes))
 if unique!=32 or leg_unique!=32 or not all(counts):raise ValueError('Missing/duplicate run leg poses')
 Image.fromarray(out).save(ASSET,optimize=True)
 (ART/'run_pack.json').write_text(json.dumps({'asset':str(ASSET.relative_to(ROOT)),'cell':[64,96],'start':88,'count':32,'stride_world':240,'source':'run_alternating_source.png','reference':'run_upper_reference.png','source_sha256':hashlib.sha256((ART/'run_alternating_source.png').read_bytes()).hexdigest(),'asset_sha256':hashlib.sha256(ASSET.read_bytes()).hexdigest(),'distinct_frames':unique,'distinct_leg_layers':leg_unique,'leg_horizontal_scale':1.0,'leg_scale_origin_x':35,'changed_pixels_by_pose':counts,'unchanged':'frames0..87 and all reference run pixels outside lower-leg mask; original torso,hair,cape,hand,sword retained','visual_acceptance':'not device/runtime verified'},indent=2)+'\n')
 print('Packed32 distinct poses; original non-run frames and pixels outside the leg mask unchanged.')

if __name__=='__main__':main()
