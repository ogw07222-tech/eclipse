#pragma once
namespace eclipse {
struct SpaceSheet {const char* file;int width,height,columns,count,pivotX,pivotY;float scale,duration;bool loop;};
inline constexpr SpaceSheet spaceActors[]={
    {"space_ruler/idle.png",192,192,9,9,96,132,1.600000f,0.900000f,true},
    {"space_ruler/hover_move.png",192,192,9,9,96,132,1.600000f,0.720000f,true},
    {"space_ruler/flight_rise.png",192,192,9,9,96,132,1.600000f,0.720000f,false},
    {"space_ruler/flight_hold.png",192,192,9,9,96,132,1.600000f,0.900000f,true},
    {"space_ruler/flight_descend.png",192,192,9,9,96,132,1.600000f,0.720000f,false},
    {"space_ruler/hurt.png",192,192,9,9,96,132,1.600000f,0.360000f,false},
    {"space_ruler/death.png",192,192,9,9,96,132,1.600000f,0.900000f,false},
    {"space_ruler/attack.png",192,192,9,9,96,132,1.600000f,0.550000f,false},
    {"space_ruler/skill1.png",192,192,9,9,96,132,1.600000f,0.650000f,false},
    {"space_ruler/skill2.png",192,192,9,9,96,132,1.600000f,0.750000f,false},
    {"space_ruler/skill3.png",192,192,9,9,96,132,1.600000f,0.600000f,false},
    {"space_ruler/ultimate.png",192,192,13,13,96,132,1.600000f,2.000000f,false},
};
inline constexpr SpaceSheet spaceEffects[]={
    {"space_ruler/void-bolt.png",184,88,4,24,140,44,1.000000f,0.800000f,true},
    {"space_ruler/gravity-field.png",240,176,8,72,120,100,2.600000f,2.400000f,true},
    {"space_ruler/dimensional-spikes.png",304,156,7,91,24,116,1.800000f,3.033333f,false},
    {"space_ruler/repulsion-wave.png",320,224,9,97,160,176,1.750000f,3.233333f,false},
    {"space_ruler/singularity-collapse.png",320,284,11,115,160,240,2.400000f,2.000000f,false},
    {"space_ruler/flight-trail.png",112,72,5,36,70,52,2.200000f,1.200000f,true},
};
}
