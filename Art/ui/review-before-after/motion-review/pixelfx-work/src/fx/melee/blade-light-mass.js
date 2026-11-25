// ECLIPSE remix: a big light-sword wave built from several PixelFX recipes.
//  - the tilted-ellipse crescent, head glint and sparks are adapted from melee/sword-slash
//    (Sprite Fusion's open-source slash, MIT License, Copyright (c) 2026 Hugo Duprez)
//  - true-translucent feathered edges and a thin bright rim echo holy/divine-shield and melee/scythe-reap
//  - flowing light streaks come from fbm noise advected along the arc, like the scrolled noise in fire effects
//  - peeling wisps and rising light motes echo scythe-reap and holy/feather-burst
// Origin (0, 0) is the sword hand area. The wave faces right.
import { Particles, P, Rng, fit, sparkle, clamp, smooth, ease, TAU, blend, fbm } from "../../core/index.js";

export const meta = {
  name: "Blade light mass (Flash Executioner)",
  desc: "One huge solid crescent of gold-white light with a hot white core, a smooth gradient to gold edges, a soft glow and drifting motes.",
  tags: ["melee", "slash", "sword", "light", "big", "directional", "one-shot"],
  size: [320, 240],
  origin: [120, 140],
  duration: 1.05,
  loop: false,
  fps: 30,
  bg: "ground",
  options: {
    palette: ["#5a3a14", "#9a6a1c", "#d9a030", "#f6cf62", "#ffe9a0", "#fff7d8", "#ffffff"],
    sparks: ["#ffffff", "#fff7d8", "#ffe9a0", "#f6cf62"],
    radius: 78,
    seed: 17,
  },
};

const ARC = { cx: 0, cy: -8, sy: 0.62, rot: -0.3, a0: -2.75, a1: 1.1 };
const T0 = 0.06, SWEEP = 0.22, HOLD = 0.16, TRAIL = 0.46;

function onArc(R, a, off = 0) {
  const rr = R + off, lx = Math.cos(a) * rr, ly = Math.sin(a) * rr * ARC.sy, c = Math.cos(ARC.rot), n = Math.sin(ARC.rot);
  return [ARC.cx + lx * c - ly * n, ARC.cy + lx * n + ly * c];
}

// one translucent crescent layer. R: radius, W: widest thickness, delay: seconds, gain: brightness, seed: noise seed
function crescent(p, C, t, R, W, delay, gain, seed) {
  const u = t - T0 - delay;
  if (u < 0) return;
  const span = ARC.a1 - ARC.a0;
  const head = 1 - Math.pow(1 - clamp(u / SWEEP), 2);
  const tail = smooth(SWEEP + HOLD, SWEEP + HOLD + TRAIL, u);
  if (tail >= 1) return;
  const ha = ARC.a0 + span * head, ta = ARC.a0 + span * tail, L = ha - ta;
  if (L < 0.03) return;
  const age = clamp(u / (SWEEP + HOLD + TRAIL));
  const c = Math.cos(ARC.rot), sn = Math.sin(ARC.rot), B = R + W * 0.4 + 8, wMax = W * (1 - tail * 0.45);
  p.each(ARC.cx - B, ARC.cy - B, ARC.cx + B, ARC.cy + B, (x, y, sx, sy) => {
    const dx = x - ARC.cx, dy = y - ARC.cy, lx = dx * c + dy * sn, ly = (-dx * sn + dy * c) / ARC.sy, r = Math.hypot(lx, ly);
    if (r > R + 3.2 || r < R - wMax - 4) return;
    let a = Math.atan2(ly, lx) - ta;
    a = ((a % TAU) + TAU) % TAU;
    if (a > L) return;
    const v = a / L; // 0 at the tail, 1 at the head
    // blade-like wedge: needle-sharp at the tail, fullest just behind the head, a sharp point at the head
    const w = (wMax * Math.pow(v, 0.8) * Math.pow(1 - v, 0.6)) / 0.384 + 0.0001;
    const q = (R - r) / w; // 0 outer rim .. 1 inner edge
    // feathered edge: smooth alpha outside the wedge on both sides, true translucency
    let edge = 1;
    if (q < 0) edge = Math.pow(clamp(1 + q * w / 3.2), 2) * 0.55;
    else if (q > 1) edge = Math.pow(clamp(1 - (q - 1) * w / 4), 2) * 0.5;
    if (edge <= 0.01) return;
    const qc = clamp(q);
    // energy profile: hot just inside the rim, falling smoothly to the inner edge
    let I = (0.32 + 0.68 * Math.exp(-Math.pow((qc - 0.1) / 0.8, 2))) * (0.86 + 0.34 * v) * gain;
    // flowing streaks: noise advected along the arc, stretched along the flow
    const flow = fbm((a * R) / 60 - u * 14, qc * 3 + seed, seed, 2) * 0.5 + 0.5;
    I *= 0.95 + 0.1 * flow;
    // slow cool-down with age, brighter rim line
    I *= 1 - age * 0.35;
    if (qc < 0.07) I = Math.max(I, 1.05 * gain);
    if (q < 0 || q > 1) I = Math.max(I, 0.55);
    I = clamp(I, 0, 1.2);
    const col = blend(C, clamp(I * 1.12));
    const alpha = clamp(edge * (0.9 + 0.3 * I) * smooth(0, 0.12, v));
    p.blendPx(sx, sy, col, alpha);
    if (I > 0.5) p.add(sx, sy, col, (I - 0.5) * 0.28);
  });
}

export function create(o) {
  const C = fit(o.palette, 7), Rg = new Rng(o.seed), R0 = o.radius;
  const ps = new Particles({ rng: Rg, ground: () => 0, sparks: o.sparks });
  // peeling wisps: thin curved threads that leave the outer edge and drift away along the swing
  const wisps = [];
  for (let i = 0; i < 16; i++) {
    const f = Rg.range(0.25, 1);
    wisps.push({
      born: T0 + SWEEP * f * 0.9, life: Rg.range(0.28, 0.55), a: ARC.a0 + (ARC.a1 - ARC.a0) * f, len: Rg.range(0.35, 0.95),
      off: Rg.range(-2, 16), lift: Rg.range(6, 18),
    });
  }
  const tipAngle = ARC.a1;
  let t = 0, landed = false;
  return {
    update(dt) {
      t += dt;
      if (!landed && t >= T0 + SWEEP) {
        landed = true;
        const [tx, ty] = onArc(R0, tipAngle, -8);
        for (let i = 0; i < 8; i++) {
          const a = Rg.range(-0.9, 0.7), sp = Rg.range(120, 320);
          ps.spawn(P.SPARK, tx, ty, Math.cos(a) * sp, Math.sin(a) * sp, Rg.range(0.12, 0.3));
        }
        for (let i = 0; i < 46; i++) {
          const a = ARC.a0 + (ARC.a1 - ARC.a0) * Rg.range(0.15, 1), [x, y] = onArc(R0, a, Rg.range(-4, 10));
          const ox = x - ARC.cx, oy = y - ARC.cy, l = Math.hypot(ox, oy) || 1, sp = Rg.range(14, 60);
          ps.spawn(P.MOTE, x, y, (ox / l) * sp, (oy / l) * sp - 26, Rg.range(0.4, 0.7), C[4 + Rg.int(0, 2)], 0.6);
        }
      }
      ps.update(dt);
    },
    draw(p) {
      // gather: light pools on the raised blade before the cut
      if (t < T0) {
        const [gx, gy] = onArc(R0, ARC.a0, -6), k = t / T0;
        p.glow(gx, gy, 12 + 22 * k, C[5], 0.25 + 0.4 * k);
        sparkle(p, gx, gy, 4 + 9 * k, C[6], C[4], 0.5);
      }
      // one solid body, with only a faint wide halo behind it (no echo layers, no strands)
      crescent(p, C, t, R0 * 1.0, 56, 0, 1.0, 3, 1.0);
      // light rides the head, with a cross flare when the edge lands
      const u = t - T0;
      if (u >= 0 && u < 0.6) {
        const head = 1 - Math.pow(1 - clamp(u / SWEEP), 2);
        const [x, y] = onArc(R0, ARC.a0 + (ARC.a1 - ARC.a0) * head, -4);
        p.glow(x, y, 56, C[4], 0.3 * (1 - u / 0.6));
      }
      const g = t - (T0 + SWEEP * 0.75);
      if (g > 0 && g < 0.26) {
        const [tx, ty] = onArc(R0, tipAngle, -6);
        sparkle(p, tx, ty, 22 * ease.outBack(Math.min(1, g / 0.06)) * (1 - g / 0.26), C[6], C[4], 0.6);
      }
      ps.draw(p);
    },
  };
}
