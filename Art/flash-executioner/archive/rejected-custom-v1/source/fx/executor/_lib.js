// Original Flash Executioner recipes using pixelfx drawing primitives.
// Choreography references: Sprite Fusion slash, holy light and blink (MIT), Hugo Duprez.
import { fit, hex, clamp, smooth, ease, TAU, sparkle, band, READY } from '../../core/index.js';
export const PALETTE=['#342146','#71529b','#ac822f','#ecb84b','#ffe69a','#fff5cf','#e8feff','#ffffff'];
const CYAN=hex('#9eefff'), VIOLET=hex('#b79aff');
const life=(t,d)=>t<=0||t>=d-1/60?0:Math.min(1,t/.025)*(1-smooth(d*.58,d,t));
function motes(p,t,d,C,n=18,reach=70,y=-38,ground=false){
 for(let i=0;i<n;i++){const delay=(i%5)*.012,u=(t-delay)/(d*.92); if(u<=0||u>=1)continue;const a=ground?Math.PI+(i*2.39996%Math.PI):(i*2.39996+1.3),r=8+reach*ease.outCubic(u),xx=Math.cos(a)*r,yy=y+Math.sin(a)*r*.55-u*9,fade=Math.sin(Math.PI*u); const size=(i%4===0?3:1.1)*fade;
  p.line(xx-Math.cos(a)*7*(1-u),yy-Math.sin(a)*4*(1-u),xx,yy,C[i%3+4],fade); if(i%3===0)sparkle(p,xx,yy,size,C[7],C[4],.22*fade);
 }
}
function ring(p,r,sy,thick,C,a,back=false,y=0){band(p,0,y,r,sy,thick,a,C.slice(3),g=>(Math.sin(g)<0)===back,false);}
function rays(p,t,d,C,n=12,r=70,y=-45,ground=false){const k=life(t,d);if(k<=0)return;for(let i=0;i<n;i++){const a=ground?Math.PI+.12+i*(Math.PI-.24)/(n-1):i*TAU/n+.12,r0=8+22*ease.outCubic(t/d),r1=r0+(r-r0)*Math.pow(k,.7),x=Math.cos(a),yy=Math.sin(a),w=(i%2?2:5)*k; p.tri(x*r0-yy*w,y+yy*r0+x*w,x*r1,y+yy*r1,x*r0+yy*w,y+yy*r0-x*w,i%2?C[4]:C[5],k*.75);p.line(x*r0,y+yy*r0,x*r1,y+yy*r1,i%3?C[6]:CYAN,k);}p.glow(0,y,34,C[4],.3*k);}
function diamond(p,x,y,w,h,C,a=1){p.poly([[x,y-h],[x+w,y],[x,y+h],[x-w,y]],C[4],a);p.poly([[x,y-h],[x,y+h],[x-w*.55,y]],C[7],a);p.line(x,y-h,x,y+h,C[6],a);}
export function drawKind(kind,p,t,o,back=false){
 const C=fit(o.palette,8),d=o.duration,k=kind==='orb'?1:life(t,d);if(!k)return;
 const u=t/d, q=ease.outCubic(clamp(u*2));
 if(kind==='orb'){
  if(back)return; const pulse=.85+.15*Math.sin(t*TAU),r=6*pulse; p.circle(0,0,r,C[4]);p.circle(-1,-1,r*.67,C[6]);p.circle(-2,-2,2,C[7]);p.glow(0,0,18,C[4],.26);for(let i=0;i<3;i++){let a=t*TAU+i*TAU/3;p.px(Math.cos(a)*10,Math.sin(a)*10,C[5]);}return;
 }
 if(kind==='slash'){
  if(back)return;const head=-2.75+3.9*ease.outExpo(clamp(t/.075)),tail=-2.75+3.9*smooth(.09,.33,t); const r=77;
  if(head>tail){for(let j=0;j<3;j++)p.arc(8,-47,r-j*7,(v)=>Math.max(.7,(12-j*3)*Math.sin(Math.PI*v)),tail,head,C[7-j*2],k);p.arc(8,-47,r+3,1,tail,head,CYAN,k*.85);p.arc(8,-47,r-24,2,tail+.18,head+.18,C[4],k*.65);p.glow(57,-62,30,C[4],.22*k);}
  motes(p,t,d,C,26,91,-47);if(t<.14)sparkle(p,67,-30,13*(1-t/.14),C[7],C[4],.3);return;
 }
 if(kind==='rush'||kind==='dash'){
  const long=kind==='rush'?150:100;
  if(back){for(let i=0;i<9;i++){const yy=-20-i*7+(i%2)*4,xx=-long+18*i%37; p.line(xx+q*12,yy,-10+(i%3)*12,yy,C[i%2?4:2],k*.65);}return;}
  const cy=-40; p.poly([[-long,cy-4],[-8,cy-9],[70,cy],[-8,cy+9]],C[4],k*.8);p.poly([[-long+25,cy],[-8,cy-3],[76,cy],[-8,cy+3]],C[7],k);p.line(-long,cy+12,32,cy+7,CYAN,k*.7);for(let i=0;i<4;i++)p.line(-100+i*16,-11-i*4,-25+i*12,-11-i*4,C[5],k*.6);motes(p,t,d,C,10,70,cy);return;
 }
 if(kind==='command'||kind==='charge'||kind==='swap'){
  const charge=kind==='charge', swap=kind==='swap', radius=charge?88:swap?77:62,cy=charge?-65:-43;
  if(back){ring(p,radius*q,.23,2,C,k*.8,true);if(charge)ring(p,38+q*25,.35,1,C,k*.7,true,-93);return;}
  ring(p,radius*q,.23,2,C,k*.8,false);const a=t*7;
  for(let i=0;i<(charge?8:4);i++){let g=a+i*TAU/(charge?8:4),rr=radius*(.65+.35*(1-u));diamond(p,Math.cos(g)*rr,cy+Math.sin(g)*rr*.55,3,8,C,k*.9);}
  if(!charge && !swap && t<.11){const pop=Math.sin(Math.PI*Math.min(1,t/.11)); diamond(p,12,-58,12*pop,24*pop,C,1);p.glow(12,-58,24,C[4],.3*pop); for(let j=0;j<3;j++){let yy=-72+j*14;p.poly([[20,yy-3],[53+16*j,yy],[20,yy+3],[30,yy]],C[j%2?5:4],pop);}} if(charge){p.line(0,-145,0,-14,C[6],k*.75);p.line(-4,-126,-4,-39,C[4],k*.6);rays(p,t,d,C,8,65,-98);}
  else {p.arc(0,cy,32+15*q,2,a,a+Math.PI*1.55,C[5],k);p.arc(0,cy,39+15*q,1,-a,-a+Math.PI*1.5,CYAN,k*.65);rays(p,t,d,C,6,32,cy);}
  motes(p,t,d,C,18,radius,cy);return;
 }
 if(kind==='blink-out'||kind==='blink-in'){
  const arrival=kind==='blink-in'; if(back){ring(p,10+72*q,.2,2.5,C,k*.8,true);return;}ring(p,10+72*q,.2,2.5,C,k*.8,false);
  const width=arrival?Math.sin(Math.PI*u):1-u;const h=67*Math.sin(Math.PI*Math.min(1,u*1.35));p.beam(0,-36-h,0,-36+h,Math.max(2,12*width),C[6],k);p.line(-3,-36-h*.8,-3,-36+h*.8,C[4],k);p.glow(0,-42,29,CYAN,.28*k);for(let i=0;i<9;i++){const yy=-12-i*9,dx=(arrival?1-u:u)*58; p.line(-dx,yy,-dx+9,yy,C[5],k*.8);p.line(dx-9,yy-3,dx,yy-3,CYAN,k*.8);}motes(p,t,d,C,22,80,-42);return;
 }
 if(kind==='lance'||kind==='sky-lance'){
  if(back)return;const sky=kind==='sky-lance';let x=sky?0:20,y=sky?-63:-5;
  if(sky){p.poly([[x,y+59],[x-6,y+35],[x-3,y-49],[x,y-64],[x+3,y-49],[x+6,y+35]],C[4],k);p.poly([[x,y+59],[x-2,y+26],[x,y-64],[x+2,y+26]],C[7],k);p.line(-10,y-54,-10,y+17,CYAN,k*.7);p.line(11,y-31,11,y+27,C[5],k*.7);p.glowLine(0,y-40,0,y+44,12,C[4],.2*k);}
  else {p.poly([[x+63,y],[x+33,y-7],[x-48,y-3],[x-64,y],[x-48,y+3],[x+33,y+7]],C[4],k);p.poly([[x+63,y],[x+26,y-2],[x-60,y],[x+26,y+2]],C[7],k);p.line(x-56,y-10,x+22,y-10,CYAN,k*.6);p.line(x-43,y+10,x+34,y+10,C[5],k*.6);p.glowLine(x-40,y,x+47,y,13,C[4],.2*k);}
  return;
 }
 if(kind==='impact'||kind==='verdict-hit'||kind==='land'||kind==='jump'){
  const big=kind==='verdict-hit',ground=big||kind==='land'||kind==='jump',r=big?116:ground?63:74,cy=ground?-4:-30;
  if(back){if(ground)ring(p,r*q,.22,3*(1-u)+1,C,k*.9,true);return;}
  if(ground)ring(p,r*q,.22,3*(1-u)+1,C,k*.9,false);else p.arc(0,cy,r*q,2*(1-u)+1,0,TAU,C[5],k*.85);
  if(big){const surge=1-smooth(.10,.40,t);if(surge>0){p.poly([[-15,-2],[-24,-26],[-9,-47],[-15,-69],[-5,-106],[0,-157],[6,-94],[18,-62],[10,-41],[24,-18],[13,-2]],C[4],surge);p.poly([[-6,-3],[-10,-27],[-3,-70],[0,-140],[4,-63],[10,-26],[5,-3]],C[6],surge);for(const side of [-1,1])for(let j=0;j<3;j++){const xx=side*(25+25*j),yy=-24-17*j,w=8-j; p.poly([[xx-side*w,-5],[xx-side*w*.5,yy],[xx+side*(9+j*5),yy-31-j*7],[xx+side*w,yy-2],[xx+side*w,-5]],C[j%2?4:5],surge);p.line(xx,-7,xx+side*(9+j*5),yy-30-j*7,C[7],surge);}}p.glow(0,-22,44,C[4],.34*k);p.glow(0,-8,18,C[6],.25*k);p.beam(0,-174,0,-9,3+8*Math.pow(1-u,3),C[6],k);p.arc(0,-4,20+70*q,2,Math.PI,TAU,CYAN,k*.8);}
  rays(p,t,d,C,big?14:8,r,cy,ground);if(big){for(const side of [-1,1]){p.arc(side*20,-30,48+22*q,4*(1-u),Math.PI*.96,TAU*.97,C[4],k*.7);diamond(p,side*(32+36*q),-38-48*q,4*k,14*k,C,k);}}if(t<.07){p.circle(0,cy,Math.max(1,13*(1-t/.07)),C[7],k);sparkle(p,0,cy,23*(1-t/.07),C[7],C[4],.3);}
  motes(p,t,d,C,big?36:20,r,cy,ground);return;
 }
}
export function makeMeta(name,kind,size,origin,duration,loop=false){return {name,desc:'White-gold light with crisp cyan rims, decisive release, layered glints and a clean fading tail.',tags:['executor','light',kind],size,origin,duration,loop,fps:60,bg:'ground',options:{palette:PALETTE,seed:17,duration}};}
