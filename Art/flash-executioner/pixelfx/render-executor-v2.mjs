// Exporter/registration only. No geometry drawing; imports unchanged upstream recipes.
import {readFileSync,writeFileSync,mkdirSync,copyFileSync} from 'node:fs';
import {join} from 'node:path';
import {loadEffect,renderFrames,spriteSheet,contactSheet,save} from './lib/fxlib.mjs';
import {createHash} from 'node:crypto';
import {encodePNG} from './lib/png.mjs';
function packStraightRGBA(r,cols){const W=cols*r.w,H=Math.ceil(r.n/cols)*r.h,bytes=new Uint8ClampedArray(W*H*4);r.frames.forEach((f,i)=>{const X=i%cols*r.w,Y=Math.floor(i/cols)*r.h;for(let y=0;y<r.h;y++)bytes.set(f.clear.subarray(y*r.w*4,(y+1)*r.w*4),((Y+y)*W+X)*4);});return encodePNG(W,H,bytes);}
const OUT=process.env.PIXELFX_EXPORT_DIR||'/storage/emulated/0/Download/flash executioner/candidates/pixelfx_v2';
const GOLD=['#2b1b38','#594047','#95652e','#d49d38','#f4c75e','#ffe8ab','#fff6d9','#ffffff'];
const actor=JSON.parse(readFileSync(join(OUT,'actor-input.json'),'utf8'));actor.pixels=new Uint32Array(actor.pixels);actor.mask.data=new Uint8Array(actor.mask.data);
function actorFor(role){const a=JSON.parse(readFileSync(join(OUT,'actor-input-'+role+'.json'),'utf8'));a.pixels=new Uint32Array(a.pixels);a.mask.data=new Uint8Array(a.mask.data);return a;}
const configs=[
 {role:'Slash',id:'melee/sword-slash',options:{radius:60,sparks:['#ffffff','#fff5c4','#f7d47c','#b78f46']},canvas:[256,216],pivot:[105,165],duration:.4,span:[0,.75],scale:2.,focal:[6,-26],mode:'actor-feet',peak:.06},
 {role:'Rush',id:'melee/thrust',options:{reach:80,sparks:['#ffffff','#fff5c4','#f7d47c','#b78f46']},canvas:[320,160],pivot:[80,110],duration:.35,span:[0,.75],scale:2.,focal:[9,-20],mode:'actor-feet',peak:.07},
 {role:'Command',id:'arcane/spell-charge',options:{},canvas:[160,144],pivot:[64,124],duration:.4,span:[.3,.7],scale:2.1,focal:[15,-22],mode:'actor-feet',peak:.5,alphaEdge:[.025,.06]},
 {role:'BlinkOut',id:'arcane/blink-out',options:{actor:actorFor('BlinkOut')},canvas:[288,224],pivot:[144,174],duration:.32,span:[0,1.6],scale:1.4,focal:[0,0],mode:'actor-feet',peak:.32},
 {role:'BlinkIn',id:'arcane/blink-in',options:{actor:actorFor('BlinkIn')},canvas:[288,224],pivot:[144,174],duration:.36,span:[0,1.4],scale:1.4,focal:[0,0],mode:'actor-feet',peak:.48},
 {role:'Verdict',id:'holy/holy-pillar',options:{shafts:7},canvas:[256,280],pivot:[128,230],duration:.728,span:[0,3.5],scale:1.7,focal:[0,0],mode:'actor-feet',peak:.66},
 {role:'Impact',id:'impacts/crit-hit',options:{height:0},canvas:[320,224],pivot:[130,112],duration:.5,span:[0,.9],scale:1.7,focal:[0,0],mode:'impact-center',peak:.025},
 {role:'Orb',id:'arcane/arcane-orb',options:{},canvas:[136,112],pivot:[68,104],duration:2.4,span:[0,2.4],loop:true,scale:.3,focal:[0,-52],mode:'effect-focal',peak:0},
 {role:'Lance',id:'ice/ice-lance',options:{length:48},canvas:[208,96],pivot:[170,48],duration:.8,span:[0,.8],loop:true,scale:1.3,focal:[0,0],mode:'projectile-tip',peak:0},
 {role:'SkyLance',id:'ice/ice-lance',options:{length:48},canvas:[208,96],pivot:[170,48],duration:.8,span:[0,.8],loop:true,scale:1.8,focal:[0,0],mode:'projectile-tip',peak:0},
 {role:'LanceHit',id:'holy/smite',options:{stone:'#796449'},canvas:[288,232],pivot:[144,188],duration:.65,span:[.55,2.8],scale:1.5,focal:[0,0],mode:'target-ground',peak:.55},
 {role:'Dash',id:'status/haste',options:{actor:actorFor('Dash'),dust:'#d9c991'},canvas:[320,224],pivot:[190,174],duration:.25,span:[0,1.2],scale:1.4,focal:[0,0],mode:'actor-feet',peak:0,alphaEdge:[.016,.04]},
 {role:'Jump',id:'impacts/heavy-landing',options:{stone:'#86745f'},canvas:[208,128],pivot:[104,88],duration:.25,span:[0,.65],scale:.55,focal:[0,0],mode:'actor-feet',peak:.01,alphaEdge:[.008,.05]},
 {role:'Land',id:'impacts/heavy-landing',options:{stone:'#86745f'},canvas:[208,128],pivot:[104,88],duration:.3,span:[0,1.6],scale:1.,focal:[0,0],mode:'actor-feet',peak:.01},
 {role:'Swap',id:'arcane/mana-burst',options:{},canvas:[256,224],pivot:[128,174],duration:.4,span:[0,1.8],scale:1.6,focal:[0,-28],mode:'actor-feet',peak:.22},
];
const configsJson=configs.map(c=>({...c,options:{...c.options,...(c.options.actor?{actor:'actor-input-'+c.role+'.json'}:{})}}));writeFileSync(join(OUT,'options-registration.json'),JSON.stringify(configsJson,null,2));
const manifest=[];
for(const c of configs){
 const original=await loadEffect(c.id);const origHash=createHash('sha256').update(readFileSync(original.file)).digest('hex');
 const meta={...original.meta,size:c.canvas,origin:c.pivot,fps:60};const fx={...original,meta,mod:{...original.mod,meta}};
 const options={...c.options,palette:GOLD,light:.85};
 const r=renderFrames(fx,{fps:60,actor:false,options}); const baselineR=renderFrames(fx,{fps:60,actor:false,options:c.options.actor?{actor:c.options.actor}:{}});
 const count=c.loop?Math.round(c.duration*60):Math.round(c.duration*60)+1;
 const chosen=[];const baselineChosen=[];let indices=[];
 for(let i=0;i<count;i++){
  const u=c.loop?i/count:i/(count-1),sourceT=c.span[0]+(c.span[1]-c.span[0])*u,index=Math.max(0,Math.min(r.n-1,Math.round(sourceT*60)));
  const f=r.frames[index],clear=f.clear.slice();let alpha=1;
  if(c.alphaEdge){let t=i/60;alpha=Math.min(1,t/c.alphaEdge[0],(c.duration-t)/c.alphaEdge[1]);alpha=Math.max(0,alpha);for(let k=3;k<clear.length;k+=4)clear[k]=Math.round(clear[k]*alpha);}
  chosen.push({...f,t:i/60,clear});baselineChosen.push({...baselineR.frames[index],t:i/60});indices.push({output:i,sourceFrame:index,sourceTime:sourceT,alphaEnvelope:alpha});
 }
 const outputR={...r,n:count,fps:60,frames:chosen};const outputMeta={...meta,duration:c.duration,loop:!!c.loop};const outputFx={...fx,id:'executor-v2/'+c.role.toLowerCase(),meta:outputMeta,mod:{...fx.mod,meta:outputMeta}};
 const dest=join(OUT,'variants',c.role);mkdirSync(dest,{recursive:true});const s=spriteSheet(outputFx,outputR,c.role);s.png=packStraightRGBA(outputR,s.json.meta.pixelfx.columns);save(join(dest,c.role+'.png'),s.png);writeFileSync(join(dest,c.role+'.json'),JSON.stringify(s.json,null,1));writeFileSync(join(dest,'frame-map.json'),JSON.stringify(indices,null,1));
 const beforeR={...outputR,frames:baselineChosen};const before=spriteSheet(outputFx,beforeR,'original-60fps');before.png=packStraightRGBA(beforeR,before.json.meta.pixelfx.columns);save(join(dest,'original-60fps.png'),before.png);writeFileSync(join(dest,'original-60fps.json'),JSON.stringify(before.json,null,1));
 const source=join(OUT,'source-original','fx',c.id+'.js');mkdirSync(join(source,'..'),{recursive:true});copyFileSync(original.file,source);
 const peakFrame=Math.max(0,indices.findIndex(f=>f.sourceFrame>=Math.ceil(c.peak*60-1e-6)));const visualPeak=peakFrame/60;
 const row={role:c.role,file:join(dest,c.role+'.png'),cell:c.canvas,pivot:c.pivot,scale:c.scale,count,columns:s.json.meta.pixelfx.columns,rows:Math.ceil(count/s.json.meta.pixelfx.columns),duration:c.duration,fps:60,loop:!!c.loop,layer:c.role==='Verdict'?'back':'front',nativeDirectionDegrees:0,sourceFocalOffset:c.focal,effectfocalOffset:c.focal,alignmentMode:c.mode,anchorMode:c.mode==='target-ground'?'target-feet':c.mode,offset:c.role==='Orb'?[0,52*c.scale]:c.role==='Command'?[10,-25]:[0,0],visualPeakSeconds:visualPeak,triggerLeadSeconds:['Slash','Rush','Command'].includes(c.role)?visualPeak:0,spawnAgeSeconds:['Impact','BlinkIn'].includes(c.role)?visualPeak:0,sourceId:c.id,sourceCommit:'5294518ce7c72e48b6100a744596e0acf57f2826',sourceSha256:origHash,sourceTimeWindow:c.span,optionsDiff:{palette:GOLD,...c.options,...(c.options.actor?{actor:'actor-input-'+c.role+'.json'}:{}),light:.85},postprocess:c.alphaEdge?{alphaEdgeSeconds:c.alphaEdge,operation:'straight RGBA direct-copy atlas packing, alpha edge multiplication only; fixes upstream canvas.paste double-premultiply; no painted pixels or new shapes'}:{operation:'frame selection, straight RGBA direct-copy atlas packing only; fixes upstream canvas.paste double-premultiply; no painted pixels or new shapes'},frameMap:join(dest,'frame-map.json'),sha256:createHash('sha256').update(s.png).digest('hex'),baselinePng:join(dest,'original-60fps.png'),baselineFps:60,status:'candidate awaiting main review'};
 if(c.role==='BlinkOut')row.triggerStartAgeSeconds=.15-Math.max(0,indices.findIndex(f=>f.sourceFrame>=Math.ceil(.52*60)))/60;
 if(c.loop&&c.role!=='Orb')row.flightFrameFps=60;
 manifest.push(row);console.log(c.role,original.meta.size+' -> '+c.canvas,count,'frames',origHash.slice(0,12));
}
writeFileSync(join(OUT,'manifest.json'),JSON.stringify({effects:manifest,sourceOnly:true,newGeometryRecipes:false,status:'candidate'},null,2));
