#pragma once
#include "slime_motion.hpp"
#include "boss_motion_data.hpp"
#include <vector>

namespace eclipse {
enum CastleEnemyKind {
    AshInfantry=3,
    DemonHound=4,
    CastlePriest=5,
    Gargoyle=6,
    VoidRemnant=7,
    GatekeeperKnight=8,
    Executioner=9,
    VoidShade=10,
    DemonKingArmored=11,
    DemonKingTrue=12
};
enum EnemyBehavior { EnemyMelee, EnemyPounce, EnemyRanged, EnemyHeavy };
struct EnemySpec {
    const char* folder;
    int health,damage;
    float speed,attackDuration,cooldown,scale,contact,engageGap,attackReach,knockbackResistance;
    float pivotX,pivotY,bodyWidth,bodyHeight;
    int behavior;
};
inline constexpr EnemySpec castleEnemySpecs[]={
    {"잿빛_보병",145,10,63,.88f,1.55f,1.42f,.64f,12,34,.28f,96,122,92,58,EnemyMelee},
    {"마계_사냥개",105,13,112,.82f,1.45f,1.55f,.70f,18,42,.12f,96,114,126,46,EnemyPounce},
    {"마왕성_사제",115,12,47,1.20f,2.0f,1.25f,.72f,100,210,.18f,96,140,84,92,EnemyRanged},
    {"가고일",205,17,78,.96f,1.65f,1.36f,.62f,28,52,.48f,96,126,104,68,EnemyPounce},
    {"공허_잔재",245,20,86,.92f,1.55f,1.52f,.58f,18,48,.56f,96,121,104,62,EnemyMelee},
    {"수문장_기사",390,26,43,1.18f,1.9f,1.30f,.66f,30,65,.78f,96,141,120,98,EnemyHeavy},
    {"처형관",335,31,76,1.05f,1.9f,1.43f,.63f,35,70,.62f,96,123,112,68,EnemyHeavy},
    {"허무의_잔영",760,34,68,1.25f,1.65f,1.55f,.58f,110,245,.88f,96,122,104,82,EnemyRanged},
    // Demon King, first form: slow heavy armored knight with long wind-ups.
    {"마왕_갑주",900,26,54,1.40f,2.20f,2.0f,.57f,100,75,1.f,128,236,84,104,EnemyHeavy},
    // Demon King, true form: agile duelist; every third action is a void burst.
    {"마왕_본체",1400,30,118,1.00f,1.15f,2.1f,.55f,90,80,1.f,128,236,58,104,EnemyHeavy}
};
inline constexpr int castleEnemyCount=static_cast<int>(sizeof(castleEnemySpecs)/sizeof(castleEnemySpecs[0]));
// First attack-frame for each reviewed PixelFX overlay, in CastleEnemyKind order.
// Values are copied from each enemy's 공격_vfx_sheet.json effectFrameStart.
inline constexpr int castleEnemyEffectStartFrames[]={7,6,5,6,0,9,5,7,14,14};
inline EnemySpec enemySpec(int kind){
    if(kind>=AshInfantry&&kind<=DemonKingTrue)return castleEnemySpecs[kind-AshInfantry];
    const auto& s=slimeSpec(kind);
    return {nullptr,s.health,s.damage,s.speed,s.attackDuration,s.cooldown,s.scale,s.contact,
            s.engageGap,s.attackReach,s.knockbackResistance,slimePivotX,slimePivotY,72,78,
            kind==1?EnemyPounce:EnemyMelee};
}
inline int enemyHealth(int kind,int area){
    const float stage=kind>=AshInfantry?1.f+.12f*std::max(0,area-1):1.f;
    return static_cast<int>(enemySpec(kind).health*stage+.5f);
}
inline int enemyDamage(int kind,int area){
    const float stage=kind>=AshInfantry?1.f+.10f*std::max(0,area-1):1.f;
    return static_cast<int>(enemySpec(kind).damage*stage+.5f);
}
inline float enemySpeed(int kind,int area){
    const float stage=kind>=AshInfantry?1.f+.035f*std::max(0,area-1):1.f;
    return enemySpec(kind).speed*stage;
}
inline constexpr int castleEnemyMoveFrames=8,castleEnemyAttackFrames=14;
// The Demon King's per-move motion sheets are longer than the shared 14-frame attack sheet so the
// body can move through its in-between poses instead of snapping between key poses.
inline constexpr int bossMotionFrames=20;
// Demon King sheets use a larger 256px cell so the colossal silhouette is not cropped.
inline constexpr int enemyCell(int kind){return kind>=DemonKingArmored?256:192;}
// animFrom lets one attack sheet serve several moves: a short combo hit plays frames animFrom..13.
// Readable timing for the long boss motion sheets: a slow wind-up, a quick snap into the hit, a short
// freeze on the impact frame, a fast follow-through that then settles. `contact` is the step fraction at
// which the sheet shows the hit; the warp is monotonic and continuous.
inline float bossShapeTime(float u,float contact){
    constexpr float hold=.04f;
    u=std::clamp(u,0.f,1.f);
    if(contact<=.05f||contact>=.98f)return u;                        // moves without a single hit stay linear
    if(u<contact-hold){const float t=u/(contact-hold);return contact*std::pow(t,1.7f);}
    if(u<=contact+hold)return contact;
    const float t=(u-contact-hold)/std::max(.001f,1.f-contact-hold);
    return contact+(1.f-contact)*(1.f-std::pow(1.f-t,2.f));
}
inline int enemyFrame(int kind,float travel,float attackAge,float attackDuration,int animFrom=0,int frames=castleEnemyAttackFrames,float contact=-1.f){
    if(kind<AshInfantry)return slimeFrame(travel,attackAge,kind);
    if(attackAge>=0){
        const int from=std::clamp(animFrom,0,frames-1),span=frames-from;
        float u=attackAge/std::max(.001f,attackDuration);
        if(contact>0&&frames>castleEnemyAttackFrames)u=bossShapeTime(u,contact);
        return from+std::clamp(static_cast<int>(u*span),0,span-1);
    }
    return static_cast<int>(travel*castleEnemyMoveFrames)%castleEnemyMoveFrames;
}
struct CastleEnemySpawn {int kind;float x;};
inline std::vector<CastleEnemySpawn> castleEnemySpawns(int area){
    switch(area){
    case 1:return {{AshInfantry,430},{DemonHound,760},{AshInfantry,1120},{DemonHound,1510},{CastlePriest,1830}};
    case 2:return {{AshInfantry,390},{DemonHound,710},{Gargoyle,900},{CastlePriest,1190},{DemonHound,1510},{Gargoyle,1830}};
    case 3:return {{DemonHound,410},{Gargoyle,710},{VoidRemnant,1010},{GatekeeperKnight,1310},{DemonHound,1550},{VoidRemnant,1870}};
    case 4:return {{Gargoyle,410},{GatekeeperKnight,730},{DemonHound,1010},{Executioner,1320},{GatekeeperKnight,1680},{Gargoyle,1920}};
    default:return {{CastlePriest,390},{Gargoyle,650},{GatekeeperKnight,900},{Executioner,1160},{CastlePriest,1430},{GatekeeperKnight,1660},{VoidShade,1900}};
    }
}

// ---------------------------------------------------------------------------
// Demon King move set. Pure data + selection, so it can be unit-tested without Combat.
// Phase 1 (armored): slow, telegraphed single heavy blows with long recovery.
// Phase 2 (true form): chained combos; each step starts the instant the previous ends.
// Every step names the body motion (sprite sheet) it plays; effects are separate pixelfx layers.
// ---------------------------------------------------------------------------
enum class BossMove : unsigned char {
    None,
    Smash,          // P1 overhead smash in front
    Quake,          // P1 sword driven into the floor; ground waves both ways (jump them)
    LeapSlam,       // P1 leaps onto the player and lands in an area blast
    Judgement,      // P1 sword raised to the sky while three void pillars erupt at the player's marks
    IronSweep,      // P1 low wide sweep (jump it)
    JuggernautRush, // P1 charge across the room; long, exposed recovery
    DashCombo,      // P2 twist cut, rising cut, thrust, each a dash toward the player
    SlashBurst,     // P2 twist cut, dash thrust, void burst
    PillarChain,    // P2 backflip, three tracking lightning pillars, lunge
    BlinkStrike,    // P2 teleports behind the player, twist cut, rising cut, lunge
    WaveVolley,     // P2 ground-wave cuts with a backflip between
    VoidRain,       // P2 backflip, five spread falling stars, lunge
    AerialDive,     // P2 backflip, leaps high and dives onto the player, lunge
    ExecutionCombo  // P2 enraged finisher chain (below 60 % health)
};
enum class BossAction : unsigned char { Melee, Sweep, Leap, Quake, Judgement, Charge, DashSlash, Burst, Backstep, Pillar, Blink, Wave, Rain, Dive };
// Body motions: Attack is the legacy 공격_sheet; the rest are 모션_<name>_sheet.png. Armored owns
// Attack..Charge, the true form owns Attack and Twist..Dive. Each sheet has 14 frames.
enum class BossMotion : unsigned char { Attack, Stab, Leap, Raise, Sweep, Charge, Twist, Rise, Burst, Sky, Cast, Backflip, Dive, Count };
inline constexpr const char* bossMotionNames[]={"attack","stab","leap","raise","sweep","charge","twist","rise","burst","sky","cast","backflip","dive"};
inline bool bossMotionOwned(int kind,BossMotion m){
    const int i=static_cast<int>(m);
    return kind==DemonKingArmored?i<=static_cast<int>(BossMotion::Charge):(m==BossMotion::Attack||i>=static_cast<int>(BossMotion::Twist));
}
// duration: seconds for this step; animFrom: first sheet frame played; contact: fraction of the
// step at which the hit lands; travel: pixels moved (negative = away from the player).
struct BossStepDef {BossAction action;float duration;int animFrom;float contact;float damageMul;float travel;BossMotion motion;};

inline int bossMoveSteps(BossMove m){
    switch(m){
    case BossMove::DashCombo:return 3;
    case BossMove::SlashBurst:return 3;
    case BossMove::PillarChain:return 5;
    case BossMove::BlinkStrike:return 4;
    case BossMove::WaveVolley:return 4;
    case BossMove::VoidRain:return 3;
    case BossMove::AerialDive:return 3;
    case BossMove::ExecutionCombo:return 5;
    case BossMove::None:return 0;
    default:return 1;
    }
}
inline BossStepDef bossStepDef(BossMove m,int s){
    using A=BossAction;using M=BossMotion;
    // Motion sheets have 20 frames; `contact` is the fraction of the step where the sheet shows the hit.
    switch(m){
    case BossMove::Smash:return {A::Melee,1.6f,0,.57f,1.5f,0,M::Attack};
    case BossMove::Quake:return {A::Quake,1.9f,0,.6f,.9f,0,M::Stab};
    case BossMove::LeapSlam:return {A::Leap,2.0f,0,.93f,1.3f,0,M::Leap};
    case BossMove::Judgement:return {A::Judgement,2.2f,0,.5f,1.6f,0,M::Raise};
    case BossMove::IronSweep:return {A::Sweep,1.5f,0,.7f,1.2f,0,M::Sweep};
    case BossMove::JuggernautRush:return {A::Charge,1.8f,0,.15f,1.3f,640,M::Charge};
    case BossMove::DashCombo:
        if(s==0)return {A::DashSlash,.85f,0,.65f,.8f,220,M::Twist};
        if(s==1)return {A::DashSlash,.85f,0,.55f,.8f,220,M::Rise};
        return {A::DashSlash,.7f,4,.45f,1.3f,260,M::Attack};
    case BossMove::SlashBurst:
        if(s==0)return {A::Melee,.85f,0,.65f,.8f,0,M::Twist};
        if(s==1)return {A::DashSlash,.55f,4,.45f,.8f,140,M::Attack};
        return {A::Burst,1.5f,0,.85f,.9f,0,M::Burst};
    case BossMove::PillarChain:
        if(s==0)return {A::Backstep,.9f,0,1.f,0,-220,M::Backflip};
        if(s<4)return {A::Pillar,.85f,0,.4f,1.0f,0,M::Cast};
        return {A::DashSlash,.6f,4,.45f,1.3f,300,M::Attack};
    case BossMove::BlinkStrike:
        if(s==0)return {A::Blink,.35f,0,1.f,0,0,M::Attack};
        if(s==1)return {A::Melee,.8f,0,.65f,.9f,0,M::Twist};
        if(s==2)return {A::Melee,.8f,0,.55f,.9f,0,M::Rise};
        return {A::DashSlash,.6f,4,.45f,1.2f,200,M::Attack};
    case BossMove::WaveVolley:
        if(s==2)return {A::Backstep,.8f,0,1.f,0,-160,M::Backflip};
        if(s==1)return {A::Wave,.85f,0,.55f,.8f,0,M::Rise};
        return {A::Wave,.85f,0,.65f,.8f,0,M::Twist};
    case BossMove::VoidRain:
        if(s==0)return {A::Backstep,.9f,0,1.f,0,-260,M::Backflip};
        if(s==1)return {A::Rain,1.6f,0,.4f,1.0f,0,M::Sky};
        return {A::DashSlash,.6f,4,.45f,1.3f,320,M::Attack};
    case BossMove::AerialDive:
        if(s==0)return {A::Backstep,.9f,0,1.f,0,-240,M::Backflip};
        if(s==1)return {A::Dive,1.7f,0,.9f,1.4f,0,M::Dive};
        return {A::DashSlash,.5f,4,.45f,1.0f,160,M::Attack};
    case BossMove::ExecutionCombo:
        if(s==0)return {A::DashSlash,.7f,0,.65f,.8f,200,M::Twist};
        if(s==1)return {A::DashSlash,.7f,0,.55f,.8f,200,M::Rise};
        if(s==2)return {A::DashSlash,.45f,4,.45f,.8f,200,M::Attack};
        if(s==3)return {A::Blink,.3f,0,1.f,0,0,M::Attack};
        return {A::Burst,1.5f,0,.85f,1.2f,0,M::Burst};
    case BossMove::None:break;
    }
    return {A::Melee,1.f,0,.5f,1.f,0,M::Attack};
}
// Frame of the step's sheet that shows the hit (used to anchor effects on the sword tip).
inline int bossSheetFrames(BossMotion m){return m==BossMotion::Attack?castleEnemyAttackFrames:bossMotionFrames;}
inline int bossContactFrame(const BossStepDef& d){
    const int span=bossSheetFrames(d.motion)-d.animFrom;
    return d.animFrom+std::clamp(static_cast<int>(d.contact*span),0,span-1);
}
// Recovery window after the whole move: the punish window for phase 1 is long, phase 2 short.
inline float bossMoveCooldown(BossMove m){
    switch(m){
    case BossMove::Smash:return 2.2f;
    case BossMove::Quake:return 2.4f;
    case BossMove::LeapSlam:return 2.4f;
    case BossMove::Judgement:return 2.6f;
    case BossMove::IronSweep:return 2.0f;
    case BossMove::JuggernautRush:return 3.2f;
    case BossMove::ExecutionCombo:return 2.0f;
    case BossMove::VoidRain:return 1.8f;
    case BossMove::None:return 1.f;
    default:return 1.5f;
    }
}
// Deterministic pick: a fixed rotation, bent by distance so every move makes sense where it is used,
// and never the same move twice in a row.
inline BossMove bossPickMove(bool phase2,int counter,float gap,bool enraged,BossMove last){
    BossMove pick;
    if(!phase2){
        static constexpr BossMove rotation[]={BossMove::Smash,BossMove::Quake,BossMove::Judgement,
            BossMove::IronSweep,BossMove::LeapSlam,BossMove::JuggernautRush};
        pick=rotation[counter%6];
        if(gap>300&&(pick==BossMove::Smash||pick==BossMove::IronSweep))pick=BossMove::LeapSlam;
        if(gap<150&&pick==BossMove::JuggernautRush)pick=BossMove::Smash;
        if(gap<110&&pick==BossMove::LeapSlam)pick=BossMove::IronSweep;
    }else{
        static constexpr BossMove rotation[]={BossMove::DashCombo,BossMove::BlinkStrike,BossMove::PillarChain,
            BossMove::SlashBurst,BossMove::AerialDive,BossMove::WaveVolley,BossMove::VoidRain};
        pick=rotation[counter%7];
        if(enraged&&counter%4==3)pick=BossMove::ExecutionCombo;
        if(gap>420){
            if(pick==BossMove::DashCombo||pick==BossMove::SlashBurst||pick==BossMove::BlinkStrike)pick=BossMove::WaveVolley;
        }else if(gap<130){
            if(pick==BossMove::WaveVolley||pick==BossMove::PillarChain)pick=BossMove::BlinkStrike;
        }
    }
    if(pick==last){
        if(!phase2)pick=pick==BossMove::Smash?BossMove::Quake:BossMove::Smash;
        else pick=pick==BossMove::DashCombo?BossMove::BlinkStrike:BossMove::DashCombo;
    }
    return pick;
}
} // namespace eclipse
