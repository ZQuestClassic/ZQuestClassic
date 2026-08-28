#include "zq/gui/quest_list_view.h"
#include "core/fonts.h"
#include "gui/common.h"
#include "gui/dialog_runner.h"
#include "zq/zquest.h"
#include "gui/size.h"
#include "gui/jwin.h"
#include "zalleg/render.h"
#include <algorithm>
#include <allegro5/allegro.h>

using namespace GUI;

// Row layout: [4px pad][32px icon][12px gap][text columns][right-aligned info][scrollbar]
static constexpr int32_t ICON_PAD = 5;
static constexpr int32_t ICON_DRAW = 32;
static constexpr int32_t TEXT_X = 4 + ICON_DRAW + 12;
static constexpr int32_t RIGHT_COL_W = 200;

static int32_t row_area_width(DIALOG* d, int32_t listsize, int32_t visrows)
{
	bool bar = listsize > visrows;
	return d->w - 4 - (bar ? 18 : 0);
}

// Bounds d1 and scrolls d2 so the selection is visible.
static void clamp_list_pos(DIALOG* d, int listsize, int visrows)
{
	d->d1 = std::clamp(d->d1, 0, std::max(0, listsize - 1));
	if (d->d1 < d->d2)
		d->d2 = d->d1;
	if (d->d1 >= d->d2 + visrows)
		d->d2 = d->d1 - visrows + 1;
	d->d2 = std::clamp(d->d2, 0, std::max(0, listsize - visrows));
}

// Bounds d1/d2 without moving the view - background row updates must not
// yank the scroll position back to the selection.
static void clamp_list_range(DIALOG* d, int listsize, int visrows)
{
	d->d1 = std::clamp(d->d1, 0, std::max(0, listsize - 1));
	d->d2 = std::clamp(d->d2, 0, std::max(0, listsize - visrows));
}

// Most of this proc is generic listbox mechanics (scrolling, click/drag
// selection, keyboard, wheel) re-implemented because jwin's list procs
// hardcode row height == text height. If another owner-drawn list widget is
// ever needed, consider extracting a shared proc parameterized by row height
// and a row-draw callback, leaving widgets like this one with only the
// MSG_DRAW row rendering.
int32_t new_quest_list_proc(int32_t msg, DIALOG* d, int32_t c)
{
	QuestListView* widg = (QuestListView*)d->dp3;
	if (!widg)
		return D_O_K;

	auto const& rows = widg->getRows();
	int32_t listsize = rows.size();
	int32_t rowh = widg->rowHeight();
	int32_t visrows = widg->visibleRowCount();

	switch(msg)
	{
	case MSG_START:
		if (!widg->rti)
		{
			widg->rti = add_dlg_layer();
			// Icons are true-color, so they render on this a5 layer. The DIALOG pointer
			// is stable while the dialog runs, which is also this layer's lifetime.
			widg->rti->render_cb = [widg, d](RenderTreeItem*, bool) {
				auto const& rows = widg->getRows();
				int32_t listsize = rows.size();
				int32_t rowh = widg->rowHeight();
				int32_t visrows = widg->visibleRowCount();
				int32_t rowW = row_area_width(d, listsize, visrows);
				int32_t x = d->x + 2;
				al_set_clipping_rectangle(x, d->y + 2, rowW, d->h - 4);
				for (int32_t i = d->d2; i < std::min(d->d2 + visrows, listsize); ++i)
				{
					int32_t y = d->y + 2 + (i - d->d2) * rowh;
					if (ALLEGRO_BITMAP* icon = widg->getIconBitmap(i))
						al_draw_scaled_bitmap(icon, 0, 0, 16, 16, x + 4, y + (rowh - ICON_DRAW) / 2, ICON_DRAW, ICON_DRAW, 0);
				}
				al_reset_clipping_rectangle();
			};
		}
		clamp_list_pos(d, listsize, visrows);
		break;

	case MSG_END:
		if (widg->rti)
		{
			remove_dlg_layer(widg->rti);
			widg->rti = nullptr;
		}
		break;

	case MSG_DRAW:
	{
		if (d->flags & D_HIDDEN)
			break;

		FONT* titlefont = widg->titleFont();
		FONT* subfont = widg->subFont();
		int32_t rowW = row_area_width(d, listsize, visrows);

		_jwin_draw_scrollable_frame(d, listsize, d->d2, visrows, 0);

		int32_t x = d->x + 2;
		for(int32_t i = d->d2; i < d->d2 + visrows; ++i)
		{
			int32_t y = d->y + 2 + (i - d->d2) * rowh;
			int32_t maxy = d->y + d->h - 3;
			int32_t y2 = std::min(y + rowh - 1, maxy);
			if (y > maxy)
				break;

			if (i >= listsize)
			{
				rectfill(screen, x, y, x + rowW - 1, y2, jwin_pal[jcTEXTBG]);
				continue;
			}

			bool sel_row = (i == d->d1) && !(d->flags & D_DISABLED);
			int32_t bg = jwin_pal[sel_row ? jcSELBG : jcTEXTBG];
			int32_t fg = jwin_pal[sel_row ? jcSELFG : jcTEXTFG];
			rectfill(screen, x, y, x + rowW - 1, y2, bg);

			// Placeholder when no icon has been baked yet: a sunken slot
			// with a centered "?" glyph.
			int32_t ix = x + 4, iy = y + (rowh - ICON_DRAW) / 2;
			if (!widg->getIconBitmap(i))
			{
				rectfill(screen, ix, iy, ix + ICON_DRAW - 1, iy + ICON_DRAW - 1, jwin_pal[jcMEDLT]);
				jwin_draw_frame(screen, ix, iy, ICON_DRAW, ICON_DRAW, FR_DEEP);
				textout_centre_ex(screen, titlefont, "?", ix + ICON_DRAW / 2,
					iy + (ICON_DRAW - text_height(titlefont)) / 2, jwin_pal[jcMEDDARK], -1);
			}

			auto const& row = rows[i];
			int32_t th_title = text_height(titlefont);
			int32_t th_sub = text_height(subfont);
			int32_t line1 = y + 8;
			int32_t line2 = line1 + th_title * 2 + 4;
			int32_t right_x = x + rowW - 12;

			// Title at double height: a4 fonts have no size variants, so
			// render at native size and stretch 2x.
			int32_t avail = rowW - TEXT_X - RIGHT_COL_W;
			if (avail > 8)
			{
				int32_t tw = std::min(text_length(titlefont, row.title.c_str()) + 1, avail / 2);
				BITMAP* tmp = create_bitmap_ex(8, tw, th_title);
				if (tmp)
				{
					clear_to_color(tmp, bg);
					textout_ex(tmp, titlefont, row.title.c_str(), 0, 0, fg, bg);
					stretch_blit(tmp, screen, 0, 0, tw, th_title,
						x + TEXT_X, line1, tw * 2, th_title * 2);
					destroy_bitmap(tmp);
				}
			}

			set_clip_rect(screen, x + TEXT_X, y, x + rowW - RIGHT_COL_W, y2);
			if (!row.byline.empty())
				textout_ex(screen, subfont, row.byline.c_str(), x + TEXT_X, line2, fg, -1);
			// Right-aligned column; ellipsize anything still too wide
			// rather than letting the clip eat its left edge.
			int32_t colw = RIGHT_COL_W - 16;
			auto fit_text = [&](std::string s) {
				if (text_length(subfont, s.c_str()) <= colw)
					return s;
				while (s.size() > 1 && text_length(subfont, (s + "..").c_str()) > colw)
					s.pop_back();
				return s + "..";
			};
			set_clip_rect(screen, x + rowW - RIGHT_COL_W + 4, y, right_x, y2);
			textout_right_ex(screen, subfont, fit_text(row.date).c_str(), right_x, line1, fg, -1);
			textout_right_ex(screen, subfont, fit_text(row.version).c_str(), right_x, line1 + th_sub + 3, fg, -1);
			set_clip_rect(screen, 0, 0, screen->w - 1, screen->h - 1);
		}

		if (!listsize)
		{
			textout_centre_ex(screen, subfont, widg->getEmptyText().c_str(),
				d->x + d->w / 2, d->y + d->h / 2 - text_height(subfont) / 2,
				jwin_pal[jcBOXFG], -1);
		}

		if (widg->rti)
			widg->rti->dirty = true;
		break;
	}

	case MSG_CLICK:
	{
		if (listsize > visrows && gui_mouse_x() >= d->x + d->w - 18)
		{
			_handle_jwin_scrollable_scroll_click(d, listsize, &d->d2, visrows);
			break;
		}

		if (!listsize)
			break;

		bool changed_any = false;
		while (gui_mouse_b())
		{
			int32_t my = gui_mouse_y();
			int32_t row = d->d2 + (my - d->y - 2) / rowh;

			// Autoscroll when dragging past the edges.
			if (my < d->y + 2 && d->d2 > 0)
				row = --d->d2;
			else if (my >= d->y + 2 + visrows * rowh && d->d2 < listsize - visrows)
				row = ++d->d2 + visrows - 1;

			row = std::clamp(row, 0, listsize - 1);
			if (row != d->d1)
			{
				d->d1 = row;
				clamp_list_pos(d, listsize, visrows);
				changed_any = true;
				GUI_EVENT(d, geCHANGE_SELECTION);
				broadcast_dialog_message(MSG_DRAW, 0);
				update_hw_screen();
			}
			broadcast_dialog_message(MSG_IDLE, 0);
		}

		if (changed_any)
			return D_REDRAWME;
		break;
	}

	case MSG_DCLICK:
	{
		if (gui_mouse_b() & 2)
			break;
		if (listsize > visrows && gui_mouse_x() >= d->x + d->w - 18)
			break;
		if (!listsize)
			break;

		int32_t i = d->d1;
		object_message(d, MSG_CLICK, 0);
		if (i == d->d1)
			GUI_EVENT(d, geDCLICK);
		break;
	}

	case MSG_WANTFOCUS:
		return D_WANTFOCUS;

	case MSG_WANTWHEEL:
		return 1;

	case MSG_WHEEL:
	{
		if (listsize <= visrows)
			break;
		int32_t delta = (c > 0) ? -2 : 2;
		int32_t d2 = std::clamp(d->d2 + delta, 0, listsize - visrows);
		if (d2 != d->d2)
		{
			d->d2 = d2;
			object_message(d, MSG_DRAW, 0);
			return D_REDRAWME;
		}
		break;
	}

	case MSG_CHAR:
	{
		if (!listsize)
			break;

		int32_t orig = d->d1;
		switch(c >> 8)
		{
		case KEY_UP: d->d1--; break;
		case KEY_DOWN: d->d1++; break;
		case KEY_HOME: d->d1 = 0; break;
		case KEY_END: d->d1 = listsize - 1; break;
		case KEY_PGUP: d->d1 -= visrows - 1; break;
		case KEY_PGDN: d->d1 += visrows - 1; break;
		case KEY_ENTER:
		case KEY_ENTER_PAD:
			GUI_EVENT(d, geDCLICK);
			return D_USED_CHAR;
		default:
			return D_O_K;
		}

		clamp_list_pos(d, listsize, visrows);
		if (d->d1 != orig)
		{
			GUI_EVENT(d, geCHANGE_SELECTION);
			d->flags |= D_DIRTY;
		}
		return D_USED_CHAR;
	}

	case MSG_IDLE:
		// Don't run scan work in the nested mouse-drag loops.
		if (!gui_mouse_b() && widg->runIdle())
			d->flags |= D_DIRTY;
		break;
	}

	return D_O_K;
}

namespace GUI
{

QuestListView::QuestListView():
	rti(nullptr), selectedIndex(0), message(-1), msg_d(-1)
{
	// Fill most of the program window, with a comfortable gap between the
	// dialog window and the application window.
	int32_t sw = screen ? screen->w : 640;
	int32_t sh = screen ? screen->h : 480;
	setPreferredWidth(Size::pixels(sw - 130));
	// Leaves room for the selected-quest path label and footer below the
	// list, snapped down to whole rows so no partial-row gap shows at the
	// bottom (+4 for the frame).
	int32_t avail = sh - 250;
	int32_t visrows = std::max(1, (avail - 4) / rowHeight());
	setPreferredHeight(Size::pixels(visrows * rowHeight() + 4));
	fgColor = jwin_pal[jcTEXTFG];
	bgColor = jwin_pal[jcTEXTBG];
}

QuestListView::~QuestListView()
{
	destroyIconBitmaps();
}

FONT* QuestListView::titleFont() const
{
	return get_zc_font(font_lfont_l);
}

FONT* QuestListView::subFont() const
{
	return widgFont ? widgFont : font;
}

int32_t QuestListView::rowHeight() const
{
	// Title is drawn at 2x height; byline below it.
	int32_t text_h = 8 + 2 * text_height(titleFont()) + 4 + text_height(subFont()) + 8;
	return std::max(ICON_DRAW + 2 * ICON_PAD, text_h);
}

int32_t QuestListView::visibleRowCount() const
{
	int32_t h = alDialog ? alDialog->h : getHeight();
	return std::max(1, (h - 4) / rowHeight());
}

void QuestListView::getVisibleRows(int32_t& start, int32_t& end) const
{
	int32_t scroll = alDialog ? alDialog->d2 : 0;
	start = std::clamp(scroll, 0, (int32_t)rows.size());
	end = std::clamp(scroll + visibleRowCount(), start, (int32_t)rows.size());
}

void QuestListView::destroyIconBitmaps()
{
	for(auto* bmp : iconBitmaps)
		if (bmp)
			al_destroy_bitmap(bmp);
	iconBitmaps.clear();
}

void QuestListView::setRows(std::vector<QuestListRow> newRows)
{
	destroyIconBitmaps();
	rows = std::move(newRows);
	iconBitmaps.assign(rows.size(), nullptr);
	if (alDialog)
	{
		clamp_list_range(alDialog.operator->(), rows.size(), visibleRowCount());
		pendDraw();
	}
}

int32_t QuestListView::getSelectedIndex() const
{
	if (alDialog)
		return alDialog->d1;
	return selectedIndex;
}

void QuestListView::setSelectedIndex(int32_t index, bool scroll_into_view)
{
	selectedIndex = index;
	if (alDialog)
	{
		alDialog->d1 = index;
		if (scroll_into_view)
			clamp_list_pos(alDialog.operator->(), rows.size(), visibleRowCount());
		else
			clamp_list_range(alDialog.operator->(), rows.size(), visibleRowCount());
		pendDraw();
	}
}

QuestListRow const* QuestListView::getSelectedRow() const
{
	if (rows.empty())
		return nullptr;
	int32_t i = std::clamp(getSelectedIndex(), 0, (int32_t)rows.size() - 1);
	return &rows[i];
}

ALLEGRO_BITMAP* QuestListView::getIconBitmap(int32_t index)
{
	if (index < 0 || index >= (int32_t)rows.size())
		return nullptr;
	if (iconBitmaps[index])
		return iconBitmaps[index];
	auto const& rgba = rows[index].icon_rgba;
	if (rgba.size() != 16 * 16 * 4)
		return nullptr;

	int32_t old_flags = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ALLEGRO_BITMAP* bmp = al_create_bitmap(16, 16);
	al_set_new_bitmap_flags(old_flags);
	if (!bmp)
		return nullptr;

	ALLEGRO_LOCKED_REGION* lr = al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_WRITEONLY);
	if (!lr)
	{
		al_destroy_bitmap(bmp);
		return nullptr;
	}
	for(int32_t y = 0; y < 16; ++y)
		memcpy((uint8_t*)lr->data + y * lr->pitch, rgba.data() + y * 16 * 4, 16 * 4);
	al_unlock_bitmap(bmp);

	iconBitmaps[index] = bmp;
	return bmp;
}

bool QuestListView::runIdle()
{
	if (onIdleFunc)
		return onIdleFunc();
	return false;
}

void QuestListView::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if (rti)
		rti->visible = visible;
	if (alDialog) alDialog.applyVisibility(visible);
}

void QuestListView::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if (alDialog) alDialog.applyDisabled(dis);
}

void QuestListView::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<new_quest_list_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		selectedIndex, 0, // d1, d2
		nullptr, widgFont, this // dp, dp2, dp3
	});
}

int32_t QuestListView::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	int32_t index = alDialog ? alDialog->d1 : selectedIndex;
	switch(event)
	{
	case geCHANGE_SELECTION:
		if (onSelectFunc)
			onSelectFunc(index);
		if (message >= 0)
			sendMessage(message, index);
		break;
	case geDCLICK:
		if (onDClickFunc)
			onDClickFunc(index);
		if (msg_d >= 0)
			sendMessage(msg_d, index);
		break;
	}
	return -1;
}

}
