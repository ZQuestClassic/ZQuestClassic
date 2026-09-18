#ifndef INIT_H_
#define INIT_H_

#include "gui/tab_ctl.h"
#include "zalleg/zalleg.h"
#include <string>
#include "core/initdata.h"
#include "gamedata.h"

// isTestQuest: editing the zeditor Test Quest dialog's init data, which reaches the player as
// an init data delta (see serialize_init_data_delta) rather than through gamedata.
int32_t doInit(zinitdata *zinit, bool isZC, bool isTestQuest = false);
void resetItems(gamedata *data, zinitdata *zinit, bool freshquest);

std::string serialize_init_data_delta(zinitdata *base, zinitdata *changed);
zinitdata *apply_init_data_delta(zinitdata *base, std::string delta, std::string& out_error);
#endif
