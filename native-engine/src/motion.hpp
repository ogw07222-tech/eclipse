#pragma once
namespace eclipse {
struct MotionClip{int start,count;float duration;};
constexpr MotionClip motionClips[]={
    {0,8,0.960000f},
    {32,16,0.343000f},
    {48,6,0.520000f},
    {64,6,0.650000f},
    {112,8,0.230000f},
    {128,10,0.250000f},
    {144,10,0.420000f},
    {96,8,0.150000f},
    {160,6,0.230000f},
    {176,12,0.900000f},
    {80,6,0.120000f},
    {16,8,0.600000f},
};
constexpr int motionCell=48,motionColumns=16,motionRows=12;
}
