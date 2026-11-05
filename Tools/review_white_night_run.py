#!/usr/bin/env python3
"""Offline PNG/timing review for the alternating run. No game/test-suite launch."""
from pathlib import Path
import json,sys,math,hashlib
import numpy as np
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'native-engine/tools'))
import eclipse_sprite_tool as player
ART=ROOT/'Art/white-night'

def main():
 old=Image.open(ART/'run_upper_reference.png').convert('RGBA')
 full=Image.open(ROOT/'Assets/Resources/Eclipse/ui/white_night_motion.png').convert('RGBA')
 new=full.crop((0,1056,512,1440));guide=json.loads((ART/'run_joint_guide.json').read_text())
 before=[old.crop((i%8*64,i//8*96,i%8*64+64,i//8*96+96)) for i in range(32)]
 after=[new.crop((i%8*64,i//8*96,i%8*64+64,i//8*96+96)) for i in range(32)]
 out=ART/'review';out.mkdir(exist_ok=True)
 marks=[]
 for i,g in enumerate(guide):
  p={}
  for leg in ('right','left'):
   x,y=g[leg]['ankle'];p[leg]=[round(35+(x-35)*.84),round(y)]
  # Crossing/occlusion: never use the projected hidden foot as a measured point.
  if i in (4,5):p['left']=None
  if i in (20,21):p['right']=None
  marks.append(p)
 # 60Hz distance-driven presentation; velocity remains280. The old strip used
 # 280 units/cycle, the new authored contact travel is matched at240 units/cycle.
 pairs=[];marked=[];clean=[]
 for tick in range(180):
  t=tick/60;bi=int((t*280/280)%1*32);ai=int((t*280/240)%1*32)
  pair=Image.new('RGBA',(768,440),'#151b26');single=Image.new('RGBA',(384,440),'#151b26')
  for f,x in [(before[bi],64),(after[ai],448)]:pair.alpha_composite(f.resize((256,384),Image.Resampling.NEAREST),(x,32))
  single.alpha_composite(after[ai].resize((256,384),Image.Resampling.NEAREST),(64,32));clean.append(single.copy())
  for leg,col in [('right','#ffe044'),('left','#36e78a')]:
   xy=marks[ai][leg]
   if xy:
    x,y=64+xy[0]*4,32+xy[1]*4;ImageDraw.Draw(single).ellipse((x-4,y-4,x+4,y+4),fill=col,outline='black')
  for canvas in (pair,single):
   d=ImageDraw.Draw(canvas)
   for x in range(-128,canvas.width+256,128):
    xx=int(x-(t*280*4/1.85)%128);d.line((xx,385,xx+50,385),fill='#9aa5b5',width=2)
  ImageDraw.Draw(pair).text((12,8),'BEFORE: same leading leg / 32FPS',fill='white');ImageDraw.Draw(pair).text((396,8),'AFTER: alternating bright-R / dark-L',fill='white')
  ImageDraw.Draw(single).text((8,8),f'pose {88+ai} / bright RIGHT, dark LEFT',fill='white');ImageDraw.Draw(single).text((8,415),'Yellow RIGHT / Green LEFT; hidden markers omitted',fill='white')
  pairs.append(pair);marked.append(single)
 player.write_mp4(pairs,out/'white_run_before_after.mp4',1,60,1,(21,27,38))
 player.write_mp4(marked,out/'white_run_marked.mp4',1,60,1,(21,27,38))
 player.write_mp4(clean,out/'white_run_clean.mp4',1,60,1,(21,27,38))
 player.write_contact(after,out/'white_run_32_poses.png',3,8,(21,27,38))
 comparison=Image.new('RGBA',(1024,384),'#151b26')
 for j,i in enumerate((0,8,16,24)):
  f=after[i].resize((256,384),Image.Resampling.NEAREST);comparison.alpha_composite(f,(j*256,0));d=ImageDraw.Draw(comparison)
  for leg,col in [('right','#ffe044'),('left','#36e78a')]:
   xy=marks[i][leg]
   if xy:d.ellipse((j*256+xy[0]*4-4,xy[1]*4-4,j*256+xy[0]*4+4,xy[1]*4+4),fill=col,outline='black')
  d.text((j*256+6,8),f'Pose {88+i}',fill='white')
 comparison.save(out/'white_run_alternating_keys.png')
 samples=[]
 for i in (0,16):
  means={}
  for leg,xy in marks[i].items():
   x,y=xy;patch=np.array(after[i])[max(0,y-2):y+3,max(0,x-2):x+3];rgb=patch[:,:,:3][patch[:,:,3]>=128];means[leg]=float(np.mean(rgb@np.array([.2126,.7152,.0722])))
  samples.append({'frame':88+i,'markers':marks[i],'ankle_patch_luminance':means,'right_is_forward':marks[i]['right'][0]>marks[i]['left'][0]})
 report={'source_asset_sha256':hashlib.sha256((ROOT/'Assets/Resources/Eclipse/ui/white_night_motion.png').read_bytes()).hexdigest(),'frames':32,'cycle_world_units':240,'base_speed':280,'cycle_seconds':240/280,'base_pose_fps':32*280/240,'identity_checks':samples,'marker_basis':'Guide-projected ankle estimates, visually checked against all32 authored poses; roughly +/-2 source pixels. Occluded crossings omitted. These are not automatic ground-truth joint measurements.','markers':marks,'loop_pixel_difference':player.metrics([after[-1],after[0]])[-1]['pixel_change_fraction_est'],'full_game_runtime':'NOT RUN','real_time_video_viewing':'NOT CLAIMED'}
 (ART/'run_review.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:report[k] for k in ('frames','base_pose_fps','identity_checks','loop_pixel_difference')}))

if __name__=='__main__':main()
