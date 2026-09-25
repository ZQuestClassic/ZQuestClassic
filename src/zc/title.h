#ifndef ZC_TITLE_H_
#define ZC_TITLE_H_

#include "zc/saves.h"
#include <optional>

extern DIALOG gamemode_dlg[];

// The save slot that the title screen cursor is currently on.
extern int32_t saveslot;

// The options on the screen that standalone mode shows after the game is quit.
enum class StandaloneQuitChoice
{
	QuitToDesktop,
	LoadLastSave,
};

// For -test-zc: when set, the standalone quit screen is skipped and this is picked instead.
extern std::optional<StandaloneQuitChoice> standalone_quit_choice_for_test;

bool prompt_for_quest_path(std::string current_qstpath);
save_t* get_unset_save_slot();
void titlescreen(int32_t lsave);
void game_over(int32_t type);
void save_game(bool savepoint);
bool save_game(bool savepoint, int32_t type);

#endif
