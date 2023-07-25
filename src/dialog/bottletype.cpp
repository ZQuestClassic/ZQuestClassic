#include "bottletype.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "base/misctypes.h"

extern bool saved;

void call_bottle_dlg(int32_t index)
{
	if(index < 0 || index > 63) return;
	BottleTypeDialog(index).show();
}

BottleTypeDialog::BottleTypeDialog(int32_t index):
	index(index), sourceBottle(QMisc.bottle_types[index]),
	tempBottle(QMisc.bottle_types[index]),
	list_counters(GUI::ZCListData::counters(true))
{}

#define NUM_FIELD(member,_min,_max) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = tempBottle.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		tempBottle.member = val; \
	})

std::shared_ptr<GUI::Widget> BottleTypeDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	char titlebuf[256];
	sprintf(titlebuf, "Bottle Type (%d): %s", index+1, tempBottle.name);
	window = Window(
		title = titlebuf,
		minwidth = 30_em,
		info = "Bottle Types to be used by the Bottle itemclass. Bottles can be filled"
			" with various things, configurable here.\nThese can be set to restore up"
			" to 3 counters, either by a value or a percentage; and also set what will"
			" remain in the bottle after this type is consumed (ex. multi-use potions).",
		onClose = message::CANCEL,
		Column(
			Row(
				Label(text = "Name:", hAlign = 0.0),
				TextField(
					maxwidth = 15_em,
					maxLength = 31,
					text = tempBottle.name,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
					{
						std::string name(str);
						strncpy(tempBottle.name, name.c_str(), 31);
						char buf[256];
						sprintf(buf, "Bottle Type (%d): %s", index+1, name.c_str());
						window->setTitle(buf);
					}
				)
			),
			Rows<3>(
				Label(text = "Refill Counters", colSpan = 3, textAlign = 1),
				//{
				DropDownList(data = list_counters,
					fitParent = true,
					selectedValue = tempBottle.counter[0],
					onSelectFunc = [&](int32_t val)
					{
						tempBottle.counter[0] = val;
					}
				),
				NUM_FIELD(amount[0],0,65535),
				Checkbox(
					text = "%", hAlign = 0.0,
					checked = tempBottle.flags & BTFLAG_PERC0,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(tempBottle.flags,BTFLAG_PERC0,state);
					}
				),
				//}
				//{
				DropDownList(data = list_counters,
					fitParent = true,
					selectedValue = tempBottle.counter[1],
					onSelectFunc = [&](int32_t val)
					{
						tempBottle.counter[1] = val;
					}
				),
				NUM_FIELD(amount[1],0,65535),
				Checkbox(
					text = "%", hAlign = 0.0,
					checked = tempBottle.flags & BTFLAG_PERC1,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(tempBottle.flags,BTFLAG_PERC1,state);
					}
				),
				//}
				//{
				DropDownList(data = list_counters,
					fitParent = true,
					selectedValue = tempBottle.counter[2],
					onSelectFunc = [&](int32_t val)
					{
						tempBottle.counter[2] = val;
					}
				),
				NUM_FIELD(amount[2],0,65535),
				Checkbox(
					text = "%", hAlign = 0.0,
					checked = tempBottle.flags & BTFLAG_PERC2,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(tempBottle.flags,BTFLAG_PERC2,state);
					}
				),
				//}
				Label(text = "Attributes", colSpan = 3),
				Label(text = "When used, becomes type:", hAlign = 0.0),
				NUM_FIELD(next_type, 0, 64), DummyWidget(),
				Checkbox(
					text = "Automatically activate upon death",
					colSpan = 3, hAlign = 0.0,
					checked = tempBottle.flags & BTFLAG_AUTOONDEATH,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(tempBottle.flags,BTFLAG_AUTOONDEATH,state);
					}
				),
				Checkbox(
					text = "Allow use when counters full",
					colSpan = 3, hAlign = 0.0,
					checked = tempBottle.flags & BTFLAG_ALLOWIFFULL,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(tempBottle.flags,BTFLAG_ALLOWIFFULL,state);
					}
				),
				Checkbox(
					text = "Cure Sword Jinx",
					colSpan = 3, hAlign = 0.0,
					checked = tempBottle.flags & BTFLAG_CURESWJINX,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(tempBottle.flags,BTFLAG_CURESWJINX,state);
					}
				),
				Checkbox(
					text = "Cure Item Jinx",
					colSpan = 3, hAlign = 0.0,
					checked = tempBottle.flags & BTFLAG_CUREITJINX,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(tempBottle.flags,BTFLAG_CUREITJINX,state);
					}
				)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
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

bool BottleTypeDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			memcpy(&sourceBottle, &tempBottle, sizeof(tempBottle));
			saved = false;
			return true;

		case message::CANCEL:
		default:
			return true;
	}
}
