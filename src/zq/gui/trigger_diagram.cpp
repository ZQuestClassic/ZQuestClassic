// The Screen > Combo Triggers diagram: draws a trigger_graph (core/trigger_graph.cpp) the way
// the combo triggers guide draws its diagrams. The dialog that hosts it, and gives it each
// card's icon and title, is zq/dialog/screen_triggers.cpp.
//
// Drawing: the widget's DIALOG proc (trigger_diagram_proc) only fills its box on the 8-bit
// screen. Everything else is drawn with Allegro 5 on a true-color dialog layer (add_dlg_layer,
// like QuestListView) whose render_cb is render(), so the chips get the guide's colors and the
// lines are antialiased. markDirty() flags that layer to be drawn again.
//
// Layout: setGraph() calls layout() once, which works out every rect and line in the diagram's
// own coordinates, (0, 0) at its top-left; render() and the hit tests only offset them by the
// view's position and scrollX/scrollY.
//  - A card stacks its header and then each trigger's chips: notes, causes, conditions,
//    effects and the effects of a failed condition, one chip per line of the trigger, with
//    small arrows between the groups.
//  - Each chain of combo changes is a row of cards. The hubs are a column on the left, each
//    level with the first line that leads to it, and each with its own vertical lane.
//  - A combo change is a solid arrow to the next card, or over the top of the row to any
//    other. A link runs left from the chip that names its hub to the hub's lane; from a card
//    that isn't first in its row, it goes down under the row first. Its arrowheads are the
//    link's reads (at the card) and writes (at the hub).
// drawDiagram() draws the laid out diagram at any offset, which is how Save as PNG draws all of
// it into a memory bitmap instead of only what's scrolled into view.
//
// Text and bitmaps are drawn at whole pixels (draw_text, draw_bitmap). The font is pixel art,
// and at a fractional scroll position its glyphs lose rows and columns.
//
// The picture is of the screen, or every screen of its scrolling region, and has its own zoom
// and pan (pictureView, in the picture's pixels); toPicture() maps a point of the widget onto
// it. The card placements the dialog gives are in the picture's pixels too.
//
// Input, in trigger_diagram_proc: the wheel scrolls the list or the diagram, or zooms the
// picture. The horizontal wheel has no message, so MSG_IDLE watches mouse_w for it, and also
// tracks the hover. A left drag pans the diagram or the picture, and a left click on a list row
// or the picture scrolls to that card. Clicks that the dialog acts on (a card, to open the
// Combo Editor; another screen of the region in the picture; a hub's note about the region's
// other screens) set `action` and raise geCLICK, which onEvent() passes to the dialog's
// function for it: the proc can't rebuild the dialog it's running in. A right click opens
// contextMenu().

#include "zq/gui/trigger_diagram.h"
#include "core/combo.h"
#include "core/fonts.h"
#include "core/qst.h"
#include "gui/common.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "gui/jwin_a5.h"
#include "gui/size.h"
#include "zalleg/render.h"
#include "zalleg/files.h"
#include "zalleg/new_menu.h"
#include "zq/zquest.h"
#include <algorithm>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <fmt/format.h>
#include <fmt/ranges.h>

using hub_kind = trigger_graph::hub_kind;
using LineStyle = GUI::TriggerDiagram::LineStyle;

namespace {

const float CARD_W = 200;
const float PAD = 8;
const float CHIP_PAD = 4;
const float CHIP_GAP = 3;
const float ARROW_H = 12;
const float GAP_X = 44;
const float ROW_GAP = 32;
const float HUB_W = 130;
const float MARGIN = 16;
const float PREVIEW_W = 256, PREVIEW_H = 176;
const char* const HINT[] = {
	"Click the picture or a row to find its",
	"combo, or a combo to edit it. Scroll:",
	"wheel (Shift: sideways), or drag. The",
	"picture zooms with the wheel, pans with",
	"a drag or the map. Double-click resets.",
};
const float MAX_PICTURE_ZOOM = 4;
// The most the map of the picture takes up in a corner of its box.
const float MAP_MAX_W = 64, MAP_MAX_H = 40;
// Until the picture is first zoomed or panned, hovering it says how.
bool picture_hint_seen = false;

// The colors of the combo triggers guide's diagrams (docs-www/source/_static/main.css).
struct Colors
{
	ALLEGRO_COLOR fill[4], stroke[4], text[4];
	ALLEGRO_COLOR state_fill, state_stroke, state_text;
};

ALLEGRO_COLOR hex(unsigned rgb)
{
	return al_map_rgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

// Indexed by ChipKind: plain, cause, condition, effect.
Colors const& chip_colors()
{
	static const Colors light = {
		{hex(0xf1efe8), hex(0xeeedfe), hex(0xfaeeda), hex(0xe1f5ee)},
		{hex(0x5f5e5a), hex(0x534ab7), hex(0x854f0b), hex(0x0f6e56)},
		{hex(0x444441), hex(0x3c3489), hex(0x633806), hex(0x085041)},
		hex(0xf1efe8), hex(0x5f5e5a), hex(0x444441),
	};
	static const Colors dark = {
		{hex(0x444441), hex(0x3c3489), hex(0x633806), hex(0x085041)},
		{hex(0xb4b2a9), hex(0xafa9ec), hex(0xef9f27), hex(0x5dcaa5)},
		{hex(0xd3d1c7), hex(0xcecbf6), hex(0xfac775), hex(0x9fe1cb)},
		hex(0x444441), hex(0xb4b2a9), hex(0xd3d1c7),
	};
	unsigned char r, g, b;
	al_unmap_rgb(a5color(jwin_pal[jcTEXTBG]), &r, &g, &b);
	return (r * 299 + g * 587 + b * 114) / 1000 < 128 ? dark : light;
}

ALLEGRO_FONT* diagram_font()
{
	return get_custom_font_a5(CFONT_DLG);
}

float line_height()
{
	return al_get_font_line_height(diagram_font());
}

float legend_row_height()
{
	return std::max(14.f, line_height() + 4);
}

float legend_height()
{
	return 2 * 6 + 4 * legend_row_height();
}

float list_row_height()
{
	return std::max(18.f, line_height() + 4);
}

// Text, and bitmaps, are drawn at whole pixels: the font is pixel art, and drawing it between
// pixels drops rows and columns of its glyphs.
void draw_text(ALLEGRO_FONT* f, ALLEGRO_COLOR color, float x, float y, int flags, const char* text)
{
	if (flags & ALLEGRO_ALIGN_CENTRE)
	{
		x -= al_get_text_width(f, text) / 2.f;
		flags &= ~ALLEGRO_ALIGN_CENTRE;
	}
	al_draw_text(f, color, std::round(x), std::round(y), flags, text);
}

void draw_bitmap(ALLEGRO_BITMAP* bmp, float x, float y)
{
	al_draw_bitmap(bmp, std::round(x), std::round(y), 0);
}

// Shortens text with "..." to fit `width`.
std::string fit_text(std::string text, float width)
{
	ALLEGRO_FONT* f = diagram_font();
	if (al_get_text_width(f, text.c_str()) <= width)
		return text;
	while (!text.empty() && al_get_text_width(f, (text + "...").c_str()) > width)
		text.pop_back();
	return text + "...";
}

// Splits text into lines no wider than `width`, at spaces where it can.
std::vector<std::string> wrap(std::string const& text, float width)
{
	ALLEGRO_FONT* f = diagram_font();
	std::vector<std::string> lines;
	std::string line;
	size_t i = 0;
	while (i < text.size())
	{
		size_t next = text.find(' ', i);
		std::string word = text.substr(i, next == std::string::npos ? std::string::npos : next - i);
		i = next == std::string::npos ? text.size() : next + 1;
		std::string candidate = line.empty() ? word : line + " " + word;
		if (al_get_text_width(f, candidate.c_str()) <= width)
		{
			line = candidate;
			continue;
		}
		if (!line.empty())
			lines.push_back(line);
		line = word;
		// A word too long for a line on its own is broken up.
		while (al_get_text_width(f, line.c_str()) > width && line.size() > 1)
		{
			size_t fit = line.size() - 1;
			while (fit > 1 && al_get_text_width(f, line.substr(0, fit).c_str()) > width)
				fit--;
			lines.push_back(line.substr(0, fit));
			line = line.substr(fit);
		}
	}
	if (!line.empty() || lines.empty())
		lines.push_back(line);
	return lines;
}

// An arrowhead pointing away from (from_x, from_y), whose tip is the pixel centred on (x, y).
// Its sides are at 45 degrees from the tip's far edge, so its rows are 1, 3, 5 and 7 pixels wide
// and it sits square on a 1 pixel line drawn through pixel centres.
void draw_arrowhead(float x, float y, float from_x, float from_y, ALLEGRO_COLOR color)
{
	float dx = x - from_x, dy = y - from_y;
	float len = std::sqrt(dx * dx + dy * dy);
	if (len < 0.01f)
		return;
	dx /= len;
	dy /= len;
	float size = 4;
	float tx = x + dx * 0.5f, ty = y + dy * 0.5f;
	al_draw_filled_triangle(tx, ty,
		tx - dx * size - dy * size, ty - dy * size + dx * size,
		tx - dx * size + dy * size, ty - dy * size - dx * size, color);
}

// A polyline, solid or with a dash pattern that carries on across its corners. Each segment
// runs on past its points by half the thickness, so that with the points at pixel centres and an
// odd thickness, it covers whole pixels and fills the corners.
void draw_path(std::vector<std::pair<float, float>> const& pts, float ox, float oy, LineStyle style,
	ALLEGRO_COLOR color, float thickness)
{
	float on = style == LineStyle::dotted ? 2 : 5, off = style == LineStyle::dotted ? 2 : 3;
	float phase = 0;
	for (size_t i = 1; i < pts.size(); i++)
	{
		float x1 = pts[i - 1].first + ox, y1 = pts[i - 1].second + oy;
		float x2 = pts[i].first + ox, y2 = pts[i].second + oy;
		float len = std::hypot(x2 - x1, y2 - y1);
		if (len < 0.01f)
			continue;
		float cap = thickness / 2;
		float ux = (x2 - x1) / len, uy = (y2 - y1) / len;
		x1 -= ux * cap, y1 -= uy * cap;
		x2 += ux * cap, y2 += uy * cap;
		float ext = len + 2 * cap;
		if (style == LineStyle::solid)
		{
			al_draw_line(x1, y1, x2, y2, color, thickness);
			continue;
		}
		float t = 0;
		while (t < ext)
		{
			float period = std::fmod(phase + t, on + off);
			float seg = period < on ? on - period : on + off - period;
			float t2 = std::min(ext, t + seg);
			if (period < on)
				al_draw_line(x1 + ux * t, y1 + uy * t, x1 + ux * t2, y1 + uy * t2, color, thickness);
			t = t2;
		}
		phase = std::fmod(phase + len, on + off);
	}
}

void draw_dashed_rect(float x1, float y1, float x2, float y2, ALLEGRO_COLOR color)
{
	draw_path({{x1, y1}, {x2, y1}, {x2, y2}, {x1, y2}, {x1, y1}}, 0, 0, LineStyle::dashed, color, 1);
}

// The "keeps firing" mark: a circling arrow.
void draw_repeat_mark(float cx, float cy, ALLEGRO_COLOR color)
{
	float r = 4;
	al_draw_arc(cx, cy, r, -ALLEGRO_PI * 0.4f, ALLEGRO_PI * 1.6f, color, 1.2f);
	float ax = cx + r * std::cos(-ALLEGRO_PI * 0.4f), ay = cy + r * std::sin(-ALLEGRO_PI * 0.4f);
	al_draw_filled_triangle(ax - 2.5f, ay - 2.5f, ax + 2.5f, ay - 1, ax, ay + 2.5f, color);
}

// The editor theme's colors, and the guide's chip colors.
struct Theme
{
	ALLEGRO_FONT* f;
	float lh;
	Colors const* colors;
	ALLEGRO_COLOR box, bg, fg, muted, line, accent;
};

ALLEGRO_COLOR mix(ALLEGRO_COLOR a, ALLEGRO_COLOR b, float t)
{
	return al_map_rgb_f(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t);
}

Theme theme()
{
	Theme t;
	t.f = diagram_font();
	t.lh = line_height();
	t.colors = &chip_colors();
	t.box = a5color(jwin_pal[jcBOX]);
	t.bg = a5color(jwin_pal[jcTEXTBG]);
	t.fg = a5color(jwin_pal[jcTEXTFG]);
	// Between the text and background colors, so they show in any editor theme.
	t.muted = mix(t.bg, t.fg, 0.7f);
	t.line = mix(t.bg, t.fg, 0.5f);
	t.accent = a5color(jwin_pal[jcSELBG]);
	return t;
}

// What the chips, boxes and lines mean. `anchor_x` is its left edge, or its right edge with
// `from_right`.
void draw_legend(Theme const& t, float anchor_x, float y, bool from_right)
{
	const char* chips[] = {"Cause", "Condition", "Effect", "State or group"};
	const char* lines[] = {"Changes into", "Sets or reads", "Copycat", "Keeps firing"};
	float sw = 22, gap = 5, row = legend_row_height(), pad = 6;
	float col_w[2] = {0, 0};
	for (int i = 0; i < 4; i++)
	{
		col_w[0] = std::max(col_w[0], sw + gap + al_get_text_width(t.f, chips[i]));
		col_w[1] = std::max(col_w[1], sw + gap + al_get_text_width(t.f, lines[i]));
	}
	float w = pad * 3 + col_w[0] + col_w[1], h = legend_height();
	float x = from_right ? anchor_x - w : anchor_x;
	al_draw_filled_rectangle(x, y, x + w, y + h, t.bg);
	al_draw_rectangle(x + 0.5f, y + 0.5f, x + w - 0.5f, y + h - 0.5f, t.line, 1);
	for (int i = 0; i < 4; i++)
	{
		float ry = y + pad + i * row, cy = ry + row / 2;
		// Chips: cause, condition and effect (ChipKind 1 to 3), then the hub box.
		float cx = x + pad;
		float y1 = cy - 5, y2 = cy + 5;
		if (i == 3)
		{
			al_draw_filled_rectangle(cx, y1, cx + sw, y2, t.colors->state_fill);
			al_draw_rectangle(cx + 0.5f, y1 + 0.5f, cx + sw - 0.5f, y2 - 0.5f, t.colors->state_stroke, 1.5f);
		}
		else
		{
			int k = i + 1;
			float radius = k == 1 ? 5 : 2;
			al_draw_filled_rounded_rectangle(cx, y1, cx + sw, y2, radius, radius, t.colors->fill[k]);
			if (k == 2)
				draw_dashed_rect(cx + 0.5f, y1 + 0.5f, cx + sw - 0.5f, y2 - 0.5f, t.colors->stroke[k]);
			else
				al_draw_rounded_rectangle(cx + 0.5f, y1 + 0.5f, cx + sw - 0.5f, y2 - 0.5f, radius, radius, t.colors->stroke[k], 1);
		}
		draw_text(t.f, t.fg, cx + sw + gap, cy - t.lh / 2, 0, chips[i]);

		// Lines.
		float lx = x + pad * 2 + col_w[0];
		if (i == 3)
			draw_repeat_mark(lx + sw / 2, cy, t.fg);
		else
		{
			LineStyle styles[] = {LineStyle::solid, LineStyle::dashed, LineStyle::dotted};
			float ly = std::floor(cy) + 0.5f;
			draw_path({{lx + 0.5f, ly}, {lx + sw - 1.5f, ly}}, 0, 0, styles[i], t.line, 1);
			draw_arrowhead(lx + sw - 0.5f, ly, lx, ly, t.line);
		}
		draw_text(t.f, t.fg, lx + sw + gap, cy - t.lh / 2, 0, lines[i]);
	}
}

// A hub's note about the combos on the region's other screens that use it.
std::string elsewhere_text(trigger_graph::hub const& hub)
{
	if (hub.elsewhere.empty())
		return "";
	int count = 0;
	bool writes = false, reads = false;
	std::vector<std::string> screens;
	for (auto const& e : hub.elsewhere)
	{
		count += e.count;
		writes |= e.writes;
		reads |= e.reads;
		screens.push_back(fmt::format("0x{:02X}", e.screen));
	}
	bool copycat = hub.kind == hub_kind::copycat;
	std::string what;
	if (writes && reads)
		what = copycat ? "activate it, and it fires them" : "count toward it and check it";
	else if (writes)
		what = copycat ? "activate it" : "count toward it";
	else
		what = copycat ? "it fires them" : "check it";
	return fmt::format("Also {} placed combo{} on {} ({})", count, count == 1 ? "" : "s",
		fmt::join(screens, ", "), what);
}

} // namespace

int32_t trigger_diagram_proc(int32_t msg, DIALOG* d, int32_t c)
{
	auto* widg = (GUI::TriggerDiagram*)d->dp;
	if (!widg)
		return D_O_K;

	switch (msg)
	{
		case MSG_START:
			widg->lastMouseW = mouse_w;
			if (!widg->rti)
			{
				widg->rti = add_dlg_layer();
				widg->rti->render_cb = [widg](RenderTreeItem*, bool) { widg->render(); };
			}
			break;

		case MSG_END:
			if (widg->rti)
			{
				remove_dlg_layer(widg->rti);
				widg->rti = nullptr;
			}
			break;

		case MSG_DRAW:
			if (d->flags & D_HIDDEN)
				break;
			rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, jwin_pal[jcBOX]);
			widg->markDirty();
			break;

		case MSG_WANTWHEEL:
			return 1;

		case MSG_WHEEL:
		{
			float step = 3 * line_height() * (c > 0 ? -1 : 1);
			if (widg->previewRect().contains(gui_mouse_x(), gui_mouse_y()))
			{
				widg->zoomPicture(c > 0 ? 1.25f : 0.8f, gui_mouse_x(), gui_mouse_y());
				picture_hint_seen = true;
			}
			else if (widg->listRect().contains(gui_mouse_x(), gui_mouse_y()))
				widg->listScroll += step;
			else if (key_shifts & KB_SHIFT_FLAG)
				widg->scrollX += step;
			else
				widg->scrollY += step;
			widg->clampScroll();
			widg->updateHover(gui_mouse_x(), gui_mouse_y());
			widg->markDirty();
			break;
		}

		case MSG_IDLE:
		{
			// The horizontal wheel (or a trackpad's sideways scroll) has no message of its own.
			// macOS also sends Shift + wheel as the horizontal wheel.
			int w = mouse_w;
			if (w != widg->lastMouseW)
			{
				if (widg->viewRect().contains(gui_mouse_x(), gui_mouse_y()))
				{
					widg->scrollX += 3 * line_height() * (w > widg->lastMouseW ? -1 : 1);
					widg->clampScroll();
					widg->markDirty();
				}
				widg->lastMouseW = w;
			}
			widg->updateHover(gui_mouse_x(), gui_mouse_y());
			break;
		}

		case MSG_CLICK:
		{
			if (gui_mouse_b() & 2)
			{
				if (widg->contextMenu(gui_mouse_x(), gui_mouse_y()))
					GUI_EVENT(d, geCLICK);
				break;
			}
			int start_x = gui_mouse_x(), start_y = gui_mouse_y();
			// Clicking or dragging on the map pans the picture there.
			if (widg->mapRect().contains(start_x, start_y))
			{
				widg->dragMap();
				break;
			}
			float scroll_x = widg->scrollX, scroll_y = widg->scrollY;
			auto start_view = widg->pictureView;
			bool in_view = widg->viewRect().contains(start_x, start_y);
			bool in_preview = widg->previewRect().contains(start_x, start_y);
			bool dragged = false;
			while (gui_mouse_b())
			{
				int dx = gui_mouse_x() - start_x, dy = gui_mouse_y() - start_y;
				if (std::abs(dx) + std::abs(dy) > 3)
					dragged = true;
				if (dragged && in_view)
				{
					widg->scrollX = scroll_x - dx;
					widg->scrollY = scroll_y - dy;
					widg->clampScroll();
					widg->markDirty();
				}
				else if (dragged && in_preview)
				{
					widg->pictureView.x = start_view.x - dx / start_view.zoom;
					widg->pictureView.y = start_view.y - dy / start_view.zoom;
					widg->clampPicture();
					widg->markDirty();
					picture_hint_seen = true;
				}
				update_hw_screen();
			}
			if (dragged)
				break;

			widg->updateHover(start_x, start_y);
			if (int card = widg->listCardAt(start_x, start_y); card >= 0)
			{
				widg->scrollToCard(card);
			}
			else if (in_preview)
			{
				auto const& pic = widg->picture;
				int screen = widg->pictureScreenAt(start_x, start_y);
				if (pic.single && screen >= 0 && screen != pic.screen && widg->onPickScreen)
				{
					widg->action = GUI::TriggerDiagram::Action::pick_screen;
					widg->pickedScreen = screen;
					GUI_EVENT(d, geCLICK);
				}
				else if (int card = widg->previewCardAt(start_x, start_y); card >= 0)
					widg->scrollToCard(card);
			}
			else if (int hub = widg->hubNoteAt(start_x, start_y); hub >= 0)
			{
				widg->action = GUI::TriggerDiagram::Action::show_elsewhere;
				widg->elsewhereHub = hub;
				GUI_EVENT(d, geCLICK);
			}
			else if (widg->hoverCard >= 0)
			{
				widg->action = GUI::TriggerDiagram::Action::edit;
				widg->clickedCid = widg->graph.cards[widg->hoverCard].cid;
				widg->clickedTrigger = widg->triggerAt(widg->hoverCard, start_y - widg->viewRect().y + widg->scrollY);
				GUI_EVENT(d, geCLICK);
			}
			break;
		}

		case MSG_DCLICK:
			if (gui_mouse_b() & 2)
				break;
			// A second click on the map soon after the first comes as a double click.
			if (widg->mapRect().contains(gui_mouse_x(), gui_mouse_y()))
				widg->dragMap();
			else if (widg->previewRect().contains(gui_mouse_x(), gui_mouse_y()))
			{
				widg->setPictureView({});
				widg->showPictureScreen();
			}
			break;
	}
	return D_O_K;
}

namespace GUI
{

TriggerDiagram::TriggerDiagram(bool expanded):
	expanded(expanded)
{
	// Expanded, the dialog fills the program window.
	setPreferredWidth(Size::pixels(zq_screen_w - (expanded ? 16 : 40)));
	setPreferredHeight(Size::pixels(zq_screen_h - (expanded ? 104 : 110)));
}

TriggerDiagram::~TriggerDiagram()
{
	freeBitmaps();
}

void TriggerDiagram::freeBitmaps()
{
	for (auto& card : cards)
		if (card.icon)
			al_destroy_bitmap(card.icon);
	cards.clear();
	if (picture.bmp)
		al_destroy_bitmap(picture.bmp);
	picture.bmp = nullptr;
}

void TriggerDiagram::setGraph(trigger_graph newGraph, std::vector<CardInfo> newCards, Picture newPicture)
{
	freeBitmaps();
	bool same_screens = picture.origin == newPicture.origin && picture.cols == newPicture.cols
		&& picture.rows == newPicture.rows;
	graph = std::move(newGraph);
	cards = std::move(newCards);
	picture = newPicture;
	// Rebuilt after an edit, the picture stays where it was.
	if (!same_screens || !pictureView.zoom)
	{
		pictureView = {};
		showPictureScreen();
	}
	hoverCard = hoverHub = hoverPicX = hoverPicY = previewFocus = focusCard = filterHub = -1;
	// The wheel may have turned in a dialog opened from here, like the Combo Editor.
	lastMouseW = mouse_w;
	layout();
	clampScroll();
	markDirty();
}

void TriggerDiagram::markDirty()
{
	if (rti)
		rti->dirty = true;
}

TriggerDiagram::Rect TriggerDiagram::bounds() const
{
	if (alDialog)
		return {float(alDialog->x), float(alDialog->y), float(alDialog->w), float(alDialog->h)};
	return {0, 0, float(getWidth()), float(getHeight())};
}

TriggerDiagram::Rect TriggerDiagram::previewRect() const
{
	Rect b = bounds();
	if (expanded)
		return {b.x, b.y, 0, 0};
	return {b.x + 8, b.y + 8, PREVIEW_W, PREVIEW_H};
}

TriggerDiagram::Rect TriggerDiagram::viewRect() const
{
	Rect b = bounds();
	float left = expanded ? 4 : 8 + PREVIEW_W + 12;
	return {b.x + left, b.y + 4, b.w - left - 4, b.h - 8};
}

TriggerDiagram::Rect TriggerDiagram::listRect() const
{
	Rect preview = previewRect();
	if (expanded)
		return preview;
	Rect b = bounds();
	float bottom = b.y + b.h - 8 - std::size(HINT) * line_height() - 6 - legend_height() - 8;
	float y = preview.y + preview.h + 8;
	return {preview.x, y, preview.w, std::max(0.f, bottom - y)};
}

int TriggerDiagram::listCardAt(int mx, int my) const
{
	Rect list = listRect();
	if (!list.contains(mx, my))
		return -1;
	size_t row = (my - list.y + listScroll) / list_row_height();
	return row < listRows.size() ? int(listRows[row].first) : -1;
}

std::pair<float, float> TriggerDiagram::toPicture(float mx, float my) const
{
	Rect preview = previewRect();
	return {pictureView.x + (mx - preview.x) / pictureView.zoom, pictureView.y + (my - preview.y) / pictureView.zoom};
}

int TriggerDiagram::pictureScreenAt(int mx, int my) const
{
	if (!previewRect().contains(mx, my))
		return -1;
	auto [x, y] = toPicture(mx, my);
	int col = std::floor(x / 256), row = std::floor(y / 176);
	if (col < 0 || row < 0 || col >= picture.cols || row >= picture.rows)
		return -1;
	return picture.origin + row * 16 + col;
}

std::pair<float, float> TriggerDiagram::pictureSize() const
{
	return {picture.cols * 256.f, picture.rows * 176.f};
}

float TriggerDiagram::minPictureZoom() const
{
	auto [pw, ph] = pictureSize();
	float fit = std::min(PREVIEW_W / pw, PREVIEW_H / ph);
	return std::min(1.f, fit);
}

std::array<bool, 4> TriggerDiagram::pictureOverflow() const
{
	auto const& v = pictureView;
	float w = PREVIEW_W / v.zoom, h = PREVIEW_H / v.zoom;
	auto [pw, ph] = pictureSize();
	// At least a pixel of the box's worth.
	return {v.x * v.zoom >= 1, v.y * v.zoom >= 1, (pw - v.x - w) * v.zoom >= 1, (ph - v.y - h) * v.zoom >= 1};
}

TriggerDiagram::Rect TriggerDiagram::mapRect() const
{
	auto over = pictureOverflow();
	if (expanded || !picture.bmp || over == std::array<bool, 4>{})
		return {0, 0, 0, 0};
	auto [pw, ph] = pictureSize();
	float scale = std::min(MAP_MAX_W / pw, MAP_MAX_H / ph);
	float w = std::round(pw * scale), h = std::round(ph * scale);
	Rect preview = previewRect();
	return {preview.x + preview.w - 4 - w, preview.y + preview.h - 4 - h, w, h};
}

void TriggerDiagram::dragMap()
{
	// Panning keeps some of the picture out of view, so the map stays where it is.
	Rect m = mapRect();
	auto [pw, ph] = pictureSize();
	do
	{
		pictureView.x = (gui_mouse_x() - m.x) * pw / m.w - PREVIEW_W / pictureView.zoom / 2;
		pictureView.y = (gui_mouse_y() - m.y) * ph / m.h - PREVIEW_H / pictureView.zoom / 2;
		clampPicture();
		markDirty();
		update_hw_screen();
	}
	while (gui_mouse_b());
	picture_hint_seen = true;
}

void TriggerDiagram::clampPicture()
{
	auto& v = pictureView;
	v.zoom = std::clamp(v.zoom, minPictureZoom(), MAX_PICTURE_ZOOM);
	// Centered when it's smaller than the box.
	auto clamp = [](float& pos, float shown, float size) {
		pos = shown >= size ? (size - shown) / 2 : std::clamp(pos, 0.f, size - shown);
	};
	auto [pw, ph] = pictureSize();
	clamp(v.x, PREVIEW_W / v.zoom, pw);
	clamp(v.y, PREVIEW_H / v.zoom, ph);
}

void TriggerDiagram::setPictureView(PictureView view)
{
	pictureView = view;
	if (!pictureView.zoom)
		pictureView.zoom = 1;
	clampPicture();
	markDirty();
}

void TriggerDiagram::showPictureScreen()
{
	if (!pictureView.zoom)
		pictureView.zoom = 1;
	int col = (picture.screen - picture.origin) % 16, row = (picture.screen - picture.origin) / 16;
	pictureView.x = col * 256 + 128 - PREVIEW_W / pictureView.zoom / 2;
	pictureView.y = row * 176 + 88 - PREVIEW_H / pictureView.zoom / 2;
	clampPicture();
	markDirty();
}

void TriggerDiagram::zoomPicture(float factor, int mx, int my)
{
	// The point under the mouse stays under it.
	auto [px, py] = toPicture(mx, my);
	Rect preview = previewRect();
	pictureView.zoom *= factor;
	// In quarter steps once it's bigger than the picture's own size, so the pixels stay even.
	if (pictureView.zoom > 1)
		pictureView.zoom = std::round(pictureView.zoom * 4) / 4;
	clampPicture();
	pictureView.x = px - (mx - preview.x) / pictureView.zoom;
	pictureView.y = py - (my - preview.y) / pictureView.zoom;
	clampPicture();
	markDirty();
}

void TriggerDiagram::showPicturePlacement(size_t card)
{
	if (cards[card].placements.empty())
		return;
	auto const& r = cards[card].placements[0];
	auto const& v = pictureView;
	float shown_w = PREVIEW_W / v.zoom, shown_h = PREVIEW_H / v.zoom;
	if (r.x >= v.x && r.y >= v.y && r.x + r.w <= v.x + shown_w && r.y + r.h <= v.y + shown_h)
		return;
	pictureView.x = r.x + r.w / 2 - shown_w / 2;
	pictureView.y = r.y + r.h / 2 - shown_h / 2;
	clampPicture();
}

int TriggerDiagram::hubNoteAt(int mx, int my) const
{
	Rect view = viewRect();
	if (!view.contains(mx, my))
		return -1;
	float x = mx - view.x + scrollX, y = my - view.y + scrollY;
	for (size_t h = 0; h < hubNoteRects.size(); h++)
		if (hubNoteRects[h].contains(x, y))
			return h;
	return -1;
}

int TriggerDiagram::previewCardAt(int mx, int my)
{
	if (!previewRect().contains(mx, my))
		return -1;
	auto [x, y] = toPicture(mx, my);
	std::vector<int> found;
	for (size_t c = 0; c < cards.size(); c++)
		for (auto const& r : cards[c].placements)
			if (r.contains(x, y))
			{
				found.push_back(c);
				break;
			}
	if (found.empty())
		return -1;
	// Clicking the same spot again moves on to the next combo placed there.
	auto it = std::find(found.begin(), found.end(), previewFocus);
	previewFocus = it == found.end() || it + 1 == found.end() ? found.front() : *(it + 1);
	return previewFocus;
}

void TriggerDiagram::scrollToCard(size_t card)
{
	focusCard = card;
	showPicturePlacement(card);
	Rect view = viewRect();
	auto const& r = cardLayouts[card].rect;
	scrollX = r.x - std::max(MARGIN, (view.w - r.w) / 2);
	scrollY = r.y - 2 * MARGIN;
	clampScroll();
	markDirty();
}

void TriggerDiagram::clampScroll()
{
	Rect list = listRect();
	listScroll = std::round(std::clamp(listScroll, 0.f, std::max(0.f, listRows.size() * list_row_height() - list.h)));
	Rect view = viewRect();
	scrollX = std::round(std::clamp(scrollX, 0.f, std::max(0.f, contentW - view.w)));
	scrollY = std::round(std::clamp(scrollY, 0.f, std::max(0.f, contentH - view.h)));
}

void TriggerDiagram::layout()
{
	float lh = line_height();
	float inner = CARD_W - 2 * PAD;
	size_t n = graph.cards.size();
	cardLayouts.assign(n, {});
	paths.clear();
	listRows.clear();
	for (auto const& chain : graph.chains)
		for (size_t k = 0; k < chain.size(); k++)
			listRows.push_back({chain[k], k > 0});

	// Where links and combo changes leave each card, relative to its top.
	std::vector<std::vector<float>> change_y(n);
	std::vector<std::vector<std::vector<float>>> link_y(n);

	for (size_t c = 0; c < n; c++)
	{
		auto const& card = graph.cards[c];
		auto& L = cardLayouts[c];
		auto const& info = cards[c];
		float text_w = inner - 20;
		size_t header_lines = wrap(info.title, text_w).size() + wrap(info.subtitle, text_w).size();
		float y = PAD + std::max(16.f, header_lines * lh) + PAD;

		for (size_t ti = 0; ti < card.triggers.size(); ti++)
		{
			auto const& t = card.triggers[ti];
			if (ti > 0)
			{
				L.dividers.push_back(y + 2);
				y += 8;
			}
			float start = y;
			if (!t.label.empty())
			{
				auto lines = wrap(t.label, inner);
				L.labels.push_back({y, lines});
				y += lines.size() * lh + 4;
			}

			size_t first_chip = L.chips.size();
			auto add_chips = [&](std::vector<std::string> const& texts, ChipKind kind, std::string const& prefix, bool repeats) {
				for (auto const& text : texts)
				{
					float indent = repeats ? 12 : 0;
					auto lines = wrap(prefix + text, inner - 2 * CHIP_PAD - indent);
					float h = lines.size() * lh + 2 * CHIP_PAD;
					L.chips.push_back({{PAD, y, inner, h}, kind, lines, repeats});
					y += h + CHIP_GAP;
					repeats = false;
				}
			};
			bool separate = !t.weapon_causes.empty() || !t.weapon_effects.empty();
			add_chips(t.notes, ChipKind::plain, "", false);
			add_chips(t.causes, ChipKind::cause, "", t.keeps_firing);
			add_chips(t.weapon_causes, ChipKind::cause, "Weapon: ", false);
			bool has_cause = !t.causes.empty() || !t.weapon_causes.empty();
			if (has_cause && (!t.conditions.empty() || !t.effects.empty() || !t.weapon_effects.empty()))
			{
				L.arrows.push_back({CARD_W / 2, y});
				y += ARROW_H;
			}
			add_chips(t.conditions, ChipKind::condition, "", false);
			if (!t.conditions.empty() && (!t.effects.empty() || !t.weapon_effects.empty()))
			{
				L.arrows.push_back({CARD_W / 2, y});
				y += ARROW_H;
			}
			add_chips(t.effects, ChipKind::effect, separate ? "Non-weapon: " : "", false);
			add_chips(t.weapon_effects, ChipKind::effect, "Weapon: ", false);
			add_chips(t.fail_effects, ChipKind::effect, "If a condition fails: ", false);
			float mid = (start + y) / 2;
			L.triggerSpans.push_back({start, y});

			// Anchor the arrows on the chip that mentions them.
			auto chip_mentioning = [&](std::string const& name) {
				for (size_t i = first_chip; i < L.chips.size(); i++)
				{
					std::string joined;
					for (auto const& line : L.chips[i].lines)
						joined += line + " ";
					if (joined.find(name) != std::string::npos)
						return L.chips[i].rect.y + L.chips[i].rect.h / 2;
				}
				return mid;
			};
			change_y[c].push_back(chip_mentioning("Change combo"));
			link_y[c].emplace_back();
			for (auto const& link : t.links)
				link_y[c].back().push_back(chip_mentioning(link.name));
		}
		L.rect.w = CARD_W;
		L.rect.h = y + PAD - CHIP_GAP;
	}

	// Each chain of combo changes is a row of cards.
	struct Row
	{
		float top, bottom;
		std::vector<size_t> hubs;
		int back_routes = 0;
	};
	std::vector<Row> rows;
	std::vector<int> row_of(n), col_of(n);
	float y = MARGIN + 12;
	// The hubs are in a column on the left, so they're in view next to each row's first card,
	// each with its own lane of lines leading to it.
	size_t nh = graph.hubs.size();
	std::vector<float> lane_x(nh);
	for (size_t h = 0; h < nh; h++)
		lane_x[h] = MARGIN + HUB_W + 12 + h * 5;
	float left = nh ? MARGIN + HUB_W + 12 + nh * 5 + 14 : MARGIN;
	float right = left;
	for (size_t r = 0; r < graph.chains.size(); r++)
	{
		Row row;
		row.top = y;
		float x = left, h = 0;
		for (size_t k = 0; k < graph.chains[r].size(); k++)
		{
			size_t c = graph.chains[r][k];
			cardLayouts[c].rect.x = x;
			cardLayouts[c].rect.y = y;
			h = std::max(h, cardLayouts[c].rect.h);
			row_of[c] = r;
			col_of[c] = k;
			x += CARD_W + GAP_X;
			for (auto const& t : graph.cards[c].triggers)
				for (auto const& link : t.links)
					if (std::find(row.hubs.begin(), row.hubs.end(), link.hub) == row.hubs.end())
						row.hubs.push_back(link.hub);
		}
		right = std::max(right, x - GAP_X);
		row.bottom = y + h;
		y = row.bottom + 10 + 6 * row.hubs.size() + ROW_GAP;
		rows.push_back(row);
	}
	contentH = y;

	hubRects.assign(nh, {});
	hubLines.assign(nh, {});
	hubNotes.assign(nh, {});
	hubNoteRects.assign(nh, {});
	// Each hub is level with the first line that leads to it.
	std::vector<float> want_y(nh, 1e9f);
	for (size_t c = 0; c < n; c++)
	{
		auto const& t = graph.cards[c].triggers;
		for (size_t ti = 0; ti < t.size(); ti++)
		{
			for (size_t li = 0; li < t[ti].links.size(); li++)
			{
				float& want = want_y[t[ti].links[li].hub];
				want = std::min(want, cardLayouts[c].rect.y + link_y[c][ti][li]);
			}
		}
	}
	std::vector<size_t> hub_order(nh);
	for (size_t h = 0; h < nh; h++)
	{
		hub_order[h] = h;
		hubLines[h] = wrap(graph.hubs[h].name(), HUB_W - 12);
		float hh = hubLines[h].size() * lh + 12;
		if (auto note = elsewhere_text(graph.hubs[h]); !note.empty())
		{
			hubNotes[h] = wrap(note, HUB_W - 12);
			hh += hubNotes[h].size() * lh + 6;
		}
		float cy = want_y[h] < 1e9f ? want_y[h] : MARGIN;
		hubRects[h] = {MARGIN, cy - hh / 2, HUB_W, hh};
	}
	std::sort(hub_order.begin(), hub_order.end(), [&](size_t a, size_t b) { return hubRects[a].y < hubRects[b].y; });
	float hub_bottom = MARGIN;
	for (size_t h : hub_order)
	{
		hubRects[h].y = std::max(hubRects[h].y, hub_bottom);
		hub_bottom = hubRects[h].y + hubRects[h].h + 10;
		if (!hubNotes[h].empty())
		{
			float note_h = hubNotes[h].size() * lh + 6;
			hubNoteRects[h] = {hubRects[h].x, hubRects[h].y + hubRects[h].h - note_h - 3, HUB_W, note_h};
		}
	}
	contentW = right + MARGIN;
	contentH = std::max(contentH, hub_bottom + MARGIN);
	// Fullscreen, room to scroll the last cards out from under the legend.
	if (expanded)
		contentH += legend_height() + 16;

	// Combo changes: an arrow into the card the combo changes into.
	for (size_t c = 0; c < n; c++)
	{
		auto const& L = cardLayouts[c];
		auto const& triggers = graph.cards[c].triggers;
		for (size_t ti = 0; ti < triggers.size(); ti++)
		{
			int target = triggers[ti].changes_to;
			if (target < 0 || size_t(target) == c)
				continue;
			auto const& T = cardLayouts[target].rect;
			float ax = L.rect.x + L.rect.w, ay = L.rect.y + change_y[c][ti];
			float ty = std::clamp(ay, T.y + 2 * lh, T.y + T.h - lh);
			Path p;
			p.style = LineStyle::solid;
			p.arrow_start = false;
			p.arrow_end = true;
			p.card = c;
			p.hub = -1;
			if (row_of[target] == row_of[c] && col_of[target] == col_of[c] + 1)
			{
				float mid = ax + GAP_X / 2;
				p.points = {{ax, ay}, {mid, ay}, {mid, ty}, {T.x - 1, ty}};
			}
			else
			{
				// Around the top of the row, so the line doesn't cross the cards between.
				auto& row = rows[row_of[c]];
				float gy = row.top - 6 - 4 * (row.back_routes++ % 4);
				float xr = ax + 10, xl = T.x - 10;
				p.points = {{ax, ay}, {xr, ay}, {xr, gy}, {xl, gy}, {xl, ty}, {T.x - 1, ty}};
			}
			paths.push_back(p);
		}
	}

	// Links: from the left of the card to the hub's lane and into the hub. From a card after
	// the first of its row, down to the gap under the row first, to pass under the cards on its
	// left.
	for (size_t c = 0; c < n; c++)
	{
		auto const& L = cardLayouts[c];
		auto const& row = rows[row_of[c]];
		auto const& triggers = graph.cards[c].triggers;
		for (size_t ti = 0; ti < triggers.size(); ti++)
		{
			for (size_t li = 0; li < triggers[ti].links.size(); li++)
			{
				auto const& link = triggers[ti].links[li];
				size_t idx = std::find(row.hubs.begin(), row.hubs.end(), link.hub) - row.hubs.begin();
				float ax = L.rect.x, ay = L.rect.y + link_y[c][ti][li];
				auto const& H = hubRects[link.hub];
				float hx = H.x + H.w, hy = std::clamp(ay, H.y + 4, H.y + H.h - 4);
				float lane = lane_x[link.hub];
				Path p;
				if (col_of[c] == 0)
				{
					p.points = {{ax - 1, ay}, {lane, ay}, {lane, hy}, {hx, hy}};
				}
				else
				{
					float x1 = ax - 6 - 3 * (idx % 8);
					float gy = row.bottom + 8 + 6 * idx;
					p.points = {{ax - 1, ay}, {x1, ay}, {x1, gy}, {lane, gy}, {lane, hy}, {hx, hy}};
				}
				p.style = graph.hubs[link.hub].kind == hub_kind::copycat ? LineStyle::dotted : LineStyle::dashed;
				p.arrow_start = link.reads;
				p.arrow_end = link.writes;
				p.card = c;
				p.hub = link.hub;
				paths.push_back(p);
			}
		}
	}

	// On whole pixels, for drawDiagram to draw the lines through pixel centres.
	for (auto& p : paths)
		for (auto& [x, y] : p.points)
			x = std::round(x), y = std::round(y);
}

void TriggerDiagram::updateHover(int mx, int my)
{
	int card = -1, hub = -1, pic_x = -1, pic_y = -1;
	Rect preview = previewRect();
	Rect view = viewRect();
	bool in_preview = preview.contains(mx, my);
	if (in_preview != hoverPreview)
	{
		hoverPreview = in_preview;
		markDirty();
	}
	if (in_preview && !mapRect().contains(mx, my))
	{
		auto [x, y] = toPicture(mx, my);
		pic_x = std::floor(x);
		pic_y = std::floor(y);
	}
	else if (listRect().contains(mx, my))
	{
		card = listCardAt(mx, my);
	}
	else if (view.contains(mx, my))
	{
		float x = mx - view.x + scrollX, y = my - view.y + scrollY;
		for (size_t c = 0; c < cardLayouts.size(); c++)
			if (cardLayouts[c].rect.contains(x, y))
				card = c;
		for (size_t h = 0; h < hubRects.size(); h++)
			if (hubRects[h].contains(x, y))
				hub = h;
	}
	if (card != hoverCard || hub != hoverHub || pic_x != hoverPicX || pic_y != hoverPicY)
	{
		hoverCard = card;
		hoverHub = hub;
		hoverPicX = pic_x;
		hoverPicY = pic_y;
		markDirty();
	}
}

bool TriggerDiagram::cardHighlighted(int card) const
{
	if (card == hoverCard || card == focusCard)
		return true;
	if (hoverHub >= 0)
		for (auto const& t : graph.cards[card].triggers)
			for (auto const& link : t.links)
				if (int(link.hub) == hoverHub)
					return true;
	if (hoverPicX >= 0)
		for (auto const& r : cards[card].placements)
			if (r.contains(hoverPicX, hoverPicY))
				return true;
	return false;
}

bool TriggerDiagram::linksFilter(int card) const
{
	if (filterHub < 0)
		return true;
	for (auto const& t : graph.cards[card].triggers)
		for (auto const& link : t.links)
			if (int(link.hub) == filterHub)
				return true;
	return false;
}

int TriggerDiagram::triggerAt(int card, float diagram_y) const
{
	auto const& L = cardLayouts[card];
	float y = diagram_y - L.rect.y;
	int trigger = -1;
	// The space between two triggers goes to the one below.
	for (size_t ti = 0; ti < L.triggerSpans.size(); ti++)
		if (y >= L.triggerSpans[ti].first - (ti ? 8 : 0) && y < L.triggerSpans[ti].second)
			trigger = ti;
	return trigger;
}

bool TriggerDiagram::contextMenu(int mx, int my)
{
	updateHover(mx, my);
	int card = hoverCard, hub = hoverHub;
	if (card < 0 && hoverPicX >= 0)
		card = previewCardAt(mx, my);
	int trigger = card >= 0 && viewRect().contains(mx, my) ? triggerAt(card, my - viewRect().y + scrollY) : -1;

	// What to edit, set by the menu: -2 for nothing, -1 for the combo, else a trigger. -3 shows
	// the whole region.
	int edit = -2;
	NewMenu menu;
	if (card >= 0)
	{
		int cid = graph.cards[card].cid;
		menu.add({"Edit Combo", [&]() { edit = -1; }});
		if (trigger >= 0)
		{
			menu.add({"Edit Trigger", [&]() { edit = trigger; }});
			menu.add({"Copy Trigger", [&]() { copyTrigger(card, trigger); }});
		}
		menu.add({"Use Combo for Drawing", [&]() { select_combo_for_drawing(cid, cards[card].cset); }});
		menu.add_sep();
	}
	if (hub >= 0)
		menu.add({"Show Only What Links Here", [&]() { filterHub = hub; }});
	if (hub >= 0 && !graph.hubs[hub].elsewhere.empty() && onShowElsewhere)
		menu.add({"Show Whole Region", [&]() { edit = -3; }});
	menu.add({"Show All", [&]() { filterHub = -1; }, nullopt, filterHub < 0 ? MFL_DIS : 0});
	menu.add_sep();
	if (devpwd())
		menu.add({"Copy as JSON", [&]() { copyJson(); }});
	menu.add({"Save as PNG...", [&]() { savePng(); }});
	menu.pop(mx, my);
	markDirty();

	if (edit == -2)
		return false;
	if (edit == -3)
	{
		action = Action::show_elsewhere;
		elsewhereHub = hub;
		return true;
	}
	action = Action::edit;
	clickedCid = graph.cards[card].cid;
	clickedTrigger = edit;
	return true;
}

void TriggerDiagram::copyJson() const
{
	set_al_clipboard(trigger_graph_json(graph).dump(2));
}

void TriggerDiagram::copyTrigger(int card, int trigger) const
{
	// The JSON the Combo Editor's trigger Copy and Paste buttons use.
	nlohmann::json data = combobuf[graph.cards[card].cid].triggers[trigger];
	set_al_clipboard(data.dump());
}

void TriggerDiagram::savePng()
{
	if (auto path = prompt_for_new_file("Save Combo Triggers Diagram (.png)", "png", nullptr, imagepath, false))
		exportPng(*path);
}

void TriggerDiagram::exportPng(std::string const& path)
{
	// The whole diagram, not just what's scrolled into view, with the legend below it.
	Theme const t = theme();
	int w = std::ceil(contentW), h = std::ceil(contentH + legend_height() + 2 * MARGIN);
	ALLEGRO_STATE state;
	al_store_state(&state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ALLEGRO_BITMAP* bmp = al_create_bitmap(w, h);
	if (bmp)
	{
		al_set_target_bitmap(bmp);
		al_clear_to_color(t.bg);
		// Without what the mouse happens to be on.
		int hover_card = hoverCard, hover_hub = hoverHub, hover_x = hoverPicX, hover_y = hoverPicY, focus = focusCard;
		hoverCard = hoverHub = hoverPicX = hoverPicY = focusCard = -1;
		drawDiagram(0, 0, {0, 0, float(w), float(h)});
		hoverCard = hover_card, hoverHub = hover_hub, hoverPicX = hover_x, hoverPicY = hover_y, focusCard = focus;
		draw_legend(t, MARGIN, h - MARGIN - legend_height(), false);
		al_save_bitmap(path.c_str(), bmp);
		al_destroy_bitmap(bmp);
	}
	al_restore_state(&state);
}

void TriggerDiagram::drawDiagram(float ox, float oy, Rect clip)
{
	Theme const t = theme();
	ALLEGRO_FONT* f = t.f;
	float lh = t.lh;
	ALLEGRO_COLOR bg = t.bg, fg = t.fg, muted = t.muted, line = t.line, accent = t.accent;
	// Over what the filter leaves out. Allegro blends with premultiplied alpha.
	float a = 0.75f;
	ALLEGRO_COLOR fade = al_map_rgba_f(bg.r * a, bg.g * a, bg.b * a, a);

	// Lines first, so the cards cover them where they pass close by. The highlighted ones go
	// on top of the rest.
	for (int pass = 0; pass < 2; pass++)
	{
		for (auto const& p : paths)
		{
			if (filterHub >= 0 && (p.hub >= 0 ? p.hub != filterHub : !linksFilter(p.card)))
				continue;
			bool hl = cardHighlighted(p.card) || (p.hub >= 0 && p.hub == hoverHub);
			if (hl != (pass == 1))
				continue;
			ALLEGRO_COLOR color = hl ? accent : line;
			// An odd width, to be centred on the arrowheads.
			float thick = hl ? 3 : 1;
			// The points are pixels, and the lines go through their centres.
			float px = ox + 0.5f, py = oy + 0.5f;
			draw_path(p.points, px, py, p.style, color, thick);
			auto const& pts = p.points;
			if (p.arrow_end)
				draw_arrowhead(pts.back().first + px, pts.back().second + py,
					pts[pts.size() - 2].first + px, pts[pts.size() - 2].second + py, color);
			if (p.arrow_start)
				draw_arrowhead(pts.front().first + px, pts.front().second + py,
					pts[1].first + px, pts[1].second + py, color);
		}
	}

	for (size_t c = 0; c < cardLayouts.size(); c++)
	{
		auto const& L = cardLayouts[c];
		auto const& info = cards[c];
		float x = L.rect.x + ox, y = L.rect.y + oy;
		if (x > clip.x + clip.w || y > clip.y + clip.h || x + L.rect.w < clip.x || y + L.rect.h < clip.y)
			continue;
		bool hl = cardHighlighted(c);
		al_draw_filled_rounded_rectangle(x, y, x + L.rect.w, y + L.rect.h, 6, 6, bg);
		// Inside the card's edge, so lines meeting it aren't covered.
		float border = hl ? 3 : 1, in = border / 2;
		al_draw_rounded_rectangle(x + in, y + in, x + L.rect.w - in, y + L.rect.h - in, 6, 6,
			hl ? accent : line, border);

		if (info.icon)
			draw_bitmap(info.icon, x + PAD, y + PAD);
		float text_x = x + PAD + 20, text_y = y + PAD;
		float text_w = CARD_W - 2 * PAD - 20;
		for (auto const& s : wrap(info.title, text_w))
		{
			draw_text(f, fg, text_x, text_y, 0, s.c_str());
			text_y += lh;
		}
		for (auto const& s : wrap(info.subtitle, text_w))
		{
			draw_text(f, muted, text_x, text_y, 0, s.c_str());
			text_y += lh;
		}

		for (float dy2 : L.dividers)
			al_draw_line(x + PAD, y + dy2, x + L.rect.w - PAD, y + dy2, line, 1);
		for (auto const& [ly, lines] : L.labels)
		{
			float yy = y + ly;
			for (auto const& s : lines)
			{
				draw_text(f, muted, x + PAD, yy, 0, s.c_str());
				yy += lh;
			}
		}
		for (auto const& [ax, ay] : L.arrows)
		{
			// Through the centres of a column of pixels, with 3 rows between it and the chips
			// above and below.
			float cx = x + ax + 0.5f, top = y + ay;
			float tip = top + ARROW_H - 3.5f;
			al_draw_line(cx, top, cx, tip - 3.5f, line, 1);
			draw_arrowhead(cx, tip, cx, top, line);
		}
		for (auto const& chip : L.chips)
		{
			int k = int(chip.kind);
			float cx1 = x + chip.rect.x, cy1 = y + chip.rect.y;
			float cx2 = cx1 + chip.rect.w, cy2 = cy1 + chip.rect.h;
			float radius = chip.kind == ChipKind::cause ? std::min(10.f, chip.rect.h / 2) : 3;
			al_draw_filled_rounded_rectangle(cx1, cy1, cx2, cy2, radius, radius, t.colors->fill[k]);
			if (chip.kind == ChipKind::condition)
				draw_dashed_rect(cx1 + 0.5f, cy1 + 0.5f, cx2 - 0.5f, cy2 - 0.5f, t.colors->stroke[k]);
			else
				al_draw_rounded_rectangle(cx1 + 0.5f, cy1 + 0.5f, cx2 - 0.5f, cy2 - 0.5f, radius, radius, t.colors->stroke[k], 1);
			float tx = cx1 + CHIP_PAD + (chip.keeps_firing ? 12 : 0);
			if (chip.kind == ChipKind::cause)
				tx += 2;
			float yy = cy1 + CHIP_PAD;
			if (chip.keeps_firing)
				draw_repeat_mark(cx1 + CHIP_PAD + 6, yy + lh / 2, t.colors->text[k]);
			for (auto const& s : chip.lines)
			{
				draw_text(f, t.colors->text[k], tx, yy, 0, s.c_str());
				yy += lh;
			}
		}
		if (!linksFilter(c))
			al_draw_filled_rounded_rectangle(x, y, x + L.rect.w, y + L.rect.h, 6, 6, fade);
	}

	for (size_t h = 0; h < hubRects.size(); h++)
	{
		auto const& H = hubRects[h];
		float x = H.x + ox, y = H.y + oy;
		auto links_hub = [&](int card) {
			return card >= 0 && std::any_of(graph.cards[card].triggers.begin(), graph.cards[card].triggers.end(),
				[&](auto const& t) {
					return std::any_of(t.links.begin(), t.links.end(), [&](auto const& link) { return link.hub == h; });
				});
		};
		bool hl = int(h) == hoverHub || links_hub(hoverCard) || links_hub(focusCard);
		al_draw_filled_rectangle(x, y, x + H.w, y + H.h, t.colors->state_fill);
		// Inside the box's edge, so lines meeting it aren't covered.
		float border = hl ? 3 : 1, in = border / 2;
		al_draw_rectangle(x + in, y + in, x + H.w - in, y + H.h - in, hl ? accent : t.colors->state_stroke, border);
		float yy = y + 6;
		for (auto const& s : hubLines[h])
		{
			draw_text(f, t.colors->state_text, x + H.w / 2, yy, ALLEGRO_ALIGN_CENTRE, s.c_str());
			yy += lh;
		}
		// The note about the region's other screens, which shows the whole region when clicked.
		if (!hubNotes[h].empty())
		{
			auto const& N = hubNoteRects[h];
			al_draw_line(x + 6, N.y + oy + 0.5f, x + H.w - 6, N.y + oy + 0.5f, t.colors->state_stroke, 1);
			yy = N.y + oy + 4;
			for (auto const& s : hubNotes[h])
			{
				draw_text(f, accent, x + H.w / 2, yy, ALLEGRO_ALIGN_CENTRE, s.c_str());
				yy += lh;
			}
		}
		if (filterHub >= 0 && int(h) != filterHub)
			al_draw_filled_rectangle(x, y, x + H.w, y + H.h, fade);
	}
}

void TriggerDiagram::render()
{
	if (!alDialog || (alDialog->flags & D_HIDDEN))
		return;

	Theme const t = theme();
	ALLEGRO_FONT* f = t.f;
	float lh = t.lh;
	ALLEGRO_COLOR bg = t.bg, fg = t.fg, line = t.line, accent = t.accent;

	clampScroll();
	Rect b = bounds();
	al_draw_filled_rectangle(b.x, b.y, b.x + b.w, b.y + b.h, t.box);

	// The picture and the list, unless the diagram is expanded.
	if (!expanded)
	{
		// The screens, with every combo that has triggers outlined.
		Rect preview = previewRect();
		auto const& v = pictureView;
		// The picture's top-left, at a whole pixel so its pixels stay even.
		float px = std::round(preview.x - v.x * v.zoom), py = std::round(preview.y - v.y * v.zoom);
		al_draw_filled_rectangle(preview.x, preview.y, preview.x + preview.w, preview.y + preview.h, al_map_rgb(0, 0, 0));
		al_set_clipping_rectangle(preview.x, preview.y, preview.w, preview.h);
		if (picture.bmp)
			al_draw_scaled_bitmap(picture.bmp, 0, 0, al_get_bitmap_width(picture.bmp), al_get_bitmap_height(picture.bmp),
				px, py, al_get_bitmap_width(picture.bmp) * v.zoom, al_get_bitmap_height(picture.bmp) * v.zoom, 0);
		// With the picture's top-left at (x0, y0) and its screens sw x sh, dims the screens
		// whose combos aren't cards.
		auto dim_other_screens = [&](float x0, float y0, float sw, float sh) {
			if (!picture.single)
				return;
			for (int row = 0; row < picture.rows; row++)
			{
				for (int col = 0; col < picture.cols; col++)
				{
					if (picture.origin + row * 16 + col == picture.screen)
						continue;
					float x = x0 + col * sw, y = y0 + row * sh;
					al_draw_filled_rectangle(x, y, x + sw, y + sh, al_map_rgba(0, 0, 0, 150));
				}
			}
		};
		if (picture.cols * picture.rows > 1)
		{
			float sw = 256 * v.zoom, sh = 176 * v.zoom;
			dim_other_screens(px, py, sw, sh);
			int col = (picture.screen - picture.origin) % 16, row = (picture.screen - picture.origin) / 16;
			float x = px + col * sw, y = py + row * sh;
			al_draw_rectangle(x + 1, y + 1, x + sw - 1, y + sh - 1, al_map_rgb(0, 0, 0), 3);
			al_draw_rectangle(x + 1, y + 1, x + sw - 1, y + sh - 1, al_map_rgb(255, 255, 255), 1);
		}
		for (size_t c = 0; c < cards.size(); c++)
		{
			bool hl = cardHighlighted(c);
			for (auto const& r : cards[c].placements)
			{
				float x = px + r.x * v.zoom, y = py + r.y * v.zoom, w = r.w * v.zoom, h = r.h * v.zoom;
				if (hl)
				{
					al_draw_rectangle(x + 1, y + 1, x + w - 1, y + h - 1, al_map_rgb(255, 255, 255), 3);
					al_draw_rectangle(x + 1, y + 1, x + w - 1, y + h - 1, accent, 1.5f);
				}
				else if (hoverCard < 0 && hoverHub < 0)
					al_draw_rectangle(x + 0.5f, y + 0.5f, x + w - 0.5f, y + h - 0.5f, al_map_rgba(255, 255, 255, 110), 1);
			}
		}

		// Shade the sides with more of the picture past them, with an arrow pointing that way.
		auto over = pictureOverflow();
		float l = preview.x, tp = preview.y, r = preview.x + preview.w, bt = preview.y + preview.h;
		const int shade = 10;
		for (int i = 0; i < shade; i++)
		{
			ALLEGRO_COLOR c = al_map_rgba(0, 0, 0, 150 * (shade - i) / shade);
			if (over[0])
				al_draw_line(l + i + 0.5f, tp, l + i + 0.5f, bt, c, 1);
			if (over[1])
				al_draw_line(l, tp + i + 0.5f, r, tp + i + 0.5f, c, 1);
			if (over[2])
				al_draw_line(r - i - 0.5f, tp, r - i - 0.5f, bt, c, 1);
			if (over[3])
				al_draw_line(l, bt - i - 0.5f, r, bt - i - 0.5f, c, 1);
		}
		ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);
		float mid_x = std::floor((l + r) / 2) + 0.5f, mid_y = std::floor((tp + bt) / 2) + 0.5f;
		if (over[0])
			draw_arrowhead(l + 1.5f, mid_y, r, mid_y, white);
		if (over[1])
			draw_arrowhead(mid_x, tp + 1.5f, mid_x, bt, white);
		if (over[2])
			draw_arrowhead(r - 1.5f, mid_y, l, mid_y, white);
		if (over[3])
			draw_arrowhead(mid_x, bt - 1.5f, mid_x, tp, white);

		// The map of the whole picture, with what's in view outlined.
		if (Rect m = mapRect(); m.w > 0)
		{
			// Its width and height are rounded apart, so they scale apart.
			auto [pw, ph] = pictureSize();
			float sx = m.w / pw, sy = m.h / ph;
			al_draw_filled_rectangle(m.x - 2, m.y - 2, m.x + m.w + 2, m.y + m.h + 2, al_map_rgb(0, 0, 0));
			al_draw_scaled_bitmap(picture.bmp, 0, 0, al_get_bitmap_width(picture.bmp), al_get_bitmap_height(picture.bmp),
				m.x, m.y, m.w, m.h, 0);
			dim_other_screens(m.x, m.y, 256 * sx, 176 * sy);
			al_draw_rectangle(m.x - 1.5f, m.y - 1.5f, m.x + m.w + 1.5f, m.y + m.h + 1.5f, line, 1);
			float x1 = std::round(m.x + std::max(0.f, v.x) * sx), y1 = std::round(m.y + std::max(0.f, v.y) * sy);
			float x2 = std::round(m.x + std::min(pw, v.x + PREVIEW_W / v.zoom) * sx);
			float y2 = std::round(m.y + std::min(ph, v.y + PREVIEW_H / v.zoom) * sy);
			al_draw_rectangle(x1 + 0.5f, y1 + 0.5f, x2 - 0.5f, y2 - 0.5f, white, 1);
		}

		// How to zoom and pan, until it's been done once. Under the top's shading and arrow.
		if (hoverPreview && !picture_hint_seen)
		{
			const char* hint = "Wheel to zoom, drag to pan";
			float w = al_get_text_width(f, hint) + 8, h = lh + 4;
			float hx = std::round((l + r - w) / 2), hy = tp + 12;
			al_draw_filled_rectangle(hx, hy, hx + w, hy + h, al_map_rgba(0, 0, 0, 190));
			draw_text(f, white, hx + 4, hy + 2, 0, hint);
		}
		al_reset_clipping_rectangle();
		al_draw_rectangle(preview.x - 0.5f, preview.y - 0.5f, preview.x + preview.w + 0.5f, preview.y + preview.h + 0.5f, line, 1);
		// The list of combos.
		Rect list = listRect();
		float rh = list_row_height();
		al_draw_filled_rectangle(list.x, list.y, list.x + list.w, list.y + list.h, bg);
		al_draw_rectangle(list.x - 0.5f, list.y - 0.5f, list.x + list.w + 0.5f, list.y + list.h + 0.5f, line, 1);
		al_set_clipping_rectangle(list.x, list.y, list.w, list.h);
		for (size_t i = 0; i < listRows.size(); i++)
		{
			float y = list.y + i * rh - listScroll;
			if (y + rh < list.y || y > list.y + list.h)
				continue;
			auto [c, indented] = listRows[i];
			if (cardHighlighted(c))
				al_draw_filled_rectangle(list.x, y, list.x + list.w, y + rh, mix(bg, accent, 0.35f));
			float x = list.x + 4;
			if (indented)
			{
				// Later in a chain of combo changes: hangs off the row above.
				al_draw_line(x + 4, y, x + 4, y + rh / 2, line, 1);
				al_draw_line(x + 4, y + rh / 2, x + 11, y + rh / 2, line, 1);
				x += 14;
			}
			if (cards[c].icon)
				draw_bitmap(cards[c].icon, x, y + (rh - 16) / 2);
			x += 20;
			draw_text(f, fg, x, y + (rh - lh) / 2, 0, fit_text(cards[c].title, list.x + list.w - x - 6).c_str());
		}
		if (listRows.size() * rh > list.h)
		{
			float h = list.h * list.h / (listRows.size() * rh), y = list.y + listScroll / (listRows.size() * rh) * list.h;
			al_draw_filled_rectangle(list.x + list.w - 4, y, list.x + list.w - 1, y + h, line);
		}
		al_reset_clipping_rectangle();

		draw_legend(t, list.x, list.y + list.h + 8, false);
		float ty = list.y + list.h + 8 + legend_height() + 6;
		for (auto const* text : HINT)
		{
			draw_text(f, fg, preview.x, ty, 0, text);
			ty += lh;
		}
	}

	Rect view = viewRect();
	al_draw_filled_rectangle(view.x, view.y, view.x + view.w, view.y + view.h, bg);
	al_draw_rectangle(view.x - 0.5f, view.y - 0.5f, view.x + view.w + 0.5f, view.y + view.h + 0.5f, line, 1);
	al_set_clipping_rectangle(view.x, view.y, view.w, view.h);
	float ox = view.x - scrollX, oy = view.y - scrollY;

	if (graph.cards.empty())
	{
		float ty = view.y + view.h / 2 - lh * emptyText.size() / 2;
		for (auto const& text : emptyText)
		{
			draw_text(f, fg, view.x + view.w / 2, ty, ALLEGRO_ALIGN_CENTRE, text.c_str());
			ty += lh;
		}
	}

	drawDiagram(ox, oy, view);

	if (filterHub >= 0)
	{
		auto text = fmt::format("Showing what links to {}. Right-click for Show All.", graph.hubs[filterHub].name());
		float tw = al_get_text_width(f, text.c_str());
		al_draw_filled_rectangle(view.x + 4, view.y + 4, view.x + 12 + tw, view.y + 8 + lh, bg);
		al_draw_rectangle(view.x + 4.5f, view.y + 4.5f, view.x + 11.5f + tw, view.y + 7.5f + lh, accent, 1);
		draw_text(f, fg, view.x + 8, view.y + 6, 0, text.c_str());
	}

	// Fullscreen, the legend has no room on the left, so it goes in the bottom-right corner.
	if (expanded)
		draw_legend(t, view.x + view.w - 8, view.y + view.h - legend_height() - 8, true);

	// Scroll position.
	if (contentW > view.w)
	{
		float w = view.w * view.w / contentW, x = view.x + scrollX / contentW * view.w;
		al_draw_filled_rectangle(x, view.y + view.h - 4, x + w, view.y + view.h - 1, line);
	}
	if (contentH > view.h)
	{
		float h = view.h * view.h / contentH, y = view.y + scrollY / contentH * view.h;
		al_draw_filled_rectangle(view.x + view.w - 4, y, view.x + view.w - 1, y + h, line);
	}
	al_reset_clipping_rectangle();
}

void TriggerDiagram::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if (rti)
		rti->visible = visible;
	if (alDialog)
		alDialog.applyVisibility(visible);
}

void TriggerDiagram::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if (alDialog)
		alDialog.applyDisabled(dis);
}

void TriggerDiagram::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<trigger_diagram_proc>,
		x, y, getWidth(), getHeight(),
		0, 0,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		this, nullptr, nullptr // dp, dp2, dp3
	});
}

int32_t TriggerDiagram::onEvent(int32_t event, MessageDispatcher&)
{
	if (event != geCLICK)
		return -1;
	Action act = action;
	action = Action::none;
	if (act == Action::edit && onClickCard && clickedCid >= 0)
		onClickCard(clickedCid, clickedTrigger);
	else if (act == Action::pick_screen && onPickScreen)
		onPickScreen(pickedScreen);
	else if (act == Action::show_elsewhere && onShowElsewhere && elsewhereHub >= 0)
		onShowElsewhere(elsewhereHub);
	return -1;
}

}
