#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <string>
#include "core.hpp"

namespace {
SDL_Window* window=nullptr;
SDL_Renderer* renderer=nullptr;
SDL_Texture* knight=nullptr;
eclipse::Actor player;
eclipse::Input input;
bool running=true, foreground=true, dirty=true;
void fill(float x,float y,float w,float h,Uint8 r,Uint8 g,Uint8 b,Uint8 a=255) {
    SDL_SetRenderDrawColor(renderer,r,g,b,a);
    SDL_FRect rect{x,y,w,h}; SDL_RenderFillRect(renderer,&rect);
}
void draw() {
    SDL_SetRenderDrawColor(renderer,15,24,32,255); SDL_RenderClear(renderer);
    fill(0,eclipse::floorY,eclipse::viewWidth,eclipse::viewHeight-eclipse::floorY,60,75,65);
    for(int i=0;i<10;++i) fill(i*54+10,eclipse::floorY+6,30,3,88,108,76);
    SDL_FRect source{static_cast<float>((player.frame%8)*48),static_cast<float>((player.frame/8)*64),48,64};
    SDL_FRect dest{player.x,player.y,48,64};
    SDL_RenderTextureRotated(renderer,knight,&source,&dest,0,nullptr,player.faceRight?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    fill(6,185,132,77,18,32,44,180);
    fill(18,205,44,43,92,113,128,180); fill(80,205,44,43,92,113,128,180);
    // Pixel arrows identify the controls without adding a font library.
    fill(28,224,25,5,225,232,224); fill(28,219,5,15,225,232,224);
    fill(89,224,25,5,225,232,224); fill(109,219,5,15,225,232,224);
    fill(203,207,70,50,65,89,112,160); fill(234,219,6,25,225,232,224); fill(228,219,18,5,225,232,224);
    fill(376,190,90,70,89,46,53,180); fill(417,205,5,34,225,232,224); fill(407,234,25,5,225,232,224);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
    SDL_RenderPresent(renderer);
}
void events() {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type==SDL_EVENT_QUIT) { running=false; continue; }
        if(e.type==SDL_EVENT_WINDOW_FOCUS_LOST || e.type==SDL_EVENT_WILL_ENTER_BACKGROUND) {
            foreground=false; input.reset(); continue;
        }
        if(e.type==SDL_EVENT_WINDOW_FOCUS_GAINED || e.type==SDL_EVENT_DID_ENTER_FOREGROUND) { foreground=true; dirty=true; continue; }
        if(e.type==SDL_EVENT_WINDOW_EXPOSED || e.type==SDL_EVENT_WINDOW_RESIZED || e.type==SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) dirty=true;
        if(e.type==SDL_EVENT_KEY_DOWN && e.key.key==SDLK_ESCAPE) { running=false; continue; }
        // Includes letterbox offsets, high-DPI and resized windows; normalized
        // touch coordinates become logical pixels after this conversion.
        if(e.type==SDL_EVENT_FINGER_DOWN || e.type==SDL_EVENT_FINGER_MOTION || e.type==SDL_EVENT_FINGER_UP) {
            if(e.tfinger.touchID==SDL_MOUSE_TOUCHID) continue;
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e))
                input.pointer(e.tfinger.fingerID,e.tfinger.x,e.tfinger.y,e.type!=SDL_EVENT_FINGER_UP);
        } else if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN || e.type==SDL_EVENT_MOUSE_BUTTON_UP) {
            if(e.button.which==SDL_TOUCH_MOUSEID || e.button.button!=SDL_BUTTON_LEFT) continue;
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e))
                input.pointer(-1,e.button.x,e.button.y,e.type==SDL_EVENT_MOUSE_BUTTON_DOWN);
        } else if(e.type==SDL_EVENT_MOUSE_MOTION && e.motion.which!=SDL_TOUCH_MOUSEID && (e.motion.state & SDL_BUTTON_LMASK)) {
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e)) input.pointer(-1,e.motion.x,e.motion.y,true);
        }
    }
    if(foreground) {
        const bool* keys=SDL_GetKeyboardState(nullptr);
        input.keys={keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT], keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT],
                    keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP], keys[SDL_SCANCODE_J] || keys[SDL_SCANCODE_Z]};
    }
}
void cleanup() {
    if(knight) SDL_DestroyTexture(knight);
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);
    SDL_Quit();
}
}
int main(int argc,char** argv) {
    const bool smoke=argc>1 && std::string(argv[1])=="--smoke-test";
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)) { SDL_Log("SDL init: %s",SDL_GetError()); return 1; }
    if(!SDL_CreateWindowAndRenderer("ECLIPSE Native",960,540,SDL_WINDOW_RESIZABLE,&window,&renderer)) {
        SDL_Log("window: %s",SDL_GetError()); cleanup(); return 1;
    }
    SDL_SetRenderLogicalPresentation(renderer,480,270,SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderVSync(renderer,1);
    const char* base=SDL_GetBasePath();
    const std::string atlas=std::string(base?base:"")+"assets/knight_sheet_48x64_v2.png";
    knight=IMG_LoadTexture(renderer,atlas.c_str());
    if(!knight) { SDL_Log("Required knight PNG missing or invalid: %s (%s)",atlas.c_str(),SDL_GetError()); cleanup(); return 1; }
    float atlasWidth=0,atlasHeight=0;
    if(!SDL_GetTextureSize(knight,&atlasWidth,&atlasHeight) || atlasWidth!=384 || atlasHeight!=448) {
        SDL_Log("Expected 384x448 knight atlas, got %.0fx%.0f",atlasWidth,atlasHeight); cleanup(); return 1;
    }
    SDL_SetTextureScaleMode(knight,SDL_SCALEMODE_NEAREST);
    if(smoke) { draw(); SDL_Log("PASS PNG decode and render (%s)",atlas.c_str()); cleanup(); return 0; }
    Uint64 last=SDL_GetTicksNS(); double accumulator=0;
    while(running) {
        const Uint64 start=SDL_GetTicksNS();
        events();
        if(!foreground) { accumulator=0; SDL_Delay(50); last=SDL_GetTicksNS(); continue; }
        accumulator+=std::min((start-last)/1e9,0.1); last=start;
        while(accumulator>=eclipse::fixedStep) {
            const auto before=player;
            player.step(static_cast<float>(eclipse::fixedStep),input.sample()); accumulator-=eclipse::fixedStep;
            dirty |= before.x!=player.x || before.y!=player.y || before.frame!=player.frame || before.faceRight!=player.faceRight;
        }
        if(dirty) { draw(); dirty=false; }
        const double remaining=eclipse::fixedStep-(SDL_GetTicksNS()-start)/1e9;
        if(remaining>0) SDL_DelayNS(static_cast<Uint64>(remaining*1e9));
    }
    cleanup(); return 0;
}
