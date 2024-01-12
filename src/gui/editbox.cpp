#include <cstring>
#include "base/zc_alleg.h"
#include "gui/jwin.h"
#include "base/fonts.h"
#include "dialog/alert.h"
#include <stdio.h>

#include "gui/EditboxNew.h"

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
			model->getView()->lineHome(CHECK_CTRL_CMD);
			ret = D_USED_CHAR;
			break;
			
		case KEY_END:
			model->getView()->lineEnd(CHECK_CTRL_CMD);
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
			if(CHECK_CTRL_CMD)
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
			if(CHECK_CTRL_CMD)
			{
				model->copy();
				ret = D_USED_CHAR;
				break;
			}
			
			ret = D_O_K;
			break;
			
		case KEY_X:
			if(CHECK_CTRL_CMD)
			{
				model->set_undo();
				model->cut();
				ret = D_USED_CHAR;
				break;
			}
			
			ret = D_O_K;
			break;
			
		case KEY_V:
			if(CHECK_CTRL_CMD)
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
			if(CHECK_CTRL_CMD)
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

std::string get_box_cfg_hdr();
int32_t d_timer_proc(int32_t, DIALOG *, int32_t);
extern int32_t zq_screen_w, zq_screen_h;

void do_box_setup(DIALOG* dlg)
{
	DIALOG edit_box_dlg[] =
	{
		/* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)      (d2)      (dp) */
		{ jwin_win_proc,        0,   0,   320,  240,  0,       vc(15), 0,      D_EXIT,       0,          0, NULL, NULL, NULL },
		{ jwin_frame_proc,   4,   23,   320-8,  240-27,   0,       0,      0,       0,             FR_DEEP,       0,       NULL, NULL, NULL },
		{ d_editbox_proc,    6,   25,   320-12,  240-6-25,  0,       0,      0,       0/*D_SELECTED*/,          0,        0,        NULL, NULL, NULL },
		{ jwin_color_swatch,    0,    0,    16,   16,  0,       0,       0,       D_EXIT,     0,          17,       NULL, NULL, NULL },
		{ jwin_color_swatch,    0,    0,    16,   16,  0,       0,       0,       D_EXIT,     0,          17,       NULL, NULL, NULL },
		{ d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,        KEY_ESC, (void *) close_dlg, NULL, NULL },
		{ d_timer_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,          0,       NULL, NULL, NULL },
		{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,          0,       NULL, NULL, NULL }
	};
	edit_box_dlg[0].w=zq_screen_w;
	edit_box_dlg[0].h=zq_screen_h;
	edit_box_dlg[1].w=zq_screen_w-8;
	edit_box_dlg[1].h=zq_screen_h-27-16;
	edit_box_dlg[2].w=zq_screen_w-8-4;
	edit_box_dlg[2].h=zq_screen_h-27-4-16;
	edit_box_dlg[3].x = zq_screen_w-36;
	edit_box_dlg[3].y = zq_screen_h-18;
	edit_box_dlg[4].x = zq_screen_w-18;
	edit_box_dlg[4].y = zq_screen_h-18;
	memcpy(dlg, edit_box_dlg, sizeof(DIALOG)*7);
}
static string file_to_str(FILE* f)
{
	string str;
	char c = fgetc(f);
	while(!feof(f))
	{
		str += c;
		c = fgetc(f);
	}
	fclose(f);
	return str;
}

bool do_box_edit(DIALOG* edit_box_dlg, std::function<bool(int)> proc, string& str, string const& title, bool wrap, bool rdonly, bool trimstr, char const* helpfile)
{
	DIALOG def_dlg[] =
	{
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL },
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL },
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL },
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL },
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL },
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL },
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL },
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
	};
	
	bool default_dlg = !edit_box_dlg;
	if(default_dlg)
	{
		do_box_setup(def_dlg);
		edit_box_dlg = def_dlg;
	}
	
	string cfg_hdr = get_box_cfg_hdr();
	auto bg = zc_get_config(cfg_hdr.c_str(), "notes_bg", vc(15));
	auto fg = zc_get_config(cfg_hdr.c_str(), "notes_fg", vc(0));
	edit_box_dlg[3].d1 = fg;
	edit_box_dlg[4].d1 = bg;
	
    string editstr = str;
	BasicEditboxView* view;
	if(wrap)
		view = new EditboxWordWrapView(&edit_box_dlg[2],get_custom_font(CFONT_TEXTBOX),fg,bg,BasicEditboxView::HSTYLE_EOTEXT);
	else
		view = new EditboxScriptView(&edit_box_dlg[2],get_custom_font(CFONT_TEXTBOX),fg,bg,BasicEditboxView::HSTYLE_EOTEXT);
    char* helpfl = const_cast<char*>(helpfile);
	EditboxModel *em = new EditboxModel(editstr, view, rdonly, helpfl);
    edit_box_dlg[0].dp = (void*)title.c_str();
    edit_box_dlg[0].dp2 = get_custom_font(CFONT_TITLE);
    edit_box_dlg[2].dp = em;
    edit_box_dlg[2].bg = bg;
    
	do_zqdialog_custom(edit_box_dlg,2,false,[&](int ret)
	{
		if(proc && !proc(ret))
			return false;
		switch(ret)
		{
			case 3:
				fg = edit_box_dlg[3].d1;
				view->setForeground(fg);
				zc_set_config(cfg_hdr.c_str(), "notes_fg", fg);
				view->draw();
				return false;
			case 4:
				bg = edit_box_dlg[4].d1;
				view->setBackground(bg);
				edit_box_dlg[2].bg = bg;
				zc_set_config(cfg_hdr.c_str(), "notes_bg", bg);
				view->draw();
				return false;
			default:
				return true;
		}
	});
	bool did_edit = false;
	if(!rdonly)
	{
		if(trimstr)
		{
			size_t first = editstr.find_first_not_of(" \t\r\n");
			size_t last = editstr.find_last_not_of(" \t\r\n");
			if(first == string::npos)
				editstr.clear();
			else editstr = editstr.substr(first, last-first+1);
		}
		if(str != editstr)
		{
			AlertDialog(title, "Save changes?", [&](bool ret,bool){
				if(ret)
				{
					did_edit = true;
					str = editstr;
				}
			}).show();
		}
		delete em;
	}
	return did_edit;
}
bool do_box_edit(DIALOG* edit_box_dlg, std::function<bool(int)> proc, FILE* f, string const& title, bool wrap, char const* helpfile)
{
	if(!f) return false;
	string str = file_to_str(f);
	return do_box_edit(edit_box_dlg, proc, str, title, wrap, true, false, helpfile);
}
bool do_box_edit(string& str, string const& title, bool wrap, bool rdonly, bool trimstr, char const* helpfile)
{
	return do_box_edit(nullptr, nullptr, str, title, wrap, rdonly, trimstr, helpfile);
}
bool do_box_edit(FILE* f, string const& title, bool wrap, char const* helpfile)
{
	if(!f) return false;
	string str = file_to_str(f);
	return do_box_edit(str, title, wrap, true, false, helpfile);
}

