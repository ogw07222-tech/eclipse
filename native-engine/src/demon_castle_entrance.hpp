#pragma once
// Optional ECLIPSE tutorial castle-entrance art module.
// Decorative only; physics floor remains the engine's y=597 collision surface.
// One 512x135 PNG is repeated across the world; no duplicate texture files.
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <array>
#include <algorithm>
#include <cstddef>
#include <cmath>
#include <string>
namespace eclipse {
class DemonCastleEntrance {
    SDL_Renderer* renderer_=nullptr;
    std::array<SDL_Texture*,5> images_{};
    bool ready_=false;
public:
    DemonCastleEntrance()=default;
    ~DemonCastleEntrance(){clear();}
    DemonCastleEntrance(const DemonCastleEntrance&)=delete;
    DemonCastleEntrance& operator=(const DemonCastleEntrance&)=delete;
    void clear() {
        for(auto*& image:images_) {if(image)SDL_DestroyTexture(image);image=nullptr;}
        ready_=false;
    }
    template<class Resolve>
    bool load(SDL_Renderer* renderer, Resolve resolve) {
        clear();renderer_=renderer;
        constexpr const char* paths[]={
            "demon_castle/entrance/sky_far.png",
            "demon_castle/entrance/far_castle.png",
            "demon_castle/entrance/mid_ruins.png",
            "demon_castle/entrance/near_gate.png",
            "demon_castle/entrance/floor_repeat_512.png"
        };
        constexpr int widths[]={1280,1280,1600,1600,512};
        constexpr int heights[]={720,720,720,720,135};
        for(size_t i=0;i<images_.size();++i) {
            const std::string path=resolve(paths[i]);
            SDL_Surface* surface=IMG_Load(path.c_str());
            if(!surface) {const std::string reason=SDL_GetError();clear();SDL_SetError("ECLIPSE ASSET ERROR: cannot load %s: %s",path.c_str(),reason.c_str());return false;}
            const bool shapeOK=surface->w==widths[i]&&surface->h==heights[i];
            if(!shapeOK){const int actualW=surface->w,actualH=surface->h;SDL_DestroySurface(surface);clear();SDL_SetError("ECLIPSE ASSET ERROR: %s has size %dx%d; expected %dx%d",path.c_str(),actualW,actualH,widths[i],heights[i]);return false;}
            images_[i]=SDL_CreateTextureFromSurface(renderer_,surface);
            SDL_DestroySurface(surface);
            if(!images_[i]){const std::string reason=SDL_GetError();clear();SDL_SetError("ECLIPSE ASSET ERROR: texture creation failed for %s: %s",path.c_str(),reason.c_str());return false;}
            SDL_SetTextureScaleMode(images_[i],SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(images_[i],SDL_BLENDMODE_BLEND);
        }
        ready_=true;return true;
    }
    bool ready() const{return ready_;}
    void draw(float camera) const {
        if(!ready_)return;
        const float travel=std::round(camera-640.f);
        const float factors[]={0.f,.18f,.42f,1.f};
        for(int i=0;i<4;++i) {
            float w= i<2?1280.f:1600.f;
            float x=std::round(-travel*factors[i]);
            SDL_FRect destination{x,0.f,w,720.f};
            SDL_RenderTexture(renderer_,images_[i],nullptr,&destination);
        }
        // Floor texture is authored for the same y=585 visible surface as the forest.
        // Draw one texture repeatedly in world space; no new platform collisions.
        for(int worldX=0;worldX<1600;worldX+=512) {
            const float w=static_cast<float>(std::min(512,1600-worldX));
            SDL_FRect source{0.f,0.f,w,135.f};
            SDL_FRect target{std::round(worldX-travel),585.f,w,135.f};
            SDL_RenderTexture(renderer_,images_[4],&source,&target);
        }
    }
};
} // namespace eclipse
