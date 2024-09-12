#include "enemypattern.h"
#include "gui/key.h"
#include "info.h"
#include "base/zsys.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "zq/zq_class.h"

extern bool saved;

void call_enemypattern_dialog()
{
	EnemyPatternDialog(Map.CurrScr()).show();
}

EnemyPatternDialog::EnemyPatternDialog(mapscr* scr) :
	thescr(scr),
	last_pattern(int32_t(scr->pattern))
{}

std::shared_ptr<GUI::Widget> EnemyPatternDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	window = Window(
		title = "Select Pattern",
		info = "These will change how the screen enemies will spawn.",
		shortcuts = {
			K = message::OK
		},
		onClose = message::CANCEL,
		Column(hAlign=1.0,
			Label(text="Classic: Pseudorandom locations near"
				"\nthe middle of the screen."),
			Label(text="Random: Any available location"
				"\nat a sufficient distance from the player."),
			scr_pattern = List(fitParent = true,
				selectedIndex = last_pattern,
				data = GUI::ZCListData::patterntypes(),
				onSelectFunc = [&](int32_t val)
				{
					last_pattern = val;
				}),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "O&K",
					minwidth = 90_px,
					onClick = message::OK
				),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL
				)
			)
		)
	);

	return window;
}

bool EnemyPatternDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::OK:
		thescr->pattern = byte(last_pattern);
		saved = false;
		[[fallthrough]];
	case message::CANCEL:
	default:
		return true;
	}
}
