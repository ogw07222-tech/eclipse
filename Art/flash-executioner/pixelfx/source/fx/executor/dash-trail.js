import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('dash-trail','dash',[256, 160],[150, 122],0.25,false);
export const draw=(p,t,o)=>drawKind('dash',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('dash',p,t,o,true);
