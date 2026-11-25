#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <cstdio>
#include <cstring>
#include <vector>
#include "core.hpp"
#include "forest_generator.hpp"
#include "forest_layers.hpp"
#include "demon_castle_entrance.hpp"
#include "story_director.hpp"
#include "memory_menu.hpp"
#include "flash_executioner_assets.hpp"
#include "space_ruler_assets.hpp"

namespace eclipse {
class Presentation {
    SDL_Renderer* renderer;
    std::array<SDL_Texture*,5> textures{};
    SDL_Texture* titleTexture_=nullptr;
    SDL_Texture* titleMenu_=nullptr;
    std::array<SDL_Texture*,static_cast<int>(FlashActor::Count)> flashActorTextures_{};
    std::array<SDL_Texture*,static_cast<int>(FlashFx::Count)> flashFxTextures_{};
    std::array<SDL_Texture*,castleEnemyCount> castleEnemyMoveTextures_{};
    std::array<SDL_Texture*,castleEnemyCount> castleEnemyAttackTextures_{};
    std::array<SDL_Texture*,castleEnemyCount> castleEnemyAttackVfxTextures_{};
    // Demon King: one 14-frame sheet per body motion, and one sprite sheet per pixelfx layer.
    std::array<std::array<SDL_Texture*,static_cast<int>(BossMotion::Count)>,2> bossMotionTextures_{};
    std::array<SDL_Texture*,static_cast<int>(BossFx::Count)> bossFxTextures_{};
    struct ReviewedFlashEvent {FlashFx role;float x,y,born;int facing;bool follow;float playbackDuration=0;};
    std::vector<ReviewedFlashEvent> reviewedFlashEvents_;
    float reviewedFlashClock_=-1,reviewedFlashMoveAge_=0;
    FlashMove reviewedFlashMove_=FlashMove::None;
    unsigned long long reviewedFlashBurstSerial_=0;
    bool reviewedFlashDash_=false,reviewedFlashSwap_=false,reviewedFlashGrounded_=true;
    SDL_Texture* goldLance_=nullptr;
    SDL_Texture* goldTrail_=nullptr;
    SDL_Texture* goldOrb_=nullptr;
    SDL_Texture* goldImpact_=nullptr;
    SDL_Texture* riftSlash_=nullptr;
    SDL_Texture* riftTrace_=nullptr;
    SDL_Texture* playerHealthFrame_=nullptr;
    SDL_Texture* enemyBarFrame_=nullptr;
    SDL_Texture* bossBarFrame_=nullptr;
    SDL_Texture* executeIcon_=nullptr;
    SDL_Texture* circularSkillButtons_=nullptr;
    float offset=0;
    int spaceRole_=-1;float spaceRoleBorn_=0;
    std::array<SDL_Texture*,12> spaceActorTextures_{};
    std::array<SDL_Texture*,6> spaceFxTextures_{};
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
        if(castleEntranceActive_ && castleEntrance_.ready()) {castleEntrance_.draw(camera,g.clock);return;}
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
        const auto spec=enemySpec(e.kind);
        const float scale=spec.scale;
        const float visualHeight=e.kind>=AshInfantry?spec.bodyHeight*scale:(e.kind==0?80:100)*scale;
        const float h=visualHeight;
        if(e.kind>=AshInfantry){
            const int index=e.kind-AshInfantry;
            const bool attacking=e.attackAge>=0;
            SDL_Texture* texture=attacking?castleEnemyAttackTextures_[index]:castleEnemyMoveTextures_[index];
            const bool king=e.kind>=DemonKingArmored;
            if(king&&attacking&&e.motion>0&&bossMotionTextures_[e.kind==DemonKingTrue][e.motion])
                texture=bossMotionTextures_[e.kind==DemonKingTrue][e.motion];   // the move's own body motion
            // During a blink the king is gone until the arrival flash opens.
            const bool blinkGone=king&&e.move!=BossMove::None&&bossStepDef(e.move,e.step).action==BossAction::Blink&&e.attackAge<.16f;
            const int frame=enemyFrame(e);
            const bool ownMotion=king&&attacking&&e.motion>0;
            const int columns=attacking?(ownMotion?bossMotionFrames:castleEnemyAttackFrames):castleEnemyMoveFrames;
            const float cell=static_cast<float>(enemyCell(e.kind));
            const SDL_FRect source{float(frame%columns)*cell,0.f,cell,cell};
            const float anchorX=e.facing>0?spec.pivotX:cell-spec.pivotX;
            // Story: arrival wake-up (rise from the floor) and Demon King armor-break shudder.
            const float wakeK=e.wake>0?std::clamp(e.wake/1.2f,0.f,1.f):0.f;
            const float shiftProgress=e.phaseShift>0&&e.phaseShiftTotal>0?1.f-e.phaseShift/e.phaseShiftTotal:0.f;
            const float jitter=e.phaseShift>0?std::sin(clock*70.f)*4.f*(1.f-shiftProgress*.6f):0.f;
            const SDL_FRect target{std::round(e.x-offset-anchorX*scale+jitter),
                std::round(e.y-spec.pivotY*scale+wakeK*34.f*scale),cell*scale,cell*scale};
            const bool shiftWhite=e.phaseShift>0;
            const bool eroded=e.darkStacks>=eclipse::stackMax,slowed=e.darkStacks>=3&&!eroded;   // void stacks tint the foe
            SDL_SetTextureColorMod(texture,(e.hitFlash>0||shiftWhite)?255:eroded?150:slowed?205:255,(e.hitFlash>0||shiftWhite)?135:eroded?110:slowed?190:255,(e.hitFlash>0||shiftWhite)?135:eroded?255:slowed?255:255);
            SDL_SetTextureAlphaMod(texture,blinkGone?0:static_cast<Uint8>(255.f*(1.f-wakeK)));
            SDL_RenderTextureRotated(renderer,texture,&source,&target,0,nullptr,
                e.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
            SDL_SetTextureColorMod(texture,255,255,255);SDL_SetTextureAlphaMod(texture,255);
            if(e.phaseShift>0)kingArmorShards(e,spec,scale,shiftProgress,clock);
            if(e.kind>=DemonKingArmored&&e.attackAge>=0&&e.move!=BossMove::None)kingMoveTelegraph(e,spec);
            if(e.wake>0)return;
            if(attacking&&frame>=castleEnemyEffectStartFrames[index]){
                SDL_Texture* vfx=castleEnemyAttackVfxTextures_[index];
                const SDL_FRect vfxSource{float(frame*416),0.f,416.f,256.f};
                const SDL_FRect vfxTarget{std::round(target.x-112.f*scale),
                    std::round(target.y-32.f*scale),416.f*scale,256.f*scale};
                SDL_RenderTextureRotated(renderer,vfx,&vfxSource,&vfxTarget,0,nullptr,
                    e.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
            }
        }else{
            const int frame=enemyFrame(e);
            SDL_FRect source{float(frame%4*slimeCell),float(frame/4*slimeCell),slimeCell,slimeCell};
            SDL_FRect target{std::round(e.x-offset-slimePivotX*scale),std::round(e.y-slimePivotY*scale),slimeCell*scale,slimeCell*scale};
            const unsigned body=e.kind==0?0x63c779:e.kind==1?0xd95762:0x8e72d9;
            disk(e.x-offset,e.y-visualHeight*.48f,27.f*scale,e.hitFlash>0?0xffe8df:body);
            rect(e.x-offset-17.f*scale,e.y-visualHeight*.56f,34.f*scale,24.f*scale,body);
            disk(e.x-offset-8.f*scale,e.y-visualHeight*.52f,3.f*scale,0xf5eee4);
            disk(e.x-offset+8.f*scale,e.y-visualHeight*.52f,3.f*scale,0xf5eee4);
        }
        // Pixel-art PNG bezel, with independent clipped fills for immediate HP and
        // the delayed white segment. 88x18, more than twice the former height.
        const float barX=std::round(e.x-offset-(e.boss?54.f:44.f)),barY=std::round(e.y-visualHeight-25);
        SDL_Texture* healthFrame=e.boss?bossBarFrame_:enemyBarFrame_;
        if(healthFrame){
            SDL_FRect dest{barX,barY,e.boss?108.f:88.f,18.f};
            SDL_RenderTexture(renderer,healthFrame,nullptr,&dest);
        } else {
            rect(barX,barY,e.boss?108.f:88.f,18,0x07090d);
            rect(barX+2,barY+2,e.boss?104.f:84.f,14,0x303943);
        }
        for(int i=0;i<e.darkStacks;++i){   // dark stacks: violet pips under the health bar
            const float px=e.x-offset-(e.darkStacks*10.f-2.f)*.5f+i*10.f,py=barY+20.f;
            rect(px-1,py-1,10,10,0x0b0618);rect(px,py,8,8,0x9b6bff);rect(px+2,py+2,3,3,0xe6d8ff);
        }
        if(e.lightMark>0){
            const float x=e.x-offset,y=barY-12;
            rect(x-2,y-7,4,14,0xffc640);
            rect(x-7,y-2,14,4,0xffc640);
            rect(x-2,y-2,4,4,0xfff6bf);
        }
        const float hpRatio=std::clamp(float(e.health)/e.maxHealth,0.f,1.f);
        const float trailRatio=std::clamp(std::max(float(e.health),e.trailingHealth)/e.maxHealth,0.f,1.f);
        const float fillX=barX+4,fillY=barY+5,fillW=e.boss?100.f:80.f;
        // Never render a minimum one-pixel fill for zero HP.
        if(trailRatio>0)rect(fillX,fillY,fillW*trailRatio,8,0x9a5c65);
        if(hpRatio>0)rect(fillX,fillY,fillW*hpRatio,8,e.boss?0x782b41:e.kind==2?0x873744:0x702731);
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
            const float x=e.x-offset-19,y=e.y-visualHeight-44;
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

    void spaceSprite(SDL_Texture* texture,const SpaceSheet& sheet,float age,float x,float y,int facing){
        int frame=static_cast<int>(std::max(0.f,age)*sheet.count/sheet.duration);
        frame=sheet.loop?frame%sheet.count:std::min(sheet.count-1,frame);
        const SDL_FRect src{float(frame%sheet.columns*sheet.width),float(frame/sheet.columns*sheet.height),float(sheet.width),float(sheet.height)};
        const float pivot=facing>0?sheet.pivotX:sheet.width-sheet.pivotX;
        const SDL_FRect dst{std::round(x-offset-pivot*sheet.scale),std::round(y-sheet.pivotY*sheet.scale),sheet.width*sheet.scale,sheet.height*sheet.scale};
        SDL_RenderTextureRotated(renderer,texture,&src,&dst,0,nullptr,facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
    }
    void spaceActor(const Combat& g,const Animation& a){
        const auto& p=g.player;int role=0;float age=g.clock;
        if(p.health<=0){role=6;age=a.time;}
        else if(p.hurtTint>0){role=5;age=.18f-p.hurtTint;}
        else if(p.spaceMove){role=6+p.spaceMove;age=p.spaceAge;}
        else if(!p.grounded){role=p.vy< -5?2:p.vy>5?4:3;}
        else if(std::abs(p.vx)>1)role=1;
        if(role!=spaceRole_||g.clock<spaceRoleBorn_){spaceRole_=role;spaceRoleBorn_=g.clock;}
        if(!p.spaceMove&&p.health>0&&p.hurtTint<=0)age=g.clock-spaceRoleBorn_;
        const float y=p.y-10+(!p.grounded&&p.health>0?std::sin(g.clock*3)*1.8f:0);
        spaceSprite(spaceActorTextures_[role],spaceActors[role],age,p.x,y,p.facing);
    }
    void drawSpaceEffects(const Combat& g){
        const auto& p=g.player;
        for(const auto& fx:g.spaceEffects){
            if(fx.kind==4)rect(0,0,viewWidth,viewHeight,0x10051f,static_cast<Uint8>(65*std::sin(std::min(1.f,fx.age/2.f)*3.14159265f)));
            spaceSprite(spaceFxTextures_[fx.kind],spaceEffects[fx.kind],fx.age,fx.x,fx.y,fx.facing);
        }
        if(p.form!=1||p.flashExecutioner||p.health<=0)return;
        if(p.spaceField>0)spaceSprite(spaceFxTextures_[1],eclipse::spaceEffects[1],3-p.spaceField,p.x,p.y-20,1);
        if(!p.grounded)spaceSprite(spaceFxTextures_[5],eclipse::spaceEffects[5],g.clock,p.x,p.y-12,p.facing);
    }
    bool loadSpaceRuler(){
        auto load=[&](const SpaceSheet& sheet,SDL_Texture*& texture){
            const auto path=asset(sheet.file);auto* surface=IMG_Load(path.c_str());
            const int expectedW=sheet.width*sheet.columns,expectedH=sheet.height*((sheet.count+sheet.columns-1)/sheet.columns);
            if(!surface||surface->w!=expectedW||surface->h!=expectedH){
                const int w=surface?surface->w:0,h=surface?surface->h:0;
                const std::string error="ECLIPSE ASSET ERROR: "+path+" expected "+std::to_string(expectedW)+"x"+std::to_string(expectedH)+" actual "+std::to_string(w)+"x"+std::to_string(h)+" "+SDL_GetError();
                if(surface)SDL_DestroySurface(surface);
                SDL_SetError("%s",error.c_str());SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",error.c_str());return false;
            }
            texture=SDL_CreateTextureFromSurface(renderer,surface);SDL_DestroySurface(surface);
            if(!texture){const std::string error="ECLIPSE ASSET ERROR: "+path+" "+SDL_GetError();SDL_SetError("%s",error.c_str());SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",error.c_str());return false;}
            SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);return true;
        };
        for(size_t i=0;i<spaceActorTextures_.size();++i)if(!load(spaceActors[i],spaceActorTextures_[i]))return false;
        for(size_t i=0;i<spaceFxTextures_.size();++i)if(!load(eclipse::spaceEffects[i],spaceFxTextures_[i]))return false;
        return true;
    }

    // One ImageGen + PixelLab Workbench sprite sheet holds all circular touch states.
    // Its 3x3 grid contains eight cells in frame order; the last cell is transparent.
    void icon(int frame,float x,float y,float size,bool down){
        if(frame<0||frame>=8||!textures[1])return;
        constexpr float frameSize=256.f,sourceInset=16.f,sourceSize=224.f,columns=3.f;
        SDL_FRect source{std::fmod(float(frame),columns)*frameSize+sourceInset,
                         std::floor(float(frame)/columns)*frameSize+sourceInset,sourceSize,sourceSize};
        SDL_FRect target{std::round(x),std::round(y),std::round(size),std::round(size)};
        SDL_SetTextureColorMod(textures[1],255,255,255);
        SDL_SetTextureAlphaMod(textures[1],down?255:238);
        SDL_RenderTexture(renderer,textures[1],&source,&target);
        SDL_SetTextureAlphaMod(textures[1],255);
    }
    void drawPixelSkillButton(const ControlRect& b,int slot,int atlasIndex,
                              float remaining,float duration,
                              bool pressed,unsigned accent,bool enabled){
        if(!circularSkillButtons_||atlasIndex<0||atlasIndex>=12)return;
        const float ready=remaining>0&&duration>0
            ?1.f-std::clamp(remaining/duration,0.f,1.f):1.f;
        const Uint8 shade=!enabled?150:remaining>0?204:pressed?232:255;
        constexpr float frameSize=256.f,sourceInset=16.f,sourceSize=224.f,columns=4.f;
        SDL_FRect source{std::fmod(float(atlasIndex),columns)*frameSize+sourceInset,
                         std::floor(float(atlasIndex)/columns)*frameSize+sourceInset,sourceSize,sourceSize};
        SDL_FRect target{std::round(b.x),std::round(b.y+(pressed?1.f:0.f)),
                         std::round(b.size),std::round(b.size)};
        SDL_SetTextureColorMod(circularSkillButtons_,shade,shade,shade);
        SDL_SetTextureAlphaMod(circularSkillButtons_,enabled?255:190);
        SDL_RenderTexture(renderer,circularSkillButtons_,&source,&target);
        SDL_SetTextureColorMod(circularSkillButtons_,255,255,255);
        SDL_SetTextureAlphaMod(circularSkillButtons_,255);

        const float barW=std::min(40.f,b.size-24.f),barH=3.f;
        const float barX=std::round(b.x+(b.size-barW)*.5f),barY=std::round(b.y+b.size-10.f);
        rect(barX,barY,barW,barH,0x090c11,235);
        if(enabled&&ready>0.f){
            const Uint8 barAlpha=pressed?255:235;
            rect(barX,barY,std::max(1.f,std::round(barW*ready)),barH,accent,barAlpha);
            if(ready>=1.f)rect(barX,barY,barW,1,0xd2d6d3,barAlpha);
        }
        if(slot>0)number(slot,std::round(b.x+b.size*.5f-3.f),std::round(b.y+5.f),2,
                          enabled?0xe4e7e3:0x70757a);
    }
    void skillButton(const ControlRect& b,int slot,int atlasIndex,
                     float remaining,float duration,bool pressed,unsigned accent,bool enabled=true){
        drawPixelSkillButton(b,slot,atlasIndex,remaining,duration,pressed,accent,enabled);
    }
    void playerHealthBar(const Player& p){
        constexpr float x=22,y=18,barX=x+49,barY=y+12,barWidth=230;
        if(playerHealthFrame_){
            SDL_FRect frame{std::round(x),std::round(y),288.f,46.f};
            SDL_RenderTexture(renderer,playerHealthFrame_,nullptr,&frame);
        }
        const float hp=std::clamp(float(p.health)/p.maxHealth(),0.f,1.f);
        const float trail=std::clamp(p.trailingHealth/p.maxHealth(),hp,1.f);
        const float liveWidth=std::round(barWidth*hp);
        const float trailWidth=std::round(barWidth*trail)-liveWidth;
        if(trailWidth>0){
            rect(barX+liveWidth,barY,trailWidth,22,0x71333d);
            rect(barX+liveWidth,barY,trailWidth,3,0xb26468);
            rect(barX+liveWidth,barY+19,trailWidth,3,0x48242e);
            rect(barX+liveWidth-1,barY,1,22,0xd0a4a0);
        }
        if(liveWidth>0){
            rect(barX,barY,liveWidth,22,0x722731);
            rect(barX,barY,liveWidth,4,0xae515a);
            rect(barX,barY+19,liveWidth,3,0x4a1925);
        }
        char label[24];
        std::snprintf(label,sizeof(label),"HP %d / %d",p.health,p.maxHealth());
        const float labelX=barX+(barWidth-12.f*std::char_traits<char>::length(label)+2.f)*.5f;
        text(label,labelX+2,barY+6,2,0x21131c);
        text(label,labelX,barY+4,2,0xffeee1);
    }
    void flashSkillButton(const ControlRect& b,int iconIndex,float remaining,float duration,
                          bool pressed,unsigned accent){
        if(iconIndex<0||iconIndex>=4)return;
        drawPixelSkillButton(b,0,8+iconIndex,remaining,duration,pressed,accent,true);
    }
    void flashSkillSlot(const ControlRect& box,int slot,int iconIndex,float remaining,float duration,
                        bool pressed,unsigned accent){
        if(iconIndex<0||iconIndex>=4)return;
        drawPixelSkillButton(box,slot,8+iconIndex,remaining,duration,pressed,accent,true);
    }
    void skillSlot(const ControlRect& box,int slot,int atlasIndex,
                   float remaining,float duration,bool pressed,unsigned accent,bool enabled){
        skillButton(box,slot,atlasIndex,remaining,duration,pressed,accent,enabled);
    }
    // ---- Demon King effects: code-drawn effect layers, never baked into the actor art ----
    void kingArmorShards(const Enemy& e,const EnemySpec& spec,float scale,float progress,float clock){
        const float cx=e.x-offset,cy=e.y-spec.bodyHeight*scale*.55f;
        for(int i=0;i<18;++i){
            const float angle=i*0.349f+((i*37)%11)*.07f;
            const float speed=150.f+(i%5)*55.f;
            const float x=cx+std::cos(angle)*speed*progress;
            const float y=cy+std::sin(angle)*speed*.55f*progress+300.f*progress*progress;
            const Uint8 alpha=static_cast<Uint8>(255.f*std::clamp(1.4f-progress,0.f,1.f));
            const unsigned colors[3]={0xd8a85a,0x2a1a3a,0x7a3aa8};
            const float size=6.f+(i%4)*3.f;
            rect(x,y,size,size*(i%2?1.6f:.8f),colors[i%3],alpha);
        }
        (void)clock;
    }
    // Readable warnings for every Demon King move. Colour language: crimson = melee reach,
    // violet = void magic. Warnings fade out at the moment of contact.
    void kingMoveTelegraph(const Enemy& e,const EnemySpec& spec){
        (void)spec;
        const BossStepDef d=bossStepDef(e.move,e.step);
        const float contactTime=d.duration*d.contact;
        const float since=e.attackAge-contactTime;
        const float prog=std::clamp(e.attackAge/std::max(.01f,contactTime),0.f,1.f);
        const float gy=e.y;
        auto strip=[&](float x0,float x1,float h,unsigned rgb,Uint8 alpha){
            rect(std::min(x0,x1)-offset,gy-h,std::abs(x1-x0),h+4,rgb,alpha);
        };
        if(d.action==BossAction::Burst){
            const float radius=kingBurstRadius*prog,cx=e.x-offset,cy=gy-6.f;
            if(since<0){ringAlpha(cx,cy,radius,0xb06bff,150,.28f);ringAlpha(cx,cy,kingBurstRadius,0x6a3aa0,60,.28f);}
            return;
        }
        if(since>=0)return;
        const Uint8 alpha=static_cast<Uint8>(40+110*prog);
        switch(d.action){
        case BossAction::Melee:case BossAction::DashSlash:{
            const auto b=enemyAttackBounds(e);
            strip(b.left,b.right,34,0xd04a6a,alpha);
            if(d.action==BossAction::DashSlash)strip(e.x,e.attackToX,6,0xff8aa0,static_cast<Uint8>(120+100*prog));
            break;}
        case BossAction::Sweep:
            strip(e.facing>0?e.x-40:e.x-300,e.facing>0?e.x+300:e.x+40,60,0xd04a6a,alpha);break;
        case BossAction::Leap:
            ringAlpha(e.attackToX-offset,gy-4,170,0xd04a6a,static_cast<Uint8>(60+150*prog),.28f);
            ringAlpha(e.attackToX-offset,gy-4,170*prog,0xff8aa0,200,.28f);break;
        case BossAction::Quake:
            strip(e.x-900*prog,e.x+900*prog,5,0xb06bff,static_cast<Uint8>(90+110*prog));break;
        case BossAction::Charge:
            strip(e.x,e.attackToX,48,0xd04a6a,static_cast<Uint8>(30+70*(.5f+.5f*std::sin(e.attackAge*24.f))));break;
        case BossAction::Wave:
            strip(e.x,e.x+e.facing*800.f*prog,5,0xb06bff,150);break;
        default:break;
        }
    }
    // One frame of a pixelfx sheet. `x` is the world position of the sheet's pivot (or of its impact
    // point when `anchorHit` is set); sheets face right and are mirrored for a left-facing caster.
    void drawBossFxFrame(BossFx kind,float age,float x,float y,float scale,bool flip,bool anchorHit=false,bool loop=false){
        const auto& sheet=bossFxSheets[static_cast<int>(kind)];
        auto* texture=bossFxTextures_[static_cast<int>(kind)];
        if(!texture||age<0)return;
        int i=static_cast<int>(age*sheet.fps);
        if(loop)i%=sheet.count;
        if(i>=sheet.count)return;
        const SDL_FRect source{float(i%sheet.columns*sheet.cellW),float(i/sheet.columns*sheet.cellH),float(sheet.cellW),float(sheet.cellH)};
        if(kind==BossFx::ImpactFrame){            // screen-wide: stretched over the whole view, a little translucent
            if(i>=6)return;                    // only the opening burst; the rest would hide the strike
            const SDL_FRect whole{0,0,viewWidth,viewHeight};
            SDL_SetTextureAlphaMod(texture,static_cast<Uint8>(95-i*10));SDL_RenderTexture(renderer,texture,&source,&whole);SDL_SetTextureAlphaMod(texture,255);
            return;
        }
        const float w=sheet.cellW*scale,h=sheet.cellH*scale,pivot=sheet.pivotX*scale;
        if(anchorHit)x-=sheet.hitDx*scale*(flip?-1.f:1.f);
        const SDL_FRect target{std::round(x-offset-(flip?w-pivot:pivot)),std::round(y-sheet.pivotY*scale),w,h};
        SDL_RenderTextureRotated(renderer,texture,&source,&target,0,nullptr,flip?SDL_FLIP_HORIZONTAL:SDL_FLIP_NONE);
    }
    // While the king attacks, the world behind him is dimmed so the (purple on purple) silhouette and the
    // effect layers stay readable. Drawn between the background and the king; the player is drawn later.
    float bossFocus_=0.f;
    void drawBossFocus(const Combat& g){
        bool attacking=false;
        for(const auto& e:g.enemies)if(!e.dead()&&e.kind>=DemonKingArmored&&e.attackAge>=0&&e.move!=BossMove::None)attacking=true;
        bossFocus_+=((attacking?1.f:0.f)-bossFocus_)*.18f;
        if(bossFocus_>.02f)rect(0,0,1280,720,0x090312,static_cast<Uint8>(120.f*bossFocus_));
    }
    // Layers that sit on the floor or behind the king are drawn before him; strikes and bursts on top.
    static bool fxBehind(BossFx k){return k==BossFx::RuneCircle||k==BossFx::Portal||k==BossFx::Circle||k==BossFx::HexSigil;}
    // Every Demon King effect is a separate pixelfx layer, drawn from Combat's event and hazard lists.
    void drawBossLayers(const Combat& g,bool back){
        for(const auto& h:g.hazards){
            if(fxBehind(h.fx)!=back)continue;
            const auto& sheet=bossFxSheets[static_cast<int>(h.fx)];
            const float peak=sheet.peak/sheet.fps;
            if(h.kind==HazardKind::Pillar){
                // The effect is timed so its impact frame lands exactly when the pillar erupts.
                const float age=h.age+peak-h.delay;
                if(h.age<h.delay&&h.fx!=BossFx::Circle&&h.fx!=BossFx::Lightning){
                    const float prog=h.age/h.delay;
                    ringAlpha(h.x-offset,h.y-4,h.radius,0xb06bff,static_cast<Uint8>(60+170*prog),.28f);
                }
                drawBossFxFrame(h.fx,age,h.x,h.y,h.fx==BossFx::Stars?1.6f:2.f,false);
            }else if(h.fx==BossFx::Spikes){
                // Blade bursts chained along the floor, one 264 px segment after another.
                const float step=264.f,segTime=step/h.speed;
                for(int k=0;k*step<h.range+step;++k)
                    drawBossFxFrame(BossFx::Spikes,h.age-k*segTime,h.startX+h.dir*k*step,h.y,1.f,h.dir<0);
            }else{
                drawBossFxFrame(h.fx,h.age,h.x,h.y-10,1.6f,h.dir<0,false,true);
            }
        }
        for(const auto& fx:g.bossFx){
            if(fxBehind(fx.kind)!=back)continue;
            const bool anchored=fx.kind==BossFx::Hammer||fx.kind==BossFx::CutUp||fx.kind==BossFx::Thrust||fx.kind==BossFx::Reap;
            drawBossFxFrame(fx.kind,fx.age,fx.x,fx.y,fx.scale,fx.facing<0,anchored,fx.life>0);
        }
    }
    // Sword-tip ribbon: the tip table gives the blade's position on every sheet frame, so a fading
    // three-tone streak can follow the real motion. It is its own layer; the actor art stays clean.
    struct TipPoint{float x,y,t;};
    std::vector<TipPoint> tipTrail_;
    void drawSwordTrail(const Combat& g){
        const Enemy* king=nullptr;
        for(const auto& e:g.enemies)if(!e.dead()&&e.kind>=DemonKingArmored&&e.wake<=0&&e.phaseShift<=0)king=&e;
        // The tip table holds the right-most pixel of each frame, which is the blade only for the armored
        // king and for the true form's lunge; elsewhere it would follow hair or cloak, so no ribbon there.
        if(!king||king->attackAge<0||(king->motion==0&&king->move==BossMove::None)||(king->kind==DemonKingTrue&&king->motion!=0)){tipTrail_.clear();return;}
        const int form=king->kind==DemonKingArmored?0:1,frame=std::clamp(enemyFrame(*king),0,king->motion>0?bossMotionFrames-1:castleEnemyAttackFrames-1);
        const auto* tip=bossTips[form][king->motion][frame];
        if(tip[0]>0){
            const float scale=enemySpec(king->kind).scale;
            const float x=king->x+king->facing*(tip[0]-128)*scale,y=king->y+(tip[1]-236)*scale;
            if(tipTrail_.empty()||std::hypot(x-tipTrail_.back().x,y-tipTrail_.back().y)>3.f)tipTrail_.push_back({x,y,g.clock});
        }
        const float life=.26f;
        tipTrail_.erase(std::remove_if(tipTrail_.begin(),tipTrail_.end(),[&](const TipPoint& p){return g.clock-p.t>life||g.clock<p.t;}),tipTrail_.end());
        for(size_t i=1;i<tipTrail_.size();++i){
            const auto& a=tipTrail_[i-1];const auto& b=tipTrail_[i];
            const float len=std::hypot(b.x-a.x,b.y-a.y);
            if(len<4.f||len>260.f)continue;                       // skip teleports and standing still
            const float fade=1.f-(g.clock-b.t)/life;
            const int steps=std::max(1,static_cast<int>(len/4.f));
            for(int k=0;k<=steps;++k){
                const float u=static_cast<float>(k)/steps,x=a.x+(b.x-a.x)*u-offset,y=a.y+(b.y-a.y)*u;
                const float w=2.f+14.f*fade*(.4f+.6f*u);
                rect(x-w*.5f,y-w*.5f,w,w,0x6b2cc0,static_cast<Uint8>(150*fade));
                rect(x-w*.32f,y-w*.32f,w*.64f,w*.64f,0xc684ff,static_cast<Uint8>(200*fade));
                rect(x-w*.14f,y-w*.14f,w*.28f,w*.28f,0xffffff,static_cast<Uint8>(255*fade));
            }
        }
    }
    // Dotted ground-plane ellipse, used for telegraphs and cues.
    void ringAlpha(float x,float y,float radius,unsigned rgb,Uint8 alpha,float flatten=1.f){
        const int points=std::max(16,static_cast<int>(radius/5));
        for(int k=0;k<points;++k){
            const float a=k*6.2831853f/points;
            rect(x+std::cos(a)*radius,y+std::sin(a)*radius*flatten,5,5,rgb,alpha);
        }
    }
    void chevron(float x,float y,float s,int dir,unsigned rgb,Uint8 alpha){
        for(int i=0;i<4;++i){
            if(dir==1){rect(x+i*s,y+i*s,s,s,rgb,alpha);rect(x+i*s,y+(6-i)*s,s,s,rgb,alpha);}
            else if(dir==-1){rect(x+(3-i)*s,y+i*s,s,s,rgb,alpha);rect(x+(3-i)*s,y+(6-i)*s,s,s,rgb,alpha);}
            else {rect(x+i*s,y+(3-i)*s,s,s,rgb,alpha);rect(x+(6-i)*s,y+(3-i)*s,s,s,rgb,alpha);}
        }
    }
    // Memory altar: the spot where the protagonist awoke. Light and motion only, no sentences.
    // (Code-drawn placeholder cue; authored art for it is still to be made.)
    void drawMemoryAltar(const Combat& g){
        if(g.castleArea!=0)return;
        const float t=g.clock;
        const float x=memoryAltarX-offset,y=floorY;
        const float pulse=.5f+.5f*std::sin(t*2.4f);
        ringAlpha(x,y-4,34+pulse*6,0xbfa8ff,static_cast<Uint8>(120+60*pulse),.28f);
        ringAlpha(x,y-4,58+pulse*8,0x8f78e0,static_cast<Uint8>(70+40*pulse),.28f);
        const float bob=std::sin(t*2.f)*4.f;
        rect(x-8,y-61+bob,16,16,0xbfa8ff,90);rect(x-5,y-58+bob,10,10,0xe6dcff,230);
        for(int k=0;k<6;++k){
            const float phase=std::fmod(t*.45f+k*.17f,1.f);
            rect(x+std::sin(k*2.1f+t)*22.f,y-14-phase*70.f,3,3,0xcfc2ff,static_cast<Uint8>(200*(1.f-phase)));
        }
        if(g.nearMemoryAltar())ringAlpha(x,y-4,46+pulse*10,0xffffff,200,.28f);
    }
    // Fallen enemies' memories streaming into the protagonist, and the pulse when one is eaten.
    void drawMemoryMotes(const Combat& g){
        const float px=g.player.x-offset,py=g.player.y-62;
        for(const auto& m:g.memoryMotes){
            const float u=std::clamp(m.age/.9f,0.f,1.f),k=u*u*(3.f-2.f*u);
            const float sx=m.x-offset;
            const float x=sx+(px-sx)*k,y=m.y+(py-m.y)*k-std::sin(u*3.1416f)*34.f;
            rect(x-6,y-6,12,12,0xbfa8ff,static_cast<Uint8>(90*(1.f-u)));
            rect(x-3,y-3,6,6,0xe6dcff,static_cast<Uint8>(255*(1.f-u*.4f)));
        }
        if(g.memoryFlash>0){
            const float k=1.f-g.memoryFlash/.9f;
            ringAlpha(px,py,18+k*70,0xe6dcff,static_cast<Uint8>(220*(1.f-k)));
            ringAlpha(px,py,10+k*46,0xbfa8ff,static_cast<Uint8>(180*(1.f-k)));
        }
    }
    void drawMemoryMenu(const Combat& g,const StoryView* story){
        namespace M=MemoryMenu;
        const int job=g.activeJob();
        rect(0,0,1280,720,0x05070d,170);
        rect(M::panelX-4,M::panelY-4,M::panelW+8,M::panelH+8,0xbfa8ff);
        rect(M::panelX,M::panelY,M::panelW,M::panelH,0x141a2b);
        text("MEMORY",M::panelX+28,M::panelY+26,3,0xe6dcff);
        number(g.progression.memories,M::panelX+235,M::panelY+30,3,0xe6dcff);
        text("MASTERY",M::panelX+300,M::panelY+26,2,0x9fd0ff);
        number(g.mastery.points(job),M::panelX+440,M::panelY+26,2,0x9fd0ff);
        rect(M::panelX+M::panelW-58,M::panelY+14,38,38,0x2a3150);text("X",M::panelX+M::panelW-47,M::panelY+22,3,0xe6dcff);
        // Two tabs: permanent stats (memories, kept) and the job's skill tree (mastery points, lost on death).
        const bool runTab=g.memoryTab==1;
        rect(M::panelX+28,M::panelY+66,220,32,runTab?0x1a2038:0x3a2f66);text("FOREVER",M::panelX+60,M::panelY+74,2,runTab?0x8f89aa:0xffffff);
        rect(M::panelX+260,M::panelY+66,220,32,runTab?0x2a4a66:0x1a2038);text("THIS RUN",M::panelX+290,M::panelY+74,2,runTab?0xffffff:0x8f89aa);
        if(!runTab){
            static constexpr const char* names[statCount]={"1 ATK","2 VIT","3 GRD","4 CRT","5 CDM","6 SPD","7 HST"};
            for(int i=0;i<statCount;++i){
                const auto stat=static_cast<Stat>(i);
                const int level=g.progression.level(job,stat),cost=g.progression.nextCost(job,stat);
                const bool can=g.progression.canUpgrade(job,stat);
                const float y=M::rowY+i*M::rowH;
                rect(M::panelX+16,y+2,M::panelW-32,M::rowH-6,can?0x232c4a:0x1a2038);
                text(names[i],M::panelX+30,y+12,2,0xe6dcff);
                for(int k=0;k<maxStatLevel;++k)rect(M::panelX+130+k*11,y+14,8,14,k<level?0xbfa8ff:0x2a3150);
                if(cost>0)number(cost,M::panelX+385,y+12,2,can?0xa6f0b0:0x7f7a96);
                else text("MAX",M::panelX+385,y+12,2,0xa6f0b0);
            }
        }else{
            // The job's skill tree: free positions, parent links, drag to scroll, tap to select, tap again to buy.
            const auto& tree=masteryTree(job);
            const int count=static_cast<int>(tree.size());
            rect(M::treeX,M::treeY,M::treeW,M::treeH,0x0e1220);
            const SDL_Rect clip{static_cast<int>(M::treeX),static_cast<int>(M::treeY),static_cast<int>(M::treeW),static_cast<int>(M::treeH)};
            SDL_SetRenderClipRect(renderer,&clip);
            for(int n=0;n<count;++n){                               // links first, so nodes sit on top of them
                float nx=0,ny=0;M::nodeRect(tree[n],g.memoryTreeScrollX,g.memoryTreeScrollY,nx,ny);
                for(int i=0;i<tree[n].parentCount;++i){
                    const int parent=tree[n].parents[i];if(parent<0||parent>=count)continue;
                    float px=0,py=0;M::nodeRect(tree[parent],g.memoryTreeScrollX,g.memoryTreeScrollY,px,py);
                    const float x0=px+M::nodeW*.5f,y0=py+M::nodeH*.5f,x1=nx+M::nodeW*.5f,y1=ny+M::nodeH*.5f;
                    const unsigned color=g.mastery.has(job,parent)?(g.mastery.unlocked(job,n)?0xffffff:0x6fb4ff):0x2a3150;
                    const int steps=std::max(10,static_cast<int>(std::hypot(x1-x0,y1-y0)/9.f));
                    for(int k=1;k<steps;++k){const float u=static_cast<float>(k)/steps;rect(x0+(x1-x0)*u-2,y0+(y1-y0)*u-2,4,4,color);}
                }
            }
            for(int n=0;n<count;++n){
                float nx=0,ny=0;M::nodeRect(tree[n],g.memoryTreeScrollX,g.memoryTreeScrollY,nx,ny);
                if(nx>M::treeX+M::treeW||nx+M::nodeW<M::treeX||ny>M::treeY+M::treeH||ny+M::nodeH<M::treeY)continue;   // off screen
                const bool own=g.mastery.has(job,n),open=g.mastery.unlocked(job,n),can=g.mastery.canBuy(job,n);
                rect(nx,ny,M::nodeW,M::nodeH,own?0x2f6aa8:can?0x1f4a3a:open?0x1f2a45:0x161b2c);
                if(can){const float pulse=.5f+.5f*std::sin(g.clock*6.f);   // buyable: a pulsing frame
                    const Uint8 a=static_cast<Uint8>(120+120*pulse);
                    rect(nx,ny,M::nodeW,2,0xa6f0b0,a);rect(nx,ny+M::nodeH-2,M::nodeW,2,0xa6f0b0,a);
                    rect(nx,ny,2,M::nodeH,0xa6f0b0,a);rect(nx+M::nodeW-2,ny,2,M::nodeH,0xa6f0b0,a);}
                if(n==g.memoryTreeSelected){   // selected: a white frame
                    rect(nx-2,ny-2,M::nodeW+4,2,0xffffff);rect(nx-2,ny+M::nodeH,M::nodeW+4,2,0xffffff);
                    rect(nx-2,ny-2,2,M::nodeH+4,0xffffff);rect(nx+M::nodeW,ny-2,2,M::nodeH+4,0xffffff);}
                text(tree[n].name,nx+6,ny+6,1,own?0xffffff:open?0xcfe6ff:0x5f6678);
                text(tree[n].tag,nx+6,ny+19,1,own?0xd8f0ff:open?0x9fb4cc:0x4a5062);
                // Corner marker: raw stat node = a solid square, signature/special node = a diamond of four dots.
                const unsigned mark=own?0xffffff:open?0x9fd0ff:0x4a5062;
                if(tree[n].raw)rect(nx+M::nodeW-16,ny+5,9,9,mark);
                else{rect(nx+M::nodeW-16,ny+8,3,3,mark);rect(nx+M::nodeW-11,ny+4,3,3,mark);rect(nx+M::nodeW-11,ny+12,3,3,mark);rect(nx+M::nodeW-6,ny+8,3,3,mark);}
                if(own)text("OWNED",nx+6,ny+40,2,0xa6f0b0);
                else if(open){number(tree[n].cost,nx+6,ny+40,2,can?0xa6f0b0:0x7f7a96);text("P",nx+30,ny+40,2,can?0xa6f0b0:0x7f7a96);}
                else{   // locked: how many parents are still missing
                    text("-",nx+6,ny+40,2,0x5f6678);number(tree[n].need-g.mastery.parentsOwned(job,n),nx+30,ny+40,2,0x5f6678);
                }
            }
            SDL_SetRenderClipRect(renderer,nullptr);
            rect(M::treeX,M::treeY,M::treeW,2,0x3a4366);rect(M::treeX,M::treeY+M::treeH-2,M::treeW,2,0x3a4366);
            rect(M::treeX,M::treeY,2,M::treeH,0x3a4366);rect(M::treeX+M::treeW-2,M::treeY,2,M::treeH,0x3a4366);
            // Detail box: the selected node's name, value and one-line effect, its price and the BUY button.
            rect(M::panelX+14,M::detailY,M::panelW-28,M::detailH,0x10162a);
            const int sel=g.memoryTreeSelected;
            if(sel>=0&&sel<count){
                const auto& d=tree[sel];
                const bool own=g.mastery.has(job,sel),can=g.mastery.canBuy(job,sel),open=g.mastery.unlocked(job,sel);
                text(d.name,M::panelX+26,M::detailY+8,2,0xffffff);
                text(d.tag,M::panelX+26,M::detailY+28,1,0x9fd0ff);
                text(d.desc,M::panelX+26,M::detailY+42,1,0xcfe6ff);
                if(own)text("OWNED",M::panelX+26,M::detailY+62,2,0xa6f0b0);
                else if(!open){text("NEED",M::panelX+26,M::detailY+62,2,0xc0a070);number(d.need-g.mastery.parentsOwned(job,sel),M::panelX+88,M::detailY+62,2,0xc0a070);}
                else{number(d.cost,M::panelX+26,M::detailY+62,2,can?0xa6f0b0:0x7f7a96);text("P",M::panelX+48,M::detailY+62,2,can?0xa6f0b0:0x7f7a96);}
                rect(M::buyX,M::buyY,M::buyW,M::buyH,can?0x2f8f5a:own?0x2a3150:0x1f2438);
                text("BUY",M::buyX+34,M::buyY+10,2,can?0xffffff:0x5f6678);
            }else{
                text("XP",M::panelX+26,M::detailY+18,1,0x9fd0ff);number(g.mastery.xpOf(job),M::panelX+26,M::detailY+30,2,0xffffff);
                text("NEXT",M::panelX+130,M::detailY+18,1,0x9fd0ff);number(g.mastery.xpToNext(job),M::panelX+130,M::detailY+30,2,0xffffff);
                text("POINTS",M::panelX+240,M::detailY+18,1,0x9fd0ff);number(g.mastery.points(job),M::panelX+240,M::detailY+30,2,0xffffff);
            }
        }
        // Tutorial: pulse a frame around the row to eat first (permanent tab only).
        if(!runTab&&story&&story->menuRow>=0&&story->menuRow<statCount){
            const float y=M::rowY+story->menuRow*M::rowH,pulse=.5f+.5f*std::sin(g.clock*6.f);
            const Uint8 a=static_cast<Uint8>(150+100*pulse);
            rect(M::panelX+12,y,M::panelW-24,3,0xf2d48c,a);rect(M::panelX+12,y+M::rowH-5,M::panelW-24,3,0xf2d48c,a);
            rect(M::panelX+12,y,3,M::rowH-2,0xf2d48c,a);rect(M::panelX+M::panelW-15,y,3,M::rowH-2,0xf2d48c,a);
        }
    }
    // Special-effect cues of tree nodes: an expanding ring where a blast or pulse happened (placeholder visuals).
    void drawProcCues(const Combat& g){
        for(const auto& c:g.procCues){
            const float u=std::clamp(c.age/.5f,0.f,1.f);
            const int action=c.action;
            const unsigned color=action==static_cast<int>(ProcAction::Pulse)?0x9fd0ff:0xffc27a;
            const float radius=std::max(18.f,c.radius)*(.25f+.75f*u);
            ringAlpha(c.x-offset,c.y,radius,color,static_cast<Uint8>(230*(1.f-u)),.6f);
            ringAlpha(c.x-offset,c.y,radius*.6f,0xffffff,static_cast<Uint8>(160*(1.f-u)),.6f);
        }
    }
    // Story overlays are glyphs, rings, fades and light only: no sentences, so nothing needs reading.
    void drawStory(const Combat& g,const StoryView& v,const ControlLayout& layout){
        const float t=g.clock;
        const float pulse=.5f+.5f*std::sin(t*6.f);
        // Time stop: the fight is tinted and framed, and a preview shows what the prompted move will do.
        if(v.freeze>0){
            const Uint8 k=static_cast<Uint8>(std::min(1.f,v.freeze)*255);
            rect(0,0,1280,720,0x0a1430,static_cast<Uint8>(k*.38f));
            rect(0,0,1280,5,0x9ed7ff,static_cast<Uint8>(k*.7f));rect(0,715,1280,5,0x9ed7ff,static_cast<Uint8>(k*.7f));
            rect(0,0,5,720,0x9ed7ff,static_cast<Uint8>(k*.7f));rect(1275,0,5,720,0x9ed7ff,static_cast<Uint8>(k*.7f));
            const float px=g.player.x-offset,py=g.player.y-50;
            if(v.previewR>0){
                ringAlpha(px,py,v.previewR,0x9ed7ff,static_cast<Uint8>(150+80*pulse),.45f);
                ringAlpha(px,py,v.previewR*(.3f+.7f*std::fmod(t*1.2f,1.f)),0xe6f2ff,120,.45f);
            }
            if(v.hintX>=0){   // dotted line to the foe the move will reach
                const float hx=v.hintX-offset,steps=std::max(1.f,std::abs(hx-px)/22.f);
                const float flow=std::fmod(t*2.f,1.f);
                for(int i=0;i<static_cast<int>(steps);++i){
                    const float u=(i+flow)/steps,x=px+(hx-px)*u,y=py-std::sin(u*3.14159f)*40.f;
                    rect(x-3,y-3,6,6,0xf2d48c,200);
                }
            }
        }
        if(v.chevrons){
            for(float x=v.chevronX0;x<v.chevronX1;x+=84.f){
                const float ground=g.castleArea>0?CastleTerrain::groundAt(g.castleArea,x):floorY;
                if(!std::isfinite(ground))continue;
                const float travel=std::fmod((x-v.chevronX0)/84.f-t*3.2f,5.f);
                const float lit=std::clamp(1.f-std::fabs(travel<0?travel+5.f:travel)*.45f,.12f,1.f);
                chevron(x-offset,ground-34,4,1,0xe6c8ff,static_cast<Uint8>(40+190*lit));
            }
        }
        if(v.gateGlow>0){
            const float ground=g.castleArea>0?CastleTerrain::groundAt(g.castleArea,v.gateX):floorY;
            const float gy=std::isfinite(ground)?ground-52:560;
            for(int k=0;k<3;++k)ringAlpha(v.gateX-offset,gy,46+k*14+pulse*10,0x78b9ff,static_cast<Uint8>(210-k*55),1.35f);
        }
        // Infinite regression: one tally mark per loop, and the single memory that survives the last one.
        if(v.loopPips>0){
            const float startX=640.f-(v.loopPips*16.f-6.f)*.5f;
            for(int k=0;k<v.loopPips;++k){
                const bool newest=k==v.loopPips-1;
                rect(startX+k*16.f-2,34,12,16,0xe6dcff,newest?90:30);
                rect(startX+k*16.f,36,8,12,newest?0xffffff:0xbfa8ff,newest?255:190);
            }
        }
        if(v.memoryGlint>0){
            const float px=g.player.x-offset,py=g.player.y-62.f-v.memoryGlint*14.f;
            const Uint8 a=static_cast<Uint8>(60+190*v.memoryGlint);
            ringAlpha(px,py,14+pulse*6+v.memoryGlint*14,0xbfa8ff,static_cast<Uint8>(a*.6f));
            rect(px-9,py-9,18,18,0xbfa8ff,static_cast<Uint8>(a*.5f));rect(px-5,py-5,10,10,0xffffff,a);
        }
        // Memory tutorial: the altar, then the foe whose memory is next.
        if(v.altarGlow>0){
            const float ax=memoryAltarX-offset;
            for(int k=0;k<3;++k)ringAlpha(ax,floorY-4,40+k*16+pulse*10,0xe6dcff,static_cast<Uint8>(210-k*55),.28f);
        }
        if(v.hintX>=0){
            const float hx=v.hintX-offset,hy=floorY-100+std::sin(t*3.f)*6.f;
            rect(hx-9,hy-9,18,18,0xbfa8ff,110);rect(hx-4,hy-4,8,8,0xffffff,235);
            ringAlpha(hx,hy,18+pulse*6,0xbfa8ff,190);
        }
        // Control practice goals: a beacon to walk to, targets to hit, orbs to collect, a ghost run to watch.
        if(v.goalX>=0){
            const float ground=g.castleArea>0?CastleTerrain::groundAt(g.castleArea,v.goalX):floorY;
            const float gy=std::isfinite(ground)?ground:floorY,gx=v.goalX-offset;
            rect(gx-3,gy-190,6,190,0xf2d48c,static_cast<Uint8>(70+60*pulse));
            rect(gx-1,gy-190,2,190,0xffffff,static_cast<Uint8>(120+100*pulse));
            for(int k=0;k<3;++k)ringAlpha(gx,gy-4,26+k*14+pulse*8,0xf2d48c,static_cast<Uint8>(230-k*60),.28f);
        }
        if(v.targetsHint){
            for(const auto& e:g.enemies){
                if(!e.dummy)continue;
                const float hx=e.x-offset,hy=e.y-150+std::sin(t*3.f+e.x)*6.f;
                rect(hx-9,hy-9,18,18,0xf2d48c,110);rect(hx-4,hy-4,8,8,0xffffff,235);
                ringAlpha(hx,hy,18+pulse*6,0xf2d48c,190);
            }
        }
        if(v.goalCount>0){
            const float w=v.goalCount*34.f,x0=(1280.f-w)*.5f;
            for(int i=0;i<v.goalCount;++i){
                const bool got=i<v.goalDone;
                rect(x0+i*34,118,26,26,0x080d17,200);
                rect(x0+i*34+4,122,18,18,got?0xfff0b0:0x354354,255);
                if(got)ringAlpha(x0+i*34+13,131,18+pulse*4,0xfff0b0,160);
            }
        }
        if(v.highlight>=0&&v.highlight<ControlLayout::Count){
            const auto& b=layout.buttons[v.highlight];
            const float cx=b.x+b.size*.5f,cy=b.y+b.size*.5f;
            ringAlpha(cx,cy,b.size*.5f+8+pulse*8,0xf2d48c,230);
            ringAlpha(cx,cy,b.size*.5f+22+pulse*10,0xf2d48c,110);
            const float tap=std::fmod(t*1.3f,1.f);
            ringAlpha(cx,cy,b.size*.18f+b.size*.32f*tap,0xffffff,static_cast<Uint8>(200*(1.f-tap)));
            if(v.highlight==ControlLayout::Stick&&v.arrow){
                const float slide=std::fmod(t*60.f,28.f);
                if(v.arrow==1)chevron(cx+b.size*.5f+10+slide,cy-14,4,1,0xf2d48c,static_cast<Uint8>(255-slide*7));
                else if(v.arrow==-1)chevron(cx-b.size*.5f-38-slide,cy-14,4,-1,0xf2d48c,static_cast<Uint8>(255-slide*7));
                else chevron(cx-14,cy-b.size*.5f-34-slide,4,2,0xf2d48c,static_cast<Uint8>(255-slide*7));
            }
        }
        if(v.pipsTotal>0){
            const float w=v.pipsTotal*16.f,x0=(1280.f-w)*.5f;
            for(int i=0;i<v.pipsTotal;++i){
                const bool done=i<v.pips;
                rect(x0+i*16,92,12,12,0x080d17,200);
                rect(x0+i*16+2,94,8,8,done?0xf2d48c:i==v.pips?0x9ed7ff:0x354354,255);
            }
        }
        if(v.skipVisible){
            rect(StoryDirector::skipX,StoryDirector::skipY,StoryDirector::skipW,StoryDirector::skipH,0x121b29,210);
            rect(StoryDirector::skipX+2,StoryDirector::skipY+2,StoryDirector::skipW-4,StoryDirector::skipH-4,0x28384a,210);
            chevron(StoryDirector::skipX+16,StoryDirector::skipY+11,3,1,0xe3bd72,255);
            chevron(StoryDirector::skipX+44,StoryDirector::skipY+11,3,1,0xe3bd72,255);
        }
        if(v.unlockBurst>0){
            const auto& b=layout.buttons[ControlLayout::Skill];
            const float k=1.f-v.unlockBurst/2.4f;
            ringAlpha(b.x+b.size*.5f,b.y+b.size*.5f,b.size*.4f+k*140.f,0xfff0b0,static_cast<Uint8>(255*(1.f-k)));
        }
        if(v.desaturate>0)rect(0,0,1280,720,0x2a2a3c,static_cast<Uint8>(std::min(1.f,v.desaturate)*170));
        if(v.rewind){
            for(int i=0;i<16;++i){
                const float y=std::fmod(i*47.f+t*130.f,720.f);
                const float x=1280.f-std::fmod(t*980.f+i*233.f,1700.f);
                rect(x,y,360,3,0xaab8ff,80);rect(x+90,y+6,190,2,0xe8eeff,60);
            }
            const float flashK=std::pow(std::clamp(1.f-v.rewindPhase,0.f,1.f),3.f);
            rect(0,0,1280,720,0xbfd0ff,static_cast<Uint8>(flashK*190));
        }
        if(v.flash>0)rect(0,0,1280,720,0xffffff,static_cast<Uint8>(std::min(1.f,v.flash)*230));
        if(v.blackout>0)rect(0,0,1280,720,0x000000,static_cast<Uint8>(std::min(1.f,v.blackout)*255));
        if(v.whiteout>0)rect(0,0,1280,720,0xffffff,static_cast<Uint8>(std::min(1.f,v.whiteout)*255));
    }
    void hud(const Combat& g,Actions held,const ControlLayout& layout,std::array<float,2> stick,bool interactionAvailable){
        const auto& p=g.player;
        playerHealthBar(p);
        for(int i=0;i<4;++i){rect(765+i*74,21,66,61,0x080d17);rect(771+i*74,27,54,49,0x667482);rect(777+i*74,33,42,37,0x222c3d);}
        text("GEAR",780,44,2);text(p.flashExecutioner?"FLASH EXECUTIONER":p.form==0?"BLADE":"SPACE RULER",28,73,2,0x9ed7ff);number(g.foes(),1180,43,5,0xece7d2);text("FOES",1090,25,2);
        const auto& c=layout.buttons;
        const auto& js=c[ControlLayout::Stick];
        const bool stickActive=held.left||held.right||held.jump;
        icon(stickActive?7:6,js.x,js.y,js.size,stickActive);
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
        const auto& attack=c[ControlLayout::Attack];
        icon(held.attack?1:0,attack.x,attack.y,attack.size,held.attack);
        if(interactionAvailable){
            rect(attack.x+attack.size*.34f,attack.y+attack.size*.22f,attack.size*.32f,attack.size*.48f,0x10151b,245);
            text("!",attack.x+attack.size*.425f,attack.y+attack.size*.20f,5,0xd2d6d3);
        }
        icon(held.dash?3:2,c[ControlLayout::Dash].x,c[ControlLayout::Dash].y,c[ControlLayout::Dash].size,held.dash);
        // Character swap remains available in every class, including Flash Executioner.
        icon(held.swap?5:4,c[ControlLayout::Swap].x,c[ControlLayout::Swap].y,c[ControlLayout::Swap].size,held.swap);
        if(p.flashExecutioner){
            flashSkillSlot(c[ControlLayout::Skill],1,0,p.skillCooldown,3.f,held.skill,0xb4d8ee);
            flashSkillSlot(c[ControlLayout::Sweep],2,1,p.sweepReady,1.f,held.skillSweep,0x9bb9d6);
            flashSkillSlot(c[ControlLayout::Buff],3,2,p.buffReady,1.5f,held.skillBuff,0xd9ecf5);
            flashSkillSlot(c[ControlLayout::Ultimate],4,3,p.verdictCooldown,9.f,held.ultimate,0xc4d7ed);
        }else if(p.form==1){
            skillButton(c[ControlLayout::Skill],1,4,p.mageSkillCooldown,7,held.skill,0x91c5e2);
            skillButton(c[ControlLayout::Sweep],2,5,p.sweepReady,7,held.skillSweep,0x91c5e2);
            skillButton(c[ControlLayout::Buff],3,6,p.buffReady,5,held.skillBuff,0x91c5e2);
            skillButton(c[ControlLayout::Ultimate],4,7,p.verdictCooldown,20,held.ultimate,0xb69bdf);
            const auto& stick=c[ControlLayout::Stick];
            rect(stick.x,stick.y-10,stick.size,5,0x26163a);
            rect(stick.x,stick.y-10,stick.size*p.spaceFuel,5,0xb36bf0);
        }else{
            skillSlot(c[ControlLayout::Skill],1,0,p.mageSkillCooldown,4.1f,
                held.skill,0xb45d68,true);
            skillSlot(c[ControlLayout::Sweep],2,1,p.sweepReady,sweepCooldown,
                held.skillSweep,0xb45d68,false);
            skillSlot(c[ControlLayout::Buff],3,2,p.buffReady,buffCooldown,
                held.skillBuff,0xb45d68,false);
            if(p.swapCooldown>0)number(static_cast<int>(std::ceil(p.swapCooldown)),c[ControlLayout::Swap].x+20,c[ControlLayout::Swap].y+25,3,0xffd277);
        }
        if(g.inventory){rect(46,135,460,340,0xe3bd72);rect(50,139,452,332,0x1c2a34);text("EQUIPMENT",66,153,3);
            for(int i=0;i<3;++i){text(p.gear.name(i),66,200+i*42,2);rect(66+i*143,413,132,45,0x304039);text(i==0?"WEAPON":i==1?"ARMOR":"CHARM",75+i*143,428,2);}
            text("ATK",66,340,2);number(p.gear.attack(),120,340,2);text("DEF",200,340,2);number(p.gear.defense(),250,340,2);text("MAX HP",66,373,2);number(p.maxHealth(),160,373,2);
        }
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
    ~Presentation(){if(playerHealthFrame_)SDL_DestroyTexture(playerHealthFrame_);if(circularSkillButtons_)SDL_DestroyTexture(circularSkillButtons_);for(auto* image:spaceActorTextures_)if(image)SDL_DestroyTexture(image);for(auto* image:spaceFxTextures_)if(image)SDL_DestroyTexture(image);for(auto* image:flashActorTextures_)if(image)SDL_DestroyTexture(image);for(auto* image:flashFxTextures_)if(image)SDL_DestroyTexture(image);for(auto* image:castleEnemyMoveTextures_)if(image)SDL_DestroyTexture(image);for(auto* image:castleEnemyAttackTextures_)if(image)SDL_DestroyTexture(image);for(auto* image:castleEnemyAttackVfxTextures_)if(image)SDL_DestroyTexture(image);for(auto& form:bossMotionTextures_)for(auto* image:form)if(image)SDL_DestroyTexture(image);for(auto* image:bossFxTextures_)if(image)SDL_DestroyTexture(image);for(auto* fx:{goldLance_,goldTrail_,goldOrb_,goldImpact_,riftSlash_,riftTrace_})if(fx)SDL_DestroyTexture(fx);if(titleTexture_)SDL_DestroyTexture(titleTexture_);if(titleMenu_)SDL_DestroyTexture(titleMenu_);if(enemyBarFrame_)SDL_DestroyTexture(enemyBarFrame_);if(bossBarFrame_)SDL_DestroyTexture(bossBarFrame_);if(executeIcon_)SDL_DestroyTexture(executeIcon_);if(settingsGear_)SDL_DestroyTexture(settingsGear_);if(settingsPanel_)SDL_DestroyTexture(settingsPanel_);if(settingsWindow_)SDL_DestroyTexture(settingsWindow_);for(auto* texture:textures)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestTextures_)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestLayerTextures_)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestV3Textures_)if(texture)SDL_DestroyTexture(texture);}
    void setForestSettings(ForestSettings settings) {forestGenerator_.regenerate(settings);}
    void setDemonCastleEntrance(bool enabled) {castleEntranceActive_=enabled && castleEntrance_.ready();}
    int castlePortalSide(float x) const {return castleEntranceActive_?castleEntrance_.portalSide(x):-1;}
    bool setDemonCastleArea(int area) {return castleEntrance_.setArea(area);}
    bool demonCastleEntranceReady() const {return castleEntrance_.ready();}
    const ForestGenerator& forestGenerator() const {return forestGenerator_;}
    static std::string asset(const char* name){
#ifdef SDL_PLATFORM_ANDROID
        // Match the Studio importer's generic asset namespace.
        const std::string logical(name);
        const auto slash=logical.find('/');
        const std::string root=logical.substr(0,slash);
        const bool direct=(root=="ui"||root=="title"||root=="monsters"||
           root=="forest"||root=="forest_v2"||root=="forest_v3"||
           root=="forest_entrance"||logical=="knight_motion.png");
        const std::string packed=direct?logical:"ui/__studio_all__/"+logical;
        // Studio extracts ZIP entries under Android internal storage/game_assets.
        // Relative paths are resolved against the process working directory,
        // which is not the Studio import directory.
        const char* storage=SDL_GetAndroidInternalStoragePath();
        if(storage&&*storage)return std::string(storage)+"/game_assets/"+packed;
        return packed;
#else
        const char* base=SDL_GetBasePath();return std::string(base?base:"")+"assets/"+name;
#endif
    }
    bool loadTitle(){
        // Canonical game title assets use the title/ namespace.
        titleTexture_=IMG_LoadTexture(renderer,asset("title/eclipse.png").c_str());
        if(!titleTexture_){SDL_SetError("ECLIPSE ASSET ERROR: title/eclipse.png missing or invalid");return false;}
        // Use the self-contained title PNG; removed cape/knight overlay paths
        // belonged to a discarded multi-layer title pack.
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
    static constexpr float titleReadyTime=5.8f;


    bool loadCastleEnemyAtlases(){
        for(int i=0;i<castleEnemyCount;++i){
            const auto& spec=castleEnemySpecs[i];
            const std::string base=std::string("monsters/demon_castle/")+spec.folder+"/";
            for(int state=0;state<2;++state){
                auto*& texture=state==0?castleEnemyMoveTextures_[i]:castleEnemyAttackTextures_[i];
                const char* sheet=state==0?"이동_sheet.png":"공격_sheet.png";
                const int cellSize=enemyCell(AshInfantry+i);
                const int width=state==0?castleEnemyMoveFrames*cellSize:castleEnemyAttackFrames*cellSize;
                const std::string file=asset((base+sheet).c_str());
                texture=IMG_LoadTexture(renderer,file.c_str());
                if(!texture){SDL_SetError("ECLIPSE ENEMY ASSET ERROR: %s: %s",file.c_str(),SDL_GetError());return false;}
                float actualW=0,actualH=0;
                if(!SDL_GetTextureSize(texture,&actualW,&actualH)||actualW!=width||actualH!=cellSize){
                    SDL_SetError("ECLIPSE ENEMY ASSET ERROR: %s expected %dx%d, got %.0fx%.0f",file.c_str(),width,cellSize,actualW,actualH);
                    SDL_DestroyTexture(texture);texture=nullptr;return false;
                }
                SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);
                SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
            }
            const std::string vfxFile=asset((base+"공격_vfx_layer.png").c_str());
            auto*& vfx=castleEnemyAttackVfxTextures_[i];
            vfx=IMG_LoadTexture(renderer,vfxFile.c_str());
            if(!vfx){SDL_SetError("ECLIPSE ENEMY ASSET ERROR: %s: %s",vfxFile.c_str(),SDL_GetError());return false;}
            float vfxW=0,vfxH=0;
            if(!SDL_GetTextureSize(vfx,&vfxW,&vfxH)||vfxW!=castleEnemyAttackFrames*416||vfxH!=256){
                SDL_SetError("ECLIPSE ENEMY ASSET ERROR: %s expected %dx256, got %.0fx%.0f",
                    vfxFile.c_str(),castleEnemyAttackFrames*416,vfxW,vfxH);
                SDL_DestroyTexture(vfx);vfx=nullptr;return false;
            }
            SDL_SetTextureScaleMode(vfx,SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(vfx,SDL_BLENDMODE_BLEND);
        }
        // Demon King body motions and effect layers. Required: a missing or mis-sized sheet is a loud error.
        for(int form=0;form<2;++form){
            const int kind=form?DemonKingTrue:DemonKingArmored;
            const std::string base=std::string("monsters/demon_castle/")+enemySpec(kind).folder+"/";
            for(int m=1;m<static_cast<int>(BossMotion::Count);++m){
                if(!bossMotionOwned(kind,static_cast<BossMotion>(m)))continue;
                const std::string file=asset((base+"모션_"+bossMotionNames[m]+"_sheet.png").c_str());
                auto*& texture=bossMotionTextures_[form][m];
                texture=IMG_LoadTexture(renderer,file.c_str());
                if(!texture){SDL_SetError("ECLIPSE ENEMY ASSET ERROR: %s: %s",file.c_str(),SDL_GetError());return false;}
                float w=0,h=0;
                if(!SDL_GetTextureSize(texture,&w,&h)||w!=bossMotionFrames*enemyCell(kind)||h!=enemyCell(kind)){
                    SDL_SetError("ECLIPSE ENEMY ASSET ERROR: %s expected %dx%d, got %.0fx%.0f",file.c_str(),
                        bossMotionFrames*enemyCell(kind),enemyCell(kind),w,h);
                    SDL_DestroyTexture(texture);texture=nullptr;return false;
                }
                SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
            }
        }
        for(int i=0;i<static_cast<int>(BossFx::Count);++i){
            const auto& sheet=bossFxSheets[i];
            const std::string file=asset(sheet.file);
            auto*& texture=bossFxTextures_[i];
            texture=IMG_LoadTexture(renderer,file.c_str());
            if(!texture){SDL_SetError("ECLIPSE ASSET ERROR: boss effect %s: %s",file.c_str(),SDL_GetError());return false;}
            float w=0,h=0;const int rows=(sheet.count+sheet.columns-1)/sheet.columns;
            if(!SDL_GetTextureSize(texture,&w,&h)||w!=sheet.columns*sheet.cellW||h!=rows*sheet.cellH){
                SDL_SetError("ECLIPSE ASSET ERROR: boss effect %s expected %dx%d, got %.0fx%.0f",file.c_str(),
                    sheet.columns*sheet.cellW,rows*sheet.cellH,w,h);
                SDL_DestroyTexture(texture);texture=nullptr;return false;
            }
            SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
        }
        return true;
    }

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
        const float frameRate=playbackDuration>0?sheet.fps*sheet.duration/duration:sheet.fps;
        const int frame=std::clamp(static_cast<int>(t*frameRate),0,sheet.count-1);
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
    void updateReviewedFlashEffects(const Combat& g){
        if(g.clock==reviewedFlashClock_)return;
        const auto& p=g.player;
        if(g.clock<reviewedFlashClock_){
            reviewedFlashEvents_.clear();reviewedFlashBurstSerial_=0;
            reviewedFlashMove_=FlashMove::None;reviewedFlashMoveAge_=0;
            reviewedFlashDash_=reviewedFlashSwap_=false;
            reviewedFlashGrounded_=p.grounded;
        }
        reviewedFlashClock_=g.clock;
        reviewedFlashEvents_.erase(std::remove_if(reviewedFlashEvents_.begin(),reviewedFlashEvents_.end(),
            [&](const ReviewedFlashEvent& event){const auto& sheet=flashEffects[static_cast<int>(event.role)];
                const float lifetime=event.playbackDuration>0?event.playbackDuration:sheet.duration;
                return g.clock-event.born>=lifetime;}),
            reviewedFlashEvents_.end());
        auto spawn=[&](FlashFx role,float x,float y,int facing,bool follow,float delay=0.f,float playbackDuration=0.f){
            if(hasReviewedFlashFx(role)){
                const auto& sheet=flashEffects[static_cast<int>(role)];
                reviewedFlashEvents_.push_back({role,x,y,g.clock-delay-sheet.spawnAgeSeconds,facing,follow,playbackDuration});
            }
        };
        const bool newMove=p.flashMove!=reviewedFlashMove_||p.flashAge<reviewedFlashMoveAge_;
        const float before=newMove?0.f:reviewedFlashMoveAge_;
        if(p.flashExecutioner&&p.flashMove!=FlashMove::None){
            FlashFx role=FlashFx::Slash;float trigger=0;
            switch(p.flashMove){
                case FlashMove::Slash:role=FlashFx::Slash;trigger=.184f;break;
                case FlashMove::Rush:role=FlashFx::Rush;trigger=.125f;break;
                case FlashMove::Command:role=FlashFx::Command;trigger=.23f;break;
                case FlashMove::Blink:role=FlashFx::BlinkOut;trigger=0;break;
                case FlashMove::Verdict:role=FlashFx::Verdict;trigger=.30f;break;
                default:break;
            }
            const float speed=p.flashMove==FlashMove::Slash?std::max(.001f,p.flashSlashSpeed):1.f;
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
            if((newMove&&trigger==0)||(before<trigger&&p.flashAge>=trigger)){
                const float sourceAge=std::max(0.f,p.flashAge-trigger)/speed+skippedPrelude+
                    (p.flashMove==FlashMove::Verdict?.20f:0.f);
                float playbackDuration=0.f;
                if(p.flashMove==FlashMove::Slash){
                    // The light-crescent slash is a long effect (1.1 s) that keeps playing after the attack
                    // clip ends, so it is no longer stretched to the clip's remaining time. It plays at its
                    // authored speed at attack speed 1.0 and speeds up with attack-speed boosts, always
                    // playing all of its frames before it disappears.
                    playbackDuration=fxSheet.duration/speed;
                }
                spawn(role,p.x,p.y,p.facing,p.flashMove!=FlashMove::Blink,sourceAge,playbackDuration);
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
                // Stored light lances (Command): two crossing blades of light, medium power.
                if(!verdictPair)spawn(FlashFx::ImpactLance,burst.x,
                    reviewedFlashImpactY(g,FlashFx::ImpactLance,burst),burst.facing,false,age);
            }else if(burst.kind==LightFx::SlashHit){
                // Basic attack: the weakest hit, a small round flash.
                spawn(FlashFx::Impact,burst.x,reviewedFlashImpactY(g,FlashFx::Impact,burst),burst.facing,false,age);
            }else if(burst.kind==LightFx::RushHit){
                // Rush: a heavy eight-point star with two shock rings, the strongest regular hit.
                spawn(FlashFx::ImpactRush,burst.x,reviewedFlashImpactY(g,FlashFx::ImpactRush,burst),burst.facing,false,age);
            }else if(burst.kind==LightFx::Verdict){
                spawn(FlashFx::LanceHit,burst.x,reviewedFlashImpactY(g,FlashFx::LanceHit,burst),
                      burst.facing,false,age);
            }else if(burst.kind==LightFx::Arrive&&p.flashMove==FlashMove::Blink)
                spawn(FlashFx::BlinkIn,burst.x,burst.y+48.f,burst.facing,false,age);
        }
        const bool dash=p.flashExecutioner&&p.dashLeft>0;
        const bool swap=p.flashExecutioner&&p.swapFlash>0;
        if(dash&&!reviewedFlashDash_)spawn(FlashFx::Dash,p.x,p.y,p.facing,true,.15f-p.dashLeft);
        if(swap&&!reviewedFlashSwap_)spawn(FlashFx::Swap,p.x,p.y,p.facing,true,.24f-p.swapFlash);
        const bool jumpStart=reviewedFlashGrounded_&&!p.grounded;
        const bool landed=!reviewedFlashGrounded_&&p.grounded;
        if(p.flashExecutioner&&jumpStart)spawn(FlashFx::Jump,p.x,p.y,p.facing,false);
        if(p.flashExecutioner&&landed)spawn(FlashFx::Land,p.x,p.y,p.facing,false);
        reviewedFlashDash_=dash;reviewedFlashSwap_=swap;
        reviewedFlashGrounded_=p.grounded;
        reviewedFlashMove_=p.flashMove;reviewedFlashMoveAge_=p.flashAge;
    }
    void drawReviewedFlashEffects(const Combat& g,const Animation&,bool front){
        const auto& p=g.player;
        for(const auto& event:reviewedFlashEvents_){
            reviewedFlashFx(event.role,g.clock-event.born,event.follow?p.x:event.x,event.follow?p.y:event.y,
                            event.follow?p.facing:event.facing,front,0,event.playbackDuration);
        }
        if(p.flashExecutioner&&hasReviewedFlashFx(FlashFx::Orb)){
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
        if(!p.flashExecutioner||p.health<=0)return false;
        FlashActor role=FlashActor::Idle;
        float phase=0,age=0,duration=1,contact=-1;
        bool phaseDriven=false;
        if(p.flashMove!=FlashMove::None){
            constexpr FlashActor moves[]{FlashActor::Idle,FlashActor::Basic,FlashActor::Skill1,
                FlashActor::Skill2,FlashActor::Skill3,FlashActor::Ultimate};
            role=moves[static_cast<int>(p.flashMove)];
            const auto clip=flashClips[static_cast<int>(p.flashMove)];
            age=p.flashAge;duration=clip.duration;contact=clip.contact;
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
        if(!texture)return false; // loadFlashExecutioner already reports required load failure.
        const int frame=phaseDriven?std::clamp(static_cast<int>(phase*sheet.count),0,sheet.count-1):
            reviewedFlashFrame(sheet,age,duration,contact);
        if(p.hurtTint>0)SDL_SetTextureColorMod(texture,255,150,160);
        reviewedFlashSprite(sheet,texture,frame,p.x,p.y,p.facing);
        SDL_SetTextureColorMod(texture,255,255,255);
        return true;
    }

    bool loadFlashExecutioner(){
        // The reviewed actor/effect sheets are the active combat assets. The
        // former white_night_motion/lance and ui/*_gold PNGs were removed when
        // the reviewed sheets replaced the legacy pack, so never gate combat
        // entry on those obsolete paths.
        return loadReviewedFlashSheets();
    }

    static constexpr std::array<SDL_FRect,3> qaCharacterButtons{{
        {410,374,460,52},{410,436,460,52},{530,562,220,44}
    }};

    void drawTitleOpening(float seconds){
        const auto progress=[seconds](float begin,float end){
            const float t=std::clamp((seconds-begin)/(end-begin),0.f,1.f);
            return t*t*(3.f-2.f*t);
        };
        const float occultation=progress(.52f,2.65f);
        const float pullback=progress(2.92f,4.85f);
        const float reveal=progress(4.12f,titleReadyTime);
        // End on the painted corona at the center of the ECLIPSE wordmark.
        const float x=640, y=338-252*pullback, radius=107-40*pullback;
        for(int i=0;i<62;++i){
            const float starX=(i*509+37)%1280, starY=(i*i*11+i*41+13)%720;
            const float glimmer=.74f+.26f*std::sin(i+seconds*1.5f);
            rect(starX,starY,i%8==0?3:2,2,0xc6beda,
                 static_cast<Uint8>((65+100*occultation)*glimmer));
        }

        // Horizontal pixel spans keep the limb stepped at native game resolution.
        const auto disc=[&](float cx,float cy,float r,unsigned tint,Uint8 alpha){
            for(int row=-static_cast<int>(r);row<=r;row+=2){
                const float half=std::sqrt(std::max(0.f,r*r-row*row));
                rect(cx-half,cy+row,2*half,2,tint,alpha);
            }
        };
        disc(x,y,radius+16,0x8873b6,22);
        disc(x,y,radius+8,0xae9ad9,55);
        disc(x,y,radius+3,0xe6dcff,200);
        disc(x,y,radius,0xffefd4,255);
        disc(x-2.3f*radius*(1-occultation),y-7*(1-occultation),
             radius-2,0x08080e,255);

        // The last bead of sunlight flares on the rim, then contracts to a point.
        const float bead=progress(2.24f,2.55f)*(1-progress(2.62f,3.15f));
        if(bead>0){
            const float tip=x+radius;
            rect(tip-37*bead,y,74*bead,2,0xffeac2);
            rect(tip,y-15*bead,2,30*bead,0xfff6dc);
        }
        if(seconds<.55f)
            rect(0,0,1280,720,0x08080e,static_cast<Uint8>(255*(1-progress(0,.55f))));
        if(reveal>0){
            const SDL_FRect screen{0,0,1280,720};
            SDL_SetTextureAlphaMod(titleTexture_,static_cast<Uint8>(255*reveal));
            SDL_RenderTexture(renderer,titleTexture_,nullptr,&screen);
            SDL_SetTextureAlphaMod(titleTexture_,255);
        }
    }

    void drawTitle(float seconds,int page=0,bool loggingIn=false,int qaFirstJob=-1){
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer,8,8,14,255);
        SDL_RenderClear(renderer);
        if(seconds<titleReadyTime){
            drawTitleOpening(seconds);
            SDL_RenderPresent(renderer);
            return;
        }
        const SDL_FRect screen{0,0,1280,720};
        SDL_RenderTexture(renderer,titleTexture_,nullptr,&screen);
        if(page==2){
            rect(386,320,508,304,0x101223);
            rect(386,320,508,3,0xad95c7);
            text(qaFirstJob<0?"QA: PICK FIRST JOB":"QA: PICK SECOND JOB",453,340,3,0xffe2b7);
            constexpr const char* labels[]={"1 SPACE RULER","2 FLASH EXECUTIONER","BACK"};
            for(size_t i=0;i<qaCharacterButtons.size();++i){
                const auto& target=qaCharacterButtons[i];
                const SDL_FRect source{0,0,112,30};
                SDL_RenderTexture(renderer,titleMenu_,&source,&target);
                const bool chosen=i<2&&qaFirstJob==static_cast<int>(i)+1;
                text(labels[i],target.x+(target.w-std::strlen(labels[i])*18.f)*.5f,
                    target.y+(target.h-21)*.5f,3,chosen?0xffd17c:0xece3f4);
                if(chosen)rect(target.x+7,target.y+10,5,target.h-20,0xffd17c);
            }
        }else{
            // Screens: MAIN=0, START_OPTIONS=1, QA=2, GEAR=3, SETTINGS=4.
            constexpr const char* labels[]={"PLAY","VACODE LOGIN","PLAY AS GUEST","SETTINGS","EXIT","BACK"};
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
        if(page!=2&&settingsGear_)
            SDL_RenderTexture(renderer,settingsGear_,nullptr,&titleGear);
        SDL_RenderPresent(renderer);
    }

    bool loadSkillButtonArt(){
        auto load=[&](const char* name,SDL_Texture*& target,int width,int height)->bool{
            const std::string path=asset(name);SDL_Surface* surface=IMG_Load(path.c_str());
            if(!surface){const std::string cause=SDL_GetError();SDL_SetError("ECLIPSE ASSET ERROR: cannot load %s: %s",path.c_str(),cause.c_str());SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",SDL_GetError());return false;}
            if(surface->w!=width||surface->h!=height){const int w=surface->w,h=surface->h;SDL_DestroySurface(surface);SDL_SetError("ECLIPSE ASSET ERROR: %s expected %dx%d, got %dx%d",path.c_str(),width,height,w,h);SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",SDL_GetError());return false;}
            target=SDL_CreateTextureFromSurface(renderer,surface);SDL_DestroySurface(surface);
            if(!target){const std::string cause=SDL_GetError();SDL_SetError("ECLIPSE ASSET ERROR: cannot create texture for %s: %s",path.c_str(),cause.c_str());SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",SDL_GetError());return false;}
            SDL_SetTextureScaleMode(target,SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(target,SDL_BLENDMODE_BLEND);return true;
        };
        if(!load("ui/player_health_frame.png",playerHealthFrame_,288,46))return false;
        if(!load("ui/enemy_bar_frame.png",enemyBarFrame_,88,18))return false;
        if(!load("ui/boss_bar_frame.png",bossBarFrame_,108,18))return false;
        if(!load("ui/circular_skill_buttons.png",circularSkillButtons_,1024,768))return false;
        return true;
    }

    bool load(){
        if(!loadSpaceRuler())return false;
        if(!loadCastleEnemyAtlases())return false;
        // Preload the castle for its future tutorial scene, but do not require
        // tutorial art to start an unrelated forest gameplay scene.
        // The explicit castle preview request checks readiness and raises an error.
        if(!castleEntrance_.load(renderer,[&](const char* name){return asset(name);})) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "ECLIPSE castle asset preload failed (castle scene unavailable): %s",SDL_GetError());
        } else SDL_Log("ECLIPSE demon castle entrance assets: ready");
        // Eight circular control states from the PixelLab-edited 3x3 frame sheet.
        const std::string controlsPath=asset("ui/circular_control_states.png");
        SDL_Surface* controlsSurface=IMG_Load(controlsPath.c_str());
        if(!controlsSurface){const std::string cause=SDL_GetError();SDL_SetError("ECLIPSE ASSET ERROR: cannot load %s: %s",controlsPath.c_str(),cause.c_str());SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",SDL_GetError());return false;}
        if(controlsSurface->w!=768||controlsSurface->h!=768){
            const int w=controlsSurface->w,h=controlsSurface->h;SDL_DestroySurface(controlsSurface);
            SDL_SetError("ECLIPSE ASSET ERROR: %s expected 768x768, got %dx%d",controlsPath.c_str(),w,h);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",SDL_GetError());return false;
        }
        textures[1]=SDL_CreateTextureFromSurface(renderer,controlsSurface);SDL_DestroySurface(controlsSurface);
        if(!textures[1]){const std::string cause=SDL_GetError();SDL_SetError("ECLIPSE ASSET ERROR: cannot create texture for %s: %s",controlsPath.c_str(),cause.c_str());SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",SDL_GetError());return false;}
        SDL_SetTextureScaleMode(textures[1],SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(textures[1],SDL_BLENDMODE_BLEND);
        if(!loadSkillButtonArt())return false;
        // Optional UI decorations never block gameplay startup.
        auto loadOptionalTexture=[&](const char* name,int width=0,int height=0)->SDL_Texture* {
            const auto path=asset(name);SDL_Surface* image=IMG_Load(path.c_str());
            if(!image)return nullptr;
            if(width&&(image->w!=width||image->h!=height)){SDL_DestroySurface(image);return nullptr;}
            SDL_Texture* texture=SDL_CreateTextureFromSurface(renderer,image);
            SDL_DestroySurface(image);
            if(texture){SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);}
            return texture;
        };
        // Legacy Knight, Mage and superseded movement/FX atlases are not
        // loaded here; current approved Flash Executioner sheets are loaded by
        // loadFlashExecutioner(), and the remaining renderer uses procedural fallbacks.
        if(!settingsGear_)settingsGear_=loadOptionalTexture("ui/settings_gear.png");
        settingsPanel_=loadOptionalTexture("ui/settings_menu_panel.png");
        settingsWindow_=loadOptionalTexture("ui/settings_window.png");
        executeIcon_=loadOptionalTexture("ui/execute_skull.png");
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
    void draw(const Combat& g,const Animation& a,float camera,Actions held,const char* capture=nullptr,const char* reviewLabel=nullptr,const ControlLayout* customLayout=nullptr,std::array<float,2> stick={},bool interactionAvailable=false,const StoryView* story=nullptr){
        offset=std::round(camera)-640;
        {const float shakeAmp=(story?story->shake*14.f:0.f)+g.shake*12.f;if(shakeAmp>0)offset+=std::round(std::sin(g.clock*95.f)*shakeAmp);}SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer,21,35,52,255);SDL_RenderClear(renderer);
        forest(g,camera);drawMemoryAltar(g);drawBossFocus(g);drawBossLayers(g,true);
        for(const auto& e:g.enemies)if(!e.dead())enemy(e,g.enemyVisualClock);
        drawBossLayers(g,false);drawSwordTrail(g);
        const auto& caster=g.player;
        updateReviewedFlashEffects(g);
        const bool verdictFreeze=caster.flashExecutioner&&caster.flashMove==FlashMove::Verdict&&
            caster.flashAge>=.12f&&caster.flashAge<.66f;
        if(verdictFreeze){
            // Desaturating veil; targets and their animations are frozen in the combat step.
            rect(0,0,1280,720,0x18140f,154);
        }
        // Authored PNG spatial cuts remain for a few frames; the incision
        // contracts rather than exploding into a diffuse magical glow.
        if(caster.flashExecutioner){
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
        if(caster.flashExecutioner&&goldOrb_&&goldImpact_){
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
        for(const auto& fx:g.eclipseFx){   // light x dark reactions (code-drawn placeholder, no authored art yet)
            const float x=fx.x-offset,y=fx.y;
            if(fx.kind==0){
                const float grow=std::min(1.f,fx.age/eclipsePullStart);
                const float r=(14.f+3.f*fx.marks)*(fx.collapsed?std::max(0.f,1.f-(fx.age-eclipseCollapseAt)*3.f):grow);
                if(!fx.collapsed){
                    ringAlpha(x,y,eclipsePullRadius*(1.f-std::min(1.f,fx.age/eclipseCollapseAt)),0x9b6bff,110,.5f);
                    for(int k=0;k<fx.marks+2;++k){   // the absorbed light streams in
                        const float u=std::fmod(fx.age*2.2f+k*.23f,1.f),a=k*2.4f;
                        rect(x+std::cos(a)*eclipsePullRadius*(1.f-u)-3,y+std::sin(a)*70.f*(1.f-u)-3,6,6,0xfff0b0,static_cast<Uint8>(230*(1.f-u*.5f)));
                    }
                }
                ringAlpha(x,y,r+6,0xffd77a,230,.9f);ringAlpha(x,y,r+3,0xffd77a,160,.9f);
                for(int d=-static_cast<int>(r);d<=static_cast<int>(r);d+=2){
                    const float h=std::sqrt(std::max(0.f,r*r-float(d*d)));rect(x+d,y-h,2,h*2,0x04020a,245);
                }
                if(fx.collapsed){
                    const float k=std::min(1.f,(fx.age-eclipseCollapseAt)/.3f);
                    ringAlpha(x,y,eclipseBlastRadius*k,0xfff0b0,static_cast<Uint8>(255*(1.f-k)),.7f);
                    ringAlpha(x,y,eclipseBlastRadius*k*.7f,0xffffff,static_cast<Uint8>(200*(1.f-k)),.7f);
                }
            }else{
                const float k=std::min(1.f,fx.age/.35f);
                ringAlpha(x,y,30+120.f*k,0xfff0b0,static_cast<Uint8>(255*(1.f-k)),.8f);
                ringAlpha(x,y,16+80.f*k,0xc9a8ff,static_cast<Uint8>(230*(1.f-k)),.8f);
                for(int i=0;i<fx.dark*2;++i){const float a=i*(6.2832f/(fx.dark*2));rect(x+std::cos(a)*(20+90*k)-3,y+std::sin(a)*(14+60*k)-3,6,6,0xfff6bf,static_cast<Uint8>(255*(1.f-k)));}
            }
        }
        for(const auto& burst:g.lightBursts){
            // The basic sword slash is now driven by the *current sword pose*
            // below, not by a detached world-space burst with its own clock.
            const bool replaced=(burst.kind==LightFx::Impact&&hasReviewedFlashFx(FlashFx::ImpactLance))||
                (burst.kind==LightFx::SlashHit&&hasReviewedFlashFx(FlashFx::Impact))||
                (burst.kind==LightFx::RushHit&&hasReviewedFlashFx(FlashFx::ImpactRush))||
                (burst.kind==LightFx::Verdict&&hasReviewedFlashFx(FlashFx::LanceHit))||
                (burst.kind==LightFx::Depart&&hasReviewedFlashFx(FlashFx::BlinkOut))||
                (burst.kind==LightFx::Arrive&&hasReviewedFlashFx(FlashFx::BlinkIn))||
                (burst.kind==LightFx::Rush&&hasReviewedFlashFx(FlashFx::Rush))||
                (burst.kind==LightFx::Command&&hasReviewedFlashFx(FlashFx::Command));
            if(burst.kind!=LightFx::Slash&&!replaced)goldenImpactParticles(burst);
        }
        drawSpaceEffects(g);
        for(const auto& b:g.projectiles){
            if(b.voidBolt){spaceSprite(spaceFxTextures_[0],spaceEffects[0],1.6f-b.life,b.x,b.y,b.facing);continue;}
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
        drawReviewedFlashEffects(g,a,false);
        if(p.form==1&&!p.flashExecutioner)spaceActor(g,a);
        else if(!drawReviewedFlashActor(g,a))mage(p);
        drawReviewedFlashEffects(g,a,true);
        if(p.swapFlash>0&&(!p.flashExecutioner||!hasReviewedFlashFx(FlashFx::Swap)))ring(p.x-offset,p.y-36,44,0xe3bd72);
        static const ControlLayout defaultLayout{};
        const ControlLayout& layout=customLayout?*customLayout:defaultLayout;
        if(layout.showNumbers)for(const auto& popup:g.popups)number(popup.amount,popup.x-offset-12,popup.y,3,popup.kind==DamageKind::Fire?0xff9155:(popup.kind==DamageKind::Bleed||popup.kind==DamageKind::Execute)?0xe76878:0xf2d48c);
        if(g.flash>0)rect(0,0,1280,720,0xe6c8ff,static_cast<Uint8>(std::min(1.f,g.flash)*140));
        drawMemoryMotes(g);drawProcCues(g);
        hud(g,held,layout,stick,interactionAvailable);
        if(story)drawStory(g,*story,layout);
        if(g.memoryMenu)drawMemoryMenu(g,story);
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

