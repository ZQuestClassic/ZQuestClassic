#include "waterpit_convert.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
//#include "zq/zq_files.h"
#include "info.h"

int32_t onPitFix()
{
	PitConvertDialog().show();
	return D_O_K;
}

void setPitDamage(int32_t flags, int32_t lowpal, int32_t highpal, int32_t damage);

//{MACROS
#define TYPE_CHECK(str, index) \
Checkbox \
( \
	text = str, \
	leftPadding = 30_px, \
	hAlign = 0.0, \
	checked = Type_Checked & (1<<index), \
	onToggleFunc = [&](bool state) \
	{ \
		if (state) \
			Type_Checked |= (1<<index); \
		else \
			Type_Checked &= ~(1<<index); \
	} \
)
//}

PitConvertDialog::PitConvertDialog()
{}

static int32_t Type_Checked = 0, lowcombo = 0, highcombo = 65279, damage = 4;

std::shared_ptr<GUI::Widget> PitConvertDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Window(
		title = "Combo Damage Convert",
		onClose = message::CANCEL,
		Columns<9>(
			Label
			(
				maxwidth = 23_em,
				textAlign = 1,
				colSpan = 2,
				bottomPadding = 8_px,
				text = "Please tick the combo types you wish to set the damage of,"
					" as well as a range of combos to port to and the damage you wish to set."
			),
			TYPE_CHECK("Pitfall", 0),
			TYPE_CHECK("Liquid (Non-Lava)", 1),
			TYPE_CHECK("Liquid (Lava)", 2),
			DummyWidget(),
			DummyWidget(),
			TYPE_CHECK("Includes Non-Zero Damage Combos", 8),
			TYPE_CHECK("Includes Warp Pitfalls", 9),
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
				rowSpan = 6,
				Label(textAlign = 2, text = "Damage:"),
				TextField
				(
					fitParent = true,
					minwidth = 8_em,
					type = GUI::TextField::type::SWAP_SSHORT,
					low=0, high=65535,
					val=damage,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
					{
						damage = val;
					}
				),
				Label(textAlign = 2, text = "From:"),
				TextField
				(
					fitParent = true,
					minwidth = 8_em,
					type = GUI::TextField::type::SWAP_SSHORT,
					low=0, high=65279,
					val=lowcombo,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
					{
						lowcombo = val;
					}
				),
				Label(textAlign = 2, text = "To:"),
				TextField
				(
					fitParent = true,
					minwidth = 8_em,
					type = GUI::TextField::type::SWAP_SSHORT,
					low=0, high=65279,
					val=highcombo,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
					{
						highcombo = val;
					}
				)
			)
		)
	);
}

bool PitConvertDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		//Exiting messages
		case message::OK:
			if (lowcombo > highcombo) zc_swap(lowcombo, highcombo);
			setPitDamage(Type_Checked, lowcombo, highcombo, damage);
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}
