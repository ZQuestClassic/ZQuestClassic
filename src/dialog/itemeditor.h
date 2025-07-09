#ifndef ZC_DIALOG_ITEMEDITOR_H_
#define ZC_DIALOG_ITEMEDITOR_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <gui/switcher.h>
#include <zq/gui/tileanim_frame.h>
#include <functional>
#include <string_view>
#include <map>
#include "items.h"
#include "base/flags.h"

void call_item_editor(int32_t index);

struct ItemNameInfo
{
	int32_t iclass;
	std::string power;
	std::string misc[10];
	std::string flag[16];
	std::string wpn[10];
	std::string actionsnd[2];
	std::string h_power;
	std::string h_misc[10];
	std::string h_flag[16];
	std::string h_wpn[10];
	std::string h_actionsnd[2];
	void clear()
	{
		iclass = -1;
		for(size_t q = 0; q < 16; ++q)
		{
			flag[q].clear();
			h_flag[q].clear();
			if(q > 9) continue;
			misc[q].clear();
			h_misc[q].clear();
			wpn[q].clear();
			h_wpn[q].clear();
			if (q > 1) continue;
			actionsnd[q].clear();
			h_actionsnd[q].clear();
		}
		power.clear();
		h_power.clear();
	}
};

extern ItemNameInfo inameinf[];
extern std::map<int32_t, ItemNameInfo *> *inamemap;
std::map<int32_t, ItemNameInfo *> *getItemNameMap();

class ItemEditorDialog: public GUI::Dialog<ItemEditorDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, DEFAULT, ITEMCLASS, RELOAD, GFXSIZE, WIZARD };
	

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	ItemEditorDialog(itemdata const& ref, char const* str, int32_t index);
	ItemEditorDialog(int32_t index);
	void loadItemClass();
	void refreshScripts();
	std::shared_ptr<GUI::Widget> IT_INITD(int index);
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tf_it_initd[8];
	std::shared_ptr<GUI::Label> l_attribs[10];
	std::shared_ptr<GUI::Label> l_it_initds[8];
	std::shared_ptr<GUI::Label> l_spr[10];
	std::shared_ptr<GUI::Checkbox> l_flags[16];
	std::shared_ptr<GUI::Label> l_power;
	std::shared_ptr<GUI::Label> l_sfx[2];
	std::string h_attribs[10];
	std::string h_spr[10];
	std::string h_it_initds[8];
	std::string h_flags[16];
	std::string h_power;
	std::string h_sfx[2];
	std::shared_ptr<GUI::Button> ib_attribs[10];
	std::shared_ptr<GUI::Button> ib_spr[10];
	std::shared_ptr<GUI::Button> ib_flags[16];
	std::shared_ptr<GUI::Button> ib_it_initds[8];
	std::shared_ptr<GUI::Button> ib_power;
	std::shared_ptr<GUI::Button> ib_sfx[2];
	std::shared_ptr<GUI::Button> wizardButton;
	std::shared_ptr<GUI::TileFrame> animFrame;
	std::shared_ptr<GUI::Switcher> animSwitcher;
	std::string itemname;
	int32_t index;
	itemdata local_itemref;
	GUI::ListData list_items, list_counters, list_sprites,
		list_itemdatscript, list_itemsprscript, list_weaponscript,
		list_bottletypes, list_sfx, list_strings;
	std::shared_ptr<GUI::Widget> ATTRIB_FIELD_IMPL(int32_t* mem, int index);
	std::shared_ptr<GUI::Widget> FLAG_CHECK(int index, item_flags bit);
	template <typename T>
	std::shared_ptr<GUI::Widget> SPRITE_DROP_IMPL(T* mem, int index);
	std::shared_ptr<GUI::Widget> MoveFlag(move_flags index, string const& str);
	friend void call_item_editor(int32_t index);
	friend class ItemWizardDialog;
};

#endif
