#include "new_menu.h"
#include "base/zdefs.h"
#include "base/fonts.h"
#include "gui/jwin.h"

void draw_arrow_horz(BITMAP *dest, int c, int x, int y, int w, bool left, bool center);
extern int32_t zq_screen_w, zq_screen_h;

static bool quit_all_menu = false;
static FONT* menu_font = nullptr;
static FONT* get_menu_font()
{
	if(!menu_font)
		menu_font = get_custom_font(CFONT_GUI);
	return menu_font;
}

static char process_char(int scancode)
{
	if(char c = scancode_to_ascii(scancode))
		return c;
	if(scancode >= KEY_A && scancode <= KEY_Z)
		return 'a'+(scancode-KEY_A);
	return 0;
}
static bool check_char_shortcut(string const& str, char c)
{
	auto pos = str.find('&');
	while(pos != string::npos)
	{
		++pos;
		if(pos >= str.size())
			break;
		if(str[pos] != '&')
		{
			if(tolower(c) == tolower(str[pos]))
				return true;
		}
		pos = str.find('&',pos+1);
	}
	return false;
}
static uint gui_strlen(FONT* f, char const* cstr)
{
	FONT* of = font;
	font = f;
	uint ret = gui_strlen(cstr);
	font = of;
	return ret;
}

MenuItem::MenuItem(string text, NewMenu* submenu,
	uint uid, bool dis, bool sel)
    : text(text), subtext(), uid(uid), callback(),
	submenu(submenu), disabled(dis), selected(sel)
{}
MenuItem::MenuItem(string text, std::function<void()> callback,
	uint uid, bool dis, bool sel)
    : text(text), subtext(), uid(uid), callback(callback),
	submenu(nullptr), disabled(dis), selected(sel)
{}
MenuItem::MenuItem(string text, string subtext, NewMenu* submenu,
	uint uid, bool dis, bool sel)
    : text(text), subtext(subtext), uid(uid), callback(),
	submenu(submenu), disabled(dis), selected(sel)
{}
MenuItem::MenuItem(string text, string subtext, std::function<void()> callback,
	uint uid, bool dis, bool sel)
    : text(text), subtext(subtext), uid(uid), callback(callback),
	submenu(nullptr), disabled(dis), selected(sel)
{}

void MenuItem::exec() const
{
	if(callback)
		callback();
}
void MenuItem::pop(uint x, uint y)
{
	if(!submenu)
		return;
	submenu->pop(x, y);
}

uint MenuItem::text_width() const
{
	return gui_strlen(get_menu_font(), text.c_str());
}
uint MenuItem::extra_width() const
{
	uint wid = 0;
	if(subtext.size())
		wid += gui_strlen(get_menu_font(), subtext.c_str())+subindent;
	if(isParent())
		wid += text_height(get_menu_font());
	return wid;
}
uint MenuItem::width() const
{
	return text_width() + extra_width() + lborder + rborder;
}
uint MenuItem::height() const
{
	return text_height(get_menu_font()) + vborder*2;
}
bool MenuItem::isDisabled() const
{
	return disabled || isEmpty();
}
bool MenuItem::isEmpty() const
{
	return text.empty();
}
bool MenuItem::isParent() const
{
	return submenu ? true : false;
}
bool MenuItem::hasDoubleExtra() const
{
	return isParent() && !subtext.empty();
}

void MenuItem::disable(bool dis)
{
	disabled = dis;
}
void MenuItem::select(bool sel)
{
	selected = sel;
}

void MenuItem::draw(BITMAP* dest, uint x, uint y, uint w, byte drawflags)
{
	FONT* of = font;
	font = get_menu_font();
	if(isDisabled())
		drawflags &= ~MENU_DRAWFLAG_HIGHLIGHT;
	if(drawflags&MENU_DRAWFLAG_HIGHLIGHT)
		rectfill(dest, x, y, x+w-1, y+height()-1, jwin_pal[jcSELBG]);
	uint arrowsz = text_height(get_menu_font())/2;
	if(text.empty())
	{
		_allegro_hline(dest, x+1, y+height()/2, x+w-2, jwin_pal[jcMEDDARK]);
		_allegro_hline(dest, x+1, y+height()/2+1, x+w-2, jwin_pal[jcLIGHT]);
	}
	else
	{
		int fg = isDisabled() ? jwin_pal[jcMEDDARK]
			: ((drawflags&MENU_DRAWFLAG_HIGHLIGHT) ? jwin_pal[jcSELFG] : jwin_pal[jcBOXFG]);
		uint subt_xoff = (drawflags&MENU_DRAWFLAG_SHIFTSUBTX) ? arrowsz*2 : 0;
		if(isDisabled())
		{
			gui_textout_ex(dest, text.c_str(), x+lborder+1, y+vborder, jwin_pal[jcLIGHT], -1, 0);
			gui_textout_ex(dest, text.c_str(), x+lborder, y+vborder-1, jwin_pal[jcMEDDARK], -1, 0);
			
			if(!subtext.empty())
			{
				gui_textout_ex(dest, subtext.c_str(), x+w-rborder-gui_strlen(subtext.c_str())-subt_xoff+1, y+vborder, jwin_pal[jcLIGHT], -1, 0);
				gui_textout_ex(dest, subtext.c_str(), x+w-rborder-gui_strlen(subtext.c_str())-subt_xoff, y+vborder-1, jwin_pal[jcMEDDARK], -1, 0);
			}
		}
		else
		{
			gui_textout_ex(dest, text.c_str(), x+lborder, y+vborder-1, fg, -1, 0);
			if(!subtext.empty())
				gui_textout_ex(dest, subtext.c_str(), x+w-rborder-gui_strlen(subtext.c_str())-subt_xoff, y+vborder-1, fg, -1, 0);
		}
		
		if(isParent())
		{
			if(isDisabled())
				draw_arrow_horz(dest, jwin_pal[jcLIGHT], x+w-arrowsz-rborder, y+1+vborder, arrowsz, false, false);
			draw_arrow_horz(dest, fg, x+w-arrowsz-rborder-1, y+vborder, arrowsz, false, false);
		}
		
		if(selected)
		{
			if(isDisabled())
			{
				draw_check(dest, x+2, y+vborder+2, x+lborder-2, y+height()-vborder, jwin_pal[jcLIGHT]);
				draw_check(dest, x+1, y+vborder+1, x+lborder-3, y+height()-vborder-1, fg);
			}
			else draw_check(dest, x+1, y+vborder+1, x+lborder-3, y+height()-vborder-1, fg);
		}
	}
	font = of;
}

//

NewMenu::NewMenu(std::initializer_list<MenuItem>&& entries)
    : entries(entries)
{}

MenuItem* NewMenu::by_uid(uint uid)
{
    if(uid)
    {
        for(MenuItem& mit : entries)
        {
            if(mit.uid == uid)
                return &mit;
        }
    }
    return nullptr;
}
MenuItem* NewMenu::at(uint indx)
{
    if(indx >= entries.size())
        return nullptr;
    return &entries[indx];
}
MenuItem* NewMenu::hovered()
{
	if(auto ind = hovered_ind())
		return &entries[*ind];
	return nullptr;
}
optional<uint> NewMenu::hovered_ind() const
{
	int x = gui_mouse_x(), y = gui_mouse_y();
	if(x < border || x >= border+width())
		return nullopt;
	if(y < border)
		return nullopt;
	int ty = border;
	uint indx = 0;
	for(MenuItem const& mit : entries)
	{
		ty += mit.height();
		if(y < ty)
		{
			if(mit.isDisabled())
				return nullopt;
			return indx;
		}
		++indx;
	}
	return nullopt;
}

uint NewMenu::width() const
{
	uint max_text_wid = 0, max_extra_wid = 0;
	for(MenuItem const& e : entries)
	{
		uint tw = e.text_width();
		if(tw > max_text_wid)
			max_text_wid = tw;
		uint exw = e.extra_width();
		if(exw > max_extra_wid)
			max_extra_wid = exw;
	}
	return max_text_wid + max_extra_wid + MenuItem::rborder + MenuItem::lborder + border*2;
}
uint NewMenu::height() const
{
	uint hei = 0;
	for(MenuItem const& e : entries)
		hei += e.height();
	return hei + border*2;
}
uint NewMenu::get_x(uint indx) const
{
	return border;
}
uint NewMenu::get_y(uint indx) const
{
	uint y = border;
	for(uint q = 0; q < indx && q < entries.size(); ++q)
		y += entries[q].height();
	return y;
}
void ttip_uninstall_all();
void NewMenu::pop(uint x, uint y, FONT* f)
{
	if(close_button_quit)
		return;
	update_hw_screen(true);
	x = MID(0, x-border, zq_screen_w-width()-1);
	y = MID(0, y-border, zq_screen_h-height()-1);
	
	menu_font = f;
	quit_all_menu = false;
	
	ttip_uninstall_all();
	
	popup_zqdialog_start(x, y, width(), height());
	zqdialog_set_skiptint(true);
	
	while(gui_mouse_b())
		rest(1);
	run(0,0);
	clear_keybuf();
	
	popup_zqdialog_end();
	
	while(gui_mouse_b())
		rest(1);
}
void NewMenu::run(uint x, uint y)
{
	optional<uint> msel = hovered_ind();
	optional<uint> sel = msel ? *msel : 0;
	auto mb = gui_mouse_b();
	bool redraw = true;
	do
	{
		if(close_button_quit)
			return;
		rest(1);
		auto omb = mb;
		mb = gui_mouse_b();
		auto mx = gui_mouse_x(), my = gui_mouse_y();
		optional<uint> osel = sel;
		optional<uint> msel2 = hovered_ind();
		if(mb || msel != msel2)
			sel = msel = msel2;
		bool doclick = false;
		if(mb && !omb) //press
		{
			if(mx < border || mx >= border+width()
				|| my < border || my >= border+height())
				return; //clicked off- go up one
		}
		else if(omb && !mb) //release
		{
			if(mx < border || mx >= border+width()
				|| my < border || my >= border+height())
				return; //clicked off- go up one
			doclick = true;
		}
		if(keypressed())
		{
			auto c = readkey();
			switch(c>>8)
			{
				case KEY_ESC:
					quit_all_menu = true;
					return;
				case KEY_UP:
					if(sel)
					{
						if(*sel)
						{
							for(int n = int(*sel)-1; n >= 0; --n)
							{
								if(entries[n].isDisabled())
									continue;
								*sel = uint(n);
								break;
							}
						}
					}
					break;
				case KEY_DOWN:
					if(sel)
					{
						if(*sel < entries.size()-1)
						{
							for(uint n = *sel+1; n < entries.size(); ++n)
							{
								if(entries[n].isDisabled())
									continue;
								*sel = n;
								break;
							}
						}
					}
					break;
				case KEY_LEFT:
					return;
				case KEY_RIGHT:
					if(sel)
					{
						if(entries[*sel].isParent())
							doclick = true;
						else return;
					}
					break;
				case KEY_SPACE:
				case KEY_ENTER:
					if(sel)
						doclick = true;
					break;
				default:
				{
					if(!(c&0xFF))
						c = process_char(c>>8);
					uint indx = 0;
					for(MenuItem& mit : entries)
					{
						if(check_char_shortcut(mit.text,c)
							|| check_char_shortcut(mit.subtext,c)
							|| (mit.subtext.size() == 1 && tolower(mit.subtext[0])==tolower(c)))
						{
							sel = indx;
							doclick = true;
						}
						++indx;
					}
					break;
				}
			}
		}
		if(sel != osel)
			redraw = true;
		
		if(redraw)
		{
			draw(screen,x,y,sel);
		}
		
		if(!sel)
			doclick = false;
		if(doclick && entries[*sel].isDisabled())
		{
			doclick = false;
			sel.reset();
		}
		if(doclick)
		{
			MenuItem& mit = entries[*sel];
			mit.exec();
			if(mit.isParent())
			{
				clear_keybuf();
				int ox,oy;
				get_zqdialog_xy(ox,oy);
				mit.pop(ox+get_x(*sel)+width()-border,oy+get_y(*sel));
				if(quit_all_menu)
					return;
			}
			else
			{
				quit_all_menu = true;
				return;
			}
		}
		update_hw_screen();
	}
	while(true);
}
void NewMenu::draw(BITMAP* dest, uint x, uint y, optional<uint> hl)
{
	uint wid = width();
	jwin_draw_win(dest, x, y, wid, height(), FR_WIN);
	x += border;
	y += border;
	uint indx = 0;
	wid -= border*2;
	byte drawflags = 0;
	for(MenuItem& mit : entries)
		if(mit.hasDoubleExtra())
			drawflags |= MENU_DRAWFLAG_SHIFTSUBTX;
	for(MenuItem& mit : entries)
	{
		mit.draw(dest,x,y,wid,
			((hl && indx==*hl) ? MENU_DRAWFLAG_HIGHLIGHT : 0)
			| drawflags);
		y += mit.height();
		++indx;
	}
}

