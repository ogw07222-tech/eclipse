// The crescent, echo, trail, glint and spark recipe is adapted from Sprite Fusion's open-source slash
// (https://www.spritefusion.com/pixel-art-vfx/slash), MIT License, Copyright (c) 2026 Hugo Duprez.
import { Particles, P, Rng, fit, sparkle, clamp, smooth, ease, TAU } from "../../core/index.js";

export const meta = {
  name: "Blade aura (big, exaggerated)",
  desc: "A huge exaggerated violet-white crescent wave with a wide echo, a hot flash at the end and sparks, big enough to hide the blade and arm.",
  tags: ["melee", "slash", "sword", "aura", "big", "directional", "one-shot"],
  size: [320, 240],
  origin: [120, 140],
  duration: 0.9,
  loop: false,
  fps: 30,
  bg: "ground",
  options: {
    palette: ["#241046", "#4a1f8f", "#7a3bd6", "#b06cff", "#d9b6ff", "#f3e8ff", "#ffffff"],
    sparks: ["#ffffff", "#e8d4ff", "#b06cff", "#7a3bd6"],
    radius: 78,
    seed: 7,
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
    const v = u / span, w = W * Math.pow(Math.sin(Math.PI * Math.pow(v, 2.2)), 0.6);
    if (w < 0.35) return;
    const q = (s.R - r) / w;
    if (q < 0 || q > 1) return;
    // brightest step at the rim while it's young, then darker steps inward, with age and towards the tail
    const ci = q < 0.22 && age < 0.5 ? 0 : Math.round(q * (n - 1) * 0.75 + age * 2.4 - v * 0.8 + 0.4);
    out.c = R[n - 1 - clamp(ci, 0, n - 1)];
    out.a = Math.min(1, v / 0.4);
  });
  for (let u = 0; u <= span; u += 0.3) {
    const [x, y] = onArc(s, s.R - W * 0.4, ta + dir * u);
    p.glow(x, y, 5 + W, R[n - 3], 0.22 * (1 - age) * (u / span));
  }
}

function drawSlash(p, s, t) {
  const u = t - s.t0;
  if (u < 0) return;
  const span = s.a1 - s.a0, head = ease.outExpo(clamp(u / s.sweep)), tail = smooth(s.sweep + s.hold, s.sweep + s.hold + s.trail, u);
  const age = clamp(u / (s.sweep + s.hold + s.trail));
  if (tail < 1) crescent(p, s, s.a0 + span * tail, span * (head - tail), s.W * (1 - tail * 0.55), age);
  // a dotted trace of the blade's path lingers where the smear has already gone
  const th = 0.7 * (1 - clamp((u - s.sweep - s.hold) / (s.trail + 0.2)));
  if (tail > 0.02 && th > 0)
    for (let k = 0; k <= tail; k += 0.6 / (Math.abs(span) * s.R)) {
      const [x, y] = onArc(s, s.R - 0.5, s.a0 + span * k);
      p.dot(x, y, s.ramp[1], th * (0.4 + 0.6 * k));
    }
}

function tipOf(s) {
  const [x, y] = onArc(s, s.R - s.W * 0.3, s.a1), [x2, y2] = onArc(s, s.R - s.W * 0.3, s.a1 + Math.sign(s.a1 - s.a0) * 0.05);
  const l = Math.hypot(x2 - x, y2 - y) || 1;
  return { x, y, dx: (x2 - x) / l, dy: (y2 - y) / l };
}

export function create(o) {
  const C = fit(o.palette, 7), Rg = new Rng(o.seed), ps = new Particles({ rng: Rg, ground: () => 0, sparks: o.sparks });
  const R0 = o.radius, a0 = -2.75, a1 = 1.1;
  // the main smear, and a thinner echo just inside it a beat behind
  const A = { cx: 0, cy: -8, R: R0, sy: 0.62, rot: -0.3, a0, a1, W: 34, t0: 0, sweep: 0.09, hold: 0.12, trail: 0.4, ramp: C };
  const E = { ...A, R: R0 * 0.8, W: 14, a0: a0 + (a1 - a0) * 0.08, a1: a1 - (a1 - a0) * 0.05, t0: 0.03, hold: 0.08, trail: 0.34, ramp: C.slice(0, 6) };
  const F = { ...A, R: R0 * 1.18, W: 11, a0: a0 + (a1 - a0) * 0.04, a1: a1 + 0.12, t0: 0.05, hold: 0.1, trail: 0.42, ramp: C.slice(0, 5) };
  const tip = tipOf(A);
  let t = 0, landed = false;
  return {
    update(dt) {
      t += dt;
      if (!landed && t >= A.sweep) {
        landed = true;
        // sparks off the edge where it stops, and bright shavings flung off the arc
        for (let i = 0; i < 26; i++) {
          const a = Math.atan2(tip.dy, tip.dx) + Rg.range(-0.5, 0.5), sp = Rg.range(150, 400);
          ps.spawn(P.SPARK, tip.x, tip.y, Math.cos(a) * sp, Math.sin(a) * sp, Rg.range(0.12, 0.35));
        }
        for (let i = 0; i < 44; i++) {
          const a = a0 + (a1 - a0) * Rg.range(0.2, 1), [x, y] = onArc(A, A.R, a);
          const ox = x - A.cx, oy = y - A.cy, l = Math.hypot(ox, oy) || 1, sp = Rg.range(30, 100);
          ps.spawn(P.MOTE, x, y, (ox / l) * sp, (oy / l) * sp - 10, Rg.range(0.25, 0.55), C[4 + Rg.int(0, 1)], 0.6);
        }
      }
      ps.update(dt);
    },
    draw(p) {
      drawSlash(p, F, t);
      drawSlash(p, E, t);
      drawSlash(p, A, t);
      // the tip glints as the edge arrives
      const g = t - A.sweep * 0.6;
      if (g > 0 && g < 0.22) sparkle(p, tip.x, tip.y, 20 * ease.outBack(Math.min(1, g / 0.05)) * (1 - g / 0.22), C[5], C[3], 0.6);
      // light rides the head of the swing
      if (t < 0.3) {
        const [x, y] = onArc(A, A.R - 4, A.a0 + (A.a1 - A.a0) * ease.outExpo(clamp(t / A.sweep)));
        p.glow(x, y, 80, C[4], 0.7 * (1 - t / 0.3));
      }
      ps.draw(p);
    },
  };
}
