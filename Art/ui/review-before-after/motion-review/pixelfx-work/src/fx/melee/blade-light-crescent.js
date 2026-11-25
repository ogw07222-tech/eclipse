// ECLIPSE remix: a big light-sword wave built from several PixelFX recipes.
//  - the tilted-ellipse crescent, head glint and sparks are adapted from melee/sword-slash
//    (Sprite Fusion's open-source slash, MIT License, Copyright (c) 2026 Hugo Duprez)
//  - true-translucent feathered edges and a thin bright rim echo holy/divine-shield and melee/scythe-reap
//  - flowing light streaks come from fbm noise advected along the arc, like the scrolled noise in fire effects
//  - peeling wisps and rising light motes echo scythe-reap and holy/feather-burst
// Origin (0, 0) is the sword hand area. The wave faces right.
import { Rng, fit, sparkle, clamp, smooth, ease, TAU, blend, fbm, WHITE } from "../../core/index.js";

export const meta = {
  name: "Blade light crescent (Flash Executioner)",
  desc: "A slender crescent sword wave of gold-white light: crisp white rim, inner edge fading to nothing, needle tips, two trailing echo arcs and wisps.",
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

// one long arc, drawn all the way through; it does not need to match the sword's path
const ARC = { cx: 0, cy: -16, sy: 0.66, rot: -0.25, a0: -3.35, a1: 1.4 };
const T0 = 0.06, SWEEP = 0.22, HOLD = 0.16, TRAIL = 0.46;

function onArc(R, a, off = 0) {
  const rr = R + off, lx = Math.cos(a) * rr, ly = Math.sin(a) * rr * ARC.sy, c = Math.cos(ARC.rot), n = Math.sin(ARC.rot);
  return [ARC.cx + lx * c - ly * n, ARC.cy + lx * n + ly * c];
}

// one translucent crescent layer. R: radius, W: widest thickness, delay: seconds, gain: brightness, seed: noise seed
// fan: how far this arc's radius bulges away from the main rim at its fullest point. It is 0 at both needle tips,
// so every arc meets the others at the same two cusps, like the horns of a half-moon.
function crescent(p, C, t, R, W, delay, gain, seed, aScale = 1, fan = 0, tl = 0) {
  const u = t - T0 - delay;
  if (u < 0) return;
  const span = ARC.a1 - ARC.a0;
  const head = 1 - Math.pow(1 - clamp(u / SWEEP), 2);
  const tail = smooth(SWEEP + HOLD + tl, SWEEP + HOLD + TRAIL + tl, u);
  if (tail >= 1) return;
  const ha = ARC.a0 + span * head, ta = ARC.a0 + span * tail, L = ha - ta;
  if (L < 0.03) return;
  const age = clamp(u / (SWEEP + HOLD + TRAIL));
  const endFade = 1 - smooth(0.5, 0.97, tail);
  const c = Math.cos(ARC.rot), sn = Math.sin(ARC.rot), B = R + Math.abs(fan) + W * 0.4 + 8, wMax = W * (1 - tail * 0.7);
  p.each(ARC.cx - B, ARC.cy - B, ARC.cx + B, ARC.cy + B, (x, y, sx, sy) => {
    const dx = x - ARC.cx, dy = y - ARC.cy, lx = dx * c + dy * sn, ly = (-dx * sn + dy * c) / ARC.sy, r = Math.hypot(lx, ly);
    if (r > R + Math.max(fan, 0) + 3.2 || r < R - wMax - 4 + Math.min(fan, 0)) return;
    let a = Math.atan2(ly, lx) - ta;
    a = ((a % TAU) + TAU) % TAU;
    if (a > L) return;
    const v = a / L; // 0 at the tail, 1 at the head
    // blade-like wedge: needle-sharp at the tail, fullest just behind the head, a sharp point at the head
    const sw = Math.sin(Math.PI * Math.pow(v, 1.25));
    // thicker only in the middle: x1.5 at the fullest point, the same as before toward both needle tips
    const w = wMax * Math.pow(sw, 1.25) * (1 + 0.5 * sw * sw) + 0.0001;
    const q = (R + fan * Math.pow(sw, 1.25) - r) / w; // 0 outer rim .. 1 inner edge; the rim itself bulges by fan*sw
    // feathered edge: smooth alpha outside the wedge on both sides, true translucency
    let edge = 1;
    if (q < 0) edge = Math.pow(clamp(1 + q * w / 3.2), 2) * 0.55;
    else if (q > 1) edge = Math.pow(clamp(1 - (q - 1) * w / 4), 2) * 0.5;
    if (edge <= 0.01) return;
    const qc = clamp(q);
    // energy profile: hot just inside the rim, falling smoothly to the inner edge
    let I = (0.3 + 0.7 * Math.exp(-Math.pow((qc - 0.06) / 0.5, 2))) * (0.86 + 0.34 * v) * gain;
    // flowing streaks: noise advected along the arc, stretched along the flow
    const flow = fbm((a * R) / 34 - u * 26, qc * 8 + seed, seed, 2) * 0.5 + 0.5;
    I *= 1 + (0.88 + 0.24 * flow - 1) * (1 - 0.8 * qc); // flow streaks fade out toward the inner edge so it stays smooth
    // slow cool-down with age, brighter rim line
    I *= 1 - age * 0.35;
    if (qc < 0.07) I = Math.max(I, 1.05 * gain * (1 - age * 0.35));
    I = clamp(I, 0, 1.2);
    const col = blend(C, clamp(I * 1.12));
    const alpha = clamp(edge * (qc < 0.1 ? 1 : (1 - 0.62 * smooth(0.25, 1, qc))) * smooth(0, 0.1, v));
    p.blendPx(sx, sy, col, alpha * aScale * endFade);
    if (I > 0.5) p.add(sx, sy, col, (I - 0.5) * 0.3 * endFade);
  });
}

export function create(o) {
  const C = fit(o.palette, 7), Rg = new Rng(o.seed), R0 = o.radius;
  // debris flies out in every direction with drag and no gravity or floor (the sword wave is not tied to the ground)
  const bits = [];
  let t = 0;
  const emit = (kind, x, y, vx, vy, life, c) => bits.push({ kind, x, y, vx, vy, life, c, born: t });
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
  let landed = false, closed = false;
  return {
    update(dt) {
      t += dt;
      if (!landed && t >= T0 + SWEEP) {
        landed = true;
        const [tx, ty] = onArc(R0, tipAngle, -8);
        for (let i = 0; i < 8; i++) {
          const a = Rg.range(0, TAU), sp = Rg.range(120, 320);
          emit("spark", tx, ty, Math.cos(a) * sp, Math.sin(a) * sp, Rg.range(0.16, 0.34), Rg.pick([WHITE, C[6], C[5]]));
        }
        for (let i = 0; i < 46; i++) {
          const a = ARC.a0 + (ARC.a1 - ARC.a0) * Rg.range(0.15, 1), [x, y] = onArc(R0, a, Rg.range(-4, 10));
          const ox = x - ARC.cx, oy = y - ARC.cy, l = Math.hypot(ox, oy) || 1, sp = Rg.range(14, 60);
          emit("mote", x, y, (ox / l) * sp, (oy / l) * sp, Rg.range(0.4, 0.7), C[4 + Rg.int(0, 2)]);
        }
      }
      if (!closed && t >= T0 + SWEEP + HOLD + TRAIL * 0.5) {
        closed = true;
        for (let i = 0; i < 30; i++) {
          const a = ARC.a1 - Rg.range(0, 1.3), [x, y] = onArc(R0, a, -Rg.range(0, 22));
          const b = Rg.range(0, TAU);
          emit("mote", x, y, Math.cos(b) * 40, Math.sin(b) * 40, Rg.range(0.18, 0.33), C[4 + Rg.int(0, 2)]);
        }
      }
    },
    draw(p) {
      // gather: light pools on the raised blade before the cut
      if (t < T0) {
        const [gx, gy] = onArc(R0, ARC.a0, -6), k = t / T0;
        p.glow(gx, gy, 12 + 22 * k, C[5], 0.25 + 0.4 * k);
        sparkle(p, gx, gy, 4 + 9 * k, C[6], C[4], 0.5);
      }
      // wisps behind the body of the wave
      for (const w of wisps) {
        const f = (t - w.born) / w.life;
        if (f < 0 || f >= 1) continue;
        const steps = 22, a1 = w.a + 0.55 * f, a0 = a1 - w.len * (1 - 0.5 * f);
        for (let i = 0; i <= steps; i++) {
          const k = i / steps, a = a0 + (a1 - a0) * k;
          const sw = Math.sin(Math.PI * Math.pow(clamp((a - ARC.a0) / (ARC.a1 - ARC.a0)), 1.25));
          const [x, y] = onArc(R0, a, (w.off + w.lift * f * (0.4 + 0.6 * k)) * sw);
          p.blend(x, y, k > 0.85 ? C[6] : C[4], (1 - f) * 0.7 * Math.pow(k, 0.8));
        }
      }
      // two trailing echo arcs behind, then the main slender crescent on top
      crescent(p, C, t, R0, 10, 0.008, 0.7, 21, 0.4, 15, 0.008);
      crescent(p, C, t, R0, 14, 0.004, 0.85, 13, 0.55, 8, 0.004);
      crescent(p, C, t, R0, 38, 0, 1.0, 3, 1.0);
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
      for (const b of bits) {
        const age = t - b.born;
        if (age < 0 || age > b.life) continue;
        const k = b.kind === "spark" ? 3.4 : 2.0, f1 = (1 - Math.exp(-k * age)) / k, f0 = (1 - Math.exp(-k * Math.max(0, age - 0.035))) / k, fade = 1 - age / b.life;
        if (b.kind === "spark") p.line(b.x + b.vx * f0, b.y + b.vy * f0, b.x + b.vx * f1, b.y + b.vy * f1, b.c, fade > 0.3 ? 1 : fade / 0.3);
        else p.dot(b.x + b.vx * f1, b.y + b.vy * f1, b.c, fade);
      }
    },
  };
}
