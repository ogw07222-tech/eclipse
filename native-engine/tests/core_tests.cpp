#include "../src/core.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool value,const char* message) { if(!value) { std::cerr<<message<<'\n'; std::exit(1); } }
int main() {
    eclipse::Input input;
    input.pointer(1,35,220,true); input.pointer(2,400,220,true);
    auto a=input.sample(); check(a.left && a.attack,"Movement plus attack must coexist");
    check(!input.sample().attack,"Holding attack must not auto-repeat");
    input.pointer(2,0,0,false); check(input.sample().left,"Releasing attack must retain movement");
    input.keys.right=true; input.pointer(1,0,0,false);
    check(input.sample().right,"Touch release must not cancel keyboard");
    input.pointer(3,230,220,true); check(input.sample().jump,"Jump press edge");
    check(!input.sample().jump,"Holding jump must not repeat");
    input.reset(); a=input.sample(); check(!a.left && !a.right && !a.jump && !a.attack,"Focus reset");
    for(int i=0;i<16;++i) input.pointer(i,400,220,true);
    input.reset(); check(!input.sample().attack,"Pointer capacity/reset");
    eclipse::Actor p;
    p.step(static_cast<float>(eclipse::fixedStep),{false,true,false,false});
    check(std::abs(p.x-152.25f)<.01f && p.frame>=6 && p.frame<14,"Movement and run atlas range");
    p.step(static_cast<float>(eclipse::fixedStep),{false,false,true,false});
    check(!p.grounded && p.vy<0 && p.frame>=14 && p.frame<18,"Jump and atlas range");
    for(int i=0;i<120;++i) p.step(static_cast<float>(eclipse::fixedStep),{});
    check(p.grounded && p.y==eclipse::floorY-64,"Landing");
    p.step(static_cast<float>(eclipse::fixedStep),{false,false,false,true});
    check(p.frame>=21 && p.frame<27,"Attack uses actual attack atlas cells");
    for(int i=0;i<1000;++i) p.step(static_cast<float>(eclipse::fixedStep),{true,false,false,false});
    check(p.x==0,"World left clamp");
    std::cout<<"PASS: input ownership/edges/reset/capacity and movement/jump/animation/walls\n";
}
