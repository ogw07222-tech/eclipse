#!/usr/bin/env python3
"""Assemble ECLIPSE castle-boss sheets from Pixellab frames (selection + alignment only).

Nothing here draws, warps or interpolates a frame. The script only
  * picks whole frames in the order given by a spec file (repeats are deliberate holds),
  * places each frame on the shared 256 px cell with a per-frame anchor (leg centroid x and the
    registered FOOT row, never the lowest pixel: a sword tip or cloak can hang below the feet),
  * reports every pair of neighbouring frames (pixel change %), flagging near-duplicates
    and jumps, so authored key poses, in-betweens and holds stay distinguishable.

Spec (JSON):
{ "export": "<unzipped Pixellab export dir>",
  "move":   ["walk:1", "walk:2", ...],            # exactly 8 entries  (anim:frame_index)
  "attack": ["attack_keys2:0", ...],              # exactly 14 entries
  "motions": {"stab": ["m_stab_keys:0", ...14 entries]},     # optional: extra per-move sheets
  "roles":  {"attack_keys2:2": "key", "inbetween_strike:0": "inbetween"} }   # optional notes

Usage: assemble_castle_boss.py <spec.json> <out_dir>
"""
import json, os, sys
import numpy as np
from PIL import Image

CELL, FEET_Y, PIVOT_X = 256, 236, 128
MOVE_FRAMES, ATTACK_FRAMES = 8, 14


def _mask(img):
    return np.array(img)[:, :, 3] > 40


def _frames(export, anim):
    d = os.path.join(export, "Idle", "animations", anim, "east")
    return [Image.open(os.path.join(d, f)).convert("RGBA") for f in sorted(os.listdir(d))]


def _offset(a, b):
    """Shift (dy, dx) that lays mask b over mask a (FFT cross-correlation); returns shift and mismatch %."""
    h, w = a.shape[0] + b.shape[0], a.shape[1] + b.shape[1]
    fa = np.fft.rfft2(a.astype(float), (h, w))
    fb = np.fft.rfft2(b.astype(float), (h, w))
    cc = np.fft.irfft2(fa * np.conj(fb), (h, w))
    y, x = np.unravel_index(int(np.argmax(cc)), cc.shape)
    overlap = cc[y, x]
    union = a.sum() + b.sum() - overlap
    if y > h // 2: y -= h
    if x > w // 2: x -= w
    return int(y), int(x), float(100 * (union - overlap) / max(1, union))


_CACHE = {}
TIPS = {}   # label -> [(tip_x, tip_y)] per frame, in cell pixels (for effect anchoring)


def _tip(cell):
    a = _mask(cell)
    ys, xs = np.nonzero(a)
    tx = int(xs.max())
    return tx, int(round(ys[xs >= tx - 2].mean()))


def _setup(export):
    """Key animations share one canvas with feet planted on one row (the reference frame's lowest row
    is the feet, because the reference is the idle pose). Every other animation is registered to a key
    frame so its feet land on the same row. The lowest pixel of a frame is NOT the feet: a sword tip or
    cloak can hang below them."""
    if export in _CACHE: return _CACHE[export]
    base = os.path.join(export, "Idle", "animations")
    keys = [a for a in sorted(os.listdir(base)) if a.endswith(("_keys", "_keys2"))]
    key_frames = [(a, i, f) for a in keys for i, f in enumerate(_frames(export, a))]
    ref = _mask(key_frames[0][2]); ys, _ = np.nonzero(ref); feet = int(ys.max())
    shifts = {}
    for anim in sorted(os.listdir(base)):
        if anim in keys: shifts[anim] = (0, 0, 0.0); continue
        frames = _frames(export, anim); best = None
        for idx in (0, len(frames) - 1):
            fm = _mask(frames[idx])
            for (_, _, kf) in key_frames:
                y, x, mis = _offset(_mask(kf), fm)
                if best is None or mis < best[2]: best = (y, x, mis)
        shifts[anim] = best
        print("  registered %-22s dy=%+d dx=%+d mismatch=%.1f%%" % (anim, best[0], best[1], best[2]))
    _CACHE[export] = (feet, shifts)
    return _CACHE[export]


def load(export, token):
    anim, idx = token.rsplit(":", 1)
    feet, shifts = _setup(export)
    img = _frames(export, anim)[int(idx)]
    return img, feet - shifts[anim][0]       # image, feet row in this frame's own coordinates


def anchor(img, feet_row):
    a = _mask(img)
    ys, xs = np.nonzero(a)
    top = ys.min()
    band = (ys > feet_row - max(6, int(0.22 * (feet_row - top)))) & (ys <= feet_row)
    if not band.any():
        # Airborne frame: nothing near the ground. Use the lowest slice of the body for x only; the
        # height stays on the shared ground baseline, so the character really is in the air.
        low = ys.max() - max(6, int(0.22 * (ys.max() - top)))
        band = ys >= low
    return int(round(xs[band].mean())), int(feet_row)


def place(img, anch):
    cell = Image.new("RGBA", (CELL, CELL), (0, 0, 0, 0))
    cell.paste(img, (PIVOT_X - anch[0], FEET_Y - anch[1]), img)
    return cell


def change(a, b):
    A = np.array(a)[:, :, 3] > 40
    B = np.array(b)[:, :, 3] > 40
    return float((A ^ B).sum()) / max(1, int((A | B).sum())) * 100


def build(export, tokens, label, roles):
    loaded = [load(export, t) for t in tokens]
    cells = [place(i, anchor(i, f)) for (i, f) in loaded]
    sheet = Image.new("RGBA", (CELL * len(cells), CELL), (0, 0, 0, 0))
    for i, c in enumerate(cells):
        sheet.alpha_composite(c, (i * CELL, 0))
    TIPS[label] = [_tip(c) for c in cells]
    print(label)
    for i, t in enumerate(tokens):
        note = roles.get(t, "")
        if i and tokens[i - 1] == t:
            note = "hold"
        step = "" if i == 0 else "  d=%.1f%%" % change(cells[i - 1], cells[i])
        flag = ""
        if i and tokens[i - 1] != t:
            d = change(cells[i - 1], cells[i])
            flag = "  NEAR-DUPLICATE" if d < 6 else ("  BIG JUMP" if d > 70 else "")
        print("  %2d %-24s %s%s%s" % (i, t, note, step, flag))
    return sheet


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    spec = json.load(open(sys.argv[1], encoding="utf-8"))
    out = sys.argv[2]
    os.makedirs(out, exist_ok=True)
    roles = spec.get("roles", {})
    if len(spec["move"]) != MOVE_FRAMES or len(spec["attack"]) != ATTACK_FRAMES:
        raise SystemExit("need %d move and %d attack entries" % (MOVE_FRAMES, ATTACK_FRAMES))
    build(spec["export"], spec["move"], "move", roles).save(os.path.join(out, "이동_sheet.png"))
    build(spec["export"], spec["attack"], "attack", roles).save(os.path.join(out, "공격_sheet.png"))
    # One 14-frame sheet per boss motion (모션_<name>_sheet.png). Moves select a motion in bossStepDef.
    for name, tokens in spec.get("motions", {}).items():
        need = spec.get("motion_frames", ATTACK_FRAMES)
        if len(tokens) != need:
            raise SystemExit("motion %s needs %d entries, has %d" % (name, need, len(tokens)))
        build(spec["export"], tokens, "motion " + name, roles).save(os.path.join(out, "모션_%s_sheet.png" % name))
    Image.new("RGBA", (416 * ATTACK_FRAMES, 256), (0, 0, 0, 0)).save(os.path.join(out, "공격_vfx_layer.png"))
    first, first_feet = load(spec["export"], spec["move"][0])
    place(first, anchor(first, first_feet)).save(os.path.join(out, "sprite.png"))
    with open(os.path.join(out, "motion_tips.json"), "w", encoding="utf-8") as f:
        json.dump(TIPS, f)
    with open(os.path.join(out, "frame_spec.json"), "w", encoding="utf-8") as f:
        json.dump(spec, f, ensure_ascii=False, indent=1)
    print("wrote", out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
