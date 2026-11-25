#pragma once
// Altar menu layout and pointer logic. Pure C++17 (no SDL): Presentation draws from these numbers,
// main forwards pointer events to MemoryMenuController, and the tests drive it headlessly.
// The skill tree can be as big as the data makes it: the viewport scrolls by dragging, a tap selects a
// node, a second tap on the selected node (or the BUY button, or Enter) buys it.
#include "combat.hpp"
#include <algorithm>
#include <cmath>

namespace eclipse {
namespace MemoryMenu {

constexpr float panelX=380,panelY=110,panelW=520,panelH=500;
constexpr float rowY=210,rowH=44;                                              // permanent tab rows
constexpr float treeX=panelX+14,treeY=panelY+112,treeW=panelW-28,treeH=262;    // skill-tree viewport
constexpr float detailY=treeY+treeH+8,detailH=panelY+panelH-detailY-10;        // selected-node box under the tree
constexpr float buyW=118,buyH=34,buyX=panelX+panelW-buyW-18,buyY=detailY+detailH-buyH-8;
constexpr float colStep=124,rowStep=84,nodeW=108,nodeH=64;                     // one grid unit of a node position
constexpr float dragThreshold=8;                                               // pixels before a press becomes a pan
constexpr int Close=100,TabPermanent=101,TabRun=102,Buy=103;                   // ids returned by hitFixed()

inline bool inside(float x,float y,float rx,float ry,float rw,float rh){return x>=rx&&x<rx+rw&&y>=ry&&y<ry+rh;}

// Screen rectangle of a node for the given scroll.
inline void nodeRect(const MasteryNodeDef& n,float scrollX,float scrollY,float& x,float& y){
    x=treeX+treeW*.5f+n.x*colStep-nodeW*.5f-scrollX;
    y=treeY+14+n.y*rowStep-scrollY;
}
// Extent of a job's tree in content coordinates (the same space nodeRect uses with scroll 0).
inline void treeExtent(int job,float& left,float& right,float& bottom){
    left=0;right=0;bottom=0;
    const auto& tree=masteryTree(job);
    for(const auto& n:tree){
        left=std::min(left,n.x*colStep-nodeW*.5f-8);
        right=std::max(right,n.x*colStep+nodeW*.5f+8);
        bottom=std::max(bottom,14+n.y*rowStep+nodeH+8);
    }
}
// Keep the view inside the tree; a tree smaller than the viewport stays centred.
inline void clampScroll(Combat& g){
    float left=0,right=0,bottom=0;treeExtent(g.activeJob(),left,right,bottom);
    const float half=treeW*.5f;
    if(right-left<=treeW)g.memoryTreeScrollX=(left+right)*.5f;
    else g.memoryTreeScrollX=std::clamp(g.memoryTreeScrollX,left+half,right-half);
    g.memoryTreeScrollY=std::clamp(g.memoryTreeScrollY,0.f,std::max(0.f,bottom-treeH));
}
// The node under a point (inside the viewport), or -1.
inline int nodeAt(const Combat& g,float x,float y){
    if(!inside(x,y,treeX,treeY,treeW,treeH))return -1;
    const auto& tree=masteryTree(g.activeJob());
    for(int i=0;i<static_cast<int>(tree.size());++i){
        float nx=0,ny=0;nodeRect(tree[i],g.memoryTreeScrollX,g.memoryTreeScrollY,nx,ny);
        if(inside(x,y,nx,ny,nodeW,nodeH))return i;
    }
    return -1;
}
// Fixed controls: Close (the X or anywhere outside), the two tabs, the BUY button (tree tab, node selected),
// a permanent-tab row (0..statCount-1), or -1.
inline int hitFixed(float x,float y,int tab,bool nodeSelected){
    if(!inside(x,y,panelX,panelY,panelW,panelH))return Close;
    if(x>=panelX+panelW-70&&y<panelY+64)return Close;
    if(inside(x,y,panelX+28,panelY+66,220,32))return TabPermanent;
    if(inside(x,y,panelX+260,panelY+66,220,32))return TabRun;
    if(tab==0){
        if(y>=rowY&&y<rowY+rowH*statCount)return static_cast<int>((y-rowY)/rowH);
        return -1;
    }
    if(nodeSelected&&inside(x,y,buyX,buyY,buyW,buyH))return Buy;
    return -1;
}

struct MemoryMenuController {
    bool pressed=false,dragging=false;
    float startX=0,startY=0,scrollStartX=0,scrollStartY=0;

    void down(Combat& g,float x,float y){
        const int hit=hitFixed(x,y,g.memoryTab,g.memoryTreeSelected>=0);
        if(hit==Close){g.memoryMenu=false;return;}
        if(hit==TabPermanent){g.memoryTab=0;return;}
        if(hit==TabRun){g.memoryTab=1;g.memoryTreeSelected=-1;clampScroll(g);return;}
        if(hit==Buy){g.buyMasteryNode(g.memoryTreeSelected);return;}
        if(g.memoryTab==0){if(hit>=0)g.menuChoose(hit);return;}
        if(inside(x,y,treeX,treeY,treeW,treeH)){            // a press in the tree: a tap or the start of a pan
            pressed=true;dragging=false;startX=x;startY=y;
            scrollStartX=g.memoryTreeScrollX;scrollStartY=g.memoryTreeScrollY;
        }
    }
    void move(Combat& g,float x,float y){
        if(!pressed)return;
        const float dx=x-startX,dy=y-startY;
        if(!dragging&&std::hypot(dx,dy)>=dragThreshold)dragging=true;
        if(dragging){
            g.memoryTreeScrollX=scrollStartX-dx;g.memoryTreeScrollY=scrollStartY-dy;
            clampScroll(g);
        }
    }
    void up(Combat& g,float x,float y){
        if(!pressed)return;
        pressed=false;
        if(dragging){dragging=false;return;}
        const int node=nodeAt(g,x,y);
        if(node<0)return;
        if(node==g.memoryTreeSelected)g.buyMasteryNode(node);   // a second tap buys
        else g.memoryTreeSelected=node;
    }
    void cancel(){pressed=dragging=false;}
};

} // namespace MemoryMenu
using MemoryMenu::MemoryMenuController;

} // namespace eclipse
