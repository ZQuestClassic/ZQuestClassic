#include "gameicons.h"
#include "gui/builder.h"
#include "gui/key.h"
#include "info.h"
#include "base/misctypes.h"
#include <fmt/format.h>
#include "zq/zq_cset.h"

extern miscQdata QMisc;

bool call_gameicons_dialog()
{
	GameIconDialog().show();
	return true;
}

GameIconDialog::GameIconDialog()
{
	memcpy(gameicons, QMisc.icons, sizeof(QMisc.icons));
}

std::shared_ptr<GUI::Widget> GameIconDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	window = Window(
		use_vsync = true,
		title = "Game Icons",
		info = "Edit the Game Icons for the file select screen."
			   "\nThese use sprite palettes 9,10,11,&12",
		onTick = [&]()
		{
			for (int q = 0; q < 4; ++q)
				tswatch[q]->setCSet(6 + q);
			return ONTICK_CONTINUE;
		},
		onClose = message::CANCEL,
		//shortcuts = {},
		Column(
			Columns<2>(
				Label(text = "Ring"),
				_d,
				Label(text = "0"),
				tswatch[0] = SelTileSwatch(
					tile = gameicons[0],
					cset = 6,
					showvals = 0,
					onSelectFunc = [&](int32_t t, int32_t, int32_t, int32_t)
					{
						gameicons[0] = t;
					}),
				Label(text = "1"),
				tswatch[1] = SelTileSwatch(
					tile = gameicons[1],
					cset = 7,
					showvals = 0,
					onSelectFunc = [&](int32_t t, int32_t, int32_t, int32_t)
					{
						gameicons[1] = t;
					}),
				Label(text = "2"),
				tswatch[2] = SelTileSwatch(
					tile = gameicons[2],
					cset = 8,
					showvals = 0,
					onSelectFunc = [&](int32_t t, int32_t, int32_t, int32_t)
					{
						gameicons[2] = t;
					}
				),
				Label(text = "3+"),
				tswatch[3] = SelTileSwatch(
					tile = gameicons[3],
					cset = 9,
					showvals = 0,
					onSelectFunc = [&](int32_t t, int32_t, int32_t, int32_t)
					{
						gameicons[3] = t;
					}
				)
			),
			Row(vAlign = 1.0, spacing = 2_em,
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

bool GameIconDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
		case message::OK:
			memcpy(QMisc.icons, gameicons, sizeof(QMisc.icons));
			[[fallthrough]];
		case message::CANCEL:
			return true;
	}
	return false;
}
