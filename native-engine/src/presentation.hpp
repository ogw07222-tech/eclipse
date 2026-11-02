#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <cstdio>
#include "core.hpp"

namespace eclipse {
class Presentation {
    SDL_Renderer* renderer;
    std::array<SDL_Texture*,5> textures{};
    float offset=0;
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
    void pine(float cx,float bottom,float height,int layer,int seed){
        constexpr unsigned trunk[]={0x243c45,0x29433b,0x3b4934},leaf[]={0x244b48,0x2c5c49,0x3b744c},highlight[]={0x33575a,0x39705a,0x538859};
        world(cx-9,bottom-height*.64f,18,height*.64f,trunk[layer],6);
        for(int tier=0;tier<4;++tier){float width=38+(3-tier)*18,yy=bottom-height+tier*height*.19f;
            for(int row=0;row<5;++row){float rw=std::max(12.f,width-(4-row)*12);world(cx-rw*.5f,yy+row*12,rw,12,leaf[layer],6);}
            world(cx-width*.23f,yy+24,width*.3f,6,highlight[layer],6);
        }
        world(cx-5,bottom-height+4,12,6,highlight[layer],6);if(seed%3==0)world(cx+16,bottom-height*.43f,6,6,highlight[layer],6);
    }
    void forest(const Combat& g,float camera){
        rect(0,0,viewWidth,viewHeight,0x152334);
        for(int row=0;row<11;++row){int half=2+std::min(row,10-row);world(1190-half*6,105+row*6,(half*2+1)*6,6,0xaebda3);}
        for(int i=0;i<62;++i)world((i*233+49)%1600,(i*97+59)%315,6,6,0x718d8c);
        constexpr float factors[]={.12f,.28f,.48f};constexpr int steps[]={124,110,104},heights[]={172,210,242};
        for(int layer=0;layer<3;++layer){float parallax=std::round((camera-640)*factors[layer]*.25f/6)*6;
            for(int i=0;i<22;++i)pine(i*steps[layer]+layer*47-100-parallax,584,heights[layer]+(i*19+layer*7)%7*6,layer,i);
        }
        world(0,578,1600,12,0x527b45);
        for(int i=0;i<210;++i){float breeze=static_cast<int>(std::sin(g.clock*1.7f+i)*1.5f)*6;world(i*47%1600+breeze,566-i%3*6,6,12,0x83a65a);}
        world(0,585,1600,135,0x2b4034);world(0,585,1600,8,0x73995d);for(int x=0;x<1600;x+=80)world(x,615,55,3,0x334661);
    }
    void enemy(const Enemy& e,float clock){
        float half=e.kind?21:16,h=e.kind?62:48;
        world(e.x-half,e.y-h,half*2,h,e.hitFlash>0?0xffffff:e.kind?0xba6c83:0x9d7abf);
        disk(e.x-7-offset,e.y-h+16,4,0xffed9c,2);disk(e.x+7-offset,e.y-h+16,4,0xffed9c,2);
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
    void icon(int texture,int col,int row,float x,float y,float size,bool down){
        SDL_FRect source{col*128.f,row*128.f,128,128},target{x,y,size,size};
        SDL_SetTextureColorMod(textures[texture],down?180:255,down?180:255,down?180:255);SDL_RenderTexture(renderer,textures[texture],&source,&target);
    }
    void hud(const Combat& g,Actions held){
        const auto& p=g.player;constexpr const char* heart[]={"..XX.XX..",".XXXXXXX.","XXXXXXXXX","XXXXXXXXX",".XXXXXXX.","..XXXXX..","...XXX...","....X...."};
        for(int i=0;i<7;++i)for(int y=0;y<8;++y)for(int x=0;x<9;++x)if(heart[y][x]=='X')rect(360+i*55+x*5,18+y*5,6,6,static_cast<float>(p.health)/p.maxHealth()>=(i+.5f)/7?0xf56b78:0x364451);
        for(int i=0;i<5;++i){rect(365+i*48,76,42,16,0x101923);rect(369+i*48,80,34,8,std::clamp(1-p.skillCooldown/4,0.f,1.f)>i/5.f?0xe3bd72:0x353a43);}
        for(int i=0;i<4;++i){rect(765+i*74,21,66,61,0x080d17);rect(771+i*74,27,54,49,0x667482);rect(777+i*74,33,42,37,0x222c3d);}
        text("GEAR",780,44,2);text(p.form==0?"BLADE":"FIRE",235,48,3,0x9ed7ff);number(g.foes(),1180,43,5,0xece7d2);text("FOES",1090,25,2);
        icon(4,0,0,35,586,128,held.left||held.right||held.jump);icon(1,held.attack?1:0,0,1110,540,116,held.attack);
        icon(2,held.dash?1:0,0,985,560,100,held.dash);icon(3,0,0,995,445,90,held.swap);icon(3,0,1,1115,420,94,held.skill);
        if(p.skillCooldown>0)number(static_cast<int>(std::ceil(p.skillCooldown)),1150,450,4,0xffd277);
        if(p.swapCooldown>0)number(static_cast<int>(std::ceil(p.swapCooldown)),1025,470,3,0xffd277);
        if(g.inventory){rect(46,135,460,340,0xe3bd72);rect(50,139,452,332,0x1c2a34);text("EQUIPMENT",66,153,3);
            for(int i=0;i<3;++i){text(p.gear.name(i),66,200+i*42,2);rect(66+i*143,413,132,45,0x304039);text(i==0?"WEAPON":i==1?"ARMOR":"CHARM",75+i*143,428,2);}
            text("ATK",66,340,2);number(p.gear.attack(),120,340,2);text("DEF",200,340,2);number(p.gear.defense(),250,340,2);text("MAX HP",66,373,2);number(p.maxHealth(),160,373,2);
        }
    }
public:
    explicit Presentation(SDL_Renderer* r):renderer(r){}
    ~Presentation(){for(auto* texture:textures)if(texture)SDL_DestroyTexture(texture);}
    static std::string asset(const char* name){
#ifdef SDL_PLATFORM_ANDROID
        return name; // SDL 3.2 routes this through Android's AssetManager.
#else
        const char* base=SDL_GetBasePath();return std::string(base?base:"")+"assets/"+name;
#endif
    }
    bool load(){
        constexpr const char* files[]={"knight_sheet_48x64_v2.png","ui/attack_states.png","ui/dash_states.png","ui/swap_skill_states.png","ui/joystick_arrows_new.png"};
        for(int i=0;i<5;++i){const auto path=asset(files[i]);SDL_Surface* image=IMG_Load(path.c_str());
            if(!image){SDL_Log("Required PNG missing or invalid: %s (%s)",path.c_str(),SDL_GetError());return false;}
            if(i==0&&(image->w!=384||image->h!=448)){SDL_Log("Expected 384x448 knight atlas");SDL_DestroySurface(image);return false;}
            // Compact oversized button sheets once, keeping only 128px per state on the GPU.
            if(i>0){int cols=(i==1||i==2||i==3)?2:1,rows=i==3?2:1;auto* scaled=SDL_ScaleSurface(image,128*cols,128*rows,SDL_SCALEMODE_NEAREST);SDL_DestroySurface(image);image=scaled;if(!image)return false;}
            textures[i]=SDL_CreateTextureFromSurface(renderer,image);SDL_DestroySurface(image);
            if(!textures[i])return false;
            SDL_SetTextureScaleMode(textures[i],SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(textures[i],SDL_BLENDMODE_BLEND);
        }
        return true;
    }
    void draw(const Combat& g,const Animation& a,float camera,Actions held,const char* capture=nullptr){
        offset=std::round(camera)-640;SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer,21,35,52,255);SDL_RenderClear(renderer);
        forest(g,camera);for(const auto& e:g.enemies)if(!e.dead())enemy(e,g.clock);
        for(const auto& b:g.projectiles){disk(b.x-offset,b.y,b.empowered?14:8,0xf16b32);disk(b.x-offset,b.y,b.empowered?7:4,0xffe7a1);}
        const auto& p=g.player;
        if(p.form==0){SDL_FRect source{static_cast<float>(a.frame%8*48),static_cast<float>(a.frame/8*64),48,64},dest{std::round(p.x-offset-48),std::round(p.y-128),96,128};
            SDL_RenderTextureRotated(renderer,textures[0],&source,&dest,0,nullptr,p.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
        }else mage(p);
        if(p.swapFlash>0)ring(p.x-offset,p.y-36,44,0xe3bd72);
        for(const auto& popup:g.popups)number(popup.amount,popup.x-offset-12,popup.y,3,popup.kind==DamageKind::Fire?0xff9155:popup.kind==DamageKind::Bleed?0xe76878:0xf2d48c);
        hud(g,held);
        if(g.ended){rect(0,0,1280,720,0x111a28,240);text(g.won?"STAGE CLEAR":"GAME OVER",390,260,7,g.won?0x8be4b5:0xff8d99);text("TAP RETRY OR PRESS R",410,350,3);
            rect(510,412,260,65,0xe3bd72);rect(514,416,252,57,0x304039);text("RETRY",548,433,4);
        }
        if(capture){auto* image=SDL_RenderReadPixels(renderer,nullptr);if(image){SDL_SaveBMP(image,capture);SDL_DestroySurface(image);}}
        SDL_RenderPresent(renderer);
    }
};
}
