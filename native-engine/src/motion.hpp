#pragma once
namespace eclipse {
struct MotionClip{int start,count;float duration,contact,stride;};
constexpr MotionClip motionClips[]={
    {0,8,0.960000f,0.500000f,0.000000f},
    {32,16,0.428571f,0.500000f,120.000000f},
    {48,6,0.520000f,0.500000f,0.000000f},
    {64,6,0.650000f,0.500000f,0.000000f},
    {112,8,0.230000f,0.430000f,0.000000f},
    {128,10,0.340000f,0.610000f,0.000000f},
    {144,10,0.420000f,0.520000f,0.000000f},
    {96,8,0.150000f,0.500000f,0.000000f},
    {160,6,0.230000f,0.500000f,0.000000f},
    {176,12,0.900000f,0.500000f,0.000000f},
    {80,6,0.120000f,0.500000f,0.000000f},
    {16,8,0.600000f,0.500000f,72.000000f},
};
constexpr int motionWidth=64,motionHeight=48,motionColumns=16,motionRows=12,motionPivotX=24,motionPivotY=44;
}
