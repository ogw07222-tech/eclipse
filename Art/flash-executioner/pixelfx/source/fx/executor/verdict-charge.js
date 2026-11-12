import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('verdict-charge','charge',[288, 256],[144, 206],0.728,false);
export const draw=(p,t,o)=>drawKind('charge',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('charge',p,t,o,true);
