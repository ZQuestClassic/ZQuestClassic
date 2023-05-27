#include "misc_color_sel.h"
#include "base/zc_alleg.h"
#include "gui/dialog_runner.h"
#include "gui/list_data.h"
#include "jwin.h"
#include <utility>

int32_t d_cs_color_proc2(int32_t msg,DIALOG *d,int32_t)
{
	int32_t w=(d->w-4)/16;
	
	switch(msg)
	{
		case MSG_DRAW:
			//rectfill(screen, d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, jwin_pal[jcBOX]);
			//top blank part
			rectfill(screen, d->x, d->y, d->x+(w*16)+3, d->y+1, jwin_pal[jcBOX]);
			jwin_draw_frame(screen,d->x,d->y+2,w*16+4, d->h-4, FR_DEEP);
			
			for(int32_t i=0; i<16; ++i)
			{
				rectfill(screen, d->x+2+(w*i), d->y+4, d->x+2+(w*(i+1))-1, d->y+d->h-5, (d->d2)*16+i);
			}
			
			// right end
			rectfill(screen, d->x+(w*16)+4, d->y, d->x+d->w-1, d->y+d->h-1, jwin_pal[jcBOX]);
			// bottom part
			rectfill(screen, d->x, d->y+d->h-2, d->x+(w*16)+3, d->y+d->h-1, jwin_pal[jcBOX]);
			
			//indicator lines
			hline(screen, d->x+2+(w*d->d1), d->y, d->x+2+(w*(d->d1+1))-1, jwin_pal[jcBOXFG]);
			hline(screen, d->x+2+(w*d->d1), d->y+d->h-1, d->x+2+(w*(d->d1+1))-1, jwin_pal[jcBOXFG]);
			
			break;
			
		case MSG_CLICK:
		{
			auto oldd1 = d->d1;
			d->d1=vbound((gui_mouse_x()-d->x-2)/w,0,15);
			if(oldd1 != d->d1)
			{
				d->flags|=D_DIRTY;
				GUI_EVENT(d, geCHANGE_SELECTION);
			}
			break;
		}
	}
	
	return D_O_K;
}

int32_t d_sys_color_proc2(int32_t msg,DIALOG *d,int32_t)
{
	int32_t w=(d->w-4)/17;
	
	switch(msg)
	{
		case MSG_DRAW:
			//rectfill(screen, d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, jwin_pal[jcBOX]);
			//top blank part
			rectfill(screen, d->x, d->y, d->x+(w*17)+3, d->y+1, jwin_pal[jcBOX]);
			jwin_draw_frame(screen,d->x,d->y+2,w*17+4, d->h-4, FR_DEEP);
			
			for(int32_t i=0; i<17; ++i)
			{
				rectfill(screen, d->x+2+(w*i), d->y+4, d->x+2+(w*(i+1))-1, d->y+d->h-5, vc(zc_max(0,i-1)));
			}
			
			line(screen, d->x+2, d->y+4, d->x+2+w-1, d->y+d->h-5, vc(15));
			line(screen, d->x+2, d->y+d->h-5, d->x+2+w-1, d->y+4, vc(15));
			// right end
			rectfill(screen, d->x+(w*17)+4, d->y, d->x+d->w-1, d->y+d->h-1, jwin_pal[jcBOX]);
			// bottom part
			rectfill(screen, d->x, d->y+d->h-2, d->x+(w*17)+3, d->y+d->h-1, jwin_pal[jcBOX]);
			
			//indicator lines
			hline(screen, d->x+2+(w*(d->d1+1)), d->y, d->x+2+(w*(d->d1+2))-1, jwin_pal[jcBOXFG]);
			hline(screen, d->x+2+(w*(d->d1+1)), d->y+d->h-1, d->x+2+(w*(d->d1+2))-1, jwin_pal[jcBOXFG]);
			
			break;
			
		case MSG_CLICK:
		{
			auto oldd1 = d->d1;
			d->d1=vbound((gui_mouse_x()-d->x-2)/w,0,16)-1;
			d->flags|=D_DIRTY;
			if(oldd1 != d->d1)
			{
				d->flags|=D_DIRTY;
				GUI_EVENT(d, geCHANGE_SELECTION);
			}
			break;
		}
	}
	
	return D_O_K;
}


namespace GUI
{
	
MiscColorRow::MiscColorRow(): onUpdate(NULL), val(0), isSys(false)
{}

void MiscColorRow::setVal(int32_t newVal)
{
	if(alDialog) alDialog->d1 = newVal;
	val = newVal;
}
int32_t MiscColorRow::getVal()
{
	if(alDialog) return alDialog->d1;
	return val;
}
void MiscColorRow::setCS(int32_t newCS)
{
	if(alDialog) alDialog->d2 = newCS;
	cs = newCS;
}

void MiscColorRow::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void MiscColorRow::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void MiscColorRow::calculateSize()
{
	setPreferredWidth(Size::pixels(4 + (isSys ? 17 : 16) * 8));
	setPreferredHeight(20_px);
	Widget::calculateSize();
}

void MiscColorRow::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	if(isSys)
	{
		alDialog = runner.push(shared_from_this(), DIALOG {
			newGUIProc<d_sys_color_proc2>,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0, // key
			getFlags(), // flags,
			val, 0, // d1, d2
			this, widgFont, nullptr // dp, dp2, dp3
		});
	}
	else
	{
		alDialog = runner.push(shared_from_this(), DIALOG {
			newGUIProc<d_cs_color_proc2>,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0, // key
			getFlags(), // flags,
			val, cs, // d1, d2
			this, widgFont, nullptr // dp, dp2, dp3
		});
	}
}

void MiscColorRow::setOnUpdate(std::function<void(int32_t)> newOnUpdate)
{
	onUpdate = std::move(newOnUpdate);
}

int32_t MiscColorRow::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	
	if(alDialog) val = alDialog->d1;
	
	if(onUpdate)
		onUpdate(val);
	return -1;
}

}
