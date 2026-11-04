#pragma once
#include <algorithm>
#include <cmath>

namespace eclipse {
// Authored atlas: 4 columns, movement 0..7, bite 8..15. Left-facing art.
constexpr int slimeCell=128,slimeColumns=4,slimePivotX=64,slimePivotY=116;
struct SlimeSpec { int health,damage; float speed,attackDuration,cooldown,scale,contact,engageGap,attackReach,knockbackResistance; };
inline constexpr SlimeSpec slimeSpecs[]={
    // Lower resistance means further knockback: red < green < purple.
    {55,9,95,.72f,1.30f,1.5f,.50f,22,26,.35f},    // green: medium
    {85,15,63,1.10f,1.80f,1.75f,.68f,108,18,.15f}, // red: lightest
    {150,23,49,1.16f,2.20f,2.25f,.61f,36,40,.65f} // purple: heaviest
};
inline const SlimeSpec& slimeSpec(int kind){return slimeSpecs[std::clamp(kind,0,2)];}
inline int slimeFrame(float travel,float attackAge,int kind){
    if(attackAge>=0)return 8+std::clamp(static_cast<int>(attackAge/slimeSpec(kind).attackDuration*8),0,7);
    return static_cast<int>(travel*8)%8;
}
}
