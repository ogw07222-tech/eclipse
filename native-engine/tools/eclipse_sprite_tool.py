#!/usr/bin/env python3
"""ECLIPSE sprite-sheet tool: offline, headless, ChatGPT-friendly.

Requires Pillow; FFmpeg is optional (MP4 only). No browser, network or GUI.
"""
import argparse
import json
import math
import shutil
import subprocess
import sys
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw, ImageFont


def parse_rgb(value):
    if value is None:
        return None
    s = value.lstrip('#')
    if len(s) != 6:
        raise argparse.ArgumentTypeError('Use hex RGB, e.g. #15151c')
    try:
        return tuple(int(s[i:i+2], 16) for i in (0, 2, 4))
    except ValueError as e:
        raise argparse.ArgumentTypeError('Invalid RGB') from e


def extract(args):
    src = Image.open(args.input).convert('RGBA')
    fw, fh = args.width, args.height
    left, top, spacing = args.margin_x, args.margin_y, args.spacing
    cols = (src.width - left * 2 + spacing) // (fw + spacing)
    rows = (src.height - top * 2 + spacing) // (fh + spacing)
    if cols < 1 or rows < 1:
        raise ValueError(f'Image {src.width}x{src.height} cannot fit {fw}x{fh} frames with provided margins/spacing')
    if args.strict:
        target_w = left * 2 + cols * fw + (cols - 1) * spacing
        target_h = top * 2 + rows * fh + (rows - 1) * spacing
        if (target_w, target_h) != src.size:
            raise ValueError(f'Uneven sheet: {src.size}, grid uses {(target_w,target_h)}. Check frame dimensions')
    boxes = []
    for row in range(rows):
        for col in range(cols):
            x, y = left + col * (fw + spacing), top + row * (fh + spacing)
            boxes.append((x, y, x + fw, y + fh))
    if args.order == 'column':
        boxes = sorted(boxes, key=lambda b: (b[0], b[1]))
    stop = args.end if args.end is not None else len(boxes)
    if not (0 <= args.start < stop <= len(boxes)):
        raise ValueError(f'Invalid frame range [{args.start}, {stop}); sheet contains {len(boxes)} frames')
    return src, [src.crop(b) for b in boxes[args.start:stop]], cols, rows


def metrics(frames):
    values = []
    prev = None
    for i, im in enumerate(frames):
        alpha = im.getchannel('A')
        bbox = alpha.getbbox()
        coverage = round(sum(1 for v in alpha.getdata() if v > 0) / (im.width * im.height), 4)
        center = None if bbox is None else [round((bbox[0] + bbox[2])/2, 2), round((bbox[1] + bbox[3])/2, 2)]
        item = {'frame': i, 'bbox': list(bbox) if bbox else None, 'center': center, 'opaque_coverage': coverage}
        if prev is not None:
            delta = ImageChops.difference(im, prev).convert('RGBA')
            # RGBA channel difference including alpha. Not a semantic motion score.
            hist = delta.histogram()
            means = [sum(j * count for j,count in enumerate(hist[c*256:(c+1)*256]))/(im.width*im.height*255) for c in range(4)]
            item['pixel_change_fraction_est'] = round(sum(means) / 4, 4)
        values.append(item)
        prev = im
    return values


def composite(im, bg):
    if bg is None:
        return im.copy()
    base = Image.new('RGBA', im.size, bg + (255,))
    base.alpha_composite(im)
    return base


def expanded_frames(frames, scale, bg):
    return [composite(im, bg).resize((im.width * scale, im.height * scale), Image.Resampling.NEAREST) for im in frames]


def write_contact(frames, out, scale, columns, bg):
    cellw, cellh = frames[0].size
    w, h = cellw * scale, cellh * scale
    pad, labelh = 12, 22
    ncol = min(columns, len(frames))
    nrow = math.ceil(len(frames) / ncol)
    contact_bg = (30, 31, 40, 255)
    canvas = Image.new('RGBA', (pad + ncol * (w + pad), pad + nrow * (h + labelh + pad)), contact_bg)
    draw = ImageDraw.Draw(canvas)
    for i, frame in enumerate(expanded_frames(frames, scale, bg)):
        x = pad + (i % ncol) * (w + pad)
        y = pad + (i // ncol) * (h + labelh + pad)
        canvas.alpha_composite(frame, (x, y))
        draw.text((x, y+h+3), f'Frame {i:03d}', fill=(230, 230, 235, 255))
    canvas.convert('RGB').save(out)


def write_gif(frames, out, scale, fps, loops, bg):
    images = expanded_frames(frames, scale, bg)
    duration_ms = round(1000/fps)
    images[0].save(out, save_all=True, append_images=images[1:], duration=duration_ms,
                   loop=loops, disposal=2, optimize=False)
    return duration_ms


def write_mp4(frames, out, scale, fps, loops, bg):
    exe = shutil.which('ffmpeg')
    if exe is None:
        raise RuntimeError('FFmpeg is required for MP4 output; GIF works without it')
    images = expanded_frames(frames, scale, bg or (20,20,28))
    # libx264 with yuv420p needs even dimensions.
    w, h = images[0].size
    if (w % 2) or (h % 2):
        w, h = w + (w % 2), h + (h % 2)
        corrected = []
        for im in images:
            canvas = Image.new('RGBA', (w,h), (20,20,28,255))
            canvas.alpha_composite(im, (0,0))
            corrected.append(canvas)
        images = corrected
    command = [exe, '-loglevel', 'error', '-y', '-f', 'rawvideo', '-pix_fmt', 'rgba',
               '-s', f'{w}x{h}', '-r', str(fps), '-i', '-', '-c:v', 'libx264',
               '-pix_fmt', 'yuv420p', '-movflags', '+faststart', str(out)]
    proc = subprocess.Popen(command, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        for frame in images * loops:
            proc.stdin.write(frame.tobytes())
        proc.stdin.close()
        stderr = proc.stderr.read().decode('utf-8', errors='replace')
        rc = proc.wait()
        if rc:
            raise RuntimeError('FFmpeg error: ' + stderr[-1200:])
    except BrokenPipeError:
        stderr = proc.stderr.read().decode('utf-8', errors='replace')
        proc.wait()
        raise RuntimeError('FFmpeg pipe error: ' + stderr[-1200:])


def main():
    p = argparse.ArgumentParser(description='Headless PNG sprite-sheet playback and inspection')
    p.add_argument('input', type=Path, help='Sprite sheet PNG')
    p.add_argument('--width', '-W', type=int, required=True, help='Frame width in pixels')
    p.add_argument('--height', '-H', type=int, required=True, help='Frame height in pixels')
    p.add_argument('--fps', type=float, default=24, help='Animation frames per second (default: 24)')
    p.add_argument('--start', type=int, default=0, help='Zero-based first frame (inclusive)')
    p.add_argument('--end', type=int, help='Zero-based end frame (exclusive)')
    p.add_argument('--order', choices=['row','column'], default='row', help='Sheet reading direction')
    p.add_argument('--margin-x', type=int, default=0)
    p.add_argument('--margin-y', type=int, default=0)
    p.add_argument('--spacing', type=int, default=0)
    p.add_argument('--scale', type=int, default=4, help='Nearest-neighbor zoom, default 4')
    p.add_argument('--background', type=parse_rgb, default=None, help='Optional #RRGGBB solid background')
    p.add_argument('--strict', action='store_true', help='Reject incomplete edge cells')
    p.add_argument('--loop-count', type=int, default=3, help='MP4 repeat count; GIF loop=0 is infinite')
    p.add_argument('--output', type=Path, default=Path('sprite_output'), help='Output directory')
    p.add_argument('--format', choices=['all','gif','mp4','report'], default='all')
    p.add_argument('--columns', type=int, default=6, help='Contact sheet columns')
    args = p.parse_args()
    if args.width <= 0 or args.height <= 0 or args.fps <= 0 or args.scale < 1 or args.loop_count < 1 or args.columns < 1:
        p.error('Frame size, FPS, scale, loop-count and columns must be positive')
    src, frames, cols, rows = extract(args)
    args.output.mkdir(parents=True, exist_ok=True)
    report = {'source': str(args.input), 'sheet_size': list(src.size), 'grid': {'columns':cols,'rows':rows},
              'frame_size':[args.width,args.height], 'frame_range':[args.start,args.start+len(frames)],
              'frame_count':len(frames), 'fps':args.fps, 'duration_seconds':round(len(frames)/args.fps,4),
              'note':'Pixel-change metric is a rough image difference, not a motion-quality verdict.',
              'frames':metrics(frames)}
    prefix = args.input.stem
    json_path = args.output / (prefix + '_report.json')
    json_path.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    contact_path = args.output / (prefix + '_contact.png')
    write_contact(frames, contact_path, args.scale, args.columns, args.background)
    outputs = [json_path, contact_path]
    if args.format in ('gif','all'):
        path = args.output / (prefix + '.gif')
        write_gif(frames, path, args.scale, args.fps, 0, args.background)
        outputs.append(path)
    if args.format in ('mp4','all'):
        path = args.output / (prefix + '.mp4')
        write_mp4(frames, path, args.scale, args.fps, args.loop_count, args.background)
        outputs.append(path)
    print(f'Frames: {len(frames)} | Grid: {cols}x{rows} | FPS: {args.fps} | Cycle: {len(frames)/args.fps:.3f}s')
    for path in outputs:
        print(path.resolve())


if __name__ == '__main__':
    try:
        main()
    except (ValueError, RuntimeError, OSError) as e:
        print(f'ERROR: {e}', file=sys.stderr)
        sys.exit(1)
