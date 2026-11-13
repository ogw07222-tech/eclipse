#!/usr/bin/env python3
"""Apply the owner's hand-authored long/thin spear pixels to a 48-cell atlas."""
from pathlib import Path
import argparse
import hashlib
from PIL import Image

EXPECTED_SOURCE_SHA256 = "47ca56f3e7c11e7d46d69f0688021bfe69543dd3ec6763404a6bd4a58ab8b489"
SHAFT = {
    47: (53, 143, "#d49d38ff"),
    48: (53, 143, "#fff6d9ff"),
    49: (53, 143, "#f4c75eff"),
}
HEAD = {
    44: (160, 164, "#d49d38ff"),
    45: (155, 169, "#d49d38ff"),
    46: (149, 171, "#f4c75eff"),
    47: (142, 172, "#ffe8abff"),
    48: (139, 173, "#fff6d9ff"),
    49: (142, 172, "#ffe8abff"),
    50: (149, 171, "#f4c75eff"),
    51: (155, 169, "#d49d38ff"),
    52: (160, 164, "#d49d38ff"),
}


def rgba(value):
    return tuple(bytes.fromhex(value.lstrip("#")))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path, help="original approved 1040x960 RGBA atlas")
    parser.add_argument("output", type=Path, help="new path; existing files are never overwritten")
    args = parser.parse_args()
    source_bytes = args.source.read_bytes()
    if hashlib.sha256(source_bytes).hexdigest() != EXPECTED_SOURCE_SHA256:
        raise SystemExit("Source hash mismatch; refusing to paint an unreviewed atlas")
    if args.output.exists():
        raise SystemExit("Output exists; choose a new review path")
    image = Image.open(args.source).convert("RGBA")
    if image.size != (1040, 960):
        raise SystemExit(f"Expected 1040x960, got {image.size}")
    for frame in range(48):
        ox, oy = frame % 5 * 208, frame // 5 * 96
        # Reduce the spherical tip bloom without changing the energy wake.
        for y in range(32, 65):
            for x in range(145, 188):
                if ((x - 166) / 20) ** 2 + ((y - 48) / 15) ** 2 > 1:
                    continue
                X, Y = ox + x, oy + y
                red, green, blue, alpha = image.getpixel((X, Y))
                if 0 < alpha < 240:
                    image.putpixel((X, Y), (red, green, blue, round(alpha * 0.20)))
        # Clear the former broad blade and crossguard, preserving surrounding VFX.
        for y in range(39, 58):
            for x in range(112, 174):
                X, Y = ox + x, oy + y
                if image.getpixel((X, Y))[3]:
                    image.putpixel((X, Y), (0, 0, 0, 0))
        # Hand-paint a restrained warm glow under the narrow shaft so the old
        # blade silhouette does not leave a hard transparent notch.
        halo = {43: 6, 44: 10, 45: 16, 46: 24, 47: 32, 48: 36,
                49: 32, 50: 24, 51: 16, 52: 10, 53: 6}
        for y, alpha in halo.items():
            for x in range(112, 174):
                image.putpixel((ox + x, oy + y), (255, 232, 171, alpha))
        # Deliberately authored 3-pixel shaft and tapered, bright point.
        for rows in (SHAFT, HEAD):
            for y, (x0, x1, color) in rows.items():
                pixel = rgba(color)
                for x in range(x0, x1 + 1):
                    image.putpixel((ox + x, oy + y), pixel)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    image.save(args.output)
    print(hashlib.sha256(args.output.read_bytes()).hexdigest())


if __name__ == "__main__":
    main()
