// Offline visual-selection sampler; no SDL, game launch, network or owner test suite.
#include "../src/combat.hpp"
#include <iostream>
#include <set>
int main(){
 using namespace eclipse;
 std::cout<<"clip,tick,state,phase,source,legacySource\n";
 Player p; p.combo=1; ++p.attackSerial; Animation a; std::set<int> weak;
 for(int i=0;i<20;++i){a.step(p,1.f/60);if(a.state==4){const int frame=knightWeakFrame(a.posePhase);weak.insert(frame);std::cout<<"weak,"<<i<<","<<a.state<<","<<a.posePhase<<","<<frame<<","<<a.frame<<"\n";}}
 if(weak.size()!=11)return 11;
 p=Player{};p.form=1;p.grounded=false;a.reset();std::set<int> rise,fall;
 for(int i=0;i<66;++i){p.vy=-650+i*(1250.f/60);a.step(p,1.f/60);const int source=(a.state==2?16:24)+mageAirPose(a.posePhase);(a.state==2?rise:fall).insert(source);std::cout<<"mage,"<<i<<","<<a.state<<","<<a.posePhase<<","<<source<<","<<a.frame<<"\n";}
 if(rise.size()!=8||fall.size()!=8)return 12;
 p=Player{};p.combo=1;++p.attackSerial;p.hitStop=.075f;a.reset();a.step(p,1.f/60);const int contact=knightWeakFrame(a.posePhase);
 if(contact!=118)return 13;
 for(int i=0;i<4;++i){a.step(p,1.f/60);if(knightWeakFrame(a.posePhase)!=contact)return 14;}
 p=Player{};p.whiteNight=true;p.vx=280;a.reset();a.step(p,1.f/60);std::set<int> white;
 for(int i=0;i<60;++i){p.x+=280.f/60;a.step(p,1.f/60);const int frame=88+std::min(31,int(a.travelPhase*32));white.insert(frame);std::cout<<"white,"<<i<<","<<a.state<<","<<a.travelPhase<<","<<frame<<","<<a.frame<<"\n";}
 if(white.size()!=32)return 15;
 const float stationaryPhase=a.travelPhase;for(int i=0;i<10;++i)a.step(p,1.f/60);if(a.travelPhase!=stationaryPhase)return 16;
 std::cerr<<"11 weak poses, 8+8 mage poses, contact hold 118, 32 white poses and stationary hold confirmed in headless 60Hz Animation sampling. Not SDL/device playback.\n";
}
