// ECLIPSE custom: a thin blade-arc trail for the basic attack's middle motion.
// The origin (0, 0) is the sword hand. The head of the arc sweeps from behind the shoulder (up-left)
// to level forward (right), and a short trail follows behind it, fading from white to deep blue.
import { P, fit, clamp, smooth, ease, TAU } from "../../core/index.js";

export const meta = {
  name: "Blade arc (mid swing)",
  desc: "A thin crescent trail from behind the shoulder to level forward, pivoting on the hand, white head fading into a blue trail.",
  tags: ["melee", "slash", "sword", "arc", "one-shot"],
  size: [96, 80],
  origin: [48, 44],
  duration: 0.2,
  loop: false,
  fps: 30,
  bg: "ground",
  options: {
    palette: ["#212f6b", "#2c58aa", "#3a8bd8", "#62c8f4", "#b8c9e8", "#e8f1ff", "#ffffff"],
    radius: 30,
    seed: 3,
  },
};

// head angle (screen radians, y down): behind-up to level forward
const A_START = -2.45; // up-left, behind the shoulder
const A_END = 0.05; // level, to the right
const TRAIL = 1.1; // radians of trail behind the head

export function draw(p, t, o) {
  const C = fit(o.palette, 7);
  const R = o.radius;
  const dur = meta.duration;
  const k = clamp(t / dur);
  if (k >= 1) return;
  const head = A_START + (A_END - A_START) * ease.outQuart(clamp(t / (dur * 0.7)));
  const tail = head - TRAIL * (0.4 + 0.6 * smooth(0, 0.7, k));
  const fade = 1 - smooth(0.55, 1, k);
  const B = R + 3;
  p.shade(-B, -B, B, B, (x, y, out) => {
    const r = Math.hypot(x + 0.5, y + 0.5);
    if (r > R + 0.5 || r < R - 3) return;
    let a = Math.atan2(y + 0.5, x + 0.5);
    // position along the trail from tail (0) to head (1)
    let u = (a - tail) % TAU;
    if (u < 0) u += TAU;
    const span = (head - tail + TAU) % TAU;
    if (u > span) return;
    const v = u / span;
    // width: thinnest at the tail, thickest near the head, and a thin edge on the outer rim
    const w = 1.2 + 1.8 * Math.pow(v, 1.6);
    const q = (R - r) / (w + 0.01);
    if (q < 0 || q > 1) return;
    const idx = clamp(Math.round(v * 5 + (1 - q) * 1.2), 0, 6);
    out.c = C[idx];
    out.a = clamp(Math.min(1, v / 0.25) * fade);
  });
  // a white glint at the head tip
  const hx = Math.cos(head) * (R - 1), hy = Math.sin(head) * (R - 1);
  if (k < 0.4) p.dot(hx, hy, C[6], 1 - k / 0.4);
}
