#include "layer_dialog.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"

void call_layer_dialog(int map, int screen)
{
	LayerDialog(map, screen).show();
}


LayerDialog::LayerDialog(int map, int screen):
	map(map), screen(screen), autolayer(0),
	dest_ref(TheMaps[map*MAPSCRS + screen]),
	local_ref(dest_ref)
{}

enum
{
	autolyr_blankscreens,
	autolyr_blanklayers,
	autolyr_any
};

#define LAYER_SETUP(lyr) \
Label(text = to_string(lyr)), \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = 0, high = map_count, \
	val = local_ref.layermap[lyr-1], minwidth = 4_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_ref.layermap[lyr-1] = (byte)val; \
	}), \
TextField( \
	type = GUI::TextField::type::SWAP_ZSINT_NO_DEC, \
	low = 0, high = MAPSCRSNORMAL-1, \
	val = local_ref.layerscreen[lyr-1], swap_type = nswapHEX, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_ref.layerscreen[lyr-1] = (byte)val; \
	}), \
Checkbox(checked = local_ref.layeropacity[lyr-1] == 128, \
	onToggleFunc = [&](bool state) \
	{ \
		local_ref.layeropacity[lyr-1] = (state ? 128 : 255); \
	})

#define LAYER_AUTO(lyr) \
Button(text = autolayer_data.contains(lyr) ? "Auto*" : "Auto", \
	onPressFunc = [&]() \
	{ \
		autolayer = lyr; \
		refresh_dlg(); \
	})

std::shared_ptr<GUI::Widget> LayerDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	if (autolayer)
	{
		bool existed_previously = autolayer_data.contains(autolayer);
		if (existed_previously)
			temp_autolayer_data = autolayer_data[autolayer];
		else if (int prev_automap = map_infos[map].autolayers[autolayer-1])
			temp_autolayer_data = { prev_automap, 1 };
		else
			temp_autolayer_data = { local_ref.layermap[autolayer-1], 1 };
		std::shared_ptr<GUI::Grid> btnrow = Row(topPadding = 0.5_em, vAlign = 1.0, spacing = 2_em);
		auto update_rset = [&](size_t idx)
			{
				temp_autolayer_data.second = (int)idx;
			};
#define AUTOLYR_FLAG_RADIO(txt, fl) \
		Radio( \
			hAlign = 0.0, \
			text = txt, \
			indx = fl, \
			checked = temp_autolayer_data.second == fl, \
			onPressFunc = update_rset \
		)
		
		btnrow->add(Button(
				text = "OK",
				minwidth = 90_px,
				onClick = message::OK));
		btnrow->add(Button(
				text = existed_previously ? "Cancel Changes" : "Cancel",
				minwidth = 90_px,
				onClick = message::CANCEL));
		if (existed_previously)
			btnrow->add(Button(
				text = "Cancel Completely",
				minwidth = 90_px,
				onPressFunc = [&]()
				{
					autolayer_data.erase(autolayer);
					autolayer = 0;
					refresh_dlg();
				}));
		
		return Window(
			title = fmt::format("Autolayer {} for Map {}", autolayer, map+1),
			onClose = message::CANCEL,
			Column(
				Column(
					Row(
						Label(text = fmt::format("Map for layer {}:", autolayer)),
						TextField(
							type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
							low = 0, high = map_count,
							val = temp_autolayer_data.first, minwidth = 4_em,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								temp_autolayer_data.first = (byte)val;
							})
					),
					AUTOLYR_FLAG_RADIO("Only Blank Screens", autolyr_blankscreens),
					AUTOLYR_FLAG_RADIO("Only Blank Layers", autolyr_blanklayers),
					AUTOLYR_FLAG_RADIO("Overwrite Layers", autolyr_any)
				),
				btnrow
			)
		);
	}
	else
	{
		auto const& md = TheMaps[map*MAPSCRS+screen];
		
		return Window(
			title = fmt::format("Layer Data for Map {} Screen 0x{:02X}", map+1, screen),
			onClose = message::CANCEL,
			Column(
				Columns<6>(
					DummyWidget(),
					Label(text = "Map:", hAlign = 1.0),
					Label(text = "Screen:", hAlign = 1.0),
					Label(text = "Transparent:", hAlign = 1.0),
					Label(text = "Toggle 'Is Background':", hAlign = 1.0),
					Label(text = "Set for whole map:", hAlign = 1.0),
					
					LAYER_SETUP(1),
					DummyWidget(),
					LAYER_AUTO(1),
					
					LAYER_SETUP(2),
					Checkbox(checked = local_ref.flags7 & fLAYER2BG,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_ref.flags7, fLAYER2BG, state);
						}),
					LAYER_AUTO(2),
					
					LAYER_SETUP(3),
					Checkbox(checked = local_ref.flags7 & fLAYER3BG,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_ref.flags7, fLAYER3BG, state);
						}),
					LAYER_AUTO(3),
					
					LAYER_SETUP(4),
					DummyWidget(),
					LAYER_AUTO(4),
					
					LAYER_SETUP(5),
					DummyWidget(),
					LAYER_AUTO(5),
					
					LAYER_SETUP(6),
					DummyWidget(),
					LAYER_AUTO(6)
				),
				Label(text = autolayer_data.empty() ? "" : fmt::format("*{} autolayer operation{} queued", autolayer_data.size(), autolayer_data.size() == 1 ? "" : "s")),
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
}

bool LayerDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			if (autolayer)
			{
				autolayer_data[autolayer] = temp_autolayer_data;
				if (temp_autolayer_data.second != autolyr_blanklayers || !dest_ref.layermap[autolayer-1])
					local_ref.layermap[autolayer-1] = temp_autolayer_data.first;
				autolayer = 0;
				rerun_dlg = true;
			}
			else
			{
				for (auto [lyr, data] : autolayer_data)
				{
					auto [new_map, flag] = data;
					map_infos[map].autolayers[lyr-1] = new_map;
					for (int scr = 0; scr < 128; ++scr)
					{
						auto& dest_scr = TheMaps[map*MAPSCRS + scr];
						if (flag == autolyr_blankscreens && (dest_scr.valid & mVALID))
							continue;
						if (flag == autolyr_blanklayers && dest_scr.layermap[lyr-1])
							continue;
						dest_scr.layermap[lyr-1] = new_map;
						dest_scr.layerscreen[lyr-1] = new_map ? scr : 0;
					}
				}
				dest_ref = local_ref;
			}
			return true;
		case message::CANCEL:
			if (autolayer)
			{
				autolayer = 0;
				rerun_dlg = true;
			}
			return true;
	}
	return false;
}
