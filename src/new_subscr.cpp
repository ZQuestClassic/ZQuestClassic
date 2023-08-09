#include "base/zdefs.h"
#include "new_subscr.h"
#include "subscr.h"
#include "base/misctypes.h"
#include "base/fonts.h"
#include "base/zsys.h"
#include "base/dmap.h"
#include "base/qrs.h"
#include "base/mapscr.h"
#include "base/packfile.h"
#include "tiles.h"
#include "qst.h"
#include "items.h"
#include "sprite.h"
#include <set>

#ifdef IS_PLAYER
extern int32_t directItem;
extern sprite_list Lwpns;
#endif

extern gamedata* game; //!TODO ZDEFSCLEAN move to gamedata.h
extern zinitdata zinit; //!TODO ZDEFSCLEAN move to zinit.h
int32_t get_dlevel();
int32_t get_currdmap();
int32_t get_homescr();
bool has_item(int32_t item_type, int32_t item);

//!TODO subscr.h/subscr.cpp trim
extern item *sel_a, *sel_b;
void subscreenitem(BITMAP *dest, int32_t x, int32_t y, int32_t itemtype);
int32_t subscreen_color(int32_t c1, int32_t c2);
void draw_textbox(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, FONT *tempfont, char *thetext, bool wword, int32_t tabsize, int32_t alignment, int32_t textstyle, int32_t color, int32_t shadowcolor, int32_t backcolor);
void lifegauge(BITMAP *dest,int32_t x,int32_t y, int32_t container, int32_t notlast_tile, int32_t notlast_cset, bool notlast_mod, int32_t last_tile, int32_t last_cset, bool last_mod,
			   int32_t cap_tile, int32_t cap_cset, bool cap_mod, int32_t aftercap_tile, int32_t aftercap_cset, bool aftercap_mod, int32_t frames, int32_t speed, int32_t delay, bool unique_last);
void magicgauge(BITMAP *dest,int32_t x,int32_t y, int32_t container, int32_t notlast_tile, int32_t notlast_cset, bool notlast_mod, int32_t last_tile, int32_t last_cset, bool last_mod,
				int32_t cap_tile, int32_t cap_cset, bool cap_mod, int32_t aftercap_tile, int32_t aftercap_cset, bool aftercap_mod, int32_t frames, int32_t speed, int32_t delay, bool unique_last, int32_t show);
int32_t get_subscreenitem_id(int32_t itemtype, bool forceItem);


int shadow_x(int shadow)
{
	switch(shadow)
	{
		case sstsSHADOWU:
		case sstsOUTLINE8:
		case sstsOUTLINEPLUS:
		case sstsOUTLINEX:
		case sstsSHADOWEDU:
		case sstsOUTLINED8:
		case sstsOUTLINEDPLUS:
		case sstsOUTLINEDX:
			return -1;
	}
	return 0;
}
int shadow_y(int shadow)
{
	switch(shadow)
	{
		case sstsOUTLINE8:
		case sstsOUTLINEPLUS:
		case sstsOUTLINEX:
		case sstsOUTLINED8:
		case sstsOUTLINEDPLUS:
		case sstsOUTLINEDX:
			return -1;
	}
	return 0;
}
int shadow_w(int shadow)
{
	switch(shadow)
	{
		case sstsSHADOW:
		case sstsSHADOWU:
		case sstsOUTLINE8:
		case sstsOUTLINEPLUS:
		case sstsOUTLINEX:
		case sstsSHADOWED:
		case sstsSHADOWEDU:
		case sstsOUTLINED8:
		case sstsOUTLINEDPLUS:
		case sstsOUTLINEDX:
			return 1;
	}
	return 0;
}
int shadow_h(int shadow)
{
	switch(shadow)
	{
		case sstsSHADOW:
		case sstsSHADOWU:
		case sstsOUTLINE8:
		case sstsOUTLINEPLUS:
		case sstsOUTLINEX:
		case sstsSHADOWED:
		case sstsSHADOWEDU:
		case sstsOUTLINED8:
		case sstsOUTLINEDPLUS:
		case sstsOUTLINEDX:
			return 1;
	}
	return 0;
}

int32_t SubscrColorInfo::get_color() const
{
	int32_t ret;
	
	switch(type)
	{
		case ssctSYSTEM:
			ret=(color==-1)?color:vc(color);
			break;
			
		case ssctMISC:
			switch(color)
			{
				case ssctTEXT:
					ret=QMisc.colors.text;
					break;
					
				case ssctCAPTION:
					ret=QMisc.colors.caption;
					break;
					
				case ssctOVERWBG:
					ret=QMisc.colors.overw_bg;
					break;
					
				case ssctDNGNBG:
					ret=QMisc.colors.dngn_bg;
					break;
					
				case ssctDNGNFG:
					ret=QMisc.colors.dngn_fg;
					break;
					
				case ssctCAVEFG:
					ret=QMisc.colors.cave_fg;
					break;
					
				case ssctBSDK:
					ret=QMisc.colors.bs_dk;
					break;
					
				case ssctBSGOAL:
					ret=QMisc.colors.bs_goal;
					break;
					
				case ssctCOMPASSLT:
					ret=QMisc.colors.compass_lt;
					break;
					
				case ssctCOMPASSDK:
					ret=QMisc.colors.compass_dk;
					break;
					
				case ssctSUBSCRBG:
					ret=QMisc.colors.subscr_bg;
					break;
					
				case ssctSUBSCRSHADOW:
					ret=QMisc.colors.subscr_shadow;
					break;
					
				case ssctTRIFRAMECOLOR:
					ret=QMisc.colors.triframe_color;
					break;
					
				case ssctBMAPBG:
					ret=QMisc.colors.bmap_bg;
					break;
					
				case ssctBMAPFG:
					ret=QMisc.colors.bmap_fg;
					break;
					
				case ssctHERODOT:
					ret=QMisc.colors.hero_dot;
					break;
					
				default:
					ret=(zc_oldrand()*1000)%256;
					break;
			}
			
			break;
			
		default:
			ret=(type<<4)+color;
	}
	
	return ret;
}

int32_t SubscrColorInfo::get_cset() const
{
	int32_t ret=type;
	
	switch(type)
	{
		case ssctMISC:
			switch(color)
			{
				case sscsTRIFORCECSET:
					ret=QMisc.colors.triforce_cset;
					break;
					
				case sscsTRIFRAMECSET:
					ret=QMisc.colors.triframe_cset;
					break;
					
				case sscsOVERWORLDMAPCSET:
					ret=QMisc.colors.overworld_map_cset;
					break;
					
				case sscsDUNGEONMAPCSET:
					ret=QMisc.colors.dungeon_map_cset;
					break;
					
				case sscsBLUEFRAMECSET:
					ret=QMisc.colors.blueframe_cset;
					break;
					
				case sscsHCPIECESCSET:
					ret=QMisc.colors.HCpieces_cset;
					break;
					
				case sscsSSVINECSET:
					ret=wpnsbuf[iwSubscreenVine].csets&15;
					break;
					
				default:
					ret=(zc_oldrand()*1000)%256;
					break;
			}
			break;
	}
	
	return ret;
}

int32_t SubscrColorInfo::read(PACKFILE *f, word s_version)
{
	if(!p_getc(&type,f))
		return qe_invalid;
	if(!p_igetw(&color,f))
		return qe_invalid;
	return 0;
}
int32_t SubscrColorInfo::write(PACKFILE *f) const
{
	if(!p_putc(type,f))
		new_return(1);
	if(!p_iputw(color,f))
		new_return(2);
	return 0;
}

void SubscrColorInfo::load_old(subscreen_object const& old, int indx)
{
	if(indx < 1 || indx > 3) return;
	switch(indx)
	{
		case 1:
			type = old.colortype1;
			color = old.color1;
			break;
		case 2:
			type = old.colortype2;
			color = old.color2;
			break;
		case 3:
			type = old.colortype3;
			color = old.color3;
			break;
	}
}

SubscrWidget::SubscrWidget(byte ty) : SubscrWidget()
{
	type = ty;
}
SubscrWidget::SubscrWidget(subscreen_object const& old) : SubscrWidget()
{
	load_old(old);
}
bool SubscrWidget::load_old(subscreen_object const& old)
{
	type = old.type;
	posflags = old.pos;
	x = old.x;
	y = old.y;
	w = old.w;
	h = old.h;
	return true;
}
int16_t SubscrWidget::getX() const
{
	return x;
}
int16_t SubscrWidget::getY() const
{
	return y;
}
word SubscrWidget::getW() const
{
	return w;
}
word SubscrWidget::getH() const
{
	return h;
}
int16_t SubscrWidget::getXOffs() const
{
	return 0;
}
int16_t SubscrWidget::getYOffs() const
{
	return 0;
}
byte SubscrWidget::getType() const
{
	return type;
}
int32_t SubscrWidget::getItemVal() const
{
	return -1;
}
void SubscrWidget::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	
}
bool SubscrWidget::visible(byte pos, bool showtime) const
{
	return posflags&pos;
}
SubscrWidget* SubscrWidget::clone() const
{
	return new SubscrWidget(*this);
}
bool SubscrWidget::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	flags = src->flags;
	posflags &= ~(sspUP|sspDOWN|sspSCROLLING);
	posflags |= src->posflags&(sspUP|sspDOWN|sspSCROLLING);
	if(all)
	{
		posflags = src->posflags;
		x = src->x;
		y = src->y;
		w = src->w;
		h = src->h;
		pos = src->pos;
		pos_up = src->pos_up;
		pos_down = src->pos_down;
		pos_left = src->pos_left;
		pos_right = src->pos_right;
		override_text = src->override_text;
		gen_script_btns = src->gen_script_btns;
		generic_script = src->generic_script;
		type = src->type;
	}
	return true;
}
int32_t SubscrWidget::read(PACKFILE *f, word s_version)
{
	//does not 'p_getc(&type)', SubscrWidget::readWidg() handles that
	if(!p_getc(&posflags,f))
		return qe_invalid;
	if(!p_igetw(&x,f))
		return qe_invalid;
	if(!p_igetw(&y,f))
		return qe_invalid;
	if(!p_igetw(&w,f))
		return qe_invalid;
	if(!p_igetw(&h,f))
		return qe_invalid;
	if(!p_igetl(&flags,f))
		return qe_invalid;
	if(flags&SUBSCRFLAG_SELECTABLE)
	{
		if(!p_igetl(&pos,f))
			return qe_invalid;
		if(!p_igetl(&pos_up,f))
			return qe_invalid;
		if(!p_igetl(&pos_down,f))
			return qe_invalid;
		if(!p_igetl(&pos_left,f))
			return qe_invalid;
		if(!p_igetl(&pos_right,f))
			return qe_invalid;
		if(!p_getcstr(&override_text,f))
			return qe_invalid;
		if(!p_getc(&gen_script_btns,f))
			return qe_invalid;
		if(!p_igetw(&generic_script,f))
			return qe_invalid;
	}
	return 0;
}
int32_t SubscrWidget::write(PACKFILE *f) const
{
	if(!p_putc(type,f))
		new_return(1);
	if(!p_putc(posflags,f))
		new_return(2);
	if(!p_iputw(x,f))
		new_return(3);
	if(!p_iputw(y,f))
		new_return(4);
	if(!p_iputw(w,f))
		new_return(5);
	if(!p_iputw(h,f))
		new_return(6);
	if(!p_iputl(flags,f))
		new_return(7);
	if(flags&SUBSCRFLAG_SELECTABLE)
	{
		if(!p_iputl(pos,f))
			new_return(8);
		if(!p_iputl(pos_up,f))
			new_return(9);
		if(!p_iputl(pos_down,f))
			new_return(10);
		if(!p_iputl(pos_left,f))
			new_return(11);
		if(!p_iputl(pos_right,f))
			new_return(12);
		if(!p_putcstr(override_text,f))
			new_return(13);
		if(!p_putc(gen_script_btns,f))
			new_return(14);
		if(!p_iputw(generic_script,f))
			new_return(15);
	}
	return 0;
}

SW_2x2Frame::SW_2x2Frame(subscreen_object const& old) : SW_2x2Frame()
{
	load_old(old);
}
bool SW_2x2Frame::load_old(subscreen_object const& old)
{
	if(old.type != sso2X2FRAME)
		return false;
	SubscrWidget::load_old(old);
	tile = old.d1;
	cs.load_old(old,1);
	SETFLAG(flags,SUBSCR_2X2FR_TRANSP,old.d4);
	SETFLAG(flags,SUBSCR_2X2FR_OVERLAY,old.d3);
	return true;
}
word SW_2x2Frame::getW() const
{
	return w*8;
}
word SW_2x2Frame::getH() const
{
	return h*8;
}
byte SW_2x2Frame::getType() const
{
	return sso2X2FRAME;
}
void SW_2x2Frame::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	frame2x2(dest, x+xofs, y+yofs, tile, cs.get_cset(), w, h, 0,
		flags&SUBSCR_2X2FR_OVERLAY, flags&SUBSCR_2X2FR_TRANSP);
}
SubscrWidget* SW_2x2Frame::clone() const
{
	return new SW_2x2Frame(*this);
}
bool SW_2x2Frame::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_2x2Frame const* other = dynamic_cast<SW_2x2Frame const*>(src);
	SubscrWidget::copy_prop(other,all);
	cs = other->cs;
	tile = other->tile;
	return true;
}
int32_t SW_2x2Frame::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&tile,f))
		return qe_invalid;
	if(auto ret = cs.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_2x2Frame::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(tile,f))
		return qe_invalid;
	if(auto ret = cs.write(f))
		return ret;
	return 0;
}

SW_Text::SW_Text(subscreen_object const& old) : SW_Text()
{
	load_old(old);
}
bool SW_Text::load_old(subscreen_object const& old)
{
	if(old.type != ssoTEXT)
		return false;
	SubscrWidget::load_old(old);
	if(old.dp1) text = (char*)old.dp1;
	else text.clear();
	fontid = to_real_font(old.d1);
	align = old.d2;
	shadtype = old.d3;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
int16_t SW_Text::getX() const
{
	return x+shadow_x(shadtype);
}
int16_t SW_Text::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Text::getW() const
{
	return text_length(get_zc_font(fontid), text.c_str());
}
word SW_Text::getH() const
{
	return text_height(get_zc_font(fontid));
}
int16_t SW_Text::getXOffs() const
{
	switch(align)
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_Text::getType() const
{
	return ssoTEXT;
}
void SW_Text::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	textout_styled_aligned_ex(dest,tempfont,text.c_str(),getX()+xofs,getY()+yofs,
		shadtype,align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color());
}
SubscrWidget* SW_Text::clone() const
{
	return new SW_Text(*this);
}
bool SW_Text::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Text const* other = dynamic_cast<SW_Text const*>(src);
	SubscrWidget::copy_prop(other,all);
	fontid = other->fontid;
	text = other->text;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	return true;
}
int32_t SW_Text::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&fontid,f))
		return qe_invalid;
	if(!p_getc(&align,f))
		return qe_invalid;
	if(!p_getc(&shadtype,f))
		return qe_invalid;
	if(!p_getwstr(&text,f))
		return qe_invalid;
	if(auto ret = c_text.read(f,s_version))
		return ret;
	if(auto ret = c_shadow.read(f,s_version))
		return ret;
	if(auto ret = c_bg.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_Text::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(2);
	if(!p_putc(shadtype,f))
		new_return(3);
	if(!p_putwstr(text,f))
		new_return(4);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	return 0;
}

SW_Line::SW_Line(subscreen_object const& old) : SW_Line()
{
	load_old(old);
}
bool SW_Line::load_old(subscreen_object const& old)
{
	if(old.type != ssoLINE)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_LINE_TRANSP,old.d4);
	c_line.load_old(old,1);
	return true;
}
byte SW_Line::getType() const
{
	return ssoLINE;
}
void SW_Line::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(flags&SUBSCR_LINE_TRANSP)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	
	line(dest, x+xofs, y+yofs, x+xofs+w-1, y+yofs+h-1, c_line.get_color());
	
	if(flags&SUBSCR_LINE_TRANSP)
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}
SubscrWidget* SW_Line::clone() const
{
	return new SW_Line(*this);
}
bool SW_Line::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Line const* other = dynamic_cast<SW_Line const*>(src);
	SubscrWidget::copy_prop(other,all);
	c_line = other->c_line;
	return true;
}
int32_t SW_Line::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(auto ret = c_line.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_Line::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(auto ret = c_line.write(f))
		return ret;
	return 0;
}

SW_Rect::SW_Rect(subscreen_object const& old) : SW_Rect()
{
	load_old(old);
}
bool SW_Rect::load_old(subscreen_object const& old)
{
	if(old.type != ssoRECT)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_RECT_TRANSP,old.d2);
	SETFLAG(flags,SUBSCR_RECT_FILLED,old.d1);
	c_fill.load_old(old,2);
	c_outline.load_old(old,1);
	return true;
}
byte SW_Rect::getType() const
{
	return ssoRECT;
}
void SW_Rect::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(flags&SUBSCR_RECT_TRANSP)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	
	auto x2 = x+xofs, y2 = y+yofs;
	if(flags&SUBSCR_RECT_FILLED)
		rectfill(dest, x2, y2, x2+w-1, y2+h-1, c_fill.get_color());
	
	rect(dest, x2, y2, x2+w-1, y2+h-1, c_outline.get_color());
	
	if(flags&SUBSCR_RECT_TRANSP)
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}
SubscrWidget* SW_Rect::clone() const
{
	return new SW_Rect(*this);
}
bool SW_Rect::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Rect const* other = dynamic_cast<SW_Rect const*>(src);
	SubscrWidget::copy_prop(other,all);
	c_fill = other->c_fill;
	c_outline = other->c_outline;
	return true;
}
int32_t SW_Rect::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(auto ret = c_fill.read(f,s_version))
		return ret;
	if(auto ret = c_outline.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_Rect::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(auto ret = c_fill.write(f))
		return ret;
	if(auto ret = c_outline.write(f))
		return ret;
	return 0;
}

SW_Time::SW_Time(byte ty) : SubscrWidget(ty){}
SW_Time::SW_Time(subscreen_object const& old) : SW_Time()
{
	load_old(old);
}
bool SW_Time::load_old(subscreen_object const& old)
{
	if(old.type != ssoBSTIME && old.type != ssoTIME
		&& old.type != ssoSSTIME)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	align = old.d2;
	shadtype = old.d3;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
int16_t SW_Time::getX() const
{
	return x+shadow_x(shadtype);
}
int16_t SW_Time::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Time::getW() const
{
	char *ts;
	auto tm = game ? game->get_time() : 0;
	switch(type)
	{
		case ssoBSTIME:
			ts = time_str_short2(tm);
			break;
		case ssoTIME:
		case ssoSSTIME:
			ts = time_str_med(tm);
			break;
	}
	return text_length(get_zc_font(fontid), ts) + shadow_w(shadtype);
}
word SW_Time::getH() const
{
	return text_height(get_zc_font(fontid)) + shadow_h(shadtype);
}
int16_t SW_Time::getXOffs() const
{
	switch(align)
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_Time::getType() const
{
	return type; //ssoBSTIME,ssoTIME,ssoSSTIME
}
void SW_Time::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	char *ts;
	auto tm = game ? game->get_time() : 0;
	switch(type)
	{
		case ssoBSTIME:
			ts = time_str_short2(tm);
			break;
		case ssoTIME:
		case ssoSSTIME:
			ts = time_str_med(tm);
			break;
	}
	FONT* tempfont = get_zc_font(fontid);
	textout_styled_aligned_ex(dest,tempfont,ts,getX()+xofs,getY()+yofs,
		shadtype,align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color());
}
bool SW_Time::visible(byte pos, bool showtime) const
{
	return showtime && SubscrWidget::visible(pos,showtime);
}
SubscrWidget* SW_Time::clone() const
{
	return new SW_Time(*this);
}
bool SW_Time::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Time const* other = dynamic_cast<SW_Time const*>(src);
	SubscrWidget::copy_prop(other,all);
	fontid = other->fontid;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	return true;
}
int32_t SW_Time::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&fontid,f))
		return qe_invalid;
	if(!p_getc(&align,f))
		return qe_invalid;
	if(!p_getc(&shadtype,f))
		return qe_invalid;
	if(auto ret = c_text.read(f,s_version))
		return ret;
	if(auto ret = c_shadow.read(f,s_version))
		return ret;
	if(auto ret = c_bg.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_Time::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(2);
	if(!p_putc(shadtype,f))
		new_return(3);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	return 0;
}

SW_MagicMeter::SW_MagicMeter(subscreen_object const& old) : SW_MagicMeter()
{
	load_old(old);
}
bool SW_MagicMeter::load_old(subscreen_object const& old)
{
	if(old.type != ssoMAGICMETER)
		return false;
	SubscrWidget::load_old(old);
	return true;
}
int16_t SW_MagicMeter::getX() const
{
	return x-10;
}
word SW_MagicMeter::getW() const
{
	return 82;
}
word SW_MagicMeter::getH() const
{
	return 8;
}
byte SW_MagicMeter::getType() const
{
	return ssoMAGICMETER;
}
void SW_MagicMeter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	magicmeter(dest, getX()+xofs, getY()+yofs);
}
SubscrWidget* SW_MagicMeter::clone() const
{
	return new SW_MagicMeter(*this);
}
bool SW_MagicMeter::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_MagicMeter const* other = dynamic_cast<SW_MagicMeter const*>(src);
	SubscrWidget::copy_prop(other,all);
	return true;
}
int32_t SW_MagicMeter::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_MagicMeter::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	return 0;
}

SW_LifeMeter::SW_LifeMeter(subscreen_object const& old) : SW_LifeMeter()
{
	load_old(old);
}
bool SW_LifeMeter::load_old(subscreen_object const& old)
{
	if(old.type != ssoLIFEMETER)
		return false;
	SubscrWidget::load_old(old);
	rows = old.d3 ? 3 : 2;
	SETFLAG(flags,SUBSCR_LIFEMET_BOT,old.d2);
	return true;
}
int16_t SW_LifeMeter::getY() const
{
	if(flags&SUBSCR_LIFEMET_BOT)
		return y;
	return (4-rows)*8;
}
word SW_LifeMeter::getW() const
{
	return 64;
}
word SW_LifeMeter::getH() const
{
	return 8*rows;
}
byte SW_LifeMeter::getType() const
{
	return ssoLIFEMETER;
}
void SW_LifeMeter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	lifemeter(dest, getX()+xofs, getY()+yofs, 1, flags&SUBSCR_LIFEMET_BOT);
}
SubscrWidget* SW_LifeMeter::clone() const
{
	return new SW_LifeMeter(*this);
}
bool SW_LifeMeter::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_LifeMeter const* other = dynamic_cast<SW_LifeMeter const*>(src);
	SubscrWidget::copy_prop(other,all);
	rows = other->rows;
	return true;
}
int32_t SW_LifeMeter::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_getc(&rows,f))
		return qe_invalid;
	return 0;
}
int32_t SW_LifeMeter::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_putc(rows,f))
		new_return(1);
	return 0;
}

SW_ButtonItem::SW_ButtonItem(subscreen_object const& old) : SW_ButtonItem()
{
	load_old(old);
}
bool SW_ButtonItem::load_old(subscreen_object const& old)
{
	if(old.type != ssoBUTTONITEM)
		return false;
	SubscrWidget::load_old(old);
	btn = old.d1;
	SETFLAG(flags,SUBSCR_BTNITM_TRANSP,old.d2);
	return true;
}
word SW_ButtonItem::getW() const
{
	return 16;
}
word SW_ButtonItem::getH() const
{
	return 16;
}
byte SW_ButtonItem::getType() const
{
	return ssoBUTTONITEM;
}
void SW_ButtonItem::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(flags&SUBSCR_BTNITM_TRANSP)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	
	buttonitem(dest, btn, x, y);
	
	if(flags&SUBSCR_BTNITM_TRANSP)
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}
SubscrWidget* SW_ButtonItem::clone() const
{
	return new SW_ButtonItem(*this);
}
bool SW_ButtonItem::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_ButtonItem const* other = dynamic_cast<SW_ButtonItem const*>(src);
	SubscrWidget::copy_prop(other,all);
	btn = other->btn;
	return true;
}
int32_t SW_ButtonItem::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_getc(&btn,f))
		return qe_invalid;
	return 0;
}
int32_t SW_ButtonItem::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_putc(btn,f))
		new_return(1);
	return 0;
}

SW_Counter::SW_Counter(subscreen_object const& old) : SW_Counter()
{
	load_old(old);
}
bool SW_Counter::load_old(subscreen_object const& old)
{
	if(old.type != ssoCOUNTER)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	align = old.d2;
	shadtype = old.d3;
	ctrs[0] = old.d7;
	ctrs[1] = old.d8;
	ctrs[2] = old.d9;
	SETFLAG(flags,SUBSCR_COUNTER_SHOW0,old.d6&0b01);
	SETFLAG(flags,SUBSCR_COUNTER_ONLYSEL,old.d6&0b10);
	digits = old.d4;
	infitm = old.d10;
	infchar = old.d5;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
int16_t SW_Counter::getX() const
{
	return x+shadow_x(shadtype);
}
int16_t SW_Counter::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Counter::getW() const
{
	return text_length(get_zc_font(fontid), "0")*zc_max(1,digits) + shadow_w(shadtype);
}
word SW_Counter::getH() const
{
	return text_height(get_zc_font(fontid)) + shadow_h(shadtype);
}
int16_t SW_Counter::getXOffs() const
{
	switch(align)
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_Counter::getType() const
{
	return ssoCOUNTER;
}
void SW_Counter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	counter(dest, getX()+xofs,getY()+yofs, tempfont, c_text.get_color(),
		c_shadow.get_color(), c_bg.get_color(),align,shadtype,digits,infchar,
		flags&SUBSCR_COUNTER_SHOW0, ctrs[0], ctrs[1], ctrs[2], infitm,
		flags&SUBSCR_COUNTER_ONLYSEL);
}
SubscrWidget* SW_Counter::clone() const
{
	return new SW_Counter(*this);
}
bool SW_Counter::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Counter const* other = dynamic_cast<SW_Counter const*>(src);
	SubscrWidget::copy_prop(other,all);
	fontid = other->fontid;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	ctrs[0] = other->ctrs[0];
	ctrs[1] = other->ctrs[1];
	ctrs[2] = other->ctrs[2];
	digits = other->digits;
	infitm = other->infitm;
	infchar = other->infchar;
	return true;
}
int32_t SW_Counter::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&fontid,f))
		return qe_invalid;
	if(!p_getc(&align,f))
		return qe_invalid;
	if(!p_getc(&shadtype,f))
		return qe_invalid;
	if(auto ret = c_text.read(f,s_version))
		return ret;
	if(auto ret = c_shadow.read(f,s_version))
		return ret;
	if(auto ret = c_bg.read(f,s_version))
		return ret;
	if(!p_igetl(&ctrs[0],f))
		return qe_invalid;
	if(!p_igetl(&ctrs[1],f))
		return qe_invalid;
	if(!p_igetl(&ctrs[2],f))
		return qe_invalid;
	if(!p_getc(&digits,f))
		return qe_invalid;
	if(!p_igetl(&infitm,f))
		return qe_invalid;
	if(!p_getc(&infchar,f))
		return qe_invalid;
	return 0;
}
int32_t SW_Counter::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(2);
	if(!p_putc(shadtype,f))
		new_return(3);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	if(!p_iputl(ctrs[0],f))
		new_return(4);
	if(!p_iputl(ctrs[1],f))
		new_return(5);
	if(!p_iputl(ctrs[2],f))
		new_return(6);
	if(!p_putc(digits,f))
		new_return(7);
	if(!p_iputl(infitm,f))
		new_return(8);
	if(!p_putc(infchar,f))
		new_return(9);
	return 0;
}

SW_Counters::SW_Counters(subscreen_object const& old) : SW_Counters()
{
	load_old(old);
}
bool SW_Counters::load_old(subscreen_object const& old)
{
	if(old.type != ssoCOUNTERS)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	SETFLAG(flags,SUBSCR_COUNTERS_USEX,old.d2);
	shadtype = old.d3;
	digits = old.d4;
	infitm = old.d10;
	infchar = old.d5;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
int16_t SW_Counters::getX() const
{
	return x+shadow_x(shadtype);
}
int16_t SW_Counters::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Counters::getW() const
{
	return 32 + shadow_w(shadtype);
}
word SW_Counters::getH() const
{
	return 32 + shadow_h(shadtype);
}
byte SW_Counters::getType() const
{
	return ssoCOUNTERS;
}
void SW_Counters::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	defaultcounters(dest, getX()+xofs, getY()+yofs, tempfont, c_text.get_color(),
		c_shadow.get_color(), c_bg.get_color(),flags&SUBSCR_COUNTERS_USEX,shadtype,
		digits,infchar);
}
SubscrWidget* SW_Counters::clone() const
{
	return new SW_Counters(*this);
}
bool SW_Counters::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Counters const* other = dynamic_cast<SW_Counters const*>(src);
	SubscrWidget::copy_prop(other,all);
	fontid = other->fontid;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	digits = other->digits;
	infitm = other->infitm;
	infchar = other->infchar;
	return true;
}
int32_t SW_Counters::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&fontid,f))
		return qe_invalid;
	if(!p_getc(&shadtype,f))
		return qe_invalid;
	if(auto ret = c_text.read(f,s_version))
		return ret;
	if(auto ret = c_shadow.read(f,s_version))
		return ret;
	if(auto ret = c_bg.read(f,s_version))
		return ret;
	if(!p_getc(&digits,f))
		return qe_invalid;
	if(!p_igetl(&infitm,f))
		return qe_invalid;
	if(!p_getc(&infchar,f))
		return qe_invalid;
	return 0;
}
int32_t SW_Counters::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(2);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	if(!p_putc(digits,f))
		new_return(3);
	if(!p_iputl(infitm,f))
		new_return(4);
	if(!p_putc(infchar,f))
		new_return(5);
	return 0;
}

SW_MMapTitle::SW_MMapTitle(subscreen_object const& old) : SW_MMapTitle()
{
	load_old(old);
}
bool SW_MMapTitle::load_old(subscreen_object const& old)
{
	if(old.type != ssoMINIMAPTITLE)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	align = old.d2;
	SETFLAG(flags,SUBSCR_MMAPTIT_REQMAP,old.d4);
	shadtype = old.d3;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
word SW_MMapTitle::getW() const
{
	return 80;
}
word SW_MMapTitle::getH() const
{
	return 16;
}
int16_t SW_MMapTitle::getXOffs() const
{
	switch(align)
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_MMapTitle::getType() const
{
	return ssoMINIMAPTITLE;
}
void SW_MMapTitle::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	if(!(flags&SUBSCR_MMAPTIT_REQMAP) || has_item(itype_map, get_dlevel()))
		minimaptitle(dest, getX()+xofs, getY()+yofs, tempfont, c_text.get_color(),
			c_shadow.get_color(),c_bg.get_color(), align, shadtype);
}
SubscrWidget* SW_MMapTitle::clone() const
{
	return new SW_MMapTitle(*this);
}
bool SW_MMapTitle::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_MMapTitle const* other = dynamic_cast<SW_MMapTitle const*>(src);
	SubscrWidget::copy_prop(other,all);
	fontid = other->fontid;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	return true;
}
int32_t SW_MMapTitle::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&fontid,f))
		return qe_invalid;
	if(!p_getc(&align,f))
		return qe_invalid;
	if(!p_getc(&shadtype,f))
		return qe_invalid;
	if(auto ret = c_text.read(f,s_version))
		return ret;
	if(auto ret = c_shadow.read(f,s_version))
		return ret;
	if(auto ret = c_bg.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_MMapTitle::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(2);
	if(!p_putc(shadtype,f))
		new_return(3);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	return 0;
}

SW_MMap::SW_MMap(subscreen_object const& old) : SW_MMap()
{
	load_old(old);
}
bool SW_MMap::load_old(subscreen_object const& old)
{
	if(old.type != ssoMINIMAP)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_MMAP_SHOWMAP,old.d1);
	SETFLAG(flags,SUBSCR_MMAP_SHOWPLR,old.d2);
	SETFLAG(flags,SUBSCR_MMAP_SHOWCMP,old.d3);
	c_plr.load_old(old,1);
	c_cmp_blink.load_old(old,2);
	c_cmp_off.load_old(old,3);
	return true;
}
word SW_MMap::getW() const
{
	return 80;
}
word SW_MMap::getH() const
{
	return 48;
}
byte SW_MMap::getType() const
{
	return ssoMINIMAP;
}
void SW_MMap::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	bool showplr = (flags&SUBSCR_MMAP_SHOWPLR) && !(TheMaps[(DMaps[get_currdmap()].map*MAPSCRS)+get_homescr()].flags7&fNOHEROMARK);
	bool showcmp = (flags&SUBSCR_MMAP_SHOWCMP) && !(DMaps[get_currdmap()].flags&dmfNOCOMPASS);
	drawdmap(dest, getX()+xofs, getY()+yofs, flags&SUBSCR_MMAP_SHOWMAP, showplr,
		showcmp, c_plr.get_color(), c_cmp_blink.get_color(), c_cmp_off.get_color());
}
SubscrWidget* SW_MMap::clone() const
{
	return new SW_MMap(*this);
}
bool SW_MMap::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_MMap const* other = dynamic_cast<SW_MMap const*>(src);
	SubscrWidget::copy_prop(other,all);
	c_plr = other->c_plr;
	c_cmp_blink = other->c_cmp_blink;
	c_cmp_off = other->c_cmp_off;
	return true;
}
int32_t SW_MMap::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(auto ret = c_plr.read(f,s_version))
		return ret;
	if(auto ret = c_cmp_blink.read(f,s_version))
		return ret;
	if(auto ret = c_cmp_off.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_MMap::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(auto ret = c_plr.write(f))
		return ret;
	if(auto ret = c_cmp_blink.write(f))
		return ret;
	if(auto ret = c_cmp_off.write(f))
		return ret;
	return 0;
}

SW_LMap::SW_LMap(subscreen_object const& old) : SW_LMap()
{
	load_old(old);
}
bool SW_LMap::load_old(subscreen_object const& old)
{
	if(old.type != ssoLARGEMAP)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_LMAP_SHOWMAP,old.d1);
	SETFLAG(flags,SUBSCR_LMAP_SHOWROOM,old.d2);
	SETFLAG(flags,SUBSCR_LMAP_SHOWPLR,old.d3);
	SETFLAG(flags,SUBSCR_LMAP_LARGE,old.d10);
	c_room.load_old(old,1);
	c_plr.load_old(old,2);
	return true;
}
word SW_LMap::getW() const
{
	return 16*((flags&SUBSCR_LMAP_LARGE)?9:7);
}
word SW_LMap::getH() const
{
	return 80;
}
byte SW_LMap::getType() const
{
	return ssoLARGEMAP;
}
void SW_LMap::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	putBmap(dest, getX()+xofs, getY()+yofs, flags&SUBSCR_LMAP_SHOWMAP,
		flags&SUBSCR_LMAP_SHOWROOM, flags&SUBSCR_LMAP_SHOWPLR, c_room.get_color(),
		c_plr.get_color(), flags&SUBSCR_LMAP_LARGE);
}
SubscrWidget* SW_LMap::clone() const
{
	return new SW_LMap(*this);
}
bool SW_LMap::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_LMap const* other = dynamic_cast<SW_LMap const*>(src);
	SubscrWidget::copy_prop(other,all);
	c_room = other->c_room;
	c_plr = other->c_plr;
	return true;
}
int32_t SW_LMap::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(auto ret = c_room.read(f,s_version))
		return ret;
	if(auto ret = c_plr.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_LMap::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(auto ret = c_room.write(f))
		return ret;
	if(auto ret = c_plr.write(f))
		return ret;
	return 0;
}

SW_Clear::SW_Clear(subscreen_object const& old) : SW_Clear()
{
	load_old(old);
}
bool SW_Clear::load_old(subscreen_object const& old)
{
	if(old.type != ssoCLEAR)
		return false;
	SubscrWidget::load_old(old);
	c_bg.load_old(old,1);
	return true;
}
word SW_Clear::getW() const
{
	return 5;
}
word SW_Clear::getH() const
{
	return 5;
}
byte SW_Clear::getType() const
{
	return ssoCLEAR;
}
void SW_Clear::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	clear_to_color(dest,c_bg.get_color());
}
SubscrWidget* SW_Clear::clone() const
{
	return new SW_Clear(*this);
}
bool SW_Clear::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Clear const* other = dynamic_cast<SW_Clear const*>(src);
	SubscrWidget::copy_prop(other,all);
	c_bg = other->c_bg;
	return true;
}
int32_t SW_Clear::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(auto ret =  c_bg.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_Clear::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(auto ret =  c_bg.write(f))
		return ret;
	return 0;
}

SW_CurrentItem::SW_CurrentItem(subscreen_object const& old) : SW_CurrentItem()
{
	load_old(old);
}
bool SW_CurrentItem::load_old(subscreen_object const& old)
{
	if(old.type != ssoCURRENTITEM)
		return false;
	SubscrWidget::load_old(old);
	iclass = old.d1;
	iid = old.d8;
	pos = old.d3;
	pos_up = old.d4;
	pos_down = old.d5;
	pos_left = old.d6;
	pos_right = old.d7;
	SETFLAG(flags,SUBSCRFLAG_SELECTABLE,pos>=0);
	SETFLAG(flags,SUBSCR_CURITM_INVIS,!(old.d2&0x1));
	SETFLAG(flags,SUBSCR_CURITM_NONEQP,old.d2&0x2);
	return true;
}
word SW_CurrentItem::getW() const
{
	return 16;
}
word SW_CurrentItem::getH() const
{
	return 16;
}
byte SW_CurrentItem::getType() const
{
	return ssoCURRENTITEM;
}
int32_t SW_CurrentItem::getItemVal() const
{
	#ifdef IS_PLAYER
	if(iid > 0)
	{
		bool select = false;
		switch(itemsbuf[iid-1].family)
		{
			case itype_bomb:
				if((game->get_bombs() ||
						// Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
						(itemsbuf[iid-1].misc1==0 && findWeaponWithParent(iid-1, wLitBomb))) ||
						current_item_power(itype_bombbag))
				{
					select=true;
				}
				break;
			case itype_bowandarrow:
			case itype_arrow:
				if(current_item_id(itype_bow)>-1)
				{
					select=true;
				}
				break;
			case itype_letterpotion:
				break;
			case itype_sbomb:
			{
				int32_t bombbagid = current_item_id(itype_bombbag);
				
				if((game->get_sbombs() ||
						// Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
						(itemsbuf[iid-1].misc1==0 && findWeaponWithParent(iid-1, wLitSBomb))) ||
						(current_item_power(itype_bombbag) && bombbagid>-1 && (itemsbuf[bombbagid].flags & ITEM_FLAG1)))
				{
					select=true;
				}
				break;
			}
			case itype_sword:
			{
				if(get_qr(qr_SELECTAWPN))
					select=true;
				break;
			}
			default:
				select = true;
				break;
		}
		if(select && !item_disabled(iid-1) && game->get_item(iid-1))
		{
			directItem = iid-1;
			auto ret = iid-1;
			if(directItem>-1 && itemsbuf[directItem].family == itype_arrow)
				ret += 0xF000; //bow
			return ret;
		}
		else return -1;
	}
	int32_t family = -1;
	switch(iclass)
	{
		case itype_bomb:
		{
			int32_t bombid = current_item_id(itype_bomb);
			
			if((game->get_bombs() ||
					// Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
					(bombid>-1 && itemsbuf[bombid].misc1==0 && Lwpns.idCount(wLitBomb)>0)) ||
					current_item_power(itype_bombbag))
			{
				family=itype_bomb;
			}
			break;
		}
		case itype_bowandarrow:
		case itype_arrow:
			if(current_item_id(itype_bow)>-1 && current_item_id(itype_arrow)>-1)
			{
				family=itype_arrow;
			}
			break;
		case itype_letterpotion:
			if(current_item_id(itype_potion)>-1)
				family=itype_potion;
			else if(current_item_id(itype_letter)>-1)
				family=itype_letter;
			break;
		case itype_sbomb:
		{
			int32_t bombbagid = current_item_id(itype_bombbag);
			int32_t sbombid = current_item_id(itype_sbomb);
			
			if((game->get_sbombs() ||
					// Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
					(sbombid>-1 && itemsbuf[sbombid].misc1==0 && Lwpns.idCount(wLitSBomb)>0)) ||
					(current_item_power(itype_bombbag) && bombbagid>-1 && (itemsbuf[bombbagid].flags & ITEM_FLAG1)))
			{
				family=itype_sbomb;
			}
			break;
		}
		case itype_sword:
		{
			if(get_qr(qr_SELECTAWPN))
				family=itype_sword;
			break;
		}
		default:
			family = iclass;
			break;
	}
	if(family < 0)
		return -1;
	int32_t itemid = current_item_id(family,false);
	if(item_disabled(itemid))
		return -1;
	if(iclass == itype_bowandarrow)
		return itemid+0xF000;
	return itemid;
	#else
	return iid>0 ? ((iid-1) | 0x8000) : iclass;
	#endif
}
void SW_CurrentItem::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	#ifdef IS_PLAYER
	if(flags&SUBSCR_CURITM_INVIS)
		return;
	#else
	if((flags&SUBSCR_CURITM_INVIS) && !(zinit.ss_flags&ssflagSHOWINVIS))
		return;
	#endif
	subscreenitem(dest, getX()+xofs,getY()+yofs, getItemVal());
}
SubscrWidget* SW_CurrentItem::clone() const
{
	return new SW_CurrentItem(*this);
}
bool SW_CurrentItem::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_CurrentItem const* other = dynamic_cast<SW_CurrentItem const*>(src);
	SubscrWidget::copy_prop(other,all);
	iid = other->iid;
	iclass = other->iclass;
	return true;
}
int32_t SW_CurrentItem::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&iclass,f))
		return qe_invalid;
	if(!p_igetl(&iid,f))
		return qe_invalid;
	return 0;
}
int32_t SW_CurrentItem::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(iclass,f))
		new_return(1);
	if(!p_iputl(iid,f))
		new_return(2);
	return 0;
}

SW_TriFrame::SW_TriFrame(subscreen_object const& old) : SW_TriFrame()
{
	load_old(old);
}
bool SW_TriFrame::load_old(subscreen_object const& old)
{
	if(old.type != ssoTRIFRAME)
		return false;
	SubscrWidget::load_old(old);
	frame_tile = old.d1;
	frame_cset = old.d2;
	piece_tile = old.d3;
	piece_cset = old.d4;
	SETFLAG(flags,SUBSCR_TRIFR_SHOWFR,old.d5);
	SETFLAG(flags,SUBSCR_TRIFR_SHOWPC,old.d6);
	SETFLAG(flags,SUBSCR_TRIFR_LGPC,old.d7);
	c_outline.load_old(old,1);
	c_number.load_old(old,2);
	return true;
}
word SW_TriFrame::getW() const
{
	return 16*((flags&SUBSCR_TRIFR_LGPC)?7:6);
}
word SW_TriFrame::getH() const
{
	return 16*((flags&SUBSCR_TRIFR_LGPC)?7:3);
}
byte SW_TriFrame::getType() const
{
	return ssoTRIFRAME;
}
void SW_TriFrame::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	puttriframe(dest, getX()+xofs,getY()+yofs, c_outline.get_color(), c_number.get_color(),
		frame_tile, frame_cset, piece_tile, piece_cset, flags&SUBSCR_TRIFR_SHOWFR,
		flags&SUBSCR_TRIFR_SHOWPC, flags&SUBSCR_TRIFR_LGPC);
}
SubscrWidget* SW_TriFrame::clone() const
{
	return new SW_TriFrame(*this);
}
bool SW_TriFrame::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_TriFrame const* other = dynamic_cast<SW_TriFrame const*>(src);
	SubscrWidget::copy_prop(other,all);
	frame_tile = other->frame_tile;
	piece_tile = other->piece_tile;
	frame_cset = other->frame_cset;
	piece_cset = other->piece_cset;
	c_outline = other->c_outline;
	c_number = other->c_number;
	return true;
}
int32_t SW_TriFrame::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&frame_tile,f))
		return qe_invalid;
	if(!p_igetl(&piece_tile,f))
		return qe_invalid;
	if(!p_getc(&frame_cset,f))
		return qe_invalid;
	if(!p_getc(&piece_cset,f))
		return qe_invalid;
	if(auto ret =  c_outline.read(f,s_version))
		return ret;
	if(auto ret =  c_number.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_TriFrame::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(frame_tile,f))
		new_return(1);
	if(!p_iputl(piece_tile,f))
		new_return(2);
	if(!p_putc(frame_cset,f))
		new_return(3);
	if(!p_putc(piece_cset,f))
		new_return(4);
	if(auto ret =  c_outline.write(f))
		return ret;
	if(auto ret =  c_number.write(f))
		return ret;
	return 0;
}

SW_McGuffin::SW_McGuffin(subscreen_object const& old) : SW_McGuffin()
{
	load_old(old);
}
bool SW_McGuffin::load_old(subscreen_object const& old)
{
	if(old.type != ssoMCGUFFIN)
		return false;
	SubscrWidget::load_old(old);
	tile = old.d1;
	cset = old.d2;
	number = old.d5;
	SETFLAG(flags,SUBSCR_MCGUF_OVERLAY,old.d3);
	SETFLAG(flags,SUBSCR_MCGUF_TRANSP,old.d4);
	cs.load_old(old,1);
	return true;
}
word SW_McGuffin::getW() const
{
	return 16;
}
word SW_McGuffin::getH() const
{
	return 16;
}
byte SW_McGuffin::getType() const
{
	return ssoMCGUFFIN;
}
void SW_McGuffin::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	puttriforce(dest,getX()+xofs,getY()+yofs,tile,cs.get_cset(),w,h,
		cset,flags&SUBSCR_MCGUF_OVERLAY,flags&SUBSCR_MCGUF_TRANSP,number);
}
SubscrWidget* SW_McGuffin::clone() const
{
	return new SW_McGuffin(*this);
}
bool SW_McGuffin::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_McGuffin const* other = dynamic_cast<SW_McGuffin const*>(src);
	SubscrWidget::copy_prop(other,all);
	tile = other->tile;
	number = other->number;
	cset = other->cset;
	cs = other->cs;
	return true;
}
int32_t SW_McGuffin::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&tile,f))
		return qe_invalid;
	if(!p_igetl(&number,f))
		return qe_invalid;
	if(!p_getc(&cset,f))
		return qe_invalid;
	if(auto ret =  cs.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_McGuffin::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(tile,f))
		new_return(1);
	if(!p_iputl(number,f))
		new_return(2);
	if(!p_putc(cset,f))
		new_return(3);
	if(auto ret =  cs.write(f))
		return ret;
	return 0;
}

SW_TileBlock::SW_TileBlock(subscreen_object const& old) : SW_TileBlock()
{
	load_old(old);
}
bool SW_TileBlock::load_old(subscreen_object const& old)
{
	if(old.type != ssoTILEBLOCK)
		return false;
	SubscrWidget::load_old(old);
	tile = old.d1;
	flip = old.d2;
	SETFLAG(flags,SUBSCR_TILEBL_OVERLAY,old.d3);
	SETFLAG(flags,SUBSCR_TILEBL_TRANSP,old.d4);
	cs.load_old(old,1);
	return true;
}
word SW_TileBlock::getW() const
{
	return w * 16;
}
word SW_TileBlock::getH() const
{
	return h * 16;
}
byte SW_TileBlock::getType() const
{
	return ssoTILEBLOCK;
}
void SW_TileBlock::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	draw_block_flip(dest,getX()+xofs,getY()+yofs,tile,cs.get_cset(),
		w,h,flip,flags&SUBSCR_TILEBL_OVERLAY,flags&SUBSCR_TILEBL_TRANSP);
}
SubscrWidget* SW_TileBlock::clone() const
{
	return new SW_TileBlock(*this);
}
bool SW_TileBlock::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_TileBlock const* other = dynamic_cast<SW_TileBlock const*>(src);
	SubscrWidget::copy_prop(other,all);
	tile = other->tile;
	flip = other->flip;
	cs = other->cs;
	return true;
}
int32_t SW_TileBlock::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&tile,f))
		return qe_invalid;
	if(!p_getc(&flip,f))
		return qe_invalid;
	if(auto ret =  cs.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_TileBlock::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(tile,f))
		new_return(1);
	if(!p_putc(flip,f))
		new_return(2);
	if(auto ret =  cs.write(f))
		return ret;
	return 0;
}

SW_MiniTile::SW_MiniTile(subscreen_object const& old) : SW_MiniTile()
{
	load_old(old);
}
bool SW_MiniTile::load_old(subscreen_object const& old)
{
	if(old.type != ssoMINITILE)
		return false;
	SubscrWidget::load_old(old);
	if(old.d1 == -1)
	{
		tile = -1;
		crn = old.d3;
	}
	else
	{
		tile = old.d1>>2;
		crn = old.d1&0b11;
	}
	special_tile = old.d2;
	flip = old.d4;
	SETFLAG(flags,SUBSCR_MINITL_OVERLAY,old.d5);
	SETFLAG(flags,SUBSCR_MINITL_TRANSP,old.d6);
	cs.load_old(old,1);
	return true;
}
word SW_MiniTile::getW() const
{
	return 8;
}
word SW_MiniTile::getH() const
{
	return 8;
}
byte SW_MiniTile::getType() const
{
	return ssoMINITILE;
}
int32_t SW_MiniTile::get_tile() const
{
	if(tile == -1)
	{
		switch(special_tile)
		{
			case ssmstSSVINETILE:
				return wpnsbuf[iwSubscreenVine].tile;
			case ssmstMAGICMETER:
				return wpnsbuf[iwMMeter].tile;
			default:
				return (zc_oldrand()*100000)%32767;
		}
	}
	else return tile;
}
void SW_MiniTile::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	auto t = (get_tile()<<2)+crn;
	auto tx = getX()+xofs, ty = getY()+yofs;
	byte cset = cs.get_cset();
	if(flags&SUBSCR_MINITL_OVERLAY)
	{
		if(flags&SUBSCR_MINITL_TRANSP)
			overtiletranslucent8(dest,t,tx,ty,cset,flip,128);
		else
			overtile8(dest,t,tx,ty,cset,flip);
	}
	else
	{
		if(flags&SUBSCR_MINITL_TRANSP)
			puttiletranslucent8(dest,t,tx,ty,cset,flip,128);
		else
			oldputtile8(dest,t,tx,ty,cset,flip);
	}
}
SubscrWidget* SW_MiniTile::clone() const
{
	return new SW_MiniTile(*this);
}
bool SW_MiniTile::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_MiniTile const* other = dynamic_cast<SW_MiniTile const*>(src);
	SubscrWidget::copy_prop(other,all);
	tile = other->tile;
	special_tile = other->special_tile;
	crn = other->crn;
	flip = other->flip;
	cs = other->cs;
	return true;
}
int32_t SW_MiniTile::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&tile,f))
		return qe_invalid;
	if(!p_igetl(&special_tile,f))
		return qe_invalid;
	if(!p_getc(&crn,f))
		return qe_invalid;
	if(!p_getc(&flip,f))
		return qe_invalid;
	if(auto ret =  cs.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_MiniTile::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(tile,f))
		new_return(1);
	if(!p_iputl(special_tile,f))
		new_return(2);
	if(!p_putc(crn,f))
		new_return(3);
	if(!p_putc(flip,f))
		new_return(4);
	if(auto ret =  cs.write(f))
		return ret;
	return 0;
}

SW_Selector::SW_Selector(byte ty) : SubscrWidget(ty)
{
	SETFLAG(flags, SUBSCR_SELECTOR_USEB, ty==ssoSELECTOR2);
}
SW_Selector::SW_Selector(subscreen_object const& old) : SW_Selector()
{
	load_old(old);
}
bool SW_Selector::load_old(subscreen_object const& old)
{
	if(old.type != ssoSELECTOR1 && old.type != ssoSELECTOR2)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_SELECTOR_TRANSP,old.d4);
	SETFLAG(flags,SUBSCR_SELECTOR_LARGE,old.d5);
	SETFLAG(flags,SUBSCR_SELECTOR_USEB,old.type==ssoSELECTOR2);
	return true;
}
word SW_Selector::getW() const
{
	return (flags&SUBSCR_SELECTOR_LARGE)?32:16;
}
word SW_Selector::getH() const
{
	return (flags&SUBSCR_SELECTOR_LARGE)?32:16;
}
byte SW_Selector::getType() const
{
	return ssoSELECTOR1;
}
void SW_Selector::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	int32_t p=-1;
	
	SubscrWidget* selitm = nullptr;
	for(size_t j=0; j < page.contents.size(); ++j)
	{
		SubscrWidget& w = *page.contents[j];
		if(w.getType()==ssoCURRENTITEM)
		{
			if(w.pos==page.cursor_pos)
			{
				p=j;
				selitm = &w;
				break;
			}
		}
	}
	
	bool big_sel=flags&SUBSCR_SELECTOR_LARGE;
	item *tempsel=(flags&SUBSCR_SELECTOR_USEB)?sel_b:sel_a;
	int32_t temptile=tempsel->tile;
	tempsel->drawstyle=0;
	
	if(flags&SUBSCR_SELECTOR_TRANSP)
		tempsel->drawstyle=1;
	int32_t itemtype = selitm ? selitm->getItemVal() : -1;
	itemdata const& tmpitm = itemsbuf[get_subscreenitem_id(itemtype, true)];
	bool oldsel = get_qr(qr_SUBSCR_OLD_SELECTOR);
	if(!oldsel) big_sel = false;
	int32_t sw = oldsel ? (tempsel->extend > 2 ? tempsel->txsz*16 : 16) : (tempsel->extend > 2 ? tempsel->hit_width : 16),
		sh = oldsel ? (tempsel->extend > 2 ? tempsel->txsz*16 : 16) : (tempsel->extend > 2 ? tempsel->hit_height : 16),
		dw = oldsel ? (tempsel->extend > 2 ? tempsel->txsz*16 : 16) : ((tmpitm.overrideFLAGS & itemdataOVERRIDE_HIT_WIDTH) ? tmpitm.hxsz : 16),
		dh = oldsel ? (tempsel->extend > 2 ? tempsel->txsz*16 : 16) : ((tmpitm.overrideFLAGS & itemdataOVERRIDE_HIT_HEIGHT) ? tmpitm.hysz : 16);
	int32_t sxofs = oldsel ? 0 : (tempsel->extend > 2 ? tempsel->hxofs : 0),
		syofs = oldsel ? 0 : (tempsel->extend > 2 ? tempsel->hyofs : 0),
		dxofs = oldsel ? (tempsel->extend > 2 ? (int)tempsel->xofs : 0) : ((tmpitm.overrideFLAGS & itemdataOVERRIDE_HIT_X_OFFSET) ? tmpitm.hxofs : 0) + (tempsel->extend > 2 ? (int)tempsel->xofs : 0),
		dyofs = oldsel ? (tempsel->extend > 2 ? (int)tempsel->yofs : 0) : ((tmpitm.overrideFLAGS & itemdataOVERRIDE_HIT_Y_OFFSET) ? tmpitm.hyofs : 0) + (tempsel->extend > 2 ? (int)tempsel->yofs : 0);
	BITMAP* tmpbmp = create_bitmap_ex(8,sw,sh);
	for(int32_t j=0; j<4; ++j)
	{
		clear_bitmap(tmpbmp);
		if(selitm)
		{
			tempsel->x=0;
			tempsel->y=0;
			int32_t tmpx = selitm->x+xofs+(big_sel?(j%2?8:-8):0);
			int32_t tmpy = selitm->y+yofs+(big_sel?(j>1?8:-8):0);
			tempsel->tile+=(zc_max(itemsbuf[tempsel->id].frames,1)*j);
			
			if(temptile)
			{
				tempsel->drawzcboss(tmpbmp);
				tempsel->tile=temptile;
			}
			masked_stretch_blit(tmpbmp, dest, vbound(sxofs, 0, sw), vbound(syofs, 0, sh), sw-vbound(sxofs, 0, sw), sh-vbound(syofs, 0, sh), tmpx+dxofs, tmpy+dyofs, dw, dh);
			
			if(!big_sel)
				break;
		}
	}
	destroy_bitmap(tmpbmp);
}
SubscrWidget* SW_Selector::clone() const
{
	return new SW_Selector(*this);
}
bool SW_Selector::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Selector const* other = dynamic_cast<SW_Selector const*>(src);
	SubscrWidget::copy_prop(other,all);
	return true;
}
int32_t SW_Selector::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_Selector::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	return 0;
}


SW_Temp::SW_Temp(byte ty) : SubscrWidget(ssoTEMPOLD)
{
	old.type = ty;
}
SW_Temp::SW_Temp(subscreen_object const& old) : SW_Temp()
{
	load_old(old);
}
SW_Temp::~SW_Temp()
{
	if(old.dp1)
	{
		delete[] old.dp1;
		old.dp1 = nullptr;
	}
}
bool SW_Temp::load_old(subscreen_object const& _old)
{
	type = ssoTEMPOLD;
	old = _old;
	if(old.dp1)
	{
		old.dp1 = new char[strlen((char*)_old.dp1)+1];
		strcpy((char*)old.dp1,(char*)_old.dp1);
	}
	return true;
}
int16_t SW_Temp::getX() const
{
	return sso_x(&old);
}
int16_t SW_Temp::getY() const
{
	return sso_y(&old);
}
word SW_Temp::getW() const
{
	return sso_w(&old);
}
word SW_Temp::getH() const
{
	return sso_h(&old);
}
int16_t SW_Temp::getXOffs() const
{
	switch(get_alignment(&old))
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_Temp::getType() const
{
	return ssoTEMPOLD;
}
void SW_Temp::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(old.d1);
	switch(getType())
	{
		case ssoTEXTBOX:
		{
			draw_textbox(dest, x, y, old.w, old.h, tempfont, (char *)old.dp1, old.d4!=0, old.d5, old.d2, old.d3, subscreen_color(old.colortype1, old.color1), subscreen_color(old.colortype2, old.color2), subscreen_color(old.colortype3, old.color3));
		}
		break;
		
		case ssoSELECTEDITEMNAME:
		{
			int32_t itemid=page.get_sel_item();
			
			// If it's a combined bow and arrow, the item ID will have 0xF000 added.
			if(itemid>=0xF000)
				itemid-=0xF000;
			
			// 0 can mean either the item with index 0 is selected or there's no
			// valid item to select, so be sure Hero has whatever it would be.
			if(!game->get_item(itemid))
				break;
				
			itemdata const& itm = itemsbuf[itemid];
			char itemname[256]="";
			strncpy(itemname, itm.get_name().c_str(), 255);
			
			draw_textbox(dest, x, y, old.w, old.h, tempfont, itemname, old.d4!=0, old.d5, old.d2, old.d3, subscreen_color(old.colortype1, old.color1), subscreen_color(old.colortype2, old.color2), subscreen_color(old.colortype3, old.color3));
		}
		break;
		
		case ssoMAGICGAUGE:
		{
			magicgauge(dest,x,y, old.d1, old.d2, old.colortype1, ((old.d10&1)?1:0), old.d3, old.color1, ((old.d10&2)?1:0),
					   old.d4, old.colortype2, ((old.d10&4)?1:0), old.d5, old.color2, ((old.d10&8)?1:0), old.d6, old.d7, old.d8, ((old.d10&16)?1:0),
					   old.d9);
		}
		break;
		
		case ssoLIFEGAUGE:
		{
			lifegauge(dest,x,y, old.d1, old.d2, old.colortype1, ((old.d10&1)?1:0), old.d3, old.color1, ((old.d10&2)?1:0),
					  old.d4, old.colortype2, ((old.d10&4)?1:0), old.d5, old.color2, ((old.d10&8)?1:0), old.d6, old.d7, old.d8, ((old.d10&16)?1:0));
		}
		break;
	}
}
SubscrWidget* SW_Temp::clone() const
{
	return new SW_Temp(*this);
}
bool SW_Temp::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_Temp const* other = dynamic_cast<SW_Temp const*>(src);
	SubscrWidget::copy_prop(other,all);
	old = other->old;
	return true;
}
int32_t SW_Temp::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_getc(&old.type,f))
		return qe_invalid;
	if(!p_getc(&old.pos,f))
		return qe_invalid;
	if(!p_igetw(&old.x,f))
		return qe_invalid;
	if(!p_igetw(&old.y,f))
		return qe_invalid;
	if(!p_igetw(&old.w,f))
		return qe_invalid;
	if(!p_igetw(&old.h,f))
		return qe_invalid;
	if(!p_getc(&old.colortype1,f))
		return qe_invalid;
	if(!p_igetw(&old.color1,f))
		return qe_invalid;
	if(!p_getc(&old.colortype2,f))
		return qe_invalid;
	if(!p_igetw(&old.color2,f))
		return qe_invalid;
	if(!p_getc(&old.colortype3,f))
		return qe_invalid;
	if(!p_igetw(&old.color3,f))
		return qe_invalid;
	if(!p_igetl(&old.d1,f))
		return qe_invalid;
	if(!p_igetl(&old.d2,f))
		return qe_invalid;
	if(!p_igetl(&old.d3,f))
		return qe_invalid;
	if(!p_igetl(&old.d4,f))
		return qe_invalid;
	if(!p_igetl(&old.d5,f))
		return qe_invalid;
	if(!p_igetl(&old.d6,f))
		return qe_invalid;
	if(!p_igetl(&old.d7,f))
		return qe_invalid;
	if(!p_igetl(&old.d8,f))
		return qe_invalid;
	if(!p_igetl(&old.d9,f))
		return qe_invalid;
	if(!p_igetl(&old.d10,f))
		return qe_invalid;
	if(!p_getc(&old.frames,f))
		return qe_invalid;
	if(!p_getc(&old.speed,f))
		return qe_invalid;
	if(!p_getc(&old.delay,f))
		return qe_invalid;
	if(!p_getc(&old.frame,f))
		return qe_invalid;
	byte len;
	if(!p_getc(&len,f))
		return qe_invalid;
	char* ptr = len ? new char[len+1] : nullptr;
	if(len)
	{
		for(byte q = 0; q < len; ++q)
		{
			if(!p_getc(&ptr[q],f))
				return qe_invalid;
		}
		ptr[len] = 0;
	}
	if(old.dp1) delete[] old.dp1;
	old.dp1 = ptr;
	return 0;
}
int32_t SW_Temp::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_putc(old.type,f))
		return qe_invalid;
	if(!p_putc(old.pos,f))
		return qe_invalid;
	if(!p_iputw(old.x,f))
		return qe_invalid;
	if(!p_iputw(old.y,f))
		return qe_invalid;
	if(!p_iputw(old.w,f))
		return qe_invalid;
	if(!p_iputw(old.h,f))
		return qe_invalid;
	if(!p_putc(old.colortype1,f))
		return qe_invalid;
	if(!p_iputw(old.color1,f))
		return qe_invalid;
	if(!p_putc(old.colortype2,f))
		return qe_invalid;
	if(!p_iputw(old.color2,f))
		return qe_invalid;
	if(!p_putc(old.colortype3,f))
		return qe_invalid;
	if(!p_iputw(old.color3,f))
		return qe_invalid;
	if(!p_iputl(old.d1,f))
		return qe_invalid;
	if(!p_iputl(old.d2,f))
		return qe_invalid;
	if(!p_iputl(old.d3,f))
		return qe_invalid;
	if(!p_iputl(old.d4,f))
		return qe_invalid;
	if(!p_iputl(old.d5,f))
		return qe_invalid;
	if(!p_iputl(old.d6,f))
		return qe_invalid;
	if(!p_iputl(old.d7,f))
		return qe_invalid;
	if(!p_iputl(old.d8,f))
		return qe_invalid;
	if(!p_iputl(old.d9,f))
		return qe_invalid;
	if(!p_iputl(old.d10,f))
		return qe_invalid;
	if(!p_putc(old.frames,f))
		return qe_invalid;
	if(!p_putc(old.speed,f))
		return qe_invalid;
	if(!p_putc(old.delay,f))
		return qe_invalid;
	if(!p_putc(old.frame,f))
		return qe_invalid;
	byte len = 0;
	char const* ptr = (char*)old.dp1;
	if(ptr)
		len = strlen(ptr);
	if(!p_putc(len,f))
		return qe_invalid;
	for(byte q = 0; ptr[q]; ++q)
	{
		if(!p_putc(ptr[q],f))
			return qe_invalid;
	}
	return 0;
}


bool new_widget_type(int ty)
{
	switch(ty)
	{
		//These have been upgraded
		case sso2X2FRAME:
		case ssoTEXT:
		case ssoLINE:
		case ssoRECT:
		case ssoBSTIME:
		case ssoTIME:
		case ssoSSTIME:
		case ssoMAGICMETER:
		case ssoLIFEMETER:
		case ssoBUTTONITEM:
		case ssoCOUNTER:
		case ssoCOUNTERS:
		case ssoMINIMAPTITLE:
		case ssoMINIMAP:
		case ssoLARGEMAP:
		case ssoCLEAR:
		case ssoCURRENTITEM:
		case ssoTRIFRAME:
		case ssoMCGUFFIN:
		case ssoTILEBLOCK:
		case ssoMINITILE:
			return true;
		//These ones are just empty
		case ssoITEM:
		case ssoICON:
		case ssoNULL:
		case ssoNONE:
			return true;
	}
	return false;
}
SubscrWidget* SubscrWidget::fromOld(subscreen_object const& old)
{
	switch(old.type)
	{
		case sso2X2FRAME:
			return new SW_2x2Frame(old);
		case ssoTEXT:
			return new SW_Text(old);
		case ssoLINE:
			return new SW_Line(old);
		case ssoRECT:
			return new SW_Rect(old);
		case ssoBSTIME:
		case ssoTIME:
		case ssoSSTIME:
			return new SW_Time(old);
		case ssoMAGICMETER:
			return new SW_MagicMeter(old);
		case ssoLIFEMETER:
			return new SW_LifeMeter(old);
		case ssoBUTTONITEM:
			return new SW_ButtonItem(old);
		case ssoCOUNTER:
			return new SW_Counter(old);
		case ssoCOUNTERS:
			return new SW_Counters(old);
		case ssoMINIMAPTITLE:
			return new SW_MMapTitle(old);
		case ssoMINIMAP:
			return new SW_MMap(old);
		case ssoLARGEMAP:
			return new SW_LMap(old);
		case ssoCLEAR:
			return new SW_Clear(old);
		case ssoCURRENTITEM:
			return new SW_CurrentItem(old);
		case ssoTRIFRAME:
			return new SW_TriFrame(old);
		case ssoMCGUFFIN:
			return new SW_McGuffin(old);
		case ssoTILEBLOCK:
			return new SW_TileBlock(old);
		case ssoMINITILE:
			return new SW_MiniTile(old);
		case ssoSELECTOR1:
		case ssoSELECTOR2:
			return new SW_Selector(old);
		case ssoMAGICGAUGE:
		case ssoLIFEGAUGE:
		case ssoTEXTBOX:
		case ssoSELECTEDITEMNAME:
			return nullptr;
			return new SW_Temp(old); //!TODO SUBSCR
		case ssoITEM:
		{
			SubscrWidget* ret = new SubscrWidget(old);
			ret->w = 16;
			ret->h = 16;
			return ret;
		}
		case ssoICON:
		{
			SubscrWidget* ret = new SubscrWidget(old);
			ret->w = 8;
			ret->h = 8;
			return ret;
		}
		case ssoNULL:
		case ssoNONE:
		case ssoCURRENTITEMTILE:
		case ssoSELECTEDITEMTILE:
		case ssoCURRENTITEMTEXT:
		case ssoCURRENTITEMNAME:
		case ssoCURRENTITEMCLASSTEXT:
		case ssoCURRENTITEMCLASSNAME:
		case ssoSELECTEDITEMCLASSNAME:
			break; //Nothingness
	}
	return nullptr;
}
SubscrWidget* SubscrWidget::readWidg(PACKFILE* f, word s_version)
{
	byte ty;
	if(!p_getc(&ty,f))
		return nullptr;
	SubscrWidget* widg = newType(ty);
	if(widg && widg->read(f,s_version))
		widg = nullptr;
	return widg;
}
SubscrWidget* SubscrWidget::newType(byte ty)
{
	SubscrWidget* widg = nullptr;
	switch(ty)
	{
		case sso2X2FRAME:
			widg = new SW_2x2Frame();
			break;
		case ssoTEXT:
		{
			SW_Text* tmp;
			widg = tmp = new SW_Text();
			tmp->c_text.type = ssctMISC;
			break;
		}
		case ssoLINE:
			widg = new SW_Line();
			break;
		case ssoRECT:
			widg = new SW_Rect();
			break;
		case ssoBSTIME:
		case ssoTIME:
		case ssoSSTIME:
		{
			SW_Time* tmp;
			widg = tmp = new SW_Time(ty);
			tmp->c_text.type = ssctMISC;
			break;
		}
		case ssoMAGICMETER:
			widg = new SW_MagicMeter();
			break;
		case ssoLIFEMETER:
			widg = new SW_LifeMeter();
			break;
		case ssoBUTTONITEM:
			widg = new SW_ButtonItem();
			break;
		case ssoCOUNTER:
		{
			SW_Counter* tmp;
			widg = tmp = new SW_Counter();
			tmp->infitm = -1; //(None) inf item
			tmp->c_text.type = ssctMISC; //Default text color
			break;
		}
		case ssoCOUNTERS:
		{
			SW_Counters* tmp;
			widg = tmp = new SW_Counters();
			tmp->c_text.type = ssctMISC;
			break;
		}
		case ssoMINIMAPTITLE:
		{
			SW_MMapTitle* tmp;
			widg = tmp = new SW_MMapTitle();
			tmp->c_text.type = ssctMISC;
			break;
		}
		case ssoMINIMAP:
			widg = new SW_MMap();
			break;
		case ssoLARGEMAP:
			widg = new SW_LMap();
			break;
		case ssoCLEAR:
			widg = new SW_Clear();
			break;
		case ssoCURRENTITEM:
			widg = new SW_CurrentItem();
			break;
		case ssoTRIFRAME:
			widg = new SW_TriFrame();
			break;
		case ssoMCGUFFIN:
			widg = new SW_McGuffin();
			break;
		case ssoTILEBLOCK:
			widg = new SW_TileBlock();
			break;
		case ssoMINITILE:
			widg = new SW_MiniTile();
			break;
		case ssoSELECTOR1:
		case ssoSELECTOR2:
			widg = new SW_Selector(ty);
			break;
		case ssoLIFEGAUGE:
			widg = new SW_Temp(ty); //!TODO SUBSCR
			break;
		case ssoMAGICGAUGE:
		{
			SW_Temp* tmp;
			widg = tmp = new SW_Temp(ty); //!TODO SUBSCR
			tmp->old.d9 = -1; // 'Always show' by default
			break;
		}
		case ssoSELECTEDITEMNAME:
		case ssoTEXTBOX:
		{
			SW_Temp* tmp;
			widg = tmp = new SW_Temp(ty); //!TODO SUBSCR
			tmp->old.colortype1 = ssctMISC;
			break;
		}
		case ssoITEM:
		{
			widg = new SubscrWidget(ty);
			widg->w = 16;
			widg->h = 16;
			break;
		}
		case ssoICON:
		{
			widg = new SubscrWidget(ty);
			widg->w = 8;
			widg->h = 8;
			break;
		}
		case ssoNULL:
		case ssoNONE:
		case ssoCURRENTITEMTILE:
		case ssoSELECTEDITEMTILE:
		case ssoCURRENTITEMTEXT:
		case ssoCURRENTITEMNAME:
		case ssoCURRENTITEMCLASSTEXT:
		case ssoCURRENTITEMCLASSNAME:
		case ssoSELECTEDITEMCLASSNAME:
			widg = new SubscrWidget(ty); //Nothingness
			break;
	}
	if(widg && !widg->type)
		widg->type = ty;
	return widg;
}

void SubscrPage::move_cursor(int dir, bool item_only)
{
	//what will be returned when all else fails.
	//don't return the forbiddenpos... no matter what -DD
	
	// verify startpos
	if(cursor_pos < 0 || cursor_pos >= 0xFF)
		cursor_pos = 0;
	
	auto& objects = contents;
	
	item_only = item_only || !get_qr(qr_FREEFORM_SUBSCREEN_CURSOR);
	bool verify = dir==SEL_VERIFY_RIGHT || dir==SEL_VERIFY_LEFT;
	
	if(verify)
	{
		SubscrWidget* widg = get_widg_pos(cursor_pos);
		int32_t wpn = widg ? widg->getItemVal() : -1;
		if(wpn > 0 && widg->getType() == ssoCURRENTITEM && !(widg->flags&SUBSCR_CURITM_NONEQP))
			return;
	}
	
	int32_t p=-1;
	int32_t curpos = cursor_pos;
	int32_t firstValidPos=-1;
	
	for(int32_t i=0; i < contents.size(); ++i)
	{
		if(contents[i]->flags&SUBSCRFLAG_SELECTABLE)
		{
			if(firstValidPos==-1 && contents[i]->pos>=0)
				firstValidPos=i;
			
			if(contents[i]->pos==curpos)
				p=i;
			if(p>-1 && firstValidPos>-1)
				break;
		}
	}
	
	if(p == -1)
	{
		if(firstValidPos>=0)
			cursor_pos = contents[firstValidPos]->pos;
		return;
	}
	
	//remember we've been here
	std::set<int32_t> oldPositions;
	oldPositions.insert(curpos);
	
	//1. Perform any shifts required by the above
	//2. If that's not possible, go to position 1 and reset the b weapon.
	//2a.  -if we arrive at a position we've already visited, give up and stay there
	//3. Get the weapon at the new slot
	//4. If it's not possible, go to step 1.
	
	for(;;)
	{
		//shift
		switch(dir)
		{
			case SEL_LEFT:
			case SEL_VERIFY_LEFT:
				curpos = contents[p]->pos_left;
				break;
				
			case SEL_RIGHT:
			case SEL_VERIFY_RIGHT:
				curpos = contents[p]->pos_right;
				break;
				
			case SEL_DOWN:
				curpos = contents[p]->pos_down;
				break;
				
			case SEL_UP:
				curpos = contents[p]->pos_up;
				break;
		}
		
		//find our new position
		SubscrWidget* widg = get_widg_pos(curpos,false);
		
		if(!widg)
			return;
		
		//if we've already been here, give up
		if(oldPositions.find(curpos) != oldPositions.end())
			return;
		
		//else, remember we've been here
		oldPositions.insert(curpos);
		
		//Valid stop point?
		if((widg->flags & SUBSCRFLAG_SELECTABLE) && (!item_only || widg->getItemVal()))
		{
			cursor_pos = curpos;
			return;
		}
	}
}
int32_t SubscrPage::move_legacy(int dir, int startp, int fp, int fp2, int fp3, bool equip_only, bool item_only)
{
	//what will be returned when all else fails.
	//don't return the forbiddenpos... no matter what -DD
	
	int32_t failpos(0);
	
	if(startp == fp || startp == fp2 || startp == fp3)
		failpos = 0xFF;
	else failpos = startp;
	
	// verify startpos
	if(startp < 0 || startp >= 0xFF)
		startp = 0;
	
	auto& objects = contents;
	
	item_only = item_only || !get_qr(qr_FREEFORM_SUBSCREEN_CURSOR);
	bool verify = dir==SEL_VERIFY_RIGHT || dir==SEL_VERIFY_LEFT;
	
	if(verify)
	{
		SubscrWidget* widg = get_widg_pos(startp);
		int32_t wpn = widg ? widg->getItemVal() : -1;
		equip_only = item_only = true;
		if(widg->getType() != ssoCURRENTITEM || (widg->flags&SUBSCR_CURITM_NONEQP))
			wpn = -1;
		
		if(wpn > 0 && startp != fp && startp != fp2 && startp != fp3)
		{
			return startp;
		}
	}
	
	int32_t p=-1;
	int32_t curpos = startp;
	int32_t firstValidPos=-1, firstValidEquipPos=-1;
	
	for(int32_t i=0; i < contents.size(); ++i)
	{
		if(contents[i]->getType()==ssoCURRENTITEM && (contents[i]->flags&SUBSCRFLAG_SELECTABLE))
		{
			if(firstValidPos==-1 && contents[i]->pos>=0)
				firstValidPos=i;
			if(firstValidEquipPos==-1 && contents[i]->pos>=0)
				if(!equip_only || !(contents[i]->flags&SUBSCR_CURITM_NONEQP))
					firstValidEquipPos=i;
			
			if(contents[i]->pos==curpos)
				p=i;
			if(p>-1 && firstValidPos>-1 && firstValidEquipPos>-1)
				break;
		}
	}
	
	if(p == -1)
	{
		//can't find the current position
		// Switch to a valid weapon if there is one; otherwise,
		// the selector can simply disappear
		if(firstValidEquipPos>=0)
			return contents[firstValidEquipPos]->pos;
		if(firstValidPos>=0)
			return contents[firstValidPos]->pos;
		//FAILURE
		else return failpos;
	}
	
	//remember we've been here
	std::set<int32_t> oldPositions;
	oldPositions.insert(curpos);
	
	//1. Perform any shifts required by the above
	//2. If that's not possible, go to position 1 and reset the b weapon.
	//2a.  -if we arrive at a position we've already visited, give up and stay there
	//3. Get the weapon at the new slot
	//4. If it's not possible, go to step 1.
	
	for(;;)
	{
		//shift
		switch(dir)
		{
			case SEL_LEFT:
			case SEL_VERIFY_LEFT:
				curpos = contents[p]->pos_left;
				break;
				
			case SEL_RIGHT:
			case SEL_VERIFY_RIGHT:
				curpos = contents[p]->pos_right;
				break;
				
			case SEL_DOWN:
				curpos = contents[p]->pos_down;
				break;
				
			case SEL_UP:
				curpos = contents[p]->pos_up;
				break;
		}
		
		//find our new position
		SubscrWidget* widg = get_widg_pos(curpos,false);
		
		if(!widg)
			return failpos;
		
		//if we've already been here, give up
		if(oldPositions.find(curpos) != oldPositions.end())
			return failpos;
		
		//else, remember we've been here
		oldPositions.insert(curpos);
		
		//Valid stop point?
		if(widg->flags & SUBSCRFLAG_SELECTABLE)
			if(curpos != fp && curpos != fp2 && curpos != fp3)
				if(!equip_only || !(widg->flags & SUBSCR_CURITM_NONEQP))
					if(!item_only || widg->getItemVal())
						return curpos;
	}
}
SubscrWidget* SubscrPage::get_widg_pos(int32_t pos, bool sel_only)
{
	for(size_t q = 0; q < contents.size(); ++q)
	{
		if(sel_only && !(contents[q]->flags & SUBSCRFLAG_SELECTABLE))
			continue;
		if(contents[q]->pos == pos)
			return contents[q];
	}
	return nullptr;
}
int32_t SubscrPage::get_item_pos(int32_t pos, bool sel_only)
{
	auto* w = get_widg_pos(pos,sel_only);
	if(w)
		return w->getItemVal();
	return -1;
}
SubscrWidget* SubscrPage::get_sel_widg()
{
	return get_widg_pos(cursor_pos);
}
int32_t SubscrPage::get_sel_item()
{
	auto* w = get_sel_widg();
	if(w)
		return w->getItemVal();
	return -1;
}
int32_t SubscrPage::get_pos_of_item(int32_t id)
{
	for(SubscrWidget* widg : contents)
	{
		if(id == widg->getItemVal())
			return widg->pos;
	}
	return -1;
}

void SubscrPage::clear()
{
	cursor_pos = 0;
	for (SubscrWidget* ptr : contents)
		delete ptr;
	contents.clear();
}
void SubscrPage::draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime)
{
	for(SubscrWidget* widg : contents)
	{
		if(widg->visible(pos,showtime))
			widg->draw(dest,xofs,yofs,*this);
	}
}
SubscrPage::~SubscrPage()
{
	clear();
}
SubscrPage& SubscrPage::operator=(SubscrPage const& other)
{
	clear();
	cursor_pos = other.cursor_pos;
	for(SubscrWidget* widg : other.contents)
	{
		contents.push_back(widg->clone());
	}
	return *this;
}
SubscrPage::SubscrPage(const SubscrPage& other)
{
	*this = other;
}
int32_t SubscrPage::read(PACKFILE *f, word s_version)
{
	clear();
    if(!p_igetl(&cursor_pos,f))
        return qe_invalid;
	word sz;
	if(!p_igetw(&sz,f))
        return qe_invalid;
	for(word q = 0; q < sz; ++q)
	{
		SubscrWidget* widg = SubscrWidget::readWidg(f,s_version);
		if(!widg)
			return qe_invalid;
		contents.push_back(widg);
	}
	return 0;
}
int32_t SubscrPage::write(PACKFILE *f) const
{
    if(!p_iputl(cursor_pos,f))
        new_return(1);
	word sz = zc_min(65535,contents.size());
	if(!p_iputw(sz,f))
		new_return(2);
	for(word q = 0; q < sz; ++q)
		if(auto ret = contents[q]->write(f))
			return ret;
	
	return 0;
}

SubscrPage& ZCSubscreen::cur_page()
{
	if(pages.empty())
		pages.emplace_back();
	curpage = vbound(curpage,pages.size()-1,0);
	return pages[curpage];
}
SubscrPage* ZCSubscreen::get_page(byte id)
{
	if(id >= pages.size()) return nullptr;
	return &pages[id];
}
bool ZCSubscreen::get_page_pos(int32_t itmid, byte& pg, byte& pos)
{
	for(byte q = 0; q < pages.size(); ++q)
	{
		int p = pages[q].get_pos_of_item(itmid);
		if(p > -1)
		{
			pg = q;
			pos = p;
			return true;
		}
	}
	return false;
}
int32_t ZCSubscreen::get_item_pos(byte pos, byte pg)
{
	if(pg >= pages.size()) return -1;
	return pages[pg].get_item_pos(pos);
}
void ZCSubscreen::clear()
{
	*this = ZCSubscreen();
}
void ZCSubscreen::draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime)
{
	if(pages.empty()) return;
	size_t page = curpage;
	if(page >= pages.size()) page = 0;
	//!TODO SUBSCR handle animations between multiple pages?
	pages[page].draw(dest,xofs,yofs,pos,showtime);
}
void ZCSubscreen::load_old(subscreen_group const& g)
{
	name = g.name;
	sub_type = g.ss_type;
	pages.clear();
	SubscrPage& p = pages.emplace_back();
	for(int ind = 0; ind < MAXSUBSCREENITEMS && g.objects[ind].type != ssoNULL; ++ind)
	{
		auto* w = SubscrWidget::fromOld(g.objects[ind]);
		if(w)
			p.contents.push_back(w);
	}
}
void ZCSubscreen::load_old(subscreen_object const* arr)
{
	pages.clear();
	SubscrPage& p = pages.emplace_back();
	for(int ind = 0; ind < MAXSUBSCREENITEMS && arr[ind].type != ssoNULL; ++ind)
	{
		SubscrWidget* w = SubscrWidget::fromOld(arr[ind]);
		if(!w) continue;
		if(w->getType() == ssoNONE || w->getType() == ssoNULL)
		{
			delete w;
			continue;
		}
		p.contents.push_back(w);
	}
}
int32_t ZCSubscreen::read(PACKFILE *f, word s_version)
{
    if(!p_getcstr(&name,f))
        return qe_invalid;
	if(!p_getc(&sub_type,f))
        return qe_invalid;
	byte pagecnt;
	if(!p_getc(&pagecnt,f))
        return qe_invalid;
	pages.clear();
	for(byte q = 0; q < pagecnt; ++q)
	{
		SubscrPage& pg = pages.emplace_back();
		if(auto ret = pg.read(f, s_version))
			return ret;
	}
	return 0;
}
int32_t ZCSubscreen::write(PACKFILE *f) const
{
    if(!p_putcstr(name,f))
        new_return(1);
	if(!p_putc(sub_type,f))
		new_return(2);
	byte pagecnt = zc_min(255,pages.size());
	if(!p_putc(pagecnt,f))
		new_return(3);
	for(byte q = 0; q < pagecnt; ++q)
		if(auto ret = pages[q].write(f))
			return ret;
	
	return 0;
}

