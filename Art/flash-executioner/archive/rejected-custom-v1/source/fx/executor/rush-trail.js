import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('rush-trail','rush',[320, 176],[190, 132],0.35,false);
export const draw=(p,t,o)=>drawKind('rush',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('rush',p,t,o,true);
