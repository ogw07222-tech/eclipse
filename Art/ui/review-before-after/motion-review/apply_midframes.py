#!/usr/bin/env python3
"""Insert already-made mid frames into an approved Flash Executioner actor sheet and sync its approval-JSON row.

Run from the repo root, then run Tools/art/import_flash_executioner.py to regenerate the header and copy the
sheet into the runtime folder. No effects are baked into the sheet: only actor frames go in.
Static checks only; no game build or test is run.
"""
import json, hashlib, copy, math, sys, shutil
from pathlib import Path
from PIL import Image

ROOT = Path.cwd()
J = ROOT / "Art/flash-executioner/review-approval.json"
BACKUP = ROOT / "Art/flash-executioner/pre-midframe-originals"


def sha(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()


def cells(sheet, cols, n):
    return [sheet.crop(((i % cols) * 192, (i // cols) * 192, (i % cols) * 192 + 192, (i // cols) * 192 + 192)) for i in range(n)]


def apply(role, insert_after, mid_frames, cols_new, contact_new, method, source_map_prefix):
    """role: 'Dash', 'Skill1' ...; insert_after: 0-based original frame index the mid frames follow."""
    d = json.loads(J.read_text(encoding="utf-8"))
    row = [r for r in d["actors"] if r["role"] == role][0]
    old = copy.deepcopy(row)
    sup = row.get("superseded_v2")
    if sup:  # re-applying: start again from the original geometry recorded the first time
        old.update({"columns": sup["columns"], "rows": sup["rows"], "count": sup["count"],
                    "contactFrame": sup["contactFrame"], "sha256": sup["sha256"],
                    "review": row["review"].split(" | 2026-11-22")[0]})
    name = Path(row["file"]).name
    target = ROOT / row["file"]
    BACKUP.mkdir(parents=True, exist_ok=True)
    keep = BACKUP / name
    if not keep.exists():
        shutil.copyfile(target, keep)  # first-ever original, never overwritten
    src = Image.open(keep).convert("RGBA")
    originals = cells(src, old["columns"], old["count"])
    frames = originals[: insert_after + 1] + [m.convert("RGBA") for m in mid_frames] + originals[insert_after + 1:]
    assert all(f.size == (192, 192) for f in frames)
    n = len(frames)
    rows = math.ceil(n / cols_new)
    sheet = Image.new("RGBA", (cols_new * 192, rows * 192), (0, 0, 0, 0))
    for i, f in enumerate(frames):
        sheet.alpha_composite(f, ((i % cols_new) * 192, (i // cols_new) * 192))
    sheet.save(target)
    bounds, uniq, empty = [], set(), []
    for i, f in enumerate(frames):
        bb = f.getchannel("A").getbbox()
        if bb is None:
            empty.append(i)
        else:
            bounds.append(list(bb))
            uniq.add(f.tobytes())
    k = len(mid_frames)
    row.update({
        "columns": cols_new, "rows": rows, "count": n, "contactFrame": contact_new, "bounds": bounds,
        "emptyFrames": empty, "uniquePixelFrames": len(uniq), "sha256": sha(target),
        "sourceFrameMap": list(range(insert_after + 1)) + [f"{source_map_prefix}:{j + 1}" for j in range(k)] + list(range(insert_after + 1, old["count"])),
        "status": "owner-approved for in-game use (2026-11-22); inserted mid frames are AI-generated candidates, not hand-authored; owner device verification pending",
        "midFrames": {"inserted": f"0-based frames {insert_after + 1}-{insert_after + k}, between original F{insert_after + 1} and F{insert_after + 2}",
                      "method": method, "hand_authored": False, "bakedEffects": False,
                      "timing": f"duration {old['duration']}s unchanged; contactFrame {old.get('contactFrame', -1)} -> {contact_new}"},
        "superseded_v2": {"columns": old["columns"], "rows": old["rows"], "count": old["count"],
                          "contactFrame": old.get("contactFrame", -1), "sha256": old["sha256"]},
    })
    row["review"] = old.get("review", "") + f" | 2026-11-22: owner approved {k} AI mid frames between F{insert_after + 1} and F{insert_after + 2} and in-game use; no baked effects; not device tested."
    J.write_text(json.dumps(d, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"{role}: {old['count']} -> {n} frames, sheet {sheet.size}, contactFrame {old.get('contactFrame', -1)} -> {contact_new}, unique {len(uniq)}/{n}, empty {empty}")


if __name__ == "__main__":
    print("import this module and call apply(...)")
