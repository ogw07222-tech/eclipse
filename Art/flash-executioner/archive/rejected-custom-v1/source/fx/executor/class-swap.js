import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('class-swap','swap',[256, 224],[128, 172],0.4,false);
export const draw=(p,t,o)=>drawKind('swap',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('swap',p,t,o,true);
