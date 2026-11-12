#!/usr/bin/env python3
"""Import only the main reviewer's explicitly approved Flash Executioner PNGs.
Static import/metadata checks only; this does not run game/device tests.
"""
from pathlib import Path
import argparse, hashlib, json, shutil, math
from PIL import Image

ACTORS = ['Basic','Skill1','Skill2','Skill3','Ultimate','Idle','Run','Jump','Fall','Land','Dash','Swap']
EFFECTS = ['Slash','Rush','Command','BlinkOut','BlinkIn','Verdict','Impact','Orb','Lance','SkyLance','LanceHit','Dash','Jump','Land','Swap']
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
            if role not in roles or role in table:
                raise SystemExit('Unknown or duplicate role: '+role)
            source=Path(row['file'])
            sha=hashlib.sha256(source.read_bytes()).hexdigest()
            if row.get('sha256')!=sha:
                raise SystemExit('Reviewer hash mismatch: '+str(source))
            cell=row['cell'];cols=row['columns'];rows=row['rows'];count=row['count']
            if min(*cell,cols,rows,count)<=0 or count>cols*rows:
                raise SystemExit('Invalid geometry: '+role)
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
            shutil.copyfile(source,target);shutil.copyfile(source,art)
            record={**row,'file':'flash_executioner/'+name,'source_path':str(art.relative_to(ROOT)),
                    'runtime_path':str(target.relative_to(ROOT)),'sha256':sha,
                    'status':'reviewed; runtime binding prepared; owner device verification pending',
                    'library_status':'not synced: no ChatGPT Library file capability available'}
            table[role]=record;imported.append(record)
    header='''#pragma once
#include <array>
namespace eclipse {
enum class FlashActor { Basic, Skill1, Skill2, Skill3, Ultimate, Idle, Run, Jump, Fall, Land, Dash, Swap, Count };
enum class FlashFx { Slash, Rush, Command, BlinkOut, BlinkIn, Verdict, Impact, Orb, Lance, SkyLance, LanceHit, Dash, Jump, Land, Swap, Count };
struct FlashSheet {
    const char* file=nullptr;
    int cellW=0,cellH=0,columns=0,rows=0,count=0;
    float pivotX=0,pivotY=0,scale=1,duration=0;
    int contactFrame=-1;
    bool loop=false;
    bool front=true;
    int flightFirst=-1,flightLast=-1;
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
                    row.get('contactFrame',-1),row.get('loop',False),row.get('layer','front')=='front',*row.get('flightFrameRange',[-1,-1])]
            header+='    {'+', '.join(v if i==0 else literal(v) for i,v in enumerate(values))+'}, // '+role+'\n'
        header+='}};\n'
    header+='}\n'
    (ROOT/'native-engine/src/flash_executioner_assets.hpp').write_text(header)
    record={'review_manifest':str(args.manifest),'review_status':'approved individual assets only',
            'assets':imported,'gameplay_changed':False,'tests_run':False,'library_synced':False}
    source_root.mkdir(parents=True,exist_ok=True)
    (source_root/'manifest.json').write_text(json.dumps(record,indent=2,ensure_ascii=False))
    print('Imported '+str(len(imported))+' main-reviewed PNG sheets; no game tests/builds run.')

if __name__=='__main__':main()
