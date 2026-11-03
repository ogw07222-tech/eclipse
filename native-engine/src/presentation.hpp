#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <cstdio>
#include "core.hpp"
#include "sword_trails.hpp"

namespace eclipse {
class Presentation {
    SDL_Renderer* renderer;
    std::array<SDL_Texture*,5> textures{};
    SDL_Texture* hitTexture=nullptr;
    struct Ghost{Player pose;int frame=0;float born=-100;};
    std::array<Ghost,6> ghosts{};int ghostIndex=0;float ghostClock=-1,lastGhost=-100;
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
    void disk(float x,float y,float radius,unsigned rgb,int cell=4,Uint8 alpha=255){
        for(int yy=-static_cast<int>(radius);yy<=radius;yy+=cell)for(int xx=-static_cast<int>(radius);xx<=radius;xx+=cell)
            if(xx*xx+yy*yy<=radius*radius)rect(x+xx,y+yy,cell,cell,rgb,alpha);
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
            else if(*s=='/')for(int k=0;k<5;++k)rect(x+(4-k)*scale,y+k*scale,scale,scale,rgb);
            x+=6*scale;
        }
    }
    void number(int n,float x,float y,int scale=3,unsigned rgb=0xf2d48c){char buffer[24];std::snprintf(buffer,sizeof(buffer),"%d",n);text(buffer,x,y,scale,rgb);}

    // Stable procedural variation; integer pixel blocks and independent layer motion.
    static unsigned forestHash(int seed,int salt){
        unsigned h=static_cast<unsigned>(seed)*374761393u+static_cast<unsigned>(salt)*668265263u;
        h=(h^(h>>13))*1274126177u;
        return (h^(h>>16))&0x7fffffffu;
    }
    void forestTree(float cx,float bottom,float height,int layer,int seed){
        constexpr unsigned trunks[]={0x243c45,0x29433b,0x3b4934};
        constexpr unsigned leaves[]={0x244b48,0x2c5c49,0x3b744c};
        constexpr unsigned highlights[]={0x33575a,0x39705a,0x538859};
        float trunk=12.f+forestHash(seed,6)%3*6;
        float crownY=bottom-height*(.62f+(forestHash(seed,7)%9)*.025f);
        world(cx-trunk*.5f,crownY,trunk,bottom-crownY,trunks[layer],6);
        if(seed%4==0){
            world(cx-24,crownY+24,30,6,trunks[layer],6);
            world(cx+3,crownY+12,30,6,trunks[layer],6);
        }else{
            world(cx-30,crownY+36,33,6,trunks[layer],6);
            world(cx+3,crownY+24,33,6,trunks[layer],6);
        }
        float radius=38.f+forestHash(seed,10)%5*6+layer*5;
        if(seed%7==0){
            for(int tier=0;tier<3;++tier){
                float y=bottom-height+tier*39,w=radius*(1.1f+tier*.15f);
                for(int row=0;row<5;++row)
                    world(cx-w*.5f+row*5,y+row*9,w-row*10,12,leaves[layer],6);
            }
        }else{
            for(int clump=0;clump<7;++clump){
                unsigned h=forestHash(seed,20+clump);
                float x=cx+(static_cast<int>(h%17)-8)*6;
                float y=bottom-height+((h/19)%9)*6+(clump%3)*15;
                float r=radius*(.42f+((h/37)%5)*.08f);
                for(int row=-3;row<=3;++row){
                    float w=r*(1.f-std::abs(row)*.16f);
                    world(x-w,y+row*9,w*2,12,leaves[layer],6);
                }
            }
        }
        for(int k=0;k<6;++k){
            unsigned h=forestHash(seed,51+k);
            float x=cx+(static_cast<int>(h%17)-8)*6;
            float y=bottom-height+(h/23%11)*6;
            world(x,y,12+h%3*6,6,highlights[layer],6);
        }
        if(layer==2&&seed%5==0){
            world(cx-3,bottom-38,6,30,0x43604a,6);
            world(cx+6,bottom-24,6,12,0x527b45,6);
        }
    }
    void forestBush(float x,float bottom,float radius,int layer,int seed){
        constexpr unsigned leaves[]={0x244b48,0x2c5c49,0x3b744c};
        constexpr unsigned highlights[]={0x33575a,0x39705a,0x538859};
        for(int n=0;n<4;++n){
            unsigned h=forestHash(seed,70+n);
            float bx=x+(n-2)*12+h%9,top=bottom-radius-h%13;
            world(bx-12,top+9,30,radius*.65f,leaves[layer],6);
            world(bx-6,top,18,12,leaves[layer],6);
            if(n%2==0)world(bx,top+6,12,6,highlights[layer],6);
        }
    }
    void forest(const Combat& g,float camera){
        rect(0,0,viewWidth,viewHeight,0x152334);
        for(int row=0;row<11;++row){
            int half=2+std::min(row,10-row);
            world(1190-half*6,105+row*6,(half*2+1)*6,6,0xaebda3);
        }
        for(int i=0;i<62;++i)world((i*233+49)%1600,(i*97+59)%315,6,6,0x718d8c);
        constexpr float factors[]={.12f,.28f,.48f};
        constexpr int steps[]={124,110,104},heights[]={172,210,242};
        for(int layer=0;layer<3;++layer){
            // world() subtracts the full camera offset; add its complement.
            float shift=std::round((camera-640)*(1.f-factors[layer])/6.f)*6.f;
            for(int i=-3;i<22;++i){
                int seed=i+layer*173;
                float x=i*steps[layer]+(static_cast<int>(forestHash(seed,1)%65)-32)+shift;
                float height=heights[layer]+(static_cast<int>(forestHash(seed,2)%12)-5)*6;
                forestTree(x,584,height,layer,seed);
            }
            for(int i=-2;i<35;++i){
                int seed=i+layer*391;
                float x=i*61+forestHash(seed,9)%42+shift;
                forestBush(x,579-layer*3,21+forestHash(seed,8)%23,layer,seed);
            }
        }
        world(0,578,1600,12,0x527b45);
        world(0,585,1600,135,0x2b4034);
        world(0,585,1600,8,0x73995d);
        for(int i=0;i<115;++i){
            int seed=i+861;
            float x=i*15+forestHash(seed,3)%12;
            if(x>1600)break;
            float sway=std::round(std::sin(g.clock*1.5f+seed))*3;
            world(x+sway,574-forestHash(seed,4)%3*6,6,6+forestHash(seed,5)%3*6,
                i%7==0?0x83a65a:i%3==0?0x527b45:0x73995d,3);
            if(i%11==0){
                world(x-6,581,18,6,0x334661,3);
                world(x,578,6,3,0x62736b,3);
            }
            if(i%19==0){
                world(x,562,3,15,0x527b45,3);
                world(x-3,556,9,6,0xb6aa88,3);
            }
        }
        for(int i=0;i<45;++i){
            int seed=1171+i;
            float x=i*37+forestHash(seed,2)%20;
            world(x,604+forestHash(seed,3)%15*6,6+forestHash(seed,4)%4*6,6,
                i%4==0?0x334661:0x3a5141,6);
        }
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
    void knight(const Player& p,int frame,SDL_Texture* texture,Uint8 alpha=255){
        SDL_FRect source{static_cast<float>(frame%motionColumns*motionWidth),static_cast<float>(frame/motionColumns*motionHeight),motionWidth,motionHeight};
        SDL_FRect dest{std::round(p.x-offset-(p.facing>0?motionPivotX:motionWidth-motionPivotX)*3),std::round(p.y-motionPivotY*3),motionWidth*3,motionHeight*3};
        SDL_SetTextureAlphaMod(texture,alpha);
        SDL_RenderTextureRotated(renderer,texture,&source,&dest,0,nullptr,p.facing>0?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL);
        SDL_SetTextureAlphaMod(texture,255);
    }
    void mage(const Player& p,bool ghost=false,Uint8 alpha=255){
        float x=p.x-offset,y=p.y;const bool flash=p.hitFlash>0;
        auto c=[&](unsigned rgb){return ghost?0x758c9cu:flash?0xffffffu:rgb;};
        rect(x-15,y-55,30,40,c(0x5a2837),alpha);rect(x-12,y-52,24,30,c(0xf69a5c),alpha);
        disk(x,y-60,15,c(0xf6d3b3),3,alpha);rect(x-12,y-70,24,10,c(0x5a2837),alpha);
        rect(x+p.facing*7-2,y-62,4,4,c(0x151b2c),alpha);rect(x-12,y-15,10,15,c(0x5a2837),alpha);rect(x+2,y-15,10,15,c(0x5a2837),alpha);
        for(int i=0;i<9;++i)rect(x+p.facing*(15+i),y-35-i*3,6,6,c(0x8f6ad4),alpha);
        disk(x+p.facing*22,y-64,7,c(0xffb74b),3,alpha);
        if(!ghost&&p.attackFlash>0)ring(x+p.facing*16,y-36,40,flash?0xffffff:0xffb74b);
    }
    void dashGhosts(const Combat& g,const Animation& a){
        if(g.clock<ghostClock||g.clock==0){ghosts={};lastGhost=-100;ghostIndex=0;}
        ghostClock=g.clock;
        if(!g.showAfterimages){ghosts={};return;}
        if(g.player.dashLeft>0&&g.clock-lastGhost>=.025f){
            ghosts[ghostIndex]={g.player,a.frame,g.clock};ghostIndex=(ghostIndex+1)%ghosts.size();lastGhost=g.clock;
        }
        for(std::size_t i=0;i<ghosts.size();++i){const auto& ghost=ghosts[(ghostIndex+i)%ghosts.size()];float age=g.clock-ghost.born;
            if(age<0||age>=.18f||g.player.health<=0)continue;
            Uint8 alpha=static_cast<Uint8>(100*(1-age/.18f));
            if(ghost.pose.form==0){SDL_SetTextureColorMod(textures[0],115,145,165);knight(ghost.pose,ghost.frame,textures[0],alpha);SDL_SetTextureColorMod(textures[0],255,255,255);}
            else mage(ghost.pose,true,alpha);
        }
    }
    void swordTrail(const Player& p,const Animation& a){
        if(p.health<=0||a.state<4||a.state>6)return;
        const auto frame=swordTrailFrames[a.frame];
        constexpr unsigned colors[]={0,0x365d70,0x7ba4ad,0xdce7db};constexpr int alpha[]={0,95,165,235};
        for(int i=frame.start;i<frame.start+frame.count;++i){const auto s=swordTrailSpans[i];
            auto box=swordSpanBounds(s,p.x,p.y,p.facing,p.gear.range());
            rect(box.x-offset,box.y,box.w,box.h,colors[s.shade],static_cast<Uint8>(alpha[s.shade]*frame.alpha/255));
        }
    }
    void icon(int texture,int col,int row,float x,float y,float size,bool down){
        SDL_FRect source{col*128.f,row*128.f,128,128},target{x,y,size,size};
        SDL_SetTextureColorMod(textures[texture],down?180:255,down?180:255,down?180:255);SDL_RenderTexture(renderer,textures[texture],&source,&target);
    }
    void hud(const Combat& g,Actions held){
        const auto& p=g.player;
        rect(354,14,392,50,0x101923);rect(358,18,384,42,0x889598);rect(362,22,376,34,0x263039);
        int fill=static_cast<int>(std::round(368.f*p.health/p.maxHealth()/4))*4;
        if(fill>0){rect(366,26,fill,26,p.health*4<=p.maxHealth()?0x984853:0x658b72);rect(366,26,fill,4,0x95b49a);}
        char hp[32];std::snprintf(hp,sizeof(hp),"HP %d / %d",p.health,p.maxHealth());text(hp,398,32,3,0x101923);text(hp,396,30,3,0xe4e7d8);
        for(int i=0;i<5;++i){rect(365+i*48,76,42,16,0x101923);rect(369+i*48,80,34,8,std::clamp(1-p.skillCooldown/4,0.f,1.f)>i/5.f?0xe3bd72:0x353a43);}
        for(int i=0;i<4;++i){rect(765+i*74,21,66,61,0x080d17);rect(771+i*74,27,54,49,0x667482);rect(777+i*74,33,42,37,0x222c3d);}
        text("GEAR",780,44,2);text(p.form==0?"BLADE":"FIRE",235,48,3,0x9ed7ff);number(g.foes(),1145,44,4,0xece7d2);text("FOES",1080,25,2);
        settingsGear(g.settings);
        icon(4,0,0,35,586,128,held.left||held.right||held.jump);icon(1,held.attack?1:0,0,1110,540,116,held.attack);
        icon(2,held.dash?1:0,0,985,560,100,held.dash);icon(3,0,0,995,445,90,held.swap);icon(3,0,1,1115,420,94,held.skill);
        if(p.skillCooldown>0)number(static_cast<int>(std::ceil(p.skillCooldown)),1150,450,4,0xffd277);
        if(p.swapCooldown>0)number(static_cast<int>(std::ceil(p.swapCooldown)),1025,470,3,0xffd277);
        if(g.inventory){rect(46,135,460,340,0xe3bd72);rect(50,139,452,332,0x1c2a34);text("EQUIPMENT",66,153,3);
            for(int i=0;i<3;++i){text(p.gear.name(i),66,200+i*42,2);rect(66+i*143,413,132,45,0x304039);text(i==0?"WEAPON":i==1?"ARMOR":"CHARM",75+i*143,428,2);}
            text("ATK",66,340,2);number(p.gear.attack(),120,340,2);text("DEF",200,340,2);number(p.gear.defense(),250,340,2);text("MAX HP",66,373,2);number(p.maxHealth(),160,373,2);
        }
    }
    void settingsGear(bool active){
        rect(1206,16,58,58,0x101923);rect(1210,20,50,50,active?0x536570:0x2c3843);
        constexpr const char* pixels[]={"00011011000","00111111100","01110001110","11100000111","11000100011","11001110011","11000100011","11100000111","01110001110","00111111100","00011011000"};
        for(int y=0;y<11;++y)for(int x=0;x<11;++x)if(pixels[y][x]=='1')rect(1213+x*4,23+y*4,4,4,0xc5ccc5);
    }
    void settingsPanel(const Combat& g){
        rect(0,0,1280,720,0x080d17,210);rect(410,160,460,410,0x899899);rect(414,164,452,402,0x182430);
        text("SETTINGS",448,192,4);text("PAUSED",698,204,2,0x93a99e);
        const char* labels[]={"SCREEN SHAKE","DAMAGE NUMBERS","DASH AFTERIMAGE"};
        bool enabled[]={g.screenShake,g.showDamage,g.showAfterimages};
        for(int row=0;row<3;++row){int y=250+row*60;rect(450,y,380,44,0x2c3b45);text(labels[row],462,y+14,2);
            rect(752,y+8,66,28,enabled[row]?0x597966:0x444c58);text(enabled[row]?"ON":"OFF",760,y+15,2,0xe4e7d8);}
        rect(450,438,380,44,0x536d60);text("RESUME",584,452,2);
        rect(450,500,380,44,0x65454e);text("RESTART",578,514,2);
    }
public:
    explicit Presentation(SDL_Renderer* r):renderer(r){}
    ~Presentation(){for(auto* texture:textures)if(texture)SDL_DestroyTexture(texture);if(hitTexture)SDL_DestroyTexture(hitTexture);}
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
            if(i==0&&(image->w!=motionWidth*motionColumns||image->h!=motionHeight*motionRows)){SDL_Log("Expected 1024x576 motion atlas");SDL_DestroySurface(image);return false;}
            // Compact oversized button sheets once, keeping only 128px per state on the GPU.
            if(i>0){int cols=(i==1||i==2||i==3)?2:1,rows=i==3?2:1;auto* scaled=SDL_ScaleSurface(image,128*cols,128*rows,SDL_SCALEMODE_NEAREST);SDL_DestroySurface(image);image=scaled;if(!image)return false;}
            textures[i]=SDL_CreateTextureFromSurface(renderer,image);
            if(i==0){
                SDL_Surface* white=SDL_ConvertSurface(image,SDL_PIXELFORMAT_RGBA32);
                if(!white){SDL_DestroySurface(image);return false;}
                SDL_LockSurface(white);
                for(int y=0;y<white->h;++y){auto* pixels=static_cast<Uint8*>(white->pixels)+y*white->pitch;
                    for(int x=0;x<white->w;++x){pixels[x*4]=255;pixels[x*4+1]=255;pixels[x*4+2]=255;}}
                SDL_UnlockSurface(white);hitTexture=SDL_CreateTextureFromSurface(renderer,white);SDL_DestroySurface(white);
                if(!hitTexture){SDL_DestroySurface(image);return false;}
                SDL_SetTextureScaleMode(hitTexture,SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(hitTexture,SDL_BLENDMODE_BLEND);
            }
            SDL_DestroySurface(image);
            if(!textures[i])return false;
            SDL_SetTextureScaleMode(textures[i],SDL_SCALEMODE_NEAREST);SDL_SetTextureBlendMode(textures[i],SDL_BLENDMODE_BLEND);
        }
        return true;
    }
    void draw(const Combat& g,const Animation& a,float camera,Actions held,const char* capture=nullptr,const char* reviewLabel=nullptr){
        offset=std::round(camera)-640;
        if(g.screenShake&&!g.settings&&g.player.hitFlash>0)offset+=static_cast<int>(g.clock*60)%2?3:-3;
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer,21,35,52,255);SDL_RenderClear(renderer);
        forest(g,camera);for(const auto& e:g.enemies)if(!e.dead())enemy(e,g.clock);
        for(const auto& b:g.projectiles){disk(b.x-offset,b.y,b.empowered?14:8,0xf16b32);disk(b.x-offset,b.y,b.empowered?7:4,0xffe7a1);}
        const auto& p=g.player;
        dashGhosts(g,a);
        if(p.form==0){swordTrail(p,a);knight(p,a.frame,textures[0]);if(p.hitFlash>0)knight(p,a.frame,hitTexture,220);}
        else mage(p);
        if(p.swapFlash>0)ring(p.x-offset,p.y-36,44,0xe3bd72);
        if(g.showDamage)for(const auto& popup:g.popups)number(popup.amount,popup.x-offset-12,popup.y,3,popup.kind==DamageKind::Fire?0xff9155:popup.kind==DamageKind::Bleed?0xe76878:0xf2d48c);
        hud(g,held);if(reviewLabel)text(reviewLabel,470,120,3);
        if(g.ended){rect(0,0,1280,720,0x111a28,240);text(g.won?"STAGE CLEAR":"GAME OVER",390,260,7,g.won?0x8be4b5:0xff8d99);text("TAP RETRY OR PRESS R",410,350,3);
            rect(510,412,260,65,0xe3bd72);rect(514,416,252,57,0x304039);text("RETRY",548,433,4);
        }
        if(g.settings)settingsPanel(g);
        if(capture){auto* image=SDL_RenderReadPixels(renderer,nullptr);if(image){SDL_SaveBMP(image,capture);SDL_DestroySurface(image);}}
        SDL_RenderPresent(renderer);
    }
};
}
