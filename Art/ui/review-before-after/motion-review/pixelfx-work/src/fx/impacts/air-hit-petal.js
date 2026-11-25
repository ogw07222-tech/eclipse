// ECLIPSE custom hit variant C: petals of light. Mid-air burst, no floor, no spark streaks.
import { Lights, Rng, fit, sparkle, clamp, smooth, ease, TAU, WHITE } from "../../core/index.js";
import { GOLD } from "../holy/_lib.js";

export const meta = {
  name: "Air hit C (petals of light)",
  desc: "A hit that bursts as a soft flash, eight petals of light blooming outward, two luminous rings and drifting motes.",
  tags: ["impact", "hit", "light", "air", "omnidirectional", "variant", "one-shot"],
  size: [240, 240], origin: [120, 120], duration: 0.6, loop: false, fps: 30, bg: "dark",
  options: { palette: GOLD, seed: 17 },
};

export function create(o) {
  const C = fit(o.palette, 8), R = new Rng(o.seed), lights = new Lights();
  lights.add(0, 0, 66, C[5], 0.8, 0.5);
  lights.add(0, 0, 22, WHITE, 0.8, 0.12);
  const rot = R.range(0, TAU);
  const petals = Array.from({ length: 8 }, (_, i) => ({ a: rot + (i / 8) * TAU, len: i % 2 ? 40 : 54, w: i % 2 ? 0.2 : 0.26, delay: i % 2 ? 0.02 : 0 }));
  const motes = Array.from({ length: 12 }, (_, i) => ({ a: rot + ((i + 0.5) / 12) * TAU, v: R.range(35, 85), life: R.range(0.3, 0.5), delay: R.range(0.04, 0.12), c: R.pick([C[6], C[5], WHITE]), r: R.range(1.6, 2.4) }));
  const glints = Array.from({ length: 4 }, (_, i) => { const a = rot + ((i + 0.5) / 4) * TAU; return { x: Math.cos(a) * 38, y: Math.sin(a) * 38, delay: 0.1 + i * 0.03, life: 0.26, s: 7, c: i % 2 ? WHITE : C[6] }; });
  let t = 0;
  return {
    update(dt) { t += dt; lights.update(dt); },
    draw(p) {
      const f = clamp(t / 0.14);
      if (f < 1) {
        p.disc(0, 0, 6 + 8 * ease.outCubic(f), (d) => (d < 0.6 ? WHITE : C[6]), 1 - f * 0.7);
        p.glow(0, 0, 46 * (1 - f * 0.3), WHITE, 0.85 * (1 - f));
      }
      // petals: tapered wedges that open outward, glow at the root, then fade from the tip
      for (const pt of petals) {
        const u = (t - pt.delay) / 0.34;
        if (u <= 0 || u >= 1) continue;
        const len = pt.len * ease.outCubic(Math.min(1, u * 2.2)), root = 5 + (len - 5) * smooth(0.45, 1, u), fade = 1 - smooth(0.4, 1, u);
        const cx = Math.cos(pt.a), cy = Math.sin(pt.a), nx = -cy, ny = cx, mid = (root + len) * 0.5, wd = (len - root) * pt.w;
        if (len - root < 3) continue;
        p.tri(cx * root + nx * wd * 0.25, cy * root + ny * wd * 0.25, cx * root - nx * wd * 0.25, cy * root - ny * wd * 0.25, cx * len, cy * len, C[6], fade);
        p.tri(cx * mid + nx * wd, cy * mid + ny * wd, cx * mid - nx * wd, cy * mid - ny * wd, cx * len, cy * len, C[5], 0.8 * fade);
        p.tri(cx * root, cy * root, cx * mid + nx * wd, cy * mid + ny * wd, cx * mid - nx * wd, cy * mid - ny * wd, WHITE, 0.9 * fade);
        p.glowLine(cx * root, cy * root, cx * len, cy * len, 7, C[5], 0.4 * fade);
      }
      for (const [d0, rmax, w0] of [[0.01, 34, 3], [0.07, 52, 2]]) {
        const u = clamp((t - d0) / 0.34);
        if (t >= d0 && u < 1) {
          const r = 6 + (rmax - 6) * ease.outCubic(u);
          p.ring(0, 0, r, Math.max(1, w0 * (1 - u)), u < 0.4 ? WHITE : C[6], 1 - smooth(0.4, 1, u));
          p.glowRing(0, 0, r, 6, C[5], 0.45 * Math.pow(1 - u, 2));
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
