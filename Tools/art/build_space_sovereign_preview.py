#!/usr/bin/env python3
"""Generate timing-aware PREVIEW in-betweens for ECLIPSE Space Sovereign.

Non-destructive: writes to void_mage/preview_generated only. Source sheets remain
untouched. Optical-flow interpolation is an assisted draft, NOT authored artwork.
Requires Pillow, numpy, opencv-python (cv2). No tests or Actions are invoked.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[2] / "Assets/Resources/Eclipse/void_mage"
SOURCE_NAMES = ("character", "grimoire", "effects")
TARGET_COUNTS = {
    "idle": 8, "run": 24, "attack": 16, "flight": 12,
    "gravity_field": 12, "dimensional_spikes": 16,
    "repulsion": 16, "singularity": 48,
}
# Normalized key moments in playback time. Attack impact is deliberately quick;
# ability release occurs after anticipation; ultimate keeps its long build-up.
BEATS = {
    "idle": (0.0, .45, 1.0),
    "run": (0.0, .23, .53, .80),
    "attack": (0.0, .26, .39, 1.0),
    "flight": (0.0, .34, 1.0),
    "gravity_field": (0.0, .28, 1.0),
    "dimensional_spikes": (0.0, .34, 1.0),
    "repulsion": (0.0, .23, 1.0),
    "singularity": (0.0, .20, .50, 1.0),
}

def frames_from_sheet(path: Path, cell: tuple[int, int], count: int) -> list[np.ndarray]:
    with Image.open(path) as image:
        image = image.convert("RGBA")
        width, height = cell
        if image.width != width * 8 or image.height < height * math.ceil(count / 8):
            raise ValueError(f"{path}: incompatible dimensions {image.size}, cell {cell}")
        frames = []
        for i in range(count):
            x, y = (i % 8) * width, (i // 8) * height
            frames.append(np.array(image.crop((x, y, x + width, y + height)), dtype=np.uint8))
        return frames

def cv_flow(a: np.ndarray, b: np.ndarray, cv2):
    # Work on grayscale premultiplied color AND alpha to track both armor and cape.
    def signal(rgba):
        rgb = rgba[:, :, :3].astype(np.float32)
        alpha = rgba[:, :, 3:4].astype(np.float32) / 255.
        luminance = rgb[:, :, 0] * .299 + rgb[:, :, 1] * .587 + rgb[:, :, 2] * .114
        return np.uint8(np.clip(luminance * alpha[:, :, 0] * .65 + rgba[:, :, 3] * .35, 0, 255))
    return cv2.calcOpticalFlowFarneback(signal(a), signal(b), None,
                                        .5, 3, 15, 3, 5, 1.2, 0)

def warp(image: np.ndarray, flow: np.ndarray, amount: float, cv2):
    h, w = image.shape[:2]
    xx, yy = np.meshgrid(np.arange(w, dtype=np.float32),
                         np.arange(h, dtype=np.float32))
    coords_x = xx - flow[:, :, 0] * amount
    coords_y = yy - flow[:, :, 1] * amount
    return cv2.remap(image, coords_x, coords_y, cv2.INTER_NEAREST,
                     borderMode=cv2.BORDER_CONSTANT, borderValue=(0, 0, 0, 0))

def inbetween(a: np.ndarray, b: np.ndarray, fraction: float, cv2):
    if fraction <= 1e-7:
        return a.copy()
    if fraction >= 1 - 1e-7:
        return b.copy()
    ab = cv_flow(a, b, cv2)
    ba = cv_flow(b, a, cv2)
    left = warp(a, ab, fraction, cv2)
    right = warp(b, ba, 1.0 - fraction, cv2)
    # Hard pixel clusters. Choose a color from one original frame; never
    # introduce semi-transparent crossfade ghosts or blurred in-betweens.
    h, w = a.shape[:2]
    yy, xx = np.indices((h, w))
    dither = ((xx * 13 + yy * 7) % 16 + .5) / 16.
    pick_right = (fraction > dither)[:, :, None]
    result = np.where(pick_right, right, left).astype(np.uint8)
    missing_left = (left[:, :, 3] == 0) & (right[:, :, 3] > 0)
    missing_right = (right[:, :, 3] == 0) & (left[:, :, 3] > 0)
    result[missing_left] = right[missing_left]
    result[missing_right] = left[missing_right]
    result[:, :, 3] = np.where(result[:, :, 3] >= 128, 255, 0).astype(np.uint8)
    result[result[:, :, 3] == 0] = 0
    return result

def render_layer(keys: list[np.ndarray], frame_count: int,
                 beats: tuple[float, ...], is_loop: bool, cv2):
    output = []
    # The last pose of a looping run/idle is returned to the first at the end.
    # End pose is not duplicated in the loop to prevent a stutter.
    if is_loop:
        beats = tuple(i / len(keys) for i in range(len(keys) + 1))
        poses = keys + [keys[0]]
    else:
        poses = keys
    for i in range(frame_count):
        t = i / frame_count if is_loop else (i / (frame_count - 1))
        section = next((j for j in range(len(beats) - 1)
                        if t <= beats[j + 1]), len(beats) - 2)
        low, high = beats[section], beats[section + 1]
        fraction = max(0., min(1., (t - low) / (high - low)))
        output.append(inbetween(poses[section], poses[section + 1],
                                fraction, cv2))
    return output

def atlas(frames: list[np.ndarray], width: int, height: int, columns: int = 8):
    rows = math.ceil(len(frames) / columns)
    image = Image.new("RGBA", (width * columns, height * rows))
    for i, frame in enumerate(frames):
        image.paste(Image.fromarray(frame, "RGBA"),
                    ((i % columns) * width, (i // columns) * height))
    return image

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=ROOT)
    parser.add_argument("--output", type=Path, default=None)
    args = parser.parse_args()
    source = args.source
    out = args.output or source / "preview_generated"
    try:
        import cv2
    except ImportError as exc:
        raise SystemExit("Missing cv2. Install OpenCV to enable real pixel-flow draft interpolation; no fake duplicate frames were generated.") from exc
    manifest = json.loads((source / "manifest.json").read_text(encoding="utf-8"))
    clips = manifest["motions"]
    source_count = manifest["layout"]["frameCount"]
    layers = {}
    for name in SOURCE_NAMES:
        cell = tuple(manifest["cell"]["effects" if name == "effects" else "character"])
        layers[name] = frames_from_sheet(source / (name + ".png"), cell, source_count)
    out.mkdir(parents=True, exist_ok=True)
    new_clips = []
    rendered = {name: [] for name in SOURCE_NAMES}
    running_index = 0
    for clip in clips:
        name = clip["name"]
        count = TARGET_COUNTS[name]
        keys = slice(clip["start"], clip["start"] + clip["count"])
        for layer in SOURCE_NAMES:
            rendered[layer].extend(render_layer(layers[layer][keys], count,
                                 BEATS[name], name in ("idle", "run"), cv2))
        new_clips.append({
            "name": name, "start": running_index, "count": count,
            "duration": clip["duration"],
            "playback_fps": round(count / clip["duration"], 3),
            "playback": "distance_driven" if name == "run" else "time_driven",
            "original_key_pose_indices": list(range(clip["start"], clip["start"] + clip["count"])),
            "production_status": "optical_flow_preview_requires_manual_pixel_cleanup",
        })
        running_index += count
    for name in SOURCE_NAMES:
        w, h = tuple(manifest["cell"]["effects" if name == "effects" else "character"])
        atlas(rendered[name], w, h).save(out / (name + ".png"), optimize=True)
    metadata = {
        "id": "sovereign_of_space_motion_preview",
        "name": "공간의 지배자",
        "source": "void_mage/*.png",
        "status": "generated_preview_not_approved_production_art",
        "columns": 8, "frames": running_index,
        "cells": manifest["cell"], "pivots": manifest["pivot"],
        "clips": new_clips,
        "warnings": [
            "Inspect foot plants, cape, arms, book registration, and effect edges.",
            "Do not replace original PNGs or claim hand-authored poses.",
            "Sync gameplay release/contact events to the named beats before runtime use.",
            "Run motion is distance-driven in game: 24 frames / 0.7 sec is a base-speed reference only."
        ]
    }
    (out / "manifest.json").write_text(json.dumps(metadata, ensure_ascii=False, indent=2) + "\n",
                                        encoding="utf-8")
    print(f"Generated {running_index} synchronized preview cells per layer in {out}")

if __name__ == "__main__":
    main()
