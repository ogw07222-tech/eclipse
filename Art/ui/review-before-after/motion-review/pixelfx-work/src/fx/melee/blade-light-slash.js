// The crescent, echo, trail, glint and spark recipe is adapted from Sprite Fusion's open-source slash
// (https://www.spritefusion.com/pixel-art-vfx/slash), MIT License, Copyright (c) 2026 Hugo Duprez.
import { Particles, P, Rng, fit, sparkle, clamp, smooth, ease, TAU } from "../../core/index.js";

export const meta = {
  name: "Blade light slash (Flash Executioner)",
  desc: "A huge continuous crescent of gold-white light with a hot leading edge, fine streaks, a lens glint and drifting light motes.",
  tags: ["melee", "slash", "sword", "light", "big", "directional", "one-shot"],
  size: [320, 240],
  origin: [120, 140],
  duration: 1.0,
  loop: false,
  fps: 30,
  bg: "ground",
  options: {
    palette: ["#3b2410", "#7a4a14", "#c07a1c", "#eeb43a", "#ffd978", "#fff0b8", "#ffffff"],
    sparks: ["#ffffff", "#fff0b8", "#ffd978", "#eeb43a"],
    radius: 78,
    seed: 11,
  },
};

// a point on the slash's tilted ellipse
function onArc(s, r, a) {
  const lx = Math.cos(a) * r, ly = Math.sin(a) * r * s.sy, c = Math.cos(s.rot), n = Math.sin(s.rot);
  return [s.cx + lx * c - ly * n, s.cy + lx * n + ly * c];
}

// the smear: thickest just behind the leading edge, tapering to a hair at the tail, white on the outer rim,
// cooling down the ramp with age and towards the tail; the tail dissolves through the dither
function crescent(p, s, ta, L, W, age) {
  if (Math.abs(L) < 0.02 || W < 0.3) return;
  const R = s.ramp, n = R.length, dir = Math.sign(L), span = Math.abs(L), c = Math.cos(s.rot), sn = Math.sin(s.rot), B = s.R + 2;
  p.shade(s.cx - B, s.cy - B, s.cx + B, s.cy + B, (x, y, out) => {
    const dx = x - s.cx, dy = y - s.cy, lx = dx * c + dy * sn, ly = (-dx * sn + dy * c) / s.sy, r = Math.hypot(lx, ly);
    if (r > s.R + 0.5 || r < s.R - W - 1) return;
    let u = (dir * (Math.atan2(ly, lx) - ta)) % TAU;
    if (u < 0) u += TAU;
    if (u > span) return;
    const v = u / span;
    // sharp at the tail, widest just behind the head: a blade-like wedge, no dithered fade
    const w = W * Math.pow(Math.sin(Math.PI * Math.pow(v, 1.6)), 0.55);
    if (w < 0.6) return;
    const q = (s.R - r) / w;
    if (q < 0 || q > 1) return;
    // hot leading rim, then gold, then amber toward the inner edge; fine curved streaks like flowing energy
    const streak = Math.sin(q * 13 + u * 1.9 + s.R * 0.01);
    let ci = q < 0.22 ? 0 : 1 + Math.round(((q - 0.22) / 0.78) * 3 + age * 1.3 - v * 0.9);
    if (ci > 0 && streak > 0.72) ci -= 1;
    out.c = R[n - 1 - clamp(ci, 0, n - 1)];
    out.a = 1;
  });
  for (let u = 0; u <= span; u += 0.35) {
    const [x, y] = onArc(s, s.R - W * 0.4, ta + dir * u);
    p.glow(x, y, 4 + W * 0.6, R[n - 3], 0.1 * (1 - age) * (u / span));
  }
}

function drawSlash(p, s, t) {
  const u = t - s.t0;
  if (u < 0) return;
  const span = s.a1 - s.a0, head = 1 - Math.pow(1 - clamp(u / s.sweep), 2), tail = smooth(s.sweep + s.hold, s.sweep + s.hold + s.trail, u);
  const age = clamp(u / (s.sweep + s.hold + s.trail));
  if (tail < 1) crescent(p, s, s.a0 + span * tail, span * (head - tail), s.W * (1 - tail * 0.5), age);
}

function tipOf(s) {
  const [x, y] = onArc(s, s.R - s.W * 0.3, s.a1), [x2, y2] = onArc(s, s.R - s.W * 0.3, s.a1 + Math.sign(s.a1 - s.a0) * 0.05);
  const l = Math.hypot(x2 - x, y2 - y) || 1;
  return { x, y, dx: (x2 - x) / l, dy: (y2 - y) / l };
}

export function create(o) {
  const C = fit(o.palette, 7), Rg = new Rng(o.seed), ps = new Particles({ rng: Rg, ground: () => 0, sparks: o.sparks });
  const R0 = o.radius, a0 = -2.75, a1 = 1.1;
  // continuous: gather glow, then one long sweep that spans the whole swing, then a slow taper
  const A = { cx: 0, cy: -8, R: R0, sy: 0.62, rot: -0.3, a0, a1, W: 34, t0: 0.06, sweep: 0.2, hold: 0.14, trail: 0.42, ramp: C };
  const E = { ...A, R: R0 * 0.8, W: 12, a0: a0 + (a1 - a0) * 0.06, a1: a1 - (a1 - a0) * 0.05, t0: 0.08, hold: 0.1, trail: 0.4, ramp: C.slice(0, 6) };
  const F = { ...A, R: R0 * 1.17, W: 9, a0: a0 + (a1 - a0) * 0.03, a1: a1 + 0.1, t0: 0.1, hold: 0.12, trail: 0.44, ramp: C.slice(0, 5) };
  const tip = tipOf(A);
  let t = 0, landed = false;
  return {
    update(dt) {
      t += dt;
      if (!landed && t >= A.t0 + A.sweep) {
        landed = true;
        // sparks off the edge where it stops, and bright shavings flung off the arc
        for (let i = 0; i < 9; i++) {
          const a = Math.atan2(tip.dy, tip.dx) + Rg.range(-0.5, 0.5), sp = Rg.range(150, 400);
          ps.spawn(P.SPARK, tip.x, tip.y, Math.cos(a) * sp, Math.sin(a) * sp, Rg.range(0.12, 0.35));
        }
        for (let i = 0; i < 40; i++) {
          const a = a0 + (a1 - a0) * Rg.range(0.2, 1), [x, y] = onArc(A, A.R, a);
          const ox = x - A.cx, oy = y - A.cy, l = Math.hypot(ox, oy) || 1, sp = Rg.range(30, 100);
          ps.spawn(P.MOTE, x, y, (ox / l) * sp, (oy / l) * sp - 34, Rg.range(0.4, 0.68), C[4 + Rg.int(0, 1)], 0.6);
        }
      }
      ps.update(dt);
    },
    draw(p) {
      drawSlash(p, F, t);
      drawSlash(p, E, t);
      drawSlash(p, A, t);
      // the tip glints as the edge arrives
      const g = t - (A.t0 + A.sweep * 0.7);
      if (g > 0 && g < 0.22) sparkle(p, tip.x, tip.y, 20 * ease.outBack(Math.min(1, g / 0.05)) * (1 - g / 0.22), C[5], C[3], 0.6);
      // light rides the head of the swing
      if (t >= A.t0 && t < A.t0 + 0.45) {
        const [x, y] = onArc(A, A.R - 4, A.a0 + (A.a1 - A.a0) * 1 - Math.pow(1 - clamp((t - A.t0) / A.sweep), 2));
        p.glow(x, y, 60, C[4], 0.32 * (1 - (t - A.t0) / 0.45));
      }
      // gather: light pools at the raised blade before the cut
      if (t < A.t0) {
        const [gx, gy] = onArc(A, A.R - 6, A.a0);
        const k = t / A.t0;
        p.glow(gx, gy, 14 + 22 * k, C[5], 0.3 + 0.4 * k);
        sparkle(p, gx, gy, 5 + 8 * k, C[6], C[4], 0.5);
      }
      ps.draw(p);
    },
  };
}
