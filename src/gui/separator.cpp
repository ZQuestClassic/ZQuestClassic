#include "gui/separator.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"

namespace GUI
{
	
int32_t separator_proc(int32_t msg, DIALOG* d, int32_t)
{
	if(!d || !d->dp) return D_O_K;
	Separator& prev = *(Separator*)d->dp;
	switch(msg)
	{
		case MSG_DRAW:
		{
			if (prev.vertical)
			{
				int thick = d->w / 2;
				rectfill(screen, d->x, d->y, d->x+thick-1, d->y+d->h-1, prev.c1);
				rectfill(screen, d->x+thick, d->y, d->x+d->w-1, d->y+d->h-1, prev.c2);
			}
			else
			{
				int thick = d->h / 2;
				rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+thick-1, prev.c1);
				rectfill(screen, d->x, d->y+thick, d->x+d->w-1, d->y+d->h-1, prev.c2);
			}
			break;
		}
	}
	return D_O_K;
}

Separator::Separator(bool vertical) :
	vertical(vertical), c1(jwin_pal[jcMEDDARK]), c2(jwin_pal[jcLIGHT])
{
	const auto DEF_SZ = 4_px;
	if (vertical)
	{
		capWidth(DEF_SZ);
		setPreferredWidth(DEF_SZ);
	}
	else
	{
		capHeight(DEF_SZ);
		setPreferredHeight(DEF_SZ);
	}
	setPadding(0_px);
	setMargins(0_px);
	setFitParent(true);
}

void Separator::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog)
		alDialog.applyVisibility(visible);
}

void Separator::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog)
		alDialog.applyDisabled(dis);
}

void Separator::realize(DialogRunner& runner)
{
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<separator_proc>,
		x, y, getWidth(), getHeight(),
		0, 0,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		(void*)this, nullptr, nullptr // dp, dp2, dp3
	});
}

}
