#include "new_menu.h"
#include "base/zdefs.h"
#include "base/fonts.h"
#include "gui/jwin.h"

void draw_arrow_horz(BITMAP *dest, int c, int x, int y, int w, bool left, bool center);
extern int32_t zq_screen_w, zq_screen_h;
void ttip_uninstall_all();

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

uint txt_hei()
{
	return text_height(get_menu_font());
}

void GuiMenu::set_font(FONT* newfont)
{
	menu_font = newfont;
}

MenuItem::MenuItem(string text, GuiMenu* submenu,
	optional<uint> uid, bool dis, bool sel)
    : text(text), subtext(), uid(uid),
	onCall(),
	submenu(submenu), disabled(dis), selected(sel)
{}
MenuItem::MenuItem(string text, string subtext, GuiMenu* submenu,
	optional<uint> uid, bool dis, bool sel)
    : text(text), subtext(subtext), uid(uid),
	onCall(),
	submenu(submenu), disabled(dis), selected(sel)
{}
MenuItem::MenuItem(string text, std::function<void()> callback,
	optional<uint> uid, bool dis, bool sel)
    : text(text), subtext(), uid(uid),
	onCall(callback),
	submenu(nullptr), disabled(dis), selected(sel)
{}
MenuItem::MenuItem(string text, string subtext, std::function<void()> callback,
	optional<uint> uid, bool dis, bool sel)
    : text(text), subtext(subtext), uid(uid),
	onCall(callback),
	submenu(nullptr), disabled(dis), selected(sel)
{}

void MenuItem::exec() const
{
	if(onCall)
		onCall();
}
void MenuItem::pop(uint x, uint y, GuiMenu* parent)
{
	if(!submenu)
		return;
	submenu->pop(x, y, parent);
}

uint MenuItem::text_width() const
{
	return gui_strlen(get_menu_font(), text.c_str());
}
uint MenuItem::calc_width(uint style, uint drawflags) const
{
	uint wid = text_width();
	switch(style)
	{
		case MISTYLE_POPUP:
			if(subtext.size())
				wid += gui_strlen(get_menu_font(), subtext.c_str())+subindent;
			if(isParent())
				wid += txt_hei();
			if(!(drawflags&MENU_DRAWFLAG_NOSEL))
				wid += selindent;
			wid += lborder + rborder;
			break;
		case MISTYLE_TOP:
			if(subtext.size())
				wid += gui_strlen(get_menu_font(), subtext.c_str())+subindent;
			wid += t_hborder*2;
			break;
	}
	return wid;
}
uint MenuItem::calc_height(uint style, uint drawflags) const
{
	uint hei = txt_hei();
	switch(style)
	{
		case MISTYLE_POPUP:
			hei += vborder*2;
			break;
		case MISTYLE_TOP:
			hei += t_vborder*2;
			break;
	}
	return hei;
}

bool MenuItem::isDisabled() const
{
	return disabled || isEmpty();
}
bool MenuItem::isSelected() const
{
	return selected;
}
bool MenuItem::isEmpty() const
{
	return text.empty();
}
bool MenuItem::isParent() const
{
	return submenu ? true : false;
}

void MenuItem::disable(bool dis)
{
	disabled = dis;
}
void MenuItem::select(bool sel)
{
	selected = sel;
}

void MenuItem::draw(BITMAP* dest, uint x, uint y, uint style, byte drawflags, optional<uint> usew)
{
	if(style >= MISTYLE_MAX)
		return;
	bool dis = isDisabled(), sel = selected;
	bool highlight = !dis && (drawflags&MENU_DRAWFLAG_HIGHLIGHT);
	bool inv_frame = (drawflags&MENU_DRAWFLAG_INVFRAME);
	bool shiftsub = (drawflags&MENU_DRAWFLAG_SHIFTSUBTX);
	bool showarrow = isParent();
	bool nosel = (drawflags&MENU_DRAWFLAG_NOSEL);
	
	uint w = usew ? *usew : calc_width(style,drawflags);
	uint h = calc_height(style,drawflags);
	FONT* of = font;
	font = get_menu_font();
	switch(style)
	{
		case MISTYLE_POPUP:
		{
			uint lb = lborder, rb = rborder, vb = vborder;
			if(!nosel)
				lb += selindent;
			if(highlight)
				rectfill(dest, x, y, x+w-1, y+h-1, jwin_pal[jcSELBG]);
			uint arrowsz = txt_hei()/2;
			if(text.empty())
			{
				_allegro_hline(dest, x+1, y+h/2, x+w-2, jwin_pal[jcMEDDARK]);
				_allegro_hline(dest, x+1, y+h/2+1, x+w-2, jwin_pal[jcLIGHT]);
			}
			else
			{
				int fg = dis ? jwin_pal[jcMEDDARK]
					: (highlight ? jwin_pal[jcSELFG] : jwin_pal[jcBOXFG]);
				uint subt_xoff = shiftsub ? arrowsz*2 : 0;
				if(dis)
				{
					gui_textout_ex(dest, text.c_str(), x+lb+1, y+vb, jwin_pal[jcLIGHT], -1, 0);
					gui_textout_ex(dest, text.c_str(), x+lb, y+vb-1, jwin_pal[jcMEDDARK], -1, 0);
					
					if(!subtext.empty())
					{
						gui_textout_ex(dest, subtext.c_str(), x+w-rb-gui_strlen(subtext.c_str())-subt_xoff+1, y+vb, jwin_pal[jcLIGHT], -1, 0);
						gui_textout_ex(dest, subtext.c_str(), x+w-rb-gui_strlen(subtext.c_str())-subt_xoff, y+vb-1, jwin_pal[jcMEDDARK], -1, 0);
					}
				}
				else
				{
					gui_textout_ex(dest, text.c_str(), x+lb, y+vb-1, fg, -1, 0);
					if(!subtext.empty())
						gui_textout_ex(dest, subtext.c_str(), x+w-rb-gui_strlen(subtext.c_str())-subt_xoff, y+vb-1, fg, -1, 0);
				}
				
				if(showarrow)
				{
					if(dis)
						draw_arrow_horz(dest, jwin_pal[jcLIGHT], x+w-arrowsz-rb, y+1+vb, arrowsz, false, false);
					draw_arrow_horz(dest, fg, x+w-arrowsz-rb-1, y+vb, arrowsz, false, false);
				}
				
				if(selected)
				{
					if(dis)
					{
						draw_check(dest, x+2, y+vb+2, x+lb-2, y+h-vb, jwin_pal[jcLIGHT]);
						draw_check(dest, x+1, y+vb+1, x+lb-3, y+h-vb-1, fg);
					}
					else draw_check(dest, x+1, y+vb+1, x+lb-3, y+h-vb-1, fg);
				}
			}
			break;
		}
		case MISTYLE_TOP:
		{
			constexpr uint lb = t_hborder, rb = t_hborder, vb = t_vborder;
			if(text.empty())
			{
				_allegro_hline(dest, x+1, y+h/2, x+w-2, jwin_pal[jcMEDDARK]);
				_allegro_hline(dest, x+1, y+h/2+1, x+w-2, jwin_pal[jcLIGHT]);
			}
			else
			{
				int fg = dis ? jwin_pal[jcMEDDARK] : jwin_pal[jcBOXFG];
				uint subt_xoff = 0;
				if(dis)
				{
					gui_textout_ex(dest, text.c_str(), x+lb+1, y+vb, jwin_pal[jcLIGHT], -1, 0);
					gui_textout_ex(dest, text.c_str(), x+lb, y+vb-1, jwin_pal[jcMEDDARK], -1, 0);
					
					if(!subtext.empty())
					{
						gui_textout_ex(dest, subtext.c_str(), x+w-rb-gui_strlen(subtext.c_str())-subt_xoff+1, y+vb, jwin_pal[jcLIGHT], -1, 0);
						gui_textout_ex(dest, subtext.c_str(), x+w-rb-gui_strlen(subtext.c_str())-subt_xoff, y+vb-1, jwin_pal[jcMEDDARK], -1, 0);
					}
				}
				else
				{
					gui_textout_ex(dest, text.c_str(), x+lb, y+vb-1, fg, -1, 0);
					if(!subtext.empty())
						gui_textout_ex(dest, subtext.c_str(), x+w-rb-gui_strlen(subtext.c_str())-subt_xoff, y+vb-1, fg, -1, 0);
				}
			}
			if(highlight)
				jwin_draw_frame(dest,x,y,w,h,inv_frame ? FR_MENU_INV : FR_MENU);
			break;
		}
	}
	font = of;
}

// Abstract; template menu

GuiMenu::GuiMenu(std::initializer_list<MenuItem>&& entries)
    : entries(entries)
{}
MenuItem* GuiMenu::by_uid(uint uid)
{
	for(MenuItem& mit : entries)
	{
		if(mit.uid && *mit.uid == uid)
			return &mit;
	}
    return nullptr;
}
MenuItem* GuiMenu::at(uint indx)
{
    if(indx >= entries.size())
        return nullptr;
    return &entries[indx];
}
MenuItem* GuiMenu::hovered()
{
	if(auto ind = hovered_ind())
		return &entries[*ind];
	return nullptr;
}
void GuiMenu::reset_state()
{
	sel_ind.reset();
	old_mb.reset();
}

bool GuiMenu::has_selected() const
{
	for(MenuItem const& mit : entries)
		if(mit.isSelected())
			return true;
	return false;
}
bool GuiMenu::has_doublewide() const
{
	for(MenuItem const& mit : entries)
		if(mit.isParent() && !mit.subtext.empty())
			return true;
	return false;
}

void GuiMenu::disable_uid(uint uid, bool dis)
{
	if(auto ptr = by_uid(uid))
		ptr->disable(dis);
}
void GuiMenu::select_uid(uint uid, bool sel)
{
	if(auto ptr = by_uid(uid))
		ptr->select(sel);
}
optional<uint> GuiMenu::press_shortcut(char c)
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
			return indx;
		}
		++indx;
	}
	return nullopt;
}
optional<uint> GuiMenu::press_shortcut_key(char c)
{
	c = process_char(c);
	uint indx = 0;
	for(MenuItem& mit : entries)
	{
		if(check_char_shortcut(mit.text,c)
			|| check_char_shortcut(mit.subtext,c)
			|| (mit.subtext.size() == 1 && tolower(mit.subtext[0])==tolower(c)))
		{
			return indx;
		}
		++indx;
	}
	return nullopt;
}

//Concrete; vertical menu

NewMenu::NewMenu(std::initializer_list<MenuItem>&& entries)
    : GuiMenu(std::move(entries))
{}
optional<uint> NewMenu::hovered_ind() const
{
	int x = gui_mouse_x(), y = gui_mouse_y();
	if(x < border || x >= border+width())
		return nullopt;
	if(y < border)
		return nullopt;
	int ty = border;
	uint indx = 0;
	byte drawflags = 0;
	// Doesn't matter for height
	// if(!has_selected())
		// drawflags |= MENU_DRAWFLAG_NOSEL;
	for(MenuItem const& mit : entries)
	{
		ty += mit.calc_height(MISTYLE_POPUP, drawflags);
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
	byte drawflags = 0;
	if(!has_selected())
		drawflags |= MENU_DRAWFLAG_NOSEL;
	for(MenuItem const& e : entries)
	{
		uint tw = e.text_width();
		if(tw > max_text_wid)
			max_text_wid = tw;
		uint exw = e.calc_width(MISTYLE_POPUP, drawflags)-tw;
		if(exw > max_extra_wid)
			max_extra_wid = exw;
	}
	return max_text_wid + max_extra_wid + border*2;
}
uint NewMenu::height() const
{
	uint hei = 0;
	byte drawflags = 0;
	// Doesn't matter for height
	// if(!has_selected())
		// drawflags |= MENU_DRAWFLAG_NOSEL;
	for(MenuItem const& e : entries)
		hei += e.calc_height(MISTYLE_POPUP, drawflags);
	return hei + border*2;
}
uint NewMenu::get_x(uint indx) const
{
	return border;
}
uint NewMenu::get_y(uint indx) const
{
	uint y = border;
	byte drawflags = 0;
	// Doesn't matter for height
	// if(!has_selected())
		// drawflags |= MENU_DRAWFLAG_NOSEL;
	for(uint q = 0; q < indx && q < entries.size(); ++q)
		y += entries[q].calc_height(MISTYLE_POPUP, drawflags);
	return y;
}
void NewMenu::pop(uint x, uint y, GuiMenu* parent, FONT* f)
{
	if(close_button_quit)
		return;
	update_hw_screen(true);
	x = MID(0, x, zq_screen_w-width()-1);
	y = MID(0, y, zq_screen_h-height()-1);
	
	menu_font = f;
	quit_all_menu = false;
	
	ttip_uninstall_all();
	
	popup_zqdialog_start(x, y, width(), height());
	zqdialog_set_skiptint(true);
	
	while(gui_mouse_b())
		rest(1);
	run_loop(0,0,parent);
	menu_font = nullptr;
	clear_keybuf();
	
	popup_zqdialog_end();
	
	while(gui_mouse_b())
		rest(1);
}
void NewMenu::draw(BITMAP* dest, uint x, uint y, optional<uint> hl)
{
	uint wid = width();
	jwin_draw_win(dest, x, y, wid, height(), FR_WIN);
	x += get_x(0);
	y += get_y(0);
	uint indx = 0;
	wid -= border*2;
	byte drawflags = 0;
	if(has_doublewide())
		drawflags |= MENU_DRAWFLAG_SHIFTSUBTX;
	if(!has_selected())
		drawflags |= MENU_DRAWFLAG_NOSEL;
	for(MenuItem& mit : entries)
	{
		mit.draw(dest,x,y,MISTYLE_POPUP,
			drawflags|((hl && indx==*hl) ? MENU_DRAWFLAG_HIGHLIGHT : 0),
			wid);
		y += mit.calc_height(MISTYLE_POPUP, drawflags);
		++indx;
	}
}
void NewMenu::run_loop(uint x, uint y, GuiMenu* parent)
{
	optional<uint> msel = hovered_ind();
	if(msel)
		sel_ind = msel;
	else
		sel_ind = 0;
	auto mb = gui_mouse_b();
	bool redraw = true;
	old_mb = mb;
	do
	{
		if(close_button_quit)
			return;
		rest(1);
		mb = gui_mouse_b();
		auto mx = gui_mouse_x(), my = gui_mouse_y();
		optional<uint> osel = sel_ind;
		optional<uint> msel2 = hovered_ind();
		if(mb || msel != msel2)
			sel_ind = msel = msel2;
		bool doclick = false;
		if(old_mb)
		{
			if(mb && !*old_mb) //press
			{
				if(mx < border || mx >= width()-border
					|| my < border || my >= height()-border)
					return; //clicked off- go up one
			}
			else if(*old_mb && !mb) //release
			{
				if(mx < border || mx >= width()-border
					|| my < border || my >= height()-border)
					return; //clicked off- go up one
				doclick = true;
			}
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
					if(sel_ind)
					{
						if(*sel_ind)
						{
							for(int n = int(*sel_ind)-1; n >= 0; --n)
							{
								if(entries[n].isDisabled())
									continue;
								*sel_ind = uint(n);
								break;
							}
						}
					}
					break;
				case KEY_DOWN:
					if(sel_ind)
					{
						if(*sel_ind < entries.size()-1)
						{
							for(uint n = *sel_ind+1; n < entries.size(); ++n)
							{
								if(entries[n].isDisabled())
									continue;
								*sel_ind = n;
								break;
							}
						}
					}
					break;
				case KEY_LEFT:
					return;
				case KEY_RIGHT:
					if(sel_ind)
					{
						if(entries[*sel_ind].isParent())
							doclick = true;
						else return;
					}
					break;
				case KEY_SPACE:
				case KEY_ENTER:
					if(sel_ind)
						doclick = true;
					break;
				default:
				{
					if(auto val = press_shortcut(c))
					{
						sel_ind = *val;
						doclick = true;
					}
					break;
				}
			}
		}
		if(sel_ind != osel)
			redraw = true;
		
		if(redraw)
			draw(screen,x,y,sel_ind);
		
		if(!sel_ind)
			doclick = false;
		if(doclick && entries[*sel_ind].isDisabled())
		{
			doclick = false;
			sel_ind.reset();
		}
		if(doclick)
		{
			MenuItem& mit = entries[*sel_ind];
			mit.exec();
			if(mit.isParent())
			{
				clear_keybuf();
				int ox,oy;
				get_zqdialog_xy(ox,oy);
				byte drawflags = 0;
				if(!has_selected())
					drawflags |= MENU_DRAWFLAG_NOSEL;
				if(has_doublewide())
					drawflags |= MENU_DRAWFLAG_SHIFTSUBTX;
				mit.draw(screen,x+get_x(*sel_ind),y+get_y(*sel_ind),MISTYLE_POPUP,
					MENU_DRAWFLAG_HIGHLIGHT|MENU_DRAWFLAG_INVFRAME|drawflags,
					width()-border*2);
				update_hw_screen(true);
				mit.pop(ox+get_x(*sel_ind)+width()-border*2,oy+get_y(*sel_ind),this);
				if(quit_all_menu)
					return;
			}
			else
			{
				quit_all_menu = true;
				return;
			}
			while(gui_mouse_b())
				rest(1);
			mb = gui_mouse_b();
		}
		update_hw_screen();
		old_mb = mb;
	}
	while(true);
}
bool NewMenu::run(uint x, uint y, GuiMenu* parent)
{
	optional<uint> msel = hovered_ind();
	if(msel)
		sel_ind = msel;
	auto mb = gui_mouse_b();
	bool redraw = true;
	if(close_button_quit)
		return true;
	mb = gui_mouse_b();
	auto mx = gui_mouse_x(), my = gui_mouse_y();
	optional<uint> osel = sel_ind;
	optional<uint> msel2 = hovered_ind();
	if(mb || sel_ind != msel2)
		sel_ind = msel = msel2;
	bool doclick = false;
	if(old_mb)
	{
		if(mb && !*old_mb) //press
		{
			if(mx < border || mx >= width()-border
				|| my < border || my >= height()-border)
				return true; //clicked off- go up one
		}
		else if(*old_mb && !mb) //release
		{
			if(mx < border || mx >= width()-border
				|| my < border || my >= height()-border)
				return true; //clicked off- go up one
			doclick = true;
		}
	}
	if(keypressed() && CHECK_ALT)
	{
		auto c = readkey();
		if(auto val = press_shortcut_key(c>>8))
		{
			sel_ind = *val;
			doclick = true;
		}
	}
	if(sel_ind != osel)
		redraw = true;
	
	if(redraw)
		draw(screen,x,y,sel_ind);
	
	if(!sel_ind)
		doclick = false;
	if(doclick && entries[*sel_ind].isDisabled())
	{
		doclick = false;
		sel_ind.reset();
	}
	if(doclick)
	{
		MenuItem& mit = entries[*sel_ind];
		mit.exec();
		if(mit.isParent())
		{
			clear_keybuf();
			int ox,oy;
			get_zqdialog_xy(ox,oy);
			byte drawflags = 0;
			if(!has_selected())
				drawflags |= MENU_DRAWFLAG_NOSEL;
			if(has_doublewide())
				drawflags |= MENU_DRAWFLAG_SHIFTSUBTX;
			mit.draw(screen,x+get_x(*sel_ind),y+get_y(*sel_ind),MISTYLE_POPUP,
				MENU_DRAWFLAG_HIGHLIGHT|MENU_DRAWFLAG_INVFRAME|drawflags,
				width()-border*2);
			update_hw_screen(true);
			mit.pop(ox+get_x(*sel_ind)+width()-border*2,oy+get_y(*sel_ind),this);
			if(quit_all_menu)
				return true;
		}
		else
		{
			quit_all_menu = true;
			return true;
		}
		while(gui_mouse_b())
			rest(1);
		mb = gui_mouse_b();
	}
	update_hw_screen();
	old_mb = mb;
	return false;
}

//Concrete; horizontal menu

TopMenu::TopMenu(std::initializer_list<MenuItem>&& entries)
    : GuiMenu(std::move(entries))
{}
optional<uint> TopMenu::hovered_ind() const
{
	int x = gui_mouse_x(), y = gui_mouse_y();
	if(y < vborder || y >= vborder+height())
		return nullopt;
	if(x < hborder)
		return nullopt;
	int tx = hborder;
	uint indx = 0;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	for(MenuItem const& mit : entries)
	{
		tx += mit.calc_width(MISTYLE_TOP, drawflags);
		if(x < tx)
		{
			if(mit.isDisabled())
				return nullopt;
			return indx;
		}
		++indx;
	}
	return nullopt;
}

uint TopMenu::width() const
{
	uint wid = 0;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	for(MenuItem const& e : entries)
		wid += e.calc_width(MISTYLE_TOP, drawflags);
	return wid + hborder*2 + 2;
}
uint TopMenu::height() const
{
	uint max_hei = 0;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	for(MenuItem const& e : entries)
	{
		uint th = e.calc_height(MISTYLE_TOP, drawflags);
		if(th > max_hei)
			max_hei = th;
	}
	return max_hei + vborder*2;
}
uint TopMenu::get_x(uint indx) const
{
	uint x = hborder;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	for(uint q = 0; q < indx && q < entries.size(); ++q)
		x += entries[q].calc_width(MISTYLE_TOP, drawflags);
	return x;
}
uint TopMenu::get_y(uint indx) const
{
	return vborder;
}
void TopMenu::pop(uint x, uint y, GuiMenu* parent, FONT* f)
{
	if(close_button_quit)
		return;
	update_hw_screen(true);
	
	menu_font = f;
	quit_all_menu = false;
	
	ttip_uninstall_all();
	
	popup_zqdialog_start(x, y, width(), height());
	zqdialog_set_skiptint(true);
	
	while(gui_mouse_b())
		rest(1);
	run_loop(0,0,parent);
	menu_font = nullptr;
	clear_keybuf();
	
	popup_zqdialog_end();
	
	while(gui_mouse_b())
		rest(1);
}
void TopMenu::draw(BITMAP* dest, uint x, uint y, optional<uint> hl)
{
	jwin_draw_win(dest, x, y, width(), height(), FR_WIN);
	x += get_x(0);
	y += get_y(0);
	uint indx = 0;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	for(MenuItem& mit : entries)
	{
		mit.draw(dest,x,y,MISTYLE_TOP,
			drawflags|((hl && indx==*hl) ? MENU_DRAWFLAG_HIGHLIGHT : 0));
		x += mit.calc_width(MISTYLE_TOP, drawflags);
		++indx;
	}
}
void TopMenu::run_loop(uint x, uint y, GuiMenu* parent)
{
	optional<uint> msel = hovered_ind();
	if(msel)
		sel_ind = msel;
	else
		sel_ind = 0;
	auto mb = gui_mouse_b();
	bool redraw = true;
	old_mb = mb;
	do
	{
		if(close_button_quit)
			return;
		rest(1);
		mb = gui_mouse_b();
		auto mx = gui_mouse_x(), my = gui_mouse_y();
		optional<uint> osel = sel_ind;
		optional<uint> msel2 = hovered_ind();
		if(mb || msel != msel2)
			sel_ind = msel = msel2;
		bool doclick = false;
		if(old_mb)
		{
			if(mb && !*old_mb) //press
			{
				if(mx < hborder || mx >= width()-hborder
					|| my < vborder || my >= height()-vborder)
					return; //clicked off- go up one
			}
			else if(*old_mb && !mb) //release
			{
				if(mx < hborder || mx >= width()-hborder
					|| my < vborder || my >= height()-vborder)
					return; //clicked off- go up one
				doclick = true;
			}
		}
		if(keypressed())
		{
			auto c = readkey();
			switch(c>>8)
			{
				case KEY_ESC:
					quit_all_menu = true;
					return;
				case KEY_LEFT:
					if(sel_ind)
					{
						if(*sel_ind)
						{
							for(int n = int(*sel_ind)-1; n >= 0; --n)
							{
								if(entries[n].isDisabled())
									continue;
								*sel_ind = uint(n);
								break;
							}
						}
					}
					break;
				case KEY_RIGHT:
					if(sel_ind)
					{
						if(*sel_ind < entries.size()-1)
						{
							for(uint n = *sel_ind+1; n < entries.size(); ++n)
							{
								if(entries[n].isDisabled())
									continue;
								*sel_ind = n;
								break;
							}
						}
					}
					break;
				case KEY_UP:
					return;
				case KEY_DOWN:
					if(sel_ind)
					{
						if(entries[*sel_ind].isParent())
							doclick = true;
						else return;
					}
					break;
				case KEY_SPACE:
				case KEY_ENTER:
					if(sel_ind)
						doclick = true;
					break;
				default:
				{
					if(auto val = press_shortcut(c))
					{
						sel_ind = *val;
						doclick = true;
					}
					break;
				}
			}
		}
		if(sel_ind != osel)
			redraw = true;
		
		if(redraw)
			draw(screen,x,y,sel_ind);
		
		if(!sel_ind)
			doclick = false;
		if(doclick && entries[*sel_ind].isDisabled())
		{
			doclick = false;
			sel_ind.reset();
		}
		if(doclick)
		{
			MenuItem& mit = entries[*sel_ind];
			mit.exec();
			if(mit.isParent())
			{
				clear_keybuf();
				int ox,oy;
				get_zqdialog_xy(ox,oy);
				byte drawflags = 0;
				if(!has_selected())
					drawflags |= MENU_DRAWFLAG_NOSEL;
				mit.draw(screen,x+get_x(*sel_ind),y+get_y(*sel_ind),MISTYLE_TOP,
					MENU_DRAWFLAG_HIGHLIGHT|MENU_DRAWFLAG_INVFRAME|drawflags);
				update_hw_screen(true);
				mit.pop(ox+get_x(*sel_ind),oy+get_y(*sel_ind)+height()-vborder*2,this);
				if(quit_all_menu)
					return;
			}
			else
			{
				quit_all_menu = true;
				return;
			}
			while(gui_mouse_b())
				rest(1);
			mb = gui_mouse_b();
		}
		update_hw_screen();
		old_mb = mb;
	}
	while(true);
}

bool TopMenu::run(uint x, uint y, GuiMenu* parent)
{
	optional<uint> msel = hovered_ind();
	if(msel)
		sel_ind = msel;
	auto mb = gui_mouse_b();
	bool redraw = false;
	if(close_button_quit)
		return true;
	mb = gui_mouse_b();
	auto mx = gui_mouse_x(), my = gui_mouse_y();
	optional<uint> osel = sel_ind;
	optional<uint> msel2 = hovered_ind();
	if(mb || sel_ind != msel2)
		sel_ind = msel = msel2;
	bool doclick = false;
	if(old_mb)
	{
		if(mb && !*old_mb) //press
		{
			if(mx < hborder || mx >= width()-hborder
				|| my < vborder || my >= height()-vborder)
				return true; //clicked off- go up one
		}
		else if(*old_mb && !mb) //release
		{
			if(mx < hborder || mx >= width()-hborder
				|| my < vborder || my >= height()-vborder)
				return true; //clicked off- go up one
			doclick = true;
		}
	}
	if(keypressed() && CHECK_ALT)
	{
		auto c = readkey();
		if(auto val = press_shortcut_key(c>>8))
		{
			sel_ind = *val;
			doclick = true;
		}
	}
	if(sel_ind != osel)
		redraw = true;
	
	if(redraw)
		draw(screen,x,y,sel_ind);
	
	if(!sel_ind)
		doclick = false;
	if(doclick && entries[*sel_ind].isDisabled())
	{
		doclick = false;
		sel_ind.reset();
	}
	if(doclick)
	{
		MenuItem& mit = entries[*sel_ind];
		mit.exec();
		if(mit.isParent())
		{
			clear_keybuf();
			int ox,oy;
			get_zqdialog_xy(ox,oy);
			byte drawflags = 0;
			if(!has_selected())
				drawflags |= MENU_DRAWFLAG_NOSEL;
			mit.draw(screen,x+get_x(*sel_ind),y+get_y(*sel_ind),MISTYLE_TOP,
				MENU_DRAWFLAG_HIGHLIGHT|MENU_DRAWFLAG_INVFRAME|drawflags);
			update_hw_screen(true);
			mit.pop(ox+get_x(*sel_ind),oy+get_y(*sel_ind)+height()-vborder*2,this);
			if(quit_all_menu)
				return true;
		}
		else
		{
			quit_all_menu = true;
			return true;
		}
		while(gui_mouse_b())
			rest(1);
		mb = gui_mouse_b();
	}
	update_hw_screen();
	old_mb = mb;
	return false;
}

