#pragma once
// ECLIPSE seeded forest layout. Pure C++17: no SDL, no allocations during rendering.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace eclipse {
enum class ForestKind : uint8_t { Far, LargeTree, MidTree, Bush, Grass, Rock, Plant, Detail, Effect };
struct ForestSprite {
    ForestKind kind;
    uint8_t variant;
    uint8_t layer;
    float x, groundY, scale;
    uint32_t phase;
};
struct ForestSettings {
    uint32_t seed = 20261103u;
    float density = .65f;       // 0 .. 1
    float size = 1.f;          // .6 .. 1.5
    float variation = .3f;     // 0 .. 1
    float motion = .08f;       // 0 .. .2, effects only
    float width = 1600.f;
    float groundY = 585.f;
};
class ForestGenerator {
    ForestSettings settings_{};
    std::vector<ForestSprite> sprites_{};
    static uint32_t mix(uint32_t v) {
        v ^= v >> 16; v *= 0x7feb352du;
        v ^= v >> 15; v *= 0x846ca68bu;
        return v ^ (v >> 16);
    }
    uint32_t random(uint32_t layer, uint32_t index, uint32_t channel) const {
        return mix(settings_.seed ^ (layer * 0x9e3779b9u) ^
                   (index * 0x85ebca6bu) ^ (channel * 0xc2b2ae35u));
    }
    float unit(uint32_t layer, uint32_t index, uint32_t channel) const {
        return float(random(layer,index,channel) & 0xffffffu) / float(0x1000000u);
    }
    static float clamp(float x,float low,float high) {
        return std::max(low,std::min(high,x));
    }
    void distribute(ForestKind kind, uint8_t layer, uint8_t varieties,
                    float spacing, float minimum, float baseScale, float baseline,
                    uint32_t salt) {
        // Slot-based jitter plus rejection prevents both regular grids and pile-ups.
        // Always traverse left to right; seed changes the layout but not the result on redraw.
        float previous = -100000.f;
        const unsigned slots=static_cast<unsigned>(std::ceil((settings_.width + 400.f)/spacing));
        const float probability=clamp(settings_.density * (kind==ForestKind::Far ? 1.35f :
            kind==ForestKind::Grass ? 1.15f : kind==ForestKind::Effect ? .22f : 1.f),0.f,1.f);
        for(unsigned i=0;i<slots;++i) {
            if(unit(salt,i,0)>probability)continue;
            const float jitter=(unit(salt,i,1)-.5f)*spacing*settings_.variation;
            const float x=-180.f + (i+.5f)*spacing + jitter;
            if(x-previous<minimum)continue;
            const float s=clamp(baseScale*settings_.size*
                (1.f + (unit(salt,i,2)-.5f)*.45f*settings_.variation),.28f,2.0f);
            uint8_t variation=static_cast<uint8_t>(random(salt,i,3)%varieties);
            sprites_.push_back({kind,variation,layer,
                std::round(x/3.f)*3.f,baseline,s,random(salt,i,4)});
            previous=x;
        }
    }
public:
    explicit ForestGenerator(ForestSettings settings={}) {regenerate(settings);}
    const ForestSettings& settings() const {return settings_;}
    const std::vector<ForestSprite>& sprites() const {return sprites_;}
    void regenerate(ForestSettings settings) {
        settings.density=clamp(settings.density,0.f,1.f);
        settings.size=clamp(settings.size,.6f,1.5f);
        settings.variation=clamp(settings.variation,0.f,1.f);
        settings.motion=clamp(settings.motion,0.f,.2f);
        settings.width=clamp(settings.width,640.f,100000.f);
        settings_=settings;
        sprites_.clear();
        // Distances reflect silhouettes and not mere sprite anchor spacing.
        // The layer numbers are sorted back-to-front for alpha compositing.
        distribute(ForestKind::Far,0,3,260,170,.95f,560,11);
        distribute(ForestKind::LargeTree,1,5,205,130,1.15f,584,17);
        distribute(ForestKind::MidTree,2,4,145,83,.92f,582,23);
        distribute(ForestKind::Bush,3,5,92,56,.82f,584,29);
        distribute(ForestKind::Rock,3,3,175,88,.83f,583,31);
        distribute(ForestKind::Plant,3,3,125,68,.75f,584,37);
        distribute(ForestKind::Detail,3,6,220,105,.75f,583,41);
        distribute(ForestKind::Grass,4,4,43,23,.7f,583,43);
        distribute(ForestKind::Effect,5,3,310,145,.65f,520,47);
        std::stable_sort(sprites_.begin(),sprites_.end(),
            [](const ForestSprite& a,const ForestSprite& b) {
                return a.layer<b.layer;
            });
    }
    static float parallax(uint8_t layer) {
        constexpr std::array<float,6> factor{{.10f,.23f,.44f,.72f,1.f,.82f}};
        return factor[std::min<size_t>(layer,factor.size()-1)];
    }
    float drawX(const ForestSprite& s, float cameraX) const {
        return std::round((s.x-(cameraX-640.f)*parallax(s.layer))/3.f)*3.f;
    }
    float microMotion(const ForestSprite& s, float clock) const {
        if(s.kind!=ForestKind::Effect)return 0.f;
        // Under one pixel for default motion; no sway on grass, trunks or shrubs.
        return std::sin(clock*.35f + float(s.phase%997u)*.01f)*settings_.motion*8.f;
    }
};
}
