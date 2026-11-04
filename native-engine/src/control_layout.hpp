#pragma once
#include <array>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace eclipse {
struct ControlRect { float x,y,size; };
class ControlLayout {
public:
    enum {Stick,Attack,Dash,Swap,Skill,Sweep,Buff,Count};
    enum Page { Sound, Controls, Game };
    static constexpr std::array<ControlRect,Count> defaultButtons{{
        {35,586,128},{1110,540,116},{985,560,100},{995,445,90},{1115,420,94},
        {902,455,82},{1030,347,82}
    }};
    std::array<ControlRect,Count> buttons=defaultButtons;
    std::string storagePath;
    bool editing=false;             // compatibility with current game pause/render loop
    bool exitRequested=false;
    bool reposition=false;          // overlay for on-screen button manipulation
    Page page=Controls;
    int selected=Stick;
    std::int64_t dragging=-999;
    float grabX=0,grabY=0;
    std::array<std::int64_t,2> touchIds{{-999,-999}};
    std::array<std::array<float,2>,2> touchPoints{};
    float pinchStart=0,pinchSize=0;
    int pinchButton=-1;
    int bgm=80,sfx=90;
    bool screenShake=true,showNumbers=true;
    static constexpr float gearX=1198,gearY=18,gearSize=64;
    static constexpr float backX=1120,backY=103,backWidth=90,backHeight=65;
    static constexpr float menuX=12,menuY=47,menuScale=.8f;
    static constexpr float menuWidth=344*menuScale,menuHeight=783*menuScale;
    static constexpr float tabX=menuX+49*menuScale,tabWidth=250*menuScale,tabHeight=86*menuScale;
    static constexpr std::array<float,4> tabY{{
        menuY+206*menuScale,menuY+306*menuScale,menuY+410*menuScale,menuY+513*menuScale
    }};
    static bool inside(float x,float y,float bx,float by,float w,float h){return x>=bx&&x<bx+w&&y>=by&&y<by+h;}
    int hit(float x,float y) const {
        for(int i=Count-1;i>=0;--i)if(inside(x,y,buttons[i].x,buttons[i].y,buttons[i].size,buttons[i].size))return i;
        return -1;
    }
    void clamp(int i) {
        auto& b=buttons[i];b.size=std::clamp(b.size,60.f,230.f);
        b.x=std::clamp(b.x,0.f,1280.f-b.size);
        b.y=std::clamp(b.y,0.f,720.f-b.size);
    }
    void defaults(){buttons=defaultButtons;selected=Stick;save();}
    std::string path() const {return storagePath;}
    void load(){
        auto file=path();if(file.empty())return;
        FILE* f=std::fopen(file.c_str(),"r");if(!f)return;
        int version=0;bool valid=std::fscanf(f,"ECLIPSE_CONTROLS %d",&version)==1&&(version>=1&&version<=3);
        auto next=defaultButtons;
        const int savedCount=version>=3?Count:5;
        for(int i=0;valid&&i<savedCount;++i)valid=std::fscanf(f,"%f %f %f",&next[i].x,&next[i].y,&next[i].size)==3&&
            std::isfinite(next[i].x)&&std::isfinite(next[i].y)&&std::isfinite(next[i].size);
        if(valid&&version>=2){
            int nextBgm=bgm,nextSfx=sfx,shake=1,numbers=1;
            if(std::fscanf(f,"%d %d %d %d",&nextBgm,&nextSfx,&shake,&numbers)==4){
                bgm=std::clamp(nextBgm,0,100);sfx=std::clamp(nextSfx,0,100);
                screenShake=shake!=0;showNumbers=numbers!=0;
            }
        }
        std::fclose(f);if(!valid)return;
        buttons=next;for(int i=0;i<Count;++i)clamp(i);
    }
    void save() const {
        auto file=path();if(file.empty())return;
        FILE* f=std::fopen(file.c_str(),"w");if(!f)return;
        std::fprintf(f,"ECLIPSE_CONTROLS 3\n");
        for(const auto& b:buttons)std::fprintf(f,"%.2f %.2f %.2f\n",b.x,b.y,b.size);
        std::fprintf(f,"%d %d %d %d\n",bgm,sfx,screenShake?1:0,showNumbers?1:0);
        std::fclose(f);
    }
    void clearTouches(){touchIds={{-999,-999}};pinchButton=-1;pinchStart=0;dragging=-999;}
    void close(){editing=false;reposition=false;clearTouches();save();}
    void startReposition(){reposition=true;clearTouches();}
    void finishReposition(){reposition=false;clearTouches();save();}
    float distance() const {
        float dx=touchPoints[0][0]-touchPoints[1][0],dy=touchPoints[0][1]-touchPoints[1][1];
        return std::sqrt(dx*dx+dy*dy);
    }
    // Modal settings input. SDL passes unique finger identifiers and logical 1280x720 coordinates.
    bool pointer(std::int64_t id,float x,float y,bool down,bool motion=false){
        if(!editing){
            if(down&&!motion&&inside(x,y,gearX,gearY,gearSize,gearSize)){
                editing=true;page=Controls;reposition=false;clearTouches();return true;
            }
            return false;
        }
        if(reposition){
            if(!down){
                for(int t=0;t<2;++t)if(touchIds[t]==id)touchIds[t]=-999;
                if(dragging==id)dragging=-999;
                if(pinchButton>=0){pinchButton=-1;pinchStart=0;}
                save();return true;
            }
            if(motion&&touchIds[0]!=id&&touchIds[1]!=id)return true;
            int slot=-1;
            for(int t=0;t<2;++t)if(touchIds[t]==id)slot=t;
            if(slot<0)for(int t=0;t<2;++t)if(touchIds[t]==-999){slot=t;touchIds[t]=id;break;}
            if(slot>=0)touchPoints[slot]={{x,y}};
            if(touchIds[0]!=-999&&touchIds[1]!=-999){
                if(pinchButton<0){
                    int a=hit(touchPoints[0][0],touchPoints[0][1]);
                    int b=hit(touchPoints[1][0],touchPoints[1][1]);
                    if(a>=0&&a==b){pinchButton=a;selected=a;pinchStart=distance();pinchSize=buttons[a].size;}
                }
                if(pinchButton>=0&&pinchStart>8){
                    auto& b=buttons[pinchButton];
                    float cx=b.x+b.size*.5f,cy=b.y+b.size*.5f;
                    b.size=std::clamp(pinchSize*distance()/pinchStart,60.f,230.f);
                    b.x=cx-b.size*.5f;b.y=cy-b.size*.5f;clamp(pinchButton);
                }
                return true;
            }
            if(dragging==id&&slot>=0){auto& b=buttons[selected];b.x=x-grabX;b.y=y-grabY;clamp(selected);return true;}
            if(!motion&&inside(x,y,1030,80,205,65)){finishReposition();return true;}
            int which=hit(x,y);
            if(which>=0){selected=which;dragging=id;grabX=x-buttons[which].x;grabY=y-buttons[which].y;}
            return true;
        }
        if(!down)return true;
        // Motion only updates sliders or an active touch layout gesture.
        if(!motion){
            if(inside(x,y,gearX,gearY,gearSize,gearSize)||
               inside(x,y,backX,backY,backWidth,backHeight)){close();return true;}
            for(int i=0;i<4;++i){
                if(!inside(x,y,tabX,tabY[i],tabWidth,tabHeight))continue;
                if(i==3){close();exitRequested=true;}
                else page=static_cast<Page>(i);
                return true;
            }
        }else if(page!=Sound)return true;
        if(page==Controls){
            if(inside(x,y,440,280,600,74)){startReposition();return true;}
            if(inside(x,y,440,385,100,68)){buttons[selected].size-=10;clamp(selected);save();return true;}
            if(inside(x,y,950,385,100,68)){buttons[selected].size+=10;clamp(selected);save();return true;}
            if(inside(x,y,445,485,285,68)){defaults();return true;}
            if(inside(x,y,750,485,285,68)){save();return true;}
        }else if(page==Sound){
            if(inside(x,y,460,270,575,78)){bgm=std::clamp(static_cast<int>((x-460)/575*100),0,100);save();return true;}
            if(inside(x,y,460,390,575,78)){sfx=std::clamp(static_cast<int>((x-460)/575*100),0,100);save();return true;}
        }else{
            if(inside(x,y,440,285,600,75)){screenShake=!screenShake;save();return true;}
            if(inside(x,y,440,385,600,75)){showNumbers=!showNumbers;save();return true;}
        }
        return true;
    }
};
}
