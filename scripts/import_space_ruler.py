"""Import approved Download sheets byte-for-byte; generate SDL atlas metadata."""
from pathlib import Path
import json,hashlib,shutil,math
from PIL import Image
root=Path(__file__).resolve().parents[1]
src=Path('/storage/emulated/0/Download/space ruler')
art=root/'Art/space-ruler';runtime=root/'Assets/Resources/Eclipse/space_ruler'
art.mkdir(parents=True,exist_ok=True);runtime.mkdir(parents=True,exist_ok=True)
actors=['idle','walk','flight_rise','flight_hold','flight_descend','hurt','death','attack','skill1','skill2','skill3','ultimate']
durations=[.9,.72,.72,.9,.72,.36,.9,.55,.65,.75,.6,2.0]
fx=['void-bolt','gravity-field','dimensional-spikes','repulsion-wave','singularity-collapse','flight-trail']
records=[];arrays=[]
for kind,names in [('actor',actors),('fx',fx)]:
 rows=[]
 for i,name in enumerate(names):
  source=(art/'walk.png' if name=='walk' else src/(name+'.png')) if kind=='actor' else src/'pixelfx'/(name+'.png')
  image=Image.open(source);w,h=image.size
  if kind=='actor':cw=ch=192;cols=w//cw;n=cols;px,py=96,132;duration=durations[i];loop=name in ('idle','walk','flight_hold');scale=1.6
  else:
   data=json.loads((source.with_suffix('.json')).read_text());m=data['meta']['pixelfx'];cw,ch=m['frameSize'];cols=m['columns'];n=m['frames'];px,py=m['pivot'];duration=2.0 if i==4 else n/m['fps'];loop=m['loop'];scale=[1.0,2.6,1.8,1.75,2.4,2.2][i]
   shutil.copy2(source.with_suffix('.json'),runtime/(name+'.json'))
  dest=runtime/(name+'.png')
  if source.resolve()!=dest.resolve():shutil.copy2(source,dest)
  art_copy=art/(name+'.png')
  if source.resolve()!=art_copy.resolve():shutil.copy2(source,art_copy)
  record=dict(name=name,kind=kind,source_path=str((art/(name+'.png')).relative_to(root)),runtime_path=str(dest.relative_to(root)),sha256=hashlib.sha256(source.read_bytes()).hexdigest(),width=w,height=h,cell=[cw,ch],columns=cols,frames=n,pivot=[px,py],duration=duration,loop=loop,scale=scale,status='integrated-owner-device-review-pending')
  if name=='walk':
   source_frames_dir=art/'walk-source/pixellab-skeleton-east'
   source_frames=[]
   for j in range(8):
    frame_path=source_frames_dir/f'frame_{j:03}.png'
    frame_image=Image.open(frame_path).convert('RGBA')
    source_frames.append({'path':str(frame_path.relative_to(root)),'sha256':hashlib.sha256(frame_path.read_bytes()).hexdigest(),'size':list(frame_image.size)})
   reference_path=source_frames_dir/'reference_96x96.png'
   record.update(source_animation='PixelLab animate_with_skeleton_v3',source_direction='east',source_job_id='79a752b1-834f-4452-8438-767fac4c5661',source_asset_id='52c806f5-c9cf-558a-b21b-5a867cc25d7e',source_frame_size=[96,96],source_frames=source_frames,source_input={'path':str(reference_path.relative_to(root)),'sha256':hashlib.sha256(reference_path.read_bytes()).hexdigest(),'colors':len(set(Image.open(reference_path).convert('RGBA').getdata()))},colors_in_source_frames=len({px for frame in source_frames for px in Image.open(root/frame['path']).convert('RGBA').getdata()}),atlas_registration={'source_pixels_per_cell_pixel':1,'source_offset':[48,48],'ground_line_y':133,'notes':'Preserves the original 96x96 pixel scale centered inside each 192x192 runtime cell; no nearest-neighbor enlargement.'},superseded_source={'reason':'The previous walk atlas used the wrong PixelLab character template and omitted the original mage/book. Replaced using the exact original sprite as reference.'})
  records.append(record);rows.append('    {"space_ruler/%s.png",%d,%d,%d,%d,%d,%d,%.6ff,%.6ff,%s},'%(name,cw,ch,cols,n,px,py,scale,duration,str(loop).lower()))
 arrays.append('inline constexpr SpaceSheet space%s[]={\n%s\n};'%('Actors' if kind=='actor' else 'Effects','\n'.join(rows)))
(art/'manifest.json').write_text(json.dumps(records,indent=2)+'\n')
(root/'native-engine/src/space_ruler_assets.hpp').write_text('#pragma once\nnamespace eclipse {\nstruct SpaceSheet {const char* file;int width,height,columns,count,pivotX,pivotY;float scale,duration;bool loop;};\n'+'\n'.join(arrays)+'\n}\n')
shutil.copy2(src.parent/'ECLIPSE_공간의지배자_스킬_모션_설계.md',art/'design.md')
print('Imported',len(records),'approved sheets')
