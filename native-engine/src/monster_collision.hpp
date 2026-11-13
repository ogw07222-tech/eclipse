#pragma once
#include "slime_motion.hpp"
#include "slime_shapes.hpp"
#include "castle_enemy.hpp"

namespace eclipse {
struct BodyRect {float left,top,right,bottom;};
inline bool overlaps(BodyRect a,BodyRect b){return a.left<b.right&&a.right>b.left&&a.top<b.bottom&&a.bottom>b.top;}
template<class E> int enemyFrame(const E& e){return eclipse::enemyFrame(e.kind,e.travelPhase,e.attackAge,enemySpec(e.kind).attackDuration);}
template<class E> BodyRect enemySourceRect(const E& e,float left,float top,float right,float bottom){
    const float scale=slimeSpec(e.kind).scale;
    const float x0=e.facing<0?left-slimePivotX:slimePivotX-right;
    const float x1=e.facing<0?right-slimePivotX:slimePivotX-left;
    return {e.x+x0*scale,e.y+(top-slimePivotY)*scale,e.x+x1*scale,e.y+(bottom-slimePivotY)*scale};
}
template<class E> BodyRect enemyBounds(const E& e){
    if(e.kind>=AshInfantry){
        const auto spec=enemySpec(e.kind);const float half=spec.bodyWidth*spec.scale*.5f;
        return {e.x-half,e.y-spec.bodyHeight*spec.scale,e.x+half,e.y};
    }
    const auto& s=slimeShapes[e.kind][enemyFrame(e)];return enemySourceRect(e,s.left,s.top,s.right,s.bottom);
}
template<class E,class Fn> bool enemyAnyBand(const E& e,Fn fn){
    if(e.kind>=AshInfantry)return fn(enemyBounds(e));
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
    auto body=enemyBounds(e);const auto spec=enemySpec(e.kind);const float reach=spec.attackReach*(e.kind>=AshInfantry?spec.scale:1.f);
    if(e.kind==1){body.left-=reach;body.right+=reach;body.top=std::max(body.top,e.y-32);body.bottom=e.y+8;return body;}
    if(e.facing<0){body.left-=reach;body.right=e.x+12;}
    else {body.left=e.x-12;body.right+=reach;}
    if(spec.behavior==EnemyRanged){body.top=e.y-spec.bodyHeight*spec.scale*.9f;body.bottom=e.y+18.f;}
    return body;
}
} // namespace eclipse
