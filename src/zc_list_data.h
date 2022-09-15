#ifndef ZC_LIST_DATA_H
#define ZC_LIST_DATA_H

#include "gui/list_data.h"

namespace GUI::ZCListData
{
    GUI::ListData fonts(bool ss_fonts = false);
    GUI::ListData shadow_types();
    GUI::ListData ss_counters();
    GUI::ListData const& alignments();
    GUI::ListData const& buttons();
	GUI::ListData items(bool numbered = false);
    GUI::ListData itemclass(bool numbered = false);
	GUI::ListData combotype(bool numbered = false, bool skipNone = false);
	GUI::ListData mapflag(int32_t numericalFlags, bool numbered = false, bool skipNone = false);
	GUI::ListData counters(bool numbered = false, bool skipNone = false);
	GUI::ListData miscsprites(bool skipNone = true, bool inclNegSpecialVals = false);
	GUI::ListData bottletype();
	GUI::ListData dmaps(bool numbered = false);
	GUI::ListData lweaptypes();
	GUI::ListData sfxnames(bool numbered = false);
	GUI::ListData itemdata_script();
	GUI::ListData itemsprite_script();
	GUI::ListData ffc_script();
	GUI::ListData lweapon_script();
	GUI::ListData combodata_script();
    GUI::ListData const& deftypes();
}

#endif
