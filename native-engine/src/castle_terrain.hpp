#pragma once
// ECLIPSE handcrafted castle terrain A1-A5, v32; v33 pit recovery is integrated by the runtime adapter.
// Coordinate space: 1280x720 viewport, world widths 1800/2160. No procedural generation.
#include <algorithm>
#include <cmath>
#include <limits>
namespace eclipse::CastleTerrain {
struct Node { float x,y; };
struct Platform { float left,right,y; };
struct Profile { const Node* nodes; int nodeCount; const Platform* platforms; int platformCount; float width,spawnX,spawnY,gapL,gapR; };
// A1 2172x724 original art maps to 2160x720 world units. Ground traced
// against the visible footpath; small gradual rise toward the entry gate.
inline constexpr Node a1Nodes[]={{0,526},{580,527},{1150,524},{1450,522},{1550,515},{1650,501},{1740,486},{2160,486}};
inline constexpr Node a2Nodes[]={{0,640},{1710,640},{1840,500},{1970,340},{2160,340}};
inline constexpr Platform a2Platforms[]={{0,305,458},{565,965,450}};
inline constexpr Node a3Nodes[]={{0,575},{460,575},{680,445},{900,445},{980,480},{1120,480},{1140,575},{1250,575},{1510,535},{1660,535},{1900,340},{2160,340}};
inline constexpr Platform a3Platforms[]={{950,1130,480},{1240,1640,535}};
inline constexpr Node a4Nodes[]={{0,625},{510,625},{690,535},{1060,535},{1230,455},{1530,455},{1720,320},{2160,320}};
inline constexpr Node a5Nodes[]={{0,580},{2160,580}};
inline constexpr Platform a5Platforms[]={{1880,2160,415}};
inline constexpr Profile profiles[]={
 {a1Nodes,8,nullptr,0,2160,130,526,-1,-1},
 {a2Nodes,5,a2Platforms,2,2160,120,458,-1,-1},
 {a3Nodes,12,a3Platforms,2,2160,115,575,1150,1220},
 {a4Nodes,8,nullptr,0,2160,140,625,-1,-1},
 {a5Nodes,2,a5Platforms,1,2160,135,580,-1,-1}
};
inline const Profile& profile(int area){ return profiles[std::clamp(area,1,5)-1]; }
inline float groundAt(int area,float x){
 const auto& p=profile(area);
 if(x<0||x>p.width||(x>p.gapL&&x<p.gapR))return std::numeric_limits<float>::infinity();
 for(int i=1;i<p.nodeCount;++i){const auto& a=p.nodes[i-1];const auto& b=p.nodes[i];
  if(x<=b.x){const float t=std::clamp((x-a.x)/(b.x-a.x),0.f,1.f);return a.y+(b.y-a.y)*t;}}
 return p.nodes[p.nodeCount-1].y;
}
inline float standingAt(int area,float x,float currentY){
 float floor=groundAt(area,x);const auto& p=profile(area);
 for(int i=0;i<p.platformCount;++i){const auto& q=p.platforms[i];
  if(x>=q.left&&x<=q.right&&q.y<=currentY+8.f&&std::fabs(currentY-q.y)<16.f)floor=std::min(floor,q.y);}
 return floor;
}
inline constexpr int pitDamagePercent=15; // rounded up: (maxHP*15+99)/100
inline constexpr float pitInvulnerabilitySeconds=.65f;
inline constexpr float pitTriggerY=880.f;
} // namespace eclipse::CastleTerrain
