// ECLIPSE custom: a holy lance landing, bursting as light in mid-air. holy/smite is a ground effect (floor shockwave in
// perspective, stone chips, cracks, dust along the floor). Here there is no floor and no spark streaks: a big white flash blooms,
// round luminous rings and wide soft rays rush out in every direction, glints twinkle and light motes drift off.
import { Lights, Rng, fit, sparkle, clamp, smooth, ease, TAU, WHITE } from "../../core/index.js";
import { GOLD } from "./_lib.js";

export const meta = {
  name: "Air burst (burst of holy light)",
  desc: "A holy lance lands in mid-air: a white flash blooms, round luminous rings and wide soft rays rush out in every direction, glints twinkle and motes drift.",
  tags: ["holy", "strike", "impact", "light", "air", "omnidirectional", "one-shot"],
  size: [320, 320],
  origin: [160, 160],
  duration: 0.85,
  loop: false,
  fps: 30,
  bg: "dark",
  options: { palette: GOLD, seed: 21 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), lights = new Lights();
  lights.add(0, 0, 112, C[5], 1, 0.55);
  lights.add(0, 0, 40, WHITE, 0.95, 0.12);
  const fan = (n, f) => Array.from({ length: n }, (_, i) => f((i + R.range(-0.35, 0.35)) / n * TAU));
  const rays = fan(14, (a) => ({ a, len: R.range(48, 92), w: R.range(2.4, 5), life: R.range(0.22, 0.4), delay: R.range(0, 0.04) }));
  const glints = Array.from({ length: 8 }, () => { const a = R.range(0, TAU), d = R.range(22, 70); return { x: Math.cos(a) * d, y: Math.sin(a) * d, delay: R.range(0.05, 0.4), life: R.range(0.25, 0.45), s: R.range(6, 12), c: R.chance(0.5) ? WHITE : C[6] }; });
  const motes = fan(20, (a) => ({ a, v: R.range(40, 130), life: R.range(0.4, 0.7), delay: R.range(0, 0.1), c: R.pick([C[6], C[5], WHITE]), r: R.range(1.8, 3) }));
  let t = 0;
  return {
    update(dt) { t += dt; lights.update(dt); },
    draw(p) {
      const f = clamp(t / 0.2);
      if (f < 1) {
        p.disc(0, 0, 9 + 15 * ease.outCubic(f), (d) => (d < 0.55 ? WHITE : C[6]), 1 - f * 0.7);
        p.glow(0, 0, 78 * (1 - f * 0.35), WHITE, 0.95 * (1 - f));
        p.glow(0, 0, 44, C[5], 0.7 * (1 - f));
      }
      for (const [d0, rmax, w0] of [[0, 72, 6], [0.06, 46, 4]]) {
        const u = clamp((t - d0) / 0.45);
        if (t >= d0 && u < 1) {
          const r = 8 + (rmax - 8) * ease.outCubic(u);
          p.ring(0, 0, r, Math.max(1, w0 * Math.pow(1 - u, 1.1)), u < 0.3 ? WHITE : u < 0.6 ? C[6] : C[4], 1 - smooth(0.5, 1, u));
          p.glowRing(0, 0, r, 9, C[5], 0.55 * Math.pow(1 - u, 1.6));
        }
      }
      const g = clamp((t - 0.02) / 0.34);
      if (g > 0 && g < 1) {
        const L = 82 * Math.pow(1 - g, 1.2);
        p.glowLine(-L, 0, L, 0, 6, C[6], 0.6 * (1 - g));
        p.glowLine(0, -L * 0.75, 0, L * 0.75, 5, C[6], 0.5 * (1 - g));
        p.glowLine(-L * 0.4, -L * 0.4, L * 0.4, L * 0.4, 3, C[5], 0.3 * (1 - g));
        p.glowLine(-L * 0.4, L * 0.4, L * 0.4, -L * 0.4, 3, C[5], 0.3 * (1 - g));
      }
      for (const r of rays) {
        const k = (t - r.delay) / r.life;
        if (k <= 0 || k >= 1) continue;
        const head = r.len * ease.outCubic(Math.min(1, k * 2.4)), tail = k < 0.35 ? 8 : 8 + (head - 8) * ((k - 0.35) / 0.65);
        if (head - tail < 2) continue;
        const cx = Math.cos(r.a), cy = Math.sin(r.a);
        p.glowLine(cx * tail, cy * tail, cx * head, cy * head, r.w * 2.2, C[5], 0.55 * (1 - k));
        p.line(cx * tail, cy * tail, cx * head, cy * head, (q) => (q > 0.6 ? WHITE : C[6]), (q) => (0.3 + 0.7 * q) * (1 - k * 0.6));
      }
      for (const m of glints) {
        const a = t - m.delay;
        if (a >= 0 && a < m.life) sparkle(p, m.x, m.y, m.s * Math.sin((a / m.life) * Math.PI), m.c, C[5], 0.55);
      }
      for (const m of motes) {
        const a = t - m.delay;
        if (a < 0 || a > m.life) continue;
        const d = (m.v * (1 - Math.exp(-1.8 * a))) / 1.8, k = 1 - a / m.life;
        p.glow(Math.cos(m.a) * d, Math.sin(m.a) * d, m.r + 1.6, m.c, 0.5 * k);
        p.dot(Math.cos(m.a) * d, Math.sin(m.a) * d, WHITE, k);
      }
      lights.draw(p);
    },
  };
}
