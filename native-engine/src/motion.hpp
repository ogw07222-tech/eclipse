#pragma once
namespace eclipse {
struct MotionClip{int start,count;float duration,contact,stride;};
constexpr int thrustSkillState=13,sweepSkillState=14,prepSkillState=15;
constexpr int skillMotionColumns=4,skillMotionRows=6,skillMotionStart=208;
constexpr int skillMotionWidth=112,skillMotionPivotX=44;
constexpr bool isSkillMotion(int state){return state>=thrustSkillState&&state<=prepSkillState;}
constexpr MotionClip motionClips[]={
    {0,8,0.960000f,0.500000f,0.000000f},
    {32,16,0.700000f,0.500000f,196.000000f},
    {48,6,0.520000f,0.500000f,0.000000f},
    {64,6,0.650000f,0.500000f,0.000000f},
    {112,16,0.230000f,0.430000f,0.000000f},
    {128,10,0.340000f,0.610000f,0.000000f},
    {144,10,0.420000f,0.520000f,0.000000f},
    {96,8,0.150000f,0.500000f,0.000000f},
    {160,6,0.230000f,0.500000f,0.000000f},
    {176,12,0.900000f,0.500000f,0.000000f},
    {80,6,0.120000f,0.500000f,0.000000f},
    {16,8,0.600000f,0.500000f,72.000000f},
    {192,8,0.380000f,0.625000f,0.000000f},
    {208,4,.48f,.455f,0.f}, // thrust contact matches the extended pose
    {212,4,.415f,.50f,0.f},  // sweep contact matches the downswing
    {216,4,.40f,.50f,0.f},  // raise the sword, brace, recover
};
constexpr int motionWidth=64,motionHeight=94,motionColumns=16,motionRows=31,motionPivotX=28,motionPivotY=88;
constexpr int knightRunMidpointStart=13*motionColumns;
constexpr int knightMidpointStart=14*motionColumns;
constexpr int knightSprintStart=27*motionColumns,knightSprintFrames=54;
// Render 24 evenly distributed distinct poses from the existing 54-pose atlas.
constexpr int knightSprintPlaybackFrames=24;
// Presentation-only selection: 11 existing authored poses / .23s = 47.83 FPS.
// Keep the contact pose (source 6 at phase .43) and both ends; combat still
// uses all original clip timing/contact values. Trails share this visual map.
// Preserve the forward swing (5), contact (6), and follow-through (7,8).
// Drop near-rest/recovery poses instead of skipping those blade angles.
constexpr int knightWeakPlayback[]={0,2,4,5,6,7,8,10,12,14,15};
constexpr int knightWeakPlaybackFrames=11;
constexpr int knightWeakFrame(float phase){
    const int index=phase<=0?0:phase>=1?knightWeakPlaybackFrames-1:
        static_cast<int>(phase*knightWeakPlaybackFrames);
    return motionClips[4].start+knightWeakPlayback[index];
}
constexpr int whiteNightRunStart=88,whiteNightRunFrames=32;
// 32 poses / 1 second at the default 280 world units/s. Distance-driven,
// so gear and analog movement still scale cadence without changing velocity.
constexpr float whiteNightRunStride=280.f;
constexpr float motionScale=1.85f;
constexpr int mageMotionColumns=8,mageMotionRows=10,mageGroundCastRow=8,mageAirCastRow=9;
constexpr int mageAirPose(float phase){
    return phase<=0?0:phase>=1?mageMotionColumns-1:
        static_cast<int>(phase*mageMotionColumns);
}
constexpr int mageCastTipX=55,mageCastTipY=50;
constexpr float mageCastDuration=.32f,mageCastReleaseAge=mageCastDuration*3/mageMotionColumns;
constexpr float airSlamWindup=.10f,airSlamRecovery=.42f,airSlamDropSpeed=1100.f,airSlamRadius=68.f,airSlamImpactDuration=.28f;
constexpr int slamContactTipX=34,slamContactTipY=86;
}
