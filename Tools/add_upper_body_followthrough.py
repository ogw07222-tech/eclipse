#!/usr/bin/env python3
"""Add restrained upper-body follow-through to ECLIPSE movement PNGs.

Pillow only; offline. Keeps leg pixels, sword below hip, frame count, and timing.
Run after fetching latest game assets. Creates original backups once.
"""
from pathlib import Path
import argparse
import hashlib
import math
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
TARGETS = (
    ("knight_walk", "knight_motion.png", 64, 94, 16, 8, 16, 2, 65),
    ("knight_run", "knight_motion.png", 64, 94, 432, 24, 16, 2, 65),
    ("white_night_run", "ui/white_night_motion.png", 64, 96, 88, 32, 8, 2, 65),
)

def shift_upper(frame, phase, hip_y, strength=1.0):
    """Subpixel phase -> integer-pixel, nearest-neighbor, no blended pixels.

    Torso lean alternates across steps; breathing-height bounce peaks twice.
    Upper pixels y<hip_y are gently shifted; y>=hip_y is untouched.
    Seam taper at the belt ensures the source legs don't disconnect.
    """
    w, h = frame.size
    rgba = frame.convert("RGBA")
    base = rgba.copy()
    dst = base.load()
    src = rgba.load()
    lean = math.sin(2 * math.pi * phase)
    bounce = (1 - math.cos(4 * math.pi * phase)) * .5
    for y in range(0, hip_y):
        # Full effect at chest/helmet; hold waist and pelvis stable.
        taper = min(1.0, max(0.0, (hip_y - y) / 15.0))
        # Lean is at most 1px, with 2px reserved only in heavy sprint.
        dx = round(1.25 * strength * lean * taper)
        dy = -round(.85 * bounce * taper)
        for x in range(w):
            dst[x, y] = (0, 0, 0, 0)
        for x in range(w):
            nx, ny = x + dx, y + dy
            # Don't write below belt, onto legs, or outside the cell.
            if 0 <= nx < w and 0 <= ny < hip_y and src[x, y][3]:
                dst[nx, ny] = src[x, y]
    # Protect everything at and below the waist, including sword and legs.
    return base

def update_sheet(original, targets):
    out = original.convert("RGBA")
    for name, _, fw, fh, start, count, columns, strength, hip_y in targets:
        for i in range(count):
            index = start + i
            x, y = index % columns * fw, index // columns * fh
            rect = (x, y, x + fw, y + fh)
            old = original.convert("RGBA").crop(rect)
            # Two footfalls per loop; torso swings once in each direction.
            new = shift_upper(old, i / count, hip_y, strength)
            if new.crop((0, hip_y, fw, fh)).tobytes() != old.crop((0, hip_y, fw, fh)).tobytes():
                raise AssertionError((name, i, "lower body changed"))
            out.paste(new, (x, y))
    return out

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--dry-run", action="store_true", help="Validate and report without writing PNGs")
    args = p.parse_args()
    groups = {}
    for target in TARGETS:
        groups.setdefault(target[1], []).append(target)
    for rel, clips in groups.items():
        path = ROOT / "Assets/Resources/Eclipse" / rel
        if not path.is_file():
            raise FileNotFoundError(path)
        backup = path.with_suffix(".pre-upper-motion.png")
        if backup.exists():
            # Always build from the pristine backup, never compound corrections.
            original = Image.open(backup)
        else:
            original = Image.open(path)
        if original.size != (1024, 2914) if rel == "knight_motion.png" else original.size != (512, 1440):
            raise ValueError((rel, "unexpected atlas dimensions", original.size))
        output = update_sheet(original, clips)
        before = original.convert("RGBA")
        changed = set()
        for _, _, fw, fh, start, count, columns, _, _ in clips:
            changed.update(range(start, start + count))
        for i in range((original.width // clips[0][2]) * (original.height // clips[0][3])):
            if i in changed:
                continue
            fw, fh, columns = clips[0][2], clips[0][3], clips[0][6]
            x, y = i % columns * fw, i // columns * fh
            rect = (x, y, x + fw, y + fh)
            if before.crop(rect).tobytes() != output.crop(rect).tobytes():
                raise AssertionError((rel, "other animation changed", i))
        print(f"{rel}: {len(changed)} movement frames processed; other clips preserved")
        if not args.dry_run:
            if not backup.exists():
                original.save(backup)
            output.save(path, optimize=True)
            print(f"  wrote {path}; original kept at {backup}")
    print("Offline sprite update complete; device playback is NOT validated.")

if __name__ == "__main__":
    main()
