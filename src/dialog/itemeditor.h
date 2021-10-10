#ifndef ZC_DIALOG_ITEMEDITOR_H
#define ZC_DIALOG_ITEMEDITOR_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <gui/tileanim_frame.h>
#include <functional>
#include <string_view>
#include <map>

void call_item_editor(int index);

struct ItemNameInfo
{
	int iclass;
	char *power;
	char *misc1;
	char *misc2;
	char *misc3;
	char *misc4;
	char *misc5;
	char *misc6;
	char *misc7;
	char *misc8;
	char *misc9;
	char *misc10;
	char *flag1;
	char *flag2;
	char *flag3;
	char *flag4;
	char *flag5;
	char *wpn1;
	char *wpn2;
	char *wpn3;
	char *wpn4;
	char *wpn5;
	char *wpn6;
	char *wpn7;
	char *wpn8;
	char *wpn9;
	char *wpn10;
	char *actionsnd;
	char *flag6;
	char *flag7;
	char *flag8;
	char *flag9;
	char *flag10;
	char *flag11;
	char *flag12;
	char *flag13;
	char *flag14;
	char *flag15;
	char *flag16;
};

extern ItemNameInfo inameinf[];
extern std::map<int, ItemNameInfo *> *inamemap;
std::map<int, ItemNameInfo *> *getItemNameMap();

class ItemEditorDialog: public GUI::Dialog<ItemEditorDialog>
{
public:
	enum class message { OK, CANCEL, DEFAULT, ITEMCLASS };
	

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	ItemEditorDialog(itemdata const& ref, char const* str, int index);
	ItemEditorDialog(int index);
	void loadItemClass();
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Label> l_attribs[10];
	std::shared_ptr<GUI::Checkbox> l_flags[16];
	std::shared_ptr<GUI::Label> l_power;
	std::shared_ptr<GUI::Label> l_sfx;
	std::shared_ptr<GUI::TileFrame> animFrame;
	std::string itemname;
	int index;
	itemdata local_itemref;
	GUI::ListData list_items, list_counters, list_sprites,
		list_itemdatscript, list_itemsprscript, list_weaponscript,
		list_weaptype, list_deftypes;
	friend void call_item_editor(int index);
};

#endif
