#include "info_lister.h"
#include <gui/builder.h>
#include <base/new_menu.h>
#include "base/files.h"
#include "itemeditor.h"
#include "spritedata.h"
#include "enemyeditor.h"
#include "midieditor.h"
#include "sfxdata.h"
#include "edit_dmap.h"
#include "info.h"
#include "zc_list_data.h"
#include "zq/zquest.h"
#include "zq/zq_misc.h"
#include "zq/zq_class.h"
#include "zq/zq_custom.h"
#include "base/qst.h"
#include <fmt/format.h>
#include <utility>
#include <sstream>

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
	
	preinit();
	
	std::shared_ptr<GUI::Grid> g;
	std::shared_ptr<GUI::Grid> btnrow;
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
		},
		Column(
			hPadding = 0_px,
			Row(vPadding = 0_px, fitParent = true,
				Checkbox(text = "Alphabetized",
					hAlign = 0.0,
					checked = alphabetized,
					onToggleFunc = [&](bool state)
					{
						alphabetized = state;
						set_config("alphabetized", state);
						resort();
					})
			),
			g = Columns<2>(
				widgList = List(data = lister, isABC = true,
					selectedValue = selected_val,
					rowSpan = 2, fitParent = true,
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
			),
			btnrow = Row(padding = 0_px)
		)
	);
	
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
		g->add(widgPrev = TileFrame(visible = false));
		g->add(widgInfo = Label(text = "", fitParent = true));
	}
	else if(use_mappreview)
	{
		_d,
		g->add(mapPrev = DMapFrame(visible = false));
		g->add(widgInfo = Label(text = "", fitParent = true));
	}
	else
	{
		g->add(widgInfo = Label(text = "", fitParent = true, rowSpan = 2));
	}
	
	resort();
	postinit();
	update();
	return window;
}

void BasicListerDialog::resort()
{
	if(alphabetized)
		lister.alphabetize(frozen_inds);
	else
		lister.valsort(frozen_inds);
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
	{
		rerun_dlg = true;
		return true;
	}
	return false;
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
		frozen_inds = 1; // lock '(None)'
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
void ItemListerDialog::update()
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
			item_string[selected_val], display_name, selected_val, itm.power, itm.fam_type,
			itm.family, itm.csets&0xF, itm.script, itm.collect_script, itm.sprite_script, itm.weap_data.script,
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
	widgPrev->overrideWidth(Size::pixels(48+4));
	widgPrev->overrideHeight(Size::pixels(48+4));
	widgPrev->resetAnim();
}
void ItemListerDialog::edit()
{
	call_item_editor(selected_val);
}
void ItemListerDialog::rclick(int x, int y)
{
	NewMenu rcmenu {
		{ "&Copy", [&](){copy(); update();} },
		{ "&Adv. Paste", [&](){adv_paste(); update();}, 0, copied_item_id < 0 },
		{ "Paste", "&v", [&](){paste(); update();}, 0, copied_item_id < 0 },
		{ "&Save", [&](){save(); update();} },
		{ "&Load", [&](){load(); update();} },
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
	saved = false;
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
	saved = false;
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
	saved = false;
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
		frozen_inds = 1; // lock '(None)'
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
void SpriteListerDialog::update()
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
	widgPrev->overrideWidth(Size::pixels(48+4));
	widgPrev->overrideHeight(Size::pixels(48+4));
	widgPrev->resetAnim();
}
void SpriteListerDialog::edit()
{
	call_sprite_dlg(selected_val);
}
void SpriteListerDialog::rclick(int x, int y)
{
	NewMenu rcmenu {
		{ "&Copy", [&](){copy(); update();} },
		{ "Paste", "&v", [&](){paste(); update();}, 0, copied_sprite_id < 0 },
		{ "&Save", [&](){save(); update();} },
		{ "&Load", [&](){load(); update();} },
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
	saved = false;
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
	saved = false;
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
void SubscrWidgListerDialog::update()
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
		frozen_inds = 1; // lock '(None)'
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
void EnemyListerDialog::update()
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
			enemy.e_tile, enemy.hp, enemy.dp, enemy.wdp, enemy.family, enemy.item_set, enemy.script, enemy.weap_data.script,
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
	widgPrev->overrideWidth(Size::pixels(48 + 4));
	widgPrev->overrideHeight(Size::pixels(48 + 4));
}
void EnemyListerDialog::edit()
{
	call_enemy_editor(selected_val);
}
void EnemyListerDialog::rclick(int x, int y)
{
	NewMenu rcmenu{
		{ "&Copy", [&]() {copy(); update(); } },
		//{ "&Adv. Paste", [&]() {adv_paste(); update(); }, 0, copied_enemy_id < 0 },
		{ "Paste", "&v", [&]() {paste(); update(); }, 0, copied_enemy_id < 0 },
		{ "&Save", [&]() {save(); update(); } },
		{ "&Load", [&]() {load(); update(); } },
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
	if (copied_enemy_id < 0 || selected_val < 0)
		return false;
	if (copied_enemy_id == selected_val)
		return false;
	guysbuf[selected_val] = guysbuf[copied_enemy_id];
	saved = false;
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
	saved = false;
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
		frozen_inds = 1; // lock '(None)'
	else
	{
		lister.removeInd(0); // remove '(None)'
		resort();
		if(selected_val < 0)
			selected_val = lister.getValue(0);
	}
	selected_val = vbound(selected_val, (selecting ? -1 : 0), MAXCUSTOMMIDIS - 1);
}

void MidiListerDialog::postinit()
{
	size_t len = 36;
	for (int q = 0; q < MAXCUSTOMMIDIS; ++q)
	{
		size_t tlen = text_length(GUI_DEF_FONT, customtunes[q].title);
		if (tlen > len)
			len = tlen;
	}
	widgInfo->minWidth(Size::pixels(len + 8));
	window->setHelp(get_info(selecting, false, false, false));
	widgInfo->capWidth(10_em); // Midi titles can be long, want them to wrap instead of widen
}
void MidiListerDialog::update()
{
	if (unsigned(selected_val) < MAXCUSTOMMIDIS)
	{
		zctune const& midi = customtunes[selected_val-1]; //vals are 1-indexed, customtunes is 0-indexed
		widgInfo->setText(fmt::format(
			"Index: {}\nVolume: {}\nLoop: {}\nStart: {}\nLoop Start: {}\nLoop End: {}\nTitle: {}",
			selected_val, midi.volume,midi.loop?"On":"Off", midi.start, midi.loop_start, midi.loop_end, midi.title));
	}
	else
	{
		widgInfo->setText(fmt::format(
			"\n\n\n\n\n\n\n\n\n"));
	}
}
void MidiListerDialog::edit()
{
	call_midi_editor(selected_val-1);
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
		frozen_inds = 1; // lock '(None)'
	else
	{
		lister.removeInd(0); // remove '(None)'
		resort();
		if(selected_val <= 0)
			selected_val = lister.getValue(0);
	}
	selected_val = vbound(selected_val, 1, sfxMAX - 1);
}

void SFXListerDialog::postinit()
{
	size_t len = 36;
	for (int q = 0; q < sfxMAX; ++q)
	{
		size_t tlen = text_length(GUI_DEF_FONT, sfx_string[q]);
		if (tlen > len)
			len = tlen;
	}
	window->setHelp(get_info(selecting, false, false, false));
}

void SFXListerDialog::edit()
{
	call_sfxdata_dialog(selected_val);
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
void DMapListerDialog::update()
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
	saved = false;
	return true;
}
