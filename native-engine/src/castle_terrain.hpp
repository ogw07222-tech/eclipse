#pragma once
// ECLIPSE handcrafted castle terrain A1-A5, v36; aligned to current background walkways.
// Coordinate space: 1280x720 viewport, 2160 world units. No procedural generation.
#include <algorithm>
#include <cmath>
#include <limits>
namespace eclipse::CastleTerrain {
struct Node { float x,y; };
struct Platform { float left,right,y; };
struct Profile { const Node* nodes; int nodeCount; const Platform* platforms; int platformCount; float width,spawnX,spawnY,gapL,gapR; };
inline constexpr Node a1Nodes[]={{0,620},{350,624},{760,616},{1200,614},{1620,612},{2160,604}};
inline constexpr Node a2Nodes[]={{0,628},{2160,628}};
inline constexpr Node a3Nodes[]={{0,626},{460,626},{600,540},{900,540},{980,626},{1120,626},{1140,626},{1250,626},{1500,620},{1660,582},{1900,540},{2160,540}};
inline constexpr Node a4Nodes[]={{0,628},{480,588},{680,548},{1050,540},{1470,508},{1680,484},{1900,460},{2160,440}};
inline constexpr Node a5Nodes[]={{0,550},{1750,550},{1880,535},{2010,500},{2160,480}};
inline constexpr Profile profiles[]={
 {a1Nodes,6,nullptr,0,2160,130,650,-1,-1},
 {a2Nodes,2,nullptr,0,2160,120,644,-1,-1},
 {a3Nodes,12,nullptr,0,2160,110,632,1150,1220},
 {a4Nodes,8,nullptr,0,2160,140,590,-1,-1},
 {a5Nodes,5,nullptr,0,2160,135,550,-1,-1}
};
inline const Profile& profile(int area){ return profiles[std::clamp(area,1,5)-1]; }
inline float portalX(int area,bool right){
 static constexpr float exits[]={1810,2080,2080,2080};
 return right&&area>=1&&area<=4?exits[area-1]:-1.f;
}
inline int nextArea(int area){return area>=1&&area<5?area+1:0;}
inline float arrivalX(int area){
 static constexpr float positions[]={0,336,158,158,158};
 return area>=1&&area<=5?positions[area-1]:0.f;
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
