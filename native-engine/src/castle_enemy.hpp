#pragma once
#include "slime_motion.hpp"
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
    VoidShade=10
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
    {"허무의_잔영",760,34,68,1.25f,1.65f,1.55f,.58f,110,245,.88f,96,122,104,82,EnemyRanged}
};
inline constexpr int castleEnemyCount=static_cast<int>(sizeof(castleEnemySpecs)/sizeof(castleEnemySpecs[0]));
inline EnemySpec enemySpec(int kind){
    if(kind>=AshInfantry&&kind<=VoidShade)return castleEnemySpecs[kind-AshInfantry];
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
inline int enemyFrame(int kind,float travel,float attackAge,float attackDuration){
    if(kind<AshInfantry)return slimeFrame(travel,attackAge,kind);
    if(attackAge>=0)return std::clamp(static_cast<int>(attackAge/std::max(.001f,attackDuration)*castleEnemyAttackFrames),0,castleEnemyAttackFrames-1);
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
} // namespace eclipse
