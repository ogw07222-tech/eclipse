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
};
// Only explicitly approved entries are bound. Other roles retain legacy art.
inline constexpr std::array<FlashSheet,static_cast<int>(FlashActor::Count)> flashActors{{
    {"flash_executioner/actor_basic.png", 192, 192, 6, 5, 28, 70.0f, 133.0f, 1.4f, 0.46f, 6, false, true, -1, -1}, // Basic
    {"flash_executioner/actor_skill1.png", 192, 192, 6, 5, 30, 70.0f, 133.0f, 1.4f, 0.5f, 6, false, true, -1, -1}, // Skill1
    {"flash_executioner/actor_skill2.png", 192, 192, 6, 5, 28, 70.0f, 133.0f, 1.4f, 0.46f, 14, false, true, -1, -1}, // Skill2
    {"flash_executioner/actor_skill3.png", 192, 192, 5, 5, 22, 70.0f, 133.0f, 1.4f, 0.36f, 11, false, true, -1, -1}, // Skill3
    {}, // Ultimate: legacy retained
    {}, // Idle: legacy retained
    {}, // Run: legacy retained
    {}, // Jump: legacy retained
    {}, // Fall: legacy retained
    {"flash_executioner/actor_land.png", 192, 192, 3, 3, 7, 70.0f, 133.0f, 1.4f, 0.12f, -1, false, true, -1, -1}, // Land
    {"flash_executioner/actor_dash.png", 192, 192, 3, 3, 9, 70.0f, 133.0f, 1.4f, 0.15f, -1, false, true, -1, -1}, // Dash
    {"flash_executioner/actor_swap.png", 192, 192, 4, 4, 14, 70.0f, 133.0f, 1.4f, 0.24f, -1, false, true, -1, -1}, // Swap
}};
inline constexpr std::array<FlashSheet,static_cast<int>(FlashFx::Count)> flashEffects{{
    {"flash_executioner/effect_slash.png", 256, 224, 5, 5, 25, 116.0f, 163.0f, 1.0f, 0.4f, -1, false, true, -1, -1}, // Slash
    {"flash_executioner/effect_rush.png", 320, 176, 4, 6, 22, 190.0f, 132.0f, 1.0f, 0.35f, -1, false, true, -1, -1}, // Rush
    {"flash_executioner/effect_command.png", 224, 200, 5, 5, 25, 112.0f, 154.0f, 1.0f, 0.4f, -1, false, true, -1, -1}, // Command
    {"flash_executioner/effect_blinkout.png", 224, 208, 5, 4, 20, 112.0f, 160.0f, 1.0f, 0.32f, -1, false, true, -1, -1}, // BlinkOut
    {"flash_executioner/effect_blinkin.png", 224, 208, 5, 5, 23, 112.0f, 160.0f, 1.0f, 0.36f, -1, false, true, -1, -1}, // BlinkIn
    {"flash_executioner/effect_verdict.png", 288, 256, 7, 7, 45, 144.0f, 206.0f, 1.0f, 0.728f, -1, false, false, -1, -1}, // Verdict
    {"flash_executioner/effect_impact.png", 224, 200, 6, 6, 31, 112.0f, 130.0f, 1.0f, 0.5f, -1, false, true, -1, -1}, // Impact
    {"flash_executioner/effect_orb.png", 48, 48, 8, 8, 60, 24.0f, 24.0f, 1.0f, 1.0f, -1, true, true, -1, -1}, // Orb
    {"flash_executioner/effect_lance.png", 192, 80, 3, 7, 19, 90.0f, 45.0f, 0.85f, 0.3f, -1, false, true, 3, 8}, // Lance
    {"flash_executioner/effect_skylance.png", 80, 208, 8, 3, 20, 40.0f, 152.0f, 0.85f, 0.32f, -1, false, true, 3, 8}, // SkyLance
    {"flash_executioner/effect_lancehit.png", 336, 320, 7, 6, 40, 168.0f, 232.0f, 1.0f, 0.65f, -1, false, true, -1, -1}, // LanceHit
    {"flash_executioner/effect_dash.png", 256, 160, 4, 4, 16, 150.0f, 122.0f, 1.0f, 0.25f, -1, false, true, -1, -1}, // Dash
    {"flash_executioner/effect_jump.png", 208, 176, 4, 4, 16, 104.0f, 100.0f, 1.0f, 0.25f, -1, false, true, -1, -1}, // Jump
    {"flash_executioner/effect_land.png", 208, 176, 5, 4, 19, 104.0f, 100.0f, 1.0f, 0.3f, -1, false, true, -1, -1}, // Land
    {"flash_executioner/effect_swap.png", 256, 224, 5, 5, 25, 128.0f, 172.0f, 1.0f, 0.4f, -1, false, true, -1, -1}, // Swap
}};
}
