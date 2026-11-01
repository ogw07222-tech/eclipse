#!/usr/bin/env python3
"""Minimal repair of the CURRENT 48x64 knight atlas, preserving all source RGB pixels."""
from PIL import Image
from collections import deque, Counter
from pathlib import Path
import shutil

path=Path("assets/knight_sheet_48x64_v2.png")
im=Image.open(path).convert("RGBA")
assert im.size==(384,448), f"Unexpected atlas size: {im.size}"
backup=Path("assets/knight_sheet_48x64_v2_original.png")
if not backup.exists(): shutil.copy2(path,backup)
W,H=48,64
pix=im.load()
removed=0
filled=0
# Conservative slash removal: only disconnected visual debris in frames whose
# artwork contains a slash (rows 2-4); never paint new knight shapes.
for row in (2,3,4):
 for col in range(8):
  x0,y0=col*W,row*H
  # Keep the largest connected alpha component in each frame (the knight).
  # Slash/particle fragments that are separate from the figure are removed.
  visited=set()
  components=[]
  for y in range(H):
   for x in range(W):
    if (x,y) in visited or pix[x0+x,y0+y][3]==0: continue
    q=deque([(x,y)]); visited.add((x,y)); comp=[]
    while q:
     cx,cy=q.popleft();comp.append((cx,cy))
     for nx,ny in ((cx+1,cy),(cx-1,cy),(cx,cy+1),(cx,cy-1)):
      if 0<=nx<W and 0<=ny<H and (nx,ny) not in visited and pix[x0+nx,y0+ny][3]>0:
       visited.add((nx,ny));q.append((nx,ny))
    components.append(comp)
  if not components:continue
  largest=max(components,key=len)
  for comp in components:
   if comp is largest or len(comp)<4: continue
   # Avoid changing small armor details. Only remove detached long, pale
   # slash fragments outside the torso area.
   xs=[x for x,y in comp]; ys=[y for x,y in comp]
   if len(comp)<10 or (max(xs)-min(xs)<9 and max(ys)-min(ys)<9): continue
   pale=sum(1 for x,y in comp if min(pix[x0+x,y0+y][:3])>=125)
   outside=sum(1 for x,y in comp if x<6 or x>36)
   if pale*2<len(comp) or outside*2<len(comp): continue
   for x,y in comp:
    r,g,b,a=pix[x0+x,y0+y];pix[x0+x,y0+y]=(r,g,b,0);removed+=1

# Only fix small *completely enclosed* accidental transparent holes.
# Never change the external transparent silhouette or the RGB of visible pixels.
for row in range(7):
 for col in range(8):
  x0,y0=col*W,row*H;visited=set()
  for y in range(1,H-1):
   for x in range(1,W-1):
    if (x,y) in visited or pix[x0+x,y0+y][3]!=0: continue
    q=deque([(x,y)]);visited.add((x,y));comp=[];touches_edge=False
    while q:
     cx,cy=q.popleft();comp.append((cx,cy))
     if cx==0 or cy==0 or cx==W-1 or cy==H-1:touches_edge=True
     for nx,ny in ((cx+1,cy),(cx-1,cy),(cx,cy+1),(cx,cy-1)):
      if 0<=nx<W and 0<=ny<H and (nx,ny) not in visited and pix[x0+nx,y0+ny][3]==0:
       visited.add((nx,ny));q.append((nx,ny))
    if touches_edge or not (1<=len(comp)<=7):continue
    neighbors=[]
    for cx,cy in comp:
     for nx,ny in ((cx+1,cy),(cx-1,cy),(cx,cy+1),(cx,cy-1)):
      if 0<=nx<W and 0<=ny<H:
       p=pix[x0+nx,y0+ny]
       if p[3]>0: neighbors.append(p)
    if len(neighbors)<4:continue
    color=Counter(neighbors).most_common(1)[0][0]
    for cx,cy in comp:pix[x0+cx,y0+cy]=color;filled+=1
assert removed<700 and filled<600, f"Safety limit exceeded: removed={removed} filled={filled}"
im.save(path,optimize=True)
print(f"Conservative alpha repair: removed {removed} detached slash pixels; filled {filled} enclosed hole pixels")
