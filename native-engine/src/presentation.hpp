#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <cstdio>
#include "core.hpp"
#include "sword_trails.hpp"
#include "forest_generator.hpp"
#include "forest_layers.hpp"

namespace eclipse {
class Presentation {
    SDL_Renderer* renderer;
    std::array<SDL_Texture*,5> textures{};
    std::array<SDL_Texture*,3> slimeTextures_{};
    float offset=0;
    ForestGenerator forestGenerator_{};
    std::array<SDL_Texture*,40> forestTextures_{};
    std::array<SDL_Texture*,4> forestLayerTextures_{};
    bool forestLayersReady_=false;
    std::array<SDL_Texture*,6> forestV3Textures_{};
    bool forestV3Ready_=false;
    bool forestEntranceReady_=false;
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
            // The ground layer has world-space parallax=1, preserving y=585.
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
        const int frame=slimeFrame(e.travelPhase,e.attackAge,e.kind);
        SDL_FRect source{float(frame%4*slimeCell),float(frame/4*slimeCell),slimeCell,slimeCell};
        SDL_FRect target{std::round(e.x-offset-slimePivotX*scale),std::round(e.y-slimePivotY*scale),slimeCell*scale,slimeCell*scale};
        auto* texture=slimeTextures_[e.kind];
        SDL_SetTextureColorMod(texture,255,e.hitFlash>0?135:255,e.hitFlash>0?135:255);
        SDL_RenderTextureRotated(renderer,texture,&source,&target,0,nullptr,e.facing>0?SDL_FLIP_HORIZONTAL:SDL_FLIP_NONE);
        SDL_SetTextureColorMod(texture,255,255,255);
        world(e.x-21,e.y-h-15,42,5,0x262435);world(e.x-21,e.y-h-15,42.f*e.health/e.maxHealth,5,0xf47777);
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
    void swordTrail(const Player& p,const Animation& a){
        if(p.health<=0||a.state<4||a.state>6)return;
        const auto frame=swordTrailFrames[a.frame];
        constexpr unsigned colors[]={0,0x365d70,0x7ba4ad,0xdce7db};constexpr int alpha[]={0,95,165,235};
        for(int i=frame.start;i<frame.start+frame.count;++i){const auto s=swordTrailSpans[i];
            float x=p.facing>0?p.x+(s.x-motionPivotX)*motionScale:p.x-(s.x+s.width-motionPivotX)*motionScale;
            rect(x-offset,p.y+(s.y-motionPivotY)*motionScale,s.width*motionScale,motionScale,colors[s.shade],static_cast<Uint8>(alpha[s.shade]*frame.alpha/255));
        }
    }
    void icon(int texture,int col,int row,float x,float y,float size,bool down){
        SDL_FRect source{col*128.f,row*128.f,128,128},target{x,y,size,size};
        SDL_SetTextureColorMod(textures[texture],down?180:255,down?180:255,down?180:255);SDL_RenderTexture(renderer,textures[texture],&source,&target);
    }
    void hud(const Combat& g,Actions held,const ControlLayout& layout,std::array<float,2> stick){
        const auto& p=g.player;constexpr const char* heart[]={"..XX.XX..",".XXXXXXX.","XXXXXXXXX","XXXXXXXXX",".XXXXXXX.","..XXXXX..","...XXX...","....X...."};
        for(int i=0;i<7;++i)for(int y=0;y<8;++y)for(int x=0;x<9;++x)if(heart[y][x]=='X')rect(360+i*55+x*5,18+y*5,6,6,static_cast<float>(p.health)/p.maxHealth()>=(i+.5f)/7?0xf56b78:0x364451);
        for(int i=0;i<5;++i){rect(365+i*48,76,42,16,0x101923);rect(369+i*48,80,34,8,std::clamp(1-p.skillCooldown/4,0.f,1.f)>i/5.f?0xe3bd72:0x353a43);}
        for(int i=0;i<4;++i){rect(765+i*74,21,66,61,0x080d17);rect(771+i*74,27,54,49,0x667482);rect(777+i*74,33,42,37,0x222c3d);}
        text("GEAR",780,44,2);text(p.form==0?"BLADE":"FIRE",235,48,3,0x9ed7ff);number(g.foes(),1180,43,5,0xece7d2);text("FOES",1090,25,2);
        const auto& c=layout.buttons;
        const auto& js=c[ControlLayout::Stick];
        icon(4,0,0,js.x,js.y,js.size,false);
        float thumb=js.size*.36f,tx=js.x+(js.size-thumb)*.5f+stick[0],ty=js.y+(js.size-thumb)*.5f+stick[1];
        rect(tx-3,ty-3,thumb+6,thumb+6,0x101923,210);
        rect(tx,ty,thumb,thumb,held.left||held.right||held.jump?0xe3bd72:0x91b4bb,245);
        rect(tx+thumb*.22f,ty+thumb*.22f,thumb*.56f,thumb*.56f,0x304f61,220);
        icon(1,held.attack?1:0,0,c[ControlLayout::Attack].x,c[ControlLayout::Attack].y,c[ControlLayout::Attack].size,held.attack);
        icon(2,held.dash?1:0,0,c[ControlLayout::Dash].x,c[ControlLayout::Dash].y,c[ControlLayout::Dash].size,held.dash);
        icon(3,0,0,c[ControlLayout::Swap].x,c[ControlLayout::Swap].y,c[ControlLayout::Swap].size,held.swap);
        icon(3,0,1,c[ControlLayout::Skill].x,c[ControlLayout::Skill].y,c[ControlLayout::Skill].size,held.skill);
        if(p.skillCooldown>0)number(static_cast<int>(std::ceil(p.skillCooldown)),c[ControlLayout::Skill].x+20,c[ControlLayout::Skill].y+25,4,0xffd277);
        if(p.swapCooldown>0)number(static_cast<int>(std::ceil(p.swapCooldown)),c[ControlLayout::Swap].x+20,c[ControlLayout::Swap].y+25,3,0xffd277);
        if(g.inventory){rect(46,135,460,340,0xe3bd72);rect(50,139,452,332,0x1c2a34);text("EQUIPMENT",66,153,3);
            for(int i=0;i<3;++i){text(p.gear.name(i),66,200+i*42,2);rect(66+i*143,413,132,45,0x304039);text(i==0?"WEAPON":i==1?"ARMOR":"CHARM",75+i*143,428,2);}
            text("ATK",66,340,2);number(p.gear.attack(),120,340,2);text("DEF",200,340,2);number(p.gear.defense(),250,340,2);text("MAX HP",66,373,2);number(p.maxHealth(),160,373,2);
        }
    }
public:
    explicit Presentation(SDL_Renderer* r):renderer(r){}
    ~Presentation(){for(auto* texture:textures)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:slimeTextures_)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestTextures_)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestLayerTextures_)if(texture)SDL_DestroyTexture(texture);
        for(auto* texture:forestV3Textures_)if(texture)SDL_DestroyTexture(texture);}
    void setForestSettings(ForestSettings settings) {forestGenerator_.regenerate(settings);}
    const ForestGenerator& forestGenerator() const {return forestGenerator_;}
    static std::string asset(const char* name){
#ifdef SDL_PLATFORM_ANDROID
        return name; // SDL 3.2 routes this through Android's AssetManager.
#else
        const char* base=SDL_GetBasePath();return std::string(base?base:"")+"assets/"+name;
#endif
    }
    bool load(){
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
    void draw(const Combat& g,const Animation& a,float camera,Actions held,const char* capture=nullptr,const char* reviewLabel=nullptr,const ControlLayout* customLayout=nullptr,std::array<float,2> stick={}){
        offset=std::round(camera)-640;SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer,21,35,52,255);SDL_RenderClear(renderer);
        forest(g,camera);for(const auto& e:g.enemies)if(!e.dead())enemy(e,g.clock);
        for(const auto& b:g.projectiles){disk(b.x-offset,b.y,b.empowered?14:8,0xf16b32);disk(b.x-offset,b.y,b.empowered?7:4,0xffe7a1);}
        const auto& p=g.player;
        if(p.form==0)swordTrail(p,a);
        if(p.form==0){SDL_FRect source{static_cast<float>(a.frame%motionColumns*motionWidth),static_cast<float>(a.frame/motionColumns*motionHeight),motionWidth,motionHeight},dest{std::round(p.x-offset-(p.facing>0?motionPivotX:motionWidth-motionPivotX)*motionScale),std::round(p.y-motionPivotY*motionScale),motionWidth*motionScale,motionHeight*motionScale};
            SDL_RenderTextureRotated(renderer,textures[0],&source,&dest,0,nullptr,p.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
        }else mage(p);
        if(p.swapFlash>0)ring(p.x-offset,p.y-36,44,0xe3bd72);
        for(const auto& popup:g.popups)number(popup.amount,popup.x-offset-12,popup.y,3,popup.kind==DamageKind::Fire?0xff9155:popup.kind==DamageKind::Bleed?0xe76878:0xf2d48c);
        static const ControlLayout defaultLayout{};
        const ControlLayout& layout=customLayout?*customLayout:defaultLayout;
        hud(g,held,layout,stick);
        rect(ControlLayout::gearX,ControlLayout::gearY,ControlLayout::gearSize,ControlLayout::gearSize,0x142736,230);
        text("CFG",1224,35,2,0xe3bd72);
        if(layout.editing){
            rect(0,0,1280,720,0x07101b,125);
            const char* names[]={"JOYSTICK","ATTACK","DASH","SWAP","SKILL"};
            text("CONTROL EDITOR",50,30,4,0xe3bd72);
            text("DRAG TO MOVE",50,83,3);
            text(names[layout.selected],50,125,3,0x9ed7ff);
            for(int i=0;i<ControlLayout::Count;++i){const auto& button=layout.buttons[i];
                if(i==layout.selected){
                    rect(button.x-5,button.y-5,button.size+10,5,0xe3bd72);
                    rect(button.x-5,button.y+button.size,button.size+10,5,0xe3bd72);
                    rect(button.x-5,button.y,5,button.size,0xe3bd72);
                    rect(button.x+button.size,button.y,5,button.size,0xe3bd72);
                }
            }
            rect(1030,90,205,66,0x304039);text("SAVE",1075,110,3);
            rect(1030,165,205,66,0x4c3536);text("RESET",1070,185,3);
            rect(1030,245,94,80,0x304039);text("-",1063,267,5);
            rect(1140,245,94,80,0x304039);text("+",1172,267,5);
            text("SIZE",1074,342,2);
        }
        if(reviewLabel)text(reviewLabel,470,120,3);
        if(g.ended){rect(0,0,1280,720,0x111a28,240);text(g.won?"STAGE CLEAR":"GAME OVER",390,260,7,g.won?0x8be4b5:0xff8d99);text("TAP RETRY OR PRESS R",410,350,3);
            rect(510,412,260,65,0xe3bd72);rect(514,416,252,57,0x304039);text("RETRY",548,433,4);
        }
        if(capture){auto* image=SDL_RenderReadPixels(renderer,nullptr);if(image){SDL_SaveBMP(image,capture);SDL_DestroySurface(image);}}
        SDL_RenderPresent(renderer);
    }
};
}

