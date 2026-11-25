// ECLIPSE custom: a hit that bursts as LIGHT in mid-air. The upstream impacts/crit-hit assumes a floor (upward spark kick,
// gravity shards that bounce) and reads like fireworks; this one has no floor and no spark streaks. A white flash blooms,
// a luminous ring and a few soft rays rush out in every direction, glints twinkle and light motes drift off.
// The ring-and-flare idea is adapted from Sprite Fusion's open-source impact
// (https://www.spritefusion.com/pixel-art-vfx/impact), MIT License, Copyright (c) 2026 Hugo Duprez.
import { Lights, Rng, fit, sparkle, clamp, smooth, ease, TAU, WHITE } from "../../core/index.js";
import { GOLD } from "../holy/_lib.js";

export const meta = {
  name: "Air hit (burst of light)",
  desc: "A hit that bursts as light in mid-air: a white flash blooms, a luminous ring and soft rays rush out in every direction, glints twinkle and motes drift.",
  tags: ["impact", "hit", "light", "air", "omnidirectional", "combat", "one-shot"],
  size: [240, 240],
  origin: [120, 120],
  duration: 0.55,
  loop: false,
  fps: 30,
  bg: "dark",
  options: { palette: GOLD, seed: 5 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), lights = new Lights();
  lights.add(0, 0, 76, C[5], 0.9, 0.45);
  lights.add(0, 0, 26, WHITE, 0.9, 0.1);
  const fan = (n, f) => Array.from({ length: n }, (_, i) => f((i + R.range(-0.35, 0.35)) / n * TAU));
  const rays = fan(10, (a) => ({ a, len: R.range(34, 60), w: R.range(2, 4), life: R.range(0.18, 0.3), delay: R.range(0, 0.03) }));
  const glints = Array.from({ length: 5 }, () => { const a = R.range(0, TAU), d = R.range(18, 46); return { x: Math.cos(a) * d, y: Math.sin(a) * d, delay: R.range(0.04, 0.22), life: R.range(0.2, 0.34), s: R.range(5, 9), c: R.chance(0.5) ? WHITE : C[6] }; });
  const motes = fan(12, (a) => ({ a, v: R.range(30, 90), life: R.range(0.3, 0.5), delay: R.range(0, 0.06), c: R.pick([C[6], C[5], WHITE]), r: R.range(1.6, 2.6) }));
  let t = 0;
  return {
    update(dt) { t += dt; lights.update(dt); },
    draw(p) {
      // the flash: a white-hot core that swells and burns off inside a big bloom
      const f = clamp(t / 0.16);
      if (f < 1) {
        p.disc(0, 0, 7 + 11 * ease.outCubic(f), (d) => (d < 0.55 ? WHITE : C[6]), 1 - f * 0.75);
        p.glow(0, 0, 54 * (1 - f * 0.35), WHITE, 0.9 * (1 - f));
        p.glow(0, 0, 30, C[5], 0.7 * (1 - f));
      }
      // one luminous ring
      const u = clamp((t - 0.02) / 0.3);
      if (t >= 0.02 && u < 1) {
        const r = 6 + 38 * ease.outCubic(u);
        p.ring(0, 0, r, Math.max(1, 3 * (1 - u)), u < 0.4 ? WHITE : C[6], 1 - smooth(0.4, 1, u));
        p.glowRing(0, 0, r, 7, C[5], 0.55 * Math.pow(1 - u, 2));
      }
      // a soft lens cross: it blooms with the flash and shrinks away
      const g = clamp((t - 0.02) / 0.28);
      if (g > 0 && g < 1) {
        const L = 50 * Math.pow(1 - g, 1.2);
        p.glowLine(-L, 0, L, 0, 5, C[6], 0.6 * (1 - g));
        p.glowLine(0, -L * 0.7, 0, L * 0.7, 4, C[6], 0.45 * (1 - g));
        p.glowLine(-L * 0.4, -L * 0.4, L * 0.4, L * 0.4, 3, C[5], 0.3 * (1 - g));
        p.glowLine(-L * 0.4, L * 0.4, L * 0.4, -L * 0.4, 3, C[5], 0.3 * (1 - g));
      }
      // a few wide soft rays of light (no thin spark streaks)
      for (const r of rays) {
        const k = (t - r.delay) / r.life;
        if (k <= 0 || k >= 1) continue;
        const head = r.len * ease.outCubic(Math.min(1, k * 2.4)), tail = k < 0.35 ? 6 : 6 + (head - 6) * ((k - 0.35) / 0.65);
        if (head - tail < 2) continue;
        const cx = Math.cos(r.a), cy = Math.sin(r.a);
        p.glowLine(cx * tail, cy * tail, cx * head, cy * head, r.w * 2.2, C[5], 0.5 * (1 - k));
        p.line(cx * tail, cy * tail, cx * head, cy * head, (q) => (q > 0.6 ? WHITE : C[6]), (q) => (0.3 + 0.7 * q) * (1 - k * 0.6));
      }
      for (const m of glints) {
        const a = t - m.delay;
        if (a >= 0 && a < m.life) sparkle(p, m.x, m.y, m.s * Math.sin((a / m.life) * Math.PI), m.c, C[5], 0.55);
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
