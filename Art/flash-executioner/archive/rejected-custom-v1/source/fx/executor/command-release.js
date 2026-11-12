import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('command-release','command',[224, 200],[112, 154],0.4,false);
export const draw=(p,t,o)=>drawKind('command',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('command',p,t,o,true);
