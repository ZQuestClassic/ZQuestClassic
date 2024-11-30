#include "edit_region.h"
#include "base/process_management.h"
#include "zq/zquest.h"
#include <gui/builder.h>
#include <fmt/format.h>
#include <base/qrs.h>

void call_edit_region_dialog(int32_t slot)
{
	EditRegionDialog(slot).show();
}

EditRegionDialog::EditRegionDialog(int32_t slot) :
	mapslot(slot), the_regions_data(&Regions[slot]), local_regions_data(Regions[slot])
{
}

#define BTN_REGIONIDX(index, indexstr) \
region_checks[index] = Checkbox(checked = region_grid->getCurrentRegionIndex() == index, \
	text = indexstr, maxheight = 16_px, \
	onToggleFunc = [&](bool state) \
	{ \
		region_grid->setCurrentRegionIndex(index); \
		refreshRegionGrid(); \
	}) \

std::shared_ptr<GUI::Widget> EditRegionDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	char titlebuf[256];
	sprintf(titlebuf, "Edit Region (%d)", mapslot);
	window = Window(
		title = titlebuf,
		onClose = message::CANCEL,
		use_vsync = true,
		Column(
			Frame(
				DMapRegionGrid(
					focused = true,
					regionMapPtr = &Map,
					localRegionsData = &local_regions_data,
					onUpdate = [&]()
					{
						refreshRegionGrid();
					})
			),
			Frame(title = "Edit Region:",
				Row(
					BTN_REGIONIDX(0, "0 (no scrolling)"),
					BTN_REGIONIDX(1, "1"),
					BTN_REGIONIDX(2, "2"),
					BTN_REGIONIDX(3, "3"),
					BTN_REGIONIDX(4, "4"),
					BTN_REGIONIDX(5, "5"),
					BTN_REGIONIDX(6, "6"),
					BTN_REGIONIDX(7, "7"),
					BTN_REGIONIDX(8, "8"),
					BTN_REGIONIDX(9, "9")
				)
			),
			Row(
				Button(text = "Z3 User Guide",
					onPressFunc = [&]()
					{
						launch_file("docs/z3_user_guide.md");
					}
				)
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

void EditRegionDialog::refreshRegionGrid()
{
	int32_t idx = region_grid->getCurrentRegionIndex();
	for (int32_t i = 0; i < 10; ++i)
	{
		region_checks[i]->setChecked(i == idx);
	}
}

bool EditRegionDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::OK:
		*the_regions_data = local_regions_data;
		saved = false;
		[[fallthrough]];
	case message::CANCEL:
	default:
		return true;
	}
	return false;
}
