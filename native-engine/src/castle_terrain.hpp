#pragma once
// ECLIPSE handcrafted castle terrain A1-A5, v34; aligned to the current merged backgrounds.
// Coordinate space: 1280x720 viewport, 2160 world units. No procedural generation.
#include <algorithm>
#include <cmath>
#include <limits>
namespace eclipse::CastleTerrain {
struct Node { float x,y; };
struct Platform { float left,right,y; };
struct Profile { const Node* nodes; int nodeCount; const Platform* platforms; int platformCount; float width,spawnX,spawnY,gapL,gapR; };
inline constexpr Node a1Nodes[]={{0,650},{350,650},{760,646},{1200,650},{1620,646},{2160,646}};
inline constexpr Node a2Nodes[]={{0,644},{2160,644}};
inline constexpr Node a3Nodes[]={{0,632},{460,632},{600,520},{900,520},{980,632},{1120,632},{1140,632},{1250,632},{1500,620},{1660,560},{1900,540},{2160,540}};
inline constexpr Node a4Nodes[]={{0,590},{480,590},{680,520},{1050,520},{1470,420},{1680,420},{1900,300},{2160,300}};
inline constexpr Node a5Nodes[]={{0,560},{2160,560}};
inline constexpr Platform a5Platforms[]={{1880,2160,445}};
inline constexpr Profile profiles[]={
 {a1Nodes,6,nullptr,0,2160,130,650,-1,-1},
 {a2Nodes,2,nullptr,0,2160,120,644,-1,-1},
 {a3Nodes,12,nullptr,0,2160,110,632,1150,1220},
 {a4Nodes,8,nullptr,0,2160,140,590,-1,-1},
 {a5Nodes,2,a5Platforms,1,2160,135,560,-1,-1}
};
inline const Profile& profile(int area){ return profiles[std::clamp(area,1,5)-1]; }
inline float portalX(int area,bool right){
 static constexpr float positions[][2]={{72,1810},{250,2080},{72,2080},{72,2080},{72,2080}};
 return positions[std::clamp(area,1,5)-1][right?1:0];
}
inline int linkedArea(int area,bool right){
 area=std::clamp(area,1,5);
 return right?(area==5?1:area+1):(area==1?5:area-1);
}
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
