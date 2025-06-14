#include "warpring_editor.h"
#include "info.h"
#include "alert.h"
#include <gui/builder.h>
#include "base/zsys.h"
#include "zc_list_data.h"
#include "zinfo.h"
#include <fmt/format.h>
#include <base/misctypes.h>
#include <base/dmap.h>
#include "zq/render.h"

extern bool saved;

#define buttonwidth 6_em
#define labelwidth 64_px

//call functions
void call_warpring_editor(int32_t ring, byte index)
{
	Warpring_Editor_Dialog(ring, index).show();
}
void call_warpring_warpselector(int32_t ring)
{
	Warpring_WarpSelector_Dialog(ring).show();
}
void call_warpring_ringselector()
{
	Warpring_RingSelector_Dialog().show();
}

//Warpring Editor Dialog

Warpring_Editor_Dialog::Warpring_Editor_Dialog(int32_t ring, byte index) :
	ring(ring), index(index),
	list_dmaps(GUI::ZCListData::dmaps(true)),
	list_warps(GUI::ZCListData::warpreturns()),
	list_litems(GUI::ZCListData::levelitems())
{}

std::shared_ptr<GUI::Widget> Warpring_Editor_Dialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	byte scrPointer = QMisc.warp[ring].scr[index];
	w_dmapframe = DMapFrame(Dmap = QMisc.warp[ring].dmap[index], rowSpan=4);

	window = Window(
		title = fmt::format("Editing Ring {} Warp {}", ring, index),
		onClose = message::CANCEL,
		Column(
			Label(text = "DMap:"),
			w_dmaplist = DropDownList(
				data = list_dmaps,
				selectedValue = QMisc.warp[ring].dmap[index],
				onSelectFunc = [&](int32_t val) {
					selectedValue = val;
					w_dmapframe->setDMap(val);
				}
			),
			Columns<4>(
				Label(text = "Screen 0x"),
				Label(text = "Use Warp Return:"),
				Label(text = "Use Level:"),
				Label(text = "Use LItem:"),
				w_screen = TextField(

					val = scrPointer,
					low = 0, high = 0x87,
					type = GUI::TextField::type::INT_HEX,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t newval) {
						scrPointer = newval;
						val = newval;
					}
				),
				w_return = DropDownList(
					data = list_warps,
					selectedValue = QMisc.warp[ring].warpreturn[index],
					onSelectFunc = [&](int32_t val) {
						selectedValue = val;
					}
				),
				w_level = TextField(
					val = QMisc.warp[ring].level[index],
					low = -1, high = 511,
					type = GUI::TextField::type::INT_DECIMAL,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t newval) {
						val = newval;
					}
				),
				w_litem = DropDownList(
					data = list_litems,
					selectedValue = QMisc.warp[ring].litem[index],
					onSelectFunc = [&](int32_t val) {
						selectedValue = val;
					}
				),
				//MISSING GRID
				w_dmapframe
			),
			Row(
				Button(
					text = "OK",
					onClick = message::OK,
					width = buttonwidth
				),
				Button(
					text = "Cancel",
					onClick = message::CANCEL,
					width = buttonwidth
				)
			)
		)
	);

	return window;
}

bool Warpring_Editor_Dialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::OK:
		QMisc.warp[ring].dmap[index] = w_dmaplist->getSelectedValue();
		QMisc.warp[ring].scr[index] = w_screen->getVal();
		QMisc.warp[ring].warpreturn[index] = w_return->getSelectedValue();
		QMisc.warp[ring].level[index] = w_level->getVal();
		QMisc.warp[ring].litem[index] = w_litem->getSelectedValue();
		saved = false;
		[[fallthrough]];
	case message::CANCEL:
		return true;
	}
	return false;
}

//Warpring Warp Selector

Warpring_WarpSelector_Dialog::Warpring_WarpSelector_Dialog(int32_t ring) :
	ring(ring),
	list_warps(GUI::ZCListData::warpreturns()),
	list_litems(GUI::ZCListData::levelitems()),
	lister(GUI::ListData::numbers(false, 0, vbound(QMisc.warp[ring].size, 3, 256)))
{}

std::shared_ptr<GUI::Widget> Warpring_WarpSelector_Dialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	window = Window(
		title = fmt::format("Ring {}, Select a Warp", ring),
		onClose = message::DONE,
		Column(
			Row(
				Label(text = "Count:"),
					w_size = TextField(
						val = QMisc.warp[ring].size,
						low = 3, high = 256,
						type = GUI::TextField::type::INT_DECIMAL,
						onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t newval) {
							QMisc.warp[ring].size = newval;
							lister = GUI::ListData::numbers(false, 0, vbound(QMisc.warp[ring].size, 3, 256));
						}
					),
					w_warplist = List(
						rowSpan = 10,
						data = lister,
						selectedIndex = 0,
						onSelectionChanged = message::CHANGEWARP,
						onDClick = message::EDIT,
						isABC = false
					)
				),
				Column(
					w_dmapframe = DMapFrame(Dmap=QMisc.warp[ring].dmap[index],rowSpan=5),
					l_map = Label(textAlign = 0, text = fmt::format("Map: {}", DMaps[QMisc.warp[ring].dmap[index]].map), width = labelwidth),
					l_screen = Label(textAlign = 0, text = fmt::format("Screen: {0:#x}", QMisc.warp[ring].scr[index]), width = labelwidth),
					l_warpreturn = Label(textAlign = 0, text = fmt::format("Warp: {}", QMisc.warp[ring].warpreturn[index]), width = labelwidth),
					l_level = Label(textAlign = 0, text = fmt::format("Level: {}", QMisc.warp[ring].level[index]), width = labelwidth),
					l_litem = Label(textAlign = 0, text = fmt::format("LItem: {}", list_litems.getText(QMisc.warp[ring].litem[index])), width = labelwidth)
				),
			Row(
				Button(
					text = "OK",
					onClick = message::EDIT,
					width = buttonwidth
				),
				Button(
					text = "Cancel",
					onClick = message::DONE,
					width = buttonwidth
				)
			)
		)
	);

	return window;
}

bool Warpring_WarpSelector_Dialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::CHANGEWARP:
		index = w_warplist->getSelectedIndex();
		if (index >= QMisc.warp[ring].size)
			index = 0;
		w_warplist->setSelectedIndex(index);
		w_dmapframe->setDMap(QMisc.warp[ring].dmap[index]);
		l_map->setText(fmt::format("Map: {}", DMaps[QMisc.warp[ring].dmap[index]].map));
		l_screen->setText(fmt::format("Screen: {0:#x}", QMisc.warp[ring].scr[index]));
		l_warpreturn->setText(fmt::format("Warp: {}", QMisc.warp[ring].warpreturn[index]));
		l_level->setText(fmt::format("Level: {}", QMisc.warp[ring].level[index]));
		l_litem->setText(fmt::format("LItem: {}", list_litems.getText(QMisc.warp[ring].litem[index])));
		return false;
	case message::EDIT:
		call_warpring_editor(ring, w_warplist->getSelectedIndex());
		return false;
	case message::DONE:
		call_warpring_ringselector();
		return true;		
	}
	return false;
}

//Warpring Ring Selector

Warpring_RingSelector_Dialog::Warpring_RingSelector_Dialog() :
	lister(GUI::ListData::numbers(false,0,9))
{}

std::shared_ptr<GUI::Widget> Warpring_RingSelector_Dialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	window = Window(
		title = "Select a Ring",
		onClose = message::CANCEL,
		Column(
			Label(text = "Select a ring"),
			w_ringlist = List(
				data = lister,
				selectedIndex = 0,
				onSelectionChanged = message::CHANGERING,
				onDClick = message::EDIT,
				isABC = false
			),
			Row(
				Button(
					text = "Edit",
					onClick = message::EDIT,
					width = buttonwidth
				),
				Button(
					text = "Cancel",
					onClick = message::CANCEL,
					width = buttonwidth
				)
			)
		)
	);
	return window;
}

bool Warpring_RingSelector_Dialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::CHANGERING:
		ring = w_ringlist->getSelectedIndex();
		return false;
	case message::EDIT:
		call_warpring_warpselector(ring);
		return false;
	case message::CANCEL:
		return true;
	}
	return false;
}


