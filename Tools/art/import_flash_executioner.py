#!/usr/bin/env python3
"""Import only the main reviewer's explicitly approved Flash Executioner PNGs.
Static import/metadata checks only; this does not run game/device tests.
"""
from pathlib import Path
import argparse, hashlib, json, shutil, math
from PIL import Image

ACTORS = ['Basic','Skill1','Skill2','Skill3','Ultimate','Idle','Run','Jump','Fall','Land','Dash','Swap']
EFFECTS = ['Slash','Rush','Command','BlinkOut','BlinkIn','Verdict','Impact','Orb','Lance','SkyLance','LanceHit','Dash','Swap']
ROOT = Path(__file__).resolve().parents[2]

def literal(value):
    if isinstance(value,bool): return 'true' if value else 'false'
    if isinstance(value,float): return str(value)+'f'
    return str(value)

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('manifest',type=Path)
    args=parser.parse_args()
    manifest=json.loads(args.manifest.read_text())
    if manifest.get('review_status')!='approved':
        raise SystemExit('Refusing import: main reviewer approval missing')
    source_root=ROOT/'Art/flash-executioner/reviewed'
    runtime_root=ROOT/'Assets/Resources/Eclipse/flash_executioner'
    actors={};effects={};imported=[]
    for kind,roles,table in [('actors',ACTORS,actors),('effects',EFFECTS,effects)]:
        for row in manifest.get(kind,[]):
            if not row.get('approved'):
                continue
            role=row['role']
            if kind=='effects' and not all(row.get(field) for field in ('sourceId','sourceCommit','sourceSha256')):
                raise SystemExit('Effect must identify its reviewed existing PixelFX source: '+role)
            if role not in roles or role in table:
                raise SystemExit('Unknown or duplicate role: '+role)
            if not math.isfinite(row['duration']) or row['duration']<=0:
                raise SystemExit('Invalid playback duration: '+role)
            flight=row.get('flightFrameRange',[-1,-1])
            if kind=='effects' and role in ('Lance','SkyLance') and not row.get('loop') and flight==[-1,-1]:
                raise SystemExit('Projectile needs an approved loop or flight-frame range: '+role)
            source=Path(row['file'])
            sha=hashlib.sha256(source.read_bytes()).hexdigest()
            if row.get('sha256')!=sha:
                raise SystemExit('Reviewer hash mismatch: '+str(source))
            cell=row['cell'];cols=row['columns'];rows=row['rows'];count=row['count']
            if min(*cell,cols,rows,count)<=0 or count>cols*rows:
                raise SystemExit('Invalid geometry: '+role)
            if flight!=[-1,-1] and not 0<=flight[0]<=flight[1]<count:
                raise SystemExit('Invalid reviewed flight range: '+role)
            if row.get('triggerLeadSeconds',0)<0 or not 0<=row.get('spawnAgeSeconds',0)<row['duration']:
                raise SystemExit('Invalid reviewed source peak timing: '+role)
            image=Image.open(source)
            if image.size!=(cell[0]*cols,cell[1]*rows) or 'A' not in image.getbands():
                raise SystemExit('PNG dimension/alpha mismatch: '+role)
            if not 0<=row.get('contactFrame',-1)<count and row.get('contactFrame',-1)!=-1:
                raise SystemExit('Invalid contact frame: '+role)
            name=kind[:-1]+'_'+role.lower()+'.png'
            target=runtime_root/name
            art=source_root/name
            target.parent.mkdir(parents=True,exist_ok=True)
            art.parent.mkdir(parents=True,exist_ok=True)
            # Existing reviewed assets can be their own editable source. Avoid
            # shutil SameFileError when source and reviewed output already match.
            if source.resolve()!=target.resolve():
                shutil.copyfile(source,target)
            if source.resolve()!=art.resolve():
                shutil.copyfile(source,art)
            record={**row,'file':'flash_executioner/'+name,'source_path':str(art.relative_to(ROOT)),
                    'runtime_path':str(target.relative_to(ROOT)),'sha256':sha,
                    'status':'reviewed; runtime binding prepared; owner device verification pending',
                    'library_status':'not synced: no ChatGPT Library file capability available'}
            if kind=='actors' and row.get('sourceFile'):
                record['source_path_raw']='Art/flash-executioner/pixellab/'+row['sourceFile']
            for source_frame in record.get('sourceFrameMap',[]):
                if isinstance(source_frame,dict):
                    original=source_frame.get('file','')
                    filename=Path(original).name
                    source_frame['file']=('Art/flash-executioner/pixellab/skill1_original_29.png'
                        if filename=='skill1.png' else 'Art/flash-executioner/pixellab/'+filename)
            table[role]=record;imported.append(record)
    # The resource directory is authoritative for this approved manifest.
    # Remove superseded runtime/source sheets so packaging's PNG glob cannot
    # silently ship a previously rejected effect.
    keep={Path(row['file']).name for row in imported}
    for folder in [runtime_root,source_root]:
        for old in folder.glob('*.png'):
            if old.name not in keep:
                old.unlink()
    header='''#pragma once
#include <array>
namespace eclipse {
enum class FlashActor { Basic, Skill1, Skill2, Skill3, Ultimate, Idle, Run, Jump, Fall, Land, Dash, Swap, Count };
enum class FlashFx { Slash, Rush, Command, BlinkOut, BlinkIn, Verdict, Impact, Orb, Lance, SkyLance, LanceHit, Dash, Swap, Count };
struct FlashSheet {
    const char* file=nullptr;
    int cellW=0,cellH=0,columns=0,rows=0,count=0;
    float pivotX=0,pivotY=0,scale=1,duration=0;
    int contactFrame=-1;
    bool loop=false;
    bool front=true;
    int flightFirst=-1,flightLast=-1;
    float triggerLeadSeconds=0,spawnAgeSeconds=0,nativeDirectionDegrees=0,flightFrameFps=60;
    float offsetX=0,offsetY=0;
    int targetFeet=0;
    float fps=60;
    float triggerStartAgeSeconds=-1;
};
// Only explicitly approved entries are bound. Other roles retain legacy art.
'''
    for name,enum,roles,table in [('flashActors','FlashActor',ACTORS,actors),('flashEffects','FlashFx',EFFECTS,effects)]:
        header+='inline constexpr std::array<FlashSheet,static_cast<int>('+enum+'::Count)> '+name+'{{\n'
        for role in roles:
            row=table.get(role)
            if not row:
                header+='    {}, // '+role+': legacy retained\n';continue
            values=[json.dumps(row['file']),*row['cell'],row['columns'],row['rows'],row['count'],
                    float(row['pivot'][0]),float(row['pivot'][1]),float(row.get('scale',1)),float(row['duration']),
                    row.get('contactFrame',-1),row.get('loop',False),row.get('layer','front')=='front',*row.get('flightFrameRange',[-1,-1]),
                    float(row.get('triggerLeadSeconds',0)),float(row.get('spawnAgeSeconds',0)),
                    float(row.get('nativeDirectionDegrees',90 if role=='SkyLance' and kind=='effects' else 0)),
                    float(row.get('flightFrameFps',60)),*map(float,row.get('offset',[0,0])),
                    1 if row.get('anchorMode') in ('target-feet','target-ground') else 0,float(row.get('fps',60)),float(row.get('triggerStartAgeSeconds',-1))]
            header+='    {'+', '.join(v if i==0 else literal(v) for i,v in enumerate(values))+'}, // '+role+'\n'
        header+='}};\n'
    header+='}\n'
    (ROOT/'native-engine/src/flash_executioner_assets.hpp').write_text(header)
    record={'review_manifest':'Art/flash-executioner/review-approval.json','review_status':'approved individual assets only',
            'assets':imported,'gameplay_changed':False,'tests_run':False,'library_synced':False}
    source_root.mkdir(parents=True,exist_ok=True)
    approval_path=ROOT/'Art/flash-executioner/review-approval.json'
    if args.manifest.resolve()!=approval_path.resolve():
        shutil.copyfile(args.manifest,approval_path)
    (source_root/'manifest.json').write_text(json.dumps(record,indent=2,ensure_ascii=False))
    print('Imported '+str(len(imported))+' main-reviewed PNG sheets; no game tests/builds run.')

if __name__=='__main__':main()
