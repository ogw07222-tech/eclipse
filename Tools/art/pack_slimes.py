#!/usr/bin/env python3
"""Pack authored 4x4 animation sheets into engine-ready 128px cells.
No procedural monster drawing: all poses come from the authored source PNG.
"""
import argparse,json
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw

def main():
 p=argparse.ArgumentParser();p.add_argument('sources',type=Path);p.add_argument('--output',type=Path,default=Path('Assets/Resources/Eclipse/monsters'));p.add_argument('--review',type=Path,default=Path('Art/Monsters/review'));a=p.parse_args()
 a.output.mkdir(parents=True,exist_ok=True);a.review.mkdir(parents=True,exist_ok=True)
 source_names={'green':'green.png','red':'red.png','purple':'purple.png'}
 allframes={}
 for kind,name in source_names.items():
  src=Image.open(a.sources/name).convert('RGBA');frames=[]
  for n in range(16):
   x,y=n%4,n//4
   cell=src.crop((round(x*src.width/4),round(y*src.height/4),round((x+1)*src.width/4),round((y+1)*src.height/4)))
   # Binary alpha discards the generated soft exterior halo. Keep cavities opaque.
   alpha=np.array(cell.getchannel('A'));alpha=np.where(alpha>=220,255,0).astype('uint8');cell.putalpha(Image.fromarray(alpha))
   box=cell.getbbox()
   if not box:raise ValueError(f'Empty pose {kind} {n}')
   frames.append(cell.crop(box))
  # One scale per species preserves anticipation squash/stretch across all poses.
  factor=min(108/max(f.width for f in frames),100/max(f.height for f in frames))
  sheet=Image.new('RGBA',(512,512))
  for n,f in enumerate(frames):
   f=f.resize((round(f.width*factor),round(f.height*factor)),Image.Resampling.NEAREST)
   sheet.alpha_composite(f,(n%4*128+64-f.width//2,n//4*128+116-f.height))
  alpha=sheet.getchannel('A')
  # Shared palette across every pose prevents per-frame color flicker.
  sheet=sheet.convert('RGB').quantize(colors=20,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE).convert('RGBA');sheet.putalpha(alpha)
  sheet.save(a.output/f'slime_{kind}.png',optimize=True)
  allframes[kind]=[sheet.crop((n%4*128,n//4*128,n%4*128+128,n//4*128+128)) for n in range(16)]
  for clip,start in [('move',0),('attack',8)]:
   sequence=[]
   for f in allframes[kind][start:start+8]:
    bg=Image.new('RGBA',(128,128),'#18202a');bg.alpha_composite(f);sequence.append(bg.convert('RGB').resize((384,384),Image.Resampling.NEAREST))
   sequence[0].save(a.review/f'{kind}_{clip}.gif',save_all=True,append_images=sequence[1:],duration=100,loop=0)
 meta={'cell':[128,128],'columns':4,'rows':4,'pivot':[64,116],'facing':'left','clips':{'move':{'start':0,'count':8,'loop':True,'stride':64},'attack':{'start':8,'count':8,'loop':False,'contactFrame':4}},'strength':['green','red','purple']}
 (a.output/'slimes.json').write_text(json.dumps(meta,indent=2)+'\n')
 # Common timeline uses actual per-species attack durations and distance-based move cadence.
 frames=[]
 for tick in range(144):
  canvas=Image.new('RGB',(900,440),'#10151e');d=ImageDraw.Draw(canvas)
  for col,(kind,speed,duration) in enumerate([('green',95,.72),('red',63,.88),('purple',49,1.04)]):
   t=tick/30;attack=t>=2.4;index=min(7,int(((t-2.4)%1.6)/duration*8))+8 if attack else int((t*speed/64)%1*8)
   f=allframes[kind][index].resize((256,256),Image.Resampling.NEAREST);canvas.paste(f,(col*300+22,70),f)
   d.text((col*300+24,24),kind.upper(),fill='white');d.text((col*300+24,355),'ATTACK' if attack else 'MOVE',fill='#bac7d6')
  frames.append(canvas)
 frames[0].save(a.review/'slimes_motion.gif',save_all=True,append_images=frames[1:],duration=33,loop=0)
 contact=Image.new('RGBA',(1024,384),'#18202a')
 for row,kind in enumerate(source_names):
  for n in range(8):contact.alpha_composite(allframes[kind][n+8],(n*128,row*128))
 contact.save(a.review/'attack_contact_sheet.png')
 print('Packed 3 atlases, 48 poses, binary alpha, 20-color shared palettes, 7 animated reviews')
if __name__=='__main__':main()
