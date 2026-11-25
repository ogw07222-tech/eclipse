// ECLIPSE custom hit for the Rush skill: the strongest non-ultimate hit. A long eight-point star of light with a heavy bloom,
// two shock rings and a ring of glints. Mid-air burst, no floor, no spark streaks.
import { Lights, Rng, fit, sparkle, clamp, smooth, ease, TAU, WHITE } from "../../core/index.js";
import { GOLD } from "../holy/_lib.js";

export const meta = {
  name: "Air hit (rush, heavy star)",
  desc: "A heavy hit: a long eight-point star of light slams in with a big bloom and two shock rings, then collapses into glints and drifting motes.",
  tags: ["impact", "hit", "light", "air", "omnidirectional", "heavy", "one-shot"],
  size: [280, 280], origin: [140, 140], duration: 0.65, loop: false, fps: 30, bg: "dark",
  options: { palette: GOLD, seed: 31 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), lights = new Lights();
  lights.add(0, 0, 84, C[5], 1, 0.5);
  lights.add(0, 0, 30, WHITE, 0.95, 0.12);
  const tilt = R.range(-0.15, 0.15);
  const spikes = Array.from({ length: 8 }, (_, i) => ({ a: tilt + (i / 8) * TAU, len: i % 2 ? 44 : 92, w: i % 2 ? 5.5 : 7.5 }));
  const motes = Array.from({ length: 16 }, (_, i) => ({ a: tilt + ((i + 0.3) / 16) * TAU, v: R.range(45, 120), life: R.range(0.3, 0.52), delay: R.range(0.05, 0.14), c: R.pick([C[6], C[5], WHITE]), r: R.range(1.8, 2.8) }));
  const glints = Array.from({ length: 8 }, (_, i) => { const a = tilt + ((i + 0.5) / 8) * TAU, d = R.range(34, 58); return { x: Math.cos(a) * d, y: Math.sin(a) * d, delay: 0.07 + i * 0.022, life: 0.28, s: R.range(7, 11), c: i % 2 ? WHITE : C[6] }; });
  let t = 0;
  return {
    update(dt) { t += dt; lights.update(dt); },
    draw(p) {
      const f = clamp(t / 0.16);
      if (f < 1) {
        p.disc(0, 0, 9 + 12 * ease.outCubic(f), (d) => (d < 0.55 ? WHITE : C[6]), 1 - f * 0.7);
        p.glow(0, 0, 70 * (1 - f * 0.3), WHITE, 0.95 * (1 - f));
        p.glow(0, 0, 40, C[5], 0.8 * (1 - f));
      }
      for (const [d0, rmax, w0] of [[0.01, 62, 5], [0.07, 40, 3]]) {
        const u = clamp((t - d0) / 0.38);
        if (t >= d0 && u < 1) {
          const r = 8 + (rmax - 8) * ease.outCubic(u);
          p.ring(0, 0, r, Math.max(1, w0 * (1 - u)), u < 0.35 ? WHITE : C[6], 1 - smooth(0.4, 1, u));
          p.glowRing(0, 0, r, 8, C[5], 0.55 * Math.pow(1 - u, 2));
        }
      }
      // the star slams in, overshoots, then collapses to its centre
      const u = clamp((t - 0.01) / 0.34);
      if (t >= 0.01 && u < 1) {
        const grow = u < 0.25 ? ease.outCubic(u / 0.25) * 1.12 : 1.12 - 0.12 * smooth(0.25, 0.4, u), shrink = 1 - smooth(0.45, 1, u), k = grow * shrink, fade = 1 - smooth(0.55, 1, u);
        for (const s of spikes) {
          const L = s.len * k, W = s.w * (0.4 + 0.6 * shrink), cx = Math.cos(s.a), cy = Math.sin(s.a), nx = -cy, ny = cx;
          if (L < 2) continue;
          p.tri(nx * W, ny * W, -nx * W, -ny * W, cx * L, cy * L, C[5], fade);
          p.tri(nx * W * 0.55, ny * W * 0.55, -nx * W * 0.55, -ny * W * 0.55, cx * L * 0.9, cy * L * 0.9, C[6], fade);
          p.tri(nx * W * 0.28, ny * W * 0.28, -nx * W * 0.28, -ny * W * 0.28, cx * L * 0.72, cy * L * 0.72, WHITE, fade);
          p.glowLine(0, 0, cx * L, cy * L, W * 1.8, C[5], 0.5 * fade);
        }
      }
      for (const g of glints) {
        const a = t - g.delay;
        if (a >= 0 && a < g.life) sparkle(p, g.x, g.y, g.s * Math.sin((a / g.life) * Math.PI), g.c, C[5], 0.55);
      }
      for (const m of motes) {
        const a = t - m.delay;
        if (a < 0 || a > m.life) continue;
        const d = (m.v * (1 - Math.exp(-2 * a))) / 2, k = 1 - a / m.life;
        p.glow(Math.cos(m.a) * d, Math.sin(m.a) * d, m.r + 1.6, m.c, 0.5 * k);
        p.dot(Math.cos(m.a) * d, Math.sin(m.a) * d, WHITE, k);
      }
      lights.draw(p);
    },
  };
}
