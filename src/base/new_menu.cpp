#include "new_menu.h"
#include "base/zdefs.h"
#include "base/fonts.h"
#include "gui/jwin.h"
#include <fmt/format.h>

void draw_arrow_horz(BITMAP *dest, int c, int x, int y, int w, bool left, bool center);
extern int32_t zq_screen_w, zq_screen_h;
void ttip_uninstall_all();

static bool quit_all_menu = false;
static FONT* default_menu_font()
{
	return get_custom_font(CFONT_GUI);
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

MenuItem::MenuItem(string text, GuiMenu* submenu,
	optional<uint> uid, int32_t flags)
    : text(text), subtext(), uid(uid),
	onCall(),
	submenu(submenu), flags(flags)
{}
MenuItem::MenuItem(string text, string subtext, GuiMenu* submenu,
	optional<uint> uid, int32_t flags)
    : text(text), subtext(subtext), uid(uid),
	onCall(),
	submenu(submenu), flags(flags)
{}
MenuItem::MenuItem(string text, std::function<void()> callback,
	optional<uint> uid, int32_t flags)
    : text(text), subtext(), uid(uid),
	onCall(callback),
	submenu(nullptr), flags(flags)
{}
MenuItem::MenuItem(string text, string subtext, std::function<void()> callback,
	optional<uint> uid, int32_t flags)
    : text(text), subtext(subtext), uid(uid),
	onCall(callback),
	submenu(nullptr), flags(flags)
{}

MenuItem::MenuItem(optional<uint> uid)
    : text(), subtext(), uid(uid), onCall(),
	submenu(nullptr), flags(0)
{}

void MenuItem::exec() const
{
	if(!onCall)
		return;
	if(flags & MFL_EXIT_PRE_PROC)
	{
		on_zqdialog_close([=]()
			{
				if(onCall)
					onCall();
			});
		return;
	}
	if(onCall)
		onCall();
}
MenuRet MenuItem::pop(uint x, uint y, GuiMenu* parent)
{
	if(!submenu)
		return MRET_NIL;
	return submenu->pop(x, y, parent);
}

uint MenuItem::calc_width(uint style, uint drawflags, optional<FONT*> usefont) const
{
	FONT* f = usefont ? *usefont : default_menu_font();
	uint wid = gui_strlen(f, text.c_str());
	switch(style)
	{
		case MISTYLE_POPUP:
			if(subtext.size())
				wid += gui_strlen(f, subtext.c_str())+subindent;
			if(isParent())
				wid += text_height(f);
			if(!(drawflags&MENU_DRAWFLAG_NOSEL))
				wid += selindent;
			wid += lborder + rborder;
			break;
		case MISTYLE_TOP:
			if(subtext.size())
				wid += gui_strlen(f, subtext.c_str())+subindent;
			wid += t_hborder*2;
			break;
	}
	return wid;
}
uint MenuItem::calc_height(uint style, uint drawflags, optional<FONT*> usefont) const
{
	if(isEmpty())
		return empty_height;
	FONT* f = usefont ? *usefont : default_menu_font();
	uint hei = text_height(f);
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
	return (flags&MFL_DIS) || isEmpty();
}
bool MenuItem::isSelected() const
{
	return (flags&MFL_SEL);
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
	SETFLAG(flags,MFL_DIS,dis);
}
void MenuItem::select(bool sel)
{
	SETFLAG(flags,MFL_SEL,sel);
}

void MenuItem::draw(BITMAP* dest, uint x, uint y, uint style, byte drawflags, optional<FONT*> usefont, optional<uint> usew)
{
	if(style >= MISTYLE_MAX)
		return;
	bool dis = isDisabled(), sel = isSelected();
	bool highlight = !dis && (drawflags&MENU_DRAWFLAG_HIGHLIGHT);
	bool inv_frame = (drawflags&MENU_DRAWFLAG_INVFRAME);
	bool shiftsub = (drawflags&MENU_DRAWFLAG_SHIFTSUBTX);
	bool showarrow = isParent();
	bool nosel = (drawflags&MENU_DRAWFLAG_NOSEL);
	
	FONT* of = font;
	if(usefont)
		font = *usefont;
	else font = default_menu_font();
	uint w = usew ? *usew : calc_width(style,drawflags,font);
	uint h = calc_height(style,drawflags,font);
	switch(style)
	{
		case MISTYLE_POPUP:
		{
			uint lb = lborder, rb = rborder, vb = vborder;
			if(!nosel)
				lb += selindent;
			if(highlight)
				rectfill(dest, x+hl_border, y+hl_border, x+w-1-hl_border, y+h-1-hl_border, jwin_pal[jcSELBG]);
			uint arrowsz = text_height(font)/2;
			if(isEmpty())
			{
				hline(dest, x+1, y+h/2-1, x+w-2, jwin_pal[jcMEDDARK]);
				hline(dest, x+1, y+h/2, x+w-2, jwin_pal[jcLIGHT]);
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
				
				if(sel)
				{
					if(dis)
					{
						draw_check(dest, x+lborder-sel_antiindent+1, y+vb+2, x+lb-2, y+h-vb, jwin_pal[jcLIGHT]);
						draw_check(dest, x+lborder-sel_antiindent, y+vb+1, x+lb-3, y+h-vb-1, fg);
					}
					else draw_check(dest, x+lborder-sel_antiindent, y+vb+1, x+lb-3, y+h-vb-1, fg);
				}
			}
			break;
		}
		case MISTYLE_TOP:
		{
			constexpr uint lb = t_hborder, rb = t_hborder, vb = t_vborder;
			if(isEmpty())
			{
				hline(dest, x+1, y+h/2-1, x+w-2, jwin_pal[jcMEDDARK]);
				hline(dest, x+1, y+h/2, x+w-2, jwin_pal[jcLIGHT]);
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

// Helper struct

void MenuState::clear()
{
	*this = MenuState();
}

// Abstract; template menu

GuiMenu::GuiMenu(std::initializer_list<MenuItem>&& entries)
    : GuiMenu()
{
	add(entries);
}
GuiMenu::GuiMenu(optional<uint> chop, bool borderless)
	: chop_index(chop), borderless(borderless),
	entries(), state(), menu_font(),
	xpos(0), ypos(0)
{}
GuiMenu::GuiMenu(optional<uint> chop, bool borderless,std::initializer_list<MenuItem>&& entries)
	: GuiMenu(chop,borderless)
{
	add(entries);
}
int GuiMenu::proc(int msg, DIALOG* d, int c)
{
	GuiMenu* ptr = static_cast<GuiMenu*>(d->dp);
	if(!ptr)
		return D_O_K;
	optional<FONT*> f;
	if(d->dp2)
		f = static_cast<FONT*>(d->dp2);
	int ret = D_O_K;
	switch(msg)
	{
		case MSG_START:
			ptr->setFont(f);
			ptr->reset_state();
			ptr->position(d->x, d->y);
			d->w = ptr->width();
			d->h = ptr->height();
			break;
		case MSG_DRAW:
		{
			ptr->setFont(f);
			ptr->position(d->x, d->y);
			ptr->draw(screen, ptr->hovered_ind());
			break;
		}
		case MSG_VSYNC:
		case MSG_GOTMOUSE:
		case MSG_CLICK:
		case MSG_XCHAR:
			ptr->setFont(f);
			ptr->position(d->x, d->y);
			ptr->run();
			ret |= D_REDRAWME;
			break;
	}
	return ret;
}

void GuiMenu::do_draw()
{
	draw(screen,state.sel_ind);
	state.dirty = false;
}

void GuiMenu::add(MenuItem const& entry)
{
	entries.emplace_back(entry);
}
void GuiMenu::add(MenuItem&& entry)
{
	entries.emplace_back(entry);
}
void GuiMenu::add(std::initializer_list<MenuItem> const& new_entries)
{
	entries.insert(entries.end(), new_entries.begin(), new_entries.end());
}
void GuiMenu::add_sep()
{
	entries.emplace_back();
}
optional<uint> GuiMenu::ind_at(uint uid)
{
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		if(mit.uid && *mit.uid == uid)
			return indx;
	}
    return nullopt;
}
MenuItem* GuiMenu::by_uid(uint uid)
{
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
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
uint GuiMenu::chop_sz() const
{
	if(chop_index && *chop_index < size())
		return *chop_index;
	return size();
}
void GuiMenu::reset_state()
{
	state.clear();
}

bool GuiMenu::has_selected() const
{
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		MenuItem const& mit = entries[indx];
		if(mit.isSelected())
			return true;
	}
	return false;
}
bool GuiMenu::has_doublewide() const
{
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		MenuItem const& mit = entries[indx];
		if(mit.isParent() && !mit.subtext.empty())
			return true;
	}
	return false;
}

void GuiMenu::select_only_index(uint selind)
{
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		mit.select(indx == selind);
	}
}
void GuiMenu::select_only_uid(uint uid)
{
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		mit.select(mit.uid && *mit.uid == uid);
	}
}
void GuiMenu::for_each(std::function<void(MenuItem&,uint)> proc)
{
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
		proc(entries[indx],indx);
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
void GuiMenu::position(uint x, uint y)
{
	xpos = x;
	ypos = y;
}
optional<uint> GuiMenu::press_shortcut(char c)
{
	if(!(c&0xFF))
		c = process_char(c>>8);
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		if(check_char_shortcut(mit.text,c)
			|| check_char_shortcut(mit.subtext,c)
			|| (mit.subtext.size() == 1 && tolower(mit.subtext[0])==tolower(c)))
		{
			return indx;
		}
	}
	return nullopt;
}
optional<uint> GuiMenu::press_shortcut_key(char c)
{
	c = process_char(c);
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		if(check_char_shortcut(mit.text,c)
			|| check_char_shortcut(mit.subtext,c)
			|| (mit.subtext.size() == 1 && tolower(mit.subtext[0])==tolower(c)))
		{
			return indx;
		}
	}
	return nullopt;
}

//Concrete; vertical menu

NewMenu::NewMenu(std::initializer_list<MenuItem>&& entries)
    : GuiMenu(std::move(entries))
{}
NewMenu::NewMenu(optional<uint> chop, bool borderless)
    : GuiMenu(chop, borderless)
{}
NewMenu::NewMenu(optional<uint> chop, bool borderless,
	std::initializer_list<MenuItem>&& entries)
    : GuiMenu(chop, borderless, std::move(entries))
{}
optional<uint> NewMenu::hovered_ind() const
{
	int mx = gui_mouse_x()-xpos, my = gui_mouse_y()-ypos;
	if(mx < border || mx >= width()-border)
		return nullopt;
	int ty = border;
	if(my < ty)
		return nullopt;
	byte drawflags = 0;
	// Doesn't matter for height
	// if(!has_selected())
		// drawflags |= MENU_DRAWFLAG_NOSEL;
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		MenuItem const& mit = entries[indx];
		ty += mit.calc_height(MISTYLE_POPUP, drawflags, menu_font);
		if(my < ty)
		{
			if(mit.isDisabled())
				return nullopt;
			return indx;
		}
	}
	return nullopt;
}
bool NewMenu::has_mouse() const
{
	int mx = gui_mouse_x()-xpos, my = gui_mouse_y()-ypos;
	if(mx < border || mx >= width()-border
		|| my < border || my >= height()-border)
		return false;
	return true;
}

uint NewMenu::width() const
{
	uint max_text_wid = 0, max_extra_wid = 0;
	byte drawflags = 0;
	if(!has_selected())
		drawflags |= MENU_DRAWFLAG_NOSEL;
	uint sz = chop_sz();
	FONT* f = menu_font ? *menu_font : default_menu_font();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		uint tw = text_length(f, mit.text.c_str());
		if(tw > max_text_wid)
			max_text_wid = tw;
		uint exw = mit.calc_width(MISTYLE_POPUP, drawflags, f)-tw;
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
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		hei += mit.calc_height(MISTYLE_POPUP, drawflags, menu_font);
	}
	return hei + border*2;
}

void NewMenu::trigger(uint indx)
{
	MenuItem& mit = entries[indx];
	mit.exec();
	if(mit.isParent())
	{
		clear_keybuf();
		pop_sub(indx,this);
		draw(screen,hovered_ind());
		state.old_mb = 0;
	}
	else quit_all_menu = true;
	while(gui_mouse_b())
		rest(1);
}
optional<uint> NewMenu::get_x(uint indx) const
{
	if(indx >= chop_sz())
		return nullopt;
	return border;
}
optional<uint> NewMenu::get_y(uint indx) const
{
	if(indx >= chop_sz())
		return nullopt;
	uint y = border;
	byte drawflags = 0;
	// Doesn't matter for height
	// if(!has_selected())
		// drawflags |= MENU_DRAWFLAG_NOSEL;
	for(uint q = 0; q < indx && q < entries.size(); ++q)
		y += entries[q].calc_height(MISTYLE_POPUP, drawflags, menu_font);
	return y;
}
MenuRet NewMenu::pop(uint x, uint y, GuiMenu* parent)
{
	if(close_button_quit)
		return MRET_NIL;
	MenuRet ret = MRET_NIL;
	update_hw_screen();
	x = MID(0, x, zq_screen_w-width()-1);
	y = MID(0, y, zq_screen_h-height()-1);
	
	quit_all_menu = false;
	
	ttip_uninstall_all();
	
	popup_zqdialog_start("popup_menu", RTI_TY_POPUP_MENU, x, y, width(), height());
	zqdialog_set_skiptint(true);
	
	while(gui_mouse_b())
		rest(1);
	position(0, 0);
	ret = run_loop(parent);
	clear_keybuf();
	
	popup_zqdialog_end();
	
	while(gui_mouse_b())
		rest(1);
	return ret;
}
void NewMenu::pop_sub(uint indx, GuiMenu* parent)
{
	if(indx >= entries.size())
		return;
	int ox,oy;
	get_zqdialog_xy(ox,oy);
	
	while(true)
	{
		auto& entry = entries[indx];
		if(!entry.submenu)
			return;
		byte drawflags = 0;
		if(!has_selected())
			drawflags |= MENU_DRAWFLAG_NOSEL;
		if(has_doublewide())
			drawflags |= MENU_DRAWFLAG_SHIFTSUBTX;
		auto ex = xpos+*get_x(indx), ey = ypos+*get_y(indx);
		entry.draw(screen,ex,ey,MISTYLE_POPUP,
			MENU_DRAWFLAG_HIGHLIGHT|MENU_DRAWFLAG_INVFRAME|drawflags, menu_font,
			width()-border*2);
		auto x = ex+ox+width()-border*2;
		auto y = ey+oy;
		auto ret = entry.submenu->pop(x, y, parent);
		auto start_ind = indx;
		switch(ret)
		{
			case MRET_LEFT:
				do
				{
					if(indx)
						--indx;
					else
						indx = entries.size()-1;
					if(!entries[indx].isDisabled())
						break;
				}
				while(indx != start_ind);
				break;
			case MRET_RIGHT:
				do
				{
					indx = (indx+1) % entries.size();
					if(!entries[indx].isDisabled())
						break;
				}
				while(indx != start_ind);
				break;
			default:
				return;
		}
		draw(screen, nullopt);
	}
}
void NewMenu::draw(BITMAP* dest, optional<uint> hl)
{
	uint x = xpos, y = ypos;
	uint wid = width();
	jwin_draw_win(dest, x, y, wid, height(), borderless ? FR_INVIS : FR_WIN);
	x += *get_x(0);
	y += *get_y(0);
	wid -= border*2;
	byte drawflags = 0;
	if(has_doublewide())
		drawflags |= MENU_DRAWFLAG_SHIFTSUBTX;
	if(!has_selected())
		drawflags |= MENU_DRAWFLAG_NOSEL;
	
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		mit.draw(dest,x,y,MISTYLE_POPUP,
			drawflags|((hl && indx==*hl) ? MENU_DRAWFLAG_HIGHLIGHT : 0), menu_font,
			wid);
		y += mit.calc_height(MISTYLE_POPUP, drawflags, menu_font);
	}
}
MenuRet NewMenu::run_loop(GuiMenu* parent)
{
	optional<uint> msel = hovered_ind();
	reset_state();
	if(msel)
		state.sel_ind = msel;
	else
		state.sel_ind = 0;
	auto mb = gui_mouse_b();
	
	MenuRet ret = MRET_NIL;
	bool esc = key[KEY_ESC];
	do
	{
		if(close_button_quit)
			return ret;
		bool earlyret = false;
		mb = gui_mouse_b();
		auto mx = gui_mouse_x()-xpos, my = gui_mouse_y()-ypos;
		optional<uint> osel = state.sel_ind;
		optional<uint> msel2 = hovered_ind();
		if(mb || msel != msel2)
			state.sel_ind = msel = msel2;
		bool doclick = false;
		if(state.old_mb)
		{
			if(XOR(mb, *state.old_mb))
			{
				if(!msel)
					earlyret = true;
				if(!mb) //released
					doclick = true;
			}
		}
		if(state.old_mb || !mb)
			state.old_mb = mb;
		poll_keyboard();
		if(esc)
		{
			if(!key[KEY_ESC])
				esc = false;
		}
		if(keypressed())
		{
			auto c = readkey();
			switch(c>>8)
			{
				case KEY_ESC:
					if(esc)
						break;
					quit_all_menu = true;
					earlyret = true;
					break;
				case KEY_UP:
					if(state.sel_ind)
					{
						int old_ind = int(*state.sel_ind);
						for(int n = old_ind-1; n != old_ind; --n)
						{
							if(n < 0)
								n = chop_sz()-1;
							if(entries[n].isDisabled())
								continue;
							*state.sel_ind = uint(n);
							break;
						}
					}
					else state.sel_ind = 0;
					break;
				case KEY_DOWN:
					if(state.sel_ind)
					{
						int old_ind = int(*state.sel_ind);
						for(int n = old_ind+1; n != old_ind; ++n)
						{
							if(n >= chop_sz())
								n = 0;
							if(entries[n].isDisabled())
								continue;
							*state.sel_ind = uint(n);
							break;
						}
					}
					else state.sel_ind = 0;
					break;
				case KEY_LEFT:
					earlyret = true;
					ret = MRET_LEFT;
					break;
				case KEY_RIGHT:
					if(state.sel_ind)
					{
						if(entries[*state.sel_ind].isParent())
							doclick = true;
						else
						{
							earlyret = true;
							ret = MRET_RIGHT;
							break;
						}
					}
					break;
				case KEY_SPACE:
				case KEY_ENTER:
					if(state.sel_ind)
						doclick = true;
					break;
				default:
				{
					if(auto val = press_shortcut(c))
					{
						state.sel_ind = *val;
						doclick = true;
					}
					break;
				}
			}
		}
		if(state.sel_ind != osel)
			state.dirty = true;
		
		if(state.dirty)
			do_draw();
		
		if(state.sel_ind && *state.sel_ind >= chop_sz())
			state.sel_ind.reset();
		if(!state.sel_ind)
			doclick = false;
		
		if(earlyret)
			return ret;
		
		if(doclick && entries[*state.sel_ind].isDisabled())
		{
			doclick = false;
			state.sel_ind.reset();
		}
		if(doclick)
		{
			trigger(*state.sel_ind);
			if(quit_all_menu)
				return ret;
			mb = gui_mouse_b();
		}
		update_hw_screen();
	}
	while(true);
	return ret;
}
void NewMenu::run(bool allow_focus, GuiMenu* parent)
{
	optional<uint> msel = hovered_ind();
	auto mb = gui_mouse_b();
	if(close_button_quit)
	{
		reset_state();
		return;
	}
	bool earlyret = false;
	mb = gui_mouse_b();
	auto mx = gui_mouse_x()-xpos, my = gui_mouse_y()-ypos;
	optional<uint> osel = state.sel_ind;
	if(mb || (msel && state.sel_ind != msel))
	{
		state.sel_ind = msel;
		state.dirty = true;
	}
	bool doclick = false;
	if(state.old_mb)
	{
		if(XOR(mb, *state.old_mb))
		{
			if(!msel)
				earlyret = true;
			if(!mb) //released
				doclick = true;
		}
	}
	if(state.old_mb || !mb)
		state.old_mb = mb;
	if(keypressed() && (CHECK_ALT||allow_focus))
	{
		if(CHECK_ALT)
		{
			auto c = readkey();
			if(auto val = press_shortcut_key(c>>8))
			{
				state.sel_ind = *val;
				doclick = true;
			}
		}
		else
		{
			auto c = peekkey();
			switch(c>>8)
			{
				case KEY_UP:
					readkey();
					if(state.sel_ind)
					{
						int old_ind = int(*state.sel_ind);
						for(int n = old_ind-1; n != old_ind; --n)
						{
							if(n < 0)
								n = entries.size()-1;
							if(entries[n].isDisabled())
								continue;
							*state.sel_ind = uint(n);
							break;
						}
					}
					else state.sel_ind = 0;
					break;
				case KEY_DOWN:
					readkey();
					if(state.sel_ind)
					{
						int old_ind = int(*state.sel_ind);
						for(int n = old_ind+1; n != old_ind; ++n)
						{
							if(n >= entries.size())
								n = 0;
							if(entries[n].isDisabled())
								continue;
							*state.sel_ind = uint(n);
							break;
						}
					}
					else state.sel_ind = 0;
					break;
				case KEY_LEFT:
					readkey();
					reset_state();
					return;
				case KEY_RIGHT:
					readkey();
					if(state.sel_ind)
					{
						if(entries[*state.sel_ind].isParent())
							doclick = true;
						else
						{
							reset_state();
							return;
						}
					}
					break;
			}
		}
	}
	if(state.sel_ind != osel)
		state.dirty = true;
	
	if(state.dirty)
		do_draw();
	
	if(state.sel_ind && *state.sel_ind >= chop_sz())
		state.sel_ind.reset();
	if(!state.sel_ind)
		doclick = false;
	
	if(earlyret)
	{
		reset_state();
		return;
	}
	
	if(doclick && entries[*state.sel_ind].isDisabled())
	{
		doclick = false;
		state.sel_ind.reset();
	}
	if(doclick)
	{
		trigger(*state.sel_ind);
		if(quit_all_menu)
		{
			reset_state();
			return;
		}
		mb = gui_mouse_b();
	}
	update_hw_screen();
	return;
}

//Concrete; horizontal menu

TopMenu::TopMenu(std::initializer_list<MenuItem>&& entries)
    : GuiMenu(std::move(entries))
{}
TopMenu::TopMenu(optional<uint> chop, bool borderless)
    : GuiMenu(chop, borderless)
{}
TopMenu::TopMenu(optional<uint> chop, bool borderless,
	std::initializer_list<MenuItem>&& entries)
    : GuiMenu(chop, borderless, std::move(entries))
{}
optional<uint> TopMenu::hovered_ind() const
{
	int mx = gui_mouse_x()-xpos, my = gui_mouse_y()-ypos;
	if(my < vborder || my >= height()-vborder)
		return nullopt;
	int tx = hborder;
	if(mx < tx)
		return nullopt;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		MenuItem const& mit = entries[indx];
		tx += mit.calc_width(MISTYLE_TOP, drawflags, menu_font);
		if(mx < tx)
		{
			if(mit.isDisabled())
				return nullopt;
			return indx;
		}
	}
	return nullopt;
}
bool TopMenu::has_mouse() const
{
	int mx = gui_mouse_x()-xpos, my = gui_mouse_y()-ypos;
	if(mx < (int)hborder || mx >= width()-hborder
		|| my < vborder || my >= height()-vborder)
		return false;
	return true;
}

uint TopMenu::width() const
{
	uint wid = 0;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		wid += mit.calc_width(MISTYLE_TOP, drawflags, menu_font);
	}
	return wid + hborder*2 + 2;
}
uint TopMenu::height() const
{
	uint max_hei = 0;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		uint th = mit.calc_height(MISTYLE_TOP, drawflags, menu_font);
		if(th > max_hei)
			max_hei = th;
	}
	return max_hei + vborder*2;
}
void TopMenu::trigger(uint indx)
{
	MenuItem& mit = entries[indx];
	mit.exec();
	if(mit.isParent())
	{
		clear_keybuf();
		pop_sub(indx,this);
		draw(screen,hovered_ind());
		state.old_mb = 0;
	}
	else quit_all_menu = true;
	while(gui_mouse_b())
		rest(1);
}
optional<uint> TopMenu::get_x(uint indx) const
{
	if(indx >= chop_sz())
		return nullopt;
	uint x = hborder;
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	for(uint q = 0; q < indx && q < entries.size(); ++q)
		x += entries[q].calc_width(MISTYLE_TOP, drawflags, menu_font);
	return x;
}
optional<uint> TopMenu::get_y(uint indx) const
{
	if(indx >= chop_sz())
		return nullopt;
	return vborder;
}
MenuRet TopMenu::pop(uint x, uint y, GuiMenu* parent)
{
	if(close_button_quit)
		return MRET_NIL;
	MenuRet ret = MRET_NIL;
	update_hw_screen();
	
	quit_all_menu = false;
	
	ttip_uninstall_all();
	
	popup_zqdialog_start("popup_menu", RTI_TY_POPUP_MENU, x, y, width(), height());
	zqdialog_set_skiptint(true);
	
	while(gui_mouse_b())
		rest(1);
	position(0, 0);
	ret = run_loop(parent);
	clear_keybuf();
	
	popup_zqdialog_end();
	
	while(gui_mouse_b())
		rest(1);
	return ret;
}
void TopMenu::pop_sub(uint indx, GuiMenu* parent)
{
	if(indx >= entries.size())
		return;
	int ox,oy;
	get_zqdialog_xy(ox,oy);
	
	while(true)
	{
		auto& entry = entries[indx];
		if(!entry.submenu)
			return;
		byte drawflags = 0;
		if(!has_selected())
			drawflags |= MENU_DRAWFLAG_NOSEL;
		auto ex = xpos+*get_x(indx), ey = ypos+*get_y(indx);
		entry.draw(screen,ex,ey,MISTYLE_TOP,
			MENU_DRAWFLAG_HIGHLIGHT|MENU_DRAWFLAG_INVFRAME|drawflags, menu_font);
		auto x = ex+ox;
		auto y = ey+oy+height()-vborder*2;
		auto ret = entry.submenu->pop(x, y, parent);
		auto start_ind = indx;
		switch(ret)
		{
			case MRET_LEFT:
				do
				{
					if(indx)
						--indx;
					else
						indx = entries.size()-1;
					if(!entries[indx].isDisabled())
						break;
				}
				while(indx != start_ind);
				break;
			case MRET_RIGHT:
				do
				{
					indx = (indx+1) % entries.size();
					if(!entries[indx].isDisabled())
						break;
				}
				while(indx != start_ind);
				break;
			default:
				return;
		}
		draw(screen, nullopt);
	}
}
void TopMenu::draw(BITMAP* dest, optional<uint> hl)
{
	uint x = xpos, y = ypos;
	jwin_draw_win(dest, x, y, width(), height(), borderless ? FR_INVIS : FR_WIN);
	x += *get_x(0);
	y += *get_y(0);
	byte drawflags = MENU_DRAWFLAG_NOSEL;
	
	uint sz = chop_sz();
	for(uint indx = 0; indx < sz; ++indx)
	{
		auto& mit = entries[indx];
		mit.draw(dest,x,y,MISTYLE_TOP,
			drawflags|((hl && indx==*hl) ? MENU_DRAWFLAG_HIGHLIGHT : 0), menu_font);
		x += mit.calc_width(MISTYLE_TOP, drawflags, menu_font);
	}
}
MenuRet TopMenu::run_loop(GuiMenu* parent)
{
	reset_state();
	optional<uint> msel = hovered_ind();
	if(msel)
		state.sel_ind = msel;
	else
		state.sel_ind = 0;
	auto mb = gui_mouse_b();
	MenuRet ret = MRET_NIL;
	bool esc = key[KEY_ESC];
	do
	{
		if(close_button_quit)
			return ret;
		bool earlyret = false;
		rest(1);
		mb = gui_mouse_b();
		auto mx = gui_mouse_x()-xpos, my = gui_mouse_y()-ypos;
		optional<uint> osel = state.sel_ind;
		optional<uint> msel2 = hovered_ind();
		if(mb || msel != msel2)
			state.sel_ind = msel = msel2;
		bool doclick = false;
		if(state.old_mb)
		{
			if(XOR(mb, *state.old_mb))
			{
				if(!msel)
					earlyret = true;
				if(!mb) //released
					doclick = true;
			}
		}
		if(state.old_mb || !mb)
			state.old_mb = mb;
		poll_keyboard();
		if(esc)
		{
			if(!key[KEY_ESC])
				esc = false;
		}
		if(keypressed())
		{
			auto c = readkey();
			switch(c>>8)
			{
				case KEY_ESC:
					if(esc)
						break;
					quit_all_menu = true;
					earlyret = true;
					break;
				case KEY_LEFT:
					if(state.sel_ind)
					{
						int old_ind = int(*state.sel_ind);
						for(int n = old_ind-1; n != old_ind; --n)
						{
							if(n < 0)
								n = chop_sz()-1;
							if(entries[n].isDisabled())
								continue;
							*state.sel_ind = uint(n);
							break;
						}
					}
					else state.sel_ind = 0;
					break;
				case KEY_RIGHT:
					if(state.sel_ind)
					{
						int old_ind = int(*state.sel_ind);
						for(int n = old_ind+1; n != old_ind; ++n)
						{
							if(n >= chop_sz())
								n = 0;
							if(entries[n].isDisabled())
								continue;
							*state.sel_ind = uint(n);
							break;
						}
					}
					else state.sel_ind = 0;
					break;
				case KEY_UP:
					earlyret = true;
					ret = MRET_LEFT;
					break;
				case KEY_DOWN:
					if(state.sel_ind)
					{
						if(entries[*state.sel_ind].isParent())
							doclick = true;
						else
						{
							earlyret = true;
							ret = MRET_RIGHT;
							break;
						}
					}
					break;
				case KEY_SPACE:
				case KEY_ENTER:
					if(state.sel_ind)
						doclick = true;
					break;
				default:
				{
					if(auto val = press_shortcut(c))
					{
						state.sel_ind = *val;
						doclick = true;
					}
					break;
				}
			}
		}
		if(state.sel_ind != osel)
			state.dirty = true;
		
		if(state.dirty)
			do_draw();
		
		if(state.sel_ind && *state.sel_ind >= chop_sz())
			state.sel_ind.reset();
		if(!state.sel_ind)
			doclick = false;
		
		if(earlyret)
			return ret;
		
		if(doclick && entries[*state.sel_ind].isDisabled())
		{
			doclick = false;
			state.sel_ind.reset();
		}
		if(doclick)
		{
			trigger(*state.sel_ind);
			if(quit_all_menu)
				return ret;
			mb = gui_mouse_b();
		}
		update_hw_screen();
	}
	while(true);
	return ret;
}

void TopMenu::run(bool allow_focus, GuiMenu* parent)
{
	optional<uint> msel = hovered_ind();
	auto mb = gui_mouse_b();
	if(close_button_quit)
	{
		reset_state();
		return;
	}
	bool earlyret = false;
	mb = gui_mouse_b();
	auto mx = gui_mouse_x()-xpos, my = gui_mouse_y()-ypos;
	optional<uint> osel = state.sel_ind;
	if(mb || (msel && state.sel_ind != msel))
	{
		state.sel_ind = msel;
		state.dirty = true;
	}
	bool doclick = false;
	if(state.old_mb)
	{
		if(XOR(mb, *state.old_mb))
		{
			if(!msel)
				earlyret = true;
			if(!mb) //released
				doclick = true;
		}
	}
	if(state.old_mb || !mb)
		state.old_mb = mb;
	if(keypressed() && (CHECK_ALT||allow_focus))
	{
		auto c = readkey();
		if(CHECK_ALT)
		{
			if(auto val = press_shortcut_key(c>>8))
			{
				state.sel_ind = *val;
				doclick = true;
			}
		}
		else
		{
			switch(c>>8)
			{
				case KEY_LEFT:
					if(state.sel_ind)
					{
						int old_ind = int(*state.sel_ind);
						for(int n = old_ind-1; n != old_ind; --n)
						{
							if(n < 0)
								n = entries.size()-1;
							if(entries[n].isDisabled())
								continue;
							*state.sel_ind = uint(n);
							break;
						}
					}
					else state.sel_ind = 0;
					break;
				case KEY_RIGHT:
					if(state.sel_ind)
					{
						int old_ind = int(*state.sel_ind);
						for(int n = old_ind+1; n != old_ind; ++n)
						{
							if(n >= entries.size())
								n = 0;
							if(entries[n].isDisabled())
								continue;
							*state.sel_ind = uint(n);
							break;
						}
					}
					else state.sel_ind = 0;
					break;
				case KEY_UP:
					reset_state();
					return;
				case KEY_DOWN:
					if(state.sel_ind)
					{
						if(entries[*state.sel_ind].isParent())
							doclick = true;
						else
						{
							reset_state();
							return;
						}
					}
					break;
			}
		}
	}
	if(state.sel_ind != osel)
		state.dirty = true;
	
	if(state.dirty)
		do_draw();
	
	if(state.sel_ind && *state.sel_ind >= chop_sz())
		state.sel_ind.reset();
	if(!state.sel_ind)
		doclick = false;
	if(earlyret)
	{
		reset_state();
		return;
	}
	if(doclick && entries[*state.sel_ind].isDisabled())
	{
		doclick = false;
		state.sel_ind.reset();
	}
	if(doclick)
	{
		trigger(*state.sel_ind);
		if(quit_all_menu)
		{
			reset_state();
			return;
		}
		mb = gui_mouse_b();
	}
	return;
}

optional<int> popup_num_menu(uint x, uint y, int min, int max, optional<int> hl, std::function<string(int)> formatter)
{
	if(min > max)
		zc_swap(min,max);
	optional<int> retval;
	NewMenu popmenu;
	for(int q = min; q < max; ++q)
		popmenu.add({ formatter ? formatter(q) : to_string(q),
			[&retval,q](){retval = q;}, nullopt, hl && *hl == q ? MFL_SEL : 0 });
	popmenu.pop(x, y);
	return retval;
}

