#pragma once
// ECLIPSE fixed-map demon castle: five layered backgrounds and one-way exit portals.
// Portal anchors share the hand-authored CastleTerrain world coordinates.
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include "castle_terrain.hpp"

namespace eclipse {
class DemonCastleEntrance {
    SDL_Renderer* renderer_=nullptr;
    std::array<SDL_Texture*,5> images_{};
    std::array<SDL_Texture*,5> floors_{};
    SDL_Texture* portal_=nullptr;
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
        for(auto*& p:floors_){if(p)SDL_DestroyTexture(p);p=nullptr;}
        if(portal_){SDL_DestroyTexture(portal_);portal_=nullptr;}
        ready_=false;active_=0;
    }
    template<class Resolve>
    bool load(SDL_Renderer* renderer,Resolve resolve){
        clear();renderer_=renderer;
        auto loadLayer=[&](const std::string& path,SDL_Texture*& texture,SDL_BlendMode blend){
            SDL_Surface* surface=IMG_Load(path.c_str());
            if(!surface){
                const std::string reason=SDL_GetError();
                SDL_SetError("ECLIPSE ASSET ERROR: cannot load %s: %s",path.c_str(),reason.c_str());
                return false;
            }
            if(surface->w!=2172||surface->h!=724){
                const int w=surface->w,h=surface->h;
                SDL_DestroySurface(surface);
                SDL_SetError("ECLIPSE ASSET ERROR: %s has size %dx%d; expected 2172x724",path.c_str(),w,h);
                return false;
            }
            texture=SDL_CreateTextureFromSurface(renderer_,surface);
            SDL_DestroySurface(surface);
            if(!texture||!SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST)||
               !SDL_SetTextureBlendMode(texture,blend)){
                const std::string reason=SDL_GetError();
                SDL_SetError("ECLIPSE ASSET ERROR: cannot prepare %s: %s",path.c_str(),reason.c_str());
                return false;
            }
            return true;
        };
        for(size_t i=0;i<files_.size();++i){
            const std::string name=files_[i];
            const std::string floor=name.substr(0,name.size()-4)+"_floor.png";
            if(!loadLayer(resolve(files_[i]),images_[i],SDL_BLENDMODE_NONE)||
               !loadLayer(resolve(floor.c_str()),floors_[i],SDL_BLENDMODE_BLEND)){
                const std::string reason=SDL_GetError();
                clear();SDL_SetError("%s",reason.c_str());
                return false;
            }
        }
        const std::string portalPath=resolve("demon_castle/blue_portal.png");
        SDL_Surface* portalSurface=IMG_Load(portalPath.c_str());
        if(!portalSurface){
            const std::string reason=SDL_GetError();clear();
            SDL_SetError("ECLIPSE ASSET ERROR: cannot load %s: %s",portalPath.c_str(),reason.c_str());
            return false;
        }
        if(portalSurface->w!=96||portalSurface->h!=128){
            const int actualW=portalSurface->w,actualH=portalSurface->h;SDL_DestroySurface(portalSurface);clear();
            SDL_SetError("ECLIPSE ASSET ERROR: %s has size %dx%d; expected 96x128",portalPath.c_str(),actualW,actualH);
            return false;
        }
        portal_=SDL_CreateTextureFromSurface(renderer_,portalSurface);SDL_DestroySurface(portalSurface);
        if(!portal_){
            const std::string reason=SDL_GetError();clear();
            SDL_SetError("ECLIPSE ASSET ERROR: cannot create texture for %s: %s",portalPath.c_str(),reason.c_str());
            return false;
        }
        SDL_SetTextureScaleMode(portal_,SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(portal_,SDL_BLENDMODE_BLEND);
        ready_=true;return true;
    }
    bool ready() const{return ready_ && images_[active_]!=nullptr;}
    bool setArea(int number){
        if(number<1||number>5){SDL_SetError("ECLIPSE ASSET ERROR: invalid castle area %d",number);return false;}
        if(!ready_||!images_[number-1]){SDL_SetError("ECLIPSE ASSET ERROR: castle area %d is not loaded",number);return false;}
        active_=number-1;return true;
    }
    int area() const{return active_+1;}
    int portalSide(float x,float radius=92.f) const{
        if(!ready())return -1;
        const float exit=CastleTerrain::portalX(area(),true);
        return exit>=0.f&&std::abs(x-exit)<=radius?1:-1;
    }
    void draw(float camera,float time=0.f) const{
        if(!ready())return;
        const float width=CastleTerrain::profile(area()).width;
        const float shift=std::clamp(std::round(camera-640.f),0.f,width-1280.f);
        // Keep the near lip at the same world X as the collision surface.
        constexpr float rearSpeed[]={.60f,.81f,.42f,.52f,.65f};
        const float rearY=area()==5?-112.f:0.f; // expose the throne seat above the arena lip
        const SDL_FRect rear{-std::round(shift*rearSpeed[active_]),rearY,width,720};
        SDL_RenderTexture(renderer_,images_[active_],nullptr,&rear);
        const SDL_FRect floor{-shift,0,width,720};
        SDL_RenderTexture(renderer_,floors_[active_],nullptr,&floor);
        const Uint8 alpha=static_cast<Uint8>(214.f+28.f*(.5f+.5f*std::sin(time*2.2f)));
        SDL_SetTextureAlphaMod(portal_,alpha);
        const float x=CastleTerrain::portalX(area(),true);
        if(x>=0.f){
            const float y=CastleTerrain::groundAt(area(),x);
            if(std::isfinite(y)){
                const SDL_FRect portalRect{x-shift-48.f,y-128.f,96.f,128.f};
                SDL_RenderTexture(renderer_,portal_,nullptr,&portalRect);
            }
        }
        SDL_SetTextureAlphaMod(portal_,255);
    }
};
} // namespace eclipse
