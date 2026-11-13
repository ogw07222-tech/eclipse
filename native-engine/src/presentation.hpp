#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <cstdio>
#include <cstring>
#include <vector>
#include "core.hpp"
#include "sword_trails.hpp"
#include "knight_skill_blades.hpp"
#include "forest_generator.hpp"
#include "forest_layers.hpp"
#include "demon_castle_entrance.hpp"
#include "flash_executioner_assets.hpp"

namespace eclipse {
class Presentation {
    SDL_Renderer* renderer;
    std::array<SDL_Texture*,5> textures{};
    SDL_Texture* titleTexture_=nullptr;
    SDL_Texture* titleBg_=nullptr;
    SDL_Texture* titleKnightFg_=nullptr;
    SDL_Texture* titleCape_=nullptr;
    SDL_Texture* titleMenu_=nullptr;
    SDL_Texture* whiteNightLance_=nullptr;
    SDL_Texture* whiteNightMotion_=nullptr;
    std::array<SDL_Texture*,static_cast<int>(FlashActor::Count)> flashActorTextures_{};
    std::array<SDL_Texture*,static_cast<int>(FlashFx::Count)> flashFxTextures_{};
    struct ReviewedFlashEvent {FlashFx role;float x,y,born;int facing;bool follow;};
    std::vector<ReviewedFlashEvent> reviewedFlashEvents_;
    float reviewedFlashClock_=-1,reviewedFlashMoveAge_=0;
    WhiteMove reviewedFlashMove_=WhiteMove::None;
    unsigned long long reviewedFlashBurstSerial_=0;
    int reviewedFlashLandingSerial_=-1;
    bool reviewedFlashDash_=false,reviewedFlashSwap_=false,reviewedFlashAirborne_=false;
    SDL_Texture* goldLance_=nullptr;
    SDL_Texture* goldTrail_=nullptr;
    SDL_Texture* goldOrb_=nullptr;
    SDL_Texture* goldImpact_=nullptr;
    SDL_Texture* riftSlash_=nullptr;
    SDL_Texture* riftTrace_=nullptr;
    std::array<SDL_Texture*,5> whiteFxTextures_{};
    std::array<SDL_Texture*,3> slimeTextures_{};
    SDL_Texture* enemyBarFrame_=nullptr;
    SDL_Texture* executeIcon_=nullptr;
    std::array<SDL_Texture*,3> knightSkillTextures_{};
    std::array<SDL_Texture*,4> whiteSkillTextures_{};
    float offset=0;
    SDL_Texture* knightSkillMotion_=nullptr;
    // Effects remain in an independent transparent atlas, never baked onto the actor.
    SDL_Texture* knightSkillFx_=nullptr;
    SDL_Texture* mageRun_=nullptr;
    SDL_Texture* mageMotion_=nullptr;
    SDL_Texture* settingsGear_=nullptr;
    SDL_Texture* settingsPanel_=nullptr;
    SDL_Texture* settingsWindow_=nullptr;
    ForestGenerator forestGenerator_{};
    std::array<SDL_Texture*,40> forestTextures_{};
    std::array<SDL_Texture*,4> forestLayerTextures_{};
    bool forestLayersReady_=false;
    std::array<SDL_Texture*,6> forestV3Textures_{};
    bool forestV3Ready_=false;
    bool forestEntranceReady_=false;
    DemonCastleEntrance castleEntrance_{};
    bool castleEntranceActive_=false;
    struct Ghost {
        float x,y,time;int frame,columns,facing;SDL_Texture* texture;bool slam;
    };
    std::vector<Ghost> ghosts_{};
    float lastGhostSample_=-1,lastGhostClock_=-1,lastGhostX_=0,lastGhostY_=0;
    static SDL_Color color(unsigned rgb,Uint8 alpha=255){return {static_cast<Uint8>(rgb>>16),static_cast<Uint8>(rgb>>8),static_cast<Uint8>(rgb),alpha};}
    void rect(float x,float y,float w,float h,unsigned rgb,Uint8 alpha=255){
        auto c=color(rgb,alpha);SDL_SetRenderDrawColor(renderer,c.r,c.g,c.b,c.a);
        SDL_FRect r{std::round(x),std::round(y),std::max(1.f,std::round(w)),std::max(1.f,std::round(h))};SDL_RenderFillRect(renderer,&r);
    }
    void world(float x,float y,float w,float h,unsigned rgb,float grid=1){
        x=std::round(x/grid)*grid;y=std::round(y/grid)*grid;w=std::max(grid,std::round(w/grid)*grid);h=std::max(grid,std::round(h/grid)*grid);
        if(x-offset+w<0||x-offset>viewWidth)return;
        rect(x-offset,y,w,h,rgb);
    }
    void disk(float x,float y,float radius,unsigned rgb,int cell=4){
        for(int yy=-static_cast<int>(radius);yy<=radius;yy+=cell)for(int xx=-static_cast<int>(radius);xx<=radius;xx+=cell)
            if(xx*xx+yy*yy<=radius*radius)rect(x+xx,y+yy,cell,cell,rgb);
    }
    void ring(float x,float y,float radius,unsigned rgb,int cell=4){
        for(int yy=-static_cast<int>(radius);yy<=radius;yy+=cell)for(int xx=-static_cast<int>(radius);xx<=radius;xx+=cell){int d=xx*xx+yy*yy;
            if(d<=radius*radius&&d>=(radius-cell*2)*(radius-cell*2))rect(x+xx,y+yy,cell,cell,rgb);
        }
    }
    static const char* glyph(char c){
        static const char* letters[]={"01110100011000111111100011000110001","11110100011000111110100011000111110","01111100001000010000100001000001111","11110100011000110001100011000111110","11111100001000011110100001000011111","11111100001000011110100001000010000","01111100001000010111100011000101111","10001100011000111111100011000110001","11111001000010000100001000010011111","00111000100001000010000101001001100","10001100101010011000101001001010001","10000100001000010000100001000011111","10001110111010110101100011000110001","10001110011010110011100011000110001","01110100011000110001100011000101110","11110100011000111110100001000010000","01110100011000110001101011001001101","11110100011000111110101001001010001","01111100001000001110000010000111110","11111001000010000100001000010000100","10001100011000110001100011000101110","10001100011000110001100010101000100","10001100011000110101101011101110001","10001100010101000100010101000110001","10001100010101000100001000010000100","11111000010001000100010001000011111"};
        static const char* digits[]={"111101101101111","010110010010111","111001111100111","111001111001111","101101111001001","111100111001111","111100111101111","111001010010010","111101111101111","111101111001111"};
        if(c>='A'&&c<='Z')return letters[c-'A'];
        if(c>='0'&&c<='9')return digits[c-'0'];
        return nullptr;
    }
    void text(const char* s,float x,float y,int scale=2,unsigned rgb=0xd4e0d2){
        for(;*s;++s){if(const char* g=glyph(*s)){bool digit=*s>='0'&&*s<='9';int cols=digit?3:5,rows=digit?5:7;
                for(int row=0;row<rows;++row)for(int col=0;col<cols;++col)if(g[row*cols+col]=='1')rect(x+col*scale,y+row*scale,scale,scale,rgb);
            }else if(*s=='-')rect(x,y+3*scale,4*scale,scale,rgb);
            else if(*s=='/')for(int i=0;i<5;++i)rect(x+(4-i)*scale,y+i*scale,scale,scale,rgb);
            x+=6*scale;
        }
    }
    void number(int n,float x,float y,int scale=3,unsigned rgb=0xf2d48c){char buffer[24];std::snprintf(buffer,sizeof(buffer),"%d",n);text(buffer,x,y,scale,rgb);}

    static size_t forestIndex(const ForestSprite& sprite) {
        switch(sprite.kind) {
            case ForestKind::Far: return sprite.variant;
            case ForestKind::LargeTree: return 3+sprite.variant;
            case ForestKind::MidTree: return 8+sprite.variant;
            case ForestKind::Bush: return 12+sprite.variant;
            case ForestKind::Grass: return 17+sprite.variant;
            case ForestKind::Rock: return 21+sprite.variant;
            case ForestKind::Plant: return 34+sprite.variant;
            case ForestKind::Detail: return 28+sprite.variant;
            case ForestKind::Effect: return 37+sprite.variant;
        }
        return 0;
    }
    void forestObject(const ForestSprite& sprite,float camera,float clock){
        const float sx=forestGenerator_.drawX(sprite,camera);
        // Tall canopies may extend far beyond their anchor; cull using sprite bounds below.
        if(sx < -1000 || sx > viewWidth+1000)return;
        const size_t index=forestIndex(sprite);
        // Far silhouettes in pack v1 have opaque rectangular backgrounds.
        // Use the seamless procedural fallback until cleaned transparent strips ship.
        SDL_Texture* image=sprite.kind==ForestKind::Far ? nullptr : forestTextures_[index];
        if(image) {
            float w=0,h=0;
            if(SDL_GetTextureSize(image,&w,&h)) {
                // Normalize sprites to the original art footprint: repairing
                // cropped PNG margins must not unexpectedly enlarge them.
                float referenceHeight=h;
                if(sprite.kind==ForestKind::LargeTree)referenceHeight=148.f;
                if(sprite.kind==ForestKind::MidTree)referenceHeight=106.f;
                if(sprite.kind==ForestKind::Bush)
                    referenceHeight=sprite.variant==0?62.f:86.f;
                float scale=sprite.scale*referenceHeight/h;
                // Avoid upper-edge clipping as user increases the tree size.
                if(sprite.kind==ForestKind::LargeTree ||
                   sprite.kind==ForestKind::MidTree)
                    scale=std::min(scale,(sprite.groundY-12.f)/h);
                // Cull only after considering the scaled image width.
                if(sx+w*scale*.5f<0 || sx-w*scale*.5f>viewWidth)return;
                const float py=sprite.groundY+forestGenerator_.microMotion(sprite,clock);
                // The alpha-trimmed root ends above the transparent canvas bottom:
                // 9 source pixels for main tree and 17 for secondary PNG.
                const float rootPadding=sprite.kind==ForestKind::LargeTree?9.f:
                    sprite.kind==ForestKind::MidTree?17.f:0.f;
                // Hanging vines anchor high above the ground; all other props use bottom pivot.
                const float top=sprite.kind==ForestKind::Detail && sprite.variant==2
                    ? 355.f : py-h*scale+rootPadding*scale;
                // Per-instance silhouette width and gentle shade variation.
                // Position stays centered, roots remain anchored on the same ground.
                const float rw=w*scale*sprite.widthFactor;
                SDL_FRect target{std::round(sx-rw*.5f),
                    std::round(top),std::round(rw),std::round(h*scale)};
                constexpr Uint8 tones[3]={232,244,255};
                const Uint8 tint=tones[std::min<size_t>(sprite.shade,2)];
                SDL_SetTextureColorMod(image,tint,tint,tint);
                if(sprite.flipX)
                    SDL_RenderTextureRotated(renderer,image,nullptr,&target,0,nullptr,SDL_FLIP_HORIZONTAL);
                else SDL_RenderTexture(renderer,image,nullptr,&target);
                SDL_SetTextureColorMod(image,255,255,255);
                return;
            }
        }
        // Missing PNG: skip this decoration instead of drawing mismatched low-res trees.
        // The original sprite library is shipped with the APK and desktop assets.
    }
    // Deliberate pixel-dirt cross section. One deterministic material palette;
    // structure first (humus / clay / bedrock), details second (roots / stones).
    static unsigned soilHash(unsigned x,unsigned y,unsigned salt) {
        unsigned h=x*374761393u+y*668265263u+salt*2246822519u;
        h=(h^(h>>13))*1274126177u;
        return h^(h>>16);
    }
    void forestGround() {
        const unsigned seed=forestGenerator_.settings().seed;
        // No green ground plane. Surface grass occupies only 3-9px.
        world(0,585,1600,135,0x332720,3);
        world(0,662,1600,58,0x29211e,3);
        // Natural-looking blocky horizons: staggered tiles conceal flat borders.
        for(unsigned i=0;i<68;++i) {
            const unsigned h=soilHash(i,1,seed);
            const float x=static_cast<float>(i*24);
            const float clay=618.f+static_cast<float>(h%5u)*6.f;
            world(x,585,24,clay-585.f,
                h%5u==0?0x463127:h%5u==1?0x493429:0x402e25,3);
            const float stone=658.f+static_cast<float>((h>>5)%6u)*6.f;
            world(x,clay,24,stone-clay,
                h%4u==0?0x3c2b26:h%4u==1?0x412d27:0x382923,3);
            world(x,stone,24,720.f-stone,
                h%4u==0?0x302420:0x29211e,3);
        }
        // Long soft sediment veins. Wider bands read as geology, not random noise.
        for(unsigned i=0;i<30;++i) {
            const unsigned h=soilHash(i,2,seed);
            const float x=static_cast<float>(h%1540u);
            const float y=612.f+static_cast<float>((h>>10)%32u)*3.f;
            const float length=27.f+static_cast<float>((h>>16)%12u)*6.f;
            world(x,y,length,3,
                i%5u==0?0x684b36:i%3u==0?0x514033:0x46352d,3);
            if(i%3u==0)world(x+9,y+6,length*.55f,3,0x30251f,3);
        }
        // Inset pebbles with a dark lower rim and a muted top highlight.
        for(unsigned i=0;i<26;++i) {
            const unsigned h=soilHash(i,3,seed);
            const float x=static_cast<float>(h%1560u);
            const float y=605.f+static_cast<float>((h>>11)%33u)*3.f;
            const float w=12.f+static_cast<float>((h>>20)%5u)*3.f;
            world(x,y+3,w,9,0x261f1b,3);
            world(x+3,y,w-6,6,h%3u==0?0x756550:0x5d5346,3);
            world(x+6,y,6,3,0x93816a,3);
        }
        // Fine crumbs are small clusters rather than a field of isolated noise.
        for(unsigned i=0;i<38;++i) {
            const unsigned h=soilHash(i,4,seed);
            const float x=static_cast<float>(h%1560u);
            const float y=599.f+static_cast<float>((h>>10)%37u)*3.f;
            world(x,y,6,3,i%3u==0?0x664938:0x241e1a,3);
            world(x+9,y+3,3,3,i%4u==0?0x806149:0x4b382d,3);
        }
        // Roots emerge from the forest floor and split once underground.
        for(unsigned i=0;i<13;++i) {
            const unsigned h=soilHash(i,5,seed);
            const float x=static_cast<float>(h%1550u);
            const float y=588.f+static_cast<float>((h>>8)%4u)*3.f;
            world(x,y,3,18+static_cast<float>((h>>12)%6u)*3.f,0x6c4932,3);
            world(x+3,y+18,12,3,0x765138,3);
            if(i%2u==0)world(x+12,y+21,3,9,0x63452f,3);
        }
        // Surface: thin broken grass/moss edge, never a bright green slab.
        for(unsigned i=0;i<134;++i) {
            const unsigned h=soilHash(i,6,seed);
            const float x=static_cast<float>(i*12);
            // Crisp soil/air interface: darker undercut, thin natural grass on top.
            // The shape varies in pixel increments but gameplay floor stays at y585.
            world(x,585,12,3,h%4u==0?0x65432f:0x4e3427,3);
            world(x,588,12,3,h%5u==0?0x503326:0x392920,3);
            if(h%7u==0)continue;
            world(x,582,12,3,h%4u==0?0x668354:h%4u==1?0x4d7147:0x42653e,3);
            if(h%4u==0)
                world(x+3,576.f-static_cast<float>((h>>7)%2u)*3.f,
                      3,6,0x73965a,3);
            if(h%9u==0)world(x+6,579,6,3,0x849860,3);
        }
    }
    void forestSky() {
        // Low-frequency stepped sky gradient in the moonlight palette.
        constexpr unsigned colors[3]={0x071326,0x10253f,0x23445b};
        for(int y=0;y<585;y+=6){
            const float t=static_cast<float>(y)/585.f;
            const unsigned a=t<.54f?colors[0]:colors[1];
            const unsigned b=t<.54f?colors[1]:colors[2];
            const float q=t<.54f?t/.54f:(t-.54f)/.46f;
            const unsigned r=static_cast<unsigned>((a>>16&255)*(1-q)+(b>>16&255)*q);
            const unsigned g=static_cast<unsigned>((a>>8&255)*(1-q)+(b>>8&255)*q);
            const unsigned bl=static_cast<unsigned>((a&255)*(1-q)+(b&255)*q);
            rect(0,static_cast<float>(y),viewWidth,6,(r<<16)|(g<<8)|bl);
        }
        const float cx=1160.f,cy=135.f;
        // Soft golden light is several translucent circles with decreasing coverage.
        for(int radius=124;radius>=42;radius-=8) {
            const float k=static_cast<float>(124-radius)/82.f;
            const Uint8 alpha=static_cast<Uint8>(2+k*k*5);
            for(int y=-radius;y<=radius;y+=8)for(int x=-radius;x<=radius;x+=8)
                if(x*x+y*y<=radius*radius)
                    rect(cx+x,cy+y,8,8,0xffe1a1,alpha);
        }
        // True circular body, block-pixel rim and a few subdued craters.
        for(int y=-30;y<=30;y+=3)for(int x=-30;x<=30;x+=3){
            if(x*x+y*y>900)continue;
            rect(cx+x,cy+y,3,3,x*x+y*y>625?0xe9d49e:0xffebbd);
        }
        for(int i=0;i<9;++i){
            const int dx=(i*19)%39-19,dy=(i*23)%37-18;
            if(dx*dx+dy*dy>22*22)continue;
            rect(cx+dx,cy+dy,i%3==0?6:3,3,i%2==0?0xbeb08b:0xd6c39a,100);
        }
        for(int i=0;i<55;++i){
            const unsigned h=soilHash(static_cast<unsigned>(i),7,20261103);
            const int x=static_cast<int>(h%1280u),y=static_cast<int>((h>>12)%330u);
            const int dx=x-1160,dy=y-135;
            if(dx*dx+dy*dy<22500)continue;
            rect(static_cast<float>(x),static_cast<float>(y),
                 i%7==0?4.f:2.f,i%7==0?4.f:2.f,
                 i%4==0?0xc0d4d5:0x6a9aaf,200);
        }
    }
    void forest(const Combat& g,float camera) {
        if(castleEntranceActive_ && castleEntrance_.ready()) {castleEntrance_.draw(camera);return;}
        if(forestV3Ready_) {
            // Layer art only: no scene randomization or NPC occupancy changes.
            // Every asset's native dimensions and world-ground height are fixed.
            const auto& activeScene=forestEntranceReady_
                ? ForestLayerScene::layersEntrance : ForestLayerScene::layersV3;
            for(size_t i=0;i<forestV3Textures_.size();++i) {
                SDL_Texture* image=forestV3Textures_[i];
                float w=0.f,h=0.f;
                if(!SDL_GetTextureSize(image,&w,&h))continue;
                const float shift=(camera-640.f)*activeScene[i].parallax;
                SDL_FRect target{std::round(-shift),0.f,w,h};
                if(i==2) {
                    const Uint8 tint=static_cast<Uint8>(
                        ForestLayerScene::farTint(forestGenerator_.settings().seed));
                    SDL_SetTextureColorMod(image,tint,tint,tint);
                }
                SDL_RenderTexture(renderer,image,nullptr,&target);
                if(i==2)SDL_SetTextureColorMod(image,255,255,255);
            }
            return;
        }
        if(forestLayersReady_){
            // Composed PNG layers: no random tree/log placement or varying collision.
            // The art surface stays at y=585; shared collision is inset into its grass edge.
            for(size_t i=0;i<forestLayerTextures_.size();++i){
                SDL_Texture* image=forestLayerTextures_[i];
                const float shift=(camera-640.f)*ForestLayerScene::layers[i].parallax;
                float w=0.f,h=0.f;
                if(!SDL_GetTextureSize(image,&w,&h))continue;
                SDL_FRect dest{std::round(-shift),0.f,w,h};
                if(i==1){
                    const Uint8 tint=static_cast<Uint8>(ForestLayerScene::farTint(forestGenerator_.settings().seed));
                    SDL_SetTextureColorMod(image,tint,tint,tint);
                }
                SDL_RenderTexture(renderer,image,nullptr,&dest);
                if(i==1)SDL_SetTextureColorMod(image,255,255,255);
            }
            return;
        }
        forestSky();
        const auto& instances=forestGenerator_.sprites();
        for(const auto& instance:instances) {
            if(instance.layer>=4)break;
            forestObject(instance,camera,g.clock);
        }
        forestGround();
        for(const auto& instance:instances) {
            if(instance.layer<4)continue;
            forestObject(instance,camera,g.clock);
        }
    }
    void enemy(const Enemy& e,float clock){
        const float scale=slimeSpec(e.kind).scale,h=(e.kind==0?80:100)*scale;
        const int frame=enemyFrame(e);
        SDL_FRect source{float(frame%4*slimeCell),float(frame/4*slimeCell),slimeCell,slimeCell};
        SDL_FRect target{std::round(e.x-offset-slimePivotX*scale),std::round(e.y-slimePivotY*scale),slimeCell*scale,slimeCell*scale};
        auto* texture=slimeTextures_[e.kind];
        SDL_SetTextureColorMod(texture,255,e.hitFlash>0?135:255,e.hitFlash>0?135:255);
        SDL_RenderTextureRotated(renderer,texture,&source,&target,0,nullptr,e.facing>0?SDL_FLIP_HORIZONTAL:SDL_FLIP_NONE);
        SDL_SetTextureColorMod(texture,255,255,255);
        // Pixel-art PNG bezel, with independent clipped fills for immediate HP and
        // the delayed white segment. 88x18, more than twice the former height.
        const float barX=std::round(e.x-offset-44),barY=std::round(e.y-h-25);
        if(enemyBarFrame_) {
            SDL_FRect dest{barX,barY,88.f,18.f};
            SDL_RenderTexture(renderer,enemyBarFrame_,nullptr,&dest);
        } else {
            rect(barX,barY,88,18,0x0b0b12);
            rect(barX+2,barY+2,84,14,0x61505d);
        }
        if(e.lightMark>0){
            const float x=e.x-offset,y=barY-12;
            rect(x-2,y-7,4,14,0xffc640);
            rect(x-7,y-2,14,4,0xffc640);
            rect(x-2,y-2,4,4,0xfff6bf);
        }
        const float hpRatio=std::clamp(float(e.health)/e.maxHealth,0.f,1.f);
        const float trailRatio=std::clamp(std::max(float(e.health),e.trailingHealth)/e.maxHealth,0.f,1.f);
        const float fillX=barX+4,fillY=barY+5,fillW=80.f;
        // Never render a minimum one-pixel fill for zero HP.
        if(trailRatio>0)rect(fillX,fillY,fillW*trailRatio,8,0xf3f0ea);
        if(hpRatio>0)rect(fillX,fillY,fillW*hpRatio,8,e.kind==2?0xbb526c:0xc74b53);
        if(e.bleedStacks==bleedMaxStacks&&!e.boss){
            // 10% execution threshold appears only while 10-stack bleed is active.
            rect(fillX+8-1,fillY-2,2,12,0xf0a8b2);
            const bool ready=e.health*10<=e.maxHealth;
            if(executeIcon_){
                SDL_SetTextureColorMod(executeIcon_,ready?255:130,ready?255:95,ready?255:103);
                SDL_FRect icon{barX+88,barY-2,24.f,24.f};
                SDL_RenderTexture(renderer,executeIcon_,nullptr,&icon);
                SDL_SetTextureColorMod(executeIcon_,255,255,255);
            }
            if(ready)text("EXECUTE",barX+2,barY-15,2,0xffb5bf);
        }
        if(e.bleedStacks>0){
            const float x=e.x-offset-19,y=e.y-h-44;
            rect(x+4,y,4,4,0xf2a0aa);
            rect(x+2,y+4,8,8,0xc84f65);
            rect(x+4,y+12,4,3,0x92354b);
            number(e.bleedStacks,x+14,y,2,e.bleedStacks==bleedMaxStacks?0xffc0c3:0xe8798d);
        }
        if(e.bleed>0)for(int i=0;i<6;++i){float phase=std::fmod(clock*(17+i%3*4)+i*11,h);world(e.x+i*13%29-14,e.y-h+phase,3,i%2==0?6:3,i%2==0?0xe34359:0x8f183a,3);}
        if(e.burn>0)for(int i=0;i<9;++i){float rise=std::fmod(clock*(29+i%4*6)+i*9,h+18);world(e.x+i*17%35-17+std::sin(clock*5+i)*3,e.y+4-rise,i%3==0?6:3,i%2==0?9:6,i%3==0?0xffb74b:i%3==1?0xf16b32:0xbb3839,3);}
        if(e.reactionFlash>0)for(int i=0;i<8;++i)world(e.x+i%4*17-26,e.y-h*.5f+i/4*18-9,9,6,0xffbc57,3);
    }
    void mage(const Player& p){
        float x=p.x,y=p.y;world(x-15,y-55,30,40,0x5a2837);world(x-12,y-52,24,30,0xf69a5c);disk(x-offset,y-60,15,0xf6d3b3,3);world(x-12,y-70,24,10,0x5a2837);
        world(x+p.facing*7-2,y-62,4,4,0x151b2c);world(x-12,y-15,10,15,0x5a2837);world(x+2,y-15,10,15,0x5a2837);
        for(int i=0;i<9;++i)world(x+p.facing*(15+i),y-35-i*3,6,6,0x8f6ad4,3);
        disk(x+p.facing*22-offset,y-64,7,0xffb74b,3);
        if(p.attackFlash>0)ring(x+p.facing*16-offset,y-36,40,0xffb74b);
    }
    void afterimages(const Combat& g,const Animation& a,SDL_Texture* actorTexture,int frame,int columns){
        const auto& p=g.player;
        if(g.clock<lastGhostClock_||(lastGhostClock_>=0 &&
            (std::abs(p.x-lastGhostX_)>260||std::abs(p.y-lastGhostY_)>280))){
            ghosts_.clear();lastGhostSample_=-1;
        }
        if(g.clock>lastGhostClock_){
            ghosts_.erase(std::remove_if(ghosts_.begin(),ghosts_.end(),[&](const Ghost& s){
                return g.clock-s.time>(s.slam?.15f:.19f);}),ghosts_.end());
            const bool dash=p.health>0&&p.dashLeft>0&&std::abs(p.vx)>300.f;
            const bool slam=p.health>0&&p.form==0&&p.airSlam&&
                p.slamAge>=airSlamWindup&&p.vy>200.f;
            if((dash||slam)&&actorTexture&&(lastGhostSample_<0||g.clock-lastGhostSample_>=(slam?.035f:.04f))){
                if(ghosts_.size()>=6)ghosts_.erase(ghosts_.begin());
                ghosts_.push_back({p.x,p.y,g.clock,frame,columns,p.facing,actorTexture,slam});
                lastGhostSample_=g.clock;
            }
            if(!dash&&!slam)lastGhostSample_=-1;
        }
        lastGhostClock_=g.clock;lastGhostX_=p.x;lastGhostY_=p.y;
        for(const auto& s:ghosts_){
            const float lifetime=s.slam?.15f:.19f,age=g.clock-s.time;
            if(age<.018f||age>lifetime)continue;
            SDL_SetTextureColorMod(s.texture,s.slam?160:120,s.slam?181:155,s.slam?196:186);
            SDL_SetTextureAlphaMod(s.texture,static_cast<Uint8>(std::clamp(1.f-age/lifetime,0.f,1.f)*(s.slam?145.f:132.f)));
            SDL_FRect source{float(s.frame%s.columns*motionWidth),float(s.frame/s.columns*motionHeight),motionWidth,motionHeight};
            SDL_FRect dest{std::round(s.x-offset-(s.facing>0?motionPivotX:motionWidth-motionPivotX)*motionScale),
                std::round(s.y-motionPivotY*motionScale),motionWidth*motionScale,motionHeight*motionScale};
            SDL_RenderTextureRotated(renderer,s.texture,&source,&dest,0,nullptr,s.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
            SDL_SetTextureAlphaMod(s.texture,255);SDL_SetTextureColorMod(s.texture,255,255,255);
        }
    }
    void slamShockwave(const Player& p){
        if(p.slamImpact<=0)return;
        const float t=std::clamp(1-p.slamImpact/airSlamImpactDuration,0.f,1.f);
        const float center=p.slamImpactX-offset,ground=p.slamImpactY;
        const float radius=(8+(airSlamRadius-8)*std::sqrt(t))*p.gear.range();
        const Uint8 fade=static_cast<Uint8>(235*(1-t));
        // Three stepped ground arcs, white front over cold-blue trailing energy.
        for(int layer=0;layer<3;++layer){
            const float r=radius*(1-layer*.16f),height=(11-layer*2)*(1-t);
            for(int x=-static_cast<int>(r);x<=r;x+=3){
                const float k=std::max(0.f,1-x*x/(r*r));
                const float y=ground-std::round(std::sqrt(k)*height/3)*3;
                rect(center+x,y+layer*3,3,3,layer==0?0xf3ffff:layer==1?0x7ec5ed:0x284a70,
                     static_cast<Uint8>(fade*(layer==0?1.f:layer==1?.75f:.45f)));
            }
        }
        // Grounded flash and ballistic pixel fragments remain at the contact.
        if(t<.35f){const float f=1-t/.35f;
            rect(center-3,ground-34*f,6,34*f,0x7ec5ed,static_cast<Uint8>(170*f));
            rect(center-1,ground-25*f,3,25*f,0xf3ffff,static_cast<Uint8>(245*f));
        }
        for(int i=0;i<9;++i){
            const float dx=(i-4)*(8+17*t),height=(9+i%3*8)*std::sin(t*3.14159265f);
            rect(center+dx,ground-height,3,i%3==0?6:3,i%2?0x7ec5ed:0xf3ffff,fade);
        }
    }

    void swordTrail(const Player& p,const Animation& a){
        if(p.health<=0||((a.state<4||a.state>6)&&a.state!=12))return;
        // The weak slash's actor and blade effect must use the same PNG pose.
        // a.frame remains the original gameplay-independent clip key.
        const int displayedFrame=p.form==0&&!p.whiteNight&&a.state==4?
            knightWeakFrame(a.posePhase):a.frame;
        const auto frame=swordTrailFrames[displayedFrame];
        for(int i=frame.start;i<frame.start+frame.count;++i){const auto s=swordTrailSpans[i];
            float x=p.facing>0?p.x+(s.x-motionPivotX)*motionScale:p.x-(s.x+s.width-motionPivotX)*motionScale;
            rect(x-offset,p.y+(s.y-motionPivotY)*motionScale,s.width*motionScale,motionScale,swordTrailColors[s.shade],static_cast<Uint8>(swordTrailAlpha[s.shade]*frame.alpha/255));
        }
    }
    void knightSkillPngEffect(const Player& p,const Animation& a){
        if(!knightSkillFx_||!isSkillMotion(a.state)||p.health<=0)return;
        // Key frames are stored in the first three rows of the 4x6 FX sheet.
        // The character's separately authored midpoint rows do not change
        // effect frame IDs, so sword trails cannot jump to another atlas row.
        const auto& clip=motionClips[a.state];
        const int pose=std::clamp(a.frame-clip.start,0,clip.count-1);
        const int fxRow=a.state-thrustSkillState;
        // FX atlas keeps four key-effect phases per skill, independent of
        // the actor atlas's twelve authored source-based display poses.
        const int fxPose=std::clamp(pose/3,0,3);
        const SDL_FRect src{float(fxPose*skillMotionWidth),
            float(fxRow*motionHeight),float(skillMotionWidth),float(motionHeight)};
        const SDL_FRect dest{
            std::round(p.x-offset-skillMotionPivotX*motionScale),
            std::round(p.y-motionPivotY*motionScale),
            skillMotionWidth*motionScale,motionHeight*motionScale};
        SDL_RenderTextureRotated(renderer,knightSkillFx_,&src,&dest,0,nullptr,
            p.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
    }
    void icon(int texture,int col,int row,float x,float y,float size,bool down){
        SDL_FRect source{col*128.f,row*128.f,128,128},target{x,y,size,size};
        SDL_SetTextureColorMod(textures[texture],down?180:255,down?180:255,down?180:255);
        SDL_SetTextureAlphaMod(textures[texture],down?191:153);
        SDL_RenderTexture(renderer,textures[texture],&source,&target);
        SDL_SetTextureAlphaMod(textures[texture],255);
    }
    void skillButton(const ControlRect& b,const char* label,float remaining,float duration,
                     bool pressed,unsigned accent,bool enabled=true,int art=-1){
        const float ready=remaining>0&&duration>0
            ?1.f-std::clamp(remaining/duration,0.f,1.f):1.f;
        SDL_Texture* skillArt=art>=3&&art<7?whiteSkillTextures_[art-3]:
            art>=0&&art<3?knightSkillTextures_[art]:nullptr;
        if(skillArt){
            SDL_Texture* image=skillArt;
            float width=0,height=0;
            if(SDL_GetTextureSize(image,&width,&height)){
                const float size=std::round(b.size);
                SDL_FRect target{std::round(b.x),std::round(b.y),size,size};
                SDL_SetTextureAlphaMod(image,pressed?240:205);
                const float filled=std::round(size*ready);
                const float sourceFill=height*filled/size;
                if(filled<size){
                    const SDL_FRect source{0,0,width,height-sourceFill};
                    const SDL_FRect mask{target.x,target.y,size,size-filled};
                    SDL_SetTextureColorMod(image,95,99,110);
                    SDL_RenderTexture(renderer,image,&source,&mask);
                }
                if(filled>0){
                    const SDL_FRect source{0,height-sourceFill,width,sourceFill};
                    const SDL_FRect fill{target.x,target.y+size-filled,size,filled};
                    SDL_SetTextureColorMod(image,pressed?255:220,pressed?255:225,pressed?255:230);
                    SDL_RenderTexture(renderer,image,&source,&fill);
                }
                SDL_SetTextureColorMod(image,255,255,255);
                SDL_SetTextureAlphaMod(image,255);
                if(ready>=1.f)rect(b.x+b.size*.39f,b.y+b.size*.83f,b.size*.22f,3,accent,210);
                return;
            }
        }

        // The lower portion brightens as the skill becomes available.
        const float unit=std::max(2.f,std::floor(b.size/18.f));
        const float cut=3*unit;
        const Uint8 alpha=enabled?(pressed?240:206):85;
        for(float y=0;y<b.size;y+=unit){
            const float inset=std::max(0.f,std::max(cut-y,y-(b.size-cut)));
            const float h=std::min(unit,b.size-y);
            rect(b.x+inset,b.y+y,b.size-inset*2,h,accent,alpha);
            if(y<unit||y+h>=b.size)continue;
            const bool cooling=y<b.size*(1.f-ready);
            rect(b.x+inset+unit,b.y+y,b.size-(inset+unit)*2,h,
                 cooling?0x12121c:pressed?0x536675:0x354650,alpha);
        }
        const int scale=b.size>=85?3:2;
        const float width=std::char_traits<char>::length(label)*6.f*scale;
        text(label,b.x+(b.size-width)/2,b.y+b.size*.25f,scale,enabled?0xe8e1d7:0x6d7079);
        if(enabled&&ready>=1.f)rect(b.x+b.size*.34f,b.y+b.size*.69f,b.size*.32f,3,accent,alpha);
    }
    void playerHealthBar(const Player& p){
        constexpr float x=22,y=18,barX=x+49,barY=y+12,barWidth=230;
        rect(x+4,y+6,284,46,0x090b12,220);
        rect(x+6,y,276,46,0x17131b);
        rect(x,y+6,288,34,0x17131b);
        rect(x+9,y+3,270,40,0x75533b);
        rect(x+3,y+9,282,28,0x75533b);
        rect(x+10,y+3,268,3,0xd9a46b);
        rect(x+10,y+40,268,3,0x4b3130);
        rect(x+44,y+8,239,30,0x271c24);
        rect(barX,barY,barWidth,22,0x16131d);

        const float hp=std::clamp(float(p.health)/p.maxHealth(),0.f,1.f);
        const float trail=std::clamp(p.trailingHealth/p.maxHealth(),hp,1.f);
        const float liveWidth=std::round(barWidth*hp);
        const float trailWidth=std::round(barWidth*trail)-liveWidth;
        if(liveWidth>0){
            rect(barX,barY,liveWidth,22,0x96414a);
            rect(barX,barY,liveWidth,3,0xe28380);
            rect(barX,barY+19,liveWidth,3,0x672b3d);
        }
        if(trailWidth>0){
            rect(barX+liveWidth,barY,trailWidth,22,0xffa441);
            rect(barX+liveWidth,barY,trailWidth,3,0xffd38a);
        }

        // Stepped metal badge with an eclipse at its center.
        rect(x+10,y+5,29,36,0x30232b);
        rect(x+7,y+10,35,26,0x30232b);
        rect(x+13,y+7,23,32,0xb78050);
        rect(x+10,y+12,29,22,0xb78050);
        rect(x+15,y+10,19,26,0x201927);
        rect(x+12,y+15,25,16,0x201927);
        disk(x+24,y+22,8,0xffbb70,2);
        disk(x+27,y+19,7,0x201927,2);
        rect(x+282,y+19,3,8,0xe8ae73);
        rect(x+279,y+22,9,2,0xe8ae73);

        char label[24];
        std::snprintf(label,sizeof(label),"HP %d / %d",p.health,p.maxHealth());
        const float labelX=barX+(barWidth-12.f*std::char_traits<char>::length(label)+2.f)*.5f;
        text(label,labelX+2,barY+6,2,0x21131c);
        text(label,labelX,barY+4,2,0xffeee1);
    }
    void skillSlot(const ControlRect& box,int slot,const char* label,float remaining,float duration,
                   bool pressed,unsigned accent,bool enabled,int art){
        skillButton(box,label,remaining,duration,pressed,accent,enabled,art);
        const float x=std::round(box.x+box.size*.14f),y=std::round(box.y+box.size*.12f);
        rect(x-3,y-2,16,15,0x121922,enabled?215:135);
        number(slot,x,y,2,enabled?0xe6ddd0:0x89909a);
    }
    void hud(const Combat& g,Actions held,const ControlLayout& layout,std::array<float,2> stick){
        const auto& p=g.player;
        playerHealthBar(p);
        for(int i=0;i<4;++i){rect(765+i*74,21,66,61,0x080d17);rect(771+i*74,27,54,49,0x667482);rect(777+i*74,33,42,37,0x222c3d);}
        text("GEAR",780,44,2);text(p.whiteNight?"WHITE NIGHT":p.form==0?"BLADE":"FIRE",28,73,2,0x9ed7ff);number(g.foes(),1180,43,5,0xece7d2);text("FOES",1090,25,2);
        const auto& c=layout.buttons;
        const auto& js=c[ControlLayout::Stick];
        const bool stickActive=held.left||held.right||held.jump;
        icon(4,0,0,js.x,js.y,js.size,stickActive);
        const float thumb=js.size*.29f;
        const float tx=js.x+js.size*.5f+stick[0],ty=js.y+js.size*.5f+stick[1];
        auto thumbLayer=[&](float size,unsigned color,Uint8 alpha){
            const float unit=size/7;
            const float x=std::round(tx-size*.5f),y=std::round(ty-size*.5f);
            rect(x+2*unit,y,3*unit,unit,color,alpha);
            rect(x+unit,y+unit,5*unit,unit,color,alpha);
            rect(x,y+2*unit,size,3*unit,color,alpha);
            rect(x+unit,y+5*unit,5*unit,unit,color,alpha);
            rect(x+2*unit,y+6*unit,3*unit,unit,color,alpha);
        };
        thumbLayer(thumb+4,0x101923,165);
        thumbLayer(thumb,stickActive?0xb8c8cf:0x91a8b4,stickActive?191:153);
        thumbLayer(thumb*.58f,0x304353,174);
        icon(1,held.attack?1:0,0,c[ControlLayout::Attack].x,c[ControlLayout::Attack].y,c[ControlLayout::Attack].size,held.attack);
        icon(2,held.dash?1:0,0,c[ControlLayout::Dash].x,c[ControlLayout::Dash].y,c[ControlLayout::Dash].size,held.dash);
        // Character swap remains available in every class, including White Night.
        icon(3,0,0,c[ControlLayout::Swap].x,c[ControlLayout::Swap].y,c[ControlLayout::Swap].size,held.swap);
        if(p.whiteNight){
            skillSlot(c[ControlLayout::Skill],1,"RUSH",p.skillCooldown,3.f,held.skill,0xb4d8ee,true,3);
            skillSlot(c[ControlLayout::Sweep],2,"RELEASE",p.sweepReady,1.f,held.skillSweep,0x9bb9d6,true,4);
            skillSlot(c[ControlLayout::Buff],3,"BLINK",p.buffReady,1.5f,held.skillBuff,0xd9ecf5,true,5);
            skillSlot(c[ControlLayout::Ultimate],4,"VERDICT",p.verdictCooldown,9.f,held.ultimate,0xc4d7ed,true,6);
        }else{
            const bool knight=p.form==0;
            skillSlot(c[ControlLayout::Skill],1,knight?"STAB":"FIRE",
                knight?p.skillCooldown:p.mageSkillCooldown,knight?thrustCooldown:4.1f,held.skill,0xcaa2a4,true,knight?0:-1);
            skillSlot(c[ControlLayout::Sweep],2,knight?"SWEEP":"-",p.sweepReady,sweepCooldown,held.skillSweep,0x92b5ce,knight,knight?1:-1);
            skillSlot(c[ControlLayout::Buff],3,knight?"PREP":"-",p.buffReady,buffCooldown,held.skillBuff,0xe3bd72,knight,knight?2:-1);
            if(knight&&p.preparation>0){
                const auto& buff=c[ControlLayout::Buff];
                rect(buff.x+buff.size*.25f,buff.y+buff.size*.88f,buff.size*.5f*p.preparation/preparationDuration,3,0xffdc91);
            }
            if(p.swapCooldown>0)number(static_cast<int>(std::ceil(p.swapCooldown)),c[ControlLayout::Swap].x+20,c[ControlLayout::Swap].y+25,3,0xffd277);
        }
        if(g.inventory){rect(46,135,460,340,0xe3bd72);rect(50,139,452,332,0x1c2a34);text("EQUIPMENT",66,153,3);
            for(int i=0;i<3;++i){text(p.gear.name(i),66,200+i*42,2);rect(66+i*143,413,132,45,0x304039);text(i==0?"WEAPON":i==1?"ARMOR":"CHARM",75+i*143,428,2);}
            text("ATK",66,340,2);number(p.gear.attack(),120,340,2);text("DEF",200,340,2);number(p.gear.defense(),250,340,2);text("MAX HP",66,373,2);number(p.maxHealth(),160,373,2);
        }
    }
    // Every White Night combat flash is a transparent 4-frame PNG atlas.
    // No procedural colored rectangles are used for these effects.
    void drawWhiteEffect(const LightBurst& burst){
        int kind=0;
        switch(burst.kind){
            case LightFx::Slash: kind=0;break;
            case LightFx::Rush: kind=1;break;
            case LightFx::Depart: kind=2;break;
            case LightFx::Arrive: kind=3;break;
            case LightFx::Verdict: kind=4;break;
            default: return;
        }
        SDL_Texture* sheet=whiteFxTextures_[kind];
        if(!sheet)return;
        const float t=std::clamp(1.f-burst.life/.24f,0.f,1.f);
        const int frame=std::clamp(static_cast<int>(t*4.f),0,3);
        const SDL_FRect source{float(frame*96),0,96,96};
        const float size=kind==4?270.f:kind==3?232.f:kind==0?220.f:kind==1?205.f:185.f;
        const SDL_FRect dest{std::round(burst.x-offset-size*.5f),
                             std::round(burst.y-size*.5f),size,size};
        SDL_SetTextureAlphaMod(sheet,static_cast<Uint8>(255.f*(1.f-.22f*t)));
        SDL_RenderTextureRotated(renderer,sheet,&source,&dest,0,nullptr,
                                 burst.facing<0?SDL_FLIP_HORIZONTAL:SDL_FLIP_NONE);
        SDL_SetTextureAlphaMod(sheet,255);
    }
    void menuTile(float x,float y,float w,float h,unsigned fill,unsigned edge){
        rect(x+4,y+4,w,h,0x070b12);
        rect(x,y,w,h,edge);
        rect(x+3,y+3,w-6,h-6,fill);
        rect(x+6,y+6,w-12,2,edge,160);
        rect(x+6,y+h-8,w-12,2,0x0a1019);
    }
    void settings(const ControlLayout& layout){
        constexpr unsigned ink=0xe4dfd1,muted=0x9eacba,gold=0xc7a575;
        const char* names[]={"JOYSTICK","ATTACK","DASH","SWAP","SKILL 1","SKILL 2","SKILL 3","ULTIMATE"};
        if(layout.reposition){
            // Fullscreen editing: leave the gameplay and HUD visible. The entire
            // settings window is suppressed until DONE is pressed.
            rect(0,0,1280,720,0x07101b,38);
            menuTile(45,26,730,110,0x172331,0x596878);
            rect(57,38,5,84,gold);
            rect(68,37,683,2,0xc1e4ec,135);
            text("CONTROL EDIT MODE",78,43,3,ink);
            text("DRAG TO MOVE  /  PINCH TO RESIZE",78,80,2,muted);
            text(names[layout.selected],78,109,2,gold);
            number(static_cast<int>(std::round(layout.buttons[layout.selected].size)),213,109,2,gold);
            text("PX",256,109,2,gold);
            const auto& b=layout.buttons[layout.selected];
            for(float y:{b.y-4,b.y+b.size+1}){
                rect(b.x-4,y,17,3,gold);
                rect(b.x+b.size-13,y,17,3,gold);
            }
            for(float x:{b.x-4,b.x+b.size+1}){
                rect(x,b.y-4,3,17,gold);
                rect(x,b.y+b.size-13,3,17,gold);
            }
            menuTile(1030,80,205,65,0x304b58,gold);
            rect(1041,90,4,44,0xc1e4ec);
            text("DONE",1094,102,3,ink);
            return;
        }

        rect(0,0,1280,720,0x080b12,184);
        // PNG frame and side scroll carry the artwork; use SDL only for live overlays.
        if(settingsPanel_){
            SDL_FRect frame{ControlLayout::menuX,ControlLayout::menuY,ControlLayout::menuWidth,ControlLayout::menuHeight};
            SDL_RenderTexture(renderer,settingsPanel_,nullptr,&frame);
        }
        if(settingsWindow_){
            SDL_FRect frame{328,86,888,549};
            SDL_RenderTexture(renderer,settingsWindow_,nullptr,&frame);
        }else menuTile(328,86,888,549,0x121b29,0x536172);
        // Etched metallic corner ornaments, translucent to preserve PNG details.
        for(int corner=0;corner<4;++corner){
            const float x=(corner&1)?1180.f:347.f;
            const float y=(corner&2)?598.f:101.f;
            rect(x,y,16,3,0xc7a575,170);rect(x,y,3,16,0xc7a575,170);
            rect(x+5,y+5,3,3,0xb6d9e4,130);
        }
        rect(360,612,800,2,0x716b65,120);

        const char* tabs[]={"SOUND","CONTROLS","GAME","EXIT"};
        if(!settingsPanel_){
            menuTile(ControlLayout::menuX,ControlLayout::menuY,
                     ControlLayout::menuWidth,ControlLayout::menuHeight,0x121b29,0x536172);
            text("SETTINGS",54,124,4,ink);
        }
        for(int i=0;i<4;++i){
            const float x=ControlLayout::tabX,y=ControlLayout::tabY[i];
            const bool active=i==static_cast<int>(layout.page);
            if(!settingsPanel_){
                menuTile(x,y,ControlLayout::tabWidth,ControlLayout::tabHeight,
                         i==3?0x382c2b:0x1b2838,0x697480);
                text(tabs[i],x+24,y+23,3,ink);
            }
            if(active){
                constexpr unsigned blue=0xa4daef;
                const float w=ControlLayout::tabWidth,h=ControlLayout::tabHeight;
                rect(x+13,y+5,w-26,2,blue,230);
                rect(x+13,y+h-7,w-26,2,blue,230);
                rect(x+4,y+11,5,h-22,blue);
                rect(x+w-12,y+14,3,h-28,gold,210);
                rect(x+13,y+13,10,4,gold);
                rect(x+13,y+h-17,10,4,gold);
            }
        }

        menuTile(376,107,716,67,0x172331,0x596878);
        rect(389,117,5,46,gold);
        rect(400,116,670,2,0xc1e4ec,105);
        text(tabs[layout.page],407,126,4,ink);
        rect(399,175,230,2,gold,150);
        rect(389,187,755,2,0x596878);
        menuTile(ControlLayout::backX,ControlLayout::backY,
                 ControlLayout::backWidth,ControlLayout::backHeight,0x213242,0x596878);
        rect(ControlLayout::backX+7,ControlLayout::backY+8,3,48,0xc1e4ec);
        text("BACK",ControlLayout::backX+10,ControlLayout::backY+22,3,ink);

        if(layout.page==ControlLayout::Controls){
            text("TOUCH BUTTON POSITION AND SIZE",390,218,2,muted);
            menuTile(440,280,600,74,0x304b58,gold);
            rect(453,291,6,52,0xc1e4ec);
            rect(470,292,553,2,0xc8d8de,140);
            text("EDIT ON SCREEN",614,306,3,ink);
            text("EDITING MODE",440,376,2,muted);
            menuTile(440,405,600,62,0x0f1926,0x465363);
            rect(452,416,4,40,gold,185);
            text("DRAG BUTTONS  /  PINCH WITH TWO FINGERS",469,428,2,ink);
            menuTile(445,485,285,68,0x302832,0x705d68);
            rect(454,495,4,46,0xa77b83);
            text("RESET",544,509,3,ink);
            menuTile(750,485,285,68,0x213242,0x596878);
            rect(759,495,4,46,0xc1e4ec);
            text("SAVE",856,509,3,ink);
            text("DONE RETURNS TO SETTINGS",440,565,2,muted);
        }else if(layout.page==ControlLayout::Sound){
            text("MUSIC AND COMBAT EFFECT LEVELS",390,218,2,muted);
            for(int i=0;i<2;++i){
                const int level=i==0?layout.bgm:layout.sfx;
                const float y=i==0?270.f:390.f;
                text(i==0?"BGM":"SFX",390,y+20,3,ink);
                menuTile(460,y,575,58,0x0f1926,0x465363);
                rect(466,y+6,563,2,0x8da9b7,100);
                for(int tick=0;tick<20;++tick){
                    const bool filled=tick*5<level;
                    const float px=470+tick*28;
                    rect(px,y+17,23,25,filled?0x688f9d:0x263747);
                    rect(px,y+17,23,3,filled?0xc1e4ec:0x34495b,filled?205:130);
                    if(filled)rect(px+3,y+39,17,2,0x243846);
                }
                rect(470+std::clamp(level/5,0,19)*28.f,y+12,23,3,gold);
                number(level,1052,y+20,3,gold);
            }
        }else{
            text("COMBAT FEEDBACK",390,218,2,muted);
            for(int i=0;i<2;++i){
                const bool on=i==0?layout.screenShake:layout.showNumbers;
                const float y=285.f+i*100.f;
                menuTile(440,y,600,75,0x1b2838,0x465363);
                rect(452,y+11,5,53,on?0xc1e4ec:0x7c626c);
                rect(464,y+10,425,2,0x8da9b7,115);
                text(i==0?"SCREEN SHAKE":"DAMAGE NUMBERS",467,y+26,3,ink);
                menuTile(905,y+14,106,46,on?0x304b58:0x302832,on?gold:0x705d68);
                rect(912,y+21,4,32,on?0xc1e4ec:0x815f6b);
                text(on?"ON":"OFF",928,y+25,3,on?ink:muted);
            }
        }
    }

public:
    explicit Presentation(SDL_Renderer* r):renderer(r){}
    ~Presentation(){for(auto* image:flashActorTextures_)if(image)SDL_DestroyTexture(image);for(auto* image:flashFxTextures_)if(image)SDL_DestroyTexture(image);for(auto* fx:{goldLance_,goldTrail_,goldOrb_,goldImpact_,riftSlash_,riftTrace_})if(fx)SDL_DestroyTexture(fx);for(auto* fx:whiteFxTextures_)if(fx)SDL_DestroyTexture(fx);if(whiteNightMotion_)SDL_DestroyTexture(whiteNightMotion_);if(whiteNightLance_)SDL_DestroyTexture(whiteNightLance_);if(titleTexture_)SDL_DestroyTexture(titleTexture_);if(titleBg_)SDL_DestroyTexture(titleBg_);if(titleKnightFg_)SDL_DestroyTexture(titleKnightFg_);if(titleCape_)SDL_DestroyTexture(titleCape_);if(titleMenu_)SDL_DestroyTexture(titleMenu_);if(knightSkillMotion_)SDL_DestroyTexture(knightSkillMotion_);if(knightSkillFx_)SDL_DestroyTexture(knightSkillFx_);for(auto* image:knightSkillTextures_)if(image)SDL_DestroyTexture(image);for(auto* image:whiteSkillTextures_)if(image)SDL_DestroyTexture(image);if(mageMotion_)SDL_DestroyTexture(mageMotion_);if(mageRun_)SDL_DestroyTexture(mageRun_);if(enemyBarFrame_)SDL_DestroyTexture(enemyBarFrame_);if(executeIcon_)SDL_DestroyTexture(executeIcon_);if(settingsGear_)SDL_DestroyTexture(settingsGear_);if(settingsPanel_)SDL_DestroyTexture(settingsPanel_);if(settingsWindow_)SDL_DestroyTexture(settingsWindow_);for(auto* texture:textures)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:slimeTextures_)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestTextures_)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestLayerTextures_)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestV3Textures_)if(texture)SDL_DestroyTexture(texture);}
    void setForestSettings(ForestSettings settings) {forestGenerator_.regenerate(settings);}
    void setDemonCastleEntrance(bool enabled) {castleEntranceActive_=enabled && castleEntrance_.ready();}
    bool demonCastleEntranceReady() const {return castleEntrance_.ready();}
    const ForestGenerator& forestGenerator() const {return forestGenerator_;}
    static std::string asset(const char* name){
#ifdef SDL_PLATFORM_ANDROID
        return name;
#else
        const char* base=SDL_GetBasePath();return std::string(base?base:"")+"assets/"+name;
#endif
    }
    bool loadTitle(){
        // Canonical game title assets use the title/ namespace.
        titleTexture_=IMG_LoadTexture(renderer,asset("title/eclipse.png").c_str());
        if(!titleTexture_){SDL_SetError("ECLIPSE ASSET ERROR: title/eclipse.png missing or invalid");return false;}
        titleCape_=IMG_LoadTexture(renderer,asset("title/cape.png").c_str());
        if(!titleCape_){SDL_SetError("ECLIPSE ASSET ERROR: title/cape.png missing or invalid");return false;}
        if(titleCape_){
            SDL_SetTextureScaleMode(titleCape_,SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(titleCape_,SDL_BLENDMODE_BLEND);
        }
        // A clean background and foreground armor are paired: never mix half
        // of a new title set with the old flattened composition.
        titleBg_=IMG_LoadTexture(renderer,asset("title/bg_clean.png").c_str());
        titleKnightFg_=IMG_LoadTexture(renderer,asset("title/knight_fg.png").c_str());
        if(!titleBg_ || !titleKnightFg_){
            if(titleBg_){SDL_DestroyTexture(titleBg_);titleBg_=nullptr;}
            if(titleKnightFg_){SDL_DestroyTexture(titleKnightFg_);titleKnightFg_=nullptr;}
            SDL_Log("ECLIPSE title: clean three-layer pack absent; using legacy backdrop");
        }else{
            for(SDL_Texture* layer:{titleBg_,titleKnightFg_}){
                SDL_SetTextureScaleMode(layer,SDL_SCALEMODE_NEAREST);
                SDL_SetTextureBlendMode(layer,SDL_BLENDMODE_BLEND);
            }
        }
        SDL_SetTextureScaleMode(titleTexture_,SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(titleTexture_,SDL_BLENDMODE_BLEND);
        titleMenu_=IMG_LoadTexture(renderer,asset("ui/title_menu.png").c_str());
        if(!titleMenu_){SDL_SetError("ECLIPSE ASSET ERROR: ui/title_menu.png missing or invalid");return false;}
        SDL_SetTextureScaleMode(titleMenu_,SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(titleMenu_,SDL_BLENDMODE_BLEND);
        if(!settingsGear_)settingsGear_=IMG_LoadTexture(renderer,asset("ui/settings_gear.png").c_str());
        if(!settingsGear_){SDL_SetError("ECLIPSE ASSET ERROR: ui/settings_gear.png missing or invalid");return false;}
        SDL_SetTextureScaleMode(settingsGear_,SDL_SCALEMODE_NEAREST);
        return true;
    }

    static constexpr std::array<SDL_FRect,2> titleOptions{{{404,552,472,62},{404,624,472,62}}};
    static constexpr SDL_FRect titleGear{1204,24,52,52};
    static constexpr SDL_FRect titleDeveloperQA{500,478,280,54};
    static constexpr SDL_FRect titleBack{26,25,190,50};
    static constexpr float titleReadyTime=0.85f;


    bool loadReviewedFlashSheets(){
        auto load=[&](const FlashSheet& sheet,SDL_Texture*& texture)->bool{
            if(!sheet.file||texture)return true;
            const std::string file=asset(sheet.file);
            texture=IMG_LoadTexture(renderer,file.c_str());
            if(!texture){
                const std::string cause=SDL_GetError();
                SDL_SetError("ECLIPSE ASSET ERROR: %s: %s",file.c_str(),cause.c_str());
                return false;
            }
            float w=0,h=0;
            if(!SDL_GetTextureSize(texture,&w,&h)||w!=sheet.cellW*sheet.columns||h!=sheet.cellH*sheet.rows){
                SDL_SetError("ECLIPSE ASSET ERROR: %s expected %dx%d, got %.0fx%.0f",
                    file.c_str(),sheet.cellW*sheet.columns,sheet.cellH*sheet.rows,w,h);
                SDL_DestroyTexture(texture);texture=nullptr;return false;
            }
            SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
            return true;
        };
        for(size_t i=0;i<flashActors.size();++i)if(!load(flashActors[i],flashActorTextures_[i]))return false;
        for(size_t i=0;i<flashEffects.size();++i)if(!load(flashEffects[i],flashFxTextures_[i]))return false;
        return true;
    }
    void reviewedFlashSprite(const FlashSheet& sheet,SDL_Texture* texture,int frame,float x,float y,int facing,Uint8 alpha=255,double angle=0){
        if(!texture||!sheet.file)return;
        x+=sheet.offsetX*facing;y+=sheet.offsetY;
        frame=std::clamp(frame,0,sheet.count-1);
        const SDL_FRect source{float(frame%sheet.columns*sheet.cellW),float(frame/sheet.columns*sheet.cellH),float(sheet.cellW),float(sheet.cellH)};
        const float anchorX=facing>0?sheet.pivotX:sheet.cellW-sheet.pivotX;
        const SDL_FRect dest{std::round(x-offset-anchorX*sheet.scale),std::round(y-sheet.pivotY*sheet.scale),
                            sheet.cellW*sheet.scale,sheet.cellH*sheet.scale};
        const SDL_FPoint pivot{anchorX*sheet.scale,sheet.pivotY*sheet.scale};
        SDL_SetTextureAlphaMod(texture,alpha);
        SDL_RenderTextureRotated(renderer,texture,&source,&dest,angle,&pivot,facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
        SDL_SetTextureAlphaMod(texture,255);
    }
    static int reviewedFlashFrame(const FlashSheet& sheet,float age,float duration,float contact=-1){
        float position=std::clamp(age/std::max(.001f,duration),0.f,1.f)*sheet.count;
        if(sheet.contactFrame>=0&&contact>0&&contact<duration){
            position=age<contact?age/contact*sheet.contactFrame:
                sheet.contactFrame+(age-contact)/(duration-contact)*(sheet.count-sheet.contactFrame);
        }
        return std::clamp(static_cast<int>(position),0,sheet.count-1);
    }

    static bool hasReviewedFlashFx(FlashFx role){
        return flashEffects[static_cast<int>(role)].file!=nullptr;
    }
    bool reviewedFlashFx(FlashFx role,float age,float x,float y,int facing,bool front,double angle=0,float playbackDuration=0){
        const int index=static_cast<int>(role);
        const auto& sheet=flashEffects[index];
        if(!sheet.file||sheet.front!=front||!flashFxTextures_[index])return false;
        const float duration=playbackDuration>0?playbackDuration:sheet.duration;
        if(!sheet.loop&&(age<0||age>=duration))return false;
        const float t=sheet.loop?std::fmod(std::max(0.f,age),duration):age;
        const int frame=std::clamp(static_cast<int>(t*sheet.fps),0,sheet.count-1);
        reviewedFlashSprite(sheet,flashFxTextures_[index],frame,x,y,facing,255,angle);
        return true;
    }

    static float reviewedFlashImpactY(const Combat& g,FlashFx role,const LightBurst& burst){
        if(!flashEffects[static_cast<int>(role)].targetFeet)return burst.y;
        // Only effects explicitly reviewed as ground-authored use target feet.
        float ground=burst.y,best=128.f;
        for(const auto& enemy:g.enemies){
            const auto bounds=enemyBounds(enemy);
            const float distance=std::abs(enemy.x-burst.x)+
                std::abs((bounds.top+bounds.bottom)*.5f-burst.y);
            if(distance<best){best=distance;ground=enemy.y;}
        }
        return ground;
    }
    void updateReviewedFlashEffects(const Combat& g,const Animation& a){
        if(g.clock==reviewedFlashClock_)return;
        const auto& p=g.player;
        if(g.clock<reviewedFlashClock_){
            reviewedFlashEvents_.clear();reviewedFlashBurstSerial_=0;
            reviewedFlashMove_=WhiteMove::None;reviewedFlashMoveAge_=0;
            reviewedFlashLandingSerial_=p.landingSerial;
            reviewedFlashDash_=reviewedFlashSwap_=reviewedFlashAirborne_=false;
        }
        reviewedFlashClock_=g.clock;
        reviewedFlashEvents_.erase(std::remove_if(reviewedFlashEvents_.begin(),reviewedFlashEvents_.end(),
            [&](const ReviewedFlashEvent& event){return g.clock-event.born>=flashEffects[static_cast<int>(event.role)].duration;}),
            reviewedFlashEvents_.end());
        auto spawn=[&](FlashFx role,float x,float y,int facing,bool follow,float delay=0.f){
            if(hasReviewedFlashFx(role)){
                const auto& sheet=flashEffects[static_cast<int>(role)];
                reviewedFlashEvents_.push_back({role,x,y,g.clock-delay-sheet.spawnAgeSeconds,facing,follow});
            }
        };
        const bool newMove=p.whiteMove!=reviewedFlashMove_||p.whiteAge<reviewedFlashMoveAge_;
        const float before=newMove?0.f:reviewedFlashMoveAge_;
        if(p.whiteNight&&p.whiteMove!=WhiteMove::None){
            FlashFx role=FlashFx::Slash;float trigger=0;
            switch(p.whiteMove){
                case WhiteMove::Slash:role=FlashFx::Slash;trigger=.184f;break;
                case WhiteMove::Rush:role=FlashFx::Rush;trigger=.125f;break;
                case WhiteMove::Command:role=FlashFx::Command;trigger=.23f;break;
                case WhiteMove::Blink:role=FlashFx::BlinkOut;trigger=0;break;
                case WhiteMove::Verdict:role=FlashFx::Verdict;trigger=.30f;break;
                default:break;
            }
            const float speed=p.whiteMove==WhiteMove::Slash?std::max(.001f,p.whiteSlashSpeed):1.f;
            // Reviewed source anticipation/peak timing is expressed in real
            // effect seconds. Preserve gameplay contact even with attack boosts.
            const auto& fxSheet=flashEffects[static_cast<int>(role)];
            float skippedPrelude=0;
            if(fxSheet.triggerStartAgeSeconds>=0){
                trigger=fxSheet.triggerStartAgeSeconds;
            }else if(trigger>0){
                trigger-=fxSheet.triggerLeadSeconds*speed;
                skippedPrelude=std::max(0.f,-trigger/speed);
                trigger=std::max(0.f,trigger);
            }
            if((newMove&&trigger==0)||(before<trigger&&p.whiteAge>=trigger)){
                const float sourceAge=std::max(0.f,p.whiteAge-trigger)/speed+skippedPrelude+
                    (p.whiteMove==WhiteMove::Verdict?.20f:0.f);
                spawn(role,p.x,p.y,p.facing,p.whiteMove!=WhiteMove::Blink,sourceAge);
            }
        }
        for(const auto& burst:g.lightBursts){
            if(burst.visualSerial<=reviewedFlashBurstSerial_)continue;
            reviewedFlashBurstSerial_=std::max(reviewedFlashBurstSerial_,burst.visualSerial);
            const float age=std::max(0.f,.24f-burst.life);
            if(burst.kind==LightFx::Impact){
                // One verdict collision publishes both burst kinds. Keep the
                // large reviewed verdict hit, not a second stacked impact.
                const bool verdictPair=hasReviewedFlashFx(FlashFx::LanceHit)&&
                    std::any_of(g.lightBursts.begin(),g.lightBursts.end(),[&](const LightBurst& other){
                        return other.kind==LightFx::Verdict&&std::abs(other.life-burst.life)<.001f&&
                               std::abs(other.x-burst.x)<1.f&&std::abs(other.y-burst.y)<1.f;
                    });
                if(!verdictPair)spawn(FlashFx::Impact,burst.x,
                    reviewedFlashImpactY(g,FlashFx::Impact,burst),burst.facing,false,age);
            }else if(burst.kind==LightFx::Verdict){
                spawn(FlashFx::LanceHit,burst.x,reviewedFlashImpactY(g,FlashFx::LanceHit,burst),
                      burst.facing,false,age);
            }else if(burst.kind==LightFx::Arrive&&p.whiteMove==WhiteMove::Blink)
                spawn(FlashFx::BlinkIn,burst.x,burst.y+48.f,burst.facing,false,age);
        }
        const bool dash=p.whiteNight&&p.dashLeft>0;
        const bool swap=p.whiteNight&&p.swapFlash>0;
        const bool airborne=p.whiteNight&&!p.grounded;
        if(dash&&!reviewedFlashDash_)spawn(FlashFx::Dash,p.x,p.y,p.facing,true,.15f-p.dashLeft);
        if(swap&&!reviewedFlashSwap_)spawn(FlashFx::Swap,p.x,p.y,p.facing,true,.24f-p.swapFlash);
        if(airborne&&!reviewedFlashAirborne_&&p.vy<0)spawn(FlashFx::Jump,p.x,p.y,p.facing,false);
        if(p.whiteNight&&p.landingSerial!=reviewedFlashLandingSerial_&&a.state==10)
            spawn(FlashFx::Land,p.x,p.y,p.facing,false);
        reviewedFlashDash_=dash;reviewedFlashSwap_=swap;reviewedFlashAirborne_=airborne;
        reviewedFlashLandingSerial_=p.landingSerial;
        reviewedFlashMove_=p.whiteMove;reviewedFlashMoveAge_=p.whiteAge;
    }
    void drawReviewedFlashEffects(const Combat& g,const Animation&,bool front){
        const auto& p=g.player;
        for(const auto& event:reviewedFlashEvents_){
            reviewedFlashFx(event.role,g.clock-event.born,event.follow?p.x:event.x,event.follow?p.y:event.y,
                            event.follow?p.facing:event.facing,front);
        }
        if(p.whiteNight&&hasReviewedFlashFx(FlashFx::Orb)){
            for(int i=0;i<p.storedLightLances;++i){
                const float phase=g.clock*2.8f+i*6.2831853f/p.storedLightLances;
                reviewedFlashFx(FlashFx::Orb,g.clock,p.x+std::cos(phase)*57.f,
                    p.y-61.f+std::sin(phase)*37.f,1,front);
            }
        }
        for(const auto& lance:g.lightLances){
            if(lance.delay>0)continue;
            const FlashFx role=lance.verdict?FlashFx::SkyLance:FlashFx::Lance;
            const int index=static_cast<int>(role);
            const auto& sheet=flashEffects[index];
            if(!sheet.file||sheet.front!=front||!flashFxTextures_[index])continue;
            // Main review specifies the nonblank flight-body range. It stays
            // visible until actual collision/removal, independent of sheet fade.
            if(sheet.flightFirst>=0&&sheet.flightLast>=sheet.flightFirst){
                const int span=sheet.flightLast-sheet.flightFirst+1;
                const int frame=sheet.flightFirst+
                    static_cast<int>(lance.visualFlightAge*sheet.flightFrameFps)%span;
                reviewedFlashSprite(sheet,flashFxTextures_[index],frame,lance.x,lance.y,1,255,
                                    lance.angle-sheet.nativeDirectionDegrees);
            }else{
                reviewedFlashFx(role,lance.visualFlightAge,lance.x,lance.y,1,front,
                                lance.angle-sheet.nativeDirectionDegrees);
            }
        }
    }
    bool drawReviewedFlashActor(const Combat& g,const Animation& a){
        const auto& p=g.player;
        if(!p.whiteNight||p.health<=0)return false;
        FlashActor role=FlashActor::Idle;
        float phase=0,age=0,duration=1,contact=-1;
        bool phaseDriven=false;
        if(p.whiteMove!=WhiteMove::None){
            constexpr FlashActor moves[]{FlashActor::Idle,FlashActor::Basic,FlashActor::Skill1,
                FlashActor::Skill2,FlashActor::Skill3,FlashActor::Ultimate};
            role=moves[static_cast<int>(p.whiteMove)];
            const auto clip=whiteClips[static_cast<int>(p.whiteMove)];
            age=p.whiteAge;duration=clip.duration;contact=clip.contact;
        }else if(p.dashLeft>0){
            role=FlashActor::Dash;age=.15f-p.dashLeft;duration=.15f;
        }else if(p.swapFlash>0){
            role=FlashActor::Swap;age=.24f-p.swapFlash;duration=.24f;
        }else if(!p.grounded){
            role=p.vy<0?FlashActor::Jump:FlashActor::Fall;
            phase=a.posePhase;phaseDriven=true;
        }else if(a.state==10){
            role=FlashActor::Land;phase=a.posePhase;phaseDriven=true;
        }else if(a.state==1||a.state==11){
            role=FlashActor::Run;phase=a.travelPhase;phaseDriven=true;
        }else{
            const auto& idle=flashActors[static_cast<int>(role)];
            age=idle.duration>0?std::fmod(a.time,idle.duration):0;
            duration=std::max(.001f,idle.duration);
        }
        const int index=static_cast<int>(role);
        const auto& sheet=flashActors[index];
        if(!sheet.file)return false; // This role has no newly approved replacement.
        auto* texture=flashActorTextures_[index];
        if(!texture)return false; // loadWhiteNight already reports required load failure.
        const int frame=phaseDriven?std::clamp(static_cast<int>(phase*sheet.count),0,sheet.count-1):
            reviewedFlashFrame(sheet,age,duration,contact);
        if(p.hurtTint>0)SDL_SetTextureColorMod(texture,255,150,160);
        reviewedFlashSprite(sheet,texture,frame,p.x,p.y,p.facing);
        SDL_SetTextureColorMod(texture,255,255,255);
        return true;
    }

    bool loadWhiteNight(){
        if(whiteNightLance_&&whiteNightMotion_)return loadReviewedFlashSheets();
        const auto path=asset("ui/white_night_lance.png");
        auto* texture=IMG_LoadTexture(renderer,path.c_str());
        if(!texture){
            const std::string cause=SDL_GetError();
            SDL_SetError("ECLIPSE ASSET ERROR: %s: %s",path.c_str(),cause.c_str());
            return false;
        }
        float width=0,height=0;
        if(!SDL_GetTextureSize(texture,&width,&height)||width!=64||height!=3){
            SDL_DestroyTexture(texture);
            SDL_SetError("ECLIPSE ASSET ERROR: %s expected 64x3, got %.0fx%.0f",path.c_str(),width,height);
            return false;
        }
        SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
        const auto motionPath=asset("ui/white_night_motion.png");
        auto* motion=IMG_LoadTexture(renderer,motionPath.c_str());
        if(!motion){
            const std::string cause=SDL_GetError();
            SDL_DestroyTexture(texture);
            SDL_SetError("ECLIPSE ASSET ERROR: %s: %s",motionPath.c_str(),cause.c_str());
            return false;
        }
        if(!SDL_GetTextureSize(motion,&width,&height)||width!=512||height!=1440){
            SDL_DestroyTexture(motion);
            SDL_DestroyTexture(texture);
            SDL_SetError("ECLIPSE ASSET ERROR: %s expected 512x1440, got %.0fx%.0f",motionPath.c_str(),width,height);
            return false;
        }
        SDL_SetTextureScaleMode(motion,SDL_SCALEMODE_NEAREST);
        whiteNightLance_=texture;
        whiteNightMotion_=motion;
        // Gold/white effects are mandatory game assets, not optional decoration.
        // Validate dimensions so the game fails visibly instead of drawing fake fallbacks.
        auto loadGold=[&](SDL_Texture*& destination,const char* name,float expectedW,float expectedH)->bool{
            if(destination)return true;
            const std::string file=asset(name);
            destination=IMG_LoadTexture(renderer,file.c_str());
            if(!destination){
                SDL_SetError("ECLIPSE ASSET ERROR: required PNG %s missing or invalid",file.c_str());
                return false;
            }
            float w=0,h=0;
            if(!SDL_GetTextureSize(destination,&w,&h)||w!=expectedW||h!=expectedH){
                SDL_SetError("ECLIPSE ASSET ERROR: %s expected %.0fx%.0f, got %.0fx%.0f",
                    file.c_str(),expectedW,expectedH,w,h);
                SDL_DestroyTexture(destination);destination=nullptr;
                return false;
            }
            SDL_SetTextureScaleMode(destination,SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(destination,SDL_BLENDMODE_BLEND);
            return true;
        };
        if(!loadGold(goldLance_,"ui/lance_gold.png",96,24)||
           !loadGold(goldTrail_,"ui/trail_gold.png",128,32)||
           !loadGold(goldOrb_,"ui/orb_gold.png",64,64)||
           !loadGold(goldImpact_,"ui/impact_gold.png",96,96)||
           !loadGold(riftSlash_,"ui/rift_slash.png",192,64)||
           !loadGold(riftTrace_,"ui/rift_trace.png",256,32))return false;
        return loadReviewedFlashSheets();
    }

    static constexpr std::array<SDL_FRect,4> qaCharacterButtons{{
        {410,374,460,52},{410,436,460,52},{410,498,460,52},{530,562,220,44}
    }};

    void drawTitle(float seconds,int page=0,bool loggingIn=false,int qaFirstJob=-1){
        constexpr float width=1280, height=720;
        constexpr float logoBand=195;
        const SDL_FRect lowerScene{0,logoBand,width,height-logoBand};
        const SDL_FRect lowerSource{0,logoBand*865/height,1536,865-logoBand*865/height};
        const float reveal=std::clamp((seconds-0.12f)/0.73f,0.f,1.f);
        const float eased=reveal*reveal*(3-2*reveal);
        SDL_SetRenderDrawColor(renderer,11,10,18,255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer,titleBg_?titleBg_:titleTexture_,&lowerSource,&lowerScene);

        // Source-aligned transparent cape is the only deforming texture.
        // The old mesh pinned the far-left cloth tips to its boundary, making
        // the center stretch while the visible tail barely moved.
        // Overscan the alpha bounds; pin the shoulder (upper-right) instead.
        constexpr int columns=28, rows=20;
        std::array<SDL_Vertex,(columns+1)*(rows+1)> vertices{};
        for(int row=0;row<=rows;++row){
            const float v=static_cast<float>(row)/rows;
            for(int col=0;col<=columns;++col){
                const float u=static_cast<float>(col)/columns;
                const float x=185.f+505.f*u, y=215.f+415.f*v;
                const float loose=std::pow(1.f-u,1.6f);
                const float vertical=std::clamp((v-0.02f)/0.26f,0.f,1.f);
                const float shoulderPin=std::clamp((690.f-x)/75.f,0.f,1.f);
                const float weight=loose*vertical*shoulderPin;
                const float phase=seconds*2.45f+u*6.8f+v*2.1f;
                const float flutter=std::sin(phase)+0.20f*std::sin(phase*1.7f+v*3.f);
                vertices[row*(columns+1)+col]={{
                    x+weight*4.0f*std::sin(phase+1.f),
                    y+weight*12.f*flutter
                },{1,1,1,1},{x/width,y/height}};
            }
        }
        static const auto indices=[]{
            std::array<int,columns*rows*6> triangles{};
            int i=0;
            for(int row=0;row<rows;++row){
                for(int col=0;col<columns;++col){
                    const int a=row*(columns+1)+col, b=a+columns+1;
                    triangles[i++]=a;triangles[i++]=a+1;triangles[i++]=b;
                    triangles[i++]=a+1;triangles[i++]=b+1;triangles[i++]=b;
                }
            }
            return triangles;
        }();
        // A transparent source-aligned cape layer prevents nearby characters,
        // knight armor and sword from deforming with the cloth mesh.
        if(titleCape_){
            // The clean backdrop contains no baked-in cape; draw cloth fully
            // opaque in its solid pixels to avoid a static double-cape ghost.
            SDL_SetTextureAlphaMod(titleCape_,titleBg_?255:220);
            SDL_RenderGeometry(renderer,titleCape_,vertices.data(),static_cast<int>(vertices.size()),
                               indices.data(),static_cast<int>(indices.size()));
        }
        // Re-cover the cape root with stationary armor, helmet and sword.
        if(titleBg_ && titleKnightFg_){
            const SDL_FRect screen{0.f,0.f,width,height};
            SDL_RenderTexture(renderer,titleKnightFg_,nullptr,&screen);
        }

        // The logo and eclipse share the upper artwork strip. Fade its
        // lower edge into the fully visible scene while the lettering appears.
        const SDL_FColor faded{1,1,1,eased}, visible{1,1,1,1};
        constexpr float fadeStart=logoBand-55;
        const SDL_Vertex logo[]={
            {{0,0},faded,{0,0}},{{width,0},faded,{1,0}},
            {{0,fadeStart},faded,{0,fadeStart/height}},{{width,fadeStart},faded,{1,fadeStart/height}},
            {{0,logoBand},visible,{0,logoBand/height}},{{width,logoBand},visible,{1,logoBand/height}}
        };
        const int strip[]={0,1,2,1,3,2,2,3,4,3,5,4};
        SDL_RenderGeometry(renderer,titleTexture_,logo,6,strip,12);
        if(seconds>=titleReadyTime&&page==2){
            rect(386,320,508,304,0x101223);
            rect(386,320,508,3,0xad95c7);
            text(qaFirstJob<0?"QA: PICK FIRST JOB":"QA: PICK SECOND JOB",453,340,3,0xffe2b7);
            constexpr const char* labels[]={"1 KNIGHT","2 FIRE MAGE","3 WHITE NIGHT","BACK"};
            for(size_t i=0;i<qaCharacterButtons.size();++i){
                const auto& target=qaCharacterButtons[i];
                const SDL_FRect source{0,0,112,30};
                SDL_RenderTexture(renderer,titleMenu_,&source,&target);
                const bool chosen=qaFirstJob==static_cast<int>(i);
                text(labels[i],target.x+(target.w-std::strlen(labels[i])*18.f)*.5f,
                    target.y+(target.h-21)*.5f,3,chosen?0xffd17c:0xece3f4);
                if(chosen)rect(target.x+7,target.y+10,5,target.h-20,0xffd17c);
            }
        }else if(seconds>=titleReadyTime){
            // Screens: MAIN=0, START_OPTIONS=1, QA=2, GEAR=3, SETTINGS=4.
            constexpr const char* labels[]={"START","VACODE LOGIN","PLAY AS GUEST","SETTINGS","EXIT","BACK"};
            const char* first=page==0?labels[0]:page==1?labels[1]:page==3?labels[3]:labels[5];
            const char* second=page==1?labels[2]:page==3?labels[4]:nullptr;
            const char* menuLabels[]={first,second};
            for(int i=0;i<(second?2:1);++i){
                const auto& target=titleOptions[i];
                const SDL_FRect source{0.f,float((i==0?0:1)*30),112.f,30.f};
                SDL_SetTextureAlphaMod(titleMenu_,static_cast<Uint8>(230+25*std::sin(seconds*2.4f+i*.8f)));
                SDL_RenderTexture(renderer,titleMenu_,&source,&target);
                const size_t length=std::strlen(menuLabels[i]);
                text(menuLabels[i],target.x+(target.w-length*18.f)*.5f,target.y+19,3,i==0?0xffe2b7:0xe1e0ef);
            }
            SDL_SetTextureAlphaMod(titleMenu_,255);
            if(page!=0)text("BACK",35,40,2,0xe1d4bf);
            if(page==1){
                text("CHOOSE YOUR JOURNEY",423,516,2,0xbeb2d9);
                if(loggingIn)text("VERIFYING VACODE ACCOUNT",425,478,2,0xf0d5aa);
            }else if(page==3){
                text("OPTIONS",576,450,2,0xbeb2d9);
                const SDL_FRect qaSource{0,0,112,30};
                SDL_RenderTexture(renderer,titleMenu_,&qaSource,&titleDeveloperQA);
                text("DEV QA",titleDeveloperQA.x+76,titleDeveloperQA.y+17,3,0xffd17c);
            }
            else if(page==4){
                rect(344,325,592,164,0x080a16,245);
                rect(344,325,592,3,0xb09ac9);
                text("SETTINGS",555,345,3,0xf0d5aa);
                text("CONTROL SETTINGS AVAILABLE IN GAME",389,406,2,0xe2d9e8);
                text("PRESS BACK TO RETURN",442,442,2,0xbbaccb);
            }
        }
        if(seconds>=titleReadyTime&&page!=2&&settingsGear_)
            SDL_RenderTexture(renderer,settingsGear_,nullptr,&titleGear);
        SDL_RenderPresent(renderer);
    }

    bool load(){
        // Preload the castle for its future tutorial scene, but do not require
        // tutorial art to start an unrelated forest gameplay scene.
        // The explicit castle preview request checks readiness and raises an error.
        if(!castleEntrance_.load(renderer,[&](const char* name){return asset(name);})) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "ECLIPSE castle asset preload failed (castle scene unavailable): %s",SDL_GetError());
        } else SDL_Log("ECLIPSE demon castle entrance assets: ready");
        constexpr const char* files[]={"knight_motion.png","ui/attack_states.png","ui/dash_states.png","ui/swap_skill_states.png","ui/joystick_arrows_new.png"};
        for(int i=0;i<5;++i){const auto path=asset(files[i]);SDL_Surface* image=IMG_Load(path.c_str());
            if(!image){SDL_Log("Required PNG missing or invalid: %s (%s)",path.c_str(),SDL_GetError());return false;}
            if(i==0&&(image->w!=motionWidth*motionColumns||image->h!=motionHeight*motionRows)){SDL_Log("Expected %dx%d motion atlas",motionWidth*motionColumns,motionHeight*motionRows);SDL_DestroySurface(image);return false;}
            // Compact oversized button sheets once, keeping only 128px per state on the GPU.
            if(i>0){int cols=(i==1||i==2||i==3)?2:1,rows=i==3?2:1;auto* scaled=SDL_ScaleSurface(image,128*cols,128*rows,SDL_SCALEMODE_NEAREST);SDL_DestroySurface(image);image=scaled;if(!image)return false;}
            textures[i]=SDL_CreateTextureFromSurface(renderer,image);SDL_DestroySurface(image);
            if(!textures[i])return false;
            SDL_SetTextureScaleMode(textures[i],SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(textures[i],SDL_BLENDMODE_BLEND);
        }
        constexpr const char* slimes[]={"monsters/slime_green.png","monsters/slime_red.png","monsters/slime_purple.png"};
        for(int i=0;i<3;++i){
            auto path=asset(slimes[i]);auto* image=IMG_Load(path.c_str());
            if(!image){SDL_Log("Required slime atlas missing: %s",path.c_str());return false;}
            if(image->w!=slimeCell*4||image->h!=slimeCell*4){SDL_DestroySurface(image);SDL_Log("Invalid slime atlas dimensions: %s",path.c_str());return false;}
            slimeTextures_[i]=SDL_CreateTextureFromSurface(renderer,image);SDL_DestroySurface(image);
            if(!slimeTextures_[i])return false;
            SDL_SetTextureScaleMode(slimeTextures_[i],SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(slimeTextures_[i],SDL_BLENDMODE_BLEND);
        }
        // Optional sprite textures allow older asset packs to keep rendering.
        auto loadOptionalTexture=[&](const char* name,int width=0,int height=0)->SDL_Texture* {
            const auto path=asset(name);SDL_Surface* image=IMG_Load(path.c_str());
            if(!image)return nullptr;
            if(width&&(image->w!=width||image->h!=height)){SDL_DestroySurface(image);return nullptr;}
            SDL_Texture* texture=SDL_CreateTextureFromSurface(renderer,image);
            SDL_DestroySurface(image);
            if(texture){SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);}
            return texture;
        };
        constexpr const char* whiteFxFiles[]={
            "ui/white_fx_slash.png","ui/white_fx_rush.png",
            "ui/white_fx_depart.png","ui/white_fx_arrive.png",
            "ui/white_fx_verdict.png"};
        for(int i=0;i<5;++i){
            whiteFxTextures_[i]=loadOptionalTexture(whiteFxFiles[i],384,96);
            if(!whiteFxTextures_[i]){
                SDL_SetError("ECLIPSE ASSET ERROR: required White Night PNG %s missing or invalid (expected 384x96)",whiteFxFiles[i]);
                return false;
            }
        }
        knightSkillMotion_=loadOptionalTexture("knight_skill_motion.png",
            skillMotionWidth*skillMotionColumns,motionHeight*skillMotionRows);
        knightSkillFx_=loadOptionalTexture("knight_skill_fx_layer.png",
            skillMotionWidth*skillMotionColumns,motionHeight*skillFxRows);
        if(!knightSkillFx_){
            SDL_SetError("ECLIPSE ASSET ERROR: knight_skill_fx_layer.png missing or invalid (expected 448x564)");
            return false;
        }
        mageRun_=loadOptionalTexture("mage_run.png",motionWidth*motionColumns,motionHeight);
        mageMotion_=loadOptionalTexture("mage_motion.png",motionWidth*mageMotionColumns,motionHeight*mageMotionRows);
        if(!settingsGear_)settingsGear_=loadOptionalTexture("ui/settings_gear.png");
        settingsPanel_=loadOptionalTexture("ui/settings_menu_panel.png");
        settingsWindow_=loadOptionalTexture("ui/settings_window.png");
        enemyBarFrame_=loadOptionalTexture("ui/enemy_bar_frame.png");
        executeIcon_=loadOptionalTexture("ui/execute_skull.png");
        knightSkillTextures_[0]=loadOptionalTexture("ui/knight_skill_stab.png");
        knightSkillTextures_[1]=loadOptionalTexture("ui/knight_skill_sweep.png");
        knightSkillTextures_[2]=loadOptionalTexture("ui/knight_skill_prep.png");
        constexpr const char* whiteButtonFiles[]={
            "ui/white_skill_rush.png","ui/white_skill_release.png",
            "ui/white_skill_blink.png","ui/white_skill_verdict.png"};
        for(int i=0;i<4;++i){
            whiteSkillTextures_[i]=loadOptionalTexture(whiteButtonFiles[i],128,128);
            if(!whiteSkillTextures_[i]){
                SDL_SetError("ECLIPSE ASSET ERROR: required White Night skill button PNG %s missing or invalid (expected 128x128)",whiteButtonFiles[i]);
                return false;
            }
        }
        // Optional sprite library: the procedural renderer remains functional without it.
        // Only startup loads textures; no per-frame filesystem work.
        constexpr const char* forestFiles[40]={
            "background/forest_far_01.png",
            "background/forest_far_02.png",
            "background/forest_far_03.png",
            "trees_large/tree_large_01.png",
            "trees_large/tree_large_02.png",
            "trees_large/tree_large_03.png",
            "trees_large/tree_large_04.png",
            "trees_large/tree_large_05.png",
            "trees_medium/tree_mid_01.png",
            "trees_medium/tree_mid_02.png",
            "trees_medium/tree_mid_03.png",
            "trees_medium/tree_mid_04.png",
            "bushes/bush_01.png",
            "bushes/bush_02.png",
            "bushes/bush_03.png",
            "bushes/bush_04.png",
            "bushes/bush_05.png",
            "grass/grass_01.png",
            "grass/grass_02.png",
            "grass/grass_03.png",
            "grass/grass_04.png",
            "rocks/rock_01.png",
            "rocks/rock_02.png",
            "rocks/rock_03.png",
            "terrain/tile_ground_01.png",
            "terrain/tile_ground_02.png",
            "terrain/tile_ground_03.png",
            "terrain/tile_ground_04.png",
            "details/fallen_log_01.png",
            "details/flower_01.png",
            "details/hanging_vines_01.png",
            "details/mushroom_01.png",
            "details/mushroom_02.png",
            "details/stump_01.png",
            "plants/plant_01.png",
            "plants/plant_02.png",
            "plants/plant_03.png",
            "effects/effect_01.png",
            "effects/effect_02.png",
            "effects/effect_03.png"
        };
        for(size_t i=0;i<forestTextures_.size();++i) {
            auto name=asset((std::string("forest/")+forestFiles[i]).c_str());
            SDL_Surface* surface=IMG_Load(name.c_str());
            if(!surface)continue;
            forestTextures_[i]=SDL_CreateTextureFromSurface(renderer,surface);
            SDL_DestroySurface(surface);
            if(forestTextures_[i]) {
                SDL_SetTextureScaleMode(forestTextures_[i],SDL_SCALEMODE_NEAREST);
                SDL_SetTextureBlendMode(forestTextures_[i],SDL_BLENDMODE_BLEND);
            }
        }
        // Prefer new authored forest entrance, then v3. Both require all six
        // layers, so partial/corrupt asset sets can never render mixed scenes.
        // Reuse the same texture slots to avoid doubling GPU memory.
        auto loadSixLayers=[&](const std::array<ForestLayer,6>& scene)->bool {
            bool ready=true;
            for(size_t i=0;i<scene.size();++i) {
                const auto filename=asset(scene[i].file);
                SDL_Surface* surface=IMG_Load(filename.c_str());
                if(!surface){ready=false;break;}
                const int expectedWidth=i==0?1280:ForestLayerScene::worldWidth;
                if(surface->w!=expectedWidth || surface->h!=720) {
                    SDL_DestroySurface(surface);ready=false;break;
                }
                forestV3Textures_[i]=SDL_CreateTextureFromSurface(renderer,surface);
                SDL_DestroySurface(surface);
                if(!forestV3Textures_[i]){ready=false;break;}
                SDL_SetTextureScaleMode(forestV3Textures_[i],SDL_SCALEMODE_NEAREST);
                SDL_SetTextureBlendMode(forestV3Textures_[i],SDL_BLENDMODE_BLEND);
            }
            if(!ready) {
                for(auto*& texture:forestV3Textures_) {
                    if(texture)SDL_DestroyTexture(texture);
                    texture=nullptr;
                }
            }
            return ready;
        };
        forestEntranceReady_=loadSixLayers(ForestLayerScene::layersEntrance);
        forestV3Ready_=forestEntranceReady_
            || loadSixLayers(ForestLayerScene::layersV3);
        if(forestEntranceReady_)SDL_Log("ECLIPSE scene: forest entrance PNG 6/6");
        else if(forestV3Ready_)SDL_Log("ECLIPSE scene: forest v3 PNG 6/6");
        else SDL_Log("ECLIPSE scene: falling back to forest v2 or legacy art");
        // Optional composed forest v2: all four PNGs must load before activation.
        // Missing or invalid parts fall back to the existing complete legacy forest.
        forestLayersReady_=true;
        for(size_t i=0;i<ForestLayerScene::layers.size();++i){
            const auto filename=asset(ForestLayerScene::layers[i].file);
            SDL_Surface* layer=IMG_Load(filename.c_str());
            if(!layer){forestLayersReady_=false;break;}
            const int expectedWidth=i==0?1280:ForestLayerScene::worldWidth;
            if(layer->w!=expectedWidth || layer->h!=720){
                SDL_DestroySurface(layer);forestLayersReady_=false;break;
            }
            forestLayerTextures_[i]=SDL_CreateTextureFromSurface(renderer,layer);
            SDL_DestroySurface(layer);
            if(!forestLayerTextures_[i]){forestLayersReady_=false;break;}
            SDL_SetTextureScaleMode(forestLayerTextures_[i],SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(forestLayerTextures_[i],SDL_BLENDMODE_BLEND);
        }
        if(!forestLayersReady_){
            for(auto*& layer:forestLayerTextures_){
                if(layer)SDL_DestroyTexture(layer);
                layer=nullptr;
            }
            SDL_Log("Forest layers missing/incomplete: using previous PNG sprite forest");
        }
        return true;
    }
    // Pixel VFX use only authored PNG textures, transformed in SDL.
    // No runtime-drawn particles, circles, or procedural placeholder geometry.
    void goldenSprite(SDL_Texture* texture,float x,float y,float w,float h,
                      double angle=0,Uint8 alpha=255){
        if(!texture||w<=0||h<=0)return;
        const SDL_FRect dest{std::round(x-w*.5f),std::round(y-h*.5f),
                             std::round(w),std::round(h)};
        SDL_SetTextureAlphaMod(texture,alpha);
        SDL_RenderTextureRotated(renderer,texture,nullptr,&dest,angle,nullptr,SDL_FLIP_NONE);
        SDL_SetTextureAlphaMod(texture,255);
    }
    void goldenImpactParticles(const LightBurst& burst){
        if(!goldImpact_||!goldOrb_||!goldTrail_)return;
        constexpr float tau=6.28318530718f;
        const float t=std::clamp(1.f-burst.life/.24f,0.f,1.f);
        const float fade=1.f-t;
        const float x=burst.x-offset,y=burst.y;
        const float scale=burst.kind==LightFx::Verdict?1.6f:
                          burst.kind==LightFx::Command?1.20f:1.f;
        const float size=(38.f+85.f*t)*scale;
        goldenSprite(goldImpact_,x,y,size,size,0,
                     static_cast<Uint8>(235*fade));
        goldenSprite(goldImpact_,x,y,size*.73f,size*.73f,45,
                     static_cast<Uint8>(195*fade));
        goldenSprite(goldOrb_,x,y,28.f*scale*(1.f-t*.6f),
                     28.f*scale*(1.f-t*.6f),0,
                     static_cast<Uint8>(255*fade));
        // Twelve stable gold-white sparks and six streaks, each an actual PNG.
        // Fixed per-event phases prevent random-frame flicker.
        for(int i=0;i<12;++i){
            const float angle=tau*(i/12.f)+.25f;
            const float radial=(12.f+91.f*t)*scale*(i%3==0?1.13f:.83f);
            const float px=x+std::cos(angle)*radial;
            const float py=y+std::sin(angle)*radial;
            const float r=(i%3==0?15.f:9.f)*scale*(1.f-.48f*t);
            goldenSprite(goldOrb_,px,py,r,r,0,
                         static_cast<Uint8>(215*fade));
            if(i%2==0){
                goldenSprite(goldTrail_,px,py,37.f*scale*(1.f-.6f*t),
                             8.f*scale,angle*57.29578f,
                             static_cast<Uint8>(180*fade));
            }
        }
    }
    void draw(const Combat& g,const Animation& a,float camera,Actions held,const char* capture=nullptr,const char* reviewLabel=nullptr,const ControlLayout* customLayout=nullptr,std::array<float,2> stick={}){
        offset=std::round(camera)-640;SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer,21,35,52,255);SDL_RenderClear(renderer);
        forest(g,camera);for(const auto& e:g.enemies)if(!e.dead())enemy(e,g.enemyVisualClock);
        const auto& caster=g.player;
        updateReviewedFlashEffects(g,a);
        const bool verdictFreeze=caster.whiteNight&&caster.whiteMove==WhiteMove::Verdict&&
            caster.whiteAge>=.12f&&caster.whiteAge<.66f;
        if(verdictFreeze){
            // Desaturating veil; targets and their animations are frozen in the combat step.
            rect(0,0,1280,720,0x18140f,154);
        }
        // Authored PNG spatial cuts remain for a few frames; the incision
        // contracts rather than exploding into a diffuse magical glow.
        if(caster.whiteNight){
            for(const auto& scar:g.riftScars){
                if((scar.slash&&hasReviewedFlashFx(FlashFx::Slash))||
                   (!scar.slash&&hasReviewedFlashFx(FlashFx::Lance)))continue;
                const float progress=std::clamp(scar.life/(scar.slash?.21f:.15f),0.f,1.f);
                const float width=(scar.slash?230.f:91.f)*(0.35f+.65f*progress);
                const float height=(scar.slash?78.f:13.f)*(.4f+.6f*progress);
                goldenSprite(scar.slash?riftSlash_:riftTrace_,scar.x-offset,scar.y,
                    width,height,scar.angle,static_cast<Uint8>(255.f*std::min(1.f,progress*2.4f)));
            }
        }
        // Persistent yellow-white PNG markers show exactly which enemies
        // are eligible for the twenty-lance sky ultimate.
        if(caster.whiteNight&&goldOrb_&&goldImpact_){
            for(const auto& e:g.enemies){
                if(e.dead()||e.lightMark<=0)continue;
                const float x=e.x-offset;
                const float y=enemyBounds(e).top-18.f;
                const float pulse=1.f+.12f*std::sin(g.enemyVisualClock*8.f);
                goldenSprite(goldOrb_,x,y,34.f*pulse,34.f*pulse,0,210);
                goldenSprite(goldImpact_,x,y,49.f*pulse,49.f*pulse,0,105);
                for(int i=0;i<4;++i){
                    const float angle=i*1.5707963f+g.enemyVisualClock*1.7f;
                    goldenSprite(goldOrb_,x+std::cos(angle)*26.f,
                        y+std::sin(angle)*20.f,9,9,0,175);
                }
            }
        }
        // Gold-white light effects are sourced from PNG textures.
        SDL_Texture* lanceArt=goldLance_?goldLance_:whiteNightLance_;
        if(lanceArt&&caster.whiteNight&&caster.storedLightLances>0&&!hasReviewedFlashFx(FlashFx::Orb)){
            for(int i=0;i<caster.storedLightLances;++i){
                const float phase=g.clock*2.8f+i*6.2831853f/caster.storedLightLances;
                const float x=caster.x+std::cos(phase)*57.f-offset;
                const float y=caster.y-61.f+std::sin(phase)*37.f;
                // Idle stock is a compact sphere, never an already-formed spear.
                // Five bright cores communicate the full +40% basic-speed passive.
                const float pulse=1.f+.08f*std::sin(g.clock*5.f+i*1.7f);
                goldenSprite(goldOrb_,x,y,37.f*pulse,37.f*pulse,0,225);
                goldenSprite(goldOrb_,x,y,16.f,16.f,0,255);
                // Tiny orbiting light fragments are also sampled from the orb PNG.
                for(int j=0;j<3;++j){
                    const float sparkPhase=phase+j*2.0943951f;
                    goldenSprite(goldOrb_,x+std::cos(sparkPhase)*24.f,
                        y+std::sin(sparkPhase)*24.f,9,9,0,155);
                }
            }
        }
        if(lanceArt)for(const auto& lance:g.lightLances){
            if(hasReviewedFlashFx(lance.verdict?FlashFx::SkyLance:FlashFx::Lance))continue;
            // Do not reveal queued sky spears before their staggered drop.
            if(lance.verdict&&lance.delay>0.f)continue;
            const float x=lance.x-offset,y=lance.y;
            const float angle=lance.angle*.017453293f;
            const float dx=std::cos(angle),dy=std::sin(angle);
            // Newly released light remains an orb while rotating toward its
            // target, then stretches in place to full spear length over 0.11s.
            if(lance.morphTotal>0.f&&lance.delay>0.f){
                const float phase=std::clamp(1.f-lance.delay/lance.morphTotal,0.f,1.f);
                const float eased=phase*phase*(3.f-2.f*phase);
                const float bodyW=14.f+82.f*eased;
                const float bodyH=27.f-3.f*eased;
                goldenSprite(goldOrb_,x,y,38.f-15.f*eased,38.f-15.f*eased,0,245);
                goldenSprite(lanceArt,x,y,bodyW,bodyH,lance.angle,
                             static_cast<Uint8>(25+230*eased));
                if(eased>.35f)
                    goldenSprite(riftTrace_,x-std::cos(angle)*bodyW*.3f,
                        y-std::sin(angle)*bodyW*.3f,bodyW*.75f,8.f,
                        lance.angle,static_cast<Uint8>(160*eased));
                continue;
            }
            const bool launched=lance.delay<=0;
            if(launched){
                // A narrow, almost-white spatial cut travels with the spear.
                // The broader golden wake stays secondary to the clean incision.
                goldenSprite(riftTrace_,x-dx*93.f,y-dy*93.f,225.f,17.f,
                    lance.angle,235);
                // Four layered PNG trails create a bright white center and a long gold wake.
                for(int i=0;i<2;++i){
                    const float distance=34.f+i*39.f;
                    goldenSprite(goldTrail_,x-dx*distance,y-dy*distance,
                        120.f+27.f*i,21.f-i*5.f,lance.angle,
                        static_cast<Uint8>(95-i*35));
                }
                // Spatial particles persist behind each moving spear.
                for(int i=0;i<5;++i){
                    const float distance=28.f+i*28.f;
                    const float sideways=std::sin(g.clock*19.f+i*2.4f)*(7.f+i*2.f);
                    const float sx=x-dx*distance-dy*sideways;
                    const float sy=y-dy*distance+dx*sideways;
                    goldenSprite(goldOrb_,sx,sy,13.f-i,13.f-i,0,
                        static_cast<Uint8>(225-i*32));
                }
            }
            goldenSprite(goldOrb_,x-dx*19,y-dy*19,50,50,0,150);
            goldenSprite(lanceArt,x-dx*10,y-dy*10,96,24,lance.angle);
        }
        for(const auto& burst:g.lightBursts){
            // The basic sword slash is now driven by the *current sword pose*
            // below, not by a detached world-space burst with its own clock.
            const bool replaced=(burst.kind==LightFx::Impact&&hasReviewedFlashFx(FlashFx::Impact))||
                (burst.kind==LightFx::Verdict&&hasReviewedFlashFx(FlashFx::LanceHit))||
                (burst.kind==LightFx::Depart&&hasReviewedFlashFx(FlashFx::BlinkOut))||
                (burst.kind==LightFx::Arrive&&hasReviewedFlashFx(FlashFx::BlinkIn))||
                (burst.kind==LightFx::Rush&&hasReviewedFlashFx(FlashFx::Rush))||
                (burst.kind==LightFx::Command&&hasReviewedFlashFx(FlashFx::Command));
            if(burst.kind!=LightFx::Slash&&!replaced)goldenImpactParticles(burst);
        }
        for(const auto& b:g.projectiles){
            const int radius=b.empowered?14:8;
            const float x=b.x-offset;
            rect(x-b.facing*radius*1.5f-3,b.y-2,6,4,0xa33524);
            disk(x-b.facing*radius,b.y,radius*0.6f,0xdb4825);
            disk(x,b.y,radius,0xf16b32);
            disk(x+b.facing*2,b.y,b.empowered?7:4,0xffe7a1);
        }
        const auto& p=g.player;
        for(const auto& effect:g.bleedSpreads){
            const float progress=1-effect.life/.23f;
            for(int i=0;i<5;++i){
                const float t=std::clamp(progress-i*.09f,0.f,1.f);
                const float x=effect.fromX+(effect.toX-effect.fromX)*t;
                const float y=effect.fromY+(effect.toY-effect.fromY)*t-50*t*(1-t);
                world(x,y,4,i==0?6:3,i==0?0xf48694:0x9d394f);
            }
        }
        slamShockwave(p);
        if(p.form==0&&!p.whiteNight){
            if(isSkillMotion(a.state)&&knightSkillMotion_)knightSkillPngEffect(p,a);
            else if(isSkillMotion(a.state)&&a.state!=prepSkillState){
                Animation fallback=a;
                fallback.state=a.state==thrustSkillState?6:5;
                const auto& clip=motionClips[fallback.state];
                fallback.frame=clip.start+static_cast<int>(clip.contact*clip.count);
                if(a.frame>=motionClips[a.state].start+2)swordTrail(p,fallback);
            }else swordTrail(p,a);
        }
        SDL_Texture* actorTexture=textures[0];
        int columns=motionColumns,frame=a.frame,width=motionWidth,pivotX=motionPivotX;
        if(p.form==0&&isSkillMotion(a.state)){
            if(knightSkillMotion_){
                actorTexture=knightSkillMotion_;
                columns=skillMotionColumns;
                frame-=skillMotionStart;
                width=skillMotionWidth;
                pivotX=skillMotionPivotX;
            }else{
                const auto& clip=motionClips[a.state];
                const auto& fallback=motionClips[a.state==thrustSkillState?6:a.state==sweepSkillState?5:0];
                frame=fallback.start+(frame-clip.start)*(fallback.count-1)/(clip.count-1);
            }
        }
        if(p.form!=0){
            actorTexture=nullptr;
            if(a.state==1&&mageRun_&&p.mageCastLeft<=0){
                actorTexture=mageRun_;
                frame-=motionClips[1].start;
            }else if(mageMotion_&&a.state>=0&&!isSkillMotion(a.state)){
                // The mage atlas stores eight poses per clip. Sprint uses its own 16-frame strip.
                constexpr int rows[]={0,1,2,3,8,8,8,5,6,7,4,1,3};
                const auto clip=motionClips[a.state];
                const bool basicCast=p.mageCastLeft>0;
                const int row=(basicCast||(a.state>=4&&a.state<=6))?
                    (p.grounded?mageGroundCastRow:mageAirCastRow):rows[a.state];
                const int index=basicCast?
                    std::clamp(static_cast<int>((1-p.mageCastLeft/mageCastDuration)*mageMotionColumns),0,mageMotionColumns-1):
                    (a.state==2||a.state==3)?
                    mageAirPose(a.posePhase):
                    (a.frame-clip.start)*(mageMotionColumns-1)/std::max(1,clip.count-1);
                actorTexture=mageMotion_;
                columns=mageMotionColumns;
                frame=row*columns+index;
            }
        }
        if(p.form==0&&a.state==4){
            frame=knightWeakFrame(a.posePhase);
        }else if(p.form==0&&a.state==1){
            const int playbackFrame=std::clamp(
                static_cast<int>(a.travelPhase*knightSprintPlaybackFrames),0,knightSprintPlaybackFrames-1);
            frame=knightSprintStart+std::clamp(
                playbackFrame*knightSprintFrames/knightSprintPlaybackFrames,0,knightSprintFrames-1);
        }else if(p.form==0&&a.midpoint&&a.state!=4&&!isSkillMotion(a.state)){
            // Skill atlas has twelve actual display poses per clip, so do not
            // apply the old 4-key/4-midpoint row offset to those frames.
            frame+=knightMidpointStart;
        }
        drawReviewedFlashEffects(g,a,false);
        if(drawReviewedFlashActor(g,a)){
            // Approved separate sheets replace only their own motion role.
        }else if(p.whiteNight&&whiteNightMotion_&&(p.whiteMove!=WhiteMove::None||p.health>0)){
            int whiteFrame=30;
            if(p.whiteMove!=WhiteMove::None){
                const auto clip=whiteClips[static_cast<int>(p.whiteMove)];
                whiteFrame=clip.start+std::min(clip.count-1,static_cast<int>(p.whiteAge*clip.count/clip.duration));
                if(p.whiteMove==WhiteMove::Command&&p.whiteAge>=.18f&&p.whiteAge<=.36f)
                    whiteFrame=clip.start+6; // Hold the sword-pointing release silhouette.
                if(p.whiteMove==WhiteMove::Verdict&&p.whiteAge>=.23f&&p.whiteAge<.70f)
                    whiteFrame=clip.start+9; // Hold the off-hand fist during the time stop.
            }else if(a.state==7){
                whiteFrame=3;
            }else if(!p.grounded){
                // The takeoff and landing sprites share the same world foot anchor.
                // Airborne poses progress with vertical velocity, not horizontal travel.
                const float phase=std::clamp((p.vy+650.f)/1300.f,0.f,1.f);
                whiteFrame=8+std::min(6,static_cast<int>(phase*7));
            }else if(a.state==10){
                whiteFrame=15;
            }else if(a.state==1||a.state==11){
                // One distance-driven 32-frame run cycle at every movement speed.
                whiteFrame=whiteNightRunStart+std::clamp(
                    static_cast<int>(a.travelPhase*whiteNightRunFrames),0,whiteNightRunFrames-1);
            }
            const SDL_FRect source{float(whiteFrame%8*64),float(whiteFrame/8*96),64,96};
            const SDL_FRect dest{std::round(p.x-offset-32*motionScale),std::round(p.y-88*motionScale),64*motionScale,96*motionScale};
            if(p.whiteMove==WhiteMove::Rush&&p.whiteAge>=.125f&&p.whiteAge<.36f){
                SDL_SetTextureColorMod(whiteNightMotion_,255,245,173);
                for(int i=3;i>0;--i){
                    SDL_FRect ghost=dest;ghost.x-=p.facing*i*18;
                    SDL_SetTextureAlphaMod(whiteNightMotion_,static_cast<Uint8>(120-i*24));
                    SDL_RenderTextureRotated(renderer,whiteNightMotion_,&source,&ghost,0,nullptr,p.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
                }
                SDL_SetTextureAlphaMod(whiteNightMotion_,255);
            }else if(p.hurtTint>0)SDL_SetTextureColorMod(whiteNightMotion_,255,150,160);
            // The sword arc tracks the SAME clip clock (including stored-lance
            // attack-speed boosts) as the 15 authored slash poses. The PNG
            // silhouette/palette is unchanged; only position, scale and opacity
            // are registered to the active swing instead of a detached burst.
            if(p.whiteMove==WhiteMove::Slash && riftSlash_&&!hasReviewedFlashFx(FlashFx::Slash)){
                const auto slashClip=whiteClips[static_cast<int>(WhiteMove::Slash)];
                const float phase=std::clamp(p.whiteAge/slashClip.duration,0.f,1.f);
                constexpr float arcIn=.26f,arcPeak=.40f,arcOut=.76f;
                if(phase>=arcIn&&phase<arcOut){
                    const float emerge=std::clamp((phase-arcIn)/(arcPeak-arcIn),0.f,1.f);
                    const float decay=std::clamp((arcOut-phase)/(arcOut-arcPeak),0.f,1.f);
                    const float opacity=std::min(emerge,decay);
                    const float sweep=std::clamp((phase-arcIn)/(arcOut-arcIn),0.f,1.f);
                    // Follow the forward-moving blade, with the center of
                    // the incision close to the sword rather than 140px ahead.
                    const float bladeX=p.x-offset+p.facing*(43.f+33.f*sweep);
                    const float bladeY=p.y-60.f+7.f*sweep;
                    const float width=175.f+58.f*emerge;
                    const float height=45.f+24.f*emerge;
                    goldenSprite(riftSlash_,bladeX,bladeY,width,height,
                        p.facing>0?-14.f:194.f,
                        static_cast<Uint8>(225.f*opacity));
                }
            }
            SDL_RenderTextureRotated(renderer,whiteNightMotion_,&source,&dest,0,nullptr,p.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
            SDL_SetTextureColorMod(whiteNightMotion_,255,255,255);
            if(p.whiteMove==WhiteMove::Command&&p.whiteAge>=.08f&&p.whiteAge<.46f&&goldOrb_&&!hasReviewedFlashFx(FlashFx::Command)){
                // A sword-tip flash builds while the blade extends, then the
                // stored lances all release on the authored contact frame.
                const float local=p.whiteAge;
                const float strength=local<.23f?local/.23f:std::max(0.f,1.f-(local-.23f)/.23f);
                const float x=p.x+p.facing*92.f-offset,y=p.y-59.f;
                goldenSprite(goldOrb_,x,y,48.f+32.f*strength,48.f+32.f*strength,0,
                             static_cast<Uint8>(180*strength));
                for(int i=0;i<5;++i){
                    const float angle=(i-2.f)*.46f;
                    const float distance=24.f+47.f*strength;
                    goldenSprite(goldTrail_,x+p.facing*distance,y+std::sin(angle)*distance,
                        49.f+40.f*strength,10.f,p.facing>0?angle*57.29578:180-angle*57.29578,
                        static_cast<Uint8>(160*strength));
                }
            }
            if(p.whiteMove==WhiteMove::Verdict&&p.whiteAge>=.04f&&p.whiteAge<.91f&&(goldOrb_||whiteFxTextures_[3])){
                // Gather a compact sphere at the off-hand before the sky volley.
                const auto clip=whiteClips[static_cast<int>(WhiteMove::Verdict)];
                const float t=std::clamp(p.whiteAge/clip.contact,0.f,1.f);
                const float release=std::clamp((p.whiteAge-clip.contact)/.14f,0.f,1.f);
                const float pulse=1.f+.055f*std::sin(g.clock*14.f);
                const float size=(37.f+14.f*(1.f-t)+25.f*release)*pulse;
                const float handX=p.x-p.facing*24.f-offset;
                const float handY=p.y-57.f;
                const SDL_FRect glowSrc{0,0,96,96};
                SDL_Texture* orb=goldOrb_?goldOrb_:whiteFxTextures_[3];
                const SDL_FRect glowDst{std::round(handX-size*.5f),
                    std::round(handY-size*.5f),size,size};
                SDL_SetTextureAlphaMod(orb,static_cast<Uint8>(190+65*release));
                SDL_RenderTexture(renderer,orb,goldOrb_?nullptr:&glowSrc,&glowDst);
                SDL_SetTextureAlphaMod(orb,255);
                // Round motes orbit while gathering, then burst outward on release.
                for(int i=0;i<9;++i){
                    const float angle=i*.6981317f-p.whiteAge*(5.2f+(i%3)*.55f);
                    const float radius=(1.f-t)*50.f+10.f+release*(38.f+i%3*8.f);
                    const float x=handX+std::cos(angle)*radius;
                    const float y=handY+std::sin(angle)*radius*.76f;
                    const float mote=5.f+2.f*(1.f-t)+2.f*release;
                    goldenSprite(goldOrb_,x,y,mote,mote,0,
                        static_cast<Uint8>(215*(1.f-.45f*release)));
                }
            }
            if(p.whiteBoost>0 && whiteFxTextures_[3]){
                const SDL_FRect glowSrc{0,0,96,96};
                const SDL_FRect glowDst{p.x-offset-48,p.y-62,96,96};
                SDL_SetTextureAlphaMod(whiteFxTextures_[3],145);
                SDL_RenderTexture(renderer,whiteFxTextures_[3],&glowSrc,&glowDst);
                SDL_SetTextureAlphaMod(whiteFxTextures_[3],255);
            }
        }else{
        afterimages(g,a,actorTexture,frame,columns);
        if(actorTexture){
            SDL_FRect source{static_cast<float>(frame%columns*width),static_cast<float>(frame/columns*motionHeight),static_cast<float>(width),motionHeight};
            SDL_FRect dest{std::round(p.x-offset-(p.facing>0?pivotX:width-pivotX)*motionScale),std::round(p.y-motionPivotY*motionScale),width*motionScale,motionHeight*motionScale};
            const bool white=p.hurtTint>.13f,red=p.hurtTint>0&&!white;
            if(red)SDL_SetTextureColorMod(actorTexture,255,125,140);
            SDL_RenderTextureRotated(renderer,actorTexture,&source,&dest,0,nullptr,p.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
            if(red)SDL_SetTextureColorMod(actorTexture,255,255,255);
            if(white){
                SDL_SetTextureBlendMode(actorTexture,SDL_BLENDMODE_ADD);
                SDL_SetTextureColorMod(actorTexture,255,244,237);
                SDL_SetTextureAlphaMod(actorTexture,185);
                SDL_RenderTextureRotated(renderer,actorTexture,&source,&dest,0,nullptr,p.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
                SDL_SetTextureAlphaMod(actorTexture,255);
                SDL_SetTextureColorMod(actorTexture,255,255,255);
                SDL_SetTextureBlendMode(actorTexture,SDL_BLENDMODE_BLEND);
            }
        }else mage(p);
        }
        drawReviewedFlashEffects(g,a,true);
        if(p.swapFlash>0&&(!p.whiteNight||!hasReviewedFlashFx(FlashFx::Swap)))ring(p.x-offset,p.y-36,44,0xe3bd72);
        static const ControlLayout defaultLayout{};
        const ControlLayout& layout=customLayout?*customLayout:defaultLayout;
        if(layout.showNumbers)for(const auto& popup:g.popups)number(popup.amount,popup.x-offset-12,popup.y,3,popup.kind==DamageKind::Fire?0xff9155:(popup.kind==DamageKind::Bleed||popup.kind==DamageKind::Execute)?0xe76878:0xf2d48c);
        hud(g,held,layout,stick);
        if(layout.editing)settings(layout);
        // PNG-backed gothic gear and vertical menu. All actions use ControlLayout hitboxes.
        if(!layout.reposition&&settingsGear_){
            SDL_FRect gear{ControlLayout::gearX,ControlLayout::gearY,ControlLayout::gearSize,ControlLayout::gearSize};
            SDL_RenderTexture(renderer,settingsGear_,nullptr,&gear);
        }else if(!layout.reposition){rect(ControlLayout::gearX,ControlLayout::gearY,ControlLayout::gearSize,ControlLayout::gearSize,0x243746);text("CFG",ControlLayout::gearX+6,ControlLayout::gearY+21,3); }
        if(reviewLabel)text(reviewLabel,470,120,3);
        if(g.ended){rect(0,0,1280,720,0x111a28,240);text(g.won?"STAGE CLEAR":"GAME OVER",390,260,7,g.won?0x8be4b5:0xff8d99);text("TAP RETRY OR PRESS R",410,350,3);
            rect(510,412,260,65,0xe3bd72);rect(514,416,252,57,0x304039);text("RETRY",548,433,4);
        }
        if(capture){auto* image=SDL_RenderReadPixels(renderer,nullptr);if(image){SDL_SaveBMP(image,capture);SDL_DestroySurface(image);}}
        SDL_RenderPresent(renderer);
    }
};
}

