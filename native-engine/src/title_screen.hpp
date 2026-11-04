#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace eclipse {
class TitleScreen {
    static constexpr int columns=32, rows=24;
    static constexpr float artWidth=1536, artHeight=865;
    static constexpr float screenWidth=1280, screenHeight=720;
    SDL_Texture* art_=nullptr;
    std::array<SDL_Vertex,(columns+1)*(rows+1)> cloth_{};
    std::array<float,(columns+1)*(rows+1)> flex_{};
    std::array<int,columns*rows*6> triangles_{};

    static float smooth(float value) {
        const float t=std::clamp(value,0.f,1.f);
        return t*t*(3.f-2.f*t);
    }

public:
    static constexpr SDL_FRect startButton{536,629,208,48};
    TitleScreen()=default;
    TitleScreen(const TitleScreen&)=delete;
    TitleScreen& operator=(const TitleScreen&)=delete;
    ~TitleScreen() { clear(); }

    void clear() {
        if(art_) SDL_DestroyTexture(art_);
        art_=nullptr;
    }

    bool load(SDL_Renderer* renderer,const char* path) {
        art_=IMG_LoadTexture(renderer,path);
        if(!art_) return false;
        SDL_SetTextureScaleMode(art_,SDL_SCALEMODE_LINEAR);
        SDL_SetTextureBlendMode(art_,SDL_BLENDMODE_NONE);
        for(int row=0;row<=rows;++row) {
            for(int column=0;column<=columns;++column) {
                const int index=row*(columns+1)+column;
                const float u=static_cast<float>(column)/columns;
                const float v=static_cast<float>(row)/rows;
                const float x=u*artWidth, y=v*artHeight;
                cloth_[index]={{u*screenWidth,v*screenHeight},{1,1,1,1},{u,v}};
                // Pin the armor, left apparition and ground. The soft edges keep
                // the moving fabric connected to the flattened background.
                const float upperEdge=std::max(322.f,540.f-(x-270.f)*.51f);
                const float rightEdge=std::min(763.f,762.f-(y-390.f)*.43f);
                flex_[index]=smooth((x-258.f)/105.f)*smooth((rightEdge-x)/125.f)
                    *smooth((y-upperEdge)/58.f)*smooth((709.f-y)/73.f);
                if(row==rows||column==columns) continue;
                const int bottom=index+columns+1;
                const std::array<int,6> quad{index,index+1,bottom,index+1,bottom+1,bottom};
                std::copy(quad.begin(),quad.end(),triangles_.begin()+(row*columns+column)*6);
            }
        }
        return true;
    }

    void draw(SDL_Renderer* renderer,float seconds) {
        SDL_SetRenderDrawColor(renderer,10,10,18,255);
        SDL_RenderClear(renderer);
        for(std::size_t index=0;index<cloth_.size();++index) {
            auto& vertex=cloth_[index];
            const float u=vertex.tex_coord.x, v=vertex.tex_coord.y;
            const float wave=seconds*2.25f+u*16.f-v*7.f;
            vertex.position={u*screenWidth+3.f*flex_[index]*std::cos(wave),
                             v*screenHeight+9.f*flex_[index]*std::sin(wave)};
        }
        if(!SDL_RenderGeometry(renderer,art_,cloth_.data(),cloth_.size(),triangles_.data(),triangles_.size()))
            SDL_RenderTexture(renderer,art_,nullptr,nullptr);

        // The original lettering and eclipse share a reveal layer. Fade a
        // feathered veil over that band so its lower edge never cuts the art.
        const float opacity=1.f-smooth((seconds-.22f)/1.25f);
        if(opacity<=0) return;
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
        const SDL_FColor dark{.035f,.034f,.057f,opacity};
        const SDL_FColor clear{dark.r,dark.g,dark.b,0.f};
        const SDL_Vertex veil[]={
            {{0,0},dark,{}},{{screenWidth,0},dark,{}},
            {{0,126},dark,{}},{{screenWidth,126},dark,{}},
            {{0,201},clear,{}},{{screenWidth,201},clear,{}}
        };
        constexpr int veilTriangles[]={0,1,2,1,3,2,2,3,4,3,5,4};
        SDL_RenderGeometry(renderer,nullptr,veil,6,veilTriangles,12);
    }
};
}
