#!/usr/bin/env python3
"""Version-stamped offline sprite review; no game build or owner tests."""
from pathlib import Path
import hashlib,json,sys,math
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'native-engine/tools'))
import eclipse_sprite_tool as player
from knight_sprint import sprint_leg
ART=ROOT/'Art/knight/sprint'

def main():
 old=Image.open(ART/'before_displayed_24.png').convert('RGBA')
 full=Image.open(ROOT/'Assets/Resources/Eclipse/knight_motion.png').convert('RGBA')
 new=Image.open(ART/'authored_run_24.png').convert('RGBA')
 digest=hashlib.sha256((ROOT/'Assets/Resources/Eclipse/knight_motion.png').read_bytes()).hexdigest()[:8]
 media_tag=hashlib.sha256((ROOT/'Assets/Resources/Eclipse/knight_motion.png').read_bytes()+Path(__file__).read_bytes()).hexdigest()[:8]
 out=ART/'review';out.mkdir(exist_ok=True)
 pairs=[];marked=[];clean=[];seen=set()
 for tick in range(126):
  i=(tick*24//42)%24;seen.add(i);t=tick/60
  rect=(i%8*64,i//8*94,i%8*64+64,i//8*94+94)
  before=old.crop(rect);after=new.crop(rect)
  actual=full.crop(((432+i)%16*64,(432+i)//16*94,((432+i)%16+1)*64,((432+i)//16+1)*94))
  if actual.tobytes()!=after.tobytes():raise ValueError('Runtime atlas/preview mapping mismatch')
  panel=Image.new('RGBA',(768,432),'#151b26');single=Image.new('RGBA',(384,432),'#151b26')
  for f,x in [(before,64),(after,448)]:panel.alpha_composite(f.resize((256,376),Image.Resampling.NEAREST),(x,28))
  single.alpha_composite(after.resize((256,376),Image.Resampling.NEAREST),(64,28));clean.append(single.copy())
  for leg,phase,color in [('right',i/24,'#ffe044'),('left',i/24+.5,'#36e78a')]:
   joints=sprint_leg(phase);x,y=joints['ankle'];x,y=64+x*4,28+y*4
   ImageDraw.Draw(single).ellipse((x-4,y-4,x+4,y+4),fill=color,outline='black')
  for canvas in (panel,single):
   d=ImageDraw.Draw(canvas)
   for x in range(-128,canvas.width+128,128):
    gx=int(x-(280*t*4/1.85)%128);d.line((gx,380,gx+48,380),fill='#93a4b5',width=2)
  ImageDraw.Draw(panel).text((8,6),'BEFORE /24 poses in0.7s',fill='white');ImageDraw.Draw(panel).text((392,6),'AFTER /24 newly baked poses in0.7s',fill='white')
  ImageDraw.Draw(single).text((8,6),f'KNIGHT /{digest} /pose {i}',fill='white');ImageDraw.Draw(single).text((8,412),'Yellow RIGHT / Green LEFT /native joint coordinates',fill='white')
  pairs.append(panel);marked.append(single)
 player.write_mp4(pairs,out/f'knight_run_{media_tag}_compare.mp4',1,60,1,(21,27,38))
 player.write_mp4(marked,out/f'knight_run_{media_tag}_marked.mp4',1,60,1,(21,27,38))
 player.write_mp4(clean,out/f'knight_run_{media_tag}_clean.mp4',1,60,1,(21,27,38))
 zoom=[]
 for tick,panel in enumerate(pairs[:84]):
  frame=Image.new('RGBA',(1040,336),'#151b26')
  for n,x in enumerate((64,448)):
   region=panel.crop((x,276,x+256,404)).resize((512,256),Image.Resampling.NEAREST)
   frame.alpha_composite(region,(n*520,38))
  d=ImageDraw.Draw(frame);d.text((8,8),'BEFORE /lower body',fill='white');d.text((528,8),'AFTER /lower body',fill='white');d.text((8,310),f'0.25x /pose {(tick*24//42)%24} /same timing',fill='white');zoom.append(frame)
 player.write_mp4(zoom,out/f'knight_run_{media_tag}_quarter_speed.mp4',1,15,1,(21,27,38))
 canvas=Image.new('RGBA',(1024,752),'#151b26')
 for c,i in enumerate((0,6,12,18)):
  box=(i%8*64,i//8*94,i%8*64+64,i//8*94+94)
  for r,im in enumerate((old,new)):
   canvas.alpha_composite(im.crop(box).resize((256,376),Image.Resampling.NEAREST),(c*256,r*376));ImageDraw.Draw(canvas).text((c*256+6,r*376+6),('BEFORE' if r==0 else 'AFTER')+f' {i}',fill='white')
 canvas.save(out/f'knight_run_{media_tag}_key_poses.png')
 lengths=[]
 for i in range(24):
  for leg in ('right','left'):
   p=sprint_leg(i/24+(0 if leg=='right' else .5));lengths.extend([math.dist(p['hip'],p['knee']),math.dist(p['knee'],p['ankle'])])
 old_min=1e9
 for i in range(24):
  for offset in (0,.5):
   phase=((i*54//24)/54+offset)%1;travel=120/1.85;stance=.48
   if phase<stance:x,y=48-phase*travel,84.
   else:
    t=(phase-stance)/(1-stance)
    x=48-travel*stance+travel*stance*(3*t*t-2*t*t*t)-travel*(1-stance)*t*(1-t)*(1-2*t)
    y=84-13*math.sin(math.pi*t)**2
   old_min=min(old_min,math.hypot(x-33,y-66))
 new_min=min(math.dist((33,66),sprint_leg(i/24+off)['ankle']) for i in range(24) for off in (0,.5))
 report=dict(asset_digest=digest,media_tag=media_tag,old_min_hip_ankle_distance=old_min,new_min_hip_ankle_distance=new_min,sampled_poses=sorted(seen),duration=.7,frames=24,pose_fps=24/.7,
     source_joint_length_min=min(lengths),source_joint_length_max=max(lengths),old_world_stride=120,runtime_world_stride=196,
     new_world_stride=196,old_contact_sliding_world_units_per_full_stance=(196-120)*.48,
     new_ideal_contact_sliding_world_units=0,limits='Native render-joint geometry; rasterization/occlusion can affect visible endpoints. No SDL/device execution or real-time video viewing claimed.')
 (ART/'review.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps(report))

if __name__=='__main__':main()
