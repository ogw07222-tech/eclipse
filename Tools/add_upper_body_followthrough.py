#!/usr/bin/env python3
"""Rebuild restrained, more upright movement upper bodies into ECLIPSE PNG atlases.

Offline Pillow-only tool. Writes REAL game PNGs, not runtime transforms.
Safe to rerun: source comes from immutable initial .pre-upper-motion.png backups.
The transparent background and all non-movement frames are preserved.
Does not rebuild legs, attacks, hitboxes, or animation timing.
"""
from pathlib import Path
from PIL import Image
import argparse
import hashlib
import math

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "Assets/Resources/Eclipse"
# (asset path, expected dimensions, cell dimensions, columns, movement clips)
CONFIG = [
    ("knight_motion.png", (1024, 2914), (64, 94), 16,
     [("walk", 16, 8, 65, 2.0), ("run", 432, 24, 65, 2.6)]),
    ("ui/white_night_motion.png", (512, 1440), (64, 96), 8,
     [("run", 88, 32, 66, 2.5)]),
]

def upper_body(frame, phase, belt, upright):
    """Resample existing torso as single continuous pixel shapes, anchored at pelvis.

    A fixed backward (leftward for right-facing character) corrective tilt makes
    the crouched silhouette more upright. Small opposite-phase sway accompanies
    alternating planted legs. Do not translate the leg or sword below the belt.
    """
    base = frame.convert("RGBA")
    w, h = base.size
    result = base.copy()
    pixels = result.load()
    source = base.load()
    # Wipe ONLY the source upper region, preserving all y >= belt verbatim.
    for y in range(belt):
        for x in range(w):
            pixels[x, y] = (0, 0, 0, 0)

    cycle = 2 * math.pi * phase
    sway = math.sin(cycle)
    lift = (1.0 - math.cos(2 * cycle)) / 2.0
    for y in range(belt):
        # Pelvis stays fixed, shoulders/head lean backward.
        strength = min(1.0, max(0.0, (belt - y) / 34.0))
        # Stationary backward stance correction up to 2 px;
        # 1 px sway, never body-length bobbing or exaggerated sneak pose.
        dx = round((-upright + 1.0 * sway) * strength)
        dy = -round(0.7 * lift * strength)
        for x in range(w):
            if source[x, y][3] == 0:
                continue
            nx, ny = x + dx, y + dy
            if 0 <= nx < w and 0 <= ny < belt:
                pixels[nx, ny] = source[x, y]
    return result

def changed_indices(clips):
    return {i for _, first, count, _, _ in clips for i in range(first, first + count)}

def build(source, fw, fh, cols, clips):
    before = source.convert("RGBA")
    out = before.copy()
    for name, start, count, belt, upright in clips:
        for i in range(count):
            index = start + i
            x = (index % cols) * fw
            y = (index // cols) * fh
            box = (x, y, x + fw, y + fh)
            original = before.crop(box)
            modified = upper_body(original, i / count, belt, upright)
            # All lower body pixels remain exactly identical.
            assert original.crop((0, belt, fw, fh)).tobytes() == modified.crop((0, belt, fw, fh)).tobytes(), (name, i)
            out.paste(modified, (x, y))
    # Other animation frame slots must be exactly unchanged.
    allowed = changed_indices(clips)
    for index in range((before.width // fw) * (before.height // fh)):
        if index in allowed:
            continue
        x = (index % cols) * fw
        y = (index // cols) * fh
        box = (x, y, x + fw, y + fh)
        assert before.crop(box).tobytes() == out.crop(box).tobytes(), ("unrelated frame modified", index)
    return out

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--preview-dir", type=Path, default=ROOT / "Art/movement-upper-review")
    args = parser.parse_args()
    args.preview_dir.mkdir(parents=True, exist_ok=True)
    for rel, expected, (fw, fh), cols, clips in CONFIG:
        path = ASSETS / rel
        backup = path.with_suffix(".pre-upper-motion.png")
        if not path.exists():
            raise FileNotFoundError(path)
        src = Image.open(backup if backup.exists() else path)
        if src.size != expected:
            raise ValueError((rel, "unexpected atlas dimensions", src.size))
        out = build(src, fw, fh, cols, clips)
        for name, start, count, belt, upright in clips:
            frames = []
            for i in range(count):
                idx = start + i
                crop = (idx % cols * fw, idx // cols * fh, (idx % cols + 1) * fw, (idx // cols + 1) * fh)
                frames.append(out.crop(crop))
            # This is the exact PNG data written to the game atlas.
            contact = Image.new("RGBA", (fw * min(8, count), fh * ((count + 7) // 8)), (0, 0, 0, 0))
            for i, frame in enumerate(frames):
                contact.alpha_composite(frame, (i % 8 * fw, i // 8 * fh))
            key = ("white_night" if rel.startswith("ui/") else "knight") + "_" + name
            contact.save(args.preview_dir / (key + "_contact.png"))
            larger = [f.resize((fw * 4, fh * 4), Image.Resampling.NEAREST) for f in frames]
            larger[0].save(args.preview_dir / (key + ".gif"), save_all=True,
                           append_images=larger[1:],
                           duration=round((700 if name=="run" else 600) / count), loop=0, disposal=2)
            # First/last continuity report; not a subjective visual verdict.
            diff = sum(a != b for a, b in zip(frames[-1].tobytes(), frames[0].tobytes()))
            print(key, "frames:", count, "loop boundary differing bytes:", diff)
        if not args.dry_run:
            if not backup.exists():
                src.save(backup, optimize=True)
            out.save(path, optimize=True)
            print("Updated actual PNG:", path, "SHA256:", hashlib.sha256(path.read_bytes()).hexdigest())
        else:
            print("Dry run; actual PNG not written:", path)
    print("NOTE: Real device playback and pose-quality approval require a human visual check.")

if __name__ == "__main__":
    main()
