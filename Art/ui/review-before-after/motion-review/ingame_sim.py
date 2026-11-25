#!/usr/bin/env python3
"""Training-ground preview of the Flash Executioner moves, drawn the way the engine draws them.

Sheet geometry, timing and offsets are read from native-engine/src/flash_executioner_assets.hpp, and the actor
frame / effect trigger rules follow native-engine/src/presentation.hpp and combat.hpp. Positions, speeds and
projectile paths that the engine decides at runtime are approximated from combat.hpp (see comments). This is a
preview, not the game: nothing is built or run.
Usage (repo root): python3 Art/ui/review-before-after/motion-review/ingame_sim.py OUT.mp4 [--poster TAKE]
"""
import math, re, subprocess, sys
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path.cwd()
RES = ROOT / "Assets/Resources/Eclipse"
W, H, FPS = 1280, 720, 60
FLOOR = 597
HDR = (ROOT / "native-engine/src/flash_executioner_assets.hpp").read_text()
FIELDS = ["file", "cellW", "cellH", "columns", "rows", "count", "pivotX", "pivotY", "scale", "duration", "contactFrame", "loop",
          "front", "flightFirst", "flightLast", "lead", "spawnAge", "nativeDir", "flightFps", "offX", "offY", "targetFeet", "fps", "startAge"]


def parse(table):
    blk = re.search(table + r"\{\{(.*?)\n\}\};", HDR, re.S).group(1)
    out = {}
    for body, role in re.findall(r"\{(\"[^\"]+\"[^{}]*)\}, // (\w+)", blk):
        vals = []
        for i, tok in enumerate([t.strip() for t in body.split(",")]):
            if i == 0:
                vals.append(tok.strip('"'))
            elif tok in ("true", "false"):
                vals.append(tok == "true")
            elif tok.endswith("f"):
                vals.append(float(tok[:-1]))
            else:
                vals.append(int(tok))
        out[role] = dict(zip(FIELDS, vals))
    return out


ACT = parse("flashActors")
FX = parse("flashEffects")
_img = {}


def sheet_img(sh):
    if sh["file"] not in _img:
        _img[sh["file"]] = Image.open(RES / sh["file"]).convert("RGBA")
    return _img[sh["file"]]


def paste(canvas, im, x, y):
    """alpha_composite with clipping (Pillow refuses negative offsets)."""
    x, y = int(x), int(y)
    sx0, sy0 = max(0, -x), max(0, -y)
    sx1, sy1 = min(im.width, canvas.width - x), min(im.height, canvas.height - y)
    if sx1 <= sx0 or sy1 <= sy0:
        return
    canvas.alpha_composite(im.crop((sx0, sy0, sx1, sy1)), (x + sx0, y + sy0))


def draw_sheet(canvas, sh, frame, x, y, facing, angle=0.0):
    """Mirror of Presentation::reviewedFlashSprite (offset/anchor/scale/rotation about the pivot)."""
    frame = max(0, min(frame, sh["count"] - 1))
    cw, ch, cols, sc = sh["cellW"], sh["cellH"], sh["columns"], sh["scale"]
    im = sheet_img(sh)
    cell = im.crop(((frame % cols) * cw, (frame // cols) * ch, (frame % cols) * cw + cw, (frame // cols) * ch + ch))
    x += sh["offX"] * facing
    y += sh["offY"]
    anchor = sh["pivotX"] if facing > 0 else cw - sh["pivotX"]
    if facing < 0:
        cell = cell.transpose(Image.FLIP_LEFT_RIGHT)
    cell = cell.resize((max(1, round(cw * sc)), max(1, round(ch * sc))), Image.NEAREST)
    px, py = anchor * sc, sh["pivotY"] * sc
    dx, dy = round(x - px), round(y - py)
    if angle:
        pad = max(cell.size)
        big = Image.new("RGBA", (cell.width + 2 * pad, cell.height + 2 * pad), (0, 0, 0, 0))
        big.paste(cell, (pad, pad))
        cell = big.rotate(-angle, center=(px + pad, py + pad), resample=Image.NEAREST)
        dx -= pad
        dy -= pad
    paste(canvas, cell, dx, dy)


def actor_frame(sh, age, duration, contact=-1.0):
    """Presentation::reviewedFlashFrame."""
    pos = max(0.0, min(age / max(.001, duration), 1.0)) * sh["count"]
    if sh["contactFrame"] >= 0 and 0 < contact < duration:
        pos = age / contact * sh["contactFrame"] if age < contact else sh["contactFrame"] + (age - contact) / (duration - contact) * (sh["count"] - sh["contactFrame"])
    return max(0, min(int(pos), sh["count"] - 1))


def loop_frame(sh, t):
    return int(((t % sh["duration"]) / sh["duration"]) * sh["count"])


# ---------------------------------------------------------------- background
def build_background():
    base = Image.new("RGBA", (W, H), (0, 0, 0, 255))
    for fn in ("sky/sky_gradient_moon_01.png", "clouds/clouds_01.png", "far/mountain_far_01.png", "mid/forest_mid_01.png", "near/ground_fixed_01.png"):
        im = Image.open(RES / "forest_v3" / fn).convert("RGBA")
        base.alpha_composite(im.crop((0, 0, W, H)))
    props = Image.open(RES / "forest_v3/near/near_props_fixed_01.png").convert("RGBA").crop((0, 0, W, H))
    base.alpha_composite(props)  # props sit behind the fighters so they never hide the target
    return base, Image.new("RGBA", (W, H), (0, 0, 0, 0))


# ---------------------------------------------------------------- target (a training dummy: any enemy sprite)
_t = Image.open(RES / "monsters/demon_castle/잿빛_보병/sprite.png").convert("RGBA")
TSC = 1.5
_tb = _t.getchannel("A").getbbox()
TARGET = _t.crop(_tb).transpose(Image.FLIP_LEFT_RIGHT).resize((round((_tb[2] - _tb[0]) * TSC), round((_tb[3] - _tb[1]) * TSC)), Image.NEAREST)


def draw_target(canvas, x, hit_age, lift=0):
    im = TARGET
    dx = 0
    if 0 <= hit_age < .16:
        k = 1 - hit_age / .16
        dx = round(math.sin(hit_age * 80) * 5 * k)
        white = Image.new("RGBA", im.size, (255, 255, 255, 255))
        white.putalpha(im.getchannel("A").point(lambda v: int(v * .6 * k)))
        im = Image.alpha_composite(im, white)
    paste(canvas, im, x - im.width / 2 + dx, FLOOR - im.height + 4 - lift)


# ---------------------------------------------------------------- helpers for the takes
class Take:
    def __init__(self, title, length):
        self.title, self.length = title, length
        self.fx = []        # (role, born, x_fn_or_xy, facing, follow, playback)
        self.hits = []      # target hit times
        self.lances = []    # dict(role,t0,start,delay,speed,verdict)
        self.player = None  # function t -> dict(x,y,facing,kind,...)
        self.target_x = 800
        self.lift = 0       # target hovers this many px above the floor
        self.orbs = None    # (t_from, t_to, [(dx,dy)...])


def fx_frame(sh, age):
    if sh["loop"]:
        return int((age % sh["duration"]) * sh["fps"])
    return min(int(age * sh["fps"]), sh["count"] - 1)


def player_idle(t, x, facing=1):
    return {"x": x, "y": FLOOR, "facing": facing, "role": "Idle", "frame": loop_frame(ACT["Idle"], t)}


# ---------------------------------------------------------------- the takes
def take_basic():
    tk = Take("BASIC ATTACK  (3 hits, the same weak burst every time)", 3.6)
    tk.target_x = 760
    starts = [0.6, 1.08, 1.56]
    px = 640

    def player(t):
        for s in starts:
            age = t - s
            if 0 <= age < .46:
                return {"x": px, "y": FLOOR, "facing": 1, "role": "Basic", "frame": actor_frame(ACT["Basic"], age, .46, .184)}
        return player_idle(t, px)
    tk.player = player
    for s in starts:
        tk.fx.append(("Slash", s + (.184 - FX["Slash"]["lead"]), None, 1, True, 0))
        tk.fx.append(("Impact", s + .184, (tk.target_x - 20, FLOOR - 38), 1, False, 0))
        tk.hits.append(s + .184)
    return tk


def take_dash_jump():
    tk = Take("DASH  /  JUMP  /  LAND", 3.4)
    tk.target_x = 1100
    td, tj, air = 0.5, 1.5, .7

    def player(t):
        x, y = 280.0, FLOOR
        if t >= td:
            x += min(max(t - td, 0), .15) * 1400            # dash travel (approximation of combat.hpp's dash)
        if t >= tj:
            u = max(0, min((t - tj) / air, 1))
            x += 140 * u
            y = FLOOR - 170 * 4 * u * (1 - u)
        if td <= t < td + .15:
            return {"x": x, "y": y, "facing": 1, "role": "Dash", "frame": actor_frame(ACT["Dash"], t - td, .15)}
        if tj <= t < tj + air:
            return {"x": x, "y": y, "facing": 1, "role": "Jump", "frame": min(int((t - tj) / ACT["Jump"]["duration"] * ACT["Jump"]["count"]), ACT["Jump"]["count"] - 1)}
        if tj + air <= t < tj + air + .12:
            return {"x": x, "y": y, "facing": 1, "role": "Land", "frame": actor_frame(ACT["Land"], t - tj - air, .12)}
        return player_idle(t, x)
    tk.player = player
    tk.fx.append(("Dash", td, None, 1, True, 0))
    tk.fx.append(("Jump", tj, (280 + 210, FLOOR), 1, False, 0))
    tk.fx.append(("Land", tj + air, (280 + 210 + 140, FLOOR), 1, False, 0))
    return tk


def take_rush():
    tk = Take("SKILL 1  RUSH", 2.6)
    tk.target_x = 500
    ts, x0 = 0.6, 240.0

    def px(t):
        age = max(0, min(t - ts, .5))
        return x0 + 1080 * max(0, min(age, .36) - .125)         # combat.hpp: 1080 px/s from contact to .36 s

    def player(t):
        age = t - ts
        if 0 <= age < .5:
            return {"x": px(t), "y": FLOOR, "facing": 1, "role": "Skill1", "frame": actor_frame(ACT["Skill1"], age, .5, .125)}
        return player_idle(t, px(t))
    tk.player = player
    tk.fx.append(("Rush", ts + .125 - FX["Rush"]["lead"], None, 1, True, 0))
    hit_age = .125 + (tk.target_x - 40 - x0) / 1080
    tk.fx.append(("ImpactRush", ts + hit_age, (tk.target_x, FLOOR - 38), 1, False, 0))
    tk.hits.append(ts + hit_age)
    return tk


def take_command():
    tk = Take("SKILL 2  COMMAND  (stored lances)", 3.2)
    tk.target_x = 900
    ts, x0 = 1.0, 330.0
    orbs = [(-52, -118), (-20, -146), (18, -146), (50, -118)]
    tk.orbs = (0.0, ts + .23, orbs)

    def player(t):
        age = t - ts
        if 0 <= age < .46:
            return {"x": x0, "y": FLOOR, "facing": 1, "role": "Skill2", "frame": actor_frame(ACT["Skill2"], age, .46, .23)}
        return player_idle(t, x0)
    tk.player = player
    tk.fx.append(("Command", ts + .23 - FX["Command"]["lead"], None, 1, True, 0))
    for i, (dx, dy) in enumerate(orbs):
        tk.lances.append({"role": "Lance", "t0": ts + .23, "start": (x0 + dx, FLOOR + dy), "delay": i * .05, "speed": 2100.0, "verdict": False,
                          "target": (tk.target_x, FLOOR - 62)})
    return tk


def take_blink():
    tk = Take("SKILL 3  BLINK  (to the marked target)", 3.0)
    tk.target_x = 960
    ts, x0 = 0.7, 300.0
    nx = tk.target_x + 58

    def player(t):
        age = t - ts
        if 0 <= age < .36:
            x, f = (x0, 1) if age < .15 else (nx, -1)
            return {"x": x, "y": FLOOR, "facing": f, "role": "Skill3", "frame": actor_frame(ACT["Skill3"], age, .36, .15)}
        if age >= .36:
            return player_idle(t, nx, -1)
        return player_idle(t, x0)
    tk.player = player
    tk.fx.append(("BlinkOut", ts, (x0, FLOOR), 1, False, 0))
    tk.fx.append(("BlinkIn", ts + .15, (nx, FLOOR), -1, False, 0))
    return tk


def take_swap():
    tk = Take("SWAP  (class change)", 1.8)
    tk.target_x = 1100
    ts, x0 = 0.5, 560.0

    def player(t):
        if ts <= t < ts + .24:
            return {"x": x0, "y": FLOOR, "facing": 1, "role": "Swap", "frame": actor_frame(ACT["Swap"], t - ts, .24)}
        return player_idle(t, x0)
    tk.player = player
    tk.fx.append(("Swap", ts, None, 1, True, 0))
    return tk


def take_verdict():
    tk = Take("ULTIMATE  VERDICT  (20 sky lances)", 4.2)
    tk.target_x = 880
    ts, x0 = 0.6, 430.0

    def player(t):
        age = t - ts
        if 0 <= age < 1.12:
            return {"x": x0, "y": FLOOR, "facing": 1, "role": "Ultimate", "frame": actor_frame(ACT["Ultimate"], age, 1.12, .728)}
        return player_idle(t, x0)
    tk.player = player
    tk.fx.append(("Verdict", ts + .30 - .20, None, 1, True, 0))     # presentation.hpp: trigger .30, source age +.20
    for i in range(20):
        tx = tk.target_x + (i - 9.5) * 18
        tk.lances.append({"role": "SkyLance", "t0": ts + .728, "start": (tx, -70 - (i % 5) * 42), "delay": i * .028, "speed": 2700.0, "verdict": True,
                          "target": (tk.target_x, FLOOR - 62)})
    return tk


def take_air():
    tk = Take("MID-AIR TARGET  (command lances, then verdict)", 6.6)
    tk.target_x, tk.lift = 820, 190
    ts, tu, x0 = 0.6, 2.9, 380.0
    tk.orbs = (0.0, ts + .23, [(-52, -118), (-20, -146), (18, -146), (50, -118)])
    ty = FLOOR - 62 - tk.lift

    def player(t):
        a1, a2 = t - ts, t - tu
        if 0 <= a1 < .46:
            return {"x": x0, "y": FLOOR, "facing": 1, "role": "Skill2", "frame": actor_frame(ACT["Skill2"], a1, .46, .23)}
        if 0 <= a2 < 1.12:
            return {"x": x0, "y": FLOOR, "facing": 1, "role": "Ultimate", "frame": actor_frame(ACT["Ultimate"], a2, 1.12, .728)}
        return player_idle(t, x0)
    tk.player = player
    tk.fx.append(("Command", ts + .23 - FX["Command"]["lead"], None, 1, True, 0))
    for i, (dx, dy) in enumerate([(-52, -118), (-20, -146), (18, -146), (50, -118)]):
        tk.lances.append({"role": "Lance", "t0": ts + .23, "start": (x0 + dx, FLOOR + dy), "delay": i * .05, "speed": 2100.0, "verdict": False, "target": (tk.target_x, ty)})
    tk.fx.append(("Verdict", tu + .30 - .20, None, 1, True, 0))
    for i in range(20):
        tx = tk.target_x + (i - 9.5) * 18
        tk.lances.append({"role": "SkyLance", "t0": tu + .728, "start": (tx, -70 - (i % 5) * 42), "delay": i * .028, "speed": 2700.0, "verdict": True, "target": (tk.target_x, ty)})
    return tk


TAKES = [take_basic, take_dash_jump, take_rush, take_command, take_blink, take_swap, take_verdict, take_air]


# ---------------------------------------------------------------- simulate lances (closed form per frame)
def lance_state(L, t):
    """Returns (x, y, angle_deg, arrived_time or None) following combat.hpp's seeking flight."""
    t_start = L["t0"] + L["delay"]
    if t < t_start:
        return None
    x, y = L["start"]
    tx, ty = L["target"]
    sp = L["speed"]
    dt = 1 / 240
    tt = t_start
    ang = math.degrees(math.atan2(ty - y, tx - x))
    life = .65 if not L["verdict"] else 2.0
    while tt < t:
        dx, dy = tx - x, ty - y
        d = math.hypot(dx, dy)
        ang = math.degrees(math.atan2(dy, dx))
        mv = sp * dt
        if d <= mv + 12:
            return (x, y, ang, tt)
        x += dx / d * mv
        y += dy / d * mv
        tt += dt
        if tt - t_start > life:
            return (x, y, ang, tt)
    return (x, y, ang, None)


def render(take, tick, bg, props):
    t = tick / FPS
    canvas = bg.copy()
    pl = take.player(t)
    # impacts / lance arrivals decide target hit time
    hit_age = 9.0
    events = list(take.fx)
    hits = list(take.hits)
    lances_draw = []
    for L in take.lances:
        st = lance_state(L, t)
        if st is None:
            continue
        x, y, ang, arr = st
        if arr is not None:
            ta = arr
            if not L["verdict"]:
                events.append(("ImpactLance", ta, (L["target"][0], L["target"][1]), 1, False, 0))
            if L["verdict"]:
                events.append(("LanceHit", ta, (L["target"][0], L["target"][1]), 1, False, 0))
            hits.append(ta)
        else:
            lances_draw.append((L["role"], x, y, ang, t - (L["t0"] + L["delay"])))
    for h in hits:
        if h <= t:
            hit_age = min(hit_age, t - h)
    draw_target(canvas, take.target_x, hit_age, take.lift)
    # effects behind the actor
    def run_fx(front_flag):
        seen = set()
        for role, born, pos, facing, follow, pb in events:
            sh = FX[role]
            if sh["front"] != front_flag:
                continue
            key = (role, round(born, 4), pos)
            if key in seen:
                continue
            seen.add(key)
            life = sh["duration"] if not sh["loop"] else 1e9
            age = t - born
            if age < 0 or age >= life:
                continue
            if follow:
                x, y = pl["x"], pl["y"]
            else:
                x, y = pos
            draw_sheet(canvas, sh, fx_frame(sh, age), x, y, facing)
    run_fx(False)
    # stored-lance orbs
    if take.orbs and take.orbs[0] <= t < take.orbs[1]:
        sh = FX["Orb"]
        for dx, dy in take.orbs[2]:
            draw_sheet(canvas, sh, fx_frame(sh, t), pl["x"] + dx, pl["y"] + dy, 1)
    # actor
    draw_sheet(canvas, ACT[pl["role"]], pl["frame"], pl["x"], pl["y"], pl["facing"])
    run_fx(True)
    for role, x, y, ang, age in lances_draw:
        sh = FX[role]
        draw_sheet(canvas, sh, fx_frame(sh, age), x, y, 1, ang - sh["nativeDir"])
    canvas.alpha_composite(props)
    img = canvas.convert("RGB")
    d = ImageDraw.Draw(img)
    d.rectangle((0, 0, W, 30), fill=(0, 0, 0))
    d.text((12, 9), f"{take.title}      t={t:4.2f}s", fill=(235, 235, 235))
    return img


def main():
    out = sys.argv[1]
    only = None
    if "--poster" in sys.argv:
        only = int(sys.argv[sys.argv.index("--poster") + 1])
    bg, props = build_background()
    takes = [mk() for mk in TAKES]
    if only is not None:
        tk = takes[only]
        ticks = [int(float(s) * FPS) for s in sys.argv[sys.argv.index("--poster") + 2:]]
        frames = [render(tk, k, bg, props) for k in ticks]
        sheet = Image.new("RGB", (W, H * len(frames) // 2 if len(frames) > 1 else H))
        for i, f in enumerate(frames):
            sheet.paste(f.resize((W, H // 2)) if len(frames) > 1 else f, (0, i * H // 2 if len(frames) > 1 else 0))
        sheet.save(out)
        return
    ff = subprocess.Popen(["ffmpeg", "-y", "-loglevel", "error", "-f", "rawvideo", "-pix_fmt", "rgb24", "-s", f"{W}x{H}", "-r", str(FPS), "-i", "-",
                           "-vf", "format=yuv420p", "-c:v", "libx264", "-crf", "20", out], stdin=subprocess.PIPE)
    total = 0
    for tk in takes:
        n = int(tk.length * FPS)
        for k in range(n):
            ff.stdin.write(render(tk, k, bg, props).tobytes())
        total += n
        print(f"{tk.title}: {n} frames", flush=True)
    ff.stdin.close()
    ff.wait()
    print("done", total, "frames", round(total / FPS, 1), "s")


if __name__ == "__main__":
    main()
