import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('sky-lance','sky-lance',[80, 208],[40, 152],0.32,false);
export const draw=(p,t,o)=>drawKind('sky-lance',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('sky-lance',p,t,o,true);
