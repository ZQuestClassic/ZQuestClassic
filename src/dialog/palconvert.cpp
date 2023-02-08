#include "palconvert.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "zquest.h"
#include "zq_files.h"
#include "info.h"

int32_t onPalFix()
{
	PaletteConvertDialog().show();
	return D_O_K;
}

void setColorPalette(int32_t flags, int32_t lowpal, int32_t highpal);

//{MACROS
#define CSET_CHECK(str, index) \
Checkbox \
( \
	text = str, \
	leftPadding = 30_px, \
	checked = CSet_Checked & (1<<index), \
	onToggleFunc = [&](bool state) \
	{ \
		if (state) \
			CSet_Checked |= (1<<index); \
		else \
			CSet_Checked &= ~(1<<index); \
	} \
)
//}

PaletteConvertDialog::PaletteConvertDialog()
{}

static int32_t CSet_Checked = 0, lowpal = 0, highpal = 0x1FF;

std::shared_ptr<GUI::Widget> PaletteConvertDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Window(
		title = "Convert Palette",
		onClose = message::CANCEL,
		Columns<10>(
			Label
			(
				maxwidth = 23_em,
				textAlign = 1,
				colSpan = 2,
				bottomPadding = 8_px,
				text = "Please tick all the csets you wish to port to level palettes,"
					" as well as a range of level palettes to port to."
			),
			CSET_CHECK("CSet 1", 1),
			CSET_CHECK("CSet 2", 2),
			CSET_CHECK("CSet 3", 3),
			CSET_CHECK("CSet 4", 4),
			CSET_CHECK("CSet 5", 5),
			CSET_CHECK("CSet 7", 7),
			CSET_CHECK("CSet 8", 8),
			CSET_CHECK("CSet 9", 9),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				colSpan = 2,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			),
			Rows<2>
			(
				padding = 0_px,
				rowSpan = 8,
				Label(textAlign = 2, text = "From:"),
				TextField
				(
					fitParent = true,
					minwidth = 8_em,
					type = GUI::TextField::type::SWAP_SSHORT,
					low=0, high=0x1FF,
					val=lowpal,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
					{
						lowpal = val;
					}
				),
				Label(textAlign = 2, text = "To:"),
				TextField
				(
					fitParent = true,
					minwidth = 8_em,
					type = GUI::TextField::type::SWAP_SSHORT,
					low=0, high=0x1FF,
					val=highpal,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
					{
						highpal = val;
					}
				)
			)
		)
	);
}

bool PaletteConvertDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		//Exiting messages
		case message::OK:
			if (lowpal > highpal) zc_swap(lowpal, highpal);
			setColorPalette(CSet_Checked, lowpal, highpal);
			return true;
		case message::CANCEL:
		default:
			return true;
	}
}
