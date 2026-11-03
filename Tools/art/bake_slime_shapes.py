#!/usr/bin/env python3
"""Bake pixel-row silhouette bands from each actual monster PNG pose.

Detached droplets are visual decoration. The largest connected opaque component
defines the body. Band edges preserve the front/back silhouette per animation
frame and use exactly the renderer's cell, pivot, scale and horizontal flip.
"""
from pathlib import Path
from collections import deque
import argparse
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]
CELL=128
NAMES=('green','red','purple')

def body_pixels(cell):
    alpha=cell.convert('RGBA').getchannel('A')
    todo={(x,y) for y in range(CELL) for x in range(CELL) if alpha.getpixel((x,y))>=128}
    largest=set()
    while todo:
        seed=todo.pop();component={seed};queue=deque([seed])
        while queue:
            x,y=queue.popleft()
            for dx,dy in ((1,0),(-1,0),(0,1),(0,-1),(1,1),(1,-1),(-1,1),(-1,-1)):
                n=(x+dx,y+dy)
                if n in todo:todo.remove(n);component.add(n);queue.append(n)
        if len(component)>len(largest):largest=component
    if len(largest)<100:raise ValueError('Monster body missing')
    return largest

def generate():
    species=[]
    for name in NAMES:
        atlas=Image.open(ROOT/f'Assets/Resources/Eclipse/monsters/slime_{name}.png')
        assert atlas.size==(512,512),(name,'atlas size')
        frames=[]
        for i in range(16):
            pixels=body_pixels(atlas.crop((i%4*CELL,i//4*CELL,(i%4+1)*CELL,(i//4+1)*CELL)))
            xs=[p[0] for p in pixels];ys=[p[1] for p in pixels]
            rows=[[] for _ in range(CELL)]
            for x,y in pixels:rows[y].append(x)
            bands=[]
            for band in range(CELL):
                points=rows[band]
                bands.append((min(points),max(points)+1) if points else (0,0))
            frames.append((min(xs),min(ys),max(xs)+1,max(ys)+1,bands))
        species.append(frames)
    header='#pragma once\n#include <array>\nnamespace eclipse {\nstruct SlimeBand { int left,right; };\nstruct SlimeShape { int left,top,right,bottom; std::array<SlimeBand,128> bands; };\ninline constexpr SlimeShape slimeShapes[3][16]={\n'
    for frames in species:
        header+='{\n'
        for left,top,right,bottom,bands in frames:
            header+='{%d,%d,%d,%d,{{'%(left,top,right,bottom)+','.join('{%d,%d}'%p for p in bands)+'}}},\n'
        header+='},\n'
    return header+'};\n}\n'

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--check',action='store_true');args=parser.parse_args()
    target=ROOT/'native-engine/src/slime_shapes.hpp';content=generate()
    if args.check:
        assert target.read_text()==content,'Monster PNG changed: rebake silhouette bands'
        print('PASS all 48 animated monster silhouettes match committed collision bands')
    else:target.write_text(content);print('Baked 48 body silhouettes from actual PNG alpha')
if __name__=='__main__':main()
