#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "presentation.hpp"
#include <memory>
#ifdef SDL_PLATFORM_ANDROID
#include <jni.h>
#endif

namespace {
SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
eclipse::Combat game;eclipse::Animation animation;eclipse::Input input;
bool running=true,foreground=true;float camera=640;
std::array<bool,SDL_SCANCODE_COUNT> keyState{};
void updateKeyboard(){const auto& k=keyState;eclipse::Actions a;
        a.left=k[SDL_SCANCODE_A]||k[SDL_SCANCODE_LEFT];a.right=k[SDL_SCANCODE_D]||k[SDL_SCANCODE_RIGHT];a.jump=k[SDL_SCANCODE_SPACE]||k[SDL_SCANCODE_W]||k[SDL_SCANCODE_UP];
        a.attack=k[SDL_SCANCODE_J]||k[SDL_SCANCODE_Z];a.skill=k[SDL_SCANCODE_K]||k[SDL_SCANCODE_X];a.swap=k[SDL_SCANCODE_L]||k[SDL_SCANCODE_C]||k[SDL_SCANCODE_TAB];
        a.dash=k[SDL_SCANCODE_LSHIFT]||k[SDL_SCANCODE_RSHIFT];a.restart=k[SDL_SCANCODE_R];a.inventory=k[SDL_SCANCODE_I];a.weapon=k[SDL_SCANCODE_1];a.armor=k[SDL_SCANCODE_2];a.charm=k[SDL_SCANCODE_3];input.keyboard(a);
}
void events(){
    SDL_Event e;
    while(SDL_PollEvent(&e)){
        if(e.type==SDL_EVENT_QUIT){running=false;continue;}
        if(e.type==SDL_EVENT_WINDOW_FOCUS_LOST||e.type==SDL_EVENT_WILL_ENTER_BACKGROUND){foreground=false;input.reset();keyState.fill(false);continue;}
        if(e.type==SDL_EVENT_WINDOW_FOCUS_GAINED||e.type==SDL_EVENT_DID_ENTER_FOREGROUND){foreground=true;continue;}
        if((e.type==SDL_EVENT_KEY_DOWN||e.type==SDL_EVENT_KEY_UP)&&e.key.scancode==SDL_SCANCODE_R)
            SDL_Log("ECLIPSE restart key type=%u foreground=%d",static_cast<unsigned>(e.type),foreground);
        if(foreground&&(e.type==SDL_EVENT_KEY_DOWN||e.type==SDL_EVENT_KEY_UP)&&e.key.scancode>=0&&e.key.scancode<SDL_SCANCODE_COUNT){
            keyState[e.key.scancode]=e.type==SDL_EVENT_KEY_DOWN;updateKeyboard();
        }
        if(e.type==SDL_EVENT_KEY_DOWN&&e.key.scancode==SDL_SCANCODE_ESCAPE){running=false;continue;}
        if(e.type==SDL_EVENT_FINGER_DOWN||e.type==SDL_EVENT_FINGER_MOTION||e.type==SDL_EVENT_FINGER_UP){
            if(e.tfinger.touchID==SDL_MOUSE_TOUCHID)continue;
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e))input.pointer(e.tfinger.fingerID,e.tfinger.x,e.tfinger.y,e.type!=SDL_EVENT_FINGER_UP,game.ended,game.inventory);
        }else if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN||e.type==SDL_EVENT_MOUSE_BUTTON_UP){
            if(e.button.which==SDL_TOUCH_MOUSEID||e.button.button!=SDL_BUTTON_LEFT)continue;
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e))input.pointer(-1,e.button.x,e.button.y,e.type==SDL_EVENT_MOUSE_BUTTON_DOWN,game.ended,game.inventory);
        }else if(e.type==SDL_EVENT_MOUSE_MOTION&&e.motion.which!=SDL_TOUCH_MOUSEID&&(e.motion.state&SDL_BUTTON_LMASK)){
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e))input.pointer(-1,e.motion.x,e.motion.y,true,game.ended,game.inventory);
        }
    }

}
void tick(eclipse::Actions action){
    if(action.restart){input.reset();keyState.fill(false);animation.reset();camera=640;}
    game.step(static_cast<float>(eclipse::fixedStep),action);animation.step(game.player,static_cast<float>(eclipse::fixedStep));
    if(action.inventory||action.swap||action.restart||action.weapon||action.armor||action.charm)
        SDL_Log("ECLIPSE state form=%d inventory=%d hp=%d foes=%d restart=%d weapon=%d armor=%d charm=%d",game.player.form,game.inventory,game.player.health,game.foes(),action.restart,game.player.gear.selected[0],game.player.gear.selected[1],game.player.gear.selected[2]);
    camera+=(std::clamp(game.player.x,640.f,960.f)-camera)*(1-std::exp(-5*static_cast<float>(eclipse::fixedStep)));
}
void cleanup(){if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}
}
#ifdef SDL_PLATFORM_ANDROID
// Android KeyEvent constants mapped to the same scancodes used on desktop.
// SDL_PushEvent is thread-safe; the simulation is still mutated only on its thread.
extern "C" JNIEXPORT jboolean JNICALL
Java_com_eclipse_nativegame_MainActivity_nativeOnGameKey(JNIEnv*,jclass,jint key,jboolean down){
    SDL_Scancode scan=SDL_SCANCODE_UNKNOWN;
    switch(key){
        case 29:scan=SDL_SCANCODE_A;break;case 32:scan=SDL_SCANCODE_D;break;case 51:scan=SDL_SCANCODE_W;break;
        case 38:scan=SDL_SCANCODE_J;break;case 54:scan=SDL_SCANCODE_Z;break;case 39:scan=SDL_SCANCODE_K;break;
        case 52:scan=SDL_SCANCODE_X;break;case 40:scan=SDL_SCANCODE_L;break;case 31:scan=SDL_SCANCODE_C;break;
        case 37:scan=SDL_SCANCODE_I;break;case 46:scan=SDL_SCANCODE_R;break;
        case 8:scan=SDL_SCANCODE_1;break;case 9:scan=SDL_SCANCODE_2;break;case 10:scan=SDL_SCANCODE_3;break;
        case 62:scan=SDL_SCANCODE_SPACE;break;case 61:scan=SDL_SCANCODE_TAB;break;
        case 59:scan=SDL_SCANCODE_LSHIFT;break;case 60:scan=SDL_SCANCODE_RSHIFT;break;
        case 21:scan=SDL_SCANCODE_LEFT;break;case 22:scan=SDL_SCANCODE_RIGHT;break;case 19:scan=SDL_SCANCODE_UP;break;
        case 111:scan=SDL_SCANCODE_ESCAPE;break;default:return JNI_FALSE;
    }
    SDL_Event e{};e.type=down?SDL_EVENT_KEY_DOWN:SDL_EVENT_KEY_UP;e.key.scancode=scan;e.key.down=down;
    return SDL_PushEvent(&e)?JNI_TRUE:JNI_FALSE;
}
#endif
int main(int argc,char** argv){
    bool smoke=argc>1&&std::string(argv[1])=="--smoke-test";
#ifdef SDL_PLATFORM_ANDROID
    SDL_SetHint(SDL_HINT_ORIENTATIONS,"LandscapeLeft LandscapeRight");
    const SDL_WindowFlags flags=SDL_WINDOW_FULLSCREEN;
#else
    const SDL_WindowFlags flags=SDL_WINDOW_RESIZABLE;
#endif
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)){SDL_Log("SDL init: %s",SDL_GetError());return 1;}
    if(!SDL_CreateWindowAndRenderer("ECLIPSE Native",1280,720,flags,&window,&renderer)){SDL_Log("Window: %s",SDL_GetError());cleanup();return 1;}
    SDL_SetRenderLogicalPresentation(renderer,1280,720,SDL_LOGICAL_PRESENTATION_LETTERBOX);SDL_SetRenderVSync(renderer,1);
    auto view=std::make_unique<eclipse::Presentation>(renderer);
    if(!view->load()){SDL_Log("ECLIPSE asset load failed: %s",SDL_GetError());view.reset();cleanup();return 1;}
    SDL_Log("ECLIPSE ready: five PNG textures, combat scene, 1280x720");
    if(smoke){
        for(int scene=0;scene<5;++scene){game.restart();if(scene==1){game.player.form=1;game.hit(game.enemies[0],1,false,true);game.hit(game.enemies[0],1,true);}
            if(scene==2)game.inventory=true;
            if(scene==3){game.ended=true;game.won=true;}
            if(scene==4){game.ended=true;game.player.health=0;}
            animation.step(game.player,1.f/60);const std::string capture=argc>2?std::string(argv[2])+"/scene-"+std::to_string(scene)+".bmp":"";
            view->draw(game,animation,camera,{},argc>2?capture.c_str():nullptr);
        }
        SDL_Log("PASS five combat/UI scenes and all five PNG textures");view.reset();cleanup();return 0;
    }
    Uint64 last=SDL_GetTicksNS();double accumulator=0;
    while(running){Uint64 start=SDL_GetTicksNS();events();
        if(!foreground){accumulator=0;SDL_Delay(50);last=SDL_GetTicksNS();continue;}
        accumulator+=std::min((start-last)/1e9,.1);last=start;
        while(accumulator>=eclipse::fixedStep){tick(input.sample());accumulator-=eclipse::fixedStep;}
        view->draw(game,animation,camera,input.held());
        double remaining=eclipse::fixedStep-(SDL_GetTicksNS()-start)/1e9;if(remaining>0)SDL_DelayNS(static_cast<Uint64>(remaining*1e9));
    }
    view.reset();cleanup();return 0;
}
