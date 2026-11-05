#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include <limits>
#include <random>
#include "motion.hpp"
#include "forest_layers.hpp"
#include "slime_motion.hpp"
#include "monster_collision.hpp"

namespace eclipse {
constexpr double fixedStep=1.0/60.0;
constexpr float viewWidth=1280,viewHeight=720,floorY=ForestLayerScene::floorY,worldWidth=1600;
struct Actions {
    bool left=false,right=false,jump=false,attack=false,dash=false,skill=false,swap=false;
    bool restart=false,inventory=false,weapon=false,armor=false,charm=false;
    bool jumpHeld=false,skillSweep=false,skillBuff=false;
};
struct Equipment {
    std::array<int,3> selected{};
    int attack()const{return (selected[0]==1?6:selected[0]==2?-2:0)+(selected[2]==1?3:0);}
    int defense()const{return (selected[1]==1?4:selected[1]==2?1:0)+(selected[2]==2?2:0);}
    int hp()const{return (selected[1]==1?30:selected[1]==2?10:0)+(selected[2]==2?15:0);}
    float speed()const{return (selected[0]==1?.88f:selected[0]==2?1.3f:1)*(selected[1]==1?.9f:selected[1]==2?1.1f:1);}
    float range()const{return selected[0]==1?1.16f:selected[0]==2?1.08f:1;}
    void cycle(int slot){if(slot>=0&&slot<3)selected[slot]=(selected[slot]+1)%3;}
    const char* name(int slot)const{
        static constexpr const char* names[3][3]={{"RUSTY SWORD","BLOOD BLADE","QUICK BLADE"},{"CLOTH ARMOR","IRON ARMOR","SCOUT ARMOR"},{"NO CHARM","FLAME CHARM","GUARDIAN CHARM"}};
        return names[slot][selected[slot]];
    }
};
constexpr float thrustCooldown=6.f,sweepCooldown=9.f,buffCooldown=14.f;
constexpr float preparationDuration=5.f;
constexpr int bleedMaxStacks=10,bleedSlowStacks=5;
// Every 3 seconds one stack expires; reapplying bleed resets that countdown.
constexpr float bleedDuration=3.f,bleedTickInterval=.75f;
enum class SwordStrike {Basic,Thrust,Sweep};

enum class WhiteMove { None, Slash, Rush, Command, Blink, Verdict };
struct WhiteClip {int start,count;float duration,contact;};
constexpr WhiteClip whiteClips[]={{0,1,1,0},{16,15,.46f,.184f},{31,12,.50f,.125f},
    {43,12,.46f,.23f},{55,12,.36f,.15f},{67,20,1.12f,.728f}};
struct Player {
    Equipment gear;
    float x=140,y=floorY,vx=0,vy=0;
    int health=100,form=0,facing=1,combo=0,attackSerial=0,hurtSerial=0,landingSerial=0;
    int storedLightLances=0;
    bool grounded=true,whiteNight=false;
    WhiteMove whiteMove=WhiteMove::None;
    float whiteAge=0,whiteBoost=0;
    size_t whiteTarget=0;
    unsigned whiteRushHits=0;
    float takeoffLock=0;
    float dashLeft=0,dashCooldown=0,attackCooldown=0,skillCooldown=0,swapCooldown=0;
    float invulnerable=0,coyote=0,jumpBuffer=0,attackFlash=0,comboWindow=0,hitStop=0,swapFlash=0;
    float trailingHealth=-1.f,healthTrailHold=0.f;
    float hurtTint=0; // visual-only: short white flash and muted-red impact
    bool airSlam=false;
    float slamAge=0,slamRecover=0,slamImpact=0,slamImpactX=0,slamImpactY=floorY;
    float mageCastLeft=0;
    int mageCastDamage=0;
    float meleeLeft=0,meleeReach=0;
    int meleeDamage=0,meleeBleed=0,attackPose=0;
    SwordStrike strike=SwordStrike::Basic;
    float sweepReady=0,buffReady=0,mageSkillCooldown=0,preparation=0;
    // Class archetypes: ranged 80%, melee 100%, assassin 125% of base run speed.
    // Keep this separate from attack speed and dash/skill movement.
    float movementMultiplier() const {return whiteNight?1.25f:form==1?.80f:1.f;}
    float basicSpeed() const {return gear.speed()*(preparation>0?1.20f:1.f);}
    int maxHealth()const{return 100+gear.hp();}
    void equip(int slot){
        const int before=maxHealth();
        gear.cycle(slot);
        const int previousHealth=health;
        health=std::clamp(health+maxHealth()-before,1,maxHealth());
        if(trailingHealth>=0)trailingHealth=std::clamp(trailingHealth+health-previousHealth,float(health),float(maxHealth()));
    }
};
struct Enemy {
    int kind=0,health=55,maxHealth=55,damage=9;
    float x=0,y=floorY,speed=95,attackTimer=0,bleed=0,bleedTick=0,burn=0,burnTick=0,hitFlash=0,reactionFlash=0,knockback=0;
    float travelPhase=0,attackAge=-1,attackFromX=0,attackToX=0,stagger=0;
    // Visual-only delayed damage segment. Actual health always changes immediately.
    float trailingHealth=-1.f,healthTrailHold=0.f;
    int facing=-1;
    int bleedStacks=0;
    float lightMark=0;
    bool boss=false;
    float knockbackResistance() const {return boss?1.f:slimeSpec(kind).knockbackResistance;}
    Enemy(int type=0,float at=0):kind(std::clamp(type,0,2)),health(slimeSpec(kind).health),maxHealth(health),damage(slimeSpec(kind).damage),x(at),speed(slimeSpec(kind).speed){}
    bool dead()const{return health<=0;}
};
struct Projectile {float x,y;int facing,damage;bool empowered;float life=1.6f,remainingDrop=0;};
struct LightLance {size_t target;float x,y,life=.65f;float offset=0,angle=90;bool verdict=false,seeking=false,markOnHit=false;float delay=0;};
enum class LightFx { Impact, Slash, Rush, Command, Depart, Arrive, Verdict };
struct LightBurst {float x,y,life=.24f;LightFx kind=LightFx::Impact;int facing=1;};
struct RiftScar {float x,y,angle,life=.16f;bool slash=false;};
enum class DamageKind{Normal,Fire,Bleed,Execute};
struct BleedSpread {float fromX,fromY,toX,toY,life=.23f;};
struct Popup{float x,y;int amount;DamageKind kind;float age=0;};
// Horizontal one-way surfaces share the same descending foot-contact rule.
struct GroundSurface {
    float left,right,y;
    bool covers(float x) const {return x>=left&&x<=right;}
};

class Combat {
    float surfaceBelow(float x,float y) const {
        float nearest=y<=floorY?floorY:std::numeric_limits<float>::infinity();
        for(const auto& surface:platforms)
            if(surface.covers(x)&&surface.y>=y)nearest=std::min(nearest,surface.y);
        return nearest;
    }
    float crossedGround(float x,float y,float nextX,float nextY) const {
        if(nextY<y)return std::numeric_limits<float>::infinity();
        float contact=y<=floorY&&nextY>=floorY?floorY:std::numeric_limits<float>::infinity();
        for(const auto& surface:platforms){
            if(surface.y<y||surface.y>nextY||surface.y>=contact)continue;
            const float t=surface.y>y?(surface.y-y)/(nextY-y):1.f;
            if(surface.covers(x+(nextX-x)*t))contact=surface.y;
        }
        return contact;
    }
    void launchJump() {
        auto& p=player;
        p.grounded=false;
        p.vy=-650;
        p.coyote=p.jumpBuffer=p.meleeLeft=0;
        p.takeoffLock=.085f;
    }
    bool canAttack() const {
        const auto& p=player;
        if(p.takeoffLock>0)return false;
        if(p.grounded)return true;
        const float clearance=surfaceBelow(p.x,p.y)-p.y;
        // Leave room to show the slam wind-up before reaching a surface.
        return clearance>=std::max(48.f,std::max(0.f,p.vy)*.10f+6.25f);
    }
    static float down(float value,float dt){return std::max(0.f,value-dt);}
    static float toward(float value,float step){return value>0?std::max(0.f,value-step):std::min(0.f,value+step);}
    template<class Character>
    static void holdHealthTrail(Character& character,float hold){
        character.trailingHealth=std::max(character.trailingHealth,float(character.health));
        character.healthTrailHold=std::max(character.healthTrailHold,hold);
    }
    template<class Character>
    static void stepHealthTrail(Character& character,int maxHealth,float dt){
        if(character.trailingHealth<0)character.trailingHealth=float(character.health);
        character.healthTrailHold=down(character.healthTrailHold,dt);
        if(character.healthTrailHold<=0)
            character.trailingHealth=std::max(float(character.health),character.trailingHealth-maxHealth*dt/.20f);
    }
    void damageEnemy(Enemy& e,int amount,DamageKind kind,bool dot){
        if(e.dead())return;
        holdHealthTrail(e,dot?.35f:.12f);
        e.health=std::max(0,e.health-amount);if(!dot)e.hitFlash=.18f;
        popups.push_back({e.x,e.y-86,amount,kind});
    }
    // Basic hits build a visible stock of light lances instead of firing automatically.
    void chargeLightLance(){
        if(player.whiteNight)player.storedLightLances=std::min(5,player.storedLightLances+1);
    }
    void releaseStoredLances(){
        auto& p=player;
        if(p.storedLightLances<=0)return;
        std::array<size_t,4> candidates{};
        size_t count=0;
        const float left=cameraCenter-viewWidth*.5f;
        for(size_t i=0;i<enemies.size();++i)
            if(!enemies[i].dead()&&enemies[i].x>=left&&enemies[i].x<=left+viewWidth)candidates[count++]=i;
        if(count==0)return;
        static std::mt19937 random{std::random_device{}()};
        const int total=p.storedLightLances;
        for(int i=0;i<total;++i){
            const size_t target=candidates[std::uniform_int_distribution<size_t>{0,count-1}(random)];
            const float spread=(i-(total-1)*.5f)*16.f;
            lightLances.push_back({target,p.x+p.facing*48.f,p.y-65.f+spread,1.3f,spread,0.f,false,true,true,i*.035f});
        }
        p.storedLightLances=0;
    }
    // The ultimate releases new lances from the off-hand light sphere, independent of stored charges.
    void launchVerdictLances(){
        auto& p=player;
        for(size_t target=0;target<enemies.size();++target){
            const auto& e=enemies[target];
            if(e.dead()||e.lightMark<=0)continue;
            for(int i=0;i<3;++i){
                const float spread=(i-1.f)*28.f;
                lightLances.push_back({target,p.x-p.facing*24.f,p.y-57.f+spread*.18f,1.3f,spread,0.f,true,true,false,i*.028f});
            }
        }
    }
    void melee(int damage,int bleed,float reach){
        const BodyRect strike=player.facing>0?BodyRect{player.x-9,player.y-102,player.x+reach,player.y+2}:
            BodyRect{player.x-reach,player.y-102,player.x+9,player.y+2};
        bool connected=false;
        for(auto& e:enemies){
            if(e.dead()||!enemyOverlaps(e,strike))continue;
            hit(e,damage,false,bleed);player.hitStop=std::max(player.hitStop,damage<20?.045f:.075f);
            connected=true;
        }
        if(connected)chargeLightLance();
    }
    static void addBleed(Enemy& e,int stacks) {
        if(e.dead()||stacks<=0)return;
        if(e.bleedStacks==0)e.bleedTick=0;
        e.bleedStacks=std::min(bleedMaxStacks,e.bleedStacks+stacks);
        e.bleed=bleedDuration;
    }
    void resolveSwordSkill() {
        auto& p=player;
        if(p.strike==SwordStrike::Thrust){
            const float reach=p.meleeReach;
            const BodyRect strike=p.facing>0?BodyRect{p.x,p.y-78,p.x+reach,p.y-29}:
                BodyRect{p.x-reach,p.y-78,p.x,p.y-29};
            Enemy* nearest=nullptr;
            float best=std::numeric_limits<float>::infinity();
            for(auto& e:enemies){
                if(e.dead()||!enemyOverlaps(e,strike))continue;
                const auto body=enemyBounds(e);
                const float edge=std::max(0.f,p.facing>0?body.left-p.x:p.x-body.right);
                if(edge<best){best=edge;nearest=&e;}
            }
            if(nearest){
                hit(*nearest,p.meleeDamage,false,2);
                p.hitStop=.075f;
                chargeLightLance();
            }
            return;
        }

        const float reach=p.meleeReach,centerY=p.y-43;
        std::array<bool,4> sources{};
        bool connected=false;
        for(size_t i=0;i<enemies.size();++i){
            auto& e=enemies[i];
            if(e.dead())continue;
            const bool intersects=enemyAnyBand(e,[&](BodyRect band){
                if(p.facing>0?band.right<=p.x:band.left>=p.x)return false;
                const float dx=std::max({0.f,band.left-p.x,p.x-band.right})/reach;
                const float dy=std::max({0.f,band.top-centerY,centerY-band.bottom})/58.f;
                return dx*dx+dy*dy<=1.f;
            });
            if(!intersects)continue;
            connected=true;
            sources[i]=e.bleedStacks>0;
            hit(e,p.meleeDamage,false);
            p.hitStop=.065f;
        }
        if(connected)chargeLightLance();
        // Snapshot sources before applying any spread so newly affected enemies cannot relay it.
        for(size_t i=0;i<enemies.size();++i){
            auto& target=enemies[i];
            if(target.dead())continue;
            for(size_t j=0;j<enemies.size();++j){
                if(i==j||!sources[j])continue;
                const auto& source=enemies[j];
                if(std::hypot(target.x-source.x,target.y-source.y)>110.f)continue;
                addBleed(target,1);
                bleedSpreads.push_back({source.x,source.y-42,target.x,target.y-42});
                break;
            }
        }
    }
    void projectile(int damage,bool empowered){projectiles.push_back({player.x+player.facing*27,player.y-33,player.facing,damage,empowered});}
    void cancelMageCast(){player.mageCastLeft=0;player.mageCastDamage=0;}
    void cancelAirSlam(){player.airSlam=false;player.slamAge=0;player.slamRecover=0;}
    void landAirSlam(){
        auto& p=player;p.airSlam=false;p.slamRecover=airSlamRecovery;
        p.slamImpact=airSlamImpactDuration;
        p.slamImpactX=p.x+p.facing*(slamContactTipX-motionPivotX)*motionScale;
        p.slamImpactY=p.y+(slamContactTipY-motionPivotY)*motionScale;
        const float radius=airSlamRadius*p.gear.range();
        const BodyRect strike{p.slamImpactX-radius,p.slamImpactY-44,p.slamImpactX+radius,p.slamImpactY+8};
        bool connected=false;
        for(auto& e:enemies){if(e.dead()||!enemyOverlaps(e,strike))continue;
            hit(e,24+p.gear.attack(),false,0,true);connected=true;}
        if(connected)chargeLightLance();
        p.hitStop=std::max(p.hitStop,connected?.065f:.025f);
    }
    void basic(){
        auto& p=player;p.attackFlash=.16f;p.attackPose=0;++p.attackSerial;
        if(p.form==0&&!p.grounded){
            p.airSlam=true;p.slamAge=0;p.slamRecover=0;p.combo=0;p.comboWindow=0;p.meleeLeft=0;
            p.jumpBuffer=p.coyote=0;p.attackCooldown=.55f/p.gear.speed();return;
        }
        if(p.form==0){p.strike=SwordStrike::Basic;p.combo=p.comboWindow>0?p.combo%3+1:1;p.comboWindow=.8f;
            p.attackCooldown=(p.combo==1?.23f:p.combo==2?.25f:.42f)/p.basicSpeed();
            const auto clip=motionClips[3+p.combo];p.meleeLeft=clip.duration*clip.contact/p.basicSpeed();
            p.meleeDamage=(p.combo==1?12:p.combo==2?15:23)+p.gear.attack();p.meleeBleed=p.combo==3?2:0;p.meleeReach=(p.combo==3?115:96)*p.gear.range();
        }else{p.combo=0;p.attackCooldown=.47f;p.mageCastLeft=mageCastDuration;p.mageCastDamage=11+p.gear.attack();}
    }
    void swordSkill(SwordStrike strike) {
        auto& p=player;
        const bool thrust=strike==SwordStrike::Thrust;
        cancelAirSlam();
        p.strike=strike;
        p.attackPose=thrust?thrustSkillState:sweepSkillState;
        const auto clip=motionClips[p.attackPose];
        p.meleeLeft=clip.duration*clip.contact;
        p.meleeReach=(thrust?120.f:125.f)*p.gear.range();
        p.meleeDamage=(thrust?32:18)+p.gear.attack();
        p.attackCooldown=std::max(p.attackCooldown,clip.duration);
        p.attackFlash=.28f;
        ++p.attackSerial;
        if(thrust)p.skillCooldown=thrustCooldown;
        else p.sweepReady=sweepCooldown;
    }
    size_t farthestLightMark()const{
        size_t target=enemies.size();
        float distance=-1;
        const float left=cameraCenter-viewWidth*.5f;
        for(size_t i=0;i<enemies.size();++i){
            const auto& e=enemies[i];
            const float dx=std::abs(e.x-player.x);
            if(!e.dead()&&e.lightMark>0&&e.x>=left&&e.x<=left+viewWidth&&dx>distance){
                target=i;distance=dx;
            }
        }
        return target;
    }
    void whiteEffect(LightFx kind,float x,float y){
        lightBursts.push_back({x,y,.24f,kind,player.facing});
    }
    bool stepWhiteMove(float dt,const Actions& a){
        auto& p=player;
        if(p.whiteMove==WhiteMove::None){
            if(!p.grounded||p.dashLeft>0||p.slamRecover>0||!canAttack())return false;
            if(a.skill&&p.skillCooldown<=0){p.whiteMove=WhiteMove::Rush;p.skillCooldown=3.f;}
            else if(a.skillSweep&&p.sweepReady<=0&&p.storedLightLances>0){p.whiteMove=WhiteMove::Command;p.sweepReady=1.f;}
            else if((a.skillBuff&&p.buffReady<=0)||(a.swap&&p.swapCooldown<=0)){
                p.whiteTarget=farthestLightMark();
                if(p.whiteTarget==enemies.size())return false;
                if(a.skillBuff&&p.buffReady<=0){p.whiteMove=WhiteMove::Blink;p.buffReady=1.5f;}
                else {p.whiteMove=WhiteMove::Verdict;p.swapCooldown=9.f;}
            }else if(a.attack&&p.attackCooldown<=0){p.whiteMove=WhiteMove::Slash;p.attackCooldown=.46f/p.basicSpeed();}
            else return false;
            p.whiteAge=0;p.whiteRushHits=0;p.vx=0;p.vy=0;
        }
        const auto move=p.whiteMove;
        const auto clip=whiteClips[static_cast<int>(move)];
        const float speed=move==WhiteMove::Slash?p.basicSpeed():1.f;
        const float old=p.whiteAge;
        p.whiteAge=std::min(clip.duration,old+dt*speed);
        if(old<clip.contact&&p.whiteAge>=clip.contact){
            if(move==WhiteMove::Slash){
                whiteEffect(LightFx::Slash,p.x+p.facing*42,p.y-48);
                riftScars.push_back({p.x+p.facing*100.f,p.y-52.f,p.facing>0?-14.f:194.f,.21f,true});
                melee(15+p.gear.attack(),0,105*p.gear.range());
            }else if(move==WhiteMove::Command){
                // The forward-pointing sword pose is the shared release frame for the entire volley.
                releaseStoredLances();whiteEffect(LightFx::Command,p.x+p.facing*52.f,p.y-55.f);
            }else if(move==WhiteMove::Blink&&!enemies[p.whiteTarget].dead()){
                auto& e=enemies[p.whiteTarget];
                whiteEffect(LightFx::Depart,p.x,p.y-48);
                const float side=e.x>=p.x?1.f:-1.f;
                p.x=std::clamp(e.x+side*58,25.f,1550.f);p.y=floorY;
                p.facing=e.x>=p.x?1:-1;
                whiteEffect(LightFx::Arrive,p.x,p.y-48);
            }else if(move==WhiteMove::Verdict){
                // A clenched off-hand releases the accumulated light from its sphere.
                whiteEffect(LightFx::Arrive,p.x-p.facing*24.f,p.y-57.f);
                for(const auto& e:enemies)
                    if(!e.dead()&&e.lightMark>0)whiteEffect(LightFx::Verdict,e.x,e.y-45);
                launchVerdictLances();
            }
        }
        if(move==WhiteMove::Rush){
            const float travel=std::max(0.f,std::min(.36f,p.whiteAge)-std::max(clip.contact,old));
            if(travel>0){
                const float from=p.x;
                p.x=std::clamp(from+p.facing*1080*travel,25.f,1550.f);
                if(static_cast<int>(old*60)%2==0)whiteEffect(LightFx::Rush,p.x,p.y-47);
                const BodyRect strike{std::min(from,p.x)-18,p.y-83,std::max(from,p.x)+18,p.y+2};
                for(size_t i=0;i<enemies.size();++i){
                    auto& e=enemies[i];
                    if(e.dead()||(p.whiteRushHits&(1u<<i))||!enemyOverlaps(e,strike))continue;
                    p.whiteRushHits|=1u<<i;
                    hit(e,18+p.gear.attack(),false);
                    whiteEffect(LightFx::Impact,e.x,e.y-38);
                }
            }
        }
        if(p.whiteAge>=clip.duration)p.whiteMove=WhiteMove::None;
        return true;
    }
    void stepPlayer(float dt,Actions a){
        auto& p=player;
        p.dashLeft=down(p.dashLeft,dt);p.dashCooldown=down(p.dashCooldown,dt);p.attackCooldown=down(p.attackCooldown,dt);
        p.skillCooldown=down(p.skillCooldown,dt);
        p.mageSkillCooldown=down(p.mageSkillCooldown,dt);
        p.sweepReady=down(p.sweepReady,dt);
        p.buffReady=down(p.buffReady,dt);
        p.preparation=down(p.preparation,dt);p.swapCooldown=down(p.swapCooldown,dt);p.invulnerable=down(p.invulnerable,dt);
        p.attackFlash=down(p.attackFlash,dt);p.comboWindow=down(p.comboWindow,dt);p.swapFlash=down(p.swapFlash,dt);
        p.whiteBoost=down(p.whiteBoost,dt);
        if(p.hitStop>0){p.hitStop=down(p.hitStop,dt);return;}
        if(p.whiteNight&&stepWhiteMove(dt,a))return;
        if(p.whiteNight){
            a.skill=a.skillSweep=a.skillBuff=a.swap=false;
            if(p.grounded)a.attack=false;
        }
        p.takeoffLock=down(p.takeoffLock,dt);
        p.slamRecover=down(p.slamRecover,dt);p.slamImpact=down(p.slamImpact,dt);
        if(p.comboWindow<=0)p.combo=0;
        p.coyote=p.grounded?.11f:down(p.coyote,dt);p.jumpBuffer=a.jump?.13f:down(p.jumpBuffer,dt);
        float axis=static_cast<float>(a.right)-static_cast<float>(a.left);
        if(axis!=0&&!p.airSlam&&p.slamRecover<=0&&(p.meleeLeft<=0||p.strike==SwordStrike::Basic))p.facing=axis>0?1:-1;
        if(a.dash&&p.dashCooldown<=0&&p.slamRecover<=0){cancelAirSlam();cancelMageCast();p.meleeLeft=0;p.dashLeft=.15f;p.dashCooldown=.85f;p.invulnerable=std::max(p.invulnerable,.15f);}
        if(p.dashLeft>0){p.vx=780*p.facing*p.gear.speed();p.vy=0;}
        else {
            p.vx=axis*280*p.gear.speed()*p.movementMultiplier();
            p.vy+=1250*dt;
            if((p.jumpBuffer>0||(a.jumpHeld&&p.grounded))&&p.coyote>0&&!p.airSlam&&p.slamRecover<=0)
                launchJump();
        }
        const bool attackAllowed=canAttack();
        if(a.swap&&!p.whiteNight&&p.swapCooldown<=0&&p.slamRecover<=0){
            cancelAirSlam();cancelMageCast();p.meleeLeft=0;p.form=1-p.form;p.swapCooldown=1;
            p.attackFlash=.22f;p.combo=0;p.comboWindow=0;p.attackPose=0;p.swapFlash=.24f;
            if(attackAllowed){
                ++p.attackSerial;
                if(p.form==0)melee(13,false,112);else projectile(14,false);
            }
        }
        if(attackAllowed&&a.skillBuff&&p.form==0&&p.buffReady<=0&&p.grounded&&p.dashLeft<=0&&p.slamRecover<=0){
            p.preparation=preparationDuration;
            p.buffReady=buffCooldown;
            if(p.meleeLeft<=0){
                p.attackPose=prepSkillState;
                ++p.attackSerial;
            }
        }
        if(attackAllowed&&a.attack&&p.attackCooldown<=0&&p.dashLeft<=0&&!p.airSlam&&p.slamRecover<=0)basic();
        if(attackAllowed&&p.dashLeft<=0&&p.slamRecover<=0){
            if(p.form==0){
                const bool windingUp=p.meleeLeft>0&&p.strike!=SwordStrike::Basic;
                if(!windingUp&&a.skill&&p.skillCooldown<=0){
                    if(p.grounded)swordSkill(SwordStrike::Thrust);
                    else if(p.airSlam)cancelAirSlam();
                }
                else if(!windingUp&&a.skillSweep&&p.sweepReady<=0&&p.grounded)swordSkill(SwordStrike::Sweep);
            }else if(a.skill&&p.mageSkillCooldown<=0){
                cancelMageCast();
                p.meleeLeft=0;p.attackPose=0;p.attackFlash=.28f;++p.attackSerial;
                p.mageSkillCooldown=4.1f;
                projectile(25+p.gear.attack(),true);
            }
        }
        if(p.airSlam){p.slamAge+=dt;p.vx=axis*120.f;
            if(p.slamAge>=airSlamWindup)p.vy=std::max(p.vy,airSlamDropSpeed);}
        else if(p.slamRecover>0)p.vx=0;
        else if(p.meleeLeft>0&&p.strike!=SwordStrike::Basic)
            p.vx=p.strike==SwordStrike::Thrust?p.facing*65.f:0.f;

        const float nextX=std::clamp(p.x+p.vx*dt,25.f,1550.f);
        const float nextY=p.y+p.vy*dt;
        const float contact=crossedGround(p.x,p.y,nextX,nextY);
        const bool landed=std::isfinite(contact)&&(!p.grounded||p.y!=contact);
        if(p.grounded&&!std::isfinite(contact))p.meleeLeft=0;
        p.x=nextX;
        p.grounded=std::isfinite(contact);
        p.y=p.grounded?contact:nextY;
        if(p.grounded){
            p.vy=0;
            if(landed){
                ++p.landingSerial;
                if(p.airSlam)landAirSlam();
                if((a.jumpHeld||p.jumpBuffer>0)&&p.dashLeft<=0&&p.slamRecover<=0)
                    launchJump();
            }
        }
        if(p.mageCastLeft>0){
            p.mageCastLeft=down(p.mageCastLeft,dt);
            if(p.mageCastDamage>0&&p.mageCastLeft<=mageCastDuration-mageCastReleaseAge){
                const float tipOffsetY=(mageCastTipY-motionPivotY)*motionScale;
                projectiles.push_back({p.x+p.facing*(mageCastTipX-motionPivotX)*motionScale,
                                       p.y+tipOffsetY,p.facing,p.mageCastDamage,false});
                // Ground casts angle down from the staff to the usual projectile height.
                if(p.grounded)projectiles.back().remainingDrop=std::max(0.f,-33.f-tipOffsetY);
                p.mageCastDamage=0;
            }
        }
        if(p.meleeLeft>0){
            p.meleeLeft=down(p.meleeLeft,dt);
            if(p.meleeLeft==0){
                if(p.strike==SwordStrike::Basic){
                    melee(p.meleeDamage,p.meleeBleed,p.meleeReach);
                    if(p.combo==2&&p.hitStop>0)p.hitStop=std::max(p.hitStop,.065f);
                }else resolveSwordSkill();
            }
        }
    }
    void stepEnemy(Enemy& e,float dt){
        if(e.dead())return;
        e.lightMark=down(e.lightMark,dt);
        e.attackTimer=down(e.attackTimer,dt);e.hitFlash=down(e.hitFlash,dt);e.reactionFlash=down(e.reactionFlash,dt);e.stagger=down(e.stagger,dt);e.knockback=toward(e.knockback,650*dt);
        // DOT ticks keep adding to the same white damage segment; after the hold
        // it shrinks at a bounded, frame-rate-independent speed.
        stepHealthTrail(e,e.maxHealth,dt);
        if(e.bleedStacks>0){
            e.bleedTick+=std::min(dt,e.bleed);
            e.bleed=down(e.bleed,dt);
            if(e.bleedTick>=bleedTickInterval){
                e.bleedTick-=bleedTickInterval;
                const int damage=3+(e.bleedStacks-1)/2+(e.boss&&e.bleedStacks==bleedMaxStacks?3:0);
                damageEnemy(e,damage,DamageKind::Bleed,true);
                if(!e.boss&&e.bleedStacks==bleedMaxStacks&&e.health*10<=e.maxHealth)
                    damageEnemy(e,e.health,DamageKind::Execute,false);
            }
            if(e.bleed<=0){
                --e.bleedStacks;
                if(e.bleedStacks>0)e.bleed=bleedDuration;
                else {e.bleedStacks=0;e.bleedTick=0;}
            }
        }
        if(e.burn>0){e.burn=down(e.burn,dt);e.burnTick+=dt;if(e.burnTick>=.6f){e.burnTick-=.6f;damageEnemy(e,2,DamageKind::Fire,true);}}
        if(e.dead())return;
        const float dx=player.x-e.x;float vx=0;
        const auto& spec=slimeSpec(e.kind);
        if(e.attackAge>=0){
            const float before=e.attackAge;e.attackAge+=dt;
            const float phase=std::min(1.f,e.attackAge/spec.attackDuration);
            if(e.kind==1){
                // Commit to a destination at takeoff; the player can dodge it.
                const float t=std::clamp((phase-.22f)/(.68f-.22f),0.f,1.f);
                const float eased=t*t*(3-2*t);
                e.x=e.attackFromX+(e.attackToX-e.attackFromX)*eased;
                e.y=floorY-4*t*(1-t)*120.f;
            }else{
                const float lunge=std::sin(phase*3.14159265f);
                e.x=e.attackFromX+e.facing*(e.kind==0?18.f:30.f)*lunge*lunge;
            }
            const float contact=spec.attackDuration*spec.contact;
            if(before<contact&&e.attackAge>=contact){
                const BodyRect playerBody{player.x-14,player.y-86,player.x+14,player.y};
                if(overlaps(enemyAttackBounds(e),playerBody))damagePlayer(e.damage);
            }
            if(e.attackAge>=spec.attackDuration){e.attackAge=-1;e.y=floorY;}
        }else{
            if(std::abs(dx)>1)e.facing=dx>0?1:-1;
            const float gap=enemyFrontGap(e,player);
            if(gap>spec.engageGap)vx=e.facing*e.speed*(e.bleedStacks>=bleedSlowStacks?.8f:1.f);
            else if(e.attackTimer<=0&&std::abs(player.y-e.y)<100){
                e.attackAge=0;e.attackTimer=spec.cooldown;e.attackFromX=e.x;
                e.attackToX=e.x+e.facing*std::clamp(std::abs(dx),60.f,240.f);
            }
        }
        e.travelPhase=std::fmod(e.travelPhase+std::abs(vx)*dt/64.f,1.f);
        e.x+=(vx+e.knockback)*dt;
        // World clipping follows the same visible silhouette as hit detection.
        const auto body=enemyBounds(e);
        if(body.left<0)e.x-=body.left;
        if(body.right>worldWidth)e.x-=body.right-worldWidth;
    }
    void stepLightLances(float dt){
        lightLances.erase(std::remove_if(lightLances.begin(),lightLances.end(),[&](LightLance& lance){
            if((lance.life-=dt)<=0)return true;
            const auto& target=enemies[lance.target];
            if(lance.seeking){
                if(target.dead())return true;
                if(lance.delay>0){lance.delay-=dt;return false;}
                const auto bounds=enemyBounds(target);
                const float tx=target.x,ty=(bounds.top+bounds.bottom)*.5f;
                const float dx=tx-lance.x,dy=ty-lance.y;
                const float distance=std::hypot(dx,dy);
                const float movement=(lance.verdict?2700.f:2100.f)*dt;
                lance.angle=std::atan2(dy,dx)*57.29578f;
                if(distance<=movement+12.f){
                    damageEnemy(enemies[lance.target],(lance.verdict?8:10)+player.gear.attack(),DamageKind::Normal,false);
                    if(lance.markOnHit&&!enemies[lance.target].dead())enemies[lance.target].lightMark=5.f;
                    lightBursts.push_back({tx,ty});
                    return true;
                }
                const float traveled=std::min(movement,distance);
                const float oldX=lance.x,oldY=lance.y;
                lance.x+=dx/distance*traveled;
                lance.y+=dy/distance*traveled;
                // Persist a paper-thin white spatial incision behind the light spear.
                if(riftScars.size()<160&&traveled>3.f)
                    riftScars.push_back({(oldX+lance.x)*.5f,(oldY+lance.y)*.5f,
                        lance.angle,.15f,false});
                return false;
            }
            const float previousX=lance.x,previousY=lance.y;
            const float speed=1050.f;
            lance.y+=speed*dt;
            if(!target.dead())lance.x=target.x+lance.offset;
            lance.angle=std::atan2(lance.y-previousY,lance.x-previousX)*57.29578f;
            return lance.y>viewHeight;
        }),lightLances.end());
    }
    void stepProjectiles(float dt){
        for(int i=static_cast<int>(projectiles.size())-1;i>=0;--i){auto& b=projectiles[i];float old=b.x;b.x+=b.facing*(b.empowered?440:540)*dt;b.life-=dt;bool remove=b.life<=0;
            const float drop=std::min(b.remainingDrop,740.f*dt);
            b.y+=drop;
            b.remainingDrop-=drop;
            for(auto& e:enemies){if(remove||e.dead())continue;
                float r=b.empowered?14:8;
                if(enemySweptDisc(e,old,b.x,b.y,r)){hit(e,b.damage,true);remove=true;}
            }
            if(remove)projectiles.erase(projectiles.begin()+i);
        }
    }
public:
    Player player;
    std::vector<GroundSurface> platforms;
    std::array<Enemy,4> enemies;
    std::vector<Projectile> projectiles;
    std::vector<LightLance> lightLances;
    std::vector<LightBurst> lightBursts;
    std::vector<RiftScar> riftScars;
    float cameraCenter=viewWidth*.5f;
    std::vector<Popup> popups;
    std::vector<BleedSpread> bleedSpreads;
    bool ended=false,won=false,inventory=false;
    float clock=0,enemyVisualClock=0;
    Combat(){projectiles.reserve(16);popups.reserve(32);restart();}
    void restart(){player={};enemies={Enemy(0,575),Enemy(1,800),Enemy(0,1020),Enemy(2,1285)};projectiles.clear();lightLances.clear();lightBursts.clear();riftScars.clear();cameraCenter=viewWidth*.5f;popups.clear();bleedSpreads.clear();ended=won=inventory=false;clock=enemyVisualClock=0;}
    int foes()const{int count=0;for(const auto& e:enemies)if(!e.dead())++count;return count;}
    void hit(Enemy& e,int amount,bool fire,int bleed=0,bool slam=false){
        if(e.dead()||ended)return;
        if(fire){e.burn=3;e.burnTick=0;if(e.bleed>0){amount+=22;e.bleed=0;e.bleedStacks=0;e.bleedTick=0;e.reactionFlash=.5f;}}
        // Knockback resistance only scales displacement; being pushed does not interrupt attacks.
        const float impulse=(slam?315.f:e.reactionFlash>0?210.f:135.f)*
            (1.f-std::clamp(e.knockbackResistance(),0.f,1.f));
        if(impulse>0.f){
            e.knockback=(e.x>=player.x?1.f:-1.f)*impulse;
        }
        damageEnemy(e,amount,fire?DamageKind::Fire:DamageKind::Normal,false);
        addBleed(e,bleed);
    }
    void damagePlayer(int amount){
        auto& p=player;if(ended||p.invulnerable>0||p.health<=0)return;
        holdHealthTrail(p,.12f);
        p.health=std::max(0,p.health-std::max(1,amount-p.gear.defense()));p.hurtTint=.18f;cancelAirSlam();cancelMageCast();p.meleeLeft=0;p.invulnerable=.7f;p.attackFlash=0;++p.hurtSerial;p.combo=0;p.comboWindow=0;
        if(p.health==0){ended=true;won=false;inventory=false;}
    }
    void step(float dt,Actions a){
        if(a.restart){restart();return;}
        clock+=dt;
        player.hurtTint=down(player.hurtTint,dt);
        for(auto& scar:riftScars)scar.life-=dt;
        riftScars.erase(std::remove_if(riftScars.begin(),riftScars.end(),
            [](const RiftScar& scar){return scar.life<=0;}),riftScars.end());
        for(auto& burst:lightBursts)burst.life-=dt;
        lightBursts.erase(std::remove_if(lightBursts.begin(),lightBursts.end(),
            [](const LightBurst& burst){return burst.life<=0;}),lightBursts.end());
        stepHealthTrail(player,player.maxHealth(),dt);
        for(auto& effect:bleedSpreads)effect.life-=dt;
        bleedSpreads.erase(std::remove_if(bleedSpreads.begin(),bleedSpreads.end(),
            [](const BleedSpread& effect){return effect.life<=0;}),bleedSpreads.end());
        for(auto& popup:popups){popup.age+=dt;popup.y-=27*dt;}
        popups.erase(std::remove_if(popups.begin(),popups.end(),[](const Popup& p){return p.age>=.6f;}),popups.end());
        if(ended)return;
        if(a.inventory)inventory=!inventory;
        if(a.weapon)player.equip(0);
        if(a.armor)player.equip(1);
        if(a.charm)player.equip(2);
        stepPlayer(dt,a);
        // A short cinematic time-stop freezes enemy motion and existing projectiles,
        // while the executioner's own animation continues to the release frame.
        const bool verdictFreeze=player.whiteNight&&player.whiteMove==WhiteMove::Verdict&&
            player.whiteAge>=.12f&&player.whiteAge<.66f;
        if(!verdictFreeze){
            enemyVisualClock+=dt;
            for(auto& enemy:enemies){stepEnemy(enemy,dt);if(ended)break;}
            if(!ended)stepProjectiles(dt);
        }
        if(!ended)stepLightLances(dt);
        if(!ended&&foes()==0){ended=true;won=true;inventory=false;}
    }
};
struct Animation {
    int state=-1,frame=0,attackSerial=0,hurtSerial=0,landingSerial=0,combo=1,attackState=4;
    float time=0,attackTime=0,attackDuration=.36f,hurtTime=0,landTime=0,travelPhase=0,lastX=140;
    bool initialized=false,wasHitStopped=false,midpoint=false;
    float posePhase=0; // Continuous visual phase, before integer atlas selection.
    void reset(){*this=Animation{};}
    void step(const Player& p,float dt){
        bool retrigger=p.attackSerial!=attackSerial;
        float distance=initialized?std::abs(p.x-lastX):0;lastX=p.x;initialized=true;
        if(retrigger){
            attackSerial=p.attackSerial;
            combo=std::clamp(p.combo,1,3);
            attackState=p.attackPose>0?p.attackPose:3+combo;
            const auto& clip=motionClips[attackState];
            attackDuration=p.attackPose>0?clip.duration:
                p.form==0&&p.combo>0?clip.duration/p.basicSpeed():p.mageCastLeft>0?mageCastDuration:.36f;
            attackTime=p.hitStop>0?attackDuration*(1-clip.contact):attackDuration;
        }
        if(p.hurtSerial!=hurtSerial){hurtSerial=p.hurtSerial;hurtTime=.23f;}
        if((isSkillMotion(attackState)&&p.form!=0)||p.dashLeft>0||hurtTime>0||p.airSlam||p.slamRecover>0||(!p.grounded&&p.form==0))attackTime=0;
        if(p.hitStop>0&&!wasHitStopped&&attackTime>0)attackTime=attackDuration*(1-motionClips[attackState].contact);
        wasHitStopped=p.hitStop>0;
        const bool landed=p.landingSerial!=landingSerial;
        landingSerial=p.landingSerial;
        if(!p.grounded)landTime=0;
        else if(landed)landTime=motionClips[10].duration;
        // Freeze visual action during a gameplay hit-stop, except death/hurt feedback.
        float visualDt=p.hitStop>0&&p.health>0&&hurtTime<=0?0:dt;
        attackTime=std::max(0.f,attackTime-visualDt);hurtTime=std::max(0.f,hurtTime-dt);if(!landed)landTime=std::max(0.f,landTime-visualDt);
        int next=p.health<=0?9:p.dashLeft>0?7:(p.airSlam||p.slamRecover>0)?12:attackTime>0?attackState:!p.grounded?(p.vy<0?2:3):landTime>0?10:std::abs(p.vx)>20?(!p.whiteNight&&std::abs(p.vx)<180?11:1):0;
        if(next!=state||retrigger){state=next;time=0;}else time+=visualDt;
        const auto clip=motionClips[next];
        const bool loop=next==0||next==1||next==11;
        float phase=0;
        if(next==0)phase=std::fmod(time/clip.duration*clip.count,static_cast<float>(clip.count));
        else if(next==1||next==11){
            const float stride=p.whiteNight?whiteNightRunStride:clip.stride;
            travelPhase=std::fmod(travelPhase+distance/stride,1.f);
            phase=travelPhase*clip.count;
        }
        else if(next==2)phase=(1-std::clamp(-p.vy/650.f,0.f,1.f))*(clip.count-1);
        else if(next==3)phase=std::clamp(p.vy/700.f,0.f,1.f)*(clip.count-1);
        else if(next==12){
            if(p.airSlam)phase=p.slamAge<airSlamWindup?std::min(2.999f,p.slamAge/airSlamWindup*3):
                3+std::min(1.999f,(p.slamAge-airSlamWindup)/.08f);
            else phase=5+std::min(2.999f,(1-p.slamRecover/airSlamRecovery)*3);
        }
        else if((next>=4&&next<=6)||isSkillMotion(next))phase=(1-attackTime/attackDuration)*clip.count;
        else if(next==7)phase=(1-std::clamp(p.dashLeft/.15f,0.f,1.f))*clip.count;
        else if(next==10)phase=(1-landTime/clip.duration)*clip.count;
        else phase=time/clip.duration*clip.count;
        posePhase=std::clamp(phase/((next==2||next==3)?clip.count-1.f:float(clip.count)),0.f,1.f);
        const int key=std::clamp(static_cast<int>(phase),0,clip.count-1);
        frame=clip.start+key;
        // Keep the original silhouettes during the jump-to-fall transition.
        midpoint=next!=2&&next!=3&&(loop||key+1<clip.count)&&phase-key>=.5f;
    }
};
}
