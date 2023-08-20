#include "misc_sfx.h"
#include "info.h"
#include "alert.h"
#include <gui/builder.h>
#include "jwin.h"
#include "zq/zquest.h"
#include "base/zsys.h"
#include "gui/use_size.h"
#include "zc_list_data.h"

static const GUI::ListData miscSFXList
{
	{ "Grass Cut:", sfxBUSHGRASS, "Played when bush/grass/etc is cut, and does not have a custom SFX set." },
	{ "'Switch With Player':", sfxSWITCHED, "Played when an enemy's 'Switch w/ Player' defense is triggered, or a switchhook with 0 for 'Switch Sound' switches." },
	{ "Low Health Beep:", sfxLOWHEART, "Played when the player's health is low." },
	{ "Error:", sfxERROR, "Played when something errors (Ex. Using an item without enough magic)" },
	{ "Player Hurt:", sfxHURTPLAYER, "Plays when the player is hurt." },
	{ "Hammer Peg Pound:", sfxHAMMERPOUND, "Plays when a hammer peg is pounded." },
	{ "Subscreen Item Select:", sfxSUBSCR_ITEM_ASSIGN, "Plays when an item is assigned to a button in the subscreen." },
	{ "Subscreen Cursor Move:", sfxSUBSCR_CURSOR_MOVE, "Plays when the subscreen cursor moves." },
	{ "Refill:", sfxREFILL, "Plays when a counter is being refilled." },
	{ "Rupee Drain:", sfxDRAIN, "Plays when rupees are being drained." }
};

MiscSFXDialog::MiscSFXDialog(byte* vals, size_t vals_per_tab, std::function<void(int32_t*)> setVals):
	setVals(setVals), vals_per_tab(vals_per_tab), sfx_list(GUI::ZCListData::sfxnames())
{
	for(auto q = 0; q < sfxMAX; ++q)
	{
		local_sfx[q] = vals[q];
	}
}

std::shared_ptr<GUI::Widget> MiscSFXDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	return Window(
		title = "Misc Sounds",
		onClose = message::CANCEL,
		Column(
			maxwidth = Size::pixels(zq_screen_w),
			DDPanel(
				padding = 3_px,
				values = local_sfx,
				count = vals_per_tab,
				ddlist = sfx_list,
				data = miscSFXList
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
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
}

bool MiscSFXDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			setVals(local_sfx);
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}
