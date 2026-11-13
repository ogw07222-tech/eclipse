// A compact airborne wake remixed from Pixelfx status/haste's speed streaks and fire/fireball's trailing puffs.
import { fit, cycle, rng, smooth, TAU } from "../../core/index.js";

export const meta = {
  name: "Flight Trail",
  desc: "A light violet wake streams behind and below a flying character: short wisps, soft pixel-clustered puffs and a few rising sparks.",
  tags: ["flight", "flying", "movement", "trail", "status", "directional", "loop"],
  size: [112, 72],
  origin: [70, 52],
  duration: 1.2,
  loop: true,
  fps: 30,
  bg: "transparent",
  options: { palette: ["#080515", "#190b35", "#321064", "#521895", "#7728bd", "#a64be8", "#d59aff", "#efe4ff"], seed: 12 },
};

const D = meta.duration;

export function drawBack(p, t, o) {
  const C = fit(o.palette, 8), f = Math.floor(t * 30);
  // Soft clustered exhaust sits below the feet and trails behind the direction of travel.
  for (let i = 0; i < 7; i++) {
    const q = (t / D + i / 7) % 1, x = -5 - q * 44, y = 1 + i % 2 * 5 + 2.5 * Math.sin(q * TAU + i);
    const r = 3 + 4.5 * q, a = 0.32 * (1 - q);
    p.disc(x, y, r, C[2], a);
    p.disc(x - 1, y - 1, r * 0.55, C[4], a * 0.72);
  }
  // Uneven separated streaks keep it airy and readable as a simple flight cue.
  for (let i = 0; i < 5; i++) {
    const g = rng(f, o.seed, i), y = -6 + i * 5 + g.range(-1, 1), x = -8 - ((t * (38 + i * 8) + i * 23) % 43), len = 9 + g.range(4, 12);
    p.beam(x, y, x - len, y + g.range(-2, 2), i === 0 ? 1.5 : 1, i === 0 ? C[6] : C[4], 0.25 + 0.2 * Math.sin(t * 8 + i));
  }
  cycle(t, D, 22, o.seed + 3, (r) => r.range(0.28, 0.56), (age, life, r, i) => {
    const q = age / life, x = -5 - q * r.range(14, 42), y = r.range(-4, 10) - q * r.range(3, 11), a = Math.min(1, q * 6) * (1 - smooth(0.7, 1, q));
    p.dot(x, y, i % 4 === 0 ? C[7] : C[5], a);
    if (i % 3 === 0) p.glow(x, y, 3, C[5], 0.34 * a);
  });
}

export function draw(p, t, o) {
  const C = fit(o.palette, 8), pulse = 0.85 + 0.15 * Math.sin(t * TAU * 2 / D);
  // Small pressure glow at the heels; keep the silhouette clear for a character layered over it.
  p.glow(-3, 2, 19, C[3], 0.16 * pulse);
  p.glow(-1, 1, 8, C[5], 0.14 * pulse);
}
