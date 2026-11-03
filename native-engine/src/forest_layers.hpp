#pragma once
// Composed PNG forest scene; NPC coordinates and collision live in the game,
// never in background art or per-scene randomization.
#include <array>
#include <cstdint>
namespace eclipse {
struct ForestLayer {
    const char* file;
    float parallax;
    bool worldLocked;
};
struct ForestLayerScene {
    static constexpr int worldWidth=1600;
    static constexpr int viewportWidth=1280;
    // Authored grass surface stays fixed; actors stand 12px inside its moss edge.
    static constexpr int surfaceY=585;
    static constexpr int floorY=surfaceY+12;
    // Forest entrance: authored pixel-art layers; default biome scene.
    // Foreground items are non-colliding visual decorations only.
    static constexpr std::array<ForestLayer,6> layersEntrance{{
        {"forest_entrance/sky/sky.png",0.f,false},
        {"forest_entrance/clouds/clouds.png",.06f,false},
        {"forest_entrance/far/mountains.png",.14f,false},
        {"forest_entrance/mid/trees.png",.40f,false},
        {"forest_entrance/near/ground.png",1.f,true},
        {"forest_entrance/near/props.png",1.f,true}
    }};
    // v3 layers are artwork-only. No random world positions or new collisions.
    static constexpr std::array<ForestLayer,6> layersV3{{
        {"forest_v3/sky/sky_gradient_moon_01.png",0.f,false},
        {"forest_v3/clouds/clouds_01.png",.06f,false},
        {"forest_v3/far/mountain_far_01.png",.14f,false},
        {"forest_v3/mid/forest_mid_01.png",.40f,false},
        {"forest_v3/near/ground_fixed_01.png",1.f,true},
        {"forest_v3/near/near_props_fixed_01.png",1.f,true}
    }};
    // Keep v2 available as a complete fallback if a v3 file is missing.
    static constexpr std::array<ForestLayer,4> layers{{
        {"forest_v2/sky/sky_01.png",0.f,false},
        {"forest_v2/far/forest_far_01.png",.12f,false},
        {"forest_v2/mid/forest_mid_01.png",.40f,false},
        {"forest_v2/near/ground_fixed_01.png",1.f,true}
    }};
    struct SafeZone {int x0,x1;};
    // Future NPC vendors/quest givers must keep these fixed free-space intervals.
    static constexpr std::array<SafeZone,3> npcZones{{
        {260,425},{710,900},{1180,1360}
    }};
    static constexpr bool npcSafe(int x,int clearance=0) {
        for(auto z:npcZones)if(x>=z.x0-clearance && x<=z.x1+clearance)return true;
        return false;
    }
    static constexpr bool canPlaceForegroundProp(int x,int clearance=60) {
        return x>=0 && x<worldWidth && !npcSafe(x,clearance);
    }
    // Only a mild far-layer tint is allowed; no positional randomness.
    static constexpr unsigned farTint(uint32_t seed) {
        return 239u+(seed%17u); // 239..255 ~= 94..100% brightness
    }
};
}