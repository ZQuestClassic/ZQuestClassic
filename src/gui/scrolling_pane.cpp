#include "gui/scrolling_pane.h"
#include "gui/dialog_runner.h"
#include "base/zc_alleg.h"
#include "gui/jwin.h"
#include <algorithm>

extern int32_t jwin_pal[jcMAX];

#define START_CLIP(d) \
	int clip = screen->clip;\
	int cl = screen->cl;\
	int cr = screen->cr;\
	int ct = screen->ct;\
	int cb = screen->cb;\
	set_clip_rect(screen, d->x+2,d->y+2, d->x+d->w-4, d->y+d->h-4)
#define START_CLIP_BYOB(d) \
	clip = screen->clip;\
	cl = screen->cl;\
	cr = screen->cr;\
	ct = screen->ct;\
	cb = screen->cb;\
	set_clip_rect(screen, d->x+2,d->y+2, d->x+d->w-4, d->y+d->h-4)

#define END_CLIP() \
	screen->clip = clip;\
	screen->cl = cl;\
	screen->cr = cr;\
	screen->ct = ct;\
	screen->cb = cb;

namespace GUI
{

static int32_t minusOneHundred()
{
	return -100;
}

/* When Allegro is searching the array for a mouse handler, mouseBreakerProc
 * checks the mouse position. If the mouse is not in the pane, it replaces
 * gui_mouse_x and gui_mouse_y so that Allegro won't find anything in the pane.
 * The only thing it will find is mouseFixerProc, which restores the old
 * mouse functions.
 */

int32_t ScrollingPane::mouseBreakerProc(int32_t msg, DIALOG* d, int32_t c)
{
	switch (msg)
	{
		case MSG_WANTMOUSE:
		{
			auto* sp = static_cast<ScrollingPane*>(d->dp);
			sp->oldMouseX = gui_mouse_x;
			sp->oldMouseY = gui_mouse_y;
			int32_t mx = gui_mouse_x();
			int32_t my = gui_mouse_y();
			DIALOG& spd = *sp->alDialog;
			if (mx < spd.x || mx >= spd.x + spd.w || my < spd.y || my >= spd.y + spd.h)
			{
				gui_mouse_x = minusOneHundred;
				gui_mouse_y = minusOneHundred;
			}
			return D_DONTWANTMOUSE;
		}
	}

	return D_O_K;
}

int32_t ScrollingPane::mouseFixerProc(int32_t msg, DIALOG* d, int32_t c)
{
	switch (msg)
	{
		case MSG_WANTMOUSE:
		{
			auto* sp = static_cast<ScrollingPane*>(d->dp);
			gui_mouse_x = sp->oldMouseX;
			gui_mouse_y = sp->oldMouseY;
			sp->oldMouseX = nullptr;
			sp->oldMouseY = nullptr;
			return D_DONTWANTMOUSE;
		}
		case MSG_DRAW:
			clear_clip_rect(screen);
			break;
	}
	return D_O_K;
}

static void _calc_scroll_bar(int32_t h, int32_t height, int32_t listsize, int32_t offset,
	int32_t& bh, int32_t& len, int32_t& pos)
{
	bh = vbound((h - 4) / 2, 14, 0);
	int bar_h = h - (bh+2) * 2;
	len = (bar_h * height + listsize / 2) / listsize;
	if (len < 6) len = 6;
	pos = (bar_h - len) * offset / (listsize - height);
}
static void _calc_scroll_bar_backwards(int32_t h, int32_t height, int32_t listsize, int32_t& offset, int32_t pos)
{
	int bh, len, dummy;
	_calc_scroll_bar(h, height, listsize, offset, bh, len, dummy);
	int bar_h = h - (bh+2) * 2;
	offset = pos * (listsize - height) / (bar_h - len);
}
int32_t scrollProc(int32_t msg, DIALOG* d, int32_t c)
{
	static BITMAP* pattern = NULL;
	ScrollingPane* sp = static_cast<ScrollingPane*>(d->dp);
	bool scroll_horz = sp->can_hscroll() && (!sp->can_vscroll() || (key_shifts & KB_SHIFT_FLAG));
	switch (msg)
	{
		case MSG_DRAWCLIPPED:
		{
			auto* child = &sp->alDialog[c];
			START_CLIP(d);
			child->flags |= D_ISCLIPPED;
			child->proc(MSG_DRAW, child, 0);
			child->flags &= ~D_ISCLIPPED;
			END_CLIP();
			return D_O_K;
		}
		case MSG_CHILDFOCUSED:
			if (sp->scrollToShowChild(c))
				return D_REDRAW;
			else
				return D_O_K;

		case MSG_WANTFOCUS:
			if (gui_mouse_b())
				return D_WANTFOCUS | D_REDRAW;
			else return D_O_K;
		case MSG_GOTFOCUS:
		case MSG_LOSTFOCUS:
			return D_O_K;

		case MSG_DRAW:
		{
			if (!pattern)
				pattern = create_bitmap_ex(bitmap_color_depth(screen), 2, 2);
			putpixel(pattern, 0, 1, scheme[jcLIGHT]);
			putpixel(pattern, 1, 0, scheme[jcLIGHT]);
			putpixel(pattern, 0, 0, scheme[jcBOX]);
			putpixel(pattern, 1, 1, scheme[jcBOX]);

			rectfill(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg);
			d->flags &= ~D_GOTFOCUS;

			jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DEEP);
			int tw = d->w, th = d->h;
			int bh, len, pos;
			if (sp->contentHeight > th) // vertical scroll
			{
				tw -= 16;
				int tx = d->x + tw - 2;
				_calc_scroll_bar(th, th, sp->contentHeight, sp->scrollPosY, bh, len, pos);

				draw_arrow_button(screen, tx, d->y + 2, 16, bh, 1, 0);
				draw_arrow_button(screen, tx, d->y + th - 2 - bh, 16, bh, 0, 0);

				if (th > 32)
				{
					int ty = d->y + 16;
					int barh = th - 32;

					drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
					rectfill(screen, tx, ty, tx + 15, ty + barh - 1, 0);
					solid_mode();

					if (th > 32 + 6)
					{
						jwin_draw_button(screen, tx, ty + pos, 16, len, 0, 1);
					}
				}
			}
			if (sp->contentWidth > tw) // horizontal scroll
			{
				th -= 16;
				int ty = d->y + th - 2;
				_calc_scroll_bar(tw, tw, sp->contentWidth, sp->scrollPosX, bh, len, pos);

				draw_arrow_button_horiz(screen, d->x + 2, ty, bh, 16, 1, 0);
				draw_arrow_button_horiz(screen, d->x + tw - 2 - bh, ty, bh, 16, 0, 0);

				if (tw > 32)
				{
					int tx = d->x + 16;
					int barw = tw - 32;

					drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
					rectfill(screen, tx, ty, tx + barw - 1, ty + 15, 0);
					solid_mode();

					if (tw > 32 + 6)
					{
						jwin_draw_button(screen, tx + pos, ty, len, 16, 0, 1);
					}
				}
			}
			if (d->flags & D_GOTFOCUS)
				dotted_rect(screen, d->x + 2, d->y + 2, d->x + tw - 3, d->y + th - 3, scheme[jcTEXTFG], scheme[jcTEXTBG]);
			START_CLIP(d);
			for (size_t i = 1; i < sp->childrenEnd; ++i)
				object_message(&sp->alDialog[i], MSG_DRAW, 0);
			END_CLIP();
			break;
		}
		case MSG_CLICK:
		{
			enum { start_btn, end_btn, bar, start_bar, end_bar };

			int mx = gui_mouse_x(), my = gui_mouse_y();
			int tw = d->w, th = d->h;
			int bh, len, pos, clicked_on, mouse_delay = 0;
			bool down = true, last_draw = false;
			if (sp->contentHeight > th) // vertical scroll
			{
				tw -= 16;
				if (mx >= d->x + tw - 2)
				{
					int tx = d->x + tw - 2;
					int ty = my;
					_calc_scroll_bar(th, th, sp->contentHeight, sp->scrollPosY, bh, len, pos);
					int tpos = sp->scrollPosY;
					int bar_click_pos = my - pos;

					if (my <= d->y + 2 + bh)
					{
						clicked_on = start_btn;
						ty = d->y + 2;
					}
					else if (my >= d->y + th - 2 - bh)
					{
						clicked_on = end_btn;
						ty = d->y + th - 2 - bh;
					}
					else if (th <= 32 + 6)
						clicked_on = bar;
					else if (my < d->y + 2 + bh + pos)
						clicked_on = start_bar;
					else if (my >= d->y + 2 + bh + pos + len)
						clicked_on = end_bar;
					else clicked_on = bar;

					while (gui_mouse_b())
					{
						_calc_scroll_bar(th, th, sp->contentHeight, sp->scrollPosY, bh, len, pos);
						switch (clicked_on)
						{
							case start_btn:
							case end_btn:
							{
								down = mouse_in_rect(tx, ty, 16, bh);
								bool redraw = down != last_draw;

								if (!down)
									mouse_delay = 0;
								else
								{
									if (!(mouse_delay & 1))
									{
										if (clicked_on == start_btn && sp->scrollPosY > 0)
										{
											sp->scroll_y(-1);
											redraw = true;
										}
										else if (clicked_on == end_btn && sp->scrollPosY < sp->maxScrollPosY)
										{
											sp->scroll_y(1);
											redraw = true;
										}
									}
									++mouse_delay;
								}

								if (redraw)
								{
									d->proc(MSG_DRAW, d, 0);
									draw_arrow_button(screen, tx, ty, 16, bh, clicked_on == start_btn, down ? 3 : 0);
									last_draw = down;
								}
								break;
							}
							case start_bar:
							case end_bar:
							{
								if (mouse_in_rect(tx, d->y + 2, 16, th - 4))
								{
									if (clicked_on == start_bar)
									{
										if (gui_mouse_y() < d->y + 2 + bh + pos)
											tpos = sp->scrollPosY - th;
									}
									else
									{
										if (gui_mouse_y() >= d->y + 2 + bh + pos + len)
											tpos = sp->scrollPosY + th;
									}

									tpos = vbound(tpos, sp->maxScrollPosY, 0);

									if (tpos != sp->scrollPosY)
									{
										sp->scroll_y(tpos - sp->scrollPosY);
										d->proc(MSG_DRAW, d, 0);
										_calc_scroll_bar(th, th, sp->contentHeight, sp->scrollPosY, bh, len, pos);
									}
								}

								if (mouse_in_rect(tx, d->y + 2 + bh + pos, 16, len)) // on center bar
								{
									clicked_on = bar;
									my = gui_mouse_y();
									bar_click_pos = my - pos;
									continue;
								}
								break;
							}
							case bar:
							default:
							{
								int tmph = th - 32;
								//tpos = (sp->contentHeight * (gui_mouse_y() - bar_click_pos) + tmph / 2) / tmph;
								_calc_scroll_bar_backwards(th, th, sp->contentHeight, tpos, gui_mouse_y() - bar_click_pos);
								tpos = vbound(tpos, sp->maxScrollPosY, 0);

								if (tpos != sp->scrollPosY)
								{
									sp->scroll_y(tpos - sp->scrollPosY);
									tpos = sp->scrollPosY;
									d->proc(MSG_DRAW, d, 0);
								}
								break;
							}
						}

						broadcast_dialog_message(MSG_IDLE, 0);
						update_hw_screen();
					}
					if (sp->scrollptr_y)
						*(sp->scrollptr_y) = sp->scrollPosY;
					return D_REDRAW;
				}
			}
			if (sp->contentWidth > tw) // horizontal scroll
			{
				th -= 16;
				if (my >= d->y + th - 2)
				{
					int ty = d->y + th - 2;
					int tx = mx;
					_calc_scroll_bar(tw, tw, sp->contentWidth, sp->scrollPosX, bh, len, pos);
					int tpos = sp->scrollPosX;
					int bar_click_pos = mx - pos;

					if (mx <= d->x + 2 + bh)
					{
						clicked_on = start_btn;
						tx = d->x + 2;
					}
					else if (mx >= d->x + tw - 2 - bh)
					{
						clicked_on = end_btn;
						tx = d->x + tw - 2 - bh;
					}
					else if (tw <= 32 + 6)
						clicked_on = bar;
					else if (mx < d->x + 2 + bh + pos)
						clicked_on = start_bar;
					else if (mx >= d->x + 2 + bh + pos + len)
						clicked_on = end_bar;
					else clicked_on = bar;

					while (gui_mouse_b())
					{
						_calc_scroll_bar(tw, tw, sp->contentWidth, sp->scrollPosX, bh, len, pos);
						switch (clicked_on)
						{
							case start_btn:
							case end_btn:
							{
								down = mouse_in_rect(tx, ty, 16, bh);
								bool redraw = down != last_draw;

								if (!down)
									mouse_delay = 0;
								else
								{
									if (!(mouse_delay & 1))
									{
										if (clicked_on == start_btn && sp->scrollPosX > 0)
										{
											sp->scroll_x(-1);
											redraw = true;
										}
										else if (clicked_on == end_btn && sp->scrollPosX < sp->maxScrollPosX)
										{
											sp->scroll_x(1);
											redraw = true;
										}
									}
									++mouse_delay;
								}

								if (redraw)
								{
									d->proc(MSG_DRAW, d, 0);
									draw_arrow_button_horiz(screen, tx, ty, bh, 16, clicked_on == start_btn, down ? 3 : 0);
									last_draw = down;
								}
								break;
							}
							case start_bar:
							case end_bar:
							{
								if (mouse_in_rect(d->x + 2, ty, tw - 4, 16))
								{
									if (clicked_on == start_bar)
									{
										if (gui_mouse_x() < d->x + 2 + bh + pos)
											tpos = sp->scrollPosX - tw;
									}
									else
									{
										if (gui_mouse_x() >= d->x + 2 + bh + pos + len)
											tpos = sp->scrollPosX + tw;
									}

									tpos = vbound(tpos, sp->maxScrollPosX, 0);

									if (tpos != sp->scrollPosX)
									{
										sp->scroll_x(tpos - sp->scrollPosX);
										d->proc(MSG_DRAW, d, 0);
										_calc_scroll_bar(tw, tw, sp->contentWidth, sp->scrollPosX, bh, len, pos);
									}
								}

								if (mouse_in_rect(d->x + 2 + bh + pos, ty, 16, len)) // on center bar
								{
									clicked_on = bar;
									mx = gui_mouse_x();
									bar_click_pos = mx - pos;
									continue;
								}
								break;
							}
							case bar:
							default:
							{
								int tmpw = tw - 32;
								//tpos = (sp->contentWidth * (gui_mouse_x() - bar_click_pos) + tmpw / 2) / tmpw;
								_calc_scroll_bar_backwards(tw, tw, sp->contentWidth, tpos, gui_mouse_x() - bar_click_pos);
								tpos = vbound(tpos, sp->maxScrollPosX, 0);

								if (tpos != sp->scrollPosX)
								{
									sp->scroll_x(tpos - sp->scrollPosX);
									tpos = sp->scrollPosX;
									d->proc(MSG_DRAW, d, 0);
								}
								break;
							}
						}

						broadcast_dialog_message(MSG_IDLE, 0);
						update_hw_screen();
					}
					if (sp->scrollptr_x)
						*(sp->scrollptr_x) = sp->scrollPosX;
					return D_REDRAW;
				}
			}
			return D_O_K;
		}

		case MSG_WHEEL:
			if (scroll_horz)
				sp->scroll_x(-8 * c);
			else sp->scroll_y(-8 * c);
			return D_REDRAW;

		case MSG_XCHAR:
			if (scroll_horz)
			{
				switch (c >> 8)
				{
					case KEY_PGDN:
					{
						if (sp->maxScrollPosX < d->w * 2)
							sp->scroll_x(d->w / 3);
						else sp->scroll_x(d->w);
						return D_USED_CHAR | D_REDRAW;
					}
					case KEY_PGUP:
					{
						if (sp->maxScrollPosX < d->w * 2)
							sp->scroll_x(-d->w / 3);
						else sp->scroll_x(-d->w);
						return D_USED_CHAR | D_REDRAW;
					}
					case KEY_HOME:
					{
						sp->scroll_x(-sp->maxScrollPosX);
						return D_USED_CHAR | D_REDRAW;
					}
					case KEY_END:
					{
						sp->scroll_x(sp->maxScrollPosX);
						return D_USED_CHAR | D_REDRAW;
					}
				}
			}
			else
			{
				switch (c >> 8)
				{
					case KEY_PGDN:
					{
						if (sp->maxScrollPosY < d->h * 2)
							sp->scroll_y(d->h / 3);
						else sp->scroll_y(d->h);
						return D_USED_CHAR | D_REDRAW;
					}
					case KEY_PGUP:
					{
						if (sp->maxScrollPosY < d->h * 2)
							sp->scroll_y(-d->h / 3);
						else sp->scroll_y(-d->h);
						return D_USED_CHAR | D_REDRAW;
					}
					case KEY_HOME:
					{
						sp->scroll_y(-sp->maxScrollPosY);
						return D_USED_CHAR | D_REDRAW;
					}
					case KEY_END:
					{
						sp->scroll_y(sp->maxScrollPosY);
						return D_USED_CHAR | D_REDRAW;
					}
				}
			}
			break;
	}

	return D_O_K;
}

ScrollingPane::ScrollingPane() : childrenEnd(0), scrollPosX(0), scrollPosY(0),
	maxScrollPosX(0), maxScrollPosY(0), contentWidth(0), contentHeight(0),
	oldMouseX(nullptr), oldMouseY(nullptr), scrollptr_x(nullptr), scrollptr_y(nullptr),
	targWid(0_px), targHei(0_px), mode(Mode::SCROLL_V)
{
	bgColor = jwin_pal[jcBOX];
}

bool ScrollingPane::can_hscroll() const noexcept
{
	return mode == Mode::SCROLL_H || mode == Mode::SCROLL_HV;
}
bool ScrollingPane::can_vscroll() const noexcept
{
	return mode == Mode::SCROLL_V || mode == Mode::SCROLL_HV;
}

void ScrollingPane::scroll_x(int32_t amount) noexcept
{
	if (!can_hscroll() || maxScrollPosX < 0) return; //No scrolling
	int32_t newPos = std::clamp(scrollPosX + amount, 0, maxScrollPosX);
	amount = newPos - scrollPosX;
	scrollPosX = newPos;
	if (scrollptr_x && alDialog) *scrollptr_x = scrollPosX;
	for (size_t i = 1; i < childrenEnd; ++i)
		alDialog[i].x -= amount;
}
void ScrollingPane::scroll_y(int32_t amount) noexcept
{
	if (!can_vscroll() || maxScrollPosY < 0) return; //No scrolling
	int32_t newPos = std::clamp(scrollPosY + amount, 0, maxScrollPosY);
	amount = newPos - scrollPosY;
	scrollPosY = newPos;
	if (scrollptr_y && alDialog) *scrollptr_y = scrollPosY;
	for (size_t i = 1; i < childrenEnd; ++i)
		alDialog[i].y -= amount;
}

bool ScrollingPane::scrollToShowChild(int32_t childPos)
{
	DIALOG& pane = *alDialog;
	DIALOG& child = alDialog[childPos];
	bool ret = false;
	if (can_vscroll())
	{
		if (child.y < pane.y)
		{
			scroll_y(child.y - pane.y);
			ret = true;
		}
		else if (child.y + child.h > pane.y + pane.h)
		{
			scroll_y((child.y + child.h) - (pane.y + pane.h));
			ret = true;
		}
	}
	
	if (can_hscroll())
	{
		if (child.x < pane.x)
		{
			scroll_x(child.x - pane.x);
			ret = true;
		}
		else if (child.x + child.w > pane.x + pane.w)
		{
			scroll_x((child.x + child.w) - (pane.x + pane.w));
			ret = true;
		}
	}
	
	return ret;
}

void ScrollingPane::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if (alDialog) alDialog.applyVisibility(visible);
	if (content)
	{
		if (screen)
		{
			int clip, cl, cr, ct, cb;
			if (isConstructed()) START_CLIP_BYOB(alDialog);
			content->applyVisibility(visible);
			if (isConstructed()) END_CLIP();
		}
		else content->applyVisibility(visible);
	}
}

void ScrollingPane::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if (alDialog) alDialog.applyDisabled(dis);
	if (content)
	{
		START_CLIP(alDialog);
		content->applyDisabled(dis);
		END_CLIP();
	}
}

void ScrollingPane::setPtrX(int32_t* ptr)
{
	scrollptr_x = ptr;
	if (ptr && alDialog)
		scroll_x(*ptr - scrollPosX);
}
void ScrollingPane::setPtrY(int32_t* ptr)
{
	scrollptr_y = ptr;
	if (ptr && alDialog)
		scroll_y(*ptr - scrollPosY);
}

void ScrollingPane::calculateSize()
{
	setPreferredWidth(25_em);
	setPreferredHeight(10_em);
	growToTarget();
	Widget::calculateSize();
}

void ScrollingPane::growToTarget()
{
	if (!content)
	{
		if (can_vscroll() && targHei > 0 && targHei > getHeight())
			setPreferredHeight(targHei);
		if (can_hscroll() && targWid > 0 && targWid > getWidth())
			setPreferredWidth(targWid);
		return;
	}
	content->calculateSize();
	contentWidth = content->getTotalWidth();
	contentHeight = content->getTotalHeight();
	
	if (!can_hscroll()) // v only; set the width, including scrollbar
		setPreferredWidth(Size::pixels(contentWidth) + 4_em);
	else if (!can_vscroll()) // h only; set the height, including scrollbar
		setPreferredHeight(Size::pixels(contentHeight) + 4_em);
	
	if (can_vscroll())
	{
		auto hei = getHeight();
		if (hei > contentHeight || targHei > contentHeight)
			setPreferredHeight(Size::pixels(contentHeight));
		else if (targHei > 0 && targHei > hei)
			setPreferredHeight(targHei);
	}
	if (can_hscroll())
	{
		auto wid = getWidth();
		if (wid > contentWidth || targWid > contentWidth)
			setPreferredWidth(Size::pixels(contentWidth));
		else if (targWid > 0 && targWid > wid)
			setPreferredWidth(targWid);
	}

	maxScrollPosX = can_hscroll() ? contentWidth - getWidth() : 0;
	maxScrollPosY = can_vscroll() ? contentHeight - getHeight() : 0;
}

void ScrollingPane::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	// We want to be about as big as possible...
	setPreferredWidth(Size::pixels(contW));
	setPreferredHeight(Size::pixels(contH));

	growToTarget();
	Widget::arrange(contX, contY, contW, contH);
	if (content)
	{
		int wid = getWidth(), hei = getHeight();
		contentWidth = content->getTotalWidth();
		contentHeight = content->getTotalHeight();
		switch (mode)
		{
			case Mode::SCROLL_V:
				if (getHeight() < contentHeight)
					wid -= 18;
				if (contentHeight > hei)
					hei = contentHeight;
				break;
			case Mode::SCROLL_H:
				if (getWidth() < contentWidth)
					hei -= 18;
				if (contentWidth > wid)
					wid = contentWidth;
				break;
			case Mode::SCROLL_HV:
				if (contentHeight > hei)
					hei = contentHeight;
				if (contentWidth > wid)
					wid = contentWidth;
				break;
			
		}
		content->arrange(x, y, wid, hei);
		contentWidth = content->getTotalWidth();
		contentHeight = content->getTotalHeight();
		maxScrollPosX = can_hscroll() ? contentWidth - getWidth() : 0;
		maxScrollPosY = can_vscroll() ? contentHeight - getHeight() : 0;
	}
}

void ScrollingPane::realize(DialogRunner& runner)
{
	oldMouseX = gui_mouse_x;
	oldMouseY = gui_mouse_y;
	/*
	runner.push(shared_from_this(), DIALOG {
		jwin_frame_proc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		3, 0, // d1, d2
		nullptr, nullptr, nullptr // dp, dp2, dp3
	});
	*/
	runner.push(shared_from_this(), DIALOG{
		mouseBreakerProc,
		0, 0, 2000, 2000, // As int32_t as it covers the screen
		0, 0,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		this, nullptr, nullptr // dp, dp2, dp3
	});
	alDialog = runner.push(shared_from_this(), DIALOG{
		scrollProc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		this, nullptr, nullptr // dp, dp2, dp3
	});

	size_t sizeBefore = runner.size();
	content->realize(runner);
	childrenEnd = runner.size() + 1 - sizeBefore;

	// Every child needs D_SCROLLING set so its proc knows to use
	// some special event handling.
	for (size_t i = 1; i < childrenEnd; i++)
		alDialog[i].flags |= D_SCROLLING;

	runner.push(shared_from_this(), DIALOG{
		mouseFixerProc,
		-100, -100, 1, 1, // Exactly where the replacement functions point
		0, 0,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		this, nullptr, nullptr // dp, dp2, dp3
	});

	if (scrollptr_x)
		scroll_x(*scrollptr_x - scrollPosX);
	if (scrollptr_y)
		scroll_y(*scrollptr_y - scrollPosY);
}

}
