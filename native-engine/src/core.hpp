#pragma once
#include "combat.hpp"
#include <cstdint>

namespace eclipse {
class Input {
    enum class Region{None,Move,Attack,Dash,Skill,Swap,Inventory,Weapon,Armor,Charm,Retry};
    struct Pointer{std::int64_t id=0;bool used=false;Region region=Region::None;Actions held;};
    std::array<Pointer,8> pointers{};
    Actions keys{},pending{};
    static bool inside(float x,float y,float bx,float by,float w,float h){return x>=bx&&y>=by&&x<bx+w&&y<by+h;}
    static Region region(float x,float y,bool ended,bool inventory){
        if(ended)return inside(x,y,510,412,260,65)?Region::Retry:Region::None;
        if(inventory&&inside(x,y,46,135,460,340)){
            for(int i=0;i<3;++i)if(inside(x,y,66+i*143.f,413,132,45))return i==0?Region::Weapon:i==1?Region::Armor:Region::Charm;
            return Region::None;
        }
        if(inside(x,y,765,21,288,61))return Region::Inventory;
        if(inside(x,y,35,586,128,128))return Region::Move;
        if(inside(x,y,1110,540,116,116))return Region::Attack;
        if(inside(x,y,985,560,100,100))return Region::Dash;
        if(inside(x,y,995,445,90,90))return Region::Swap;
        if(inside(x,y,1115,420,94,94))return Region::Skill;
        return Region::None;
    }
    static void merge(Actions& a,Actions b){
        a.left|=b.left;a.right|=b.right;a.jump|=b.jump;a.attack|=b.attack;a.dash|=b.dash;a.skill|=b.skill;a.swap|=b.swap;
        a.restart|=b.restart;a.inventory|=b.inventory;a.weapon|=b.weapon;a.armor|=b.armor;a.charm|=b.charm;
    }
    void latch(Actions now,Actions before){
        pending.jump|=now.jump&&!before.jump;pending.attack|=now.attack&&!before.attack;pending.dash|=now.dash&&!before.dash;
        pending.skill|=now.skill&&!before.skill;pending.swap|=now.swap&&!before.swap;pending.restart|=now.restart&&!before.restart;
        pending.inventory|=now.inventory&&!before.inventory;pending.weapon|=now.weapon&&!before.weapon;pending.armor|=now.armor&&!before.armor;pending.charm|=now.charm&&!before.charm;
    }
public:
    void keyboard(Actions next){latch(next,keys);keys=next;}
    void pointer(std::int64_t id,float x,float y,bool down,bool ended=false,bool inventory=false){
        Pointer* slot=nullptr;
        for(auto& p:pointers)if(p.used&&p.id==id){slot=&p;break;}
        if(!down){if(slot)*slot={};return;}
        if(!slot){for(auto& p:pointers)if(!p.used){slot=&p;break;}if(!slot)return;slot->used=true;slot->id=id;slot->region=region(x,y,ended,inventory);}
        Actions now{};
        if(slot->region==Region::Move){float dx=x-99,dy=y-650,length=std::sqrt(dx*dx+dy*dy);if(length>50){dx*=50/length;dy*=50/length;}
            now.left=dx< -50*.23f;now.right=dx>50*.23f;now.jump=dy< -50*.52f;
        }else if(region(x,y,ended,inventory)==slot->region){
            switch(slot->region){case Region::Attack:now.attack=true;break;case Region::Dash:now.dash=true;break;case Region::Skill:now.skill=true;break;
                case Region::Swap:now.swap=true;break;case Region::Inventory:now.inventory=true;break;case Region::Weapon:now.weapon=true;break;
                case Region::Armor:now.armor=true;break;case Region::Charm:now.charm=true;break;case Region::Retry:now.restart=true;break;default:break;}
        }
        latch(now,slot->held);slot->held=now;
    }
    Actions held()const{Actions a=keys;for(const auto& p:pointers)if(p.used)merge(a,p.held);return a;}
    Actions sample(){Actions a=pending,h=held();a.left=h.left;a.right=h.right;pending={};return a;}
    void reset(){keys={};pending={};pointers={};}
};
}
