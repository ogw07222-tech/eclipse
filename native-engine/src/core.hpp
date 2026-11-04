#pragma once
#include "combat.hpp"
#include "control_layout.hpp"
#include <cstdint>

namespace eclipse {
class Input {
    enum class Region{None,Move,Attack,Dash,Skill,Sweep,Buff,Swap,Inventory,Weapon,Armor,Charm,Retry};
    struct Pointer{std::int64_t id=0;bool used=false;Region region=Region::None;Actions held;};
    std::array<Pointer,8> pointers{};
    Actions keys{},pending{};
    const ControlLayout* layout_=nullptr;
    ControlLayout fallbackLayout_{};
    float stickX_=0,stickY_=0;
    static bool inside(float x,float y,float bx,float by,float w,float h){return x>=bx&&y>=by&&x<bx+w&&y<by+h;}
    Region region(float x,float y,bool ended,bool inventory) const {
        if(ended)return inside(x,y,510,412,260,65)?Region::Retry:Region::None;
        if(inventory&&inside(x,y,46,135,460,340)){
            for(int i=0;i<3;++i)if(inside(x,y,66+i*143.f,413,132,45))return i==0?Region::Weapon:i==1?Region::Armor:Region::Charm;
            return Region::None;
        }
        if(inside(x,y,765,21,288,61))return Region::Inventory;
        {
            const auto& b=(layout_?layout_:&fallbackLayout_)->buttons;
            const Region regions[]={Region::Move,Region::Attack,Region::Dash,Region::Swap,Region::Skill,Region::Sweep,Region::Buff};
            for(int i=ControlLayout::Count-1;i>=0;--i)
                if(inside(x,y,b[i].x,b[i].y,b[i].size,b[i].size))return regions[i];
        }
        return Region::None;
    }
    static void merge(Actions& a,Actions b){
        a.jumpHeld|=b.jumpHeld;a.left|=b.left;a.right|=b.right;a.jump|=b.jump;a.attack|=b.attack;a.dash|=b.dash;a.skill|=b.skill;a.skillSweep|=b.skillSweep;a.skillBuff|=b.skillBuff;a.swap|=b.swap;
        a.restart|=b.restart;a.inventory|=b.inventory;a.weapon|=b.weapon;a.armor|=b.armor;a.charm|=b.charm;
    }
    void latch(Actions now,Actions before){
        pending.jump|=now.jump&&!before.jump;pending.attack|=now.attack&&!before.attack;pending.dash|=now.dash&&!before.dash;
        pending.skill|=now.skill&&!before.skill;
        pending.skillSweep|=now.skillSweep&&!before.skillSweep;pending.skillBuff|=now.skillBuff&&!before.skillBuff;pending.swap|=now.swap&&!before.swap;pending.restart|=now.restart&&!before.restart;
        pending.inventory|=now.inventory&&!before.inventory;pending.weapon|=now.weapon&&!before.weapon;pending.armor|=now.armor&&!before.armor;pending.charm|=now.charm&&!before.charm;
    }
public:
    void setLayout(const ControlLayout* layout){layout_=layout;}
    std::array<float,2> stickOffset()const{return {stickX_,stickY_};}
    void keyboard(Actions next){latch(next,keys);keys=next;}
    void pointer(std::int64_t id,float x,float y,bool down,bool ended=false,bool inventory=false){
        Pointer* slot=nullptr;
        for(auto& p:pointers)if(p.used&&p.id==id){slot=&p;break;}
        if(!down){if(slot){if(slot->region==Region::Move){stickX_=stickY_=0;}*slot={};}return;}
        if(!slot){for(auto& p:pointers)if(!p.used){slot=&p;break;}if(!slot)return;slot->used=true;slot->id=id;slot->region=region(x,y,ended,inventory);}
        Actions now{};
        if(slot->region==Region::Move){const auto& b=(layout_?layout_:&fallbackLayout_)->buttons[ControlLayout::Stick];
            float dx=x-(b.x+b.size*.5f),dy=y-(b.y+b.size*.5f),radius=b.size*.39f,length=std::sqrt(dx*dx+dy*dy);
            if(length>radius){dx*=radius/length;dy*=radius/length;}
            stickX_=dx;stickY_=dy;
            now.left=dx< -radius*.23f;now.right=dx>radius*.23f;now.jump=dy< -radius*.52f;now.jumpHeld=now.jump;
        }else if(region(x,y,ended,inventory)==slot->region){
            switch(slot->region){case Region::Attack:now.attack=true;break;case Region::Dash:now.dash=true;break;case Region::Skill:now.skill=true;break;
                case Region::Sweep:now.skillSweep=true;break;case Region::Buff:now.skillBuff=true;break;
                case Region::Swap:now.swap=true;break;case Region::Inventory:now.inventory=true;break;case Region::Weapon:now.weapon=true;break;
                case Region::Armor:now.armor=true;break;case Region::Charm:now.charm=true;break;case Region::Retry:now.restart=true;break;default:break;}
        }
        latch(now,slot->held);slot->held=now;
    }
    Actions held()const{Actions a=keys;for(const auto& p:pointers)if(p.used)merge(a,p.held);return a;}
    Actions sample(){Actions a=pending,h=held();a.left=h.left;a.right=h.right;a.jumpHeld=h.jumpHeld;pending={};return a;}
    void reset(){keys={};pending={};pointers={};stickX_=stickY_=0;}
};
}
