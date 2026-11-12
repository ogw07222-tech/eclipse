#pragma once
// ECLIPSE fixed-map demon castle: five independent, single-image backgrounds.
// Rendering-only replacement. Combat collision coordinates are NOT modified here.
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <string>

namespace eclipse {
class DemonCastleEntrance {
    SDL_Renderer* renderer_=nullptr;
    std::array<SDL_Texture*,5> images_{};
    std::array<float,5> widths_{};
    int active_=0;
    bool ready_=false;
    static constexpr std::array<const char*,5> files_{{
        "demon_castle/areas/a1_ashen_approach.png",
        "demon_castle/areas/a2_broken_gate.png",
        "demon_castle/areas/a3_void_corridor.png",
        "demon_castle/areas/a4_throne_stairway.png",
        "demon_castle/areas/a5_night_throne.png"
    }};
public:
    DemonCastleEntrance()=default;
    ~DemonCastleEntrance(){clear();}
    DemonCastleEntrance(const DemonCastleEntrance&)=delete;
    DemonCastleEntrance& operator=(const DemonCastleEntrance&)=delete;
    void clear(){
        for(auto*& p:images_){if(p)SDL_DestroyTexture(p);p=nullptr;}
        widths_.fill(0.f);ready_=false;active_=0;
    }
    template<class Resolve>
    bool load(SDL_Renderer* renderer,Resolve resolve){
        clear();renderer_=renderer;
        constexpr int expectedW[]={2172,2172,2172,2172,2172};
        constexpr int expectedH[]={724,724,724,724,724};
        for(int i=0;i<5;++i){
            const std::string path=resolve(files_[i]);
            SDL_Surface* surface=IMG_Load(path.c_str());
            if(!surface){
                const std::string reason=SDL_GetError();clear();
                SDL_SetError("ECLIPSE ASSET ERROR: cannot load %s: %s",path.c_str(),reason.c_str());
                return false;
            }
            const int actualW=surface->w,actualH=surface->h;
            if(actualW!=expectedW[i]||actualH!=expectedH[i]){
                SDL_DestroySurface(surface);clear();
                SDL_SetError("ECLIPSE ASSET ERROR: %s has size %dx%d; expected %dx%d",
                    path.c_str(),actualW,actualH,expectedW[i],expectedH[i]);
                return false;
            }
            images_[i]=SDL_CreateTextureFromSurface(renderer_,surface);
            SDL_DestroySurface(surface);
            if(!images_[i]){
                const std::string reason=SDL_GetError();clear();
                SDL_SetError("ECLIPSE ASSET ERROR: cannot create texture for %s: %s",path.c_str(),reason.c_str());
                return false;
            }
            SDL_SetTextureScaleMode(images_[i],SDL_SCALEMODE_LINEAR);
            SDL_SetTextureBlendMode(images_[i],SDL_BLENDMODE_NONE);
            widths_[i]=static_cast<float>(expectedW[i])*720.f/static_cast<float>(expectedH[i]);
        }
        ready_=true;return true;
    }
    bool ready() const{return ready_ && images_[active_]!=nullptr;}
    bool setArea(int number){
        if(number<1||number>5){SDL_SetError("ECLIPSE ASSET ERROR: invalid castle area %d",number);return false;}
        if(!ready_||!images_[number-1]){SDL_SetError("ECLIPSE ASSET ERROR: castle area %d is not loaded",number);return false;}
        active_=number-1;return true;
    }
    int area() const{return active_+1;}
    void draw(float camera) const{
        if(!ready())return;
        // Preserve the engine's 1280x720 reference viewport and world camera.
        // No parallax, no background tiling, no procedural scenery.
        const float travel=std::max(0.f,std::round(camera-640.f));
        const float width=widths_[active_];
        const float maxOffset=std::max(0.f,width-1280.f);
        const float shift=std::min(travel,maxOffset);
        const SDL_FRect destination{-shift,0.f,width,720.f};
        SDL_RenderTexture(renderer_,images_[active_],nullptr,&destination);
    }
};
} // namespace eclipse
