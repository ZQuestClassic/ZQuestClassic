#include "palette_frame.h"
#include "zquest.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "../jwin_a5.h"
#include "base/zsys.h"
#include <cassert>
#include <utility>
#include "tiles.h"
#include "zq_misc.h"

extern byte cset_hold[15][16*3];
extern byte cset_hold_cnt;
extern bool cset_ready;
extern int32_t cset_count,cset_first;
extern PALETTE pal,undopal;
extern RGB** gUndoPal;

void draw_cset_proc_a5(DIALOG *d)
{
	if(d->flags & D_HIDDEN)
		return;
    int d1 = zc_min(d->d1, d->d2);
    int d2 = zc_max(d->d1, d->d2);
    
	GUI::PaletteFrame& fr = *((GUI::PaletteFrame*)d->dp);
	
	byte* cdat = ((byte*)d->dp2);
	int tw = d->w / fr.scale, th = d->h / fr.scale;
	for(int x = 0; x < tw; ++x)
		for(int y = 0; y < th; ++y)
		{
			byte* cd = cdat+(x+(y*16))*3;
			al_draw_filled_rectangle(d->x+(x*fr.scale),d->y+(y*fr.scale),
				d->x+(x*fr.scale)+fr.scale,d->y+(y*fr.scale)+fr.scale,a5color(_RGB(cd[0],cd[1],cd[2])));
		}
	
	unsigned char gr = abs(fr.grcol);
    al_draw_rectangle(d->x+1,(d1*fr.scale)+d->y+1, //highlights currently selected cset(s)
		d->x+d->w,(d2*fr.scale)+d->y+fr.scale,al_map_rgb(gr,gr,gr),0);
    int drc;
    
    if((d->flags & D_GOTFOCUS))
        drc = (d->flags & D_DISABLED) ? jcDISABLED_FG : d->fg;
    else
        drc = d->bg;
        
    dotted_rect_a5(d->x-1, d->y-1, d->x+d->w+1, d->y+d->h+1, jwin_a5_pal(drc), jwin_a5_pal(d->bg));
}

int d_cset_proc_a5(int msg,DIALOG *d,int c)
{
	switch(msg)
	{
		case MSG_START:
			d->d2 = d->d1;
			break;
			
		case MSG_WANTFOCUS:
			return D_WANTFOCUS;
			
		case MSG_DRAW:
			draw_cset_proc_a5(d);
			break;
			
		case MSG_CLICK:
		{
			bool dragging=false;
			int src=0;
			int x;
			int y;
			
			// Start dragging?
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				x=gui_mouse_x();
				y=gui_mouse_y();
				
				if(isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
				{
					dragging=true;
					src=vbound((int)((y-d->y) / (1.5))>>3,0,15) * 16 +
						vbound((int)((x-d->x) / (1.5))>>3,0,15);
				}
			}
			
			do
			{
				x=gui_mouse_x();
				y=gui_mouse_y();
				
				if(!dragging && isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
				{
					d->d2 = vbound((int)((y-d->y)/(1.5))>>3,0,15);
					
					if(!(key_shifts&KB_SHIFT_FLAG))
						d->d1 = d->d2;
					draw_cset_proc_a5(d);
				}
				
				broadcast_dialog_message(MSG_IDLE,0);
				custom_vsync();
			}
			while(gui_mouse_b());
			
			if(dragging && isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
			{
				int dest=vbound((int)((y-d->y) / (1.5))>>3,0,15) * 16 +
						 vbound((int)((x-d->x) / (1.5))>>3,0,15);
						 
				if(src!=dest)
				{
					if(gUndoPal)
						memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
					else memcpy(undopal,pal,sizeof(pal));
					
					if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
					{
						for(int i=0; i<3; i++)
							zc_swap(*(((byte*)d->dp2)+dest*3+i), *(((byte*)d->dp2)+src*3+i));
					}
					else
					{
						for(int i=0; i<3; i++)
							*(((byte*)d->dp2)+dest*3+i) = *(((byte*)d->dp2)+src*3+i);
					}
					
					for(int i=0; i<cset_count; i++)
						load_cset(pal,i,cset_first+i);
					draw_cset_proc_a5(d);
					saved=false;
				}
			}
			GUI_EVENT(d, geCHANGE_SELECTION);
		}
		break;
		
		case MSG_CHAR:
		{
			int shift = (key_shifts&KB_SHIFT_FLAG);
			int k=c>>8;
			
			switch(k)
			{
				case KEY_UP:
					if(d->d2>0) --d->d2;
					
					if(!shift) d->d1 = d->d2;
					
					break;
					
				case KEY_DOWN:
					if(d->d2<((int)ceil((d->h)/(1.5))>>3)-1)
						++d->d2;
						
					if(!shift) d->d1 = d->d2;
					
					break;
					
				case KEY_PGUP:
					d->d2=0;
					
					if(!shift) d->d1 = d->d2;
					
					break;
					
				case KEY_PGDN:
					d->d2=((int)ceil((d->h)/(1.5))>>3)-1;
					
					if(!shift) d->d1 = d->d2;
					
					break;
					
				case KEY_C:
					cset_hold_cnt=0;
					
					for(int row=0; row <= abs(d->d1 - d->d2); row++)
					{
						int d1 = zc_min(d->d1,d->d2);
						++cset_hold_cnt;
						
						for(int i=0; i<16*3; i++)
							cset_hold[row][i] = *(((byte*)d->dp2)+CSET(d1+row)*3+i);
					}
					
					cset_ready=true;
					break;
					
					
				case KEY_V:
					if(cset_ready)
					{
						if(gUndoPal)
							memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
						else memcpy(undopal,pal,sizeof(pal));
						int d1 = zc_min(d->d1, d->d2);
						for(int row=0; row<cset_hold_cnt && d1+row<cset_count; row++)
						{
							for(int i=0; i<16*3; i++)
								*(((byte*)d->dp2)+CSET(d1+row)*3+i) = cset_hold[row][i];
						}
						
						for(int i=0; i<cset_count; i++)
							load_cset(pal,i,cset_first+i);
						draw_cset_proc_a5(d);
						saved=false;
					}
					
					break;
					
				case KEY_U:
					undo_pal();
					
					for(int i=0; i<cset_count; i++)
						load_cset(pal,i,cset_first+i);
					draw_cset_proc_a5(d);
					break;
					
				default:
					return D_O_K;
			}
			
			draw_cset_proc_a5(d);
			custom_vsync();
			GUI_EVENT(d, geCHANGE_SELECTION);
			return D_USED_CHAR;
		}
		
		case MSG_VSYNC:
		{
			GUI::PaletteFrame& fr = *((GUI::PaletteFrame*)d->dp);
			//Cycle a grayscale color from black to white, and vice versa
			const int rate = 8;
			fr.grcol += rate;
			if(fr.grcol > 255)
				fr.grcol -= 510;
			draw_cset_proc_a5(d);
			break;
		}
	}
	
	return D_O_K;
}

namespace GUI
{

PaletteFrame::PaletteFrame(): alDialog(), plt(NULL), ColorData(NULL), count(14),
	onUpdate(NULL), grcol(0), scale(16)
{
	calculateSize();
	bgColor = jcLIGHT;
	fgColor = jcBOXFG;
}

void PaletteFrame::setScale(int value)
{
	scale = value;
}

void PaletteFrame::setColorData(byte* value)
{
	ColorData = value;
	if(alDialog)
	{
		alDialog->dp2 = value;
		pendDraw();
	}
}

void PaletteFrame::setPal(PALETTE* value)
{
	plt = value;
	if(alDialog)
	{
		alDialog->dp3 = *plt;
		pendDraw();
	}
}

int PaletteFrame::getSelection()
{
	if (alDialog) return alDialog->d2;
	
	return 0;
}

void PaletteFrame::setCount(uint8_t value)
{
	count = vbound(value, 1, 14);
}

void PaletteFrame::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void PaletteFrame::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void PaletteFrame::calculateSize()
{
	setPreferredWidth(Size::pixels(scale*16));
	setPreferredHeight(Size::pixels(scale*count));
	Widget::calculateSize();
}

void PaletteFrame::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<d_cset_proc_a5>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0,
		getFlags(),
		0, 0, // d1, d2,
		this, ColorData, *plt // dp, dp2, dp3
	});
}

int PaletteFrame::onEvent(int event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	
	if(onUpdate)
		onUpdate();
	return -1;
}

void PaletteFrame::setOnUpdate(std::function<void()> newOnUpdate)
{
	onUpdate = std::move(newOnUpdate);
}

}
