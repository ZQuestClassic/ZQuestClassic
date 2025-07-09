#ifndef ZC_DIALOG_INITDLG_H_
#define ZC_DIALOG_INITDLG_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/text_field.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include <map>
#include <gui/switcher.h>
#include "base/initdata.h"

void call_init_dlg(zinitdata& sourcezinit, bool zc);

class InitDataDialog: public GUI::Dialog<InitDataDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, LEVEL };

	InitDataDialog(zinitdata const& start, bool zc, std::function<void(zinitdata const&)> setVals);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	void setOfs(size_t ofs);
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> sBombMax;
	std::shared_ptr<GUI::Label> l_lab[10];
	std::shared_ptr<GUI::Checkbox> l_comp[10], l_maps[10], l_bkey[10], l_mcguff[10],
		l_bkill[10], l_custom1[10], l_custom2[10], l_custom3[10];
	std::shared_ptr<GUI::TextField> l_keys[10];
	std::shared_ptr<GUI::Switcher> icswitcher;
	std::shared_ptr<GUI::Button> genscr_btn;
	std::map<int32_t,int32_t> switchids;
	zinitdata local_zinit;
	size_t levelsOffset;
	GUI::ListData list_dmaps, list_items, list_genscr, list_bottle_content;
	bool isZC;
	
	std::function<void(zinitdata const&)> setVals;

	// Various helper functions to build the GUI.
	std::shared_ptr<GUI::Widget> WORD_FIELD(word* member);
	template <typename T>
	std::shared_ptr<GUI::Widget> VAL_FIELD_IMPL(T minval, T maxval, T* member, bool dis);
	std::shared_ptr<GUI::Widget> COUNTER_FRAME(int ctr);
	std::shared_ptr<GUI::Widget> BTN_100(int val);
	std::shared_ptr<GUI::Widget> BTN_10(int val);
};

class InitGenscriptWizard : public GUI::Dialog<InitGenscriptWizard>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	InitGenscriptWizard(zinitdata& start, size_t index);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Button> databtn;
	zinitdata local_zinit;
	zinitdata& dest_zinit;
	size_t index;
	GUI::ListData list_dmaps, list_items;
	
	std::shared_ptr<GUI::Widget> GEN_INITD(int ind,zasm_meta const& meta);
};
#endif
