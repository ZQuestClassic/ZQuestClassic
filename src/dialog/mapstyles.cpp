#include "mapstyles.h"
#include "gui/key.h"
#include "info.h"
#include "alert.h"
#include "tiles.h"
#include "gui/builder.h"
#include "base/misctypes.h"

extern miscQdata QMisc;

bool call_mapstyles_dialog()
{
	MapStylesDialog().show();
	return true;
}

MapStylesDialog::MapStylesDialog() :
	local_mapstyles(QMisc.colors)
{}

std::shared_ptr<GUI::Widget> MapStylesDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	window = Window(
		use_vsync = true,
		title = "Map Styles",
		onClose = message::CANCEL,
		Column(
			Row(
				Column(vAlign=0.0,
					Label(text = "", bottomPadding = 0_px),
					Label(text = "Frame"),
					SelTileSwatch(
						tile = local_mapstyles.blueframe_tile,
						cset = local_mapstyles.blueframe_cset,
						showFlip = false,
						showvals = false,
						tilewid = 2,
						tilehei = 2,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
						{
							local_mapstyles.blueframe_tile = t;
							local_mapstyles.blueframe_cset = c;
						}
					),
					Label(text = "Heart", bottomPadding = 0_px, topPadding = 16_px),
					Label(text = "Container", bottomPadding = 0_px),
					Label(text = "Piece"),
					SelTileSwatch(
						tile = local_mapstyles.HCpieces_tile,
						cset = local_mapstyles.HCpieces_cset,
						showFlip = false,
						showvals = false,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
						{
							local_mapstyles.HCpieces_tile = t;
							local_mapstyles.HCpieces_cset = c;
						}
					)
				),
				Column(vAlign = 0.0,
					Label(text = "McGuffin", bottomPadding = 0_px),
					Label(text = "Fragment"),
					SelTileSwatch(
						tile = local_mapstyles.triforce_tile,
						cset = local_mapstyles.triforce_cset,
						showFlip = false,
						showvals = false,
						tilewid = 2,
						tilehei = 3,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
						{
							local_mapstyles.triforce_tile = t;
							local_mapstyles.triforce_cset = c;
						}
					)
				),
				Column(vAlign = 0.0,
					Label(text = "McGuffin Frame", bottomPadding = 0_px),
					Label(text = "(Normal or Double Sized)"),
					SelTileSwatch(
						tile = local_mapstyles.triframe_tile,
						cset = local_mapstyles.triframe_cset,
						showFlip = false,
						showvals = false,
						tilewid = 7,
						tilehei = 7,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
						{
							local_mapstyles.triframe_tile = t;
							local_mapstyles.triframe_cset = c;
						}
					)
				),
				Column(vAlign = 0.0,
					Label(text = "", bottomPadding = 0_px),
					Label(text = "Overworld Map"),
					SelTileSwatch(
						tile = local_mapstyles.overworld_map_tile,
						cset = local_mapstyles.overworld_map_cset,
						showFlip = false,
						showvals = false,
						tilewid = 5,
						tilehei = 3,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
						{
							local_mapstyles.overworld_map_tile = t;
							local_mapstyles.overworld_map_cset = c;
						}
					),
					Label(text = "Dungeon Map"),
					SelTileSwatch(
						tile = local_mapstyles.dungeon_map_tile,
						cset = local_mapstyles.dungeon_map_cset,
						showFlip = false,
						showvals = false,
						tilewid = 5,
						tilehei = 3,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
						{
							local_mapstyles.dungeon_map_tile = t;
							local_mapstyles.dungeon_map_cset = c;
						}
					)
				)
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
	return window;
}

bool MapStylesDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::OK: //Misc Colors and Map Styles are combined just change what is changed.
		QMisc.colors.triforce_cset = local_mapstyles.triforce_cset;
		QMisc.colors.triframe_cset = local_mapstyles.triframe_cset;;
		QMisc.colors.overworld_map_cset = local_mapstyles.overworld_map_cset;
		QMisc.colors.dungeon_map_cset = local_mapstyles.dungeon_map_cset;
		QMisc.colors.blueframe_cset = local_mapstyles.blueframe_cset;
		QMisc.colors.triforce_tile = local_mapstyles.triforce_tile;
		QMisc.colors.triframe_tile = local_mapstyles.triframe_tile;
		QMisc.colors.overworld_map_tile = local_mapstyles.overworld_map_tile;
		QMisc.colors.dungeon_map_tile = local_mapstyles.dungeon_map_tile;
		QMisc.colors.blueframe_tile = local_mapstyles.blueframe_tile;
		QMisc.colors.HCpieces_tile = local_mapstyles.HCpieces_tile;
		QMisc.colors.HCpieces_cset = local_mapstyles.HCpieces_cset;
		[[fallthrough]];
	case message::CANCEL:
		return true;
	}
	return false;
}