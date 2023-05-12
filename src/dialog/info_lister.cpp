#include "info_lister.h"
#include <gui/builder.h>
#include "itemeditor.h"
#include "info.h"
#include "zc_list_data.h"
#include "zquest.h"
#include "zq_misc.h"
#include "zq_class.h"
#include "zq_custom.h"
#include "qst.h"
#include <fmt/format.h>
#include <utility>

extern char *item_string[];

static const std::string def_info_nosel =
	"Ctrl+C/Ctrl+V - Copy/Paste"
	"\nCtrl+S/Ctrl+L - Save/Load"
	"\nDouble-Click - Edit";
static const std::string def_info_sel =
	"Ctrl+C/Ctrl+V - Copy/Paste"
	"\nCtrl+S/Ctrl+L - Save/Load"
	"\nDouble-Click - Confirm";

int lister_index = -1;

std::shared_ptr<GUI::Widget> BasicListerDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	lister_index = start_ind;
	
	preinit();
	
	std::shared_ptr<GUI::Widget> okbtn;
	if(selecting)
		okbtn = Button(
			text = "OK",
			topPadding = 0.5_em,
			minwidth = 90_px,
			onClick = message::OK);
	else okbtn = DummyWidget();
	
	window = Window(
		title = titleTxt,
		onClose = message::EXIT,
		hPadding = 0_px,
		use_vsync = true,
		shortcuts={
			Ctrl+C=message::COPY,
			Ctrl+V=message::PASTE,
			Ctrl+S=message::SAVE,
			Ctrl+L=message::LOAD,
			Enter=message::CONFIRM,
		},
		Column(
			hPadding = 0_px, 
			Columns<2>(
				widgList = List(data = lister, isABC = true,
					selectedValue = selected_index,
					rowSpan = 2, fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						if(selected_index == val)
							return;
						selected_index = val;
						update();
					},
					onRClickFunc = [&](int32_t val, int32_t x, int32_t y)
					{
						if(selected_index != val)
						{
							selected_index = val;
							update();
						}
						forceDraw();
						rclick(x,y);
					},
					onDClick = message::CONFIRM),
				widgPrev = TileFrame(visible = false),
				widgInfo = Label(text = "", fitParent = true)
			),
			Row(padding = 0_px,
				okbtn,
				Button(
					text = "Edit",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::EDIT,
					focused = true),
				Button(
					text = selecting?"Cancel":"Done",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::EXIT)
			)
		)
	);
	postinit();
	update();
	return window;
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
			lister_index = selected_index;
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

static MENU cpsl_rclick_menu[] =
{
	{ "&Copy",    NULL, NULL, 0, NULL },
	{ "Paste &v", NULL, NULL, 0, NULL },
	{ "&Save",    NULL, NULL, 0, NULL },
	{ "&Load",    NULL, NULL, 0, NULL },
	{ NULL,       NULL, NULL, 0, NULL }
};

ItemListerDialog::ItemListerDialog(int itemid, bool selecting):
	BasicListerDialog("Select Item",itemid,selecting)
{}
void ItemListerDialog::preinit()
{
	lister = GUI::ZCListData::items(true);
	if(!selecting)
	{
		lister.removeInd(0); //remove '(None)'
		if(selected_index < 0)
			selected_index = lister.getValue(0);
	}
	selected_index = vbound(selected_index, (selecting?-1:0), MAXITEMS-1);
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
	window->setHelp(selecting ? def_info_sel : def_info_nosel);
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
	if(unsigned(selected_index) < MAXITEMS)
	{
		itemdata const& itm = itemsbuf[selected_index];
		std::string display_name = itm.display_name[0]
			? itm.get_name(true)
			: "[No Display Name]";
		widgInfo->setText(fmt::format(
			"{}\n{}\n#{}\nPower: {}\nLevel: {}"
			"\nType: {}\nCSet: {}\nScripts:\nAction: {}\nPickup: {}\nSprite: {}\nWeapon: {}"
			"\n\nCopied:\n{}",
			item_string[selected_index], display_name, selected_index, itm.power, itm.fam_type,
			itm.family, itm.csets, itm.script, itm.collect_script, itm.sprite_script, itm.weaponscript,
			copied_name));
		widgPrev->setVisible(true);
		widgPrev->setDisabled(false);
		widgPrev->setTile(itm.tile);
		widgPrev->setCSet(itm.csets&0xF);
		widgPrev->setFrames(itm.frames);
		widgPrev->setSpeed(itm.speed);
		widgPrev->setDelay(itm.delay);
		widgPrev->setSkipX((itm.overrideFLAGS & itemdataOVERRIDE_TILEWIDTH)
			? itm.tilew-1 : 0);
		widgPrev->setSkipY((itm.overrideFLAGS & itemdataOVERRIDE_TILEHEIGHT)
			? itm.tileh-1 : 0);
		widgPrev->setDoSized(true);
	}
	else
	{
		widgInfo->setText(fmt::format(
			"\n\n\n\n"
			"\n\n\n\n\n\n\n"
			"\n\nCopied:\n{}",
			copied_name));
		widgPrev->setVisible(true);
		widgPrev->setDisabled(true);
		widgPrev->setTile(0);
		widgPrev->setCSet(0);
		widgPrev->setFrames(0);
		widgPrev->setSpeed(0);
		widgPrev->setDelay(0);
		widgPrev->setSkipX(0);
		widgPrev->setSkipY(0);
		widgPrev->setDoSized(true);
	}
}
void ItemListerDialog::edit()
{
	call_item_editor(selected_index);
}
void ItemListerDialog::rclick(int x, int y)
{
	SETFLAG(cpsl_rclick_menu[1].flags,D_DISABLED,copied_item_id<0);
	
	int ret=popup_menu(cpsl_rclick_menu, x, y);
	
	if(ret==0)
		copy();
	else if(ret==1)
		paste();
	else if(ret==2)
		save();
	else if(ret==3)
		load();
	if(ret > -1)
		update();
}
void ItemListerDialog::copy()
{
	copied_item_id = selected_index;
	update();
}
bool ItemListerDialog::paste()
{
	if(copied_item_id < 0 || selected_index < 0)
		return false;
	if(copied_item_id == selected_index)
		return false;
	itemsbuf[selected_index] = itemsbuf[copied_item_id];
	saved = false;
	return true;
}
int32_t readoneitem(PACKFILE *f, int32_t id);
int32_t writeoneitem(PACKFILE *f, int32_t id);
void ItemListerDialog::save()
{
	if(selected_index < 0)
		return;
	if(!getname(fmt::format("Save Item '{}' #{} (.zitem)",itemsbuf[selected_index].get_name(true),selected_index).c_str(),"zitem",NULL,datapath,false))
		return;
	
	PACKFILE *f=pack_fopen_password(temppath,F_WRITE,"");
	if(!f) return;
	if (!writeoneitem(f,selected_index))
	{
		Z_error("Could not write to .zitem packfile %s\n", temppath);
		InfoDialog("ZItem Error", "Could not save the specified item.").show();
	}
	pack_fclose(f);
}
bool ItemListerDialog::load()
{
	if(selected_index < 0)
		return false;
	if(!getname(fmt::format("Load Item (replacing '{}' #{}) (.zitem)",itemsbuf[selected_index].get_name(true),selected_index).c_str(),"zitem",NULL,datapath,false))
		return false;
	
	PACKFILE *f=pack_fopen_password(temppath,F_READ,"");
	if(!f) return false;
	if (!readoneitem(f,selected_index))
	{
		Z_error("Could not read from .zitem packfile %s\n", temppath);
		InfoDialog("ZItem Error", "Could not load the specified item.").show();
	}
	pack_fclose(f);
	saved = false;
	return true;
}

