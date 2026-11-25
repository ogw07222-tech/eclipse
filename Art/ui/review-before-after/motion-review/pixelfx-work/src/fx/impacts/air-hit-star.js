// ECLIPSE custom hit variant D: a sharp eight-point star of light. Mid-air burst, no floor, no spark streaks.
import { Lights, Rng, fit, sparkle, clamp, smooth, ease, TAU, WHITE } from "../../core/index.js";
import { GOLD } from "../holy/_lib.js";

export const meta = {
  name: "Air hit D (eight-point star)",
  desc: "A hit that bursts as a sharp eight-point star of light that pulses and collapses, with a soft bloom, glints and drifting motes.",
  tags: ["impact", "hit", "light", "air", "omnidirectional", "variant", "one-shot"],
  size: [240, 240], origin: [120, 120], duration: 0.5, loop: false, fps: 30, bg: "dark",
  options: { palette: GOLD, seed: 23 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), lights = new Lights();
  lights.add(0, 0, 64, C[5], 0.85, 0.45);
  lights.add(0, 0, 22, WHITE, 0.9, 0.1);
  const tilt = R.range(-0.2, 0.2);
  const spikes = Array.from({ length: 8 }, (_, i) => ({ a: tilt + (i / 8) * TAU, len: i % 2 ? 34 : 72, w: i % 2 ? 4.5 : 6 }));
  const motes = Array.from({ length: 10 }, (_, i) => ({ a: tilt + ((i + 0.3) / 10) * TAU, v: R.range(35, 90), life: R.range(0.26, 0.44), delay: R.range(0.05, 0.12), c: R.pick([C[6], C[5], WHITE]), r: R.range(1.6, 2.4) }));
  const glints = Array.from({ length: 4 }, (_, i) => { const a = tilt + ((i + 0.5) / 4) * TAU, d = R.range(26, 44); return { x: Math.cos(a) * d, y: Math.sin(a) * d, delay: 0.08 + i * 0.025, life: 0.24, s: R.range(6, 9), c: i % 2 ? WHITE : C[6] }; });
  let t = 0;
  return {
    update(dt) { t += dt; lights.update(dt); },
    draw(p) {
      const f = clamp(t / 0.12);
      if (f < 1) {
        p.disc(0, 0, 6 + 9 * ease.outCubic(f), (d) => (d < 0.55 ? WHITE : C[6]), 1 - f * 0.7);
        p.glow(0, 0, 50 * (1 - f * 0.3), WHITE, 0.9 * (1 - f));
      }
      // the star slams in, overshoots, then collapses to its centre
      const u = clamp((t - 0.01) / 0.3);
      if (t >= 0.01 && u < 1) {
        const grow = u < 0.25 ? ease.outCubic(u / 0.25) * 1.12 : 1.12 - 0.12 * smooth(0.25, 0.4, u), shrink = 1 - smooth(0.45, 1, u), k = grow * shrink, fade = 1 - smooth(0.55, 1, u);
        for (const s of spikes) {
          const L = s.len * k, W = s.w * (0.4 + 0.6 * shrink), cx = Math.cos(s.a), cy = Math.sin(s.a), nx = -cy, ny = cx;
          if (L < 2) continue;
          p.tri(nx * W, ny * W, -nx * W, -ny * W, cx * L, cy * L, C[5], fade);
          p.tri(nx * W * 0.55, ny * W * 0.55, -nx * W * 0.55, -ny * W * 0.55, cx * L * 0.9, cy * L * 0.9, C[6], fade);
          p.tri(nx * W * 0.28, ny * W * 0.28, -nx * W * 0.28, -ny * W * 0.28, cx * L * 0.72, cy * L * 0.72, WHITE, fade);
          p.glowLine(0, 0, cx * L, cy * L, W * 1.6, C[5], 0.45 * fade);
        }
        p.glowRing(0, 0, 16 + 22 * ease.outCubic(u), 7, C[5], 0.4 * Math.pow(1 - u, 2));
      }
      for (const g of glints) {
        const a = t - g.delay;
        if (a >= 0 && a < g.life) sparkle(p, g.x, g.y, g.s * Math.sin((a / g.life) * Math.PI), g.c, C[5], 0.55);
      }
      for (const m of motes) {
        const a = t - m.delay;
        if (a < 0 || a > m.life) continue;
        const d = (m.v * (1 - Math.exp(-2 * a))) / 2, k = 1 - a / m.life;
        p.glow(Math.cos(m.a) * d, Math.sin(m.a) * d, m.r + 1.4, m.c, 0.5 * k);
        p.dot(Math.cos(m.a) * d, Math.sin(m.a) * d, WHITE, k);
      }
      lights.draw(p);
    },
  };
}
