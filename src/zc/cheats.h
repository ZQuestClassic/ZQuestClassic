#ifndef _CHEATS_H_
#define _CHEATS_H_

#include <vector>
#include <string>

// Always add to the end of this enum (before Last).
enum Cheat
{
    None,
    Life,
    Magic,
    Rupies,
    Bombs,
    Arrows,
    Kill,
    Fast,
    Clock,
    Walls,
    Freeze,
    Light,
    GoTo,
    IgnoreSideView,
    MaxLife,
    MaxMagic,
    MaxBombs,
    PlayerData,
    Last,
};

Cheat cheat_from_string(std::string name);
std::string cheat_to_string(Cheat cheat);
void cheats_enqueue(Cheat cheat, int arg1 = -1, int arg2 = -1, std::string arg3 = "");
void cheats_execute_queued();

#endif
