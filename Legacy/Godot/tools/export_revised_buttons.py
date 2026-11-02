#!/usr/bin/env python3
"""Export actual transparent, nearest-neighbor PNG controls for ECLIPSE.
Does not modify the knight, gameplay code, or prior UI assets.
"""
from PIL import Image, ImageDraw
from pathlib import Path

OUT=Path("assets/ui/revised")
OUT.mkdir(parents=True,exist_ok=True)
INK=(12,21,30,255); BG=(31,46,56,245); BORDER=(147,171,172,255)
WHITE=(246,245,226,255); BLUE=(89,165,245,255); YELLOW=(255,210,91,255)
def save(name,draw_icon,pressed=False):
    im=Image.new("RGBA",(48,48),(0,0,0,0))
    d=ImageDraw.Draw(im)
    # Coarse stepped circular control, crisp unmixed pixels.
    step=[(14,2,33,45),(9,5,38,42),(5,9,42,38),(2,14,45,33)]
    for box in step:d.rectangle(box,fill=INK)
    inner=[(15,5,32,42),(10,8,37,39),(7,12,40,35),(5,15,42,32)]
    for box in inner:d.rectangle(box,fill=BORDER if pressed else BG)
    d.rectangle((8,17,39,30),fill=BG)
    d.rectangle((17,8,30,39),fill=BG)
    draw_icon(d)
    im.save(OUT/name,optimize=True)

def attack(d):
    d.polygon([(14,31),(17,34),(34,17),(34,12),(30,14)],fill=WHITE)
    d.rectangle((11,31,21,33),fill=BORDER)
    d.rectangle((14,34,17,38),fill=BORDER)
def dash(d):
    d.polygon([(14,28),(21,19),(24,22),(34,10),(31,24),(23,30)],fill=YELLOW)
    d.rectangle((8,17,17,19),fill=WHITE)
    d.rectangle((7,24,13,26),fill=WHITE)
def swap(d):
    d.line([(11,18),(30,18),(30,14)],fill=WHITE,width=4)
    d.polygon([(29,9),(37,17),(29,24)],fill=WHITE)
    d.line([(37,31),(17,31),(17,35)],fill=WHITE,width=4)
    d.polygon([(18,40),(10,32),(18,25)],fill=WHITE)
def skill(d):
    d.rectangle((21,11,26,37),fill=WHITE)
    d.rectangle((11,21,37,26),fill=WHITE)
    for x,y in [(13,13),(34,13),(13,34),(34,34)]:
        d.rectangle((x,y,x+2,y+2),fill=BLUE)
def arrows(d):
    for pts in [[(24,6),(17,14),(21,14),(21,18),(27,18),(27,14),(31,14)],
                [(24,42),(17,34),(21,34),(21,30),(27,30),(27,34),(31,34)],
                [(6,24),(14,17),(14,21),(18,21),(18,27),(14,27),(14,31)],
                [(42,24),(34,17),(34,21),(30,21),(30,27),(34,27),(34,31)]]:
        d.polygon(pts,fill=WHITE)
    d.rectangle((22,22,26,26),fill=BORDER)

for name,fn in [('attack',attack),('dash',dash),('swap',swap),('skill',skill),('move_arrows',arrows)]:
    save(name+'.png',fn)
    save(name+'_pressed.png',fn,True)
    print(OUT/(name+'.png'))
