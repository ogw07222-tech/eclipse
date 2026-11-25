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
#include "castle_enemy.hpp"
#include "monster_collision.hpp"
#include "castle_terrain.hpp" // ECLIPSE_CASTLE_FIXED_TERRAIN_V32
#include "boss_fx_assets.hpp"
#include "progression.hpp"

namespace eclipse {
constexpr double fixedStep=1.0/60.0;
constexpr float viewWidth=1280,viewHeight=720,floorY=ForestLayerScene::floorY,worldWidth=1600;
struct Actions {
    bool left=false,right=false,jump=false,attack=false,dash=false,skill=false,swap=false,ultimate=false;
    bool restart=false,inventory=false,weapon=false,armor=false,charm=false;
    bool jumpHeld=false,skillSweep=false,skillBuff=false,interact=false;
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
constexpr float kingPhaseShiftSeconds=2.8f;
constexpr float kingBurstRadius=300.f;
constexpr float thrustCooldown=6.f,sweepCooldown=9.f,buffCooldown=14.f;
// Space Ruler base damage (before gear). Only the values the design leaves open are tuned: field 6/tick,
// repulsion 12, every cooldown, radius and timing stay as specified. Static estimate of single-target DPS:
// Flash Executioner ~80, blade ~56, Space Ruler 34 -> ~45 with these (it keeps its range/area advantage).
constexpr int spaceBoltDamage=15,spaceSpikesDamage=36,spaceSingularityDamage=105;
// The memory altar stands where the protagonist awoke (the forest start, Character x default 140).
constexpr float memoryAltarX=140.f,memoryAltarReach=100.f;
constexpr float preparationDuration=5.f;
constexpr int bleedMaxStacks=10,bleedSlowStacks=5;
// Every 3 seconds one stack expires; reapplying bleed resets that countdown.
constexpr float bleedDuration=3.f,bleedTickInterval=.75f;
enum class SwordStrike {Basic,Thrust,Sweep};

enum class FlashMove { None, Slash, Rush, Command, Blink, Verdict };
struct FlashClip {int start,count;float duration,contact;};
constexpr FlashClip flashClips[]={{0,1,1,0},{16,15,.46f,.244f},{31,12,.50f,.125f},
    {43,12,.46f,.23f},{55,12,.36f,.15f},{67,20,1.12f,.728f}};
struct Player {
    Equipment gear;
    float x=140,y=floorY,vx=0,vy=0;
    int health=100,form=0,facing=1,combo=0,attackSerial=0,hurtSerial=0,landingSerial=0;
    int storedLightLances=0;
    bool grounded=true,flashExecutioner=false;
    // Only these two QA-selected jobs are available to the swap button.
    std::array<int,2> equippedJobs{{1,2}};
    int activeJob=0;
    FlashMove flashMove=FlashMove::None;
    float flashAge=0,flashBoost=0,flashSlashSpeed=1.f;
    size_t flashTarget=0;
    unsigned flashRushHits=0;
    float takeoffLock=0;
    // Rebirth handicap: lost legendary power (see PlayerHandicap).
    bool handicapped=false,dashLocked=false;
    int unlockedSkills=4;
    float damageScale=1.f,moveScale=1.f;
    int statHealth=0;float statMove=1.f;   // progression bonuses (see progression.hpp)
    float lanceSpeedExtra=0.f;int lanceCapExtra=0;   // job mastery tree (see refreshStats)
    float dashLeft=0,dashCooldown=0,attackCooldown=0,skillCooldown=0,swapCooldown=0,verdictCooldown=0;
    float invulnerable=0,coyote=0,jumpBuffer=0,attackFlash=0,comboWindow=0,hitStop=0,swapFlash=0;
    float trailingHealth=-1.f,healthTrailHold=0.f;
    float hurtTint=0; // visual-only: short white flash and muted-red impact
    bool airSlam=false;
    float slamAge=0,slamRecover=0,slamImpact=0,slamImpactX=0,slamImpactY=floorY;
    // Space Ruler occupies the ranged job slot (form 1).
    int spaceMove=0; // 0 none, 1 attack, 2 field, 3 spikes, 4 repulsion, 5 collapse
    float spaceAge=0,spaceFuel=1,spaceFlightBase=floorY,spaceField=0,spaceFieldTick=0;
    bool spaceReleased=false,spaceExhausted=false;
    float mageCastLeft=0;
    int mageCastDamage=0;
    float meleeLeft=0,meleeReach=0;
    int meleeDamage=0,meleeBleed=0,attackPose=0;
    SwordStrike strike=SwordStrike::Basic;
    float sweepReady=0,buffReady=0,mageSkillCooldown=0,preparation=0;
    // Class archetypes: ranged 80%, melee 100%, assassin 125% of base run speed.
    // Keep this separate from attack speed and dash/skill movement.
    float movementMultiplier() const {return (flashExecutioner?1.25f:form==1?.80f:1.f)*moveScale*statMove;}
    // Each stored light orb empowers basic sword attacks, creating a choice
    // between retaining up to +40% attack speed and spending the orbs on marks.
    float basicSpeed() const {return gear.speed()*(preparation>0?1.20f:1.f)*
        (flashExecutioner?1.f+(.08f+lanceSpeedExtra)*std::clamp(storedLightLances,0,5+lanceCapExtra):1.f);}
    int maxHealth()const{return 100+gear.hp()+statHealth;}
    // Changing the progression health bonus keeps the current health ratio-neutral (like equip()).
    void setStatHealth(int value){
        const int before=maxHealth();
        statHealth=value;
        if(health>0){
            const int previousHealth=health;
            health=std::clamp(health+maxHealth()-before,1,maxHealth());
            if(trailingHealth>=0)trailingHealth=std::clamp(trailingHealth+health-previousHealth,float(health),float(maxHealth()));
        }
    }
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
    float lightMark=0;                 // light stacks fade together when this timer runs out (>0 counts as one stack even if set directly)
    int lightStacks=0;
    int darkStacks=0;float darkTimer=0,reactCd=0,erodeTick=0;   // dark stacks; max stackMax, fade after stackSeconds
    bool boss=false;
    bool rewarded=false;   // its memory was already absorbed
    // Story-scripted state: arrival wake-up, Demon King armor-break shift and attack counter.
    float wake=0,phaseShift=0,phaseShiftTotal=0;
    int attackCount=0;
    bool burstAttack=false,phase2=false,unbeatable=false;
    // Demon King move engine state (see BossMove in castle_enemy.hpp).
    BossMove move=BossMove::None,lastMove=BossMove::None;
    int step=0,moveCounter=0,animFrom=0,motion=0;
    float moveDur=0,moveContact=.5f;
    bool hitDone=false;
    // Control-practice target: stands still, never attacks, never dies, counts the hits it takes.
    bool dummy=false,fights=false;int dummyHits=0;   // fights: a live practice foe that attacks like a real one
    float knockbackResistance() const {return boss?1.f:enemySpec(kind).knockbackResistance;}
    Enemy(int type=0,float at=0,int area=0):kind(std::clamp(type,0,static_cast<int>(DemonKingTrue))),
        health(enemyHealth(kind,area)),maxHealth(health),damage(enemyDamage(kind,area)),
        x(at),speed(enemySpeed(kind,area)),boss(kind==VoidShade||kind>=DemonKingArmored){}
    bool dead()const{return health<=0;}
};
struct Projectile {float x,y;int facing,damage;bool empowered;float life=1.6f,remainingDrop=0;bool voidBolt=false;};
struct LightLance {size_t target;float x,y,life=.65f;float offset=0,angle=90;bool verdict=false,seeking=false,markOnHit=false;float delay=0,morphTotal=0;float visualFlightAge=0;};
enum class LightFx { Impact, Slash, Rush, Command, Depart, Arrive, Verdict, SlashHit, RushHit };
struct LightBurst {float x,y,life=.24f;LightFx kind=LightFx::Impact;int facing=1;unsigned long long visualSerial=0;};
struct SpaceEffect {int kind;float x,y,age=0;int facing=1;};
struct RiftScar {float x,y,angle,life=.16f;bool slash=false;};
enum class DamageKind{Normal,Fire,Bleed,Execute};
struct BleedSpread {float fromX,fromY,toX,toY,life=.23f;};
struct Popup{float x,y;int amount;DamageKind kind;float age=0;};
// Light x dark reactions. Eclipse (light stack first, then a Ruler hit): absorb the light, pull foes in, collapse.
// Solar burst (dark stacks first, then a Flash hit): the stacks flash into an instant blast.
constexpr int stackMax=5;
constexpr float voidExecutePerStack=.06f,voidKnockbackPerStack=.5f;   // spikes execute below 6 % max health per void stack (never bosses); repulsion +50 % per stack
constexpr float stackSeconds=6.f,reactionCooldown=1.5f;
// Elements work on any character: a direct hit carries an ElementTag (element + stacks it leaves) and reactions
// look only at the elements on the foe, never at who hit it. A new job opts in by tagging its hits here.
enum class Element : unsigned char {None,Light,Dark};
struct ElementTag {Element element=Element::None;int stacks=0;};
namespace tags {
inline constexpr ElementTag none{};
// Flash Executioner (light)
inline constexpr ElementTag flashStrike{Element::Light,0},flashLance{Element::Light,1},flashVerdict{Element::Light,0};
// Space Ruler (dark)
inline constexpr ElementTag rulerBolt{Element::Dark,1},rulerSpikes{Element::Dark,2},rulerRepulse{Element::Dark,1},
    rulerField{Element::Dark,1},rulerSingularity{Element::Dark,0};
}
constexpr float eclipsePullStart=.25f,eclipseCollapseAt=.85f,eclipseEnd=1.15f,eclipsePullRadius=220.f,eclipseBlastRadius=160.f;
struct EclipseFx {int kind=0;float x=0,y=0,age=0;int marks=0,dark=0;bool collapsed=false;};   // kind 0 eclipse well, 1 solar burst
// Boss ground hazards: pillars erupt at a marked spot after a visible delay; waves slide along the
// floor and pass harmlessly under a player who is airborne above `height`.
enum class HazardKind : unsigned char {Pillar,Wave};
struct BossHazard {
    HazardKind kind=HazardKind::Pillar;
    BossFx fx=BossFx::Circle;   // pixelfx layer that draws this hazard
    float x=0,y=0,radius=100,delay=.6f,age=0,height=70,speed=520,range=900,startX=0;
    int dir=1,damage=10;
    bool detonated=false,hit=false,spent=false;
    float spentAge=0;
};
constexpr float hazardPillarLinger=1.2f;   // keeps the hazard (and its effect layer) alive after it erupts
// A scheduled pixelfx layer. age < 0 means it has not started; the effect's impact frame lands when
// the move's contact lands (bossFxSheets[kind].peak / fps after age 0).
struct BossFxEvent {BossFx kind;float x,y;int facing;float age;float scale;float life;};   // life > 0: looping layer that ends after `life` seconds
// Horizontal one-way surfaces share the same descending foot-contact rule.
struct GroundSurface {
    float left,right,y;
    bool covers(float x) const {return x>=left&&x<=right;}
};

class Combat {
    float surfaceBelow(float x,float y) const {
        const float base=castleArea?CastleTerrain::groundAt(castleArea,x):floorY;
        float nearest=y<=base?base:std::numeric_limits<float>::infinity();
        for(const auto& surface:platforms)
            if(surface.covers(x)&&surface.y>=y)nearest=std::min(nearest,surface.y);
        return nearest;
    }
    float crossedGround(float x,float y,float nextX,float nextY) const {
        if(nextY<y)return std::numeric_limits<float>::infinity();
        const float base=castleArea?CastleTerrain::groundAt(castleArea,nextX):floorY;
        float contact=y<=base&&nextY>=base?base:std::numeric_limits<float>::infinity();
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
        p.vy=-700;
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
        if(e.dead()||e.phaseShift>0||e.wake>0)return;
        bool crit=false;
        // Attack/crit stats scale direct hits only: not bleed/burn/field ticks, and not the execute finisher.
        if(!dot&&kind!=DamageKind::Execute){
            amount=Progression::applyDamage(amount,stats,stats.critChance>0.f?statRoll():1.f,&crit);
            // Blade tree, Last Stand: hits below 40 % health deal more.
            if(perks.lastStandMul>1.f&&player.health*10<player.maxHealth()*4)amount=static_cast<int>(amount*perks.lastStandMul+.5f);
        }
        holdHealthTrail(e,dot?.35f:.12f);
        // The true form cannot be killed in the story fight: damage stops at a floor.
        const int floorHealth=e.unbeatable?e.maxHealth*35/100:0;
        e.health=std::max(floorHealth,e.health-amount);if(!dot)e.hitFlash=.18f;
        if(e.dummy){++e.dummyHits;e.health=std::max(e.health,std::max(1,e.maxHealth*3/10));}   // practice foes never fall below 30 %
        popups.push_back({e.x,e.y-86,amount,kind});
        if(e.kind==DemonKingArmored&&e.health<=0){
            // Armor breaks instead of dying: hold at 1 HP through the transformation.
            e.health=1;e.phaseShift=e.phaseShiftTotal=kingPhaseShiftSeconds;
            e.attackAge=-1;e.burstAttack=false;e.move=BossMove::None;e.motion=0;e.hitFlash=.5f;++kingShiftSerial;hazards.clear();bossFx.clear();
        }
        if(crit)fireProcs(Trigger::OnCrit,&e);
        if(e.health<=0&&!e.rewarded){
            e.rewarded=true;
            if(dropsEnabled){
                const int amount=Progression::memoryDrop(e.maxHealth,e.boss);
                progression.addMemories(amount);
                mastery.addXp(activeJob(),MasteryState::xpDrop(e.boss));   // the job in use gains mastery
                if(player.health>0&&perks.healPerKill>0)player.health=std::min(player.maxHealth(),player.health+perks.healPerKill);
                memoryMotes.push_back({e.x,e.y-46,0.f,amount});   // visual only: the memory flows into the protagonist
            }
            fireProcs(Trigger::OnKill,&e);
        }
    }
    // Basic hits build a visible stock of light lances instead of firing automatically.
    void chargeLightLance(){
        if(player.flashExecutioner)player.storedLightLances=std::min(5+perks.lanceCapBonus,player.storedLightLances+1);
    }
    void releaseStoredLances(){
        auto& p=player;
        if(p.storedLightLances<=0)return;
        std::vector<size_t> candidates;
        candidates.reserve(enemies.size());
        const float left=cameraCenter-viewWidth*.5f;
        for(size_t i=0;i<enemies.size();++i)
            if(!enemies[i].dead()&&enemies[i].x>=left&&enemies[i].x<=left+viewWidth)candidates.push_back(i);
        if(candidates.empty())return;
        static std::mt19937 random{std::random_device{}()};
        const int total=p.storedLightLances;
        for(int i=0;i<total;++i){
            const size_t target=candidates[std::uniform_int_distribution<size_t>{0,candidates.size()-1}(random)];
            const float spread=(i-(total-1)*.5f)*16.f;
            const float phase=clock*2.8f+i*6.2831853f/total;
            const float orbX=p.x+std::cos(phase)*57.f;
            const float orbY=p.y-61.f+std::sin(phase)*37.f;
            // A stationary orb stretches into a spear before accelerating.
            const float morph=.11f;
            lightLances.push_back({target,orbX,orbY,1.4f,spread,0.f,false,true,true,morph,morph});
        }
        p.storedLightLances=0;
    }
    // Verdict rains exactly 20 sky lances, split as evenly as possible
    // among enemies marked at cast time. The remainder is assigned randomly.
    void launchVerdictLances(){
        std::vector<size_t> marked;
        for(size_t i=0;i<enemies.size();++i)
            if(!enemies[i].dead()&&enemies[i].lightMark>0)marked.push_back(i);
        if(marked.empty())return;
        static std::mt19937 random{std::random_device{}()};
        std::shuffle(marked.begin(),marked.end(),random);
        constexpr int volley=20;
        std::vector<size_t> assignments;
        assignments.reserve(volley);
        for(size_t i=0;i<marked.size();++i){
            const int count=volley/static_cast<int>(marked.size())
                +(static_cast<int>(i)<volley%static_cast<int>(marked.size())?1:0);
            for(int j=0;j<count;++j)assignments.push_back(marked[i]);
        }
        std::shuffle(assignments.begin(),assignments.end(),random);
        std::array<float,volley> delays{};
        std::uniform_real_distribution<float> delayJitter{-.045f,.075f};
        for(int i=1;i<volley;++i)
            delays[i]=std::max(.025f,i*.072f+delayJitter(random));
        std::shuffle(delays.begin()+1,delays.end(),random);
        const float viewLeft=cameraCenter-viewWidth*.5f;
        std::uniform_real_distribution<float> startX{viewLeft-viewWidth*.12f,
                                                       viewLeft+viewWidth*1.12f};
        std::uniform_real_distribution<float> skyDepth{300.f,520.f};
        for(int i=0;i<volley;++i){
            const size_t target=assignments[i];
            const auto& e=enemies[target];
            // Scatter origins across and above the view, then let each lance
            // steer to its balanced marked-enemy assignment on an uneven beat.
            const float skyY=std::min(-135.f,enemyBounds(e).top-skyDepth(random));
            lightLances.push_back({target,startX(random),skyY,3.5f,0.f,90.f,
                                   true,true,false,delays[i]});
        }
    }
public:
    // ---- element stacks and reactions (character-agnostic) ----------------------------------------
    static Element opposite(Element e){return e==Element::Light?Element::Dark:e==Element::Dark?Element::Light:Element::None;}
    static int stacksOf(const Enemy& e,Element el){
        if(el==Element::Light)return e.lightMark>0?std::max(1,e.lightStacks):0;
        if(el==Element::Dark)return e.darkStacks;
        return 0;
    }
    static void addStacks(Enemy& e,Element el,int n){
        if(e.dead()||n<=0)return;
        if(el==Element::Light){e.lightStacks=std::min(stackMax,stacksOf(e,el)+n);e.lightMark=stackSeconds;}
        else if(el==Element::Dark){e.darkStacks=std::min(stackMax,e.darkStacks+n);e.darkTimer=stackSeconds;}
    }
    static int consumeStacks(Enemy& e,Element el){
        const int n=stacksOf(e,el);
        if(el==Element::Light){e.lightStacks=0;e.lightMark=0;}else if(el==Element::Dark){e.darkStacks=0;e.darkTimer=0;}
        return n;
    }
    // After a direct hit has dealt its damage: the opposite element already on the foe reacts, then this hit's stacks stick.
    void elementReact(Enemy& e,ElementTag tag){
        if(e.dead()||tag.element==Element::None)return;
        if(e.reactCd<=0&&stacksOf(e,opposite(tag.element))>0){
            if(tag.element==Element::Dark)startEclipse(e);else startSolar(e);
        }
        addStacks(e,tag.element,tag.stacks);
    }
    void elementHit(Enemy& e,int amount,ElementTag tag){
        damageEnemy(e,amount,DamageKind::Normal,false);
        elementReact(e,tag);
    }
    // Void spikes: a non-boss foe left under (6 % x void stacks) of its health is executed.
    void voidExecute(Enemy& e){
        if(e.dead()||e.boss||e.darkStacks<=0)return;
        if(e.health<=e.maxHealth*voidExecutePerStack*e.darkStacks)damageEnemy(e,e.health,DamageKind::Execute,false);
    }
    // Light first, dark after: the light is taken in, foes are pulled to it, then it collapses.
    void startEclipse(Enemy& e){
        int marks=0;const int dark=e.darkStacks;
        for(auto& other:enemies){
            if(other.dead()||stacksOf(other,Element::Light)<=0||std::hypot(other.x-e.x,other.y-e.y)>eclipsePullRadius)continue;
            marks+=consumeStacks(other,Element::Light);other.reactCd=reactionCooldown;   // the light is taken in
        }
        e.reactCd=reactionCooldown;
        eclipseFx.push_back({0,e.x,e.y-48,0,std::max(1,marks),dark,false});
    }
    // Dark first, light after: the dark stacks flash into an instant blast around the foe.
    void startSolar(Enemy& e){
        const int stacks=consumeStacks(e,Element::Dark);e.reactCd=reactionCooldown;
        eclipseFx.push_back({1,e.x,e.y-48,0,0,stacks,true});
        const int amount=20+16*stacks+player.gear.attack();
        for(auto& other:enemies)
            if(!other.dead()&&&other!=&e&&std::hypot(other.x-e.x,other.y-e.y)<150.f)damageEnemy(other,amount,DamageKind::Normal,false);
        damageEnemy(e,amount,DamageKind::Normal,false);
    }
    void stepEclipse(float dt){
        for(auto& fx:eclipseFx){
            fx.age+=dt;
            if(fx.kind!=0)continue;
            if(fx.age>=eclipsePullStart&&!fx.collapsed){
                for(auto& e:enemies){
                    const float dx=fx.x-e.x;
                    if(e.dead()||std::abs(dx)>eclipsePullRadius||std::abs(dx)<6.f)continue;
                    const float step=std::min(std::abs(dx),(260.f+40.f*e.darkStacks)*dt*(1-e.knockbackResistance()));
                    const float nx=e.x+(dx>0?step:-step);
                    if(!castleArea||std::isfinite(CastleTerrain::groundAt(castleArea,nx)))e.x=nx;
                }
            }
            if(fx.age>=eclipseCollapseAt&&!fx.collapsed){
                fx.collapsed=true;
                const int amount=40+12*fx.marks+10*fx.dark+player.gear.attack();
                for(auto& e:enemies){
                    const float d=std::hypot(e.x-fx.x,e.y-48-fx.y);
                    if(e.dead()||d>eclipseBlastRadius)continue;
                    damageEnemy(e,d<40?static_cast<int>(amount*1.3f):amount,DamageKind::Normal,false);
                    e.darkStacks=0;e.darkTimer=0;
                }
                shake=std::max(shake,.5f);
            }
        }
        eclipseFx.erase(std::remove_if(eclipseFx.begin(),eclipseFx.end(),[](const EclipseFx& f){return f.age>=(f.kind==0?eclipseEnd:.45f);}),eclipseFx.end());
    }
private:
    void melee(int damage,int bleed,float reach){
        const BodyRect strike=player.facing>0?BodyRect{player.x-9,player.y-102,player.x+reach,player.y+2}:
            BodyRect{player.x-reach,player.y-102,player.x+9,player.y+2};
        bool connected=false;
        for(auto& e:enemies){
            if(e.dead()||!enemyOverlaps(e,strike))continue;
            hit(e,damage,false,bleed);player.hitStop=std::max(player.hitStop,damage<20?.045f:.075f);
            if(player.flashExecutioner){flashEffect(LightFx::SlashHit,e.x,e.y-38);elementReact(e,tags::flashStrike);}
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
            p.meleeDamage=static_cast<int>(((p.combo==1?12:p.combo==2?15:23)+p.gear.attack())*perks.comboMul+.5f);p.meleeBleed=p.combo==3?2+perks.bleedStacksBonus:0;p.meleeReach=(p.combo==3?115:96)*p.gear.range();
        }else{p.combo=0;p.attackCooldown=.47f;p.mageCastLeft=mageCastDuration;p.mageCastDamage=11+p.gear.attack();}
    }
    void swordSkill(SwordStrike strike) {
        auto& p=player;
        const bool thrust=strike==SwordStrike::Thrust;
        cancelAirSlam();
        p.strike=strike;
        fireProcs(Trigger::OnSkill,nullptr);
        p.attackPose=thrust?thrustSkillState:sweepSkillState;
        const auto clip=motionClips[p.attackPose];
        p.meleeLeft=clip.duration*clip.contact;
        p.meleeReach=(thrust?120.f:125.f)*p.gear.range()*(thrust?1.f:perks.sweepReachMul);
        p.meleeDamage=static_cast<int>(((thrust?32:18)+p.gear.attack())*(thrust?perks.thrustMul:perks.sweepMul)+.5f);
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
    void flashEffect(LightFx kind,float x,float y){
        lightBursts.push_back({x,y,.24f,kind,player.facing,++lightBurstVisualSerial});
    }
    bool stepFlashMove(float dt,const Actions& a){
        auto& p=player;
        if(p.flashMove==FlashMove::None){
            if(!p.grounded||p.dashLeft>0||p.slamRecover>0||!canAttack())return false;
            if(a.skill&&p.skillCooldown<=0){p.flashMove=FlashMove::Rush;p.skillCooldown=3.f;}
            else if(a.skillSweep&&p.sweepReady<=0&&p.storedLightLances>0){p.flashMove=FlashMove::Command;p.sweepReady=1.f;}
            else if((a.skillBuff&&p.buffReady<=0)||(a.ultimate&&p.verdictCooldown<=0)){
                p.flashTarget=farthestLightMark();
                if(p.flashTarget==enemies.size())return false;
                if(a.ultimate&&p.verdictCooldown<=0){p.flashMove=FlashMove::Verdict;p.verdictCooldown=9.f*perks.verdictCooldownMul;}
                else {p.flashMove=FlashMove::Blink;p.buffReady=1.5f*perks.blinkCooldownMul;}
            }else if(a.attack&&p.attackCooldown<=0){p.flashMove=FlashMove::Slash;p.attackCooldown=.46f/p.basicSpeed();}
            else return false;
            p.flashAge=0;p.flashRushHits=0;p.vx=0;p.vy=0;
            if(p.flashMove==FlashMove::Slash)p.flashSlashSpeed=p.basicSpeed();
            else fireProcs(Trigger::OnSkill,nullptr);
        }
        const auto move=p.flashMove;
        const auto clip=flashClips[static_cast<int>(move)];
        const float speed=move==FlashMove::Slash?p.flashSlashSpeed:1.f;
        const float old=p.flashAge;
        p.flashAge=std::min(clip.duration,old+dt*speed);
        if(old<clip.contact&&p.flashAge>=clip.contact){
            if(move==FlashMove::Slash){
                // The authored slash clip drives the PNG sword arc in Presentation.
                // Keep the hit on its original contact frame; do not create
                // a second stationary world-space slash or rift on top of it.
                melee(15+p.gear.attack(),0,105*p.gear.range());
            }else if(move==FlashMove::Command){
                // The forward-pointing sword pose is the shared release frame for the entire volley.
                releaseStoredLances();flashEffect(LightFx::Command,p.x+p.facing*52.f,p.y-55.f);
            }else if(move==FlashMove::Blink&&!enemies[p.flashTarget].dead()){
                auto& e=enemies[p.flashTarget];
                flashEffect(LightFx::Depart,p.x,p.y-48);
                const float side=e.x>=p.x?1.f:-1.f;
                p.x=std::clamp(e.x+side*58,25.f,castleArea?castleWorldWidth()-35.f:1550.f);p.y=castleArea?CastleTerrain::groundAt(castleArea,p.x):floorY;
                p.facing=e.x>=p.x?1:-1;
                p.invulnerable=std::max(p.invulnerable,perks.blinkInvulnerability);   // Afterflash
                flashEffect(LightFx::Arrive,p.x,p.y-48);
            }else if(move==FlashMove::Verdict){
                // The off-hand signals the sky volley; target impact flashes
                // occur only as individual lances actually land.
                flashEffect(LightFx::Arrive,p.x-p.facing*24.f,p.y-57.f);
                launchVerdictLances();
            }
        }
        if(move==FlashMove::Rush){
            const float travel=std::max(0.f,std::min(.36f,p.flashAge)-std::max(clip.contact,old));
            if(travel>0){
                const float from=p.x;
                p.x=std::clamp(from+p.facing*1450*travel,25.f,castleArea?castleWorldWidth()-35.f:1550.f);
                if(static_cast<int>(old*60)%2==0)flashEffect(LightFx::Rush,p.x,p.y-47);
                const BodyRect strike{std::min(from,p.x)-18,p.y-83,std::max(from,p.x)+18,p.y+2};
                for(size_t i=0;i<enemies.size();++i){
                    auto& e=enemies[i];
                    if(e.dead()||(p.flashRushHits&(1u<<i))||!enemyOverlaps(e,strike))continue;
                    p.flashRushHits|=1u<<i;
                    hit(e,static_cast<int>((18+p.gear.attack())*perks.rushMul+.5f),false);elementReact(e,tags::flashStrike);
                    flashEffect(LightFx::RushHit,e.x,e.y-38);
                }
            }
        }
        if(p.flashAge>=clip.duration)p.flashMove=FlashMove::None;
        return true;
    }
    void stepSpace(float dt,Actions& a){
        auto& p=player;
        if(p.form!=1||p.flashExecutioner)return;
        constexpr float durations[]={0,.55f,.65f,.75f,.6f,2.f};
        constexpr float contacts[]={0,.18f,.22f,.24f,.15f,1.68f};
        if(p.spaceMove==0&&p.hurtTint<=0&&p.dashLeft<=0){
            int move=0;
            if(a.ultimate&&p.verdictCooldown<=0){move=5;p.verdictCooldown=20*perks.singularityCooldownMul;}
            else if(a.skillBuff&&p.buffReady<=0){move=4;p.buffReady=5;}
            else if(a.skillSweep&&p.sweepReady<=0){move=3;p.sweepReady=7;}
            else if(a.skill&&p.mageSkillCooldown<=0){move=2;p.mageSkillCooldown=7;}
            else if(a.attack&&p.attackCooldown<=0){move=1;p.attackCooldown=.55f;}
            if(move){p.spaceMove=move;p.spaceAge=0;p.spaceReleased=false;++p.attackSerial;
                if(move>=2)fireProcs(Trigger::OnSkill,nullptr);
                if(move==5)spaceEffects.push_back({4,p.x,p.y,0,p.facing});}
        }
        a.attack=a.skill=a.skillSweep=a.skillBuff=a.ultimate=false;
        if(p.spaceMove){
            const int move=p.spaceMove;p.spaceAge+=dt;
            if(move==5&&p.spaceAge<1.68f){
                for(auto& e:enemies)if(!e.dead()&&std::abs(e.x-p.x)<340){
                    const float amount=std::min(std::abs(p.x-e.x),(100+30*e.darkStacks)*dt*(1-e.knockbackResistance()));
                    const float nx=e.x+(e.x<p.x?amount:-amount);
                    if(!castleArea||std::isfinite(CastleTerrain::groundAt(castleArea,nx)))e.x=nx;
                }
            }
            if(!p.spaceReleased&&p.spaceAge>=contacts[move]){
                p.spaceReleased=true;
                if(move==1){
                    const int boltDamage=static_cast<int>((spaceBoltDamage+p.gear.attack())*perks.boltMul+.5f);
                    projectiles.push_back({p.x+p.facing*30,p.y-48,p.facing,boltDamage,false});projectiles.back().voidBolt=true;
                    if(perks.twinBolt){projectiles.push_back({p.x+p.facing*30,p.y-70,p.facing,boltDamage,false});projectiles.back().voidBolt=true;}   // Twin Bolt
                }
                if(move==2){p.spaceField=3.f*perks.fieldDurationMul;p.spaceFieldTick=0;}
                if(move==3){
                    spaceEffects.push_back({2,p.x+p.facing*24,p.y,0,p.facing});
                    const BodyRect area{p.facing>0?p.x:p.x-245,p.y-130,p.facing>0?p.x+245:p.x,p.y+15};
                    for(auto& e:enemies)if(!e.dead()&&enemyOverlaps(e,area)){elementHit(e,static_cast<int>((spaceSpikesDamage+p.gear.attack())*perks.spikesMul+.5f),tags::rulerSpikes);voidExecute(e);}
                }
                if(move==4){spaceEffects.push_back({3,p.x,p.y,0,p.facing});
                    for(auto& e:enemies)if(!e.dead()&&std::hypot(e.x-p.x,e.y-p.y)<180*perks.repulseRadiusMul){elementHit(e,12+p.gear.attack(),tags::rulerRepulse);e.knockback=(e.x<p.x?-1:1)*400*(1.f+voidKnockbackPerStack*e.darkStacks)*(1-e.knockbackResistance());}}
                if(move==5){   // the singularity eats the dark stacks on a foe: +30 % damage per stack
                    for(auto& e:enemies)if(!e.dead()&&std::hypot(e.x-p.x,e.y-p.y)<340){
                        const int stacks=consumeStacks(e,Element::Dark);
                        elementHit(e,static_cast<int>((spaceSingularityDamage+p.gear.attack())*perks.singularityMul*(1.f+.3f*stacks)+.5f),tags::rulerSingularity);
                    }
                }
            }
            if(p.spaceAge>=durations[move])p.spaceMove=0;
        }
        if(p.spaceField>0){
            p.spaceField=down(p.spaceField,dt);p.spaceFieldTick+=dt;
            while(p.spaceFieldTick>=.5f){p.spaceFieldTick-=.5f;for(auto& e:enemies)if(!e.dead()&&std::hypot(e.x-p.x,e.y-p.y)<150){damageEnemy(e,6,DamageKind::Normal,true);addStacks(e,tags::rulerField.element,tags::rulerField.stacks);}}
        }
    }
    void stepPlayer(float dt,Actions a){
        auto& p=player;
        p.dashLeft=down(p.dashLeft,dt);p.dashCooldown=down(p.dashCooldown,dt);p.attackCooldown=down(p.attackCooldown,dt);
        const float skillDt=dt*stats.cooldownRate;   // Haste: skills and ultimates only, never dash or swap
        p.skillCooldown=down(p.skillCooldown,skillDt);
        p.mageSkillCooldown=down(p.mageSkillCooldown,skillDt);
        p.sweepReady=down(p.sweepReady,skillDt);
        p.buffReady=down(p.buffReady,skillDt);
        p.preparation=down(p.preparation,dt);p.swapCooldown=down(p.swapCooldown,dt);p.verdictCooldown=down(p.verdictCooldown,skillDt);p.invulnerable=down(p.invulnerable,dt);
        p.attackFlash=down(p.attackFlash,dt);p.comboWindow=down(p.comboWindow,dt);p.swapFlash=down(p.swapFlash,dt);
        p.flashBoost=down(p.flashBoost,dt);
        if(p.hitStop>0){p.hitStop=down(p.hitStop,dt);return;}
        if(p.handicapped){
            // Lost power after rebirth: skills unlock one by one, no class swap.
            if(p.unlockedSkills<1)a.skill=false;
            if(p.unlockedSkills<2)a.skillSweep=false;
            if(p.unlockedSkills<3)a.skillBuff=false;
            if(p.unlockedSkills<4)a.ultimate=false;
            a.swap=false;
            if(p.dashLocked)a.dash=false;
        }
        // Swap only toggles the two equipped jobs. Verdict is a separate action.
        if(a.swap&&p.swapCooldown<=0&&p.slamRecover<=0&&
           (!p.flashExecutioner||p.flashMove==FlashMove::None)){
            const bool mayEntryAttack=canAttack();
            cancelAirSlam();cancelMageCast();p.meleeLeft=0;
            p.activeJob=1-p.activeJob;
            const int next=p.equippedJobs[p.activeJob];
            p.flashExecutioner=next==2;
            p.form=next==1?1:0;
            refreshStats();
            p.flashMove=FlashMove::None;p.flashAge=0;p.spaceMove=0;p.spaceField=0;p.spaceFlightBase=p.y;
            p.swapCooldown=1.f;p.attackFlash=.22f;p.combo=0;
            p.comboWindow=0;p.attackPose=0;p.swapFlash=.24f;
            if(mayEntryAttack){
                ++p.attackSerial;
                if(p.flashExecutioner)flashEffect(LightFx::Arrive,p.x,p.y-48);
                else if(p.form==0)melee(13,false,112);
                else {p.spaceMove=1;p.spaceAge=0;p.spaceReleased=false;}
            }
            return;
        }
        stepSpace(dt,a);
        if(p.flashExecutioner&&stepFlashMove(dt,a))return;
        if(p.flashExecutioner){
            a.skill=a.skillSweep=a.skillBuff=a.ultimate=false;
            if(p.grounded)a.attack=false;
        }
        p.takeoffLock=down(p.takeoffLock,dt);
        p.slamRecover=down(p.slamRecover,dt);p.slamImpact=down(p.slamImpact,dt);
        if(p.comboWindow<=0)p.combo=0;
        p.coyote=p.grounded?.11f:down(p.coyote,dt);p.jumpBuffer=a.jump?.13f:down(p.jumpBuffer,dt);
        float axis=static_cast<float>(a.right)-static_cast<float>(a.left);
        if(axis!=0&&!p.airSlam&&p.slamRecover<=0&&(p.meleeLeft<=0||p.strike==SwordStrike::Basic))p.facing=axis>0?1:-1;
        if(a.dash&&p.dashCooldown<=0&&p.slamRecover<=0){cancelAirSlam();cancelMageCast();p.meleeLeft=0;p.dashLeft=.18f;p.dashCooldown=.85f;p.invulnerable=std::max(p.invulnerable,.18f);fireProcs(Trigger::OnDash,nullptr);}
        if(p.dashLeft>0){p.vx=1150*p.facing*p.gear.speed();p.vy=0;}
        else {
            p.vx=axis*340*p.gear.speed()*p.movementMultiplier();
            if(p.form==1&&!p.flashExecutioner){
                if(p.grounded)p.spaceFlightBase=p.y;
                if(!a.jumpHeld&&p.grounded)p.spaceExhausted=false;
                const bool flying=a.jumpHeld&&p.spaceFuel>0&&!p.spaceExhausted;
                if(flying){p.spaceFuel=std::max(0.f,p.spaceFuel-dt*.25f);if(p.spaceFuel<=0)p.spaceExhausted=true;p.vy=p.y>p.spaceFlightBase-160?-120.f:0.f;}
                else if(!p.grounded)p.vy=110.f;
                else {p.vy=0;p.spaceFuel=std::min(1.f,p.spaceFuel+dt*.2f);}
                if(p.spaceMove==5)p.vx=0;
            }else p.vy+=1250*dt;
            if(p.form!=1&&(p.jumpBuffer>0||(a.jumpHeld&&p.grounded))&&p.coyote>0&&!p.airSlam&&p.slamRecover<=0)
                launchJump();
        }
        const bool attackAllowed=canAttack();
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

        const float nextX=std::clamp(p.x+p.vx*dt,25.f,castleArea?castleWorldWidth()-35.f:1550.f);
        const float nextY=p.y+p.vy*dt;
        float contact=crossedGround(p.x,p.y,nextX,nextY);
        if(castleArea>0 && p.grounded && p.vy>=0.f){
            const float nextBase=CastleTerrain::standingAt(castleArea,nextX,p.y);
            if(std::isfinite(nextBase)&&std::fabs(nextBase-p.y)<=12.f)
                contact=std::min(contact,nextBase);
        }
        // A change in ground height while walking a slope is not a landing.
        // Only count the transition from airborne to grounded; otherwise
        // sloped areas retrigger the landing animation/effect every tick.
        const bool landed=std::isfinite(contact)&&!p.grounded;
        if(p.grounded&&!std::isfinite(contact))p.meleeLeft=0;
        p.x=nextX;
        p.grounded=std::isfinite(contact);
        p.y=p.grounded?contact:nextY;
        if(castleArea>0&&p.y>CastleTerrain::pitTriggerY){
            // ECLIPSE_CASTLE_PIT_DAMAGE_V33: percent max HP, no armor mitigation.
            const int fallDamage=std::max(1,(p.maxHealth()*CastleTerrain::pitDamagePercent+99)/100);
            holdHealthTrail(p,.12f);
            p.health=std::max(playerHealthFloor,p.health-fallDamage);
            p.hurtTint=.18f;
            p.invulnerable=std::max(p.invulnerable,CastleTerrain::pitInvulnerabilitySeconds);
            const auto& area=CastleTerrain::profile(castleArea);
            p.x=area.spawnX;p.y=area.spawnY;
            p.vx=p.vy=0.f;p.grounded=true;
            p.jumpBuffer=p.coyote=0.f;
            p.airSlam=false;p.slamAge=0.f;p.slamRecover=0.f;
            p.dashLeft=0.f;p.meleeLeft=0.f;
            ++pitRespawnSerial;
            if(p.health<=0){if(suppressGameOver)defeatPlayer();else{ended=true;won=false;inventory=false;runLost=true;}}
        }
        if(p.grounded){
            p.vy=0;
            if(landed){
                ++p.landingSerial;
                if(p.airSlam)landAirSlam();
                if(p.form!=1&&(a.jumpHeld||p.jumpBuffer>0)&&p.dashLeft<=0&&p.slamRecover<=0)
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
    // ---- Demon King move engine -------------------------------------------------------------
    float groundUnder(float x) const {return castleArea?CastleTerrain::groundAt(castleArea,x):floorY;}
    bool standable(float x) const {return std::isfinite(groundUnder(x));}
    float clampWorld(float x) const {return std::clamp(x,60.f,(castleArea?castleWorldWidth():worldWidth)-60.f);}
    static int bossDamage(const Enemy& e,float mul){return std::max(1,static_cast<int>(e.damage*mul+.5f));}
    void spawnPillar(float x,float delay,float radius,int damage,BossFx fx){
        BossHazard h;h.kind=HazardKind::Pillar;h.fx=fx;h.x=clampWorld(x);
        const float g=groundUnder(h.x);h.y=std::isfinite(g)?g:floorY;
        h.delay=delay;h.radius=radius;h.damage=damage;hazards.push_back(h);
    }
    void spawnWave(float x,int dir,float speed,float range,int damage,BossFx fx){
        BossHazard h;h.kind=HazardKind::Wave;h.fx=fx;h.x=h.startX=x;h.dir=dir;h.speed=speed;h.range=range;h.damage=damage;
        const float g=groundUnder(x);h.y=std::isfinite(g)?g:floorY;hazards.push_back(h);
    }
    // Schedule a pixelfx layer so its impact frame lands `untilImpact` seconds from now.
    void emitFx(BossFx kind,float x,float y,int facing,float untilImpact,float scale=2.f){
        const auto& sheet=bossFxSheets[static_cast<int>(kind)];
        bossFx.push_back({kind,x,y,facing,sheet.peak/sheet.fps-untilImpact,scale,0.f});
    }
    // A looping layer (wind-up energy, floor runes, a rift) that starts `after` seconds from now and lasts `life`.
    void emitFxLoop(BossFx kind,float x,float y,int facing,float after,float life,float scale=2.f){
        bossFx.push_back({kind,x,y,facing,-after,scale,life});
    }
    // World position of the sword tip on the frame that shows the hit, for a boss standing at `x`.
    void tipWorld(const Enemy& e,const BossStepDef& d,float x,float& tx,float& ty) const {
        const int form=e.kind==DemonKingArmored?0:1,frame=bossContactFrame(d);
        const auto* t=bossTips[form][static_cast<int>(d.motion)][frame];
        const float scale=enemySpec(e.kind).scale;
        tx=x+e.facing*(t[0]-128)*scale;ty=e.y+(t[1]-236)*scale;
    }
    // Starts the current step of e.move: sets timing/animation window, places telegraphed hazards and
    // schedules the separate pixelfx layers of this motion: wind-up energy, the strike itself, the
    // shock that follows and the dust or debris it leaves. Each layer is its own sprite sheet.
    void bossStartStep(Enemy& e){
        const BossStepDef d=bossStepDef(e.move,e.step);
        e.attackAge=0;e.moveDur=d.duration;e.moveContact=d.contact;e.animFrom=d.animFrom;e.motion=static_cast<int>(d.motion);
        e.hitDone=false;e.burstAttack=d.action==BossAction::Burst;e.attackFromX=e.x;e.attackToX=e.x;
        e.facing=player.x>=e.x?1:-1;
        const float gapX=std::abs(player.x-e.x),ct=d.duration*d.contact;
        const int dmg=bossDamage(e,d.damageMul);
        const bool armored=e.kind==DemonKingArmored;
        float tx=0,ty=0;
        auto chargeUp=[&](float until,float scale=2.f){            // energy gathering on the king until the hit
            emitFxLoop(BossFx::SpellCharge,e.x+e.facing*30.f,e.y-90.f*scale/2.f,e.facing,0.f,until,scale);};
        auto crash=[&](float x,float until,float scale,bool screen){  // shock ring, dust and blast where something lands
            emitFx(BossFx::Thunderclap,x,e.y,e.facing,until,scale);
            emitFx(BossFx::Landing,x-60.f,e.y,-1,until,1.6f);emitFx(BossFx::Landing,x+60.f,e.y,1,until,1.6f);
            if(screen)emitFx(BossFx::ImpactFrame,0,0,1,until,4.f);
        };
        switch(d.action){
        case BossAction::DashSlash:{
            const float travel=std::min(std::abs(d.travel),std::max(0.f,gapX-50.f));
            e.attackToX=clampWorld(e.x+e.facing*travel);if(!standable(e.attackToX))e.attackToX=e.x;
            emitFx(BossFx::Landing,e.x,e.y,e.facing,0.f,1.4f);                       // push-off dust
            slashFx(e,d,e.attackToX,ct);
            emitFx(BossFx::Landing,e.attackToX,e.y,e.facing,ct,1.4f);break;}
        case BossAction::Melee:
            if(armored)chargeUp(ct*.95f,2.2f);
            slashFx(e,d,e.x,ct);
            if(armored){tipWorld(e,d,e.x,tx,ty);crash(tx,ct,2.f,true);}
            break;
        case BossAction::Sweep:
            tipWorld(e,d,e.x,tx,ty);
            emitFxLoop(BossFx::RuneCircle,e.x,e.y,e.facing,0.f,ct,2.2f);              // low stance, rune ring under his feet
            emitFx(BossFx::Reap,tx-e.facing*40.f,e.y,e.facing,ct,2.2f);
            emitFx(BossFx::Flurry,tx-e.facing*80.f,e.y-30.f,e.facing,ct,2.f);
            emitFx(BossFx::Thunderclap,tx,e.y,e.facing,ct,1.8f);
            emitFx(BossFx::Landing,tx,e.y,e.facing,ct,1.8f);emitFx(BossFx::Landing,tx-e.facing*140.f,e.y,e.facing,ct*.9f,1.8f);
            break;
        case BossAction::Quake:
            emitFxLoop(BossFx::RuneCircle,e.x,e.y,e.facing,0.f,ct,2.4f);              // rune ring while he drives the sword down
            tipWorld(e,d,e.x,tx,ty);
            emitFx(BossFx::Spike,tx,e.y,e.facing,ct);
            emitFx(BossFx::DarkBlast,tx,e.y,e.facing,ct,1.2f);
            crash(tx,ct,2.4f,true);break;
        case BossAction::Leap:
            e.attackToX=clampWorld(player.x);if(!standable(e.attackToX))e.attackToX=e.x;
            emitFx(BossFx::Landing,e.x,e.y,e.facing,.3f*d.duration,2.f);                 // take-off
            emitFxLoop(BossFx::RuneCircle,e.attackToX,e.y,e.facing,0.f,ct,2.2f);      // landing mark
            emitFx(BossFx::Slam,e.attackToX,e.y,e.facing,ct);
            emitFx(BossFx::DarkBlast,e.attackToX,e.y,e.facing,ct,1.5f);
            crash(e.attackToX,ct,2.6f,true);break;
        case BossAction::Dive:
            e.attackToX=clampWorld(player.x);if(!standable(e.attackToX))e.attackToX=e.x;
            emitFxLoop(BossFx::Portal,e.x,e.y,e.facing,0.f,.3f*d.duration,2.f);       // rift he springs from
            emitFx(BossFx::Landing,e.x,e.y,e.facing,.25f*d.duration,2.f);
            emitFxLoop(BossFx::RuneCircle,e.attackToX,e.y,e.facing,0.f,ct,2.0f);
            emitFx(BossFx::Dirt,e.attackToX,e.y,e.facing,ct);
            emitFx(BossFx::DarkBlast,e.attackToX,e.y,e.facing,ct,1.4f);
            crash(e.attackToX,ct,2.4f,true);break;
        case BossAction::Backstep:
            e.attackToX=clampWorld(e.x-e.facing*std::abs(d.travel));if(!standable(e.attackToX))e.attackToX=e.x;
            emitFx(BossFx::Landing,e.x,e.y,e.facing,.05f,1.8f);                          // push-off
            emitFx(BossFx::Landing,e.attackToX,e.y,e.facing,d.duration*.92f,1.6f);
            emitFx(BossFx::Landing,e.attackToX,e.y,e.facing,d.duration*.92f,1.8f);break;
        case BossAction::Charge:{
            e.attackToX=clampWorld(e.x+e.facing*std::abs(d.travel));
            emitFxLoop(BossFx::RuneCircle,e.x,e.y,e.facing,0.f,.14f*d.duration,2.2f); // brace
            for(int i=0;i<6;++i){                                                      // dust kicking up along the run
                const float t=.14f+.41f*(i+1)/7.f;
                const float x=e.attackFromX+(e.attackToX-e.attackFromX)*((t-.14f)/.41f);
                emitFx(BossFx::Landing,x,e.y,e.facing,d.duration*t,1.4f);
                emitFx(BossFx::Landing,x,e.y,-e.facing,d.duration*t,1.5f);
            }
            emitFx(BossFx::Slam,e.attackToX,e.y,e.facing,d.duration*.55f,1.8f);
            emitFx(BossFx::DarkBlast,e.attackToX,e.y,e.facing,d.duration*.55f,1.2f);
            crash(e.attackToX,d.duration*.55f,2.2f,true);break;}
        case BossAction::Judgement:
            spawnPillar(player.x,1.25f,130,dmg,BossFx::Circle);spawnPillar(player.x-200,1.5f,130,dmg,BossFx::Circle);
            spawnPillar(player.x+200,1.75f,130,dmg,BossFx::Circle);
            emitFx(BossFx::HexSigil,e.x,e.y-240.f,e.facing,.35f*d.duration,2.f);       // sigil drawn above his head
            emitFxLoop(BossFx::RuneCircle,e.x,e.y,e.facing,0.f,ct,2.4f);
            chargeUp(ct,2.4f);
            for(const auto& h:hazards)if(h.age==0.f&&h.kind==HazardKind::Pillar&&h.fx==BossFx::Circle){
                emitFx(BossFx::HolyPillar,h.x,h.y,1,h.delay,1.6f);emitFx(BossFx::Thunderclap,h.x,h.y,1,h.delay,1.6f);}
            break;
        case BossAction::Pillar:{
            spawnPillar(player.x,.6f,100,dmg,BossFx::Lightning);
            chargeUp(ct+.2f,1.6f);
            emitFx(BossFx::Thunderclap,hazards.back().x,hazards.back().y,1,.6f,1.4f);break;}
        case BossAction::Rain:{
            static constexpr float offsets[5]={0,-170,170,-340,340};
            emitFx(BossFx::HexSigil,e.x,e.y-240.f,e.facing,.3f*d.duration,2.f);
            chargeUp(ct,2.0f);
            for(int i=0;i<5;++i){
                spawnPillar(player.x+offsets[i],.6f+i*.18f,90,dmg,BossFx::Stars);
                emitFx(BossFx::Thunderclap,hazards.back().x,hazards.back().y,1,.6f+i*.18f,1.2f);
            }
            break;}
        case BossAction::Burst:
            emitFxLoop(BossFx::VoidOrb,e.x,e.y-95.f,e.facing,0.f,ct,2.0f);            // orb condensing around him
            chargeUp(ct,2.2f);
            emitFx(BossFx::Burst,e.x,e.y,e.facing,ct);
            emitFx(BossFx::DarkBlast,e.x,e.y,e.facing,ct,1.8f);
            crash(e.x,ct,3.0f,true);break;
        case BossAction::Blink:{
            const float side=player.x>=e.x?1.f:-1.f;float to=clampWorld(player.x+side*95.f);
            if(!standable(to))to=clampWorld(player.x-side*95.f);
            emitFx(BossFx::BlinkOut,e.x,e.y,e.facing,0.f);emitFxLoop(BossFx::Portal,e.x,e.y,e.facing,0.f,.3f,1.6f);
            if(standable(to)){
                blinkFromX=e.x;blinkToX=to;blinkY=e.y;blinkTime=.4f;++kingBlinkSerial;
                emitFx(BossFx::BlinkIn,to,e.y,e.facing,.12f);emitFxLoop(BossFx::Portal,to,e.y,e.facing,.1f,.35f,1.6f);
                emitFx(BossFx::Landing,to,e.y,e.facing,.14f,1.6f);
                e.x=to;e.attackFromX=e.attackToX=to;
            }
            e.facing=player.x>=e.x?1:-1;break;}
        case BossAction::Wave:
            tipWorld(e,d,e.x,tx,ty);
            emitFx(BossFx::Flurry,tx-e.facing*60.f,e.y,e.facing,ct,2.f);
            emitFx(BossFx::Landing,e.x+e.facing*40.f,e.y,e.facing,ct,1.6f);break;
        }
    }
    // The cut effect that matches the step's body motion, anchored on where the sword ends up.
    void slashFx(const Enemy& e,const BossStepDef& d,float standX,float contactTime){
        float tx=0,ty=0;tipWorld(e,d,standX,tx,ty);
        switch(d.motion){
        case BossMotion::Twist:
            emitFx(BossFx::Spin,standX,e.y,e.facing,contactTime);
            emitFx(BossFx::Flurry,standX+e.facing*60.f,e.y,e.facing,contactTime,2.f);break;
        case BossMotion::Rise:
            emitFx(BossFx::CutUp,tx-e.facing*70.f,e.y,e.facing,contactTime);
            emitFx(BossFx::Flurry,tx-e.facing*40.f,e.y-50.f,e.facing,contactTime,1.8f);break;
        case BossMotion::Attack:
            if(e.kind==DemonKingArmored)emitFx(BossFx::Hammer,tx,e.y,e.facing,contactTime);
            else{emitFx(BossFx::Thrust,standX+e.facing*30.f,e.y,e.facing,contactTime);
                 emitFx(BossFx::Landing,tx,e.y,e.facing,contactTime,1.4f);}
            break;
        default:break;
        }
    }
    void bossAi(Enemy& e,float dt,float& vx){
        const auto spec=enemySpec(e.kind);
        const bool phase2=e.kind==DemonKingTrue;
        if(e.attackAge<0){
            e.move=BossMove::None;e.motion=0;
            const float dx=player.x-e.x;
            if(std::abs(dx)>1)e.facing=dx>0?1:-1;
            const float gap=enemyFrontGap(e,player);
            if(e.attackTimer<=0){
                const bool enraged=phase2&&e.health*10<=e.maxHealth*6;
                const BossMove m=bossPickMove(phase2,e.moveCounter,gap,enraged,e.lastMove);
                ++e.moveCounter;bossStartMove(e,m);
                return;
            }
            const float want=phase2?110.f:spec.engageGap;
            const float chase=e.bleedStacks>=bleedSlowStacks?.8f:1.f;
            if(gap>want)vx=e.facing*e.speed*chase;
            return;
        }
        const BossStepDef d=bossStepDef(e.move,e.step);
        const float before=e.attackAge;e.attackAge+=dt;
        const float u=std::min(1.f,e.attackAge/d.duration);
        const float contactTime=d.duration*d.contact;
        const bool crossed=before<contactTime&&e.attackAge>=contactTime;
        const BodyRect playerBody{player.x-14,player.y-86,player.x+14,player.y};
        const int damage=bossDamage(e,d.damageMul);
        auto ease=[](float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);};
        auto moveTo=[&](float x){if(standable(x))e.x=x;};
        switch(d.action){
        case BossAction::DashSlash:
            moveTo(e.attackFromX+(e.attackToX-e.attackFromX)*ease(e.attackAge/std::max(.01f,contactTime)));
            if(crossed){
                if(overlaps(enemyAttackBounds(e),playerBody))damagePlayer(damage);
                if(d.damageMul>=1.2f)shake=std::max(shake,.35f);
            }
            break;
        case BossAction::Melee:
            if(crossed){
                if(overlaps(enemyAttackBounds(e),playerBody))damagePlayer(damage);
                shake=std::max(shake,d.damageMul>=1.4f?.7f:.25f);if(d.damageMul>=1.4f)flash=std::max(flash,.55f);
            }
            break;
        case BossAction::Sweep:
            if(crossed){
                const float reach=300.f;
                const BodyRect zone=e.facing>0?BodyRect{e.x-40,e.y-70,e.x+reach,e.y+8}:BodyRect{e.x-reach,e.y-70,e.x+40,e.y+8};
                if(overlaps(zone,playerBody))damagePlayer(damage);
                shake=std::max(shake,.5f);
            }
            break;
        case BossAction::Leap:case BossAction::Dive:{
            // Leap: crouch, rise, hang, land at the marked spot. Dive: higher arc and a faster fall.
            const bool dive=d.action==BossAction::Dive;
            const float from=dive?.25f:.3f,to=dive?.88f:.9f,height=dive?230.f:150.f,radius=dive?150.f:170.f;
            const float t=ease((u-from)/(to-from));
            moveTo(e.attackFromX+(e.attackToX-e.attackFromX)*t);
            const float g=groundUnder(e.x);
            e.y=(std::isfinite(g)?g:floorY)-4*t*(1-t)*height;
            if(crossed){
                if(std::isfinite(g))e.y=g;
                if(std::abs(player.x-e.x)<radius&&player.y>e.y-120.f)damagePlayer(damage);
                shake=std::max(shake,.9f);flash=std::max(flash,.6f);
            }
            break;}
        case BossAction::Quake:
            if(crossed){
                spawnWave(e.x,-1,520,900,damage,BossFx::Spikes);spawnWave(e.x,1,520,900,damage,BossFx::Spikes);
                shake=std::max(shake,.8f);flash=std::max(flash,.5f);
            }
            break;
        case BossAction::Charge:{
            const float t=ease((u-.14f)/.41f);
            const float target=e.attackFromX+(e.attackToX-e.attackFromX)*t;
            if(u>=.14f&&standable(target))e.x=target;
            if(u>=.14f&&u<=.6f&&!e.hitDone&&overlaps(enemyAttackBounds(e),playerBody)){
                e.hitDone=true;damagePlayer(damage);shake=std::max(shake,.8f);
            }
            break;}
        case BossAction::Burst:
            if(crossed){
                if(std::abs(player.x-e.x)<kingBurstRadius&&std::abs(player.y-e.y)<150.f)damagePlayer(damage);
                ++kingBurstSerial;kingBurstX=e.x;kingBurstY=e.y;shake=std::max(shake,.8f);flash=std::max(flash,.6f);
            }
            break;
        case BossAction::Backstep:{
            const float t=ease(u);
            moveTo(e.attackFromX+(e.attackToX-e.attackFromX)*t);
            break;}
        case BossAction::Wave:
            if(crossed){
                spawnWave(e.x+e.facing*40,e.facing,600,800,damage,BossFx::VoidBolt);shake=std::max(shake,.25f);
            }
            break;
        case BossAction::Pillar:case BossAction::Rain:case BossAction::Judgement:case BossAction::Blink:break;
        }
        const bool airborne=d.action==BossAction::Leap||d.action==BossAction::Dive;
        if(d.action==BossAction::Backstep){
            // The backflip really leaves the ground.
            const float g=groundUnder(e.x),t=ease(u);
            if(std::isfinite(g))e.y=g-4*t*(1-t)*110.f;
        }else if(!airborne){const float g=groundUnder(e.x);if(std::isfinite(g))e.y=g;}
        if(e.attackAge>=d.duration){
            e.attackAge=-1;e.burstAttack=false;
            if(++e.step<bossMoveSteps(e.move))bossStartStep(e);
            else{e.attackTimer=bossMoveCooldown(e.move);e.move=BossMove::None;e.animFrom=0;e.moveDur=0;e.step=0;e.motion=0;}
        }
    }
    void stepHazards(float dt){
        for(auto& h:hazards){
            h.age+=dt;
            if(h.kind==HazardKind::Pillar){
                if(!h.detonated&&h.age>=h.delay){
                    h.detonated=true;shake=std::max(shake,.5f);++hazardSerial;
                    if(std::abs(player.x-h.x)<h.radius&&player.y>=h.y-240.f)damagePlayer(h.damage);
                }
            }else{
                if(!h.spent){
                    h.x+=h.dir*h.speed*dt;
                    const float g=groundUnder(h.x);if(std::isfinite(g))h.y=g;
                    if(!h.hit&&std::abs(player.x-h.x)<26.f&&player.y>h.y-h.height){h.hit=true;damagePlayer(h.damage);}
                    const float limit=castleArea?castleWorldWidth():worldWidth;
                    if(std::abs(h.x-h.startX)>h.range||h.x<-60||h.x>limit+60){h.spent=true;h.spentAge=h.age;}
                }
            }
        }
        hazards.erase(std::remove_if(hazards.begin(),hazards.end(),[&](const BossHazard& h){
            if(h.kind==HazardKind::Pillar)return h.age>=h.delay+hazardPillarLinger;
            return h.spent&&h.age>h.spentAge+1.4f;   // the effect layer finishes before the hazard is removed
        }),hazards.end());
    }
    void morphKing(Enemy& e){
        // Armor falls away: the same body continues as the lean true form.
        e.kind=DemonKingTrue;e.phase2=true;e.unbeatable=kingUnbeatable;
        e.maxHealth=std::max(1,static_cast<int>(enemyHealth(e.kind,castleArea)*kingPower+.5f));
        e.health=e.maxHealth;e.trailingHealth=float(e.health);e.healthTrailHold=0;
        e.damage=std::max(1,static_cast<int>(enemyDamage(e.kind,castleArea)*kingDamageScale+.5f));
        e.speed=enemySpeed(e.kind,castleArea);e.attackAge=-1;e.attackTimer=1.4f;e.attackCount=0;
        e.move=e.lastMove=BossMove::None;e.step=e.moveCounter=e.animFrom=0;e.moveDur=0;e.burstAttack=false;
        e.knockback=0;e.bleedStacks=0;e.bleed=0;e.burn=0;e.hitFlash=0;
        e.y=castleArea?CastleTerrain::groundAt(castleArea,e.x):floorY;
        ++kingPhase2Serial;
    }
    void stepDummy(Enemy& e,float dt){
        e.lightMark=down(e.lightMark,dt);e.hitFlash=down(e.hitFlash,dt);e.reactionFlash=down(e.reactionFlash,dt);
        e.stagger=down(e.stagger,dt);e.knockback=0;
        e.facing=player.x>=e.x?1:-1;
        stepHealthTrail(e,e.maxHealth,dt);
    }
    void stepEnemy(Enemy& e,float dt){
        if(e.dead())return;
        e.reactCd=down(e.reactCd,dt);
        if(e.darkStacks>0){e.darkTimer=down(e.darkTimer,dt);if(e.darkTimer<=0)e.darkStacks=0;}
        if(e.lightMark<=0)e.lightStacks=0;
        const float oldEnemyX=e.x;
        if(e.wake>0){e.wake=down(e.wake,dt);return;}
        if(e.phaseShift>0){
            e.phaseShift=down(e.phaseShift,dt);
            if(e.phaseShift<=0)morphKing(e);
            return;
        }
        if(e.dummy&&!e.fights){stepDummy(e,dt);return;}
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
        // Void stacks, 5: erosion. A foe stops dead (bosses keep moving) and the void eats it from inside.
        if(e.darkStacks>=stackMax){
            e.erodeTick+=dt;
            if(e.erodeTick>=.5f){e.erodeTick-=.5f;damageEnemy(e,std::max(3,e.maxHealth/50),DamageKind::Normal,true);}
            if(!e.boss&&!e.dead()){e.knockback=0;e.travelPhase=0;return;}
        }else e.erodeTick=0;
        if(e.dead())return;
        const float dx=player.x-e.x;float vx=0;
        const auto spec=enemySpec(e.kind);
        if(e.kind>=DemonKingArmored){bossAi(e,dt,vx);}
        else if(e.attackAge>=0){
            const float before=e.attackAge;e.attackAge+=dt;
            const float phase=std::min(1.f,e.attackAge/spec.attackDuration);
            if(spec.behavior==EnemyPounce){
                // Commit to a destination at takeoff; the player can dodge it.
                const float t=std::clamp((phase-.22f)/(.68f-.22f),0.f,1.f);
                const float eased=t*t*(3-2*t);
                e.x=e.attackFromX+(e.attackToX-e.attackFromX)*eased;
                const float takeoffY=castleArea?CastleTerrain::groundAt(castleArea,e.attackFromX):floorY;
                e.y=(std::isfinite(takeoffY)?takeoffY:floorY)-4*t*(1-t)*120.f;
            }else{
                const float lunge=std::sin(phase*3.14159265f);
                e.x=e.attackFromX+e.facing*std::max(18.f,spec.attackReach*.65f)*lunge*lunge;
            }
            const float contact=spec.attackDuration*spec.contact;
            if(before<contact&&e.attackAge>=contact){
                const BodyRect playerBody{player.x-14,player.y-86,player.x+14,player.y};
                if(overlaps(enemyAttackBounds(e),playerBody))damagePlayer(e.damage);
            }
            if(e.attackAge>=spec.attackDuration){e.attackAge=-1;e.y=castleArea?CastleTerrain::groundAt(castleArea,e.x):floorY;}
        }else{
            if(std::abs(dx)>1)e.facing=dx>0?1:-1;
            const float gap=enemyFrontGap(e,player);
            const float chaseScale=e.bleedStacks>=bleedSlowStacks?.8f:1.f;
            if(spec.behavior==EnemyRanged){
                const float rangedReach=spec.attackReach*spec.scale;
                if(gap>rangedReach)vx=e.facing*e.speed*chaseScale;
                else if(gap<spec.engageGap)vx=-e.facing*e.speed*.65f*chaseScale;
                else if(e.attackTimer<=0){e.attackAge=0;e.attackTimer=spec.cooldown;e.attackFromX=e.x;e.attackToX=e.x;++e.attackCount;}
            }else if(gap>spec.engageGap)vx=e.facing*e.speed*chaseScale;
            else if(e.attackTimer<=0&&std::abs(player.y-e.y)<120){
                e.attackAge=0;e.attackTimer=spec.cooldown;e.attackFromX=e.x;++e.attackCount;
                e.attackToX=e.x+e.facing*std::clamp(std::abs(dx),60.f,240.f);
            }
        }
        if(player.form==1&&!player.flashExecutioner&&player.spaceField>0&&std::hypot(e.x-player.x,e.y-player.y)<150)vx*=.7f;
        // Void stacks, 3-4: slow. Slower walking and slower attack cooldowns.
        if(e.darkStacks>=3&&e.darkStacks<stackMax){vx*=.6f;e.attackTimer+=dt*.4f;}
        e.travelPhase=std::fmod(e.travelPhase+std::abs(vx)*dt/64.f,1.f);
        e.x+=(vx+e.knockback)*dt;
        if(castleArea>0){
            const float foot=CastleTerrain::groundAt(castleArea,e.x);
            if(!std::isfinite(foot)){e.x=oldEnemyX;e.knockback=0.f;}
            const float safeFoot=CastleTerrain::groundAt(castleArea,e.x);
            const bool bossLeaping=e.kind>=DemonKingArmored&&e.attackAge>=0.f&&(e.move==BossMove::LeapSlam||e.move==BossMove::AerialDive||e.move==BossMove::PillarChain||e.move==BossMove::WaveVolley||e.move==BossMove::VoidRain);
            if(std::isfinite(safeFoot)&&!bossLeaping&&!(spec.behavior==EnemyPounce&&e.attackAge>=0.f))e.y=safeFoot;
        }
        // World clipping follows the same visible silhouette as hit detection.
        const auto body=enemyBounds(e);
        if(body.left<0)e.x-=body.left;
        const float rightEdge=castleArea?castleWorldWidth():worldWidth;
        if(body.right>rightEdge)e.x-=body.right-rightEdge;
    }
    void stepLightLances(float dt){
        lightLances.erase(std::remove_if(lightLances.begin(),lightLances.end(),[&](LightLance& lance){
            if((lance.life-=dt)<=0)return true;
            const auto& target=enemies[lance.target];
            if(lance.delay<=0)lance.visualFlightAge+=dt; // Cosmetic frame clock only.
            if(lance.seeking){
                if(target.dead())return true;
                const auto bounds=enemyBounds(target);
                const float aimX=target.x-lance.x;
                const float aimY=(bounds.top+bounds.bottom)*.5f-lance.y;
                lance.angle=std::atan2(aimY,aimX)*57.29578f;
                if(lance.delay>0){lance.delay=std::max(0.f,lance.delay-dt);return false;}
                const float tx=target.x,ty=(bounds.top+bounds.bottom)*.5f;
                const float dx=tx-lance.x,dy=ty-lance.y;
                const float distance=std::hypot(dx,dy);
                const float movement=(lance.verdict?2700.f:2100.f)*dt;
                lance.angle=std::atan2(dy,dx)*57.29578f;
                if(distance<=movement+12.f){
                    damageEnemy(enemies[lance.target],static_cast<int>(((lance.verdict?8:10)+player.gear.attack())*perks.lanceMul+.5f),DamageKind::Normal,false);
                    elementReact(enemies[lance.target],lance.markOnHit?tags::flashLance:tags::flashVerdict);
                    if(lance.verdict)flashEffect(LightFx::Verdict,tx,ty);
                    flashEffect(LightFx::Impact,tx,ty);
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
                if(enemySweptDisc(e,old,b.x,b.y,r)){
                    if(b.voidBolt){elementHit(e,b.damage,tags::rulerBolt);
                        const float nx=e.x+(b.x-e.x)*.2f*(1-e.knockbackResistance());
                        if(!castleArea||std::isfinite(CastleTerrain::groundAt(castleArea,nx)))e.x=nx;
                    }else hit(e,b.damage,true);remove=true;}
            }
            if(remove)projectiles.erase(projectiles.begin()+i);
        }
    }
public:
    Player player;
    std::vector<GroundSurface> platforms;
    int castleArea=0; // 0 retains the legacy scene outside the A1-A5 castle route
    unsigned pitRespawnSerial=0; // ECLIPSE_CASTLE_PIT_DAMAGE_V33
    float castleWorldWidth() const {return castleArea?CastleTerrain::profile(castleArea).width:1600.f;}
    void configureCastleArea(int area){
        castleArea=std::clamp(area,1,5);
        platforms.clear();
        const auto& layout=CastleTerrain::profile(castleArea);
        for(int i=0;i<layout.platformCount;++i){
            const auto& item=layout.platforms[i];
            platforms.push_back({item.left,item.right,item.y});
        }
        restart();
    }
    void transitionCastleArea(int area,float entryX){
        castleArea=std::clamp(area,1,5);platforms.clear();const auto& layout=CastleTerrain::profile(castleArea);
        for(int i=0;i<layout.platformCount;++i){const auto& item=layout.platforms[i];platforms.push_back({item.left,item.right,item.y});}
        auto& p=player;p.x=std::clamp(entryX,25.f,layout.width-35.f);const float floor=CastleTerrain::groundAt(castleArea,p.x);
        p.y=std::isfinite(floor)?floor:layout.spawnY;p.vx=p.vy=0.f;p.grounded=true;p.facing=entryX<layout.width*.5f?1:-1;
        p.meleeLeft=0.f;p.mageCastLeft=0.f;p.flashMove=FlashMove::None;p.spaceMove=0;p.dashLeft=0.f;p.attackFlash=0.f;p.invulnerable=std::max(p.invulnerable,.65f);
        populateCastleEnemies();
        projectiles.clear();lightLances.clear();lightBursts.clear();eclipseFx.clear();riftScars.clear();spaceEffects.clear();popups.clear();bleedSpreads.clear();hazards.clear();bossFx.clear();ended=won=inventory=false;clock=enemyVisualClock=0;cameraCenter=viewWidth*.5f;
    }
    std::vector<Enemy> enemies;
    // ---- Story/tutorial hooks (all defaults keep the plain combat playtest unchanged) ----
    bool scriptedSpawns=false;      // StoryDirector places enemies instead of the area table
    bool endOnClear=true;           // false: clearing foes does not raise STAGE CLEAR
    bool suppressGameOver=false;    // true: lethal damage sets `defeated` instead of GAME OVER
    bool defeated=false;            // scripted defeat reached; simulation freezes
    bool kingUnbeatable=true;       // true form stops taking damage at a health floor
    // ---- Stat progression (progression.hpp) ----
    Progression progression;        // levels per job + memories; the host loads/saves it
    StatBlock stats;                // bonuses of the active job, refreshed by refreshStats()
    bool dropsEnabled=false;        // memories are left only once the story turns this on (not in the tutorial)
    // Memory altar: upgrades happen only at the forest start. While the menu is open the world is frozen.
    bool memoryMenu=false;
    int memoryTab=0;                // 0 permanent stats, 1 run upgrades
    bool menuLeftHeld=false,menuRightHeld=false;
    // Run upgrades (회차 강화): a skill tree per job, fed by that job's mastery; lost with the run.
    MasteryState mastery;
    MasteryBlock perks;             // what the active job's owned nodes do (rebuilt by refreshStats)
    bool runLost=false;             // the run ended in death; the next restart() starts a fresh run
    // ---- special effects (procs) of the owned tree nodes ----
    std::array<int,16> procCounter{};std::array<float,16> procCooldown{};   // per proc of `perks.procs` (the first 16)
    struct ProcCue{float x,y,age,radius;int action;};
    std::vector<ProcCue> procCues;  // visual only
    bool inProc=false;              // a proc never triggers another proc
    int memoryTreeSelected=-1;float memoryTreeScrollX=0,memoryTreeScrollY=0;   // the altar menu's tree view
    void fireProcs(Trigger trigger,Enemy* target){
        if(inProc)return;
        for(size_t i=0;i<perks.procs.size()&&i<procCounter.size();++i){
            const ProcDef& pd=perks.procs[i];
            if(pd.trigger!=trigger||procCooldown[i]>0.f)continue;
            if(pd.every>1&&(++procCounter[i])%pd.every!=0)continue;
            if(pd.chance<1.f&&statRoll()>=pd.chance)continue;
            procCooldown[i]=pd.cooldown;
            inProc=true;runProc(pd,target);inProc=false;
        }
    }
    void runProc(const ProcDef& pd,Enemy* target){
        auto& p=player;
        const float cx=target?target->x:p.x,cy=target?target->y:p.y;
        auto inRange=[&](const Enemy& e,float x,float y){return !e.dead()&&std::hypot(e.x-x,e.y-y)<=pd.radius;};
        switch(pd.action){
        case ProcAction::Explode:
            for(auto& e:enemies)if(inRange(e,cx,cy))damageEnemy(e,static_cast<int>(pd.value),DamageKind::Normal,true);
            procCues.push_back({cx,cy-30,0.f,pd.radius,static_cast<int>(pd.action)});break;
        case ProcAction::Pulse:
            for(auto& e:enemies)if(inRange(e,p.x,p.y)){
                damageEnemy(e,static_cast<int>(pd.value),DamageKind::Normal,true);
                e.knockback=(e.x<p.x?-1.f:1.f)*350.f*(1.f-e.knockbackResistance());
            }
            procCues.push_back({p.x,p.y-30,0.f,pd.radius,static_cast<int>(pd.action)});break;
        case ProcAction::Bleed:
            if(target&&!target->dead())addBleed(*target,static_cast<int>(pd.value));break;
        case ProcAction::AddLance:
            if(p.flashExecutioner)p.storedLightLances=std::min(5+perks.lanceCapBonus,p.storedLightLances+static_cast<int>(pd.value));break;
        case ProcAction::Heal:
            if(p.health>0)p.health=std::min(p.maxHealth(),p.health+static_cast<int>(pd.value));break;
        case ProcAction::RefundCooldown:
            p.skillCooldown=down(p.skillCooldown,pd.value);p.mageSkillCooldown=down(p.mageSkillCooldown,pd.value);
            p.sweepReady=down(p.sweepReady,pd.value);p.buffReady=down(p.buffReady,pd.value);p.verdictCooldown=down(p.verdictCooldown,pd.value);break;
        case ProcAction::Count:break;
        }
    }
    void beginRun(){mastery=MasteryState{};runLost=false;refreshStats();}
    bool buyMasteryNode(int node){
        if(!mastery.buy(activeJob(),node))return false;
        refreshStats();memoryFlash=.9f;
        return true;
    }
    // Menu choice for the current tab: a permanent stat (tab 0) or a node of the active job's tree (tab 1).
    bool menuChoose(int index){
        if(memoryTab==0)return index>=0&&index<statCount&&upgradeStat(static_cast<Stat>(index));
        return buyMasteryNode(index);
    }
    float memoryFlash=0;            // visual only: pulse when a memory is eaten
    struct MemoryMote{float x,y,age;int amount;};
    std::vector<MemoryMote> memoryMotes;   // visual only
    bool nearMemoryAltar()const{
        return castleArea==0&&player.grounded&&!ended&&!defeated&&std::abs(player.x-memoryAltarX)<=memoryAltarReach;
    }
    static float statRoll(){
        static std::mt19937 random{std::random_device{}()};
        return std::uniform_real_distribution<float>{0.f,1.f}(random);
    }
    void refreshStats(){
        const int job=player.equippedJobs[player.activeJob];
        stats=progression.blockFor(job);
        perks=mastery.block(job);
        procCounter.fill(0);procCooldown.fill(0);
        // Raw nodes of the job tree (flat stats) add to the permanent stats while the job is active.
        stats.damageMul+=perks.rawDamage;
        stats.bonusHealth+=perks.rawHealth;
        stats.bonusDefense+=perks.rawDefense;
        stats.critChance=std::min(.5f,stats.critChance+perks.rawCrit);
        stats.moveMul+=perks.rawMove;
        stats.cooldownRate+=perks.rawHaste;
        player.setStatHealth(stats.bonusHealth);
        player.statMove=stats.moveMul;
        player.lanceSpeedExtra=perks.lanceSpeedBonus;player.lanceCapExtra=perks.lanceCapBonus;
    }
    // Eat memories to raise the active job's stat; false if it is maxed or too expensive.
    bool upgradeStat(Stat stat){
        if(!progression.upgrade(player.equippedJobs[player.activeJob],stat))return false;
        refreshStats();memoryFlash=.9f;
        return true;
    }
    int activeJob()const{return player.equippedJobs[player.activeJob];}
    int playerHealthFloor=0;        // tutorial safety net while > 0
    float enemyPower=1.f,enemyDamageScale=1.f;   // tutorial weakened roster
    float kingPower=1.f,kingDamageScale=1.f;
    unsigned kingShiftSerial=0,kingPhase2Serial=0,kingBurstSerial=0;
    float kingBurstX=0,kingBurstY=0;
    std::vector<BossHazard> hazards;
    std::vector<BossFxEvent> bossFx;   // scheduled pixelfx layers (visual only)
    unsigned hazardSerial=0,kingBlinkSerial=0;
    float shake=0,flash=0,blinkTime=0,blinkFromX=0,blinkToX=0,blinkY=0;   // visual cues only
    // Begin a Demon King move immediately (the AI uses this; tests and tools may force a move).
    void bossStartMove(Enemy& e,BossMove m){
        e.move=m;e.lastMove=m;e.step=0;++e.attackCount;bossStartStep(e);
    }
    struct PlayerHandicap{bool active=false;bool dashLocked=false;int skills=4;float damage=1.f,move=1.f;};
    PlayerHandicap handicap;
    void applyHandicap(){
        player.handicapped=handicap.active;player.dashLocked=handicap.dashLocked;
        player.unlockedSkills=handicap.skills;player.damageScale=handicap.damage;player.moveScale=handicap.move;
    }
    // powerMul scales only this enemy's health on top of the tutorial roster power (default 1: no change).
    void spawn(int kind,float x,float wake=0,float powerMul=1.f){
        const bool king=kind>=DemonKingArmored;
        Enemy e(kind,x,castleArea);
        const float power=(king?kingPower:enemyPower)*powerMul,damage=king?kingDamageScale:enemyDamageScale;
        e.health=e.maxHealth=std::max(1,static_cast<int>(e.health*power+.5f));
        e.damage=std::max(1,static_cast<int>(e.damage*damage+.5f));
        e.wake=wake;e.attackTimer=wake+.8f;
        const float foot=CastleTerrain::groundAt(castleArea,x);
        if(castleArea>0&&std::isfinite(foot))e.y=foot;
        enemies.push_back(e);
    }
    // Scripted defeat: story-controlled death that never shows GAME OVER.
    void defeatPlayer(){
        auto& p=player;
        holdHealthTrail(p,.3f);p.health=0;p.hurtTint=.3f;p.spaceMove=0;p.flashMove=FlashMove::None;
        cancelAirSlam();cancelMageCast();p.meleeLeft=0;p.dashLeft=0;p.vx=0;++p.hurtSerial;
        projectiles.clear();hazards.clear();bossFx.clear();inventory=false;defeated=true;runLost=true;
    }
    // Starting forest: the legacy open-air scene with the weakened baseline roster.
    // The forest roster is weaker than the castle fodder. restart() rebuilds it at full strength,
    // so anything that restarts the forest must apply this again.
    void weakenForestFoes(){
        for(auto& e:enemies){e.health=e.maxHealth=std::max(1,static_cast<int>(e.health*.6f+.5f));e.damage=std::max(1,e.damage/2);}
    }
    void enterForest(){
        castleArea=0;platforms.clear();scriptedSpawns=false;endOnClear=false;
        restart();
        weakenForestFoes();
    }
    void populateCastleEnemies(){
        enemies.clear();
        if(scriptedSpawns)return;
        if(castleArea<=0){
            enemies={Enemy(0,575),Enemy(1,800),Enemy(0,1020),Enemy(2,1285)};
        }else{
            for(const auto& spawn:castleEnemySpawns(castleArea))
                enemies.emplace_back(spawn.kind,spawn.x,castleArea);
        }
        for(auto& enemy:enemies){
            const float foot=CastleTerrain::groundAt(castleArea,enemy.x);
            if(std::isfinite(foot))enemy.y=foot;
        }
    }
    std::vector<SpaceEffect> spaceEffects;
    std::vector<Projectile> projectiles;
    std::vector<LightLance> lightLances;
    std::vector<EclipseFx> eclipseFx;
    std::vector<LightBurst> lightBursts;
    unsigned long long lightBurstVisualSerial=0; // Presentation identity only; no combat timing/state effects.
    std::vector<RiftScar> riftScars;
    float cameraCenter=viewWidth*.5f;
    std::vector<Popup> popups;
    std::vector<BleedSpread> bleedSpreads;
    bool ended=false,won=false,inventory=false;
    float clock=0,enemyVisualClock=0;
    float timeScale=1.f;   // story time stop: 0 freezes the whole fight (the clock keeps running for UI pulses)
    Combat(){projectiles.reserve(16);popups.reserve(32);restart();}
    void restart(){eclipseFx.clear();const auto jobs=player.equippedJobs;player={};if(castleArea){const auto& area=CastleTerrain::profile(castleArea);player.x=area.spawnX;player.y=area.spawnY;}player.equippedJobs=jobs;player.activeJob=0;player.form=jobs[0]==1?1:0;player.flashExecutioner=jobs[0]==2;applyHandicap();refreshStats();memoryMenu=false;memoryFlash=0;memoryMotes.clear();if(runLost)beginRun();defeated=false;populateCastleEnemies();
        spaceEffects.clear();projectiles.clear();lightLances.clear();lightBursts.clear();eclipseFx.clear();riftScars.clear();hazards.clear();bossFx.clear();shake=0;cameraCenter=viewWidth*.5f;popups.clear();bleedSpreads.clear();ended=won=inventory=false;clock=enemyVisualClock=0;}
    int foes()const{int count=0;for(const auto& e:enemies)if(!e.dead())++count;return count;}
    void hit(Enemy& e,int amount,bool fire,int bleed=0,bool slam=false){
        if(e.dead()||ended||defeated)return;
        if(player.damageScale<1.f)amount=std::max(1,static_cast<int>(amount*player.damageScale+.5f));
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
        p.health=std::max(playerHealthFloor,p.health-std::max(1,amount-p.gear.defense()-stats.bonusDefense));p.hurtTint=.18f;p.spaceMove=0;cancelAirSlam();cancelMageCast();p.meleeLeft=0;p.invulnerable=.7f;p.attackFlash=0;++p.hurtSerial;p.combo=0;p.comboWindow=0;
        if(p.health==0){if(suppressGameOver)defeatPlayer();else{ended=true;won=false;inventory=false;runLost=true;}}
        else fireProcs(Trigger::OnDamaged,nullptr);
    }
    void step(float dt,Actions a){
        if(a.restart){restart();return;}
        if(timeScale<=0.f){clock+=dt;return;}
        dt*=timeScale;
        clock+=dt;
        for(auto& effect:spaceEffects)effect.age+=dt;
        spaceEffects.erase(std::remove_if(spaceEffects.begin(),spaceEffects.end(),[](const SpaceEffect& e){return e.age>=(e.kind==2?91.f/30:e.kind==3?97.f/30:2.f);}),spaceEffects.end());
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
        shake=down(shake,dt*1.8f);flash=down(flash,dt*3.2f);blinkTime=down(blinkTime,dt);
        for(auto& fx:bossFx)fx.age+=dt;
        bossFx.erase(std::remove_if(bossFx.begin(),bossFx.end(),[](const BossFxEvent& fx){
            const auto& sheet=bossFxSheets[static_cast<int>(fx.kind)];
            return fx.life>0?fx.age>=fx.life:fx.age*sheet.fps>=sheet.count;}),bossFx.end());
        memoryFlash=down(memoryFlash,dt);
        for(auto& c:procCooldown)c=down(c,dt);
        for(auto& c:procCues)c.age+=dt;
        procCues.erase(std::remove_if(procCues.begin(),procCues.end(),[](const ProcCue& c){return c.age>=.5f;}),procCues.end());
        for(auto& mote:memoryMotes)mote.age+=dt;
        memoryMotes.erase(std::remove_if(memoryMotes.begin(),memoryMotes.end(),[](const MemoryMote& m){return m.age>=.9f;}),memoryMotes.end());
        for(auto& popup:popups){popup.age+=dt;popup.y-=27*dt;}
        popups.erase(std::remove_if(popups.begin(),popups.end(),[](const Popup& p){return p.age>=.6f;}),popups.end());
        if(ended||defeated)return;
        // Memory altar menu: interact opens it next to the altar; interact/inventory closes it. The world is frozen meanwhile.
        if(memoryMenu){
            if(a.right&&!menuRightHeld)memoryTab=1;
            if(a.left&&!menuLeftHeld)memoryTab=0;
            menuLeftHeld=a.left;menuRightHeld=a.right;
            if(a.interact||a.inventory)memoryMenu=false;
            return;
        }
        if(a.interact&&nearMemoryAltar()){memoryMenu=true;memoryTab=0;memoryTreeSelected=-1;menuLeftHeld=a.left;menuRightHeld=a.right;player.vx=0;return;}
        if(a.inventory)inventory=!inventory;
        if(a.weapon)player.equip(0);
        if(a.armor)player.equip(1);
        if(a.charm)player.equip(2);
        stepPlayer(dt,a);
        // A short cinematic time-stop freezes enemy motion and existing projectiles,
        // while the executioner's own animation continues to the release frame.
        const bool verdictFreeze=player.flashExecutioner&&player.flashMove==FlashMove::Verdict&&
            player.flashAge>=.12f&&player.flashAge<.66f;
        if(!verdictFreeze){
            enemyVisualClock+=dt;
            for(auto& enemy:enemies){stepEnemy(enemy,dt);if(ended)break;}
            if(!ended)stepProjectiles(dt);
            if(!ended)stepHazards(dt);
            stepEclipse(dt);
        }
        if(!ended)stepLightLances(dt);
        if(!ended&&endOnClear&&foes()==0){ended=true;won=true;inventory=false;}
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
        int next=p.health<=0?9:p.dashLeft>0?7:(p.airSlam||p.slamRecover>0)?12:attackTime>0?attackState:!p.grounded?(p.vy<0?2:3):landTime>0?10:std::abs(p.vx)>20?(!p.flashExecutioner&&std::abs(p.vx)<180?11:1):0;
        if(next!=state||retrigger){state=next;time=0;}else time+=visualDt;
        const auto clip=motionClips[next];
        const bool loop=next==0||next==1||next==11;
        float phase=0;
        if(next==0)phase=std::fmod(time/clip.duration*clip.count,static_cast<float>(clip.count));
        else if(next==1||next==11){
            const float stride=p.flashExecutioner?flashExecutionerRunStride:clip.stride;
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
