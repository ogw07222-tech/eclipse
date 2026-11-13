// The shifting aperture and its pulled-in motes are remixed from Pixelfx's arcane/portal
// and shadow/void-orb resources: a local space-fold effect, shown fully side-on, without a tornado silhouette.
import { PAL, fit, cycle, rng, smooth, ease, TAU, WHITE } from "../../core/index.js";

export const meta = {
  name: "Gravity Field",
  desc: "Nearby space unfolds around a jagged violet rift: layered planes peel apart, bend toward the seam, and shed chunky fragments before folding back.",
  tags: ["shadow", "space", "gravity", "rift", "fold", "side-view", "loop"],
  size: [240, 176],
  origin: [120, 100],
  duration: 2.4,
  loop: true,
  fps: 30,
  bg: "transparent",
  options: { palette: PAL.shadow, seed: 8 },
};

const D = meta.duration;

function fold(p, C, t, seed) {
  const phase=t/D, pulse=0.5+0.5*Math.sin(phase*TAU), wave=(y,k)=>Math.sin(y*0.11+phase*TAU*1.4+k)*4+Math.sin(y*0.23-phase*TAU+k*2)*2;
  const seam=(y)=>2*Math.sin(y*0.25+phase*TAU)+1.3*Math.sin(y*0.61-phase*TAU*2);
  // Unequal polygon planes split at the character's local space and slide sideways like pages unfolding.
  const planes=[
    {s:-1,pts:[[3,-65],[27,-60],[73,-55],[103,-41],[94,-30],[53,-35],[18,-45]],fill:C[2],edge:C[5],a:0.72},
    {s:-1,pts:[[2,8],[22,5],[53,-1],[85,-15],[71,-24],[43,-17],[15,-10]],fill:C[1],edge:C[4],a:0.68},
    {s:1,pts:[[2,-59],[25,-55],[54,-46],[78,-37],[68,-29],[40,-36],[17,-43]],fill:C[3],edge:C[6],a:0.68},
    {s:1,pts:[[2,5],[20,1],[46,-7],[91,-21],[108,-13],[69,-5],[31,-4]],fill:C[2],edge:C[5],a:0.72},
  ];
  for(let k=0;k<planes.length;k++){
    const q=planes[k], side=q.s, drift=5*Math.sin(phase*TAU+k*1.4), sy=2*Math.sin(phase*TAU*2+k);
    const pts=q.pts.map(([x,y],i)=>[side*(x+drift*(i/6)),y+sy*Math.sin(i*1.8)]);
    p.poly(pts,q.fill,q.a);
    p.polyline([pts[0],pts[1],pts[2],pts[3]],q.edge,0.88);
    if(k%2===0)p.polyline([pts[4],pts[5],pts[6]],C[4],0.72);
    // Pixel clumps break the planar rims into chipped, tangible pieces.
    for(let i=1;i<6;i++){
      const [x,y]=pts[i],g=rng(k,i,seed),j=g.range(-1.5,1.5);
      if(g.next()>0.28)p.rect(x*0.72+side*j,y+j, g.range(1,3),g.range(1,2),i%2?C[5]:C[3],0.85);
    }
  }
  // Horizontal strata shear across the local area; each stops and restarts at different points.
  for(let j=0;j<7;j++){
    const y0=-57+j*9, left=-108+j*5, right=72+(j%3)*12, pts=[];
    for(let i=0;i<=8;i++){
      const x=left+(right-left)*i/8, y=y0+wave(y0+i*6,j)*0.55+2*Math.sin(i*0.8+phase*TAU+j);
      pts.push([x,y]);
    }
    const col=j%3===0?C[6]:j%2?C[4]:C[3];
    p.polyline(pts,col,j%3===0?0.66:0.4);
    if(j%2===0)p.glow(0,y0,52,col,0.1+0.08*pulse);
  }
  // Jagged hinge crack, bright only in short pulses so the folded planes remain the main silhouette.
  const seamPts=[];
  for(let y=-67;y<=12;y+=4)seamPts.push([seam(y),y]);
  p.polyline(seamPts,C[6],0.7+0.24*pulse);
  for(let y=-64;y<10;y+=9){const x=seam(y);p.dot(x,y,C[7]);p.glow(x,y,6,C[5],0.2+0.16*pulse);}
  // Chunky shards caught around the opening, spaced irregularly in front and behind.
  for(let i=0;i<20;i++){
    const g=rng(i,seed,21),a=phase*TAU+i*2.399,side=i%2?1:-1;
    const x=side*(36+g.range(0,74))+5*Math.sin(a*1.4),y=-29+g.range(-40,35)+4*Math.cos(a),w=g.range(2,7),h=g.range(3,9),skew=g.range(-3,3);
    const pts=[[x-w,y-h/2],[x+skew,y-h],[x+w,y+skew],[x-w/2,y+h]];
    p.poly(pts,i%4===0?C[5]:i%3===0?C[3]:C[2],0.88);
    p.polyline([[x-w,y-h/2],[x+skew,y-h],[x+w,y+skew]],i%4===0?C[7]:C[6],0.9);
    if(i%3===0)p.glow(x,y,5,C[5],0.28);
  }
  const flare=Math.pow(Math.max(0,Math.sin(phase*TAU)),12);
  p.beam(0,-64,0,9,2+flare*2,C[7],0.45+flare*0.5);
  p.glow(0,-28,76,C[3],0.22+0.1*pulse);
  p.glow(0,-28,26,C[5],0.24+0.32*flare);
}

function motes(p, t, C, seed) {
  cycle(t, D, 34, seed + 11, (r) => r.range(0.4, 0.9), (age, life, r, i) => {
    const u = age / life, side = i % 2 ? 1 : -1, start = side * r.range(74, 128), y0 = r.range(-66, 8);
    const x = start * (1 - ease.inQuad(u)) + side * 3 * Math.sin(u * TAU + i), y = y0 + 22 * Math.sin(u * Math.PI) - u * 8;
    const alpha = Math.min(1, u * 5) * (1 - smooth(0.72, 1, u)), c = u > 0.7 ? C[7] : u > 0.35 ? C[6] : C[4];
    p.dot(x, y, c, alpha);
    if (u > 0.25) p.glow(x, y, 3 + u * 2, C[5], 0.3 * alpha);
  });
}

export function drawBack(p, t, o) {
  const C = fit(o.palette, 8);
  p.glow(0, -28, 94, C[2], 0.12 + 0.04 * Math.sin(t * TAU / D));
}

export function draw(p, t, o) {
  const C = fit(o.palette, 8);
  fold(p, C, t, o.seed);
  motes(p, t, C, o.seed);
}
