import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('light-orb','orb',[48, 48],[24, 24],1,true);
export const draw=(p,t,o)=>drawKind('orb',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('orb',p,t,o,true);
