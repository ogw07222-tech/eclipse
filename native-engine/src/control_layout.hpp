#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace eclipse {
struct ControlRect { float x,y,size; };
class ControlLayout {
public:
    enum {Stick,Attack,Dash,Swap,Skill,Count};
    std::array<ControlRect,Count> buttons{{{35,586,128},{1110,540,116},{985,560,100},{995,445,90},{1115,420,94}}};
    std::string storagePath;
    bool editing=false;
    int selected=Stick;
    std::int64_t dragging=-999;
    float grabX=0,grabY=0;
    static constexpr float gearX=1218,gearY=18,gearSize=48;
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
    void defaults(){buttons={{{35,586,128},{1110,540,116},{985,560,100},{995,445,90},{1115,420,94}}};selected=Stick;save();}
    std::string path() const {return storagePath;}
    void load(){
        auto file=path();if(file.empty())return;
        FILE* f=std::fopen(file.c_str(),"r");if(!f)return;
        int version=0;bool valid=std::fscanf(f,"ECLIPSE_CONTROLS %d",&version)==1&&version==1;
        auto next=buttons;
        for(int i=0;valid&&i<Count;++i)valid=std::fscanf(f,"%f %f %f",&next[i].x,&next[i].y,&next[i].size)==3&&
            std::isfinite(next[i].x)&&std::isfinite(next[i].y)&&std::isfinite(next[i].size);
        std::fclose(f);if(!valid)return;
        buttons=next;for(int i=0;i<Count;++i)clamp(i);
    }
    void save() const {
        auto file=path();if(file.empty())return;
        FILE* f=std::fopen(file.c_str(),"w");if(!f)return;
        std::fprintf(f,"ECLIPSE_CONTROLS 1\n");
        for(const auto& b:buttons)std::fprintf(f,"%.2f %.2f %.2f\n",b.x,b.y,b.size);
        std::fclose(f);
    }
    void close(){editing=false;dragging=-999;save();}
    // Returns true if the gesture belongs to settings rather than gameplay.
    bool pointer(std::int64_t id,float x,float y,bool down){
        if(!editing){
            if(down&&inside(x,y,gearX,gearY,gearSize,gearSize)){editing=true;dragging=-999;return true;}
            return false;
        }
        if(!down){if(dragging==id){dragging=-999;save();}return true;}
        if(dragging==id){
            auto& b=buttons[selected];b.x=x-grabX;b.y=y-grabY;clamp(selected);return true;
        }
        // All editor chrome is intentionally outside the bottom game input area.
        if(inside(x,y,1030,90,205,66)){close();return true;}
        if(inside(x,y,1030,165,205,66)){defaults();return true;}
        if(inside(x,y,1030,245,94,80)){buttons[selected].size-=12;clamp(selected);save();return true;}
        if(inside(x,y,1140,245,94,80)){buttons[selected].size+=12;clamp(selected);save();return true;}
        int i=hit(x,y);
        if(i>=0){selected=i;dragging=id;grabX=x-buttons[i].x;grabY=y-buttons[i].y;}
        return true;
    }
};
}
