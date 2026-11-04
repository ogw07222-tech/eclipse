#pragma once
// In-game pixel finishing for the EXISTING mage designs.
// No new silhouette, props, palette family, coordinates, alpha or animation timing.
// Applied ONCE when PNGs are loaded, never once per render frame.
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>

namespace eclipse {
inline SDL_Surface* finishMagePixels(SDL_Surface* original, int cellW=64, int cellH=94) {
    if (!original || original->w % cellW || original->h % cellH) return nullptr;
    SDL_Surface* source=SDL_ConvertSurface(original, SDL_PIXELFORMAT_RGBA32);
    if (!source) return nullptr;
    SDL_Surface* target=SDL_ConvertSurface(source, SDL_PIXELFORMAT_RGBA32);
    if (!target) { SDL_DestroySurface(source); return nullptr; }

    // Stronger fold contrast is restricted to solid interior pixels.
    // Source texture is read-only: neighboring pixels are never contaminated
    // by an already processed pixel, and frame boundaries never bleed.
    for (int yy=0; yy<source->h; ++yy) {
        for (int xx=0; xx<source->w; ++xx) {
            Uint8 r,g,b,a;
            if (!SDL_ReadSurfacePixel(source,xx,yy,&r,&g,&b,&a) || a<250) continue;
            const int x=xx%cellW, y=yy%cellH;
            const int brightness=(54*r+183*g+19*b)/256;
            if (brightness<32 || brightness>244) continue; // preserve ink and bright accents
            auto sample=[&](int sx,int sy, Uint8& rr,Uint8& gg,Uint8& bb)->bool {
                if(sx<0 || sx>=cellW || sy<0 || sy>=cellH) return false;
                Uint8 alpha=0;
                return SDL_ReadSurfacePixel(source,xx-x+sx,yy-y+sy,&rr,&gg,&bb,&alpha) && alpha>=250;
            };
            // Three-pixel material interior prevents halos around the original outline.
            Uint8 nr,ng,nb;
            bool interior=true;
            for (const int d[4][2]={{-1,0},{1,0},{0,-1},{0,1}}) {
                if (!sample(x+d[0],y+d[1],nr,ng,nb)) {interior=false;break;}
            }
            if (!interior) continue;
            Uint8 topR,topG,topB,belowR,belowG,belowB,leftR,leftG,leftB,rightR,rightG,rightB;
            sample(x,y-1,topR,topG,topB);
            sample(x,y+1,belowR,belowG,belowB);
            sample(x-1,y,leftR,leftG,leftB);
            sample(x+1,y,rightR,rightG,rightB);
            const int up=(54*topR+183*topG+19*topB)/256;
            const int down=(54*belowR+183*belowG+19*belowB)/256;
            const int left=(54*leftR+183*leftG+19*leftB)/256;
            const int right=(54*rightR+183*rightG+19*rightB)/256;
            // Existing drawn fold boundaries drive lighting, not arbitrary canvas bands.
            int shade=0;
            if (up+12<brightness && brightness>down+5) shade+=12; // lit fabric crest
            if (up>brightness+15 && down>brightness+5) shade-=13; // recessed fold
            if (left>brightness+13 && right>brightness+8) shade-=9;
            if (left+12<brightness && brightness>right+5) shade+=8;
            // Restrained directional lighting, only when surrounding pixels
            // already describe a surface (avoids a new costume or speckle noise).
            if (std::abs(up-down)>10 || std::abs(left-right)>10) {
                if (left>right+13) shade+=5;
                if (right>left+13) shade-=5;
            }
            shade=std::clamp(shade,-21,21);
            if (!shade) continue;
            // Material-sensitive ramps keep the source's red/purple identity.
            const bool warm=r>g*1.30f && r>b*1.13f;
            const bool violet=b>g*1.13f && r>g*1.07f;
            const float redScale=warm?1.0f:(violet?.85f:1.0f);
            const float greenScale=warm?.60f:(violet?.72f:1.0f);
            const float blueScale=warm?.55f:(violet?1.15f:1.0f);
            auto channel=[&](int v,float scale)->Uint8 {
                return static_cast<Uint8>(std::clamp(v+static_cast<int>(std::round(shade*scale)),0,255));
            };
            SDL_WriteSurfacePixel(target,xx,yy,channel(r,redScale),
                                  channel(g,greenScale),channel(b,blueScale),a);
        }
    }
    SDL_DestroySurface(source);
    return target;
}
} // namespace eclipse
