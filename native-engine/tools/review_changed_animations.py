#!/usr/bin/env python3
"""Review PR #72 using the owner's unmodified offline sprite tool.

Run explicitly from anywhere. Compiles only a small Animation selection sampler,
then exports original PNGs in actual sampled 60Hz order. Does not launch SDL,
run the owner's test suite, change assets, or access the network.
"""
import argparse
import csv
import hashlib
import io
import json
from pathlib import Path
import shutil
import subprocess
import eclipse_sprite_tool as player

ROOT = Path(__file__).resolve().parents[2]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'native-engine/review/changed')
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    compiler=shutil.which('c++')
    if not compiler:raise RuntimeError('A C++17 compiler is required for actual Animation phase sampling')
    exe=args.output/'animation-selection-sample'
    subprocess.run([compiler,'-std=c++17','-O2','-Wall','-Wextra','-Werror',
                    str(Path(__file__).with_name('sample_changed_animations.cpp')),'-o',str(exe)],check=True)
    result=subprocess.run([str(exe.resolve())],text=True,capture_output=True,check=True)
    (args.output/'trace.csv').write_text(result.stdout)
    rows=list(csv.DictReader(io.StringIO(result.stdout)))
    definitions={
        'weak':('knight_motion.png',64,94,16),
        'mage':('mage_motion.png',64,94,8),
        'white':('ui/white_night_motion.png',64,96,8),
    }
    report={'scope':'60Hz Animation selection + frame-by-frame PNG inspection; not SDL/device playback or a motion-quality verdict',
            'sampler':result.stderr.strip(),'clips':[]}
    for name,(path,w,h,columns) in definitions.items():
        samples=[r for r in rows if r['clip']==name]
        indices=[int(r['source']) for r in samples]
        image=player.Image.open(ROOT/'Assets/Resources/Eclipse'/path).convert('RGBA')
        frames=[image.crop((i%columns*w,i//columns*h,(i%columns+1)*w,(i//columns+1)*h)) for i in indices]
        # Contact sheets omit consecutive capture holds only; MP4 retains every
        # 60Hz sample. This does not create or inflate authored PNG pose counts.
        distinct_order=[frame for i,frame in enumerate(frames) if i==0 or indices[i]!=indices[i-1]]
        player.write_contact(distinct_order,args.output/f'{name}_sampled_contact.png',3,8,(24,28,36))
        if shutil.which('ffmpeg'):
            player.write_mp4(frames,args.output/f'{name}_sampled.mp4',4,60,3,(24,28,36))
            player.write_mp4(frames,args.output/f'{name}_sampled_slow.mp4',4,15,1,(24,28,36))
        # GIF frame delays are quantized to 10ms. Do not use GIFs as FPS evidence.
        report['clips'].append({'name':name,'asset':path,'samples':len(frames),'indices':indices,
            'unique_png_frames':len(set(indices)),'sample_seconds':len(frames)/60,
            'metrics':player.metrics(frames),
            'first_last_difference':player.metrics([frames[-1],frames[0]])[-1].get('pixel_change_fraction_est')})
    report['sprite_tool_sha256']=hashlib.sha256(Path(player.__file__).read_bytes()).hexdigest()
    (args.output/'selection-review.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print(result.stderr.strip())
    print('Saved contact sheets, 60Hz traces and MP4s (when FFmpeg is available) to',args.output)

if __name__=='__main__':main()
