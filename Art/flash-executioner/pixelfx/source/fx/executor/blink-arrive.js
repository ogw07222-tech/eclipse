import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('blink-arrive','blink-in',[224, 208],[112, 160],0.36,false);
export const draw=(p,t,o)=>drawKind('blink-in',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('blink-in',p,t,o,true);
