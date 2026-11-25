#!/usr/bin/env python3
"""How much do the effects get in the way? Renders a take with and without effects and measures, per frame,
(a) the share of the whole screen the effects cover and (b) the share of the player's own silhouette they cover.
Usage (repo root): python3 Art/ui/review-before-after/motion-review/occlusion_metric.py TAKE_INDEX"""
import sys, copy
sys.path.insert(0, "Art/ui/review-before-after/motion-review")
import numpy as np
import ingame_sim as S

def metrics(idx, step=3):
    bg, props = S.build_background()
    tk = S.TAKES[idx]()
    plain = copy.copy(tk); plain.fx = []; plain.lances = []; plain.orbs = None; plain.hits = []
    rows = []
    for tick in range(0, int(tk.length * S.FPS), step):
        full = np.asarray(S.render(tk, tick, bg, props), dtype=np.int16)[40:]
        base = np.asarray(S.render(plain, tick, bg, props), dtype=np.int16)[40:]
        empty = np.asarray(bg.convert("RGB"), dtype=np.int16)[40:]
        # target sprite is in both renders, so the silhouette = anything that differs from the empty background
        sil = np.abs(base - empty).sum(axis=2) > 40
        changed = np.abs(full - base).sum(axis=2) > 90
        pl = tk.player(tick / S.FPS)
        # restrict the silhouette to the player's own box (the target is drawn far to the right)
        x0, x1 = int(pl["x"] - 110), int(pl["x"] + 110)
        mask = np.zeros_like(sil); mask[:, max(0, x0):x1] = True
        psil = sil & mask
        rows.append((tick / S.FPS, changed.mean() * 100, (changed & psil).sum() / max(1, psil.sum()) * 100 if psil.sum() > 200 else 0.0, changed.any()))
    return rows

if __name__ == "__main__":
    idx = int(sys.argv[1]); rows = metrics(idx)
    scr = [r[1] for r in rows]; occ = [r[2] for r in rows]; vis = sum(1 for r in rows if r[3]) * 3 / S.FPS
    print(f"take {idx}: screen covered by effects  peak {max(scr):.1f}%  mean {np.mean(scr):.1f}%")
    print(f"        player silhouette covered  peak {max(occ):.0f}%  mean {np.mean(occ):.0f}%")
    print(f"        seconds with any effect on screen: {vis:.2f} of {rows[-1][0]:.2f}")
