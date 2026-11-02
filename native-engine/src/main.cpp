#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace {
constexpr float VW=480.0f, VH=270.0f, floorY=225.0f;
constexpr float speed=135.0f, gravity=850.0f, jumpSpeed=-335.0f;
struct Actor { float x=150,y=floorY-64,vy=0; bool grounded=true,faceRight=true; int frame=0; float anim=0,attack=0; } player;
SDL_Window* window=nullptr;
SDL_Renderer* renderer=nullptr;
SDL_Texture* knight=nullptr;
bool running=true, left=false,right=false,jump=false,attack=false;
bool pointerDown=false;
float pointerX=0,pointerY=0;
void setPointer(float x,float y,bool down) {
 pointerDown=down;pointerX=x;pointerY=y;
 // UI bottom strip: left half movement, right quarter attack, center jump.
 left=down && x<135 && y>170 && x<75;
 right=down && x<135 && y>170 && x>=75;
 jump=down && x>185 && x<315 && y>195;
 attack=down && x>345 && y>160;
}
void fill(float x,float y,float w,float h,Uint8 r,Uint8 g,Uint8 b,Uint8 a=255) {
 SDL_SetRenderDrawColor(renderer,r,g,b,a);
 SDL_FRect rect{x,y,w,h};SDL_RenderFillRect(renderer,&rect);
}
void tryAsset(const char* path) {
 if(knight)return;
 SDL_Texture* t=IMG_LoadTexture(renderer,path);
 if(t){knight=t;SDL_SetTextureScaleMode(knight,SDL_SCALEMODE_NEAREST);}
}
void draw() {
 SDL_SetRenderLogicalPresentation(renderer,(int)VW,(int)VH,SDL_LOGICAL_PRESENTATION_LETTERBOX);
 SDL_SetRenderDrawColor(renderer,15,24,32,255); SDL_RenderClear(renderer);
 fill(0,floorY,VW,VH-floorY,60,75,65);
 for(int i=0;i<10;++i) fill(i*54+10,floorY+6,30,3,88,108,76);
 // Each source cell is 48x64; the existing atlas has eight columns.
 if(knight) {
   SDL_FRect source{static_cast<float>((player.frame%8)*48),0,48,64};
   SDL_FRect dest{player.x,player.y,48,64};
   SDL_FlipMode flip=player.faceRight?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL;
   SDL_RenderTextureRotated(renderer,knight,&source,&dest,0,nullptr,flip);
 } else {
   // Visible fallback if the source art is not packaged; no sprite is generated.
   fill(player.x+12,player.y+8,24,48,160,173,185);
 }
 // Attack indicator is intentionally separate from the character PNG.
 if(player.attack>0) fill(player.x+(player.faceRight?43:-21),player.y+22,24,5,210,229,255,180);
 SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
 fill(6,185,132,77,18,32,44,180);
 fill(18,205,44,43,92,113,128,180); fill(80,205,44,43,92,113,128,180);
 fill(203,207,70,50,65,89,112,160);
 fill(376,190,90,70,89,46,53,180);
 SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
 SDL_RenderPresent(renderer);
}
void step(float dt) {
 float axis=(right?1.0f:0.0f)-(left?1.0f:0.0f);
 if(axis!=0)player.faceRight=axis>0;
 player.x=std::clamp(player.x+axis*speed*dt,0.0f,VW-48.0f);
 if(jump&&player.grounded){player.vy=jumpSpeed;player.grounded=false;}
 player.vy+=gravity*dt;player.y+=player.vy*dt;
 if(player.y>=floorY-64){player.y=floorY-64;player.vy=0;player.grounded=true;}
 player.anim+=dt;
 if(player.anim>0.12f){player.anim=0;player.frame=(player.frame+1)%8;}
 if(attack&&player.attack<=0)player.attack=0.22f;
 player.attack=std::max(0.0f,player.attack-dt);
}
}
int main(int argc,char** argv) {
 (void)argc;(void)argv;
 if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)){SDL_Log("SDL init: %s",SDL_GetError());return 1;}
 if(!SDL_CreateWindowAndRenderer("ECLIPSE Engine Prototype",960,540,SDL_WINDOW_RESIZABLE,&window,&renderer)){
 SDL_Log("window: %s",SDL_GetError());SDL_Quit();return 1;}
 tryAsset("assets/knight_sheet_48x64_v2.png");
 tryAsset("../assets/knight_sheet_48x64_v2.png");
 Uint64 last=SDL_GetTicks();
 while(running) {
 SDL_Event e;
 while(SDL_PollEvent(&e)){
 switch(e.type){
 case SDL_EVENT_QUIT:running=false;break;
 case SDL_EVENT_KEY_DOWN:case SDL_EVENT_KEY_UP:{
 bool on=e.type==SDL_EVENT_KEY_DOWN;
 switch(e.key.key){
 case SDLK_A:case SDLK_LEFT:left=on;break;
 case SDLK_D:case SDLK_RIGHT:right=on;break;
 case SDLK_SPACE:case SDLK_W:case SDLK_UP:jump=on;break;
 case SDLK_J:attack=on;break;
 case SDLK_ESCAPE:if(on)running=false;break;
 default:break;}break;}
 case SDL_EVENT_FINGER_DOWN:case SDL_EVENT_FINGER_MOTION:
 setPointer(e.tfinger.x*VW,e.tfinger.y*VH,true);break;
 case SDL_EVENT_FINGER_UP:setPointer(0,0,false);break;
 case SDL_EVENT_MOUSE_BUTTON_DOWN:case SDL_EVENT_MOUSE_MOTION:
 if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN || (e.motion.state & SDL_BUTTON_LMASK)) {
 float mx=e.type==SDL_EVENT_MOUSE_BUTTON_DOWN?e.button.x:e.motion.x;
 float my=e.type==SDL_EVENT_MOUSE_BUTTON_DOWN?e.button.y:e.motion.y;
 float ww=0,hh=0; SDL_GetWindowSize(window,(int*)&ww,(int*)&hh);
 if(ww>0&&hh>0)setPointer(mx*VW/ww,my*VH/hh,true);
 }break;
 case SDL_EVENT_MOUSE_BUTTON_UP:setPointer(0,0,false);break;
 default:break;
 }
 }
 Uint64 now=SDL_GetTicks();float dt=std::min((now-last)/1000.f,0.033f);last=now;
 step(dt);draw();SDL_Delay(1);
 }
 if(knight)SDL_DestroyTexture(knight);
 SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 0;
}
