#include <cstring>
#include "base/zc_alleg.h"
#include "jwin.h"
#include <stdio.h>

#include "EditboxNew.h"

extern int32_t scheme[];
/* d_editbox_proc:
 *  A text box object. The dp field points to a char * which is the text
 *  to be displayed in the text box. If the text is int32_t, there will be
 *  a vertical scrollbar on the right hand side of the object which can
 *  be used to scroll through the text. The default is to print the text
 *  with word wrapping, but if the D_SELECTED flag is set, the text will
 *  be printed with character wrapping. The d1 field is used internally
 *  to store the number of lines of text, and d2 is used to store how far
 *  it has scrolled through the text.
 */
int32_t d_editbox_proc(int32_t msg, DIALOG *d, int32_t c)
{
	EditboxModel *model= (EditboxModel *)d->dp;
	int32_t ret = D_O_K;
	
	static std::chrono::steady_clock::time_point ticks;
	auto now = std::chrono::steady_clock::now();
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
		if((d->flags & D_GOTFOCUS)&&(now>ticks))
		{
			d->flags |= D_DIRTY;
			ticks = now + std::chrono::milliseconds(500);
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
			if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
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
			model->getView()->lineHome(key[KEY_LCONTROL]||key[KEY_RCONTROL]);
			ret = D_USED_CHAR;
			break;
			
		case KEY_END:
			model->getView()->lineEnd(key[KEY_LCONTROL]||key[KEY_RCONTROL]);
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
			model->set_undo();
			model->clear();
			model->getCursor().insertNewline();
			ret = D_USED_CHAR;
			break;
			
		case KEY_TAB:
		{
			model->set_undo();
			model->clear();
			int32_t ch = Unicode::getCharAtOffset(uconvert_ascii("\t",NULL),0);
			model->getCursor().insertChar(ch);
			ret = D_USED_CHAR;
			break;
		}
		
		case KEY_DEL:
		case KEY_DEL_PAD:
			model->set_undo();
			if(model->getSelection().hasSelection())
				model->clear();
			else
				model->getCursor().deleteChar();
				
			ret = D_USED_CHAR;
			break;
			
		case KEY_BACKSPACE:
			model->set_undo();
			if(model->getSelection().hasSelection())
				model->clear();
			else if(model->getCursor().getPosition() != 0)
			{
				model->getCursor()--;
				model->getCursor().deleteChar();
			}
			
			ret = D_USED_CHAR;
			break;
			
		case KEY_A:
			if(key[KEY_LCONTROL]||key[KEY_RCONTROL])
			{
				model->getSelection().clearSelection();
				model->getView()->lineHome(true);
				model->getSelection().restartSelection(model->getCursor());
				model->getView()->lineEnd(true);
				model->getSelection().adjustSelection(model->getCursor());
				ret = D_USED_CHAR;
				break;
			}
			
			ret = D_O_K;
			break;
			
		case KEY_C:
			if(key[KEY_LCONTROL]||key[KEY_RCONTROL])
			{
				model->copy();
				ret = D_USED_CHAR;
				break;
			}
			
			ret = D_O_K;
			break;
			
		case KEY_X:
			if(key[KEY_LCONTROL]||key[KEY_RCONTROL])
			{
				model->set_undo();
				model->cut();
				ret = D_USED_CHAR;
				break;
			}
			
			ret = D_O_K;
			break;
			
		case KEY_V:
			if(key[KEY_LCONTROL]||key[KEY_RCONTROL])
			{
				model->set_undo();
				model->clear();
				model->paste();
				ret = D_USED_CHAR;
				break;
			}
			
			ret = D_O_K;
			break;
		case KEY_Z:
			if(key[KEY_LCONTROL]||key[KEY_RCONTROL])
			{
				model->undo();
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
		if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
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
		if(model->isReadonly())
			break;
		ret = D_USED_CHAR;
		
		if((c >= ' ') && (uisok(c)))
		{
			model->set_undo();
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
				object_message(d, MSG_DRAW, 0);
			}
			rest(1);
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
			
		ticks = now + std::chrono::milliseconds(500);
		model->getView()->ensureCursorOnScreen();
		d->flags |= D_DIRTY;
	}
	
	return ret;
}
