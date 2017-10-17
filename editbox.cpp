/*                 __                  __
  *                /_/\  __  __  __    /_/\  ______
  *               _\_\/ / /\/ /\/ /\  _\_\/ / ____ \
  *              / /\  / / / / / / / / /\  / /\_ / /\
  *         __  / / / / /_/ /_/ / / / / / / / / / / /
  *        / /_/ / / /_________/ / /_/ / /_/ / /_/ /
  *        \____/ /  \_________\/  \_\/  \_\/  \_\/
  *         \___\/
  *
  *
  *
  *     jwin.c
  *
  *     Windows(R) style GUI for Allegro.
  *     by Jeremy Craner
  *
  *     Most routines are adaptations of Allegro code.
  *     Allegro is by Shawn Hargreaves, et al.
  *
  *     Version: 3/22/00
  *     Allegro version: 3.1x  (don't know if it works with WIP)
  *
  */

/* This code is not fully tested */

#include <string.h>
#include "zc_alleg.h"
#include "jwin.h"
#include <stdio.h>
#include "EditboxNew.h"

//#ifndef _MSC_VER
#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))
//#endif

extern int scheme[];
/* d_editbox_proc:
  *  A text box object. The dp field points to a char * which is the text
  *  to be displayed in the text box. If the text is long, there will be
  *  a vertical scrollbar on the right hand side of the object which can
  *  be used to scroll through the text. The default is to print the text
  *  with word wrapping, but if the D_SELECTED flag is set, the text will
  *  be printed with character wrapping. The d1 field is used internally
  *  to store the number of lines of text, and d2 is used to store how far
  *  it has scrolled through the text.
  */
int d_editbox_proc(int msg, DIALOG *d, int c)
{
	EditboxModel *model= (EditboxModel *)d->dp;
	int ret = D_O_K;
  
	static clock_t ticks;
	bool dontredraw=false;
	switch(msg)
	{
	case MSG_START:
		{
			model->getSelection().clearSelection();
			model->getView()->initialize(model);
			break;
		}
	case MSG_IDLE:
		{
			if ((d->flags & D_GOTFOCUS)&&(clock()>ticks))
			{
				d->flags |= D_DIRTY;
				ticks=clock()+(CLOCKS_PER_SEC/2);
				model->getCursor().invertVisibility();
			}
			break;
		}
	case MSG_DRAW:
		{
			model->getView()->draw();
			break;
		}
	case MSG_WANTFOCUS:
		{
			ret = D_WANTFOCUS;
			break;
		}
	case MSG_CHAR:
		{
			//handle selecting (bah)
			
			switch(c>>8)
			{
			case KEY_LEFT:
			case KEY_RIGHT:
			case KEY_UP:
			case KEY_DOWN:
			case KEY_HOME:
			case KEY_END:
			case KEY_PGUP:
			case KEY_PGDN:
				if (key[KEY_LSHIFT]||key[KEY_RSHIFT])
				{
					model->getSelection().ensureSelecting(model->getCursor());
				}
				else
				{
					model->getSelection().clearSelection();
				}
			}
			
			//normal event handling
			switch(c>>8)
			{
			case KEY_LEFT:
				model->getCursor()--;
				ret = D_USED_CHAR;
				break;
			case KEY_RIGHT:
				model->getCursor()++;
				ret = D_USED_CHAR;
				break;
			case KEY_UP:
				model->getView()->lineUp();
				ret = D_USED_CHAR;
				break;
			case KEY_DOWN:
				model->getView()->lineDown();
				ret = D_USED_CHAR;
				break;
			case KEY_HOME:
				model->getView()->lineHome();
				ret = D_USED_CHAR;
				break;
			case KEY_END:
				model->getView()->lineEnd();
				ret = D_USED_CHAR;
				break;
			case KEY_PGDN:
				model->getView()->pageDown();
				ret = D_USED_CHAR;
				break;
			case KEY_PGUP:
				model->getView()->pageUp();
				ret = D_USED_CHAR;
				break;
			case KEY_ENTER:
			case KEY_ENTER_PAD:
				model->clear();
				model->getCursor().insertNewline();
				ret = D_USED_CHAR;
				break;
			case KEY_TAB:
				{
					model->clear();
					int ch = Unicode::getCharAtOffset(uconvert_ascii("\t",NULL),0);
					model->getCursor().insertChar(ch);
					ret = D_USED_CHAR;
					break;
				}
			case KEY_DEL:
			case KEY_DEL_PAD:
				if(model->getSelection().hasSelection())
					model->clear();
				else
					model->getCursor().deleteChar();
				ret = D_USED_CHAR;
				break;
			case KEY_BACKSPACE:
				if(model->getSelection().hasSelection())
					model->clear();
				else if(model->getCursor().getPosition() != 0)
				{
					model->getCursor()--;
					model->getCursor().deleteChar();
				}
				ret = D_USED_CHAR;
				break;
			case KEY_C:
				if (key[KEY_LCONTROL]||key[KEY_RCONTROL])
				{
					model->copy();
					ret = D_USED_CHAR;
					break;
				}
				ret = D_O_K;
				break;
			case KEY_X:
				if (key[KEY_LCONTROL]||key[KEY_RCONTROL])
				{
					model->cut();
					ret = D_USED_CHAR;
					break;
				}
				ret = D_O_K;
				break;
			case KEY_V:
				if(key[KEY_LCONTROL]||key[KEY_RCONTROL])
				{
					model->clear();
					model->paste();
					ret = D_USED_CHAR;
					break;
				}
				ret = D_O_K;
				break;
			case KEY_ESC:
				return D_EXIT;
			case KEY_F1:
				model->doHelp();
				ret = D_USED_CHAR;
				dontredraw=true;
				break;
			}
			//selection post-processing
			if (key[KEY_LSHIFT]||key[KEY_RSHIFT])
			{
				switch(c>>8)
				{
				case KEY_LEFT:
				case KEY_RIGHT:
				case KEY_UP:
				case KEY_DOWN:
				case KEY_HOME:
				case KEY_END:
				case KEY_PGUP:
				case KEY_PGDN:
					model->getSelection().adjustSelection(model->getCursor());
				}
			}
			break;
		}
	case MSG_UCHAR:
		{
			ret = D_USED_CHAR;
			if ((c >= ' ') && (uisok(c)))
			{
				model->clear();
				model->getCursor().insertChar(c);
			}
		}
		break;
	case MSG_CLICK:
		{
			bool redraw = model->getView()->mouseClick(gui_mouse_x(), gui_mouse_y());
			if(model->getCursor().isVisible())
					model->getCursor().invertVisibility();
			if(redraw)
			{
				object_message(d, MSG_DRAW, 0);
			}
			while(gui_mouse_b())
			{
				
				if(model->getView()->mouseDrag(gui_mouse_x(), gui_mouse_y()))
				{
					scare_mouse();
					object_message(d, MSG_DRAW, 0);
					unscare_mouse();
				}
			}
			model->getView()->mouseRelease(gui_mouse_x(), gui_mouse_y());
			if(!model->getCursor().isVisible())
					model->getCursor().invertVisibility();
			d->flags |= D_DIRTY;
			break;
		}
	case MSG_WHEEL:
		{
			if(c>0)
				model->getView()->scrollUp();
			else
				model->getView()->scrollDown();
			d->flags |= D_DIRTY;
			break;
		}
    
	}
	if(ret == D_USED_CHAR && !dontredraw)
	{
		//redraw
		if(!model->getCursor().isVisible())
			model->getCursor().invertVisibility();
		ticks=clock()+(CLOCKS_PER_SEC/2);
		model->getView()->ensureCursorOnScreen();
		d->flags |= D_DIRTY;
	}
	return ret;
} 
 
