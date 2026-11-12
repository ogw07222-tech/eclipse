import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('light-impact','impact',[224, 200],[112, 130],0.5,false);
export const draw=(p,t,o)=>drawKind('impact',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('impact',p,t,o,true);
