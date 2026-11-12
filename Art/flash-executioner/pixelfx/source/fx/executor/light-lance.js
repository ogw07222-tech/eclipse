import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('light-lance','lance',[192, 80],[90, 45],0.3,false);
export const draw=(p,t,o)=>drawKind('lance',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('lance',p,t,o,true);
