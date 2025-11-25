#include "undercombo_dlg.h"
#include "common.h"
#include "alert.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "base/combo.h"

extern bool saved;
extern int32_t CSet;

void call_undercombo_dlg(int map, int screen)
{
	UnderComboDialog(map, screen).show();
}

static void get_screens(mapscr& s, mapscr* screens[7])
{
	screens[0] = &s;
	for (int layer = 1; layer < 7; ++layer)
	{
		screens[layer] = nullptr;
		if (s.layermap[layer-1] > 0)
			screens[layer] = Map.AbsoluteScr(s.layermap[layer-1], s.layerscreen[layer-1]);
	}
}

UnderComboDialog::UnderComboDialog(int map, int screen):
	window(), map(map), screen(screen), lyr_widgets(), preview_idx(-1),
	underc_idx(-1), overwrite_mode(false), whole_map(false)
{
	mapscr& base_screen = TheMaps[map * MAPSCRS + screen];
	get_screens(base_screen, cur_screens);
	for (int layer = 0; layer < 7; ++layer)
	{
		mapscr* m = cur_screens[layer];
		layer_enabled[layer] = m != nullptr;
		undercombos[layer] = m ? m->undercombo : 0;
		undercsets[layer] = m ? m->undercset : CSet;
	}
}

void UnderComboDialog::refresh_layer(int layer)
{
	newcombo const& cmb = combobuf[undercombos[layer]];
	auto cs = undercsets[layer];
	auto& vec = lyr_widgets[layer];
	
	if (vec.size() > preview_idx)
	{
		GUI::TileFrame& prev_widg = *static_cast<GUI::TileFrame*>(vec[preview_idx].get());
		prev_widg.setTile(cmb.tile);
		prev_widg.setCSet(cs);
		prev_widg.setCSet2(cmb.csets);
		prev_widg.setFrames(cmb.frames);
		prev_widg.setSpeed(cmb.speed);
		prev_widg.setSkipX(cmb.skipanim);
		prev_widg.setSkipY(cmb.skipanimy);
		prev_widg.setFlip(cmb.flip);
		prev_widg.setDisabled(!layer_enabled[layer]);
	}
	
	if (vec.size() > underc_idx)
	{
		GUI::SelComboSwatch& cmb_widg = *static_cast<GUI::SelComboSwatch*>(vec[underc_idx].get());
		cmb_widg.setDisabled(!layer_enabled[layer]);
	}
}

shared_ptr<GUI::Widget> UnderComboDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	shared_ptr<GUI::Grid> g = Column();
	
	string layer_info = fmt::format("Undercombos for layers >0 are just the undercombos for the"
		" layer screen that is currently assigned as that layer.{}",
			whole_map
				? "\nUnchecked layers will be ignored.."
				: "\nLayers that are not assigned a screen cannot be modified.");
	
	mapscr& base_screen = TheMaps[map * MAPSCRS + screen];
	
	int layer = 0;
	lyr_widgets.fill({});
	
	while (layer < 7)
	{
		if (layer > 0)
			g->add(HSeparator());
		shared_ptr<GUI::Grid> subg = GUI::Internal::makeRowsColumns(2, whole_map ? 2 : 3);
		subg->setHAlign(0.0);
		
		subg->add(Label(text = "Layer:", hAlign = 1.0));
		subg->add(INFOBTN(layer_info));
		if (!whole_map)
		{
			subg->add(Label(text = "Old Undercombo:", hAlign = 1.0));
			subg->add(INFOBTN("The previously set undercombo"));
		}
		subg->add(Label(text = "Undercombo:", hAlign = 1.0));
		subg->add(INFOBTN("The combo+cset set as the undercombo for the layer screen."
			"\nFirst box selects combo, second box shows animated preview of the combo."));
		
		for (int q = 0; q < 4 && layer < 7; ++layer)
		{
			if (!whole_map && !layer_enabled[layer])
				continue;
			++q;
			size_t idx = 0;
			#define ADD(w) \
			do \
			{ \
				auto widg = w; \
				lyr_widgets[layer].push_back(widg); \
				subg->add(widg); \
				++idx; \
			} \
			while(false)
			
			if (whole_map)
			{
				ADD(Checkbox(colSpan = 2,
					text = to_string(layer),
					boxPlacement = GUI::Checkbox::boxPlacement::RIGHT,
					checked = layer_enabled[layer],
					onToggleFunc = [&, layer](bool state)
					{
						layer_enabled[layer] = state;
						refresh_layer(layer);
					}
				));
			}
			else
			{
				ADD(Label(text = to_string(layer), colSpan = 2));
				ADD(DummyWidget());
				if (mapscr* m = cur_screens[layer])
				{
					newcombo const& cmb = combobuf[m->undercombo];
					ADD(TileFrame(
						tile = cmb.tile,
						cset = m->undercset,
						cset2 = cmb.csets,
						frames = cmb.frames,
						speed = cmb.speed,
						skipx = cmb.skipanim,
						skipy = cmb.skipanimy,
						flip = cmb.flip
					));
				}
				else
				{
					ADD(TileFrame(disabled = true));
				}
			}
			
			underc_idx = idx;
			ADD(SelComboSwatch(showvals = false,
				combo = undercombos[layer],
				cset = undercsets[layer],
				onSelectFunc = [&, layer](int32_t cmb, int32_t c)
				{
					undercombos[layer] = cmb;
					undercsets[layer] = c;
					refresh_layer(layer);
				}
			));
			
			preview_idx = idx;
			ADD(TileFrame());
			
			#undef ADD
		}
		g->add(subg);
	}
	
	if (whole_map)
	{
		g->add(Row(
			INFOBTN("If checked, all undercombos will be overwritten."
				"\nIf unchecked, only undercombos set to combo 0 will be overwritten."),
			Checkbox(text = "Overwrite", checked = overwrite_mode,
				onToggleFunc = [&](bool state)
				{
					overwrite_mode = state;
				})
		));
	}
	
	shared_ptr<GUI::Grid> btnrow = Row(vAlign = 1.0, spacing = 2_em);
	btnrow->add(Button(
		focused = true,
		text = "OK",
		minwidth = 90_px,
		onClick = message::OK));
	if (!whole_map)
	{
		btnrow->add(Button(
			text = "For Whole Map",
			minwidth = 90_px,
			onPressFunc = [&]()
			{
				save_results();
				whole_map = true;
				refresh_dlg();
			}));
	}
	btnrow->add(Button(
		text = "Cancel",
		minwidth = 90_px,
		onClick = message::CANCEL)
	);
	
	string window_info = layer_info;
	if (whole_map)
		window_info += "\nBase Screens that are 'invalid' (blued-out) will be ignored.";
	window = Window(
		title = whole_map
			? fmt::format("Undercombos for Whole Map {}", map+1)
			: fmt::format("Undercombos for Map {} Screen {}", map+1, screen),
		info = window_info,
		use_vsync = true,
		onClose = message::CANCEL,
		Column(
			g,
			btnrow
		)
	);
	for (int layer = 0; layer < 7; ++layer)
		refresh_layer(layer);
	return window;
}

bool UnderComboDialog::save_results()
{
	if (whole_map)
	{
		bool do_set_map = false;
		AlertDialog("Are you sure?",
			fmt::format("This will set undercombos for this entire map, and selected layers!\n{}",
				overwrite_mode
					? "All undercombos will be overwritten!"
					: "Only blank undercombos (set to combo 0) will be overwritten."),
			[&](bool ret,bool)
			{
				do_set_map = ret;
			}).show();
		if (!do_set_map) return false;
		for (int scr = 0; scr < 0x80; ++scr)
		{
			mapscr& s = TheMaps[map * MAPSCRS + scr];
			if (!(s.valid & mVALID))
				continue;
			mapscr* lyrs[7];
			get_screens(s, lyrs);
			for (int layer = 0; layer < 7; ++layer)
			{
				if (lyrs[layer])
				{
					if (overwrite_mode || !lyrs[layer]->undercombo)
					{
						lyrs[layer]->undercombo = undercombos[layer];
						lyrs[layer]->undercset = undercsets[layer];
					}
				}
			}
		}
	}
	else
	{
		for (int layer = 0; layer < 7; ++layer)
		{
			if (cur_screens[layer])
			{
				cur_screens[layer]->undercombo = undercombos[layer];
				cur_screens[layer]->undercset = undercsets[layer];
				if (layer > 0)
					Map.AbsoluteScrMakeValid(cur_screens[0]->layermap[layer-1], cur_screens[0]->layerscreen[layer-1]);
			}
		}
	}
	saved = false;
	return true;
}

bool UnderComboDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			return save_results();

		case message::CANCEL:
			return true;
	}
	return false;
}
