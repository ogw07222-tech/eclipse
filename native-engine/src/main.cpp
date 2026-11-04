#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "presentation.hpp"
#include <memory>
#include <fstream>
#ifdef SDL_PLATFORM_ANDROID
#include <jni.h>
#endif

namespace {
SDL_Window* window=nullptr;SDL_Renderer* renderer=nullptr;
eclipse::Combat game;eclipse::Animation animation;eclipse::Input input;eclipse::ControlLayout layout;
bool running=true,foreground=true,titleSettings=false;float camera=640;
void requestVacodeLogin(){
#ifdef SDL_PLATFORM_ANDROID
    auto* env=static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    auto activity=static_cast<jobject>(SDL_GetAndroidActivity());
    if(!env||!activity){SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"VACODE JNI unavailable");return;}
    jclass clazz=env->GetObjectClass(activity);
    if(clazz){
        jmethodID method=env->GetMethodID(clazz,"showVacodeLogin","()V");
        if(method)env->CallVoidMethod(activity,method);
        if(env->ExceptionCheck()){env->ExceptionDescribe();env->ExceptionClear();}
        env->DeleteLocalRef(clazz);
    }
    env->DeleteLocalRef(activity);
#else
    SDL_Log("VACODE login is available in Android APK only");
#endif
}
std::array<bool,SDL_SCANCODE_COUNT> keyState{};
void updateKeyboard(){const auto& k=keyState;eclipse::Actions a;
        a.left=k[SDL_SCANCODE_A]||k[SDL_SCANCODE_LEFT];a.right=k[SDL_SCANCODE_D]||k[SDL_SCANCODE_RIGHT];a.jump=k[SDL_SCANCODE_SPACE]||k[SDL_SCANCODE_W]||k[SDL_SCANCODE_UP];
        a.attack=k[SDL_SCANCODE_J]||k[SDL_SCANCODE_Z];a.skill=k[SDL_SCANCODE_K]||k[SDL_SCANCODE_X];a.swap=k[SDL_SCANCODE_L]||k[SDL_SCANCODE_C]||k[SDL_SCANCODE_TAB];
        a.skillSweep=k[SDL_SCANCODE_U];a.skillBuff=k[SDL_SCANCODE_O];
        a.dash=k[SDL_SCANCODE_LSHIFT]||k[SDL_SCANCODE_RSHIFT];a.restart=k[SDL_SCANCODE_R];a.inventory=k[SDL_SCANCODE_I];a.weapon=k[SDL_SCANCODE_1];a.armor=k[SDL_SCANCODE_2];a.charm=k[SDL_SCANCODE_3];input.keyboard(a);
}
void events(bool& titleActive,float titleSeconds){
    const bool wasTitle=titleActive;
    SDL_Event e;
    while(!layout.exitRequested&&SDL_PollEvent(&e)){
        if(e.type==SDL_EVENT_QUIT){running=false;continue;}
        if(e.type==SDL_EVENT_WINDOW_FOCUS_LOST||e.type==SDL_EVENT_WILL_ENTER_BACKGROUND){foreground=false;input.reset();keyState.fill(false);continue;}
        if(e.type==SDL_EVENT_WINDOW_FOCUS_GAINED||e.type==SDL_EVENT_DID_ENTER_FOREGROUND){foreground=true;continue;}
        if(wasTitle){
            if(!foreground)continue;
            if(e.type==SDL_EVENT_KEY_DOWN&&e.key.scancode==SDL_SCANCODE_ESCAPE){
                running=false;continue;
            }
            if(titleSeconds<eclipse::Presentation::titleReadyTime)continue;
            int selection=-1;
            if(e.type==SDL_EVENT_KEY_DOWN&&!e.key.repeat){
                if(e.key.scancode==SDL_SCANCODE_RETURN||e.key.scancode==SDL_SCANCODE_KP_ENTER||e.key.scancode==SDL_SCANCODE_SPACE)selection=0;
                if(e.key.scancode==SDL_SCANCODE_ESCAPE){titleSettings=false;continue;}
            }
            const bool finger=e.type==SDL_EVENT_FINGER_DOWN&&e.tfinger.touchID!=SDL_MOUSE_TOUCHID;
            const bool mouse=e.type==SDL_EVENT_MOUSE_BUTTON_DOWN&&e.button.which!=SDL_TOUCH_MOUSEID&&e.button.button==SDL_BUTTON_LEFT;
            if((finger||mouse)&&SDL_ConvertEventToRenderCoordinates(renderer,&e)){
                const SDL_FPoint position=finger?SDL_FPoint{e.tfinger.x,e.tfinger.y}:SDL_FPoint{e.button.x,e.button.y};
                for(int i=0;i<4;++i)if(SDL_PointInRectFloat(&position,&eclipse::Presentation::titleButtons[i]))selection=i;
            }
            switch(selection){
                case 0: titleActive=false;titleSettings=false;break;
                case 1: requestVacodeLogin();break;
                case 2: titleSettings=!titleSettings;break;
                case 3: running=false;break;
                default:break;
            }
            continue;
        }
        if((e.type==SDL_EVENT_KEY_DOWN||e.type==SDL_EVENT_KEY_UP)&&e.key.scancode==SDL_SCANCODE_R)
            SDL_Log("ECLIPSE restart key type=%u foreground=%d",static_cast<unsigned>(e.type),foreground);
        if(foreground&&(e.type==SDL_EVENT_KEY_DOWN||e.type==SDL_EVENT_KEY_UP)&&e.key.scancode>=0&&e.key.scancode<SDL_SCANCODE_COUNT){
            keyState[e.key.scancode]=e.type==SDL_EVENT_KEY_DOWN;updateKeyboard();
        }
        if(e.type==SDL_EVENT_KEY_DOWN&&e.key.scancode==SDL_SCANCODE_ESCAPE){if(layout.editing){layout.close();input.reset();}else running=false;continue;}
        if(e.type==SDL_EVENT_FINGER_DOWN||e.type==SDL_EVENT_FINGER_MOTION||e.type==SDL_EVENT_FINGER_UP){
            if(e.tfinger.touchID==SDL_MOUSE_TOUCHID)continue;
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e)){
                bool wasEditing=layout.editing;
                bool consumed=layout.pointer(e.tfinger.fingerID,e.tfinger.x,e.tfinger.y,e.type!=SDL_EVENT_FINGER_UP,e.type==SDL_EVENT_FINGER_MOTION);
                if(consumed){if(!wasEditing||!layout.editing)input.reset();}
                else input.pointer(e.tfinger.fingerID,e.tfinger.x,e.tfinger.y,e.type!=SDL_EVENT_FINGER_UP,game.ended,game.inventory);
            }
        }else if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN||e.type==SDL_EVENT_MOUSE_BUTTON_UP){
            if(e.button.which==SDL_TOUCH_MOUSEID||e.button.button!=SDL_BUTTON_LEFT)continue;
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e)){
                bool wasEditing=layout.editing;
                bool consumed=layout.pointer(-1,e.button.x,e.button.y,e.type==SDL_EVENT_MOUSE_BUTTON_DOWN);
                if(consumed){if(!wasEditing||!layout.editing)input.reset();}
                else input.pointer(-1,e.button.x,e.button.y,e.type==SDL_EVENT_MOUSE_BUTTON_DOWN,game.ended,game.inventory);
            }
        }else if(e.type==SDL_EVENT_MOUSE_MOTION&&e.motion.which!=SDL_TOUCH_MOUSEID&&(e.motion.state&SDL_BUTTON_LMASK)){
            if(SDL_ConvertEventToRenderCoordinates(renderer,&e)){
                if(!layout.pointer(-1,e.motion.x,e.motion.y,true,true))
                    input.pointer(-1,e.motion.x,e.motion.y,true,game.ended,game.inventory);
            }
        }
    }

}
void tick(eclipse::Actions action){
    if(action.restart){input.reset();keyState.fill(false);animation.reset();camera=640;}
    const int previousMotion=animation.state;
    game.step(static_cast<float>(eclipse::fixedStep),action);animation.step(game.player,static_cast<float>(eclipse::fixedStep));
    if(animation.state!=previousMotion)SDL_Log("ECLIPSE motion state=%d frame=%d",animation.state,animation.frame);
    if(action.inventory||action.swap||action.restart||action.weapon||action.armor||action.charm)
        SDL_Log("ECLIPSE state form=%d inventory=%d hp=%d foes=%d restart=%d weapon=%d armor=%d charm=%d",game.player.form,game.inventory,game.player.health,game.foes(),action.restart,game.player.gear.selected[0],game.player.gear.selected[1],game.player.gear.selected[2]);
    camera+=(std::clamp(game.player.x,640.f,960.f)-camera)*(1-std::exp(-5*static_cast<float>(eclipse::fixedStep)));
}
void cleanup(){if(renderer)SDL_DestroyRenderer(renderer);if(window)SDL_DestroyWindow(window);SDL_Quit();}
}
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
    if(char* pref=SDL_GetPrefPath("ECLIPSE","Native")){
        layout.storagePath=std::string(pref)+"controls.cfg";SDL_free(pref);
    }
    layout.load();input.setLayout(&layout);
    if(!view->load()){
        const std::string reason=SDL_GetError();
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"ECLIPSE ASSET ERROR: %s",reason.c_str());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"ECLIPSE ASSET ERROR",reason.c_str(),window);
        view.reset();cleanup();return 1;
    }
    // Castle assets are required only when entering the castle scene.
    // Never display an unrelated forest scene for an explicit castle request.
    if(argc>1 && std::string(argv[1])=="--castle-preview"){
        if(!view->demonCastleEntranceReady()){
            const std::string reason=SDL_GetError();
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"ECLIPSE ASSET ERROR: %s",reason.c_str());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"ECLIPSE CASTLE ASSET ERROR",reason.c_str(),window);
            view.reset();cleanup();return 1;
        }
        view->setDemonCastleEntrance(true);
    }
    SDL_Log("ECLIPSE ready: five UI/knight textures, three slime atlases, combat scene, 1280x720");
    if(argc>2&&std::string(argv[1])=="--slime-review"){
        // Simulate the real AI/physics, including the red slime's leap arc.
        std::array<eclipse::Combat,3> reviews;
        for(int kind=0;kind<3;++kind){auto& simulation=reviews[kind];
            for(auto& e:simulation.enemies)e.health=0;
            const float base=260.f+kind*360;
            simulation.enemies[0]=eclipse::Enemy(kind,base);
            simulation.enemies[0].attackTimer=2.4f;
            simulation.player.x=base-180;simulation.player.invulnerable=1000;
        }
        for(int n=0;n<210;++n){
            game.restart();game.player.x=1500;game.enemies[3].health=0;
            const float t=n/30.f;game.clock=t;
            for(int kind=0;kind<3;++kind){auto& simulation=reviews[kind];
                if(n==150)simulation.player.x=440.f+kind*360;
                simulation.step(1.f/30,{});game.enemies[kind]=simulation.enemies[0];
            }
            const std::string capture=std::string(argv[2])+"/slime-"+std::to_string(n)+".bmp";
            view->draw(game,animation,640,{},capture.c_str(),n<72?"SLIME MOVE":"BITE LEAP AND CRUSH");
        }
        view.reset();cleanup();return 0;
    }
    if(smoke){
        for(int scene=0;scene<8;++scene){game.restart();if(scene==1){game.player.form=1;game.hit(game.enemies[0],1,false,true);game.hit(game.enemies[0],1,true);}
            if(scene==2)game.inventory=true;
            if(scene==3){game.ended=true;game.won=true;}
            if(scene==4){game.ended=true;game.player.health=0;}
            if(scene==5||scene==7)game.player.facing=-1;
            if(scene>=6){animation.state=5;animation.frame=134;}
            else{animation.step(game.player,1.f/60);}
            const std::string capture=argc>2?std::string(argv[2])+"/scene-"+std::to_string(scene)+".bmp":"";
            view->draw(game,animation,camera,{},argc>2?capture.c_str():nullptr);
        }
        SDL_Log("PASS eight combat/UI/facing/trail scenes and all five PNG textures");view.reset();cleanup();return 0;
    }
    if(argc>2&&std::string(argv[1])=="--motion-review"){
        constexpr const char* names[]={"IDLE","RUN","JUMP","FALL","WEAK SLASH","STRONG SLASH","THRUST","DASH","HIT FLASH","DEATH","LAND","WALK","AIR SLAM"};
        int captureIndex=0;std::ofstream positions(std::string(argv[2])+"/positions.csv");positions<<"frame,x,y,motion\n";
        for(int motion=0;motion<13;++motion){
            game.restart();animation.reset();
            for(int n=0;n<90;++n){
                auto& p=game.player;const float dt=1.f/60;
                if(motion==1||motion==11){p.vx=motion==1?280:120;p.x+=p.vx*dt;}
                if(motion==2||motion==3||motion==7||motion==10){
                    if(n==0&&(motion==3||motion==10)){p.y-=motion==3?165:20;p.grounded=false;p.vy=motion==3?0:200;}
                    eclipse::Actions reviewAction{};reviewAction.jump=motion==2&&n==0;reviewAction.dash=motion==7&&(n==0||n==60);
                    game.step(dt,reviewAction);
                }
                if(motion>=4&&motion<=6){
                    if(n==0&&motion>4){p.combo=motion-4;p.comboWindow=1;}
                    eclipse::Actions reviewAction{};reviewAction.attack=n==12;game.step(dt,reviewAction);
                }
                if(motion==12){eclipse::Actions action{};action.jump=n==0;action.attack=n==12;game.step(dt,action);}
                if(motion==8){if(n==12)game.damagePlayer(9);game.step(dt,{});}
                if(motion==9&&n>=12)p.health=0;
                animation.step(p,dt);if(n%5)continue;
                char filename[64];std::snprintf(filename,sizeof(filename),"/frame-%04d.bmp",captureIndex++);
                const auto capture=std::string(argv[2])+filename;
                const float reviewCamera=std::clamp(p.x,640.f,960.f);
                positions<<captureIndex-1<<","<<p.x-reviewCamera+640<<","<<p.y<<","<<names[motion]<<"\n";
                view->draw(game,animation,reviewCamera,{},capture.c_str(),names[motion]);
            }
            SDL_Log("REVIEW %s complete",names[motion]);
        }
        game.restart();animation.reset();camera=640;game.player.x=450;game.enemies[0].x=520;
        for(int n=0;n<420;++n){
            eclipse::Actions action{};action.right=n<45;action.jump=n==15;action.dash=n==60;
            action.attack=n==95||n==120||n==150;action.skill=n==190||n==285;action.skillSweep=n==218;action.skillBuff=n==90;action.swap=n==245;action.restart=n==390;
            if(n==345)game.damagePlayer(999);
            tick(action);if(n%5)continue;
            char filename[64];std::snprintf(filename,sizeof(filename),"/frame-%04d.bmp",captureIndex++);
            const auto capture=std::string(argv[2])+filename;
            positions<<captureIndex-1<<","<<game.player.x-camera+640<<","<<game.player.y<<",COMBAT REPLAY\n";
            view->draw(game,animation,camera,{},capture.c_str(),"COMBAT REPLAY");
        }
        SDL_Log("REVIEW live combat replay complete");
        view.reset();cleanup();return 0;
    }
    bool titleActive=view->loadTitle();
    if(!titleActive){
        const std::string reason=SDL_GetError();
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"ECLIPSE ASSET ERROR: %s",reason.c_str());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"ECLIPSE TITLE ASSET ERROR",reason.c_str(),window);
        view.reset();cleanup();return 1;
    }
    float titleSeconds=0;
    Uint64 last=SDL_GetTicksNS();double accumulator=0;
    while(running){Uint64 start=SDL_GetTicksNS();events(titleActive,titleSeconds);
        if(!running||layout.exitRequested)break;
        if(!foreground){accumulator=0;SDL_Delay(50);last=SDL_GetTicksNS();continue;}
        const double elapsed=std::min((start-last)/1e9,.1);last=start;
        if(titleActive){
            titleSeconds+=static_cast<float>(elapsed);
            view->drawTitle(titleSeconds,titleSettings);
        }else{
            accumulator+=elapsed;
            while(accumulator>=eclipse::fixedStep){if(!layout.editing)tick(input.sample());accumulator-=eclipse::fixedStep;}
            view->draw(game,animation,camera,input.held(),nullptr,nullptr,&layout,input.stickOffset());
        }
        double remaining=eclipse::fixedStep-(SDL_GetTicksNS()-start)/1e9;if(remaining>0)SDL_DelayNS(static_cast<Uint64>(remaining*1e9));
    }
    view.reset();cleanup();return 0;
}
