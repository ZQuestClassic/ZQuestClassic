#include "zq/dialog/screen_triggers.h"
#include "core/mapscr.h"
#include "core/trigger_graph.h"
#include "gui/builder.h"
#include "tiles.h"
#include "zalleg/render.h"
#include "zinfo.h"
#include "zq/zq_misc.h"
#include "zq/zq_tiles.h"
#include "zq/zquest.h"
#include <allegro5/allegro.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <optional>
#include <set>

void call_screen_triggers_dialog()
{
	ScreenTriggersDialog().show();
	// Combos may have been edited. Not while the dialog is up: the screen is its own then.
	refresh(rALL);
}

// The current screen and its layers, as the editor draws them.
static std::array<mapscr*, 7> screen_layers()
{
	std::array<mapscr*, 7> layers = {Map.CurrScr()};
	for (int i = 0; i < 6; i++)
	{
		int layer_map = layers[0]->layermap[i] - 1;
		if (layer_map >= 0 && layer_map < map_count)
			layers[i + 1] = Map.AbsoluteScr(layer_map, layers[0]->layerscreen[i]);
	}
	return layers;
}

static ALLEGRO_BITMAP* to_a5(BITMAP* bmp)
{
	int old_flags = al_get_new_bitmap_flags();
	set_bitmap_create_flags(true);
	ALLEGRO_BITMAP* a5 = al_create_bitmap(bmp->w, bmp->h);
	al_set_new_bitmap_flags(old_flags);
	all_render_a5_bitmap(bmp, a5);
	destroy_bitmap(bmp);
	return a5;
}

static std::string placement_text(trigger_graph::card const& card)
{
	if (card.placements.empty())
		return "Not placed (combo change)";
	std::set<int> layers;
	bool ffc = false;
	for (auto const& p : card.placements)
	{
		if (p.pos < 0)
			ffc = true;
		else if (p.layer)
			layers.insert(p.layer);
	}
	std::string text = fmt::format("{} placed", card.placements.size());
	if (!layers.empty())
		text += fmt::format(", layer{} {}", layers.size() > 1 ? "s" : "", fmt::join(layers, ", "));
	if (ffc)
		text += ", FFC";
	return text;
}

void ScreenTriggersDialog::refresh()
{
	auto layers = screen_layers();
	auto graph = build_trigger_graph({{Map.getCurrScr(), layers}});

	// A combo is pictured in the CSet it's placed with; a combo it changes into, in the CSet
	// of the first placed combo of its chain.
	auto placed_cset = [&](size_t card) -> std::optional<int> {
		if (graph.cards[card].placements.empty())
			return std::nullopt;
		auto const& p = graph.cards[card].placements[0];
		return p.pos < 0 ? layers[0]->ffcs[p.ffc].cset : layers[p.layer]->cset[p.pos];
	};
	std::vector<int> csets(graph.cards.size(), 0);
	for (auto const& chain : graph.chains)
	{
		std::optional<int> chain_cset;
		for (size_t c : chain)
			if (!chain_cset)
				chain_cset = placed_cset(c);
		for (size_t c : chain)
			csets[c] = placed_cset(c).value_or(chain_cset.value_or(0));
	}

	std::vector<GUI::TriggerDiagram::CardInfo> infos;
	for (size_t c = 0; c < graph.cards.size(); c++)
	{
		auto const& card = graph.cards[c];
		newcombo const& cmb = combobuf[card.cid];
		GUI::TriggerDiagram::CardInfo info;

		BITMAP* icon = create_bitmap_ex(8, 16, 16);
		clear_bitmap(icon);
		overcombo(icon, 0, 0, card.cid, csets[c]);
		info.icon = to_a5(icon);
		info.cset = csets[c];

		info.title = cmb.label.empty() ? fmt::format("Combo {}", card.cid) : fmt::format("{} ({})", cmb.label, card.cid);
		if (card.count > 1)
			info.title += fmt::format(" x{}", card.count);
		info.subtitle = placement_text(card);
		if (cmb.type)
			info.subtitle = fmt::format("{}, {}", ZI.getComboTypeName(cmb.type), info.subtitle);

		for (auto const& p : card.placements)
		{
			if (p.pos < 0)
			{
				auto const& ffc = layers[0]->ffcs[p.ffc];
				info.placements.push_back({float(int(ffc.x)), float(int(ffc.y)),
					float(layers[0]->ffTileWidth(p.ffc) * 16), float(layers[0]->ffTileHeight(p.ffc) * 16)});
			}
			else
				info.placements.push_back({float(p.pos % 16 * 16), float(p.pos / 16 * 16), 16, 16});
		}
		infos.push_back(std::move(info));
	}

	BITMAP* screen_bmp = create_bitmap_ex(8, 256, 176);
	clear_bitmap(screen_bmp);
	Map.draw(screen_bmp, 0, 0, cNODARK, -1, -1, -1);

	diagram->setGraph(std::move(graph), std::move(infos), to_a5(screen_bmp));
}

std::shared_ptr<GUI::Widget> ScreenTriggersDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	// Keep the scroll position when the dialog is rebuilt to expand or shrink.
	float scroll_x = diagram ? diagram->getScrollX() : 0, scroll_y = diagram ? diagram->getScrollY() : 0;
	diagram = std::make_shared<GUI::TriggerDiagram>(expanded);
	diagram->setOnClickCard([this](int cid, int trigger) {
		edit_combo_triggers(cid, trigger);
		refresh();
	});
	refresh();
	diagram->setScroll(scroll_x, scroll_y);

	return Window(
		title = fmt::format("Combo Triggers (Map {}, Screen 0x{:02X})", Map.getCurrMap() + 1, Map.getCurrScr()),
		onClose = message::OK,
		Column(
			diagram,
			Row(
				Button(
					text = expanded ? "Exit Fullscreen" : "Fullscreen",
					minwidth = 90_px,
					onPressFunc = [this]()
					{
						expanded = !expanded;
						refresh_dlg();
					}),
				Button(
					text = "Close",
					minwidth = 90_px,
					onClick = message::OK)
			)
		)
	);
}

bool ScreenTriggersDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
		case message::OK:
			return true;
	}
	return false;
}
