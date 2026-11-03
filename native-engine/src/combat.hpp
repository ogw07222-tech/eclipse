#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include "motion.hpp"
#include "sword_trails.hpp"

namespace eclipse {
constexpr double fixedStep=1.0/60.0;
constexpr float viewWidth=1280,viewHeight=720,floorY=585,worldWidth=1600;
struct Actions {
    bool left=false,right=false,jump=false,attack=false,dash=false,skill=false,swap=false;
    bool restart=false,inventory=false,weapon=false,armor=false,charm=false;
    bool settings=false,shake=false,damageNumbers=false,afterimages=false;
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
struct Player {
    Equipment gear;
    float x=140,y=floorY,vx=0,vy=0;
    int health=100,form=0,facing=1,combo=0,attackSerial=0,hurtSerial=0;
    bool grounded=true;
    float dashLeft=0,dashCooldown=0,attackCooldown=0,skillCooldown=0,swapCooldown=0;
    float invulnerable=0,coyote=0,jumpBuffer=0,attackFlash=0,comboWindow=0,hitStop=0,swapFlash=0;
    float meleeLeft=0,meleeTime=0,meleeDuration=0,hitFlash=0;int meleeDamage=0;bool meleeBleed=false,swordActive=false;
    std::array<bool,4> swordHits{};
    int maxHealth()const{return 100+gear.hp();}
    void equip(int slot){int before=maxHealth();gear.cycle(slot);health=std::clamp(health+maxHealth()-before,1,maxHealth());}
};
struct Enemy {
    int kind=0,health=55,maxHealth=55,damage=9;
    float x=0,y=floorY,speed=95,attackTimer=0,bleed=0,bleedTick=0,burn=0,burnTick=0,hitFlash=0,reactionFlash=0,knockback=0;
    Enemy(int type=0,float at=0):kind(type),health(type?85:55),maxHealth(health),damage(type?15:9),x(at),speed(type?63:95){}
    bool dead()const{return health<=0;}
};
struct Projectile {float x,y;int facing,damage;bool empowered;float life=1.6f;};
enum class DamageKind{Normal,Fire,Bleed};
struct Popup{float x,y;int amount;DamageKind kind;float age=0;};
class Combat {
    static float down(float value,float dt){return std::max(0.f,value-dt);}
    static float toward(float value,float step){return value>0?std::max(0.f,value-step):std::min(0.f,value+step);}
    void damageEnemy(Enemy& e,int amount,DamageKind kind,bool dot){
        if(e.dead())return;
        e.health=std::max(0,e.health-amount);if(!dot)e.hitFlash=.18f;
        popups.push_back({e.x,e.y-86,amount,kind});
    }
    void beginSword(int damage,bool bleed,int combo){
        auto& p=player;p.combo=combo;p.comboWindow=.8f;
        p.meleeTime=0;p.meleeDuration=motionClips[3+combo].duration/p.gear.speed();
        p.meleeLeft=p.meleeDuration*motionClips[3+combo].contact;
        p.meleeDamage=damage;p.meleeBleed=bleed;p.swordActive=true;p.swordHits={};
    }
    void swordContact(float dt){
        auto& p=player;if(!p.swordActive)return;
        const auto clip=motionClips[3+p.combo];
        float previous=p.meleeTime;p.meleeTime=std::min(p.meleeTime+dt,p.meleeDuration);
        p.meleeLeft=std::max(0.f,p.meleeDuration*clip.contact-p.meleeTime);
        if(p.meleeTime>=p.meleeDuration*clip.contact){
            int first=std::max(static_cast<int>(previous/p.meleeDuration*clip.count),static_cast<int>(clip.contact*clip.count));
            int last=std::min(clip.count-1,static_cast<int>(p.meleeTime/p.meleeDuration*clip.count));
            for(int f=first;f<=last;++f){const auto frame=swordTrailFrames[clip.start+f];
                for(std::size_t i=0;i<enemies.size();++i){auto& e=enemies[i];if(e.dead()||p.swordHits[i])continue;
                    float half=e.kind?21:16,height=e.kind?62:48;
                    for(int n=frame.start;n<frame.start+frame.count;++n){
                        if(swordSpanHits(swordTrailSpans[n],p.x,p.y,p.facing,p.gear.range(),e.x-half,e.y-height,e.x+half,e.y)){
                            hit(e,p.meleeDamage,false,p.meleeBleed);p.swordHits[i]=true;
                            p.hitStop=std::max(p.hitStop,p.combo==3?.075f:p.combo==2?.065f:.045f);break;
                        }
                    }
                }
            }
        }
        if(p.meleeTime>=p.meleeDuration)p.swordActive=false;
    }
    void projectile(int damage,bool empowered){projectiles.push_back({player.x+player.facing*27,player.y-33,player.facing,damage,empowered});}
    void basic(){
        auto& p=player;p.attackFlash=.16f;++p.attackSerial;
        if(p.form==0){p.combo=p.comboWindow>0?p.combo%3+1:1;p.comboWindow=.8f;
            p.attackCooldown=(p.combo==1?.23f:p.combo==2?.25f:.42f)/p.gear.speed();
            beginSword((p.combo==1?12:p.combo==2?15:23)+p.gear.attack(),p.combo==3,p.combo);
        }else{p.combo=0;p.attackCooldown=.47f;projectile(11+p.gear.attack(),false);}
    }
    void skill(){
        auto& p=player;p.meleeLeft=0;p.swordActive=false;p.skillCooldown=p.form==0?3.4f:4.1f;p.attackFlash=.28f;++p.attackSerial;
        if(p.form==0){beginSword(23+p.gear.attack(),true,2);p.attackCooldown=std::max(p.attackCooldown,p.meleeDuration);}else projectile(25+p.gear.attack(),true);
    }
    void stepPlayer(float dt,Actions a){
        auto& p=player;
        p.dashLeft=down(p.dashLeft,dt);p.dashCooldown=down(p.dashCooldown,dt);p.attackCooldown=down(p.attackCooldown,dt);
        p.skillCooldown=down(p.skillCooldown,dt);p.swapCooldown=down(p.swapCooldown,dt);p.invulnerable=down(p.invulnerable,dt);
        p.attackFlash=down(p.attackFlash,dt);p.comboWindow=down(p.comboWindow,dt);p.swapFlash=down(p.swapFlash,dt);p.hitFlash=down(p.hitFlash,dt);
        if(p.hitStop>0){p.hitStop=down(p.hitStop,dt);return;}
        if(p.comboWindow<=0)p.combo=0;
        p.coyote=p.grounded?.11f:down(p.coyote,dt);p.jumpBuffer=a.jump?.13f:down(p.jumpBuffer,dt);
        float axis=static_cast<float>(a.right)-static_cast<float>(a.left);
        if(axis!=0)p.facing=axis>0?1:-1;
        if(a.swap&&p.swapCooldown<=0){p.meleeLeft=0;p.swordActive=false;p.form=1-p.form;p.swapCooldown=1;p.attackFlash=.22f;p.combo=0;p.comboWindow=0;p.swapFlash=.24f;++p.attackSerial;
            if(p.form==0){beginSword(13,false,1);p.attackCooldown=std::max(p.attackCooldown,p.meleeDuration);}else projectile(14,false);
        }
        if(a.dash&&p.dashCooldown<=0){p.meleeLeft=0;p.swordActive=false;p.dashLeft=.15f;p.dashCooldown=.85f;p.invulnerable=std::max(p.invulnerable,.15f);}
        if(p.dashLeft>0){p.vx=780*p.facing*p.gear.speed();p.vy=0;}
        else{p.vx=axis*280*p.gear.speed();p.vy+=1250*dt;
            if(p.jumpBuffer>0&&p.coyote>0){p.vy=-650;p.coyote=p.jumpBuffer=0;p.grounded=false;}
        }
        if(a.attack&&p.attackCooldown<=0&&p.dashLeft<=0)basic();
        if(a.skill&&p.skillCooldown<=0&&p.dashLeft<=0)skill();
        p.x=std::clamp(p.x+p.vx*dt,25.f,1550.f);p.y+=p.vy*dt;p.grounded=p.y>=floorY;
        if(p.grounded){p.y=floorY;p.vy=0;
            if(p.jumpBuffer>0&&p.dashLeft<=0){p.vy=-650;p.grounded=false;p.coyote=p.jumpBuffer=0;}
        }
        swordContact(dt);
    }
    void stepEnemy(Enemy& e,float dt){
        if(e.dead())return;
        e.attackTimer=down(e.attackTimer,dt);e.hitFlash=down(e.hitFlash,dt);e.reactionFlash=down(e.reactionFlash,dt);e.knockback=toward(e.knockback,650*dt);
        if(e.bleed>0){e.bleed=down(e.bleed,dt);e.bleedTick+=dt;if(e.bleedTick>=.75f){e.bleedTick=0;damageEnemy(e,3,DamageKind::Bleed,true);}}
        if(e.burn>0){e.burn=down(e.burn,dt);e.burnTick+=dt;if(e.burnTick>=.6f){e.burnTick-=.6f;damageEnemy(e,2,DamageKind::Fire,true);}}
        if(e.dead())return;
        float dx=player.x-e.x,vx=0;
        if(std::abs(dx)>46)vx=(dx>0?1:-1)*e.speed;
        else if(e.attackTimer<=0&&std::abs(player.y-e.y)<70){damagePlayer(e.damage);e.attackTimer=e.kind?1.8f:1.3f;}
        const float half=e.kind?23:18;e.x=std::clamp(e.x+(vx+e.knockback)*dt,half,worldWidth-half);
    }
    void stepProjectiles(float dt){
        for(int i=static_cast<int>(projectiles.size())-1;i>=0;--i){auto& b=projectiles[i];float old=b.x;b.x+=b.facing*(b.empowered?440:540)*dt;b.life-=dt;bool remove=b.life<=0;
            for(auto& e:enemies){if(remove||e.dead())continue;
                float nx=std::clamp(e.x,std::min(old,b.x),std::max(old,b.x)),dx=nx-e.x,dy=b.y-(e.y-30),r=b.empowered?34:25;
                if(dx*dx+dy*dy<r*r){hit(e,b.damage,true);remove=true;}
            }
            if(remove)projectiles.erase(projectiles.begin()+i);
        }
    }
public:
    Player player;
    std::array<Enemy,4> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Popup> popups;
    bool ended=false,won=false,inventory=false,settings=false;
    bool screenShake=true,showDamage=true,showAfterimages=true;
    float clock=0;
    Combat(){projectiles.reserve(16);popups.reserve(32);restart();}
    void restart(){player={};enemies={Enemy(0,575),Enemy(1,800),Enemy(0,1020),Enemy(1,1285)};projectiles.clear();popups.clear();ended=won=inventory=settings=false;clock=0;}
    int foes()const{int count=0;for(const auto& e:enemies)if(!e.dead())++count;return count;}
    void hit(Enemy& e,int amount,bool fire,bool bleed=false){
        if(e.dead()||ended)return;
        if(fire){e.burn=3;e.burnTick=0;if(e.bleed>0){amount+=22;e.bleed=0;e.reactionFlash=.5f;}}
        if(bleed){e.bleed=4.5f;e.bleedTick=0;}
        e.knockback=(e.x>=player.x?1:-1)*(e.reactionFlash>0?210:135);
        damageEnemy(e,amount,fire?DamageKind::Fire:DamageKind::Normal,false);
    }
    void damagePlayer(int amount){
        auto& p=player;if(ended||p.invulnerable>0||p.health<=0)return;
        p.health=std::max(0,p.health-std::max(1,amount-p.gear.defense()));p.meleeLeft=0;p.swordActive=false;p.hitFlash=.18f;p.invulnerable=.7f;p.attackFlash=.28f;++p.hurtSerial;p.combo=0;p.comboWindow=0;
        if(p.health==0){ended=true;won=false;inventory=false;}
    }
    void step(float dt,Actions a){
        if(a.settings){settings=!settings;inventory=false;return;}
        if(settings){
            if(a.shake)screenShake=!screenShake;
            if(a.damageNumbers)showDamage=!showDamage;
            if(a.afterimages)showAfterimages=!showAfterimages;
            if(a.restart)restart();
            return;
        }
        if(a.restart){restart();return;}
        clock+=dt;
        for(auto& popup:popups){popup.age+=dt;popup.y-=27*dt;}
        popups.erase(std::remove_if(popups.begin(),popups.end(),[](const Popup& p){return p.age>=.6f;}),popups.end());
        if(ended)return;
        if(a.inventory)inventory=!inventory;
        if(a.weapon)player.equip(0);
        if(a.armor)player.equip(1);
        if(a.charm)player.equip(2);
        stepPlayer(dt,a);for(auto& enemy:enemies){stepEnemy(enemy,dt);if(ended)break;}
        if(!ended)stepProjectiles(dt);
        if(!ended&&foes()==0){ended=true;won=true;inventory=false;}
    }
};
struct Animation {
    int state=-1,frame=0,attackSerial=0,hurtSerial=0,combo=1;
    float time=0,attackTime=0,attackDuration=.36f,hurtTime=0,landTime=0,travelPhase=0,lastX=140;
    bool wasGrounded=true,initialized=false,wasHitStopped=false;
    void reset(){*this=Animation{};}
    void step(const Player& p,float dt){
        bool retrigger=p.attackSerial!=attackSerial;
        float distance=initialized?std::abs(p.x-lastX):0;lastX=p.x;initialized=true;
        if(retrigger){attackSerial=p.attackSerial;combo=std::clamp(p.combo,1,3);
            attackDuration=p.form==0&&p.combo>0?motionClips[3+combo].duration/p.gear.speed():.36f;attackTime=p.hitStop>0?attackDuration*(1-motionClips[3+combo].contact):attackDuration;}
        if(p.hurtSerial!=hurtSerial){hurtSerial=p.hurtSerial;hurtTime=.23f;}
        if(p.dashLeft>0||hurtTime>0)attackTime=0;
        if(p.hitStop>0&&!wasHitStopped&&attackTime>0)attackTime=p.swordActive?p.meleeDuration-p.meleeTime:attackDuration*(1-motionClips[3+combo].contact);
        wasHitStopped=p.hitStop>0;
        if(!wasGrounded&&p.grounded&&p.health>0)landTime=std::abs(p.vx)>20?.06f:.12f;
        wasGrounded=p.grounded;
        // Freeze visual action during a gameplay hit-stop, except death/hurt feedback.
        float visualDt=p.hitStop>0&&p.health>0&&hurtTime<=0?0:dt;
        attackTime=std::max(0.f,attackTime-visualDt);hurtTime=std::max(0.f,hurtTime-dt);landTime=std::max(0.f,landTime-dt);
        if(p.form==0&&p.swordActive&&hurtTime<=0&&p.dashLeft<=0)attackTime=std::max(0.f,p.meleeDuration-p.meleeTime);
        int next=p.health<=0?9:hurtTime>0?8:p.dashLeft>0?7:attackTime>0?3+combo:!p.grounded?(p.vy<0?2:3):landTime>0?10:std::abs(p.vx)>20?(std::abs(p.vx)<180?11:1):0;
        if(next!=state||retrigger){state=next;time=0;}else time+=visualDt;
        const auto clip=motionClips[next];int index=0;
        if(next==0)index=static_cast<int>(time/clip.duration*clip.count)%clip.count;
        else if(next==1||next==11){travelPhase=std::fmod(travelPhase+distance/clip.stride,1.f);index=static_cast<int>(travelPhase*clip.count);}
        else if(next==2)index=static_cast<int>((1-std::clamp(-p.vy/650.f,0.f,1.f))*(clip.count-1));
        else if(next==3)index=static_cast<int>(std::clamp(p.vy/700.f,0.f,1.f)*(clip.count-1));
        else if(next>=4&&next<=6)index=static_cast<int>((1-attackTime/attackDuration)*clip.count);
        else if(next==7)index=static_cast<int>((1-std::clamp(p.dashLeft/.15f,0.f,1.f))*clip.count);
        else if(next==8)index=static_cast<int>((1-hurtTime/.23f)*clip.count);
        else if(next==10)index=static_cast<int>((1-landTime/.12f)*clip.count);
        else index=static_cast<int>(time/clip.duration*clip.count);
        frame=clip.start+std::clamp(index,0,clip.count-1);
    }
};
}
