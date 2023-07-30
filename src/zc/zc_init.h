//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#ifndef _ZC_INIT_H_
#define _ZC_INIT_H_

#include <string>

int32_t onCheatConsole();
void center_zc_init_dialog();
zinitdata *copyIntoZinit(gamedata *src);
std::string serialize_init_data_delta(zinitdata *base, zinitdata *changed);
zinitdata *apply_init_data_delta(zinitdata *base, std::string delta);
#endif
 
