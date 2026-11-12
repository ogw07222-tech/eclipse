import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('basic-slash','slash',[256, 224],[116, 163],0.4,false);
export const draw=(p,t,o)=>drawKind('slash',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('slash',p,t,o,true);
