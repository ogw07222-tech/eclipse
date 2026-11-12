import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('blink-depart','blink-out',[224, 208],[112, 160],0.32,false);
export const draw=(p,t,o)=>drawKind('blink-out',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('blink-out',p,t,o,true);
