#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace eclipse {
constexpr double fixedStep = 1.0 / 60.0;
constexpr float viewWidth = 480, viewHeight = 270, floorY = 225;
// Fixed-size touch slots: another finger releasing cannot cancel the movement
// finger. Keyboard and touch have separate owners and combine at sampling time.
struct Actions { bool left=false, right=false, jump=false, attack=false; };
class Input {
    struct Pointer { std::int64_t id=0; bool used=false; Actions held; };
    std::array<Pointer, 8> pointers{};
    Actions previous{};
public:
    Actions keys{};
    void pointer(std::int64_t id, float x, float y, bool down) {
        Pointer* slot=nullptr;
        for(auto& p:pointers) if(p.used && p.id==id) { slot=&p; break; }
        if(!down) { if(slot) *slot={}; return; }
        if(!slot) for(auto& p:pointers) if(!p.used) { slot=&p; break; }
        if(!slot) return;
        slot->used=true; slot->id=id;
        slot->held={x>=6 && x<75 && y>170 && y<270,
                    x>=75 && x<138 && y>170 && y<270,
                    x>185 && x<315 && y>195 && y<270,
                    x>345 && x<480 && y>160 && y<270};
    }
    Actions held() const {
        auto a=keys;
        for(const auto& p:pointers) if(p.used) {
            a.left |= p.held.left; a.right |= p.held.right;
            a.jump |= p.held.jump; a.attack |= p.held.attack;
        }
        return a;
    }
    Actions sample() {
        auto a=held(), output=a;
        output.jump &= !previous.jump; output.attack &= !previous.attack;
        previous=a; return output;
    }
    void reset() { keys={}; previous={}; pointers={}; }
};
struct Actor {
    float x=150,y=floorY-64,vy=0;
    bool grounded=true,faceRight=true;
    int animation=0, frame=0;
    float anim=0,attack=0;
    void step(float dt, Actions input) {
        const float axis=static_cast<float>(input.right)-static_cast<float>(input.left);
        if(axis!=0) faceRight=axis>0;
        x=std::clamp(x+axis*135*dt,0.0f,viewWidth-48);
        if(input.jump && grounded) { vy=-335; grounded=false; }
        vy+=850*dt; y+=vy*dt;
        if(y>=floorY-64) { y=floorY-64; vy=0; grounded=true; }
        attack=std::max(0.0f,attack-dt);
        if(input.attack && attack<=0) attack=.22f;
        // Exact sequential ranges from ECLIPSE's original 48x64 / 8-column atlas.
        const int next=attack>0 ? 4 : !grounded ? (vy<0 ? 2 : 3) : axis!=0 ? 1 : 0;
        if(next!=animation) { animation=next; anim=0; } else anim+=dt;
        constexpr int starts[]={0,6,14,18,21};
        constexpr int counts[]={6,8,4,3,6};
        const int index=static_cast<int>(anim*12);
        const bool loop=next==0 || next==1 || next==3;
        frame=starts[next]+(loop ? index%counts[next] : std::min(index,counts[next]-1));
    }
};
}
