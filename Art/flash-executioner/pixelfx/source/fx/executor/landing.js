import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('landing','land',[208, 176],[104, 100],0.3,false);
export const draw=(p,t,o)=>drawKind('land',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('land',p,t,o,true);
