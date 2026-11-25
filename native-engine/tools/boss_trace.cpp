// SDL-free trace of the Demon King's moves, straight from the real Combat logic.
// Writes one JSON line per rendered frame (30 fps) so a renderer can replay the exact motion choice,
// sheet frame, effect schedule and hazards the engine would produce.
//   c++ -std=c++17 native-engine/tools/boss_trace.cpp -o boss_trace && ./boss_trace trace.jsonl
#include "../src/core.hpp"
#include <cstdio>
#include <string>

using namespace eclipse;

namespace {
struct Seg {const char* name;int kind;BossMove move;float tail;};
const Seg segs[]={
    {"Smash",DemonKingArmored,BossMove::Smash,1.4f},
    {"Quake - jump the waves",DemonKingArmored,BossMove::Quake,2.4f},
    {"Leap slam",DemonKingArmored,BossMove::LeapSlam,1.4f},
    {"Judgement - void pillars",DemonKingArmored,BossMove::Judgement,1.6f},
    {"Iron sweep - jump it",DemonKingArmored,BossMove::IronSweep,1.2f},
    {"Juggernaut rush",DemonKingArmored,BossMove::JuggernautRush,1.4f},
    {"Dash combo",DemonKingTrue,BossMove::DashCombo,1.2f},
    {"Blink strike",DemonKingTrue,BossMove::BlinkStrike,1.2f},
    {"Pillar chain",DemonKingTrue,BossMove::PillarChain,1.4f},
    {"Slash burst",DemonKingTrue,BossMove::SlashBurst,1.4f},
    {"Wave volley",DemonKingTrue,BossMove::WaveVolley,1.6f},
    {"Void rain",DemonKingTrue,BossMove::VoidRain,1.6f},
    {"Aerial dive",DemonKingTrue,BossMove::AerialDive,1.4f},
    {"Execution combo",DemonKingTrue,BossMove::ExecutionCombo,1.4f},
};
}

int main(int argc,char** argv){
    FILE* out=std::fopen(argc>1?argv[1]:"boss_trace.jsonl","w");
    if(!out)return 1;
    for(const auto& seg:segs){
        Combat g;g.scriptedSpawns=true;g.endOnClear=false;g.suppressGameOver=true;g.playerHealthFloor=1;
        g.configureCastleArea(5);g.spawn(seg.kind,1000,0);
        Enemy& king=g.enemies.back();king.attackTimer=1000;
        king.wake=0;
        g.player.x=420;g.player.invulnerable=0;g.player.equippedJobs={{1,2}};
        g.bossStartMove(king,seg.move);
        float hurt=0;int lastHealth=g.player.health;
        const float dt=1.f/60;int frame=0;bool finished=false;float tailLeft=seg.tail;
        for(int n=0;n<60*14;++n){
            // A small dodging player so the preview shows both hits and clean dodges.
            Actions a{};
            float danger=0;
            for(const auto& h:g.hazards){
                if(h.kind==HazardKind::Wave&&!h.spent&&std::abs(h.x-g.player.x)<230&&(h.x-g.player.x)*h.dir<0)a.jump=true;
                if(h.kind==HazardKind::Pillar&&!h.detonated&&h.age>h.delay-1.0f&&std::abs(h.x-g.player.x)<h.radius+60)danger=h.x<g.player.x?1.f:-1.f;
            }
            if(danger>0)a.right=true;else if(danger<0)a.left=true;
            if(seg.move==BossMove::IronSweep&&king.attackAge>.5f&&king.attackAge<.8f)a.jump=true;
            g.player.invulnerable=0;
            g.step(dt,a);
            if(g.player.health<lastHealth)hurt=.4f;
            lastHealth=g.player.health;g.player.health=g.player.maxHealth();lastHealth=g.player.health;
            hurt=std::max(0.f,hurt-dt);
            if(n%2==0){
                std::fprintf(out,"{\"seg\":\"%s\",\"f\":%d,\"kind\":%d,\"boss\":{\"x\":%.1f,\"y\":%.1f,\"facing\":%d,\"attacking\":%d,\"motion\":%d,\"frame\":%d,\"move\":%d,\"step\":%d,\"age\":%.3f,\"flash\":%.2f,\"blink\":%d,\"act\":%d,\"contact\":%.3f,\"dur\":%.3f,\"toX\":%.1f},",
                    seg.name,frame,seg.kind,king.x,king.y,king.facing,king.attackAge>=0?1:0,king.motion,enemyFrame(king),
                    static_cast<int>(king.move),king.step,king.attackAge,king.hitFlash,
                    (king.move!=BossMove::None&&bossStepDef(king.move,king.step).action==BossAction::Blink&&king.attackAge<.16f)?1:0,
                    king.move!=BossMove::None?static_cast<int>(bossStepDef(king.move,king.step).action):-1,king.moveContact,king.moveDur,king.attackToX);
                std::fprintf(out,"\"player\":{\"x\":%.1f,\"y\":%.1f,\"hurt\":%.2f},\"shake\":%.2f,\"flash\":%.2f,\"fx\":[",g.player.x,g.player.y,hurt,g.shake,g.flash);
                bool first=true;
                for(const auto& fx:g.bossFx){
                    std::fprintf(out,"%s{\"k\":%d,\"x\":%.1f,\"y\":%.1f,\"face\":%d,\"age\":%.3f,\"s\":%.2f,\"life\":%.2f}",first?"":",",static_cast<int>(fx.kind),fx.x,fx.y,fx.facing,fx.age,fx.scale,fx.life);first=false;
                }
                std::fprintf(out,"],\"haz\":[");first=true;
                for(const auto& h:g.hazards){
                    std::fprintf(out,"%s{\"w\":%d,\"fx\":%d,\"x\":%.1f,\"y\":%.1f,\"sx\":%.1f,\"age\":%.3f,\"delay\":%.2f,\"r\":%.0f,\"dir\":%d,\"speed\":%.0f,\"range\":%.0f}",
                        first?"":",",h.kind==HazardKind::Wave?1:0,static_cast<int>(h.fx),h.x,h.y,h.startX,h.age,h.delay,h.radius,h.dir,h.speed,h.range);first=false;
                }
                std::fprintf(out,"]}\n");
                ++frame;
            }
            if(king.move==BossMove::None&&king.attackAge<0)finished=true;
            if(finished){tailLeft-=dt;if(tailLeft<=0)break;}
        }
    }
    std::fclose(out);
    return 0;
}
