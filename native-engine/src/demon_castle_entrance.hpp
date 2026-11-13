#pragma once
// ECLIPSE fixed-map demon castle: five single-image backgrounds and paired blue portals.
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
    std::array<float,5> widths_{};
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
        if(portal_){SDL_DestroyTexture(portal_);portal_=nullptr;}
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
            SDL_SetTextureScaleMode(images_[i],SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(images_[i],SDL_BLENDMODE_NONE);
            widths_[i]=static_cast<float>(expectedW[i])*720.f/static_cast<float>(expectedH[i]);
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
        // Preserve the engine's 1280x720 reference viewport and world camera.
        // No parallax, no background tiling, no procedural scenery.
        const float travel=std::max(0.f,std::round(camera-640.f));
        const float width=widths_[active_];
        const float maxOffset=std::max(0.f,width-1280.f);
        const float shift=std::min(travel,maxOffset);
        const SDL_FRect destination{-shift,0.f,width,720.f};
        SDL_RenderTexture(renderer_,images_[active_],nullptr,&destination);
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
