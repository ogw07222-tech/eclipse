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
    {192,8,0.380000f,0.625000f,0.000000f},
};
constexpr int motionWidth=64,motionHeight=94,motionColumns=16,motionRows=13,motionPivotX=28,motionPivotY=88;
constexpr float motionScale=1.85f;
constexpr int mageMotionColumns=8,mageMotionRows=10,mageGroundCastRow=8,mageAirCastRow=9;
constexpr int mageCastTipX=55,mageCastTipY=50;
constexpr float mageCastDuration=.32f,mageCastReleaseAge=mageCastDuration*3/mageMotionColumns;
constexpr float airSlamWindup=.10f,airSlamRecovery=.42f,airSlamDropSpeed=1100.f,airSlamRadius=68.f,airSlamImpactDuration=.28f;
constexpr int slamContactTipX=34,slamContactTipY=86;
}
