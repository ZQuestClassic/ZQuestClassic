#include "info_lister.h"
#include <gui/builder.h>
#include <base/new_menu.h>
#include "base/files.h"
#include "itemeditor.h"
#include "ffc_editor.h"
#include "spritedata.h"
#include "enemyeditor.h"
#include "midieditor.h"
#include "save_menu_editor.h"
#include "sfxdata.h"
#include "edit_dmap.h"
#include "info.h"
#include "zc_list_data.h"
#include "zq/zquest.h"
#include "zq/zq_misc.h"
#include "zq/zq_class.h"
#include "zq/zq_custom.h"
#include "base/qst.h"
#include "base/dmap.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <utility>
#include <sstream>
#include "advanced_music.h"

extern char *item_string[];
extern char *weapon_string[];

static string get_info(bool sel, bool advpaste, bool saveload = true, bool copypaste = true)
{
	std::ostringstream oss;
	if(copypaste)
		oss << "Ctrl+C/Ctrl+V - Copy/Paste\n";
	if(advpaste)
		oss << "Ctrl+A or Ctrl+Shift+V - Adv. Paste\n";
	if(saveload)
		oss << "Ctrl+S/Ctrl+L - Save/Load\n";
	oss << "Double-Click - " << (sel ? "Confirm" : "Edit");
	return oss.str();
}

int lister_sel_val = -1;

std::shared_ptr<GUI::Widget> BasicListerDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	lister_sel_val = start_val;
	
	widgList.reset(); // make sure calling `resort` from `preinit` is safe
	btnrow2.reset();
	
	preinit();
	
	std::shared_ptr<GUI::Grid> g, wcolumn;
	window = Window(
		title = titleTxt,
		onClose = message::EXIT,
		hPadding = 0_px,
		use_vsync = true,
		// Don't set info text, because the following shortcuts are only utilized if the subclass implements the functions.
		// Subclasses should use window->setHelp(get_info(...))
		shortcuts={
			Ctrl+C=message::COPY,
			Ctrl+V=message::PASTE,
			Ctrl+A=message::ADV_PASTE,
			Ctrl+Shift+V=message::ADV_PASTE,
			Ctrl+S=message::SAVE,
			Ctrl+L=message::LOAD,
			Enter=message::CONFIRM,
			Del=message::CLEAR,
		},
		wcolumn = Column(hPadding = 0_px)
	);
	
	if (use_alpha)
		wcolumn->add(Row(vPadding = 0_px, fitParent = true,
			Checkbox(text = "Alphabetized",
				hAlign = 0.0,
				checked = alphabetized,
				onToggleFunc = [&](bool state)
				{
					alphabetized = state;
					set_config("alphabetized", state);
					resort();
				})
		));
	
	wcolumn->add(g = Columns<3>(
		widgList = List(data = lister, isABC = true,
			selectedValue = selected_val,
			rowSpan = 3, fitParent = true,
			focused = true,
			onSelectFunc = [&](int32_t val)
			{
				if(selected_val == val)
					return;
				selected_val = val;
				update();
			},
			onRClickFunc = [&](int32_t val, int32_t x, int32_t y)
			{
				if(selected_val != val)
				{
					selected_val = val;
					update();
				}
				forceDraw();
				rclick(x,y);
			},
			onDClick = message::CONFIRM)
	));
	
	wcolumn->add(btnrow = Row(padding = 0_px));
	
	//Generate the btnrow
	{
		if(!editable || selecting)
			btnrow->add(Button(
				text = "OK",
				topPadding = 0.5_em,
				minwidth = 90_px,
				onClick = message::OK));
		if(editable)
			btnrow->add(Button(
				text = "Edit",
				topPadding = 0.5_em,
				minwidth = 90_px,
				onClick = message::EDIT));
		btnrow->add(Button(
			text = selecting?"Cancel":"Done",
			topPadding = 0.5_em,
			minwidth = 90_px,
			onClick = message::EXIT));
	}
		
	if (use_preview)
	{
		g->add(Column(prev_holder = Column(padding = 0_px,
			widgPrev = TileFrame(visible = false)
		)));
		g->add(widgInfo = Label(text = "", fitParent = true));
	}
	else if(use_mappreview)
	{
		g->add(mapPrev = DMapFrame(visible = false));
		g->add(widgInfo = Label(text = "", fitParent = true));
	}
	else
	{
		g->add(widgInfo = Label(text = "", fitParent = true, rowSpan = 2));
	}
	
	resort();
	postinit();
	update(true);
	
	if (btnrow2)
		g->add(btnrow2);
	else widgList->setRowSpan(2);
	
	return window;
}

void BasicListerDialog::resort()
{
	if(alphabetized)
		lister.alphabetize(frozen_start, frozen_end);
	else
		lister.valsort(frozen_start, frozen_end);
	if(widgList)
		widgList->setSelectedValue(selected_val);
}

bool BasicListerDialog::get_config(string const& name, bool default_val)
{
	return get_config(name, default_val ? 1 : 0) != 0;
}
int32_t BasicListerDialog::get_config(string const& name, int32_t default_val)
{
	if(cfg_key.empty()) return default_val;
	return zc_get_config("zquest", fmt::format("lister_{}_{}", cfg_key, name).c_str(), default_val);
}
string BasicListerDialog::get_config(string const& name, string const& default_val)
{
	if(cfg_key.empty()) return default_val;
	return zc_get_config("zquest", fmt::format("lister_{}_{}", cfg_key, name).c_str(), default_val.c_str());
}
double BasicListerDialog::get_config(string const& name, double default_val)
{
	if(cfg_key.empty()) return default_val;
	return zc_get_config("zquest", fmt::format("lister_{}_{}", cfg_key, name).c_str(), default_val);
}
void BasicListerDialog::set_config(string const& name, bool value)
{
	set_config(name, value ? 1 : 0);
}
void BasicListerDialog::set_config(string const& name, int32_t value)
{
	if(cfg_key.empty()) return;
	zc_set_config("zquest", fmt::format("lister_{}_{}", cfg_key, name).c_str(), value);
}
void BasicListerDialog::set_config(string const& name, string const& value)
{
	if(cfg_key.empty()) return;
	zc_set_config("zquest", fmt::format("lister_{}_{}", cfg_key, name).c_str(), value.c_str());
}
void BasicListerDialog::set_config(string const& name, double value)
{
	if(cfg_key.empty()) return;
	zc_set_config("zquest", fmt::format("lister_{}_{}", cfg_key, name).c_str(), value);
}

bool BasicListerDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	bool refresh = false;
	auto m = msg.message;
	if(m == message::CONFIRM)
		m = selecting ? message::OK : message::EDIT;
	switch(m)
	{
		case message::OK:
			lister_sel_val = selected_val;
			return true;
		case message::EXIT:
			return true;
		case message::CLEAR:
			refresh = clear();
			break;
		case message::EDIT:
			edit();
			refresh = true;
			break;
		case message::COPY:
			copy();
			break;
		case message::PASTE:
			refresh = paste();
			break;
		case message::ADV_PASTE:
			refresh = adv_paste();
			break;
		case message::SAVE:
			save();
			break;
		case message::LOAD:
			refresh = load();
			break;
	}
	if(refresh)
		rerun_dlg = true;
	return rerun_dlg;
}

ItemListerDialog::ItemListerDialog(int itemid, bool selecting):
	BasicListerDialog("Select Item","itemdata",itemid,selecting)
{
	use_preview = true;
	alphabetized = get_config("alphabetized", true);
}
void ItemListerDialog::preinit()
{
	lister = GUI::ZCListData::items(true);
	if(selecting)
		frozen_start = 1; // lock '(None)'
	else
	{
		lister.removeInd(0); // remove '(None)'
		resort();
		if(selected_val < 0)
			selected_val = lister.getValue(0);
	}
	selected_val = vbound(selected_val, (selecting?-1:0), MAXITEMS-1);
}
void ItemListerDialog::postinit()
{
	size_t len = 16;
	for(int q = 0; q < MAXITEMS; ++q)
	{
		size_t tlen = text_length(GUI_DEF_FONT,itemsbuf[q].get_name(true).c_str());
		if(tlen > len)
			len = tlen;
		tlen = text_length(GUI_DEF_FONT,item_string[q]);
		if(tlen > len)
			len = tlen;
	}
	widgInfo->minWidth(Size::pixels(len+8));
	window->setHelp(get_info(selecting, true));
}
static int copied_item_id = -1;
void ItemListerDialog::update(bool)
{
	std::string copied_name = "(None)\n";
	if(unsigned(copied_item_id) < MAXITEMS)
	{
		itemdata const& copied_itm = itemsbuf[copied_item_id];
		copied_name = fmt::format("{}\n{}",item_string[copied_item_id],
			copied_itm.display_name[0] ? copied_itm.get_name(true) : "[No Display Name]");
	}
	if(unsigned(selected_val) < MAXITEMS)
	{
		itemdata const& itm = itemsbuf[selected_val];
		std::string display_name = itm.display_name[0]
			? itm.get_name(true)
			: "[No Display Name]";
		widgInfo->setText(fmt::format(
			"{}\n{}\n#{}\nPower: {}\nLevel: {}"
			"\nType: {}\nCSet: {}\nScripts:\nAction: {}\nPickup: {}\nSprite: {}\nWeapon: {}"
			"\n\nCopied:\n{}",
			item_string[selected_val], display_name, selected_val, itm.power, itm.level,
			itm.type, itm.csets&0xF, itm.script, itm.collect_script, itm.sprite_script, itm.weap_data.script,
			copied_name));
		widgPrev->setDisabled(false);
		widgPrev->setTile(itm.tile);
		widgPrev->setCSet(itm.csets&0xF);
		widgPrev->setFrames(itm.frames);
		widgPrev->setSpeed(itm.speed);
		widgPrev->setDelay(itm.delay);
		widgPrev->setSkipX((itm.overrideFLAGS & OVERRIDE_TILE_WIDTH)
			? itm.tilew-1 : 0);
		widgPrev->setSkipY((itm.overrideFLAGS & OVERRIDE_TILE_HEIGHT)
			? itm.tileh-1 : 0);
		widgPrev->setFlashCS((itm.misc_flags & 1) ? itm.csets >> 4 : -1);
	}
	else
	{
		widgInfo->setText(fmt::format(
			"\n\n\n\n"
			"\n\n\n\n\n\n\n"
			"\n\nCopied:\n{}",
			copied_name));
		widgPrev->setDisabled(true);
		widgPrev->setTile(0);
		widgPrev->setCSet(0);
		widgPrev->setFrames(0);
		widgPrev->setSpeed(0);
		widgPrev->setDelay(0);
		widgPrev->setSkipX(0);
		widgPrev->setSkipY(0);
		widgPrev->setFlashCS(-1);
	}
	widgPrev->setVisible(true);
	widgPrev->setDoSized(true);
	widgPrev->overrideWidth(Size::pixels(16 * 3 + 4));
	widgPrev->overrideHeight(Size::pixels(16 * 3 + 4));
	widgPrev->resetAnim();
}
void ItemListerDialog::edit()
{
	call_item_editor(selected_val);
}
void ItemListerDialog::rclick(int x, int y)
{
	NewMenu rcmenu {
		{ "&Copy", [&](){copy();} },
		{ "&Adv. Paste", [&](){adv_paste();}, 0, copied_item_id < 0 ? MFL_DIS : 0 },
		{ "Paste", "&v", [&](){paste();}, 0, copied_item_id < 0 ? MFL_DIS : 0 },
		{ "&Save", [&](){save();} },
		{ "&Load", [&](){load();} },
	};
	rcmenu.pop(x, y);
}
void ItemListerDialog::copy()
{
	copied_item_id = selected_val;
	update();
}
bool ItemListerDialog::paste()
{
	if(copied_item_id < 0 || selected_val < 0)
		return false;
	if(copied_item_id == selected_val)
		return false;
	itemsbuf[selected_val] = itemsbuf[copied_item_id];
	refresh_dlg();
	mark_save_dirty();
	return true;
}
bool ItemListerDialog::adv_paste()
{
	if(copied_item_id < 0 || selected_val < 0)
		return false;
	if(copied_item_id == selected_val)
		return false;
	static bitstring pasteflags;
	static const vector<CheckListInfo> advp_names =
	{
		{ "Name" },
		{ "Display Name" },
		{ "Itemclass" },
		{ "Equipment Item" },
		{ "Attributes", "The 10 type-based attributes, as well"
			" as 'Level' and 'Power', from the 'Attrib' tab" },
		{ "Generic Flags", "Does not include 'Constant Script'" },
		{ "Flags", "The 15 type-based flags on the 'Flags' tab" },
		{ "Use Costs" },
		{ "Use SFX" },
		{ "Pickup", "Everything related to 'Pickup', excluding the"
			" 'Pickup Flags' tab and pickup string settings." },
		{ "Pickup Strings" },
		{ "Pickup Flags" },
		{ "Weapon Data" },
		{ "Graphics" },
		{ "Tile Mod" },
		{ "Sprites" },
		{ "Item Size" },
		{ "Weapon Size" },
		{ "Item Scripts", "Includes 'Constant Script', all 3 item scripts, and related parameters" },
		{ "Weapon Script" },
	};
	if(!call_checklist_dialog("Advanced Paste",advp_names,pasteflags))
		return false;
	itemsbuf[selected_val].advpaste(itemsbuf[copied_item_id], pasteflags);
	if(pasteflags.get(ITM_ADVP_NAME))
		strcpy(item_string[selected_val], item_string[copied_item_id]);
	refresh_dlg();
	mark_save_dirty();
	return true;
}
int32_t readoneitem(PACKFILE *f, int32_t id);
int32_t writeoneitem(PACKFILE *f, int32_t id);
void ItemListerDialog::save()
{
	if(selected_val < 0)
		return;
	if(!prompt_for_new_file_compat(fmt::format("Save Item '{}' #{} (.zitem)",itemsbuf[selected_val].get_name(true),selected_val).c_str(),"zitem",NULL,datapath,false))
		return;
	
	PACKFILE *f=pack_fopen_password(temppath,F_WRITE,"");
	if(!f) return;
	if (!writeoneitem(f,selected_val))
	{
		Z_error("Could not write to .zitem packfile %s\n", temppath);
		InfoDialog("ZItem Error", "Could not save the specified item.").show();
	}
	pack_fclose(f);
}
bool ItemListerDialog::load()
{
	if(selected_val < 0)
		return false;
	if(!prompt_for_existing_file_compat(fmt::format("Load Item (replacing '{}' #{}) (.zitem)",itemsbuf[selected_val].get_name(true),selected_val).c_str(),"zitem",NULL,datapath,false))
		return false;
	
	PACKFILE *f=pack_fopen_password(temppath,F_READ,"");
	if(!f) return false;
	if (!readoneitem(f,selected_val))
	{
		Z_error("Could not read from .zitem packfile %s\n", temppath);
		InfoDialog("ZItem Error", "Could not load the specified item.").show();
	}
	pack_fclose(f);
	refresh_dlg();
	mark_save_dirty();
	return true;
}

SpriteListerDialog::SpriteListerDialog(int spriteid, bool selecting):
	BasicListerDialog("Select Sprite","spritedata",spriteid,selecting)
{
	use_preview = true;
	alphabetized = get_config("alphabetized", true);
}
void SpriteListerDialog::preinit()
{
	lister = GUI::ZCListData::miscsprites(!selecting, false, true);
	if(selecting)
		frozen_start = 1; // lock '(None)'
	else
	{
		resort();
		if(selected_val < 0)
			selected_val = lister.getValue(0);
	}
	selected_val = vbound(selected_val, (selecting?-1:0), MAXWPNS-1);
}
void SpriteListerDialog::postinit()
{
	size_t len = 16;
	for(int q = 0; q < MAXWPNS; ++q)
	{
		size_t tlen = text_length(GUI_DEF_FONT,weapon_string[q]);
		if(tlen > len)
			len = tlen;
	}
	widgInfo->minWidth(Size::pixels(len+8));
	widgList->minHeight(Size::pixels(320));
	window->setHelp(get_info(selecting, true));
}
static int copied_sprite_id = -1;
void SpriteListerDialog::update(bool)
{
	std::string copied_name = "(None)\n";
	if(unsigned(copied_sprite_id) < MAXWPNS)
		copied_name = weapon_string[copied_sprite_id];
	if(unsigned(selected_val) < MAXWPNS)
	{
		wpndata const& spr = wpnsbuf[selected_val];
		widgInfo->setText(fmt::format(
			"{}\nCSet: {}\nFlip: {}\nFrames: {}\nSpeed: {}"
			"\n\nCopied:\n{}",
			weapon_string[selected_val], spr.cs(), spr.flip(), spr.frames, spr.speed,
			copied_name));
		widgPrev->setDisabled(false);
		widgPrev->setTile(spr.tile);
		widgPrev->setCSet(spr.cs());
		widgPrev->setFrames(spr.frames);
		widgPrev->setSpeed(spr.speed);
		widgPrev->setFlashCS(-1);
	}
	else
	{
		widgInfo->setText(fmt::format(
			"\n\n\n\n"
			"\n\nCopied:\n{}",
			copied_name));
		widgPrev->setDisabled(true);
		widgPrev->setTile(0);
		widgPrev->setCSet(0);
		widgPrev->setFrames(0);
		widgPrev->setSpeed(0);
		widgPrev->setFlashCS(-1);
	}
	widgPrev->setVisible(true);
	widgPrev->overrideWidth(Size::pixels(16 * 3 + 4));
	widgPrev->overrideHeight(Size::pixels(16 * 3 + 4));
	widgPrev->resetAnim();
}
void SpriteListerDialog::edit()
{
	call_sprite_dlg(selected_val);
}
void SpriteListerDialog::rclick(int x, int y)
{
	NewMenu rcmenu {
		{ "&Copy", [&](){copy();} },
		{ "Paste", "&v", [&](){paste();}, 0, copied_sprite_id < 0 ? MFL_DIS : 0 },
		{ "&Save", [&](){save();} },
		{ "&Load", [&](){load();} },
	};
	rcmenu.pop(x, y);
}
void SpriteListerDialog::copy()
{
	copied_sprite_id = selected_val;
	update();
}
bool SpriteListerDialog::paste()
{
	if(copied_sprite_id < 0 || selected_val < 0)
		return false;
	if(copied_sprite_id == selected_val)
		return false;
	wpnsbuf[selected_val] = wpnsbuf[copied_sprite_id];
	refresh_dlg();
	mark_save_dirty();
	return true;
}
int32_t readoneweapon(PACKFILE *f, int32_t id);
int32_t writeoneweapon(PACKFILE *f, int32_t id);
void SpriteListerDialog::save()
{
	if(selected_val < 0)
		return;
	if(!prompt_for_new_file_compat(fmt::format("Save Sprite '{}' #{} (.zwpnspr)",weapon_string[selected_val],selected_val).c_str(),"zwpnspr",NULL,datapath,false))
		return;
	
	PACKFILE *f=pack_fopen_password(temppath,F_WRITE,"");
	if(!f) return;
	if (!writeoneweapon(f,selected_val))
	{
		Z_error("Could not write to .zwpnspr packfile %s\n", temppath);
		InfoDialog("ZWpnSpr Error", "Could not save the specified sprite.").show();
	}
	pack_fclose(f);
}
bool SpriteListerDialog::load()
{
	if(selected_val < 0)
		return false;
	if(!prompt_for_existing_file_compat(fmt::format("Load Sprite (replacing '{}' #{}) (.zwpnspr)",weapon_string[selected_val],selected_val).c_str(),"zwpnspr",NULL,datapath,false))
		return false;
	
	PACKFILE *f=pack_fopen_password(temppath,F_READ,"");
	if(!f) return false;
	if (!readoneweapon(f,selected_val))
	{
		Z_error("Could not read from .zwpnspr packfile %s\n", temppath);
		InfoDialog("ZWpnSpr Error", "Could not load the specified sprite.").show();
	}
	pack_fclose(f);
	refresh_dlg();
	mark_save_dirty();
	return true;
}

SubscrWidgListerDialog::SubscrWidgListerDialog():
	BasicListerDialog("Select Widget Type","subscr_widgets",0,true)
{
	alphabetized = get_config("alphabetized", true);
	lister = GUI::ZCListData::subscr_widgets();
	lister.removeInd(0); //remove '(None)'
	selected_val = lister.getValue(0);
	editable = false;
}
void SubscrWidgListerDialog::preinit()
{
}
void SubscrWidgListerDialog::postinit()
{
	widgInfo->setMaxLines(15);
	widgInfo->overrideWidth(150_px);
	widgList->setSelectedIndex(0);
}
void SubscrWidgListerDialog::update(bool)
{
	if(selected_val && unsigned(selected_val) < widgMAX)
		widgInfo->setText(lister.findInfo(selected_val));
}

EnemyListerDialog::EnemyListerDialog(int enemyid, bool selecting):
	BasicListerDialog("Select Enemy", "enemydata", enemyid, selecting)
{
	use_preview = true;
	alphabetized = get_config("alphabetized", true);
}
void EnemyListerDialog::preinit()
{
	lister = GUI::ZCListData::enemies(true);
	if(selecting)
		frozen_start = 1; // lock '(None)'
	else
	{
		lister.removeInd(0); // remove '(None)'
		if (selected_val < 0)
			selected_val = lister.getValue(0);
	}
	selected_val = vbound(selected_val, (selecting ? -1 : 0), MAXGUYS - 1);
}
void EnemyListerDialog::postinit()
{
	size_t len = 16;
	for (int q = 0; q < MAXGUYS; ++q)
	{
		size_t tlen = text_length(GUI_DEF_FONT, guy_string[q]);
		if (tlen > len)
			len = tlen;
	}
	widgInfo->minWidth(Size::pixels(len + 8));
	window->setHelp(get_info(selecting, true));
}
static int copied_enemy_id = -1;
void EnemyListerDialog::update(bool)
{
	std::string copied_name = "(None)\n";
	if (unsigned(copied_enemy_id) < MAXGUYS)
	{
		guydata const& copied_enemy = guysbuf[copied_enemy_id];
		copied_name = fmt::format("{}", guy_string[copied_enemy_id]);
	}
	if (unsigned(selected_val) < MAXGUYS)
	{
		guydata const& enemy = guysbuf[selected_val];
		widgInfo->setText(fmt::format(
			"#{}\nTile: {}\nsTile: {}"
			"\neTile: {}\nHP: {}\nDamage: {}\nW. Damage: {}\nFamily: {}\nDrop: {}\nScript: {}\nW Script: {}"
			"\n\nCopied:\n{}",
			selected_val, enemy.tile, enemy.s_tile,
			enemy.e_tile, enemy.hp, enemy.dp, enemy.wdp, enemy.type, enemy.item_set, enemy.script, enemy.weap_data.script,
			copied_name));
		if(unsigned(selected_val) > 0)
		{
			widgPrev->setDisabled(false);
			if (get_qr(qr_NEWENEMYTILES))
				widgPrev->setTile(enemy.e_tile + efrontfacingtile(selected_val));
			else
				widgPrev->setTile(enemy.tile + efrontfacingtile(selected_val));
			widgPrev->setCSet(enemy.cset & 0xF);
			widgPrev->setFrames(0);
			widgPrev->setSpeed(0);
			widgPrev->setDelay(0);
			widgPrev->setSkipX((enemy.SIZEflags & OVERRIDE_TILE_WIDTH)
				? enemy.txsz - 1 : 0);
			widgPrev->setSkipY((enemy.SIZEflags & OVERRIDE_TILE_HEIGHT)
				? enemy.tysz - 1 : 0);
		}
		else
			widgPrev->setDisabled(true);
	}
	else
	{
		widgInfo->setText(fmt::format(
			"\n\n\n\n"
			"\n\n\n\n\n\n\n"
			"\n\nCopied:\n{}",
			copied_name));
		widgPrev->setDisabled(true);
		widgPrev->setTile(0);
		widgPrev->setCSet(0);
		widgPrev->setFrames(0);
		widgPrev->setSpeed(0);
		widgPrev->setDelay(0);
		widgPrev->setSkipX(0);
		widgPrev->setSkipY(0);
	}
	widgPrev->setVisible(true);
	widgPrev->setDoSized(true);
	widgPrev->overrideWidth(Size::pixels(16 * 3 + 4));
	widgPrev->overrideHeight(Size::pixels(16 * 3 + 4));
}
void EnemyListerDialog::edit()
{
	call_enemy_editor(selected_val);
}
void EnemyListerDialog::rclick(int x, int y)
{
	NewMenu rcmenu{
		{ "&Copy", [&]() {copy(); } },
		//{ "&Adv. Paste", [&]() {if(adv_paste()) refresh_dlg(); }, 0, copied_enemy_id < 0 ? MFL_DIS : 0 },
		{ "Paste", "&v", [&]() {paste();}, 0, copied_enemy_id < 0 ? MFL_DIS : 0 },
		{ "&Save", [&]() {save(); } },
		{ "&Load", [&]() {load(); } },
	};
	rcmenu.pop(x, y);
}
void EnemyListerDialog::copy()
{
	copied_enemy_id = selected_val;
	update();
}
bool EnemyListerDialog::paste()
{
	if (unsigned(copied_enemy_id) >= MAXGUYS || unsigned(selected_val) >= MAXGUYS)
		return false;
	if (copied_enemy_id == selected_val)
		return false;
	guysbuf[selected_val] = guysbuf[copied_enemy_id];
	refresh_dlg();
	mark_save_dirty();
	return true;
}
int32_t readonenpc(PACKFILE* f, int32_t id);
int32_t writeonenpc(PACKFILE* f, int32_t id);
void EnemyListerDialog::save()
{
	if (selected_val < 0)
		return;
	if (!prompt_for_new_file_compat(fmt::format("Save NPC '{}' #{} (.znpc)", guy_string[selected_val], selected_val).c_str(), "znpc", NULL, datapath, false))
		return;

	PACKFILE* f = pack_fopen_password(temppath, F_WRITE, "");
	if (!f) return;
	if (!writeonenpc(f, selected_val))
	{
		Z_error("Could not write to .znpc packfile %s\n", temppath);
		InfoDialog("ZNPC Error", "Could not save the specified NPC.").show();
	}
	pack_fclose(f);
}
bool EnemyListerDialog::load()
{
	if (selected_val < 0)
		return false;
	if (!prompt_for_existing_file_compat(fmt::format("Load NPC (replacing '{}' #{}) (.znpc)", guy_string[selected_val], selected_val).c_str(), "znpc", NULL, datapath, false))
		return false;

	PACKFILE* f = pack_fopen_password(temppath, F_READ, "");
	if (!f) return false;
	if (!readonenpc(f, selected_val))
	{
		Z_error("Could not read from .znpc packfile %s\n", temppath);
		InfoDialog("ZNPC Error", "Could not load the specified npc.").show();
	}
	pack_fclose(f);
	refresh_dlg();
	mark_save_dirty();
	return true;
}

MidiListerDialog::MidiListerDialog(int index, bool selecting) :
	BasicListerDialog("Select MIDI", "mididata", index, selecting)
{
	alphabetized = get_config("alphabetized", false);
}

void MidiListerDialog::preinit()
{
	lister = GUI::ZCListData::midinames(true, false);
	if(selecting)
		frozen_start = 1; // lock '(None)'
	else
	{
		lister.removeInd(0); // remove '(None)'
		resort();
		if(selected_val < 0)
			selected_val = lister.getValue(0);
	}
	selected_val = vbound(selected_val, (selecting ? 0 : 1), MAXCUSTOMMIDIS);
}

void MidiListerDialog::postinit()
{
	size_t len = 36;
	for (uint q = 0; q < MAXCUSTOMMIDIS; ++q)
	{
		size_t tlen = text_length(GUI_DEF_FONT, customtunes[q].song_title.c_str());
		if (tlen > len)
			len = tlen;
	}
	Size sz = Size::pixels(len + 8);
	const Size max_sz = 20_em; // Midi titles can be long, want them to wrap instead of widen
	if (sz > max_sz)
		sz = max_sz;
	widgInfo->minWidth(sz);
	window->setHelp(get_info(selecting, false, false, false));
	widgInfo->capWidth(max_sz);
}
static size_t copied_midi_id = 0;
void MidiListerDialog::update(bool)
{
	if (unsigned(selected_val-1) < MAXCUSTOMMIDIS)
	{
		zctune const& midi = customtunes[selected_val-1]; //vals are 1-indexed, customtunes is 0-indexed
		widgInfo->setText(fmt::format(
			"Index: {}\nVolume: {}\nLoop: {}\nStart: {}\nLoop Start: {}\nLoop End: {}\nTitle: {}\nCopied: {}",
			selected_val, midi.volume,midi.loop?"On":"Off", midi.start, midi.loop_start, midi.loop_end, midi.song_title, copied_midi_id));
	}
	else widgInfo->setText("\n\n\n\n\n\n\n\n\n\n");
}
void MidiListerDialog::edit()
{
	call_midi_editor(selected_val-1);
}
void MidiListerDialog::rclick(int x, int y)
{
	size_t idx = size_t(selected_val-1);
	bool valid = idx < MAXCUSTOMMIDIS;
	NewMenu rcmenu {
		{ "Clear", [&](){clear();}, 0, valid ? 0 : MFL_DIS },
		{ "&Copy", [&](){copy();}, 0, valid ? 0 : MFL_DIS },
		{ "Paste", "&v", [&](){paste();}, 0, valid && copied_midi_id > 0 ? 0 : MFL_DIS },
		// { "&Save", [&](){save(); update();} },
		// { "&Load", [&](){load(); update();} },
	};
	rcmenu.pop(x, y);
}
bool MidiListerDialog::clear()
{
	size_t idx = size_t(selected_val-1);
	if (idx >= MAXCUSTOMMIDIS)
		return false;
	if (!alert_confirm(fmt::format("Clear MIDI {}", selected_val),
		fmt::format("Clear MIDI #{}, '{}'?", selected_val, customtunes[idx].song_title)))
		return false;
	customtunes[idx].reset();
	refresh_dlg();
	mark_save_dirty();
	return true;
}
void MidiListerDialog::copy()
{
	if (unsigned(selected_val-1) >= MAXCUSTOMMIDIS)
		return;
	copied_midi_id = selected_val;
	update();
}
bool MidiListerDialog::paste()
{
	if (selected_val == copied_midi_id)
		return false;
	if (unsigned(selected_val-1) >= MAXCUSTOMMIDIS || unsigned(copied_midi_id-1) >= MAXCUSTOMMIDIS)
		return false;
	customtunes[selected_val-1] = customtunes[copied_midi_id-1];
	refresh_dlg();
	mark_save_dirty();
	return true;
}


SFXListerDialog::SFXListerDialog(int index, bool selecting) :
	BasicListerDialog("Select SFX", "sfxdata", index, selecting)
{
	use_preview = false;
	alphabetized = get_config("alphabetized", true);
}

void SFXListerDialog::preinit()
{
	lister = GUI::ZCListData::sfxnames(true);
	if(selecting)
		frozen_start = 1; // lock '(None)'
	else
		lister.removeInd(0); // remove '(None)'
	if (quest_sounds.size() < MAX_SFX)
	{
		lister.add(fmt::format("<New SFX> ({:03})", quest_sounds.size() + 1), quest_sounds.size() + 1);
		frozen_end = 1;
	}
	else frozen_end = 0;
	if(selected_val < 0)
		selected_val = lister.getValue(0);
	selected_val = vbound(selected_val, 1, MAX_SFX);
}
static bool empty_sound(ZCSFX const& ref)
{
	return ref.is_invalid();
}
void SFXListerDialog::postinit()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	window->setHelp(get_info(selecting, false, false));
	widgInfo->overrideWidth(300_px);
	widgInfo->minHeight(10_em);
	bool has_empty = false;
	for (auto const& sound : quest_sounds)
	{
		if (empty_sound(sound)) // don't show button if nothing to clean up
		{
			has_empty = true;
			break;
		}
	}
	btnrow2 = Column(
		Columns<2>(
			Button(text = "Cleanup",
				fitParent = true,
				disabled = !has_empty,
				onPressFunc = [&]()
				{
					if (!alert_confirm("Clear empty sounds?",
						"This will clear any entries that have no playable sound set."
						"\nThis will offset any entries after empty entries; sfx may need to be updated."))
						return;
					
					delete_quest_sounds(empty_sound);
					refresh_dlg();
				}),
			del_btn = Button(text = "Delete",
				fitParent = true,
				onClick = message::CLEAR),
			up_btn = Button(text = "Up",
				fitParent = true,
				onPressFunc = [&]()
				{
					swap_quest_sounds(selected_val, selected_val-1);
					--selected_val;
					refresh_dlg();
				}),
			down_btn = Button(text = "Down",
				fitParent = true,
				onPressFunc = [&]()
				{
					swap_quest_sounds(selected_val, selected_val+1);
					++selected_val;
					refresh_dlg();
				})
		),
		Label(fitParent = true, maxwidth = 250_px,
			text = "WARNING: Re-ordering SFX will NOT update anything that uses them."
			"\nThis includes the re-ordering that happens when deleting SFX."
		)
	);
}

static int16_t copied_sfx_id = -1;
void SFXListerDialog::update(bool)
{
	string info;
	if (!selected_val)
		info = "[None]";
	else if (unsigned(selected_val-1) < quest_sounds.size())
	{
		auto const& sound = quest_sounds[selected_val-1]; // vals are 1-indexed
		info = sound.get_sound_info();
	}
	string copystr = fmt::format("Copied: {}", copied_sfx_id+1);
	widgInfo->setText(fmt::format("{}\n{}", info, copystr));
	up_btn->setDisabled(unsigned(selected_val - 2) >= quest_sounds.size() - 1);
	down_btn->setDisabled(unsigned(selected_val - 1) >= quest_sounds.size() - 1);
	del_btn->setDisabled(unsigned(selected_val - 1) >= quest_sounds.size());
}
void SFXListerDialog::edit()
{
	if (unsigned(selected_val-1) > quest_sounds.size())
		return;
	call_sfxdata_dialog(selected_val);
	refresh_dlg();
}
void SFXListerDialog::rclick(int x, int y)
{
	if (unsigned(selected_val-1) >= quest_sounds.size())
		return; // no rclick menu on the 'None' option
	bool oob = unsigned(selected_val-1) > quest_sounds.size();
	NewMenu rcmenu {
		{ "Clear", [&](){clear_nondelete();}, 0, oob ? MFL_DIS : 0 },
		{ "&Copy", [&](){copy();}, 0, oob ? MFL_DIS : 0 },
		{ "Paste", "&v", [&](){paste();}, 0, copied_sfx_id < 0 ? MFL_DIS : 0 },
		{ "Delete", [&](){clear();}, 0, oob ? MFL_DIS : 0 },
		// { "&Save", [&](){save(); update();} },
		// { "&Load", [&](){load(); update();} },
	};
	rcmenu.pop(x, y);
}
bool SFXListerDialog::clear_nondelete()
{
	if (unsigned(selected_val-1) >= quest_sounds.size())
		return false;
	auto& sound = quest_sounds[selected_val-1];
	if (!alert_confirm(fmt::format("Clear SFX {}", selected_val),
		fmt::format("Clear SFX #{}, '{}'?\nThis will reset it to blank.", selected_val, sound.sfx_name)))
		return false;
	sound.clear();
	refresh_dlg();
	return true;
}
bool SFXListerDialog::clear()
{
	if (unsigned(selected_val-1) >= quest_sounds.size())
		return false;
	auto& sound = quest_sounds[selected_val-1];
	if (!alert_confirm(fmt::format("Delete SFX '{}'?", sound.sfx_name),
		fmt::format("This will delete SFX #{}, '{}'."
		"\nThis will offset all entries after it; sfx of various things may need to be updated.",
		selected_val, sound.sfx_name)))
		return false;
	delete_quest_sounds(selected_val);
	refresh_dlg();
	return true;
}
void SFXListerDialog::copy()
{
	if (!selected_val) return; // skip none
	if (selected_val-1 >= quest_sounds.size()) return; // skip <New Music>
	copied_sfx_id = selected_val-1;
	update();
}
bool SFXListerDialog::paste()
{
	if (unsigned(copied_sfx_id) >= quest_sounds.size() || unsigned(selected_val-1) > quest_sounds.size())
		return false;
	if (selected_val-1 == quest_sounds.size())
		quest_sounds.emplace_back(quest_sounds[copied_sfx_id]);
	else quest_sounds[selected_val-1] = quest_sounds[copied_sfx_id];
	refresh_dlg();
	mark_save_dirty();
	return true;
}


DMapListerDialog::DMapListerDialog(int index, bool selecting) :
	BasicListerDialog("Select DMap", "dmap", index, selecting)
{
	use_preview = false;
	use_mappreview = true; //ugly hack but it works.
	alphabetized = get_config("alphabetized", true); //doesnt work???
}

void DMapListerDialog::preinit()
{
	lister = GUI::ZCListData::dmaps(true);
	if (selected_val < 0)
		selected_val = lister.getValue(0);
	selected_val = vbound(selected_val, 0, MAXDMAPS - 1);
}
void DMapListerDialog::postinit()
{
	window->setHelp(get_info(selecting, true));
}
static int16_t copied_dmap_id = -1;
void DMapListerDialog::update(bool)
{
	widgInfo->setText(fmt::format("\nMap: {}\nLevel: {}\n\nCopied: {}",
		DMaps[selected_val].map + 1, DMaps[selected_val].level, copied_dmap_id));
	mapPrev->setDMap(selected_val);
	mapPrev->setVisible(true);
}
void DMapListerDialog::edit()
{
	call_editdmap_dialog(selected_val);
}
void DMapListerDialog::copy()
{
	copied_dmap_id = selected_val;
	update();
}
bool DMapListerDialog::paste()
{
	if (copied_dmap_id < 0 || selected_val < 0)
		return false;
	if (copied_dmap_id == selected_val)
		return false;
	DMaps[selected_val] = DMaps[copied_dmap_id];
	mark_save_dirty();
	return true;
}


FFCListerDialog::FFCListerDialog(int index, bool selecting) :
	BasicListerDialog("Select FFC", "ffc", index, selecting)
{
	use_preview = true;
	use_alpha = false;
	alphabetized = false;
}

void FFCListerDialog::preinit()
{
	mapscr* curscr = Map.CurrScr();
	lister = GUI::ListData::numbers(false, 0, MAXFFCS, [&](int v)
		{
			if (curscr->ffcs.size() <= v)
				return fmt::format("Empty ({:03})", v+1);
			return fmt::format("({:03})", v+1);
		});
	if (selected_val < 0)
		selected_val = lister.getValue(0);
	selected_val = vbound(selected_val, 0, MAXDMAPS - 1);
}
void FFCListerDialog::postinit()
{
	window->setHelp(get_info(selecting, false, false));
}
void FFCListerDialog::update(bool startup)
{
	auto copied_ffc_id = Map.getCopyFFC();
	auto copied_ffc_mapscr = Map.CopyScr();
	auto copied_ffc_map = copied_ffc_mapscr >= 0 ? (copied_ffc_mapscr >> 8) : -1;
	auto copied_ffc_scr = copied_ffc_mapscr >= 0 ? (copied_ffc_mapscr & 0xFF) : -1;
	
	string copystr = "None";
	if (copied_ffc_id && copied_ffc_map > 0 && copied_ffc_scr > -1)
	{
		if (copied_ffc_map != Map.getCurrMap() || copied_ffc_scr != Map.getCurrScr())
			copystr = fmt::format("{} from M{},S0x{:02X}", copied_ffc_id, copied_ffc_map, copied_ffc_scr);
		else copystr = to_string(copied_ffc_id);
	}
	mapscr* curscr = Map.CurrScr();
	int max_tw = 1, max_th = 1;
	for (auto const& ffc : curscr->ffcs)
	{
		if (ffc.txsz > max_tw)
			max_tw = ffc.txsz;
		if (ffc.tysz > max_th)
			max_th = ffc.tysz;
	}
	if (!startup && (cache_max_tw != max_tw || cache_max_th != max_th))
		refresh_dlg();
	cache_max_tw = max_tw;
	cache_max_th = max_th;
	
	if (curscr->ffcs.size() > selected_val)
	{
		auto& ffc = curscr->ffcs[selected_val];
		widgInfo->setText(fmt::format("Combo: {}\nCSet: {}\nTSize: {}, {}\nEffSize: {}, {}"
			"\nPos: {}, {}\nSpeed: {}, {}"
			"\nAccel: {}, {}\nLinked: {}\nMove Delay: {}\n\nCopied: {}",
			ffc.data, ffc.cset, ffc.txsz, ffc.tysz, ffc.hit_width, ffc.hit_height,
			ffc.x.str_trim(), ffc.y.str_trim(), ffc.vx.str_trim(), ffc.vy.str_trim(),
			ffc.ax.str_trim(), ffc.ay.str_trim(), ffc.link, ffc.delay, copystr));
		
		newcombo const& cmb = combobuf[ffc.data];
		widgPrev->setTile(cmb.tile);
		widgPrev->setCSet2(cmb.csets);
		widgPrev->setFrames(cmb.frames);
		widgPrev->setSpeed(cmb.speed);
		widgPrev->setSkipX(cmb.skipanim);
		widgPrev->setSkipY(cmb.skipanimy);
		widgPrev->setFlip(cmb.flip);
		
		widgPrev->setCSet(ffc.cset);
		widgPrev->setTileW(ffc.txsz);
		widgPrev->setTileH(ffc.tysz);
		
		widgPrev->setDisabled(false);
		
		cache_tw = ffc.txsz;
		cache_th = ffc.tysz;
	}
	else
	{
		widgInfo->setText(fmt::format("\n\n\n\n\n\n\n\n\n\nCopied: {}", copystr));
		widgPrev->setTileW(1);
		widgPrev->setTileH(1);
		widgPrev->setDisabled(true);
		cache_tw = 1;
		cache_th = 1;
	}
	widgInfo->minWidth(Size::pixels(text_length(GUI_DEF_FONT, "EffSize: 8.8888,8.8888") + 5));
	widgPrev->setVisible(true);
	prev_holder->overrideWidth(Size::pixels(16 * 2 * cache_max_tw + 4));
	prev_holder->overrideHeight(Size::pixels(16 * 2 * cache_max_th + 4));
	widgPrev->overrideWidth(Size::pixels(16 * 2 * cache_tw + 4));
	widgPrev->overrideHeight(Size::pixels(16 * 2 * cache_th + 4));
	widgPrev->update_ref_size();
	widgPrev->resetAnim();
}
void FFCListerDialog::edit()
{
	call_ffc_dialog(selected_val, Map.CurrScr(), Map.getCurrScr());
}
void FFCListerDialog::rclick(int x, int y)
{
	mapscr* curscr = Map.CurrScr();
	NewMenu rcmenu {
		{ "Clear", [&](){clear();}, 0, selected_val >= curscr->ffcs.size() ? MFL_DIS : 0 },
		{ "&Copy", [&](){copy();}, 0, selected_val >= curscr->ffcs.size() ? MFL_DIS : 0 },
		{ "Paste", "&v", [&](){paste();}, 0, Map.getCopyFFC() < 0 ? MFL_DIS : 0 },
		// { "&Save", [&](){save(); update();} },
		// { "&Load", [&](){load(); update();} },
	};
	rcmenu.pop(x, y);
}
bool FFCListerDialog::clear()
{
	mapscr* curscr = Map.CurrScr();
	if (unsigned(selected_val) >= curscr->ffcs.size())
		return false;
	if (!alert_confirm(fmt::format("Clear FFC {}", selected_val),
		fmt::format("Clear FFC #{}?", selected_val)))
		return false;
	curscr->ffcs[selected_val].clear();
	update();
	return true;
}
void FFCListerDialog::copy()
{
	Map.CopyFFC(Map.getCurrScr(), selected_val);
	update();
}
bool FFCListerDialog::paste()
{
	if (Map.getCopyFFC() < 0 || selected_val < 0)
		return false;
	Map.DoSetFFCCommand(Map.getCurrMap(), Map.getCurrScr(), selected_val, Map.getCopyFFCData());
	update();
	return true;
}

SaveMenuListerDialog::SaveMenuListerDialog(int index, bool selecting) :
	BasicListerDialog("Select Save Menu", "save_menu", index, selecting)
{
	use_preview = false;
	alphabetized = get_config("alphabetized", false);
}

void SaveMenuListerDialog::preinit()
{
	lister = GUI::ZCListData::savemenus(true);
	if(selecting)
		frozen_start = 1; // lock '(None)'
	else
		lister.removeInd(0); // remove '(None)'
	resort();
	if (selected_val < 0)
		selected_val = lister.getValue(0);
	selected_val = vbound(selected_val, 0, NUM_SAVE_MENUS - 1);
}
void SaveMenuListerDialog::postinit()
{
	window->setHelp(get_info(selecting, false, false));
}
static int16_t copied_savemenu_id = -1;
void SaveMenuListerDialog::update(bool)
{
	static const string nl_str = string(MAX_SAVEMENU_OPTIONS-1, '\n');
	string info;
	if (!selected_val)
		info = fmt::format("[Default Menu]{}", nl_str);
	else if (unsigned(selected_val-1) < NUM_SAVE_MENUS)
	{
		SaveMenu const& sm = QMisc.save_menus[selected_val-1]; // vals are 1-indexed
		if (sm.is_empty())
			info = fmt::format("[Empty]{}", nl_str);
		else if (sm.options.empty())
			info = fmt::format("[Invalid - No Options]{}", nl_str);
		else
		{
			vector<string> opt_names;
			opt_names.reserve(MAX_SAVEMENU_OPTIONS);
			for (auto const& opt : sm.options)
				opt_names.push_back(opt.text);
			opt_names.resize(MAX_SAVEMENU_OPTIONS);
			info = fmt::format("{}", fmt::join(opt_names, "\n"));
		}
	}
	else info = nl_str;
	widgInfo->setText(fmt::format("{}\nCopied: {}", info, copied_savemenu_id+1));
}
void SaveMenuListerDialog::edit()
{
	call_editsavemenu_dialog(selected_val);
}
void SaveMenuListerDialog::rclick(int x, int y)
{
	if (unsigned(selected_val-1) < NUM_SAVE_MENUS)
		return; // no rclick menu on the 'Default' option
	SaveMenu& sm = QMisc.save_menus[selected_val-1];
	NewMenu rcmenu {
		{ "Clear", [&](){clear();}, 0, sm.is_empty() ? MFL_DIS : 0 },
		{ "&Copy", [&](){copy();}, 0, sm.is_empty() ? MFL_DIS : 0 },
		{ "Paste", "&v", [&](){paste();}, 0, copied_savemenu_id < 0 ? MFL_DIS : 0 },
		// { "&Save", [&](){save(); update();} },
		// { "&Load", [&](){load(); update();} },
	};
	rcmenu.pop(x, y);
}
bool SaveMenuListerDialog::clear()
{
	if (unsigned(selected_val-1) < NUM_SAVE_MENUS)
		return false;
	SaveMenu& sm = QMisc.save_menus[selected_val-1];
	if (!alert_confirm(fmt::format("Clear Save Menu {}", selected_val),
		fmt::format("Clear Save Menu #{}, '{}'?", selected_val, sm.name)))
		return false;
	sm.clear();
	refresh_dlg();
	return true;
}
void SaveMenuListerDialog::copy()
{
	if (!selected_val) return;
	copied_savemenu_id = selected_val-1;
	update();
}
bool SaveMenuListerDialog::paste()
{
	if (unsigned(copied_savemenu_id) >= NUM_SAVE_MENUS || unsigned(selected_val-1) >= NUM_SAVE_MENUS)
		return false;
	QMisc.save_menus[selected_val-1] = QMisc.save_menus[copied_savemenu_id];
	refresh_dlg();
	mark_save_dirty();
	return true;
}

void call_music_dialog(int index)
{
	MusicListerDialog(index).show();
}
MusicListerDialog::MusicListerDialog(int index, bool selecting) :
	BasicListerDialog("Select Music", "music", index, selecting)
{
	use_preview = false;
	alphabetized = get_config("alphabetized", false);
}

void MusicListerDialog::preinit()
{
	lister = GUI::ZCListData::music_names(true, false);
	if(selecting)
		frozen_start = 1; // lock '(None)'
	else
		lister.removeInd(0); // remove '(None)'
	if (quest_music.size() < MAX_QUEST_MUSIC)
	{
		lister.add(fmt::format("<New Music> ({:03})", quest_music.size() + 1), quest_music.size() + 1);
		frozen_end = 1;
	}
	else frozen_end = 0;
	resort();
	if (selected_val < 0)
		selected_val = lister.getValue(0);
	selected_val = vbound(selected_val, 0, MAX_QUEST_MUSIC - 1);
}
static bool empty_music(AdvancedMusic const& ref)
{
	if (!ref.enhanced.is_empty())
		return false;
	if (!ref.midi)
		return true;
	else if (ref.midi < BUILTIN_MIDI_MIN)
		return true;
	else if (ref.midi < 0)
		return false; // builtin midis
	else if (ref.midi > MAXCUSTOMMIDIS)
		return true;
	return !customtunes[ref.midi - 1].data;
}
void MusicListerDialog::postinit()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	window->setHelp(get_info(selecting, false, false));
	widgInfo->overrideWidth(300_px);
	widgInfo->minHeight(10_em);
	bool has_empty = false;
	for (auto const& amus : quest_music)
	{
		if (empty_music(amus)) // don't show button if nothing to clean up
		{
			has_empty = true;
			break;
		}
	}
	btnrow2 = Columns<2>(
		Button(text = "Cleanup",
			fitParent = true,
			disabled = !has_empty,
			onPressFunc = [&]()
			{
				if (!alert_confirm("Clear empty musics?",
					"This will clear any entries that have no playable music set."
					"\nAnything using these entries will be cleared to using '0', the (None) entry."))
					return;
				
				delete_quest_music(empty_music);
				refresh_dlg();
			}),
		del_btn = Button(text = "Delete",
			fitParent = true,
			onClick = message::CLEAR),
		up_btn = Button(text = "Up",
			fitParent = true,
			onPressFunc = [&]()
			{
				swap_quest_music(selected_val, selected_val-1);
				--selected_val;
				refresh_dlg();
			}),
		down_btn = Button(text = "Down",
			fitParent = true,
			onPressFunc = [&]()
			{
				swap_quest_music(selected_val, selected_val+1);
				++selected_val;
				refresh_dlg();
			})
	);
}
static int16_t copied_music_id = -1;
void MusicListerDialog::update(bool)
{
	string info;
	if (!selected_val)
		info = "[None]\n";
	else if (unsigned(selected_val-1) < quest_music.size())
	{
		auto const& amus = quest_music[selected_val-1]; // vals are 1-indexed
		if (amus.is_empty())
			info = "[Empty]\n";
		else
		{
			vector<string> lines;
			lines.reserve(5);
			lines.emplace_back(amus.name);
			
			if (amus.midi)
			{
				string midi_name = "(?ERROR?)";
				if (unsigned(amus.midi-1) < MAXCUSTOMMIDIS)
					midi_name = customtunes[amus.midi-1].song_title;
				else if (amus.midi < 0)
				{
					int m = amus.get_real_midi();
					if (unsigned(m) < ZC_MIDI_COUNT)
						midi_name = builtin_midi_names[m];
				}
				lines.emplace_back(fmt::format("MIDI: {} '{}'", amus.midi, midi_name));
			}
			else lines.emplace_back("MIDI: (None)");
			
			if (amus.enhanced.is_empty())
			{
				lines.emplace_back("[No Enhanced Music]");
			}
			else
			{
				lines.emplace_back(fmt::format("Enh: '{}' #{}", amus.enhanced.path, amus.enhanced.track));
				lines.emplace_back(fmt::format("Loop: {} - {}", amus.enhanced.loop_start, amus.enhanced.loop_end));
				lines.emplace_back(fmt::format("Crossfade: In {}, Out {}", amus.enhanced.xfade_in, amus.enhanced.xfade_out));
			}
			info = fmt::format("{}", fmt::join(lines, "\n"));
		}
	}
	string copystr = fmt::format("Copied: {}", copied_music_id+1);
	if (info.empty())
		widgInfo->setText(copystr);
	else widgInfo->setText(fmt::format("{}\n{}", info, copystr));
	up_btn->setDisabled(unsigned(selected_val - 2) >= quest_music.size() - 1);
	down_btn->setDisabled(unsigned(selected_val - 1) >= quest_music.size() - 1);
	del_btn->setDisabled(unsigned(selected_val - 1) >= quest_music.size());
}
void call_edit_music_dialog(size_t idx);
void MusicListerDialog::edit()
{
	if (unsigned(selected_val-1) > quest_music.size())
		return;
	call_edit_music_dialog(selected_val-1);
	refresh_dlg();
}
void MusicListerDialog::rclick(int x, int y)
{
	if (unsigned(selected_val-1) >= quest_music.size())
		return; // no rclick menu on the 'None' option
	bool oob = unsigned(selected_val-1) > quest_music.size();
	NewMenu rcmenu {
		{ "Clear", [&](){clear_nondelete();}, 0, (oob || quest_music[selected_val-1].is_empty()) ? MFL_DIS : 0 },
		{ "&Copy", [&](){copy();}, 0, oob ? MFL_DIS : 0 },
		{ "Paste", "&v", [&](){paste();}, 0, copied_music_id < 0 ? MFL_DIS : 0 },
		{ "Delete", [&](){clear();}, 0, oob ? MFL_DIS : 0 },
		// { "&Save", [&](){save(); update();} },
		// { "&Load", [&](){load(); update();} },
	};
	rcmenu.pop(x, y);
}
bool MusicListerDialog::clear_nondelete()
{
	if (unsigned(selected_val-1) >= quest_music.size())
		return false;
	auto& amus = quest_music[selected_val-1];
	if (!alert_confirm(fmt::format("Clear Music {}", selected_val),
		fmt::format("Clear Music #{}, '{}'?\nThis will reset it to default.", selected_val, amus.name)))
		return false;
	amus.clear();
	refresh_dlg();
	return true;
}
bool MusicListerDialog::clear()
{
	if (unsigned(selected_val-1) >= quest_music.size())
		return false;
	auto& amus = quest_music[selected_val-1];
	if (!alert_confirm(fmt::format("Delete Music '{}'?", amus.name),
		fmt::format("This will delete Music #{}, '{}'."
		"\nAnything using this entry will be cleared to using '0', the (None) entry.",
		selected_val, amus.name)))
		return false;
	delete_quest_music(selected_val);
	refresh_dlg();
	return true;
}
void MusicListerDialog::copy()
{
	if (!selected_val) return; // skip none
	if (selected_val-1 >= quest_music.size()) return; // skip <New Music>
	copied_music_id = selected_val-1;
	update();
}
bool MusicListerDialog::paste()
{
	if (unsigned(copied_music_id) >= quest_music.size() || unsigned(selected_val-1) > quest_music.size())
		return false;
	if (selected_val-1 == quest_music.size())
		quest_music.emplace_back(quest_music[copied_music_id]);
	else quest_music[selected_val-1] = quest_music[copied_music_id];
	quest_music[selected_val-1].id = selected_val;
	refresh_dlg();
	mark_save_dirty();
	return true;
}

