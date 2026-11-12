#pragma once
#include <array>
namespace eclipse {
enum class FlashActor { Basic, Skill1, Skill2, Skill3, Ultimate, Idle, Run, Jump, Fall, Land, Dash, Swap, Count };
enum class FlashFx { Slash, Rush, Command, BlinkOut, BlinkIn, Verdict, Impact, Orb, Lance, SkyLance, LanceHit, Dash, Jump, Land, Swap, Count };
struct FlashSheet {
    const char* file=nullptr;
    int cellW=0,cellH=0,columns=0,rows=0,count=0;
    float pivotX=0,pivotY=0,scale=1,duration=0;
    int contactFrame=-1;
    bool loop=false;
    bool front=true;
    int flightFirst=-1,flightLast=-1;
    float triggerLeadSeconds=0,spawnAgeSeconds=0,nativeDirectionDegrees=0,flightFrameFps=60;
    float offsetX=0,offsetY=0;
    int targetFeet=0;
    float fps=60;
    float triggerStartAgeSeconds=-1;
};
// Only explicitly approved entries are bound. Other roles retain legacy art.
inline constexpr std::array<FlashSheet,static_cast<int>(FlashActor::Count)> flashActors{{
    {"flash_executioner/actor_basic.png", 192, 192, 6, 5, 28, 70.0f, 133.0f, 1.4f, 0.46f, 6, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Basic
    {"flash_executioner/actor_skill1.png", 192, 192, 6, 5, 30, 70.0f, 133.0f, 1.4f, 0.5f, 6, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Skill1
    {"flash_executioner/actor_skill2.png", 192, 192, 6, 5, 28, 70.0f, 133.0f, 1.4f, 0.46f, 14, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Skill2
    {"flash_executioner/actor_skill3.png", 192, 192, 5, 5, 22, 70.0f, 133.0f, 1.4f, 0.36f, 11, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Skill3
    {}, // Ultimate: legacy retained
    {}, // Idle: legacy retained
    {}, // Run: legacy retained
    {}, // Jump: legacy retained
    {}, // Fall: legacy retained
    {"flash_executioner/actor_land.png", 192, 192, 3, 3, 7, 70.0f, 133.0f, 1.4f, 0.12f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Land
    {"flash_executioner/actor_dash.png", 192, 192, 3, 3, 9, 70.0f, 133.0f, 1.4f, 0.15f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Dash
    {"flash_executioner/actor_swap.png", 192, 192, 4, 4, 14, 70.0f, 133.0f, 1.4f, 0.24f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Swap
}};
inline constexpr std::array<FlashSheet,static_cast<int>(FlashFx::Count)> flashEffects{{
    {"flash_executioner/effect_slash.png", 256, 216, 5, 5, 25, 105.0f, 165.0f, 2.0f, 0.4f, -1, false, true, -1, -1, 0.03333333333333333f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Slash
    {"flash_executioner/effect_rush.png", 320, 160, 4, 6, 22, 80.0f, 110.0f, 2.0f, 0.35f, -1, false, true, -1, -1, 0.05f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Rush
    {"flash_executioner/effect_command.png", 160, 144, 5, 5, 25, 64.0f, 124.0f, 2.1f, 0.4f, -1, false, true, -1, -1, 0.2f, 0.0f, 0.0f, 60.0f, 10.0f, -25.0f, 0, 60.0f, -1.0f}, // Command
    {"flash_executioner/effect_blinkout.png", 288, 224, 4, 5, 20, 144.0f, 174.0f, 1.4f, 0.32f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, 0.033333333333333326f}, // BlinkOut
    {"flash_executioner/effect_blinkin.png", 288, 224, 5, 5, 23, 144.0f, 174.0f, 1.4f, 0.36f, -1, false, true, -1, -1, 0.0f, 0.13333333333333333f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // BlinkIn
    {"flash_executioner/effect_verdict.png", 256, 280, 8, 6, 45, 128.0f, 230.0f, 1.7f, 0.728f, -1, false, false, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Verdict
    {"flash_executioner/effect_impact.png", 320, 224, 5, 7, 31, 130.0f, 112.0f, 1.7f, 0.5f, -1, false, true, -1, -1, 0.0f, 0.016666666666666666f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Impact
    {"flash_executioner/effect_orb.png", 136, 112, 11, 14, 144, 68.0f, 104.0f, 0.3f, 2.4f, -1, true, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 15.6f, 0, 60.0f, -1.0f}, // Orb
    {"flash_executioner/effect_lance.png", 208, 96, 5, 10, 48, 170.0f, 48.0f, 1.3f, 0.8f, -1, true, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Lance
    {"flash_executioner/effect_skylance.png", 208, 96, 5, 10, 48, 170.0f, 48.0f, 1.8f, 0.8f, -1, true, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // SkyLance
    {"flash_executioner/effect_lancehit.png", 288, 232, 6, 7, 40, 144.0f, 188.0f, 1.5f, 0.65f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 1, 60.0f, -1.0f}, // LanceHit
    {"flash_executioner/effect_dash.png", 320, 224, 4, 4, 16, 190.0f, 174.0f, 1.4f, 0.25f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Dash
    {"flash_executioner/effect_jump.png", 208, 128, 4, 4, 16, 104.0f, 88.0f, 0.55f, 0.25f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Jump
    {"flash_executioner/effect_land.png", 208, 128, 4, 5, 19, 104.0f, 88.0f, 1.0f, 0.3f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Land
    {"flash_executioner/effect_swap.png", 256, 224, 5, 5, 25, 128.0f, 174.0f, 1.6f, 0.4f, -1, false, true, -1, -1, 0.0f, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0, 60.0f, -1.0f}, // Swap
}};
}
