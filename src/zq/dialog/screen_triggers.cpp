#include "zq/dialog/screen_triggers.h"
#include "core/mapscr.h"
#include "core/trigger_graph.h"
#include "base/util.h"
#include "gui/builder.h"
#include "tiles.h"
#include "zalleg/render.h"
#include "zinfo.h"
#include "zq/zq_misc.h"
#include "zq/zq_tiles.h"
#include "zq/zquest.h"
#include <algorithm>
#include <allegro5/allegro.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <optional>
#include <set>

extern int32_t prv_mode;

void call_screen_triggers_dialog()
{
	ScreenTriggersDialog().show();
	// Combos may have been edited. Not while the dialog is up: the screen is its own then.
	refresh(rALL);
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

// `many_screens` when the cards come from more than one screen, so each says which.
static std::string placement_text(trigger_graph::card const& card, bool many_screens)
{
	if (card.placements.empty())
		return many_screens ? fmt::format("Not placed (combo change), 0x{:02X}", card.screen) : "Not placed (combo change)";
	std::set<int> layers, screens;
	bool ffc = false;
	for (auto const& p : card.placements)
	{
		screens.insert(p.screen);
		if (p.pos < 0)
			ffc = true;
		else if (p.layer)
			layers.insert(p.layer);
	}
	std::string text = fmt::format("{} placed", card.placements.size());
	if (many_screens)
	{
		std::vector<std::string> names;
		for (int screen : screens)
			names.push_back(fmt::format("0x{:02X}", screen));
		text += fmt::format(" on {}", fmt::join(names, ", "));
	}
	if (!layers.empty())
		text += fmt::format(", layer{} {}", layers.size() > 1 ? "s" : "", fmt::join(layers, ", "));
	if (ffc)
		text += ", FFC";
	return text;
}

ScreenTriggersDialog::ScreenTriggersDialog():
	screen(Map.getCurrScr()),
	whole_region(zc_get_config("zquest", "combo_triggers_whole_region", 1))
{
}

void ScreenTriggersDialog::refresh()
{
	int map = Map.getCurrMap();
	region = trigger_graph_region(map, screen);
	bool whole = whole_region && region.size() > 1;
	auto graph = build_trigger_graph(region, whole ? -1 : screen);
	hubs = graph.hubs;

	// The region's screens, side by side.
	GUI::TriggerDiagram::Picture picture;
	int min_col = region[0].screen % 16, min_row = region[0].screen / 16;
	int max_col = min_col, max_row = min_row;
	for (auto const& s : region)
	{
		min_col = std::min(min_col, s.screen % 16);
		min_row = std::min(min_row, s.screen / 16);
		max_col = std::max(max_col, s.screen % 16);
		max_row = std::max(max_row, s.screen / 16);
	}
	picture.origin = min_row * 16 + min_col;
	picture.cols = max_col - min_col + 1;
	picture.rows = max_row - min_row + 1;
	picture.screen = screen;
	picture.single = !whole;
	auto screen_of = [&](int s) -> trigger_graph_screen const& {
		return *std::find_if(region.begin(), region.end(), [&](auto const& r) { return r.screen == s; });
	};
	auto offset = [&](int s) {
		return std::pair<float, float>((s % 16 - min_col) * 256, (s / 16 - min_row) * 176);
	};

	// A combo is pictured in the CSet it's placed with; a combo it changes into, in the CSet
	// of the first placed combo of its chain.
	auto placed_cset = [&](size_t card) -> std::optional<int> {
		if (graph.cards[card].placements.empty())
			return std::nullopt;
		auto const& p = graph.cards[card].placements[0];
		auto const& layers = screen_of(p.screen).layers;
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
		info.subtitle = placement_text(card, whole);
		if (cmb.type)
			info.subtitle = fmt::format("{}, {}", ZI.getComboTypeName(cmb.type), info.subtitle);

		for (auto const& p : card.placements)
		{
			auto [ox, oy] = offset(p.screen);
			if (p.pos < 0)
			{
				mapscr* scr = screen_of(p.screen).layers[0];
				auto const& ffc = scr->ffcs[p.ffc];
				info.placements.push_back({ox + int(ffc.x), oy + int(ffc.y),
					float(scr->ffTileWidth(p.ffc) * 16), float(scr->ffTileHeight(p.ffc) * 16)});
			}
			else
				info.placements.push_back({ox + p.pos % 16 * 16, oy + p.pos / 16 * 16, 16, 16});
		}
		infos.push_back(std::move(info));
	}

	BITMAP* bmp = create_bitmap_ex(8, picture.cols * 256, picture.rows * 176);
	clear_bitmap(bmp);
	for (auto const& s : region)
	{
		// In preview mode zmap::draw only draws the screen being previewed, whichever screen
		// it's asked for.
		if (prv_mode && s.screen != Map.get_prv_scr())
			continue;
		auto [ox, oy] = offset(s.screen);
		Map.draw(bmp, ox, oy, cNODARK, map, s.screen, -1);
	}
	picture.bmp = to_a5(bmp);

	std::vector<std::string> empty_text = {
		whole ? "No combos in this region have triggers." : "No combos on this screen have triggers."};
	if (graph.cards.empty() && !whole && region.size() > 1 && !build_trigger_graph(region).cards.empty())
		empty_text.push_back("Other screens of its region have some: check Whole Region to see them.");
	diagram->setEmptyText(std::move(empty_text));
	diagram->setGraph(std::move(graph), std::move(infos), picture);
}

std::shared_ptr<GUI::Widget> ScreenTriggersDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	// Keep the scroll position and the picture's when the dialog is rebuilt to expand or shrink,
	// but start over on another screen or the whole region.
	float scroll_x = 0, scroll_y = 0;
	GUI::TriggerDiagram::PictureView picture_view;
	if (diagram)
	{
		picture_view = diagram->getPictureView();
		if (!new_graph)
		{
			scroll_x = diagram->getScrollX();
			scroll_y = diagram->getScrollY();
		}
	}
	diagram = std::make_shared<GUI::TriggerDiagram>(expanded);
	diagram->setOnClickCard([this](int cid, int trigger) {
		edit_combo_triggers(cid, trigger);
		refresh();
	});
	diagram->setOnPickScreen([this](int s) {
		screen = s;
		new_graph = true;
		refresh_dlg();
	});
	diagram->setOnShowElsewhere([this](size_t hub) {
		show_hub = hubs[hub];
		whole_region = true;
		new_graph = true;
		refresh_dlg();
	});
	refresh();
	diagram->setScroll(scroll_x, scroll_y);
	if (picture_view.zoom)
	{
		diagram->setPictureView(picture_view);
		if (new_graph)
			diagram->showPictureScreen();
	}
	// From a hub's note about the region's other screens: the same hub, in the whole region.
	if (show_hub)
	{
		for (size_t h = 0; h < hubs.size(); h++)
			if (hubs[h].kind == show_hub->kind && hubs[h].screen < 0 && util::contains(hubs[h].ids, show_hub->ids[0]))
				diagram->showOnlyHub(h);
		show_hub.reset();
	}
	new_graph = false;

	bool in_region = region.size() > 1;
	std::string where = in_region && whole_region
		? fmt::format("Screens 0x{:02X}-0x{:02X}", region.front().screen, region.back().screen)
		: fmt::format("Screen 0x{:02X}", screen);
	return Window(
		title = fmt::format("Combo Triggers (Map {}, {})", Map.getCurrMap() + 1, where),
		onClose = message::OK,
		Column(
			diagram,
			Row(
				Checkbox(
					text = "Whole Region",
					visible = in_region,
					checked = whole_region,
					onToggleFunc = [this](bool state)
					{
						whole_region = state;
						zc_set_config("zquest", "combo_triggers_whole_region", state);
						new_graph = true;
						refresh_dlg();
					}),
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
