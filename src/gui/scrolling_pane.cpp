#include "scrolling_pane.h"
#include "dialog_runner.h"
#include "../zc_alleg.h"
#include "../jwin.h"
#include "../zquest.h"
#include <algorithm>

extern int jwin_pal[jcMAX];

int screen_w, screen_h;
#define START_CLIP(d) set_clip_rect( \
	gui_get_screen(), d->x+2,d->y+2, d->x+d->w-4, d->y+d->h-4)
#define END_CLIP() set_clip_rect( \
	gui_get_screen(), 0, 0, 800, 600)

namespace GUI
{

static int minusOneHundred()
{
	return -100;
}

/* When Allegro is searching the array for a mouse handler, mouseBreakerProc
 * checks the mouse position. If the mouse is not in the pane, it replaces
 * gui_mouse_x and gui_mouse_y so that Allegro won't find anything in the pane.
 * The only thing it will find is mouseFixerProc, which restores the old
 * mouse functions.
 */

int ScrollingPane::mouseBreakerProc(int msg, DIALOG* d, int c)
{
	switch(msg)
	{
	case MSG_WANTMOUSE:
	{
		auto* sp=static_cast<ScrollingPane*>(d->dp);
		sp->oldMouseX=gui_mouse_x;
		sp->oldMouseY=gui_mouse_y;
		int mx=gui_mouse_x();
		int my=gui_mouse_y();
		DIALOG& spd=*sp->alDialog;
		if(mx<spd.x || mx>=spd.x+spd.w || my<spd.y || my>=spd.y+spd.h)
		{
			gui_mouse_x=minusOneHundred;
			gui_mouse_y=minusOneHundred;
		}
		return D_DONTWANTMOUSE;
	}
	}

	return D_O_K;
}

int ScrollingPane::mouseFixerProc(int msg, DIALOG* d, int c)
{
	switch(msg)
	{
	case MSG_WANTMOUSE:
	{
		auto* sp=static_cast<ScrollingPane*>(d->dp);
		gui_mouse_x=sp->oldMouseX;
		gui_mouse_y=sp->oldMouseY;
		sp->oldMouseX=nullptr;
		sp->oldMouseY=nullptr;
		return D_DONTWANTMOUSE;
	}
	case MSG_DRAW:
		set_clip_rect(gui_get_screen(), 0, 0, 800, 600);
		break;
	}
	return D_O_K;
}

int scrollProc(int msg, DIALOG* d, int c)
{
	switch(msg)
	{
	case MSG_DRAWCLIPPED:
	{
		// This message is sent by a widget inside the pane when redrawing
		// after MSG_GOTFOCUS or MSG_LOSTFOCUS.
		auto* sp=static_cast<ScrollingPane*>(d->dp);
		auto* child=&sp->alDialog[c];
		child->flags &= ~D_NEEDSCLIPPED; // Do this first or it'll recurse
		START_CLIP(d);
		child->proc(MSG_DRAW, child, 0);
		set_clip_rect(gui_get_screen(), 0, 0, 800, 600);
		return D_O_K;
	}
	case MSG_CHILDFOCUSED:
		if(static_cast<ScrollingPane*>(d->dp)->scrollToShowChild(c))
			return D_REDRAW;
		else
			return D_O_K;

	case MSG_DRAW:
	{
		auto* sp = static_cast<ScrollingPane*>(d->dp);
		rectfill(gui_get_screen(), d->x, d->y, d->x+d->w-1, d->y+d->h-1, d->bg);
		_jwin_draw_scrollable_frame(d, sp->contentHeight, sp->scrollPos, d->h, 0);
		START_CLIP(d);
		if(d->d1)
		{
			// The scrollbar is being dragged; we need to scroll and redraw
			// everything in the pane.
			int scrollAmount=d->d2-sp->scrollPos;
			d->d2=sp->scrollPos;
			for(size_t i = 1; i < sp->childrenEnd; ++i)
			{
				DIALOG* child=&sp->alDialog[i];
				child->y += scrollAmount;
				object_message(child, MSG_DRAW, 0);
			}
			END_CLIP();
		}
		break;
	}
	case MSG_CLICK:
	{
		auto* sp=static_cast<ScrollingPane*>(d->dp);
		if(gui_mouse_x() >= d->x+d->w-18)
		{
			// This emits MSG_DRAW as it scrolls
			d->d1=1;
			d->d2=sp->scrollPos;
			_handle_jwin_scrollable_scroll_click(d, sp->contentHeight, &sp->scrollPos, nullptr);
			d->d1=0;
		}
		return D_O_K;
	}

	case MSG_WHEEL:
		static_cast<ScrollingPane*>(d->dp)->scroll(-8*c);
		return D_REDRAW;

	case MSG_XCHAR:
		if(c>>8 == KEY_PGDN)
		{
			static_cast<ScrollingPane*>(d->dp)->scroll(d->h);
			return D_USED_CHAR|D_REDRAW;
		}
		else if(c>>8 == KEY_PGUP)
		{
			static_cast<ScrollingPane*>(d->dp)->scroll(-d->h);
			return D_USED_CHAR|D_REDRAW;
		}
		break;
	}

	return D_O_K;
}

ScrollingPane::ScrollingPane(): childrenEnd(0), scrollPos(0), maxScrollPos(0),
	contentHeight(0), oldMouseX(nullptr), oldMouseY(nullptr)
{
	bgColor=jwin_pal[jcBOX];
}

void ScrollingPane::scroll(int amount) noexcept
{
	int newPos=std::clamp(scrollPos+amount, 0, maxScrollPos);
	amount=newPos-scrollPos;
	scrollPos=newPos;
	for(size_t i = 1; i < childrenEnd; ++i)
		alDialog[i].y-=amount;
}

bool ScrollingPane::scrollToShowChild(int childPos)
{
	DIALOG& pane=*alDialog;
	DIALOG& child=alDialog[childPos];
	if(child.y < pane.y)
	{
		scroll(child.y-pane.y);
		return true;
	}
	else if(child.y+child.h > pane.y+pane.h)
	{
		scroll((child.y+child.h)-(pane.y+pane.h));
		return true;
	}
	else
		return false;
}

void ScrollingPane::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
	if(content)
	{
		START_CLIP(alDialog);
		content->applyVisibility(visible);
		END_CLIP();
	}
}

void ScrollingPane::calculateSize()
{
	setPreferredWidth(25_em);
	setPreferredHeight(18_em);
	if(content)
	{
		content->calculateSize();
		contentHeight=content->getTotalHeight();
		maxScrollPos=contentHeight-getHeight();
	}
}

void ScrollingPane::arrange(int contX, int contY, int contW, int contH)
{
	// We want to be about as big as possible...
	setPreferredWidth(Size::pixels(contW));
	setPreferredHeight(Size::pixels(contH));
	Widget::arrange(contX, contY, contW, contH);
	if(content)
		content->arrange(x, y, content->getWidth(), content->getHeight());
}

void ScrollingPane::realize(DialogRunner& runner)
{
	oldMouseX=gui_mouse_x;
	oldMouseY=gui_mouse_y;
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
	runner.push(shared_from_this(), DIALOG {
		mouseBreakerProc,
		0, 0, 2000, 2000, // As long as it covers the screen
		0, 0,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		this, nullptr, nullptr // dp, dp2, dp3
	});
	alDialog = runner.push(shared_from_this(), DIALOG {
		scrollProc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		this, nullptr, nullptr // dp, dp2, dp3
	});

	size_t sizeBefore=runner.size();
	content->realize(runner);
	childrenEnd=runner.size()+1-sizeBefore;

	// Every child needs D_SCROLLING set so its proc knows to use
	// some special event handling.
	for(size_t i=1; i<childrenEnd; i++)
		alDialog[i].flags|=D_SCROLLING;

	runner.push(shared_from_this(), DIALOG {
		mouseFixerProc,
		-100, -100, 1, 1, // Exactly where the replacement functions point
		0, 0,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		this, nullptr, nullptr // dp, dp2, dp3
	});
}

}
