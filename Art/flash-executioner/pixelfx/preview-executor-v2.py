from pathlib import Path
from PIL import Image,ImageDraw
import json,hashlib,math,shutil,subprocess
out=Path('/storage/emulated/0/Download/flash executioner/candidates/pixelfx_v2');data=json.loads((out/'manifest.json').read_text());actor=Image.open('/storage/emulated/0/Download/flash executioner/eclipse_executor_attack_firstframe_192.png').convert('RGBA').resize((269,269),Image.Resampling.NEAREST)
proof=[]
for e in data['effects']:
 dest=Path(e['file']).parent;sheet=Image.open(e['file']).convert('RGBA');before=Image.open(e['baselinePng']).convert('RGBA');w,h=e['cell'];cols=e['columns'];n=e['count']
 def frames(im):return [im.crop((i%cols*w,i//cols*h,(i%cols+1)*w,(i//cols+1)*h)) for i in range(n)]
 fs=frames(sheet);bs=frames(before)
 e['firstAlphaEmpty']=not fs[0].getbbox();e['lastAlphaEmpty']=not fs[-1].getbbox();e['uniquePixelFrames']=len({hashlib.sha256(f.tobytes()).hexdigest() for f in fs})
 e['edgePixels']=sum(1 for f in fs for x,y in [(0,yy) for yy in range(h)]+[(w-1,yy) for yy in range(h)]+[(xx,0) for xx in range(w)]+[(xx,h-1) for xx in range(w)] if f.getpixel((x,y))[3]>100)
 samples=sorted(set([0,round((n-1)*.12),round((n-1)*.25),round((n-1)*.5),round((n-1)*.75),n-1]))
 contact=Image.new('RGB',(max(w,200)*len(samples),h*2+52),'#171522');d=ImageDraw.Draw(contact)
 for j,i in enumerate(samples):
  for row,im in [(0,bs[i]),(1,fs[i])]:
   bg=Image.new('RGBA',(w,h),'#171522');bg.alpha_composite(im);contact.paste(bg.convert('RGB'),(j*max(w,200),26+row*(h+13)))
  d.text((j*max(w,200)+3,3),f'{e["role"]} frame{i}',fill='white')
 d.text((3,h+31),'BOTTOM: GOLD VARIANT / TOP: SAME-FPS ORIGINAL',fill='#ffe6ab');contact.save(dest/'before-after.png')
 for bgname,bgcolor in [('dark','#171522'),('light','#d9dce5')]:
  cs=Image.new('RGB',(max(w,200)*len(samples),h+24),bgcolor);dd=ImageDraw.Draw(cs)
  for j,i in enumerate(samples):
   b=Image.new('RGBA',(w,h),bgcolor);b.alpha_composite(fs[i]);cs.paste(b.convert('RGB'),(j*max(w,200),24));dd.text((j*max(w,200)+3,4),f'{e["role"]} {i}',fill='white' if bgname=='dark' else 'black')
  cs.save(dest/('contact-'+bgname+'.png'))
 # GIF is approximate inspection preview; atlas manifest is60fps authoritative.
 display=[]
 for f in fs:
  bg=Image.new('RGBA',(w,h),'#171522');bg.alpha_composite(f);display.append(bg.convert('RGB'))
 display[0].save(dest/'preview.gif',save_all=True,append_images=display[1:],duration=20,loop=0)
 peak=max(range(n),key=lambda i:sum(1 for a in fs[i].getchannel('A').getdata() if a>180))
 e['peakCoverageFrame']=peak
 proof.append(dict(role=e['role'],sourceId=e['sourceId'],sourceHash=e['sourceSha256'],variantHash=e['sha256'],changedSourceFunctions=False,alphaEdge=e['postprocess'],frameMap=e['frameMap'],edgePixels=e['edgePixels']))
# Frame at maximal opaque coverage, actual104px actor, runtime scale + focal offsets.
for bgname,bgcolor in [('dark','#171522'),('light','#d9dce5')]:
 ov=Image.new('RGB',(5*512,3*512),bgcolor);d=ImageDraw.Draw(ov)
 for j,e in enumerate(data['effects']):
  p=Image.new('RGBA',(512,488),bgcolor);ox,oy=256,350
  sh=Image.open(e['file']).convert('RGBA');w,h=e['cell'];i=e['peakCoverageFrame'];co=e['columns'];f=sh.crop((i%co*w,i//co*h,(i%co+1)*w,(i//co+1)*h));scale=e['scale'];f=f.resize((round(w*scale),round(h*scale)),Image.Resampling.NEAREST)
  if e['role']=='Orb':target=(ox+74,oy-80+e['offset'][1])
  elif e['role'] in ['Impact','Lance']:target=(ox+70,oy-50)
  elif e['role']=='SkyLance':target=(ox+95,oy-12)
  else:target=(ox+e['offset'][0],oy+e['offset'][1])
  pivotScaled=(e['pivot'][0]*scale,e['pivot'][1]*scale)
  if e['role']=='SkyLance':
   f=f.transpose(Image.Transpose.ROTATE_270);pivotScaled=(f.width-pivotScaled[1],pivotScaled[0])
  fxpos=(round(target[0]-pivotScaled[0]),round(target[1]-pivotScaled[1]))
  if e['layer']=='back':p.alpha_composite(f,fxpos)
  actorHere=actor
  poseMap={'Slash':('basic.png',6,6),'Rush':('skill1.png',6,6),'Command':('skill2.png',6,14),'BlinkOut':('skill3.png',5,10),'BlinkIn':('skill3.png',5,12),'Dash':('dash.png',3,2),'Land':('land.png',3,2),'Swap':('swap.png',4,5)}
  if e['role'] in poseMap:
   nm,ac,ai=poseMap[e['role']];ash=Image.open('/storage/emulated/0/Download/flash executioner/approved/'+nm).convert('RGBA');actorHere=ash.crop((ai%ac*192,ai//ac*192,(ai%ac+1)*192,(ai//ac+1)*192)).resize((269,269),Image.Resampling.NEAREST)
  p.alpha_composite(actorHere,(ox-round(70*1.4),oy-round(133*1.4)))
  if e['layer']!='back':p.alpha_composite(f,fxpos)
  x=j%5*512;y=j//5*512;ov.paste(p.convert('RGB'),(x,y+24));d.text((x+4,y+4),e['role']+' '+e['sourceId'],fill='#ffe6ab' if bgname=='dark' else 'black')
 ov.save(out/('actor-overview-'+bgname+'.png'))
(out/'manifest.json').write_text(json.dumps(data,indent=2),encoding='utf-8');(out/'postprocess-evidence.json').write_text(json.dumps(proof,indent=2),encoding='utf-8')
# Verify original source hash against committed upstream bytes.
repo=Path('/data/data/com.termux/files/usr/tmp/pixelfx-executor');verified=[]
for e in data['effects']:
 b=subprocess.check_output(['git','show','HEAD:src/fx/'+e['sourceId']+'.js'],cwd=repo)
 verified.append(dict(role=e['role'],source_sha256_matches_commit=hashlib.sha256(b).hexdigest()==e['sourceSha256'],variant_hash_matches=hashlib.sha256(Path(e['file']).read_bytes()).hexdigest()==e['sha256']))
(out/'source-verification.json').write_text(json.dumps(verified,indent=2))
shutil.copy(repo/'tools/render-executor-v2.mjs',out/'render-executor-v2.mjs')
shutil.copy(Path(__file__),out/'preview-executor-v2.py')
for bgname in ['light','dark']:
 shutil.copy(out/('actor-overview-'+bgname+'.png'),out/'packing-correction'/('after-actor-overview-'+bgname+'.png'))
print(json.dumps([(e['role'],e['count'],e['edgePixels'],e['firstAlphaEmpty'],e['lastAlphaEmpty']) for e in data['effects']]))
