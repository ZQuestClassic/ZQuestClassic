#include "edit_dmap.h"
#include <gui/builder.h>
#include "jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "zc_list_data.h"
#include <fmt/format.h>

static size_t editdmap_tab = 0;
void call_editdmap_dialog(int32_t slot)
{
	EditDMapDialog(slot).show();
}
void call_editdmap_dialog(size_t forceTab, int32_t slot)
{
	editdmap_tab = forceTab;
	call_editdmap_dialog(slot);
}

EditDMapDialog::EditDMapDialog(int32_t slot) :
	thedmap(&DMaps[slot]), local_dmap(DMaps[slot]), dmapslot(slot),
	list_maps(GUI::ListData::numbers(false, 1, map_count)),
	list_types(GUI::ZCListData::dmaptypes())
{}

bool sm_dmap(int dmaptype)
{
	switch (dmaptype & dmfTYPE)
	{
		case dmOVERW:
			return false;
		default:
			return true;
	}
}

std::shared_ptr<GUI::Widget> EditDMapDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	char titlebuf[256];
	sprintf(titlebuf, "Edit DMap (%d)", dmapslot);
	window = Window(
		title = titlebuf,
		onClose = message::CANCEL,
		Column(
			Row(hAlign = 0.0,
				Label(text = "Name:", hAlign = 0.0, colSpan = 2),
				TextField(
					fitParent = true,
					type = GUI::TextField::type::TEXT,
					maxLength = 20,
					text = local_dmap.name,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
					{
						std::string str;
						str.assign(text);
						strncpy(local_dmap.name, str.c_str(), 20);
						local_dmap.name[20] = 0;
					}
				)
			),
			TabPanel(
				ptr = &editdmap_tab,
				TabRef(name = "Mechanics", Column(
					Rows<6>(
						Label(text = "Map:"),
						DropDownList(data = list_maps,
							fitParent = true,
							selectedValue = local_dmap.map + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.map = val - 1;
								dmap_mmap->setCurMap(val);
							}),
						Label(text = "Type:"),
						DropDownList(data = list_types,
							fitParent = true,
							selectedValue = local_dmap.type & dmfTYPE,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.type = (local_dmap.type & ~dmfTYPE) | (val & dmfTYPE);
								dmap_mmap->setSmallDMap(sm_dmap(local_dmap.type));
							}),
						Label(text = "Level:"),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 511, val = local_dmap.level,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								local_dmap.level = val;
							}),
						dmap_mmap = DMapMinimap(
							colSpan = 3,
							curMap = local_dmap.map + 1,
							smallDMap = sm_dmap(local_dmap.type),
							offset = local_dmap.xoff
						),
						Label(
							colSpan = 3, 
							text = "Placeholder"
						)
					)
				)),
				TabRef(name = "Appearance", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Music", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Maps", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Flags", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Disable", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Scripts", Column(
					Row(
						Label(text = "Placeholder")
					)
				))
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					focused = true,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	return window;
}

bool EditDMapDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::REFR_INFO:
		break;
	case message::OK:
		*thedmap = local_dmap;
		saved = false;
		[[fallthrough]];
	case message::CANCEL:
	default:
		return true;
	}
	return false;
}
