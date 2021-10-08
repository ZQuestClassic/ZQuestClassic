#ifndef ZC_DIALOG_INITDLG_H
#define ZC_DIALOG_INITDLG_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include <map>
#include <gui/switcher.h>

void call_init_dlg(zinitdata& sourcezinit, bool zc);

class InitDataDialog: public GUI::Dialog<InitDataDialog>
{
public:
	enum class message { OK, CANCEL, LEVEL };

	InitDataDialog(zinitdata const& start, bool zc, std::function<void(zinitdata const&)> setVals);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	void setOfs(size_t ofs);
	std::shared_ptr<GUI::TextField> sBombMax;
	std::shared_ptr<GUI::Label> l_lab[10];
	std::shared_ptr<GUI::Checkbox> l_maps[10];
	std::shared_ptr<GUI::Checkbox> l_comp[10];
	std::shared_ptr<GUI::Checkbox> l_bkey[10];
	std::shared_ptr<GUI::TextField> l_keys[10];
	std::shared_ptr<GUI::Switcher> icswitcher;
	std::map<int,int> switchids;
	zinitdata local_zinit;
	size_t levelsOffset;
	GUI::ListData list_dmaps, list_items;
	bool isZC;
	
	std::function<void(zinitdata const&)> setVals;
};

#endif
