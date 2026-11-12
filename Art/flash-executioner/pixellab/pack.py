from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json,hashlib,math
root=Path(__file__).resolve().parent
SPECS={
'skill3':dict(role='Skill3',source_count=25,source_cols=5,map=[i for i in range(25) if i not in [2,13,14]],duration=.36,contactFrame=11),
'dash':dict(role='Dash',source_count=9,source_cols=3,map=list(range(9)),duration=.15,contactFrame=-1),
'swap':dict(role='Swap',source_count=17,source_cols=5,map=[round(i*16/13) for i in range(14)],duration=.24,contactFrame=-1),
'jump':dict(role='Jump',source_count=17,source_cols=5,map=list(range(17)),duration=0,contactFrame=-1),
'fall':dict(role='Fall',source_count=17,source_cols=5,map=list(range(1,17)),duration=0,contactFrame=-1),
'land':dict(role='Land',source_count=9,source_cols=3,map=[1,2,3,4,5,6,8],duration=.12,contactFrame=-1),
'skill2':dict(role='Skill2',source_count=29,source_cols=6,map=list(range(28)),duration=.46,contactFrame=14),
'basic':dict(role='Basic',source_count=29,source_cols=6,map=[i for i in range(29) if i!=6],duration=.46,contactFrame=6),
}
def run(name):
 spec=SPECS[name];src=Image.open(root/(name+'_raw.png')).convert('RGBA');cols=spec['source_cols']
 raw=[src.crop((i%cols*192,i//cols*192,(i%cols+1)*192,(i//cols+1)*192)) for i in range(spec['source_count'])]
 frames=[raw[i] for i in spec['map']]
 n=len(frames);columns=math.ceil(math.sqrt(n));rows=math.ceil(n/columns)
 sheet=Image.new('RGBA',(columns*192,rows*192))
 preview=root/'preview';preview.mkdir(exist_ok=True)
 frame_dir=root/'frames'/name;frame_dir.mkdir(parents=True,exist_ok=True)
 contact=Image.new('RGB',(columns*192,rows*210),'#222330');draw=ImageDraw.Draw(contact)
 gif=[]
 for i,f in enumerate(frames):
  sheet.paste(f,(i%columns*192,i//columns*192))
  f.save(frame_dir/f'{i:03}.png')
  bg=Image.new('RGBA',(192,192),'#222330');bg.alpha_composite(f)
  contact.paste(bg.convert('RGB'),(i%columns*192,i//columns*210+18))
  draw.text((i%columns*192+4,i//columns*210+3),f"{i} (source {spec['map'][i]})",fill='white')
  gif.append(bg.convert('RGB'))
 path=root/(name+'.png');sheet.save(path);contact.save(preview/(name+'_contact.png'))
 duration=spec['duration'] or .5
 gif[0].save(preview/(name+'.gif'),save_all=True,append_images=gif[1:],duration=max(10,round(duration*1000/n)),loop=0)
 data=dict(role=spec['role'],file=str(path),cell=[192,192],columns=columns,rows=rows,count=n,pivot=[70,133],scale=1.4,duration=spec['duration'],contactFrame=spec['contactFrame'],loop=False,sourceFile=name+'_raw.png',frameMap=spec['map'],rawColumns=cols,uniquePixelFrames=len(set(hashlib.sha256(f.tobytes()).hexdigest() for f in frames)),emptyFrames=[i for i,f in enumerate(frames) if not f.getbbox()],bounds=[f.getbbox() for f in frames],sha256=hashlib.sha256(path.read_bytes()).hexdigest(),status='candidate-awaiting-parent-review',model='PixMiniMax H3',timing='velocity phase' if not spec['duration'] else 'fixed duration')
 (root/(name+'.json')).write_text(json.dumps(data,indent=2),encoding='utf-8')
 print(json.dumps({k:v for k,v in data.items() if k!='bounds'}))
if __name__=='__main__':
 import sys
 for name in sys.argv[1:]:run(name)
