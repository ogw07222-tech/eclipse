import {makeMeta,drawKind} from './_lib.js';
export const meta=makeMeta('jump-takeoff','jump',[208, 176],[104, 100],0.25,false);
export const draw=(p,t,o)=>drawKind('jump',p,t,o,false);
export const drawBack=(p,t,o)=>drawKind('jump',p,t,o,true);
