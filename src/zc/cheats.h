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
	TrigSecrets,
	TrigSecretsPerm,
	ShowL0,
	ShowL1,
	ShowL2,
	ShowL3,
	ShowL4,
	ShowL5,
	ShowL6,
	ShowFFC,
	ShowSprites,
	ShowWalkability,
	ShowEffects,
	ShowOverhead,
	ShowPushblock,
	ShowHitbox,
	ShowFFCScripts,
	Last,
};

extern int32_t cheatkeys[Cheat::Last][2];
Cheat cheat_from_string(std::string name);
std::string cheat_to_string(Cheat cheat);
std::string cheat_help(Cheat cheat);
int cheat_lvl(Cheat cheat);
bool bindable_cheat(Cheat cheat);
void cheats_hit_bind(Cheat cheat);
void cheats_enqueue(Cheat cheat, int arg1 = -1, int arg2 = -1, std::string arg3 = "");
void cheats_execute_queued();

#endif
