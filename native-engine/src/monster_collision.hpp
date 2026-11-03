#pragma once
#include "slime_motion.hpp"
#include "slime_shapes.hpp"

namespace eclipse {
struct BodyRect {float left,top,right,bottom;};
inline bool overlaps(BodyRect a,BodyRect b){return a.left<b.right&&a.right>b.left&&a.top<b.bottom&&a.bottom>b.top;}
template<class E> int enemyFrame(const E& e){return slimeFrame(e.travelPhase,e.attackAge,e.kind);}
template<class E> BodyRect enemySourceRect(const E& e,float left,float top,float right,float bottom){
    const float scale=slimeSpec(e.kind).scale;
    const float x0=e.facing<0?left-slimePivotX:slimePivotX-right;
    const float x1=e.facing<0?right-slimePivotX:slimePivotX-left;
    return {e.x+x0*scale,e.y+(top-slimePivotY)*scale,e.x+x1*scale,e.y+(bottom-slimePivotY)*scale};
}
template<class E> BodyRect enemyBounds(const E& e){
    const auto& s=slimeShapes[e.kind][enemyFrame(e)];return enemySourceRect(e,s.left,s.top,s.right,s.bottom);
}
template<class E,class Fn> bool enemyAnyBand(const E& e,Fn fn){
    const auto& shape=slimeShapes[e.kind][enemyFrame(e)];
    for(int i=0;i<slimeCell;++i){const auto band=shape.bands[i];if(band.right<=band.left)continue;
        const auto b=enemySourceRect(e,band.left,std::max(shape.top,i),band.right,std::min(shape.bottom,i+1));
        if(fn(b))return true;
    }
    return false;
}
template<class E> bool enemyOverlaps(const E& e,BodyRect attack){return enemyAnyBand(e,[&](BodyRect band){return overlaps(band,attack);});}
template<class E> bool enemySweptDisc(const E& e,float oldX,float newX,float y,float radius){
    const float left=std::min(oldX,newX),right=std::max(oldX,newX);
    return enemyAnyBand(e,[&](BodyRect b){
        const float dx=std::max({0.f,b.left-right,left-b.right});
        const float dy=std::max({0.f,b.top-y,y-b.bottom});
        return dx*dx+dy*dy<=radius*radius;
    });
}
template<class E,class P> float enemyFrontGap(const E& e,const P& p){
    const auto body=enemyBounds(e);
    return e.facing<0?body.left-(p.x+14):p.x-14-body.right;
}
template<class E> BodyRect enemyAttackBounds(const E& e){
    auto body=enemyBounds(e);const float reach=slimeSpec(e.kind).attackReach;
    if(e.kind==1){body.left-=reach;body.right+=reach;body.top=std::max(body.top,e.y-32);body.bottom=e.y+8;return body;}
    if(e.facing<0){body.left-=reach;body.right=e.x+12;}
    else {body.left=e.x-12;body.right+=reach;}
    return body;
}
}
