import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('verdict-impact','verdict-hit',[336, 320],[168, 232],0.65,false);
export const draw=(p,t,o)=>drawKind('verdict-hit',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('verdict-hit',p,t,o,true);
