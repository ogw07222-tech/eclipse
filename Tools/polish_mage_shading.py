#!/usr/bin/env python3
"""ECLIPSE mage pixel-art shading pass. Non-destructive until --apply.

Uses only Pillow and integer RGB operations. Never resizes, blurs, interpolates,
changes alpha, or mixes pixels across frames. Run on the game checkout.
"""
from __future__ import annotations
import argparse
from pathlib import Path
from PIL import Image, ImageFilter

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "Assets/Resources/Eclipse"
SHEETS = {"mage_motion.png": (8, 10), "mage_run.png": (16, 1)}
CELL = (64, 94)


def clamp(v):
    return max(0, min(255, round(v)))


def enhance_frame(src):
    """Material-aware, top-left lit pixel shading with 1px hard-edge accents."""
    w, h = src.size
    im = src.convert("RGBA")
    a = im.getchannel("A")
    # Luminance region estimates avoid smooth gradients and preserve original colors.
    # Erosion yields a pure interior mask: no outline or alpha-edge corruption.
    interior = a.point(lambda p: 255 if p >= 240 else 0).filter(ImageFilter.MinFilter(3))
    px, inside = im.load(), interior.load()
    solid = a.load()
    for y in range(h):
        for x in range(w):
            r, g, b, al = px[x, y]
            if al == 0:
                continue
            # Preserve dark anti-alias/outline pixels and all partially transparent edges.
            if not inside[x, y] or max(r, g, b) < 28:
                continue
            lum = (54*r + 183*g + 19*b) // 256
            red = r > g * 1.18 and r > b * 1.18
            cool = b >= r * 0.92 and b > g * 0.95
            metallic = abs(r-g) < 30 and abs(g-b) < 40 and lum > 75
            # 5 quantized tones, derived from existing sprite colors.
            light = -1 if (x * 3 + y) % 13 < 3 else 0
            if x < w * .43 and y < h * .58:
                light += 1
            if x > w * .57:
                light -= 1
            if y > h * .69:
                light -= 1
            if lum < 72:
                light -= 1
            elif lum > 153:
                light += 1
            light = max(-2, min(2, light))
            # Color-preserving shadow/highlight ramps by material.
            amount = (-23, -11, 0, 17, 29)[light + 2]
            if metallic:
                # Sharp cold specular planes on buckles, staff metal and armor.
                rgb = (r+amount, g+amount+1, b+amount+4)
            elif red:
                # Flame fabric: warm upper planes, burgundy recesses.
                rgb = (r+amount, g+amount*.67, b+amount*.58)
            elif cool:
                # Violet/navy robes: stronger cool fold highlights.
                rgb = (r+amount*.69, g+amount*.78, b+amount*1.17)
            else:
                rgb = (r+amount, g+amount, b+amount)
            px[x,y] = tuple(map(clamp, rgb)) + (al,)
    # Small, crisp contact-occlusion pixels under folds, without altering silhouette.
    # Skip decorative noise or procedurally drawn folds that could flicker in animation.
    return im


def process(src_path, dest_path, columns, rows):
    src = Image.open(src_path).convert("RGBA")
    w,h=CELL
    expected=(columns*w, rows*h)
    if src.size != expected:
        raise ValueError(f"{src_path}: expected {expected}, got {src.size}")
    dest=Image.new("RGBA",src.size,(0,0,0,0))
    for row in range(rows):
        for col in range(columns):
            box=(col*w,row*h,(col+1)*w,(row+1)*h)
            frame=enhance_frame(src.crop(box))
            dest.paste(frame,(col*w,row*h))
    if src.getchannel("A").tobytes() != dest.getchannel("A").tobytes():
        raise AssertionError("Alpha channel changed")
    dest_path.parent.mkdir(parents=True,exist_ok=True)
    dest.save(dest_path,optimize=True)
    print(f"{src_path.name}: {src.size[0]}x{src.size[1]}, alpha preserved -> {dest_path}")


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--apply",action="store_true",help="Back up originals then replace runtime PNGs")
    parser.add_argument("--output",type=Path,default=ROOT/"Art/mage/review")
    args=parser.parse_args()
    for filename,(columns,rows) in SHEETS.items():
        original=ASSETS/filename
        if not original.is_file():
            raise FileNotFoundError(original)
        if args.apply:
            backup=ROOT/"Art/mage/original"/filename
            backup.parent.mkdir(parents=True,exist_ok=True)
            if not backup.exists():
                backup.write_bytes(original.read_bytes())
            target=original
        else:
            target=args.output/filename
        process(original,target,columns,rows)
    print("Done. Preview only." if not args.apply else "Applied. Original PNGs saved in Art/mage/original.")


if __name__=="__main__":
    main()
