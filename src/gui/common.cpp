#include "gui/common.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "gui/widget.h"

namespace GUI
{

int32_t scrollProc(int32_t msg, DIALOG* d, int32_t c);

void findScrollingPane(DIALOG* d, DIALOG** sp, int32_t* index)
{
	int32_t i=0;
	do
	{
		--d;
		++i;
	} while(d->proc!=scrollProc);
	*sp=d;
	*index=i;
}

int32_t newGUIProcImpl(int32_t msg, DIALOG* d, int32_t c, int32_t (*base)(int32_t, DIALOG*, int32_t))
{
	// Tooltips: any widget with tooltip text shows it while hovered.
	if(d->flags&D_NEW_GUI)
	{
		switch(msg)
		{
		case MSG_GOTMOUSE:
			if(Widget* w = DialogRunner::findWidget(d))
				if(!w->getTooltip().empty())
					gui_tooltip_show(w->getTooltip(), d->x, d->y, d->w, d->h);
			break;

		case MSG_LOSTMOUSE:
		case MSG_END:
		// Typing dismisses it too (until the mouse leaves and comes back),
		// so it doesn't pop up over a field the user is working in.
		case MSG_CHAR:
		case MSG_XCHAR:
			gui_tooltip_hide();
			break;
		}
	}

	if(d->flags&D_SCROLLING)
	{
		// This widget is in a scrolling pane and needs some special handling.
		DIALOG* sp;
		int32_t index;

		switch(msg)
		{
		case MSG_WHEEL:
			if(base(MSG_WANTWHEEL, d, 0))
				break;
			findScrollingPane(d, &sp, &index);
			return sp->proc(msg, sp, c);

		case MSG_GOTFOCUS:
		case MSG_CLICK:
		{
			findScrollingPane(d, &sp, &index);
			int32_t ret=sp->proc(MSG_CHILDFOCUSED, sp, index); // MSG_CHILDFOCUSED
			return base(msg, d, c)|ret;                    // should be first
		}

		case MSG_DRAW:
			if(!(d->flags&D_ISCLIPPED))
			{
				findScrollingPane(d, &sp, &index);
				return sp->proc(MSG_DRAWCLIPPED, sp, index);
			}
			break;
		}
	}

	return base(msg, d, c);
}

int32_t getAccelKey(const std::string_view text)
{
	bool lastWasAmpersand = false;
	for(auto& c: text)
	{
		if(c == '&')
		{
			if(lastWasAmpersand)
				lastWasAmpersand = false;
			else
				lastWasAmpersand = true;
		}
		else if(lastWasAmpersand)
			return c;
	}
	return 0;
}

}
