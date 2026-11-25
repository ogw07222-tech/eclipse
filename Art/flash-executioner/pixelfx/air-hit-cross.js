// ECLIPSE custom hit variant B: two crossing blades of light. Mid-air burst, no floor, no spark streaks.
import { Lights, Rng, fit, sparkle, clamp, smooth, ease, TAU, WHITE } from "../../core/index.js";
import { GOLD } from "../holy/_lib.js";

export const meta = {
  name: "Air hit B (crossing light blades)",
  desc: "A hit that bursts as two crossing blades of light with a small bloom, glints along the cuts and drifting motes.",
  tags: ["impact", "hit", "light", "air", "omnidirectional", "variant", "one-shot"],
  size: [240, 240], origin: [120, 120], duration: 0.55, loop: false, fps: 30, bg: "dark",
  options: { palette: GOLD, seed: 11 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), lights = new Lights();
  lights.add(0, 0, 60, C[5], 0.8, 0.45);
  lights.add(0, 0, 20, WHITE, 0.8, 0.1);
  // [angle, half length, delay, width]
  const cuts = [[-0.62, 66, 0, 3.2], [0.72, 52, 0.035, 2.6]];
  const glints = cuts.flatMap(([a, L, d]) => [-1, 1].map((s) => ({ x: Math.cos(a) * L * s * 0.9, y: Math.sin(a) * L * s * 0.9, delay: d + 0.05, life: 0.26, s: R.range(6, 9), c: s > 0 ? WHITE : C[6] })));
  const motes = Array.from({ length: 12 }, (_, i) => { const c = cuts[i % 2], s = i % 4 < 2 ? 1 : -1, a = c[0] + (s > 0 ? 0 : Math.PI) + R.range(-0.25, 0.25); return { a, v: R.range(40, 100), life: R.range(0.28, 0.46), delay: R.range(0.03, 0.09), c: R.pick([C[6], WHITE, C[5]]), r: R.range(1.6, 2.4) }; });
  let t = 0;
  return {
    update(dt) { t += dt; lights.update(dt); },
    draw(p) {
      const f = clamp(t / 0.13);
      if (f < 1) {
        p.disc(0, 0, 5 + 8 * ease.outCubic(f), (d) => (d < 0.55 ? WHITE : C[6]), 1 - f * 0.7);
        p.glow(0, 0, 40 * (1 - f * 0.3), WHITE, 0.8 * (1 - f));
      }
      for (const [a, L, d, w] of cuts) {
        const u = (t - d) / 0.3;
        if (u <= 0 || u >= 1) continue;
        const head = L * ease.outCubic(Math.min(1, u * 3.2)), tail = u < 0.3 ? 0 : head * smooth(0.3, 1, u);
        const cx = Math.cos(a), cy = Math.sin(a), fade = 1 - smooth(0.35, 1, u);
        // a blade tapering to a point at both ends, thick at the middle
        for (const s of [-1, 1]) {
          const x0 = cx * tail * s, y0 = cy * tail * s, x1 = cx * head * s, y1 = cy * head * s;
          if (head - tail < 1) continue;
          p.glowLine(x0, y0, x1, y1, w * 3.2, C[5], 0.55 * fade);
          p.beam(x0, y0, x1, y1, w * 1.6, (q) => (q < 0.55 ? WHITE : C[6]), fade);
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
        p.glow(Math.cos(m.a) * d, Math.sin(m.a) * d, m.r + 1.4, m.c, 0.5 * k);
        p.dot(Math.cos(m.a) * d, Math.sin(m.a) * d, WHITE, k);
      }
      lights.draw(p);
    },
  };
}
