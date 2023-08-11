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
void draw_textbox(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, FONT *tempfont, char const*thetext, bool wword, int32_t tabsize, int32_t alignment, int32_t textstyle, int32_t color, int32_t shadowcolor, int32_t backcolor);
void lifegauge(BITMAP *dest,int32_t x,int32_t y, int32_t container, int32_t notlast_tile, int32_t notlast_cset, bool notlast_mod, int32_t last_tile, int32_t last_cset, bool last_mod,
			   int32_t cap_tile, int32_t cap_cset, bool cap_mod, int32_t aftercap_tile, int32_t aftercap_cset, bool aftercap_mod, int32_t frames, int32_t speed, int32_t delay, bool unique_last);
void magicgauge(BITMAP *dest,int32_t x,int32_t y, int32_t container, int32_t notlast_tile, int32_t notlast_cset, bool notlast_mod, int32_t last_tile, int32_t last_cset, bool last_mod,
				int32_t cap_tile, int32_t cap_cset, bool cap_mod, int32_t aftercap_tile, int32_t aftercap_cset, bool aftercap_mod, int32_t frames, int32_t speed, int32_t delay, bool unique_last, int32_t show);

int32_t to_real_font(int32_t ss_font)
{
	switch(ss_font)
	{
		case ssfSMALL: return font_sfont;
		case ssfSMALLPROP: return font_spfont;
		case ssfSS1: return font_ssfont1;
		case ssfSS2: return font_ssfont2;
		case ssfSS3: return font_ssfont3;
		case ssfSS4: return font_ssfont4;
		case ssfZTIME: return font_ztfont;
		case ssfZELDA: return font_zfont;
		case ssfZ3: return font_z3font;
		case ssfZ3SMALL: return font_z3smallfont;
		case ssfGBLA: return font_gblafont;
		case ssfGORON: return font_goronfont;
		case ssfZORAN: return font_zoranfont;
		case ssfHYLIAN1: return font_hylian1font;
		case ssfHYLIAN2: return font_hylian2font;
		case ssfHYLIAN3: return font_hylian3font;
		case ssfHYLIAN4: return font_hylian4font;
		case ssfGBORACLE: return font_gboraclefont;
		case ssfGBORACLEP: return font_gboraclepfont;
		case ssfDSPHANTOM: return font_dsphantomfont;
		case ssfDSPHANTOMP: return font_dsphantompfont;
		case ssfAT800: return font_atari800font;
		case ssfACORN: return font_acornfont;
		case ssADOS: return font_adosfont;
		case ssfALLEG: return font_baseallegrofont;
		case ssfAPL2: return font_apple2font;
		case ssfAPL280: return font_apple280colfont;
		case ssfAPL2GS: return font_apple2gsfont;
		case ssfAQUA: return font_aquariusfont;
		case ssfAT400: return font_atari400font;
		case ssfC64: return font_c64font;
		case ssfC64HR: return font_c64hiresfont;
		case ssfCGA: return font_cgafont;
		case ssfCOCO: return font_cocofont;
		case ssfCOCO2: return font_coco2font;
		case ssfCOUPE: return font_coupefont;
		case ssfCPC: return font_cpcfont;
		case ssfFANTASY: return font_fantasyfont;
		case ssfFDSKANA: return font_fdskanafont;
		case ssfFDSLIKE: return font_fdslikefont;
		case ssfFDSROM: return font_fdsromanfont;
		case ssfFF: return font_finalffont;
		case ssfFUTHARK: return font_futharkfont;
		case ssfGAIA: return font_gaiafont;
		case ssfHIRA: return font_hirafont;
		case ssfJP: return font_jpfont;
		case ssfKONG: return font_kongfont;
		case ssfMANA: return font_manafont;
		case ssfML: return font_mlfont;
		case ssfMOT: return font_motfont;
		case ssfMSX0: return font_msxmode0font;
		case ssfMSX1: return font_msxmode1font;
		case ssfPET: return font_petfont;
		case ssfPSTART: return font_pstartfont;
		case ssfSATURN: return font_saturnfont;
		case ssfSCIFI: return font_scififont;
		case ssfSHERW: return font_sherwoodfont;
		case ssfSINQL: return font_sinqlfont;
		case ssfSPEC: return font_spectrumfont;
		case ssfSPECLG: return font_speclgfont;
		case ssfTI99: return font_ti99font;
		case ssfTRS: return font_trsfont;
		case ssfZ2: return font_z2font;
		case ssfZX: return font_zxfont;
		case ssfLISA: return font_lisafont;
	}
	return font_zfont;
}

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
	return get_color(type,color);
}
int32_t SubscrColorInfo::get_color(byte type, int16_t color)
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
	return get_cset(type,color);
}
int32_t SubscrColorInfo::get_cset(byte type, int16_t color)
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

int32_t SubscrMTInfo::tile() const
{
	return tilecrn>>2;
}
byte SubscrMTInfo::crn() const
{
	return tilecrn%2;
}
void SubscrMTInfo::setTileCrn(int32_t tile, byte crn)
{
	tilecrn = (tile<<2)|(crn%4);
}
int32_t SubscrMTInfo::read(PACKFILE *f, word s_version)
{
	if(!p_igetl(&tilecrn,f))
		return qe_invalid;
	if(!p_getc(&cset,f))
		return qe_invalid;
	return 0;
}
int32_t SubscrMTInfo::write(PACKFILE *f) const
{
	if(!p_iputl(tilecrn,f))
		new_return(1);
	if(!p_putc(cset,f))
		new_return(2);
	return 0;
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
	if(unsigned(old.d1) >= ssfMAX)
		compat_flags |= SUBSCRCOMPAT_FONT_RAND;
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
	return widgNULL;
}
int32_t SubscrWidget::getItemVal(bool display) const
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
	//Intentionally offset by one byte ('getType()') from ::write() -Em
	if(!p_getc(&type,f))
		return qe_invalid;
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
	if(!p_putc(getType(),f))
		new_return(1);
	if(!p_putc(type,f))
		new_return(2);
	if(!p_putc(posflags,f))
		new_return(3);
	if(!p_iputw(x,f))
		new_return(4);
	if(!p_iputw(y,f))
		new_return(5);
	if(!p_iputw(w,f))
		new_return(6);
	if(!p_iputw(h,f))
		new_return(7);
	if(!p_iputl(flags,f))
		new_return(8);
	if(flags&SUBSCRFLAG_SELECTABLE)
	{
		if(!p_iputl(pos,f))
			new_return(9);
		if(!p_iputl(pos_up,f))
			new_return(10);
		if(!p_iputl(pos_down,f))
			new_return(11);
		if(!p_iputl(pos_left,f))
			new_return(12);
		if(!p_iputl(pos_right,f))
			new_return(13);
		if(!p_putcstr(override_text,f))
			new_return(14);
		if(!p_putc(gen_script_btns,f))
			new_return(15);
		if(!p_iputw(generic_script,f))
			new_return(16);
	}
	return 0;
}
void SubscrWidget::replay_rand_compat(byte pos) const
{
	if((compat_flags & SUBSCRCOMPAT_FONT_RAND) && (posflags&pos) && replay_version_check(0,19))
		zc_oldrand();
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
	return widgFRAME;
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
	return widgTEXT;
}
void SW_Text::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	textout_styled_aligned_ex(dest,tempfont,text.c_str(),x+xofs,y+yofs,
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
	return widgLINE;
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
	return widgRECT;
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
	SETFLAG(flags,SUBSCR_TIME_ALTSTR,old.type == ssoBSTIME);
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
	return widgTIME;
}
void SW_Time::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	char *ts;
	auto tm = game ? game->get_time() : 0;
	if(flags&SUBSCR_TIME_ALTSTR)
		ts = time_str_short2(tm);
	else ts = time_str_med(tm);
	FONT* tempfont = get_zc_font(fontid);
	textout_styled_aligned_ex(dest,tempfont,ts,x+xofs,y+yofs,
		shadtype,align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color());
}
bool SW_Time::visible(byte pos, bool showtime) const
{
	if(type && type != ssoSSTIME && replay_version_check(0,19))
		showtime = true;
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
	return widgMMETER;
}
void SW_MagicMeter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	magicmeter(dest, x+xofs, y+yofs);
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
	return y+(4-rows)*8;
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
	return widgLMETER;
}
void SW_LifeMeter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	lifemeter(dest, getX()+xofs, y+yofs, 1, flags&SUBSCR_LIFEMET_BOT);
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
	return widgBTNITM;
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
	return widgCOUNTER;
}
void SW_Counter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	counter(dest, x+xofs,y+yofs, tempfont, c_text.get_color(),
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
	return widgOLDCTR;
}
void SW_Counters::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	defaultcounters(dest, x+xofs, y+yofs, tempfont, c_text.get_color(),
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
	return widgMMAPTITLE;
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
	return widgMMAP;
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
	return widgLMAP;
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
	return widgBGCOLOR;
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
	iid = old.d8-1;
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
	return widgITEMSLOT;
}
int32_t SW_CurrentItem::getItemVal(bool display) const
{
#ifdef IS_PLAYER
	if(iid > -1)
	{
		bool select = false;
		switch(itemsbuf[iid].family)
		{
			case itype_bomb:
				if(display && get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN))
					select = true;
				else if((game->get_bombs() ||
						// Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
						(itemsbuf[iid].misc1==0 && findWeaponWithParent(iid, wLitBomb))) ||
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
				
				if(display && get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN))
					select = true;
				else if((game->get_sbombs() ||
						// Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
						(itemsbuf[iid].misc1==0 && findWeaponWithParent(iid, wLitSBomb))) ||
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
		if(select && !item_disabled(iid) && game->get_item(iid))
		{
			directItem = iid;
			auto ret = iid;
			if(ret>-1 && itemsbuf[ret].family == itype_arrow)
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
			
			if(display && get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN))
				family=itype_bomb;
			else if((game->get_bombs() ||
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
			
			if(display && get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN))
				family=itype_sbomb;
			else if((game->get_sbombs() ||
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
	int32_t itemid = current_item_id(family, false);
	if(item_disabled(itemid))
		return -1;
	if(iclass == itype_bowandarrow)
		return itemid+0xF000;
	return itemid;
#else
	if(iid > -1) return iid;
	int fam = iclass;
	switch(fam)
	{
		case itype_letterpotion:
			if(current_item_id(itype_potion)==-1)
				fam = itype_letter;
			else fam = itype_potion;
			break;
		case itype_bowandarrow:
			fam = itype_arrow;
			break;
		case itype_map:
			return iMap;
		case itype_compass:
			return iCompass;
		case itype_bosskey:
			return iBossKey;
		case itype_heartpiece:
			return iHCPiece;
	}
	int itemid = current_item_id(fam,false);
	if(itemid == -1) return -1;
	if(fam == itype_bowandarrow)
		itemid |= 0xF000;
	return itemid;
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
	subscreenitem(dest, getX()+xofs,getY()+yofs, iid > -1 ? (iid|0x8000) : iclass);
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
	return widgMCGUFF_FRAME;
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
	return widgMCGUFF;
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
	return widgTILEBLOCK;
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
	return widgMINITILE;
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
	return widgSELECTOR;
}
void SW_Selector::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	SubscrWidget* selitm = page.get_sel_widg();
	
	bool big_sel=flags&SUBSCR_SELECTOR_LARGE;
	item *tempsel=(flags&SUBSCR_SELECTOR_USEB)?sel_b:sel_a;
	int32_t temptile=tempsel->tile;
	tempsel->drawstyle=0;
	
	if(flags&SUBSCR_SELECTOR_TRANSP)
		tempsel->drawstyle=1;
	int32_t id = selitm ? selitm->getItemVal() : -1;
	itemdata const& tmpitm = itemsbuf[id];
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

SW_LifeGaugePiece::SW_LifeGaugePiece(subscreen_object const& old) : SW_LifeGaugePiece()
{
	load_old(old);
}
bool SW_LifeGaugePiece::load_old(subscreen_object const& old)
{
	if(old.type != ssoLIFEGAUGE)
		return false;
	SubscrWidget::load_old(old);
	mts[0].tilecrn = old.d2;
	mts[0].cset = old.colortype1;
	mts[1].tilecrn = old.d3;
	mts[1].cset = old.color1;
	mts[2].tilecrn = old.d4;
	mts[2].cset = old.colortype2;
	mts[3].tilecrn = old.d5;
	mts[3].cset = old.color2;
	SETFLAG(flags, SUBSCR_LGAUGE_MOD1, old.d10&0x01);
	SETFLAG(flags, SUBSCR_LGAUGE_MOD2, old.d10&0x02);
	SETFLAG(flags, SUBSCR_LGAUGE_MOD3, old.d10&0x04);
	SETFLAG(flags, SUBSCR_LGAUGE_MOD4, old.d10&0x08);
	SETFLAG(flags, SUBSCR_LGAUGE_UNQLAST, old.d10&0x10);
	frames = old.d6;
	speed = old.d7;
	delay = old.d8;
	container = old.d1;
	return true;
}
word SW_LifeGaugePiece::getW() const
{
	return 8;
}
word SW_LifeGaugePiece::getH() const
{
	return 8;
}
byte SW_LifeGaugePiece::getType() const
{
	return widgLGAUGE;
}
void SW_LifeGaugePiece::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	lifegauge(dest, x, y, container,
		mts[0].tilecrn, mts[0].cset, flags&SUBSCR_LGAUGE_MOD1,
		mts[1].tilecrn, mts[1].cset, flags&SUBSCR_LGAUGE_MOD2,
		mts[2].tilecrn, mts[2].cset, flags&SUBSCR_LGAUGE_MOD3,
		mts[3].tilecrn, mts[3].cset, flags&SUBSCR_LGAUGE_MOD4,
		frames, speed, delay, flags&SUBSCR_LGAUGE_UNQLAST);
}
SubscrWidget* SW_LifeGaugePiece::clone() const
{
	return new SW_LifeGaugePiece(*this);
}
bool SW_LifeGaugePiece::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_LifeGaugePiece const* other = dynamic_cast<SW_LifeGaugePiece const*>(src);
	SubscrWidget::copy_prop(other,all);
	mts[0] = other->mts[0];
	mts[1] = other->mts[1];
	mts[2] = other->mts[2];
	mts[3] = other->mts[3];
	frames = other->frames;
	speed = other->speed;
	delay = other->delay;
	if(all) container = other->container;
	return true;
}
int32_t SW_LifeGaugePiece::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	for(auto q = 0; q < 4; ++q)
		if(auto ret = mts[q].read(f,s_version))
			return ret;
	if(!p_igetw(&frames, f))
		return qe_invalid;
	if(!p_igetw(&speed, f))
		return qe_invalid;
	if(!p_igetw(&delay, f))
		return qe_invalid;
	if(!p_igetw(&container, f))
		return qe_invalid;
	return 0;
}
int32_t SW_LifeGaugePiece::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	for(auto q = 0; q < 4; ++q)
		if(auto ret = mts[q].write(f))
			return ret;
	if(!p_iputw(frames, f))
		new_return(1);
	if(!p_iputw(speed, f))
		new_return(2);
	if(!p_iputw(delay, f))
		new_return(3);
	if(!p_iputw(container, f))
		new_return(4);
	return 0;
}

SW_MagicGaugePiece::SW_MagicGaugePiece(subscreen_object const& old) : SW_MagicGaugePiece()
{
	load_old(old);
}
bool SW_MagicGaugePiece::load_old(subscreen_object const& old)
{
	if(old.type != ssoMAGICGAUGE)
		return false;
	SubscrWidget::load_old(old);
	mts[0].tilecrn = old.d2;
	mts[0].cset = old.colortype1;
	mts[1].tilecrn = old.d3;
	mts[1].cset = old.color1;
	mts[2].tilecrn = old.d4;
	mts[2].cset = old.colortype2;
	mts[3].tilecrn = old.d5;
	mts[3].cset = old.color2;
	SETFLAG(flags, SUBSCR_MGAUGE_MOD1, old.d10&0x01);
	SETFLAG(flags, SUBSCR_MGAUGE_MOD2, old.d10&0x02);
	SETFLAG(flags, SUBSCR_MGAUGE_MOD3, old.d10&0x04);
	SETFLAG(flags, SUBSCR_MGAUGE_MOD4, old.d10&0x08);
	SETFLAG(flags, SUBSCR_MGAUGE_UNQLAST, old.d10&0x10);
	frames = old.d6;
	speed = old.d7;
	delay = old.d8;
	container = old.d1;
	showdrain = old.d9;
	return true;
}
word SW_MagicGaugePiece::getW() const
{
	return 8;
}
word SW_MagicGaugePiece::getH() const
{
	return 8;
}
byte SW_MagicGaugePiece::getType() const
{
	return widgMGAUGE;
}
void SW_MagicGaugePiece::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	magicgauge(dest, x, y, container,
		mts[0].tilecrn, mts[0].cset, flags&SUBSCR_MGAUGE_MOD1,
		mts[1].tilecrn, mts[1].cset, flags&SUBSCR_MGAUGE_MOD2,
		mts[2].tilecrn, mts[2].cset, flags&SUBSCR_MGAUGE_MOD3,
		mts[3].tilecrn, mts[3].cset, flags&SUBSCR_MGAUGE_MOD4,
		frames, speed, delay, flags&SUBSCR_MGAUGE_UNQLAST, showdrain);
}
SubscrWidget* SW_MagicGaugePiece::clone() const
{
	return new SW_MagicGaugePiece(*this);
}
bool SW_MagicGaugePiece::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_MagicGaugePiece const* other = dynamic_cast<SW_MagicGaugePiece const*>(src);
	SubscrWidget::copy_prop(other,all);
	mts[0] = other->mts[0];
	mts[1] = other->mts[1];
	mts[2] = other->mts[2];
	mts[3] = other->mts[3];
	frames = other->frames;
	speed = other->speed;
	delay = other->delay;
	if(all) container = other->container;
	return true;
}
int32_t SW_MagicGaugePiece::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	for(auto q = 0; q < 4; ++q)
		if(auto ret = mts[q].read(f,s_version))
			return ret;
	if(!p_igetw(&frames, f))
		return qe_invalid;
	if(!p_igetw(&speed, f))
		return qe_invalid;
	if(!p_igetw(&delay, f))
		return qe_invalid;
	if(!p_igetw(&container, f))
		return qe_invalid;
	if(!p_igetw(&showdrain, f))
		return qe_invalid;
	return 0;
}
int32_t SW_MagicGaugePiece::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	for(auto q = 0; q < 4; ++q)
		if(auto ret = mts[q].write(f))
			return ret;
	if(!p_iputw(frames, f))
		new_return(1);
	if(!p_iputw(speed, f))
		new_return(2);
	if(!p_iputw(delay, f))
		new_return(3);
	if(!p_iputw(container, f))
		new_return(4);
	if(!p_iputw(showdrain, f))
		new_return(5);
	return 0;
}


SW_TextBox::SW_TextBox(subscreen_object const& old) : SW_TextBox()
{
	load_old(old);
}
bool SW_TextBox::load_old(subscreen_object const& old)
{
	if(old.type != ssoTEXTBOX)
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
	SETFLAG(flags,SUBSCR_TEXTBOX_WORDWRAP,old.d4);
	tabsize = old.d5;
	return true;
}
byte SW_TextBox::getType() const
{
	return widgTEXTBOX;
}
void SW_TextBox::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	draw_textbox(dest, getX()+xofs, getY()+yofs, getW(), getH(), tempfont, text.c_str(),
		flags&SUBSCR_TEXTBOX_WORDWRAP, tabsize, align, shadtype,
		c_text.get_color(),c_shadow.get_color(),c_bg.get_color());
}
SubscrWidget* SW_TextBox::clone() const
{
	return new SW_TextBox(*this);
}
bool SW_TextBox::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_TextBox const* other = dynamic_cast<SW_TextBox const*>(src);
	SubscrWidget::copy_prop(other,all);
	fontid = other->fontid;
	if(all) text = other->text;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	tabsize = other->tabsize;
	return true;
}
int32_t SW_TextBox::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&fontid,f))
		return qe_invalid;
	if(!p_getc(&align,f))
		return qe_invalid;
	if(!p_getc(&shadtype,f))
		return qe_invalid;
	if(!p_getc(&tabsize,f))
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
int32_t SW_TextBox::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(2);
	if(!p_putc(shadtype,f))
		new_return(3);
	if(!p_putc(tabsize,f))
		new_return(4);
	if(!p_putwstr(text,f))
		new_return(5);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	return 0;
}

SW_SelectedText::SW_SelectedText(subscreen_object const& old) : SW_SelectedText()
{
	load_old(old);
}
bool SW_SelectedText::load_old(subscreen_object const& old)
{
	if(old.type != ssoSELECTEDITEMNAME)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	align = old.d2;
	shadtype = old.d3;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	SETFLAG(flags,SUBSCR_SELTEXT_WORDWRAP,old.d4);
	tabsize = old.d5;
	return true;
}
byte SW_SelectedText::getType() const
{
	return widgSELECTEDTEXT;
}
void SW_SelectedText::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	std::string str;
	
	int32_t itemid=page.get_sel_item(true);
	if(itemid > -1)
	{
		// If it's a combined bow and arrow, the item ID will have 0xF000 added.
		if(itemid>=0xF000)
			itemid-=0xF000;
		
		// 0 can mean either the item with index 0 is selected or there's no
		// valid item to select, so be sure Hero has whatever it would be.
		if(!game->get_item(itemid))
			return;
		
		itemdata const& itm = itemsbuf[itemid];
		str = itm.get_name();
	}
	if(str.size())
		draw_textbox(dest, getX()+xofs, getY()+yofs, getW(), getH(), tempfont,
			str.c_str(), flags&SUBSCR_SELTEXT_WORDWRAP, tabsize, align, shadtype,
			c_text.get_color(),c_shadow.get_color(),c_bg.get_color());
}
SubscrWidget* SW_SelectedText::clone() const
{
	return new SW_SelectedText(*this);
}
bool SW_SelectedText::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_SelectedText const* other = dynamic_cast<SW_SelectedText const*>(src);
	SubscrWidget::copy_prop(other,all);
	fontid = other->fontid;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	tabsize = other->tabsize;
	return true;
}
int32_t SW_SelectedText::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&fontid,f))
		return qe_invalid;
	if(!p_getc(&align,f))
		return qe_invalid;
	if(!p_getc(&shadtype,f))
		return qe_invalid;
	if(!p_getc(&tabsize,f))
		return qe_invalid;
	if(auto ret = c_text.read(f,s_version))
		return ret;
	if(auto ret = c_shadow.read(f,s_version))
		return ret;
	if(auto ret = c_bg.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_SelectedText::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(2);
	if(!p_putc(shadtype,f))
		new_return(3);
	if(!p_putc(tabsize,f))
		new_return(4);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	return 0;
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
		case ssoLIFEGAUGE:
			return new SW_LifeGaugePiece(old);
		case ssoMAGICGAUGE:
			return new SW_MagicGaugePiece(old);
		case ssoTEXTBOX:
			return new SW_TextBox(old);
		case ssoSELECTEDITEMNAME:
			return new SW_SelectedText(old);
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
		case widgFRAME:
			widg = new SW_2x2Frame();
			break;
		case widgTEXT:
			widg = new SW_Text();
			break;
		case widgLINE:
			widg = new SW_Line();
			break;
		case widgRECT:
			widg = new SW_Rect();
			break;
		case widgTIME:
			widg = new SW_Time(ty);
			break;
		case widgMMETER:
			widg = new SW_MagicMeter();
			break;
		case widgLMETER:
			widg = new SW_LifeMeter();
			break;
		case widgBTNITM:
			widg = new SW_ButtonItem();
			break;
		case widgCOUNTER:
			widg = new SW_Counter();
			break;
		case widgOLDCTR:
			widg = new SW_Counters();
			break;
		case widgMMAPTITLE:
			widg = new SW_MMapTitle();
			break;
		case widgMMAP:
			widg = new SW_MMap();
			break;
		case widgLMAP:
			widg = new SW_LMap();
			break;
		case widgBGCOLOR:
			widg = new SW_Clear();
			break;
		case widgITEMSLOT:
			widg = new SW_CurrentItem();
			break;
		case widgMCGUFF_FRAME:
			widg = new SW_TriFrame();
			break;
		case widgMCGUFF:
			widg = new SW_McGuffin();
			break;
		case widgTILEBLOCK:
			widg = new SW_TileBlock();
			break;
		case widgMINITILE:
			widg = new SW_MiniTile();
			break;
		case widgSELECTOR:
			widg = new SW_Selector(ty);
			break;
		case widgLGAUGE:
			widg = new SW_LifeGaugePiece();
			break;
		case widgMGAUGE:
			widg = new SW_MagicGaugePiece();
			break;
		case widgTEXTBOX:
			widg = new SW_TextBox();
			break;
		case widgSELECTEDTEXT:
			widg = new SW_SelectedText();
			break;
	}
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
		if(widg && widg->getType() == widgITEMSLOT
			&& (widg->flags & SUBSCRFLAG_SELECTABLE)
			&& !(widg->flags & SUBSCR_CURITM_NONEQP)
			&& widg->getItemVal() > -1)
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
	SubscrWidget* widg = contents[p];
	for(;;)
	{
		//shift
		switch(dir)
		{
			case SEL_LEFT:
			case SEL_VERIFY_LEFT:
				curpos = widg->pos_left;
				break;
				
			case SEL_RIGHT:
			case SEL_VERIFY_RIGHT:
				curpos = widg->pos_right;
				break;
				
			case SEL_DOWN:
				curpos = widg->pos_down;
				break;
				
			case SEL_UP:
				curpos = widg->pos_up;
				break;
		}
		
		//find our new position
		widg = get_widg_pos(curpos,false);
		
		if(!widg)
			return;
		
		//if we've already been here, give up
		if(oldPositions.find(curpos) != oldPositions.end())
			return;
		
		//else, remember we've been here
		oldPositions.insert(curpos);
		
		//Valid stop point?
		if((widg->flags & SUBSCRFLAG_SELECTABLE) && (!item_only || widg->getItemVal() > -1))
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
		equip_only = item_only = true;
		if(startp != fp && startp != fp2 && startp != fp3)
			if(SubscrWidget* widg = get_widg_pos(startp))
			{
				if(widg->getType() == widgITEMSLOT && !(widg->flags&SUBSCR_CURITM_NONEQP)
					&& widg->getItemVal() > -1)
					return startp;
			}
	}
	
	int32_t p=-1;
	int32_t curpos = startp;
	int32_t firstValidPos=-1, firstValidEquipPos=-1;
	
	for(int32_t i=0; i < contents.size(); ++i)
	{
		if(contents[i]->getType()==widgITEMSLOT && (contents[i]->flags&SUBSCRFLAG_SELECTABLE))
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
	SubscrWidget* widg = contents[p];
	for(;;)
	{
		//shift
		switch(dir)
		{
			case SEL_LEFT:
			case SEL_VERIFY_LEFT:
				curpos = widg->pos_left;
				break;
				
			case SEL_RIGHT:
			case SEL_VERIFY_RIGHT:
				curpos = widg->pos_right;
				break;
				
			case SEL_DOWN:
				curpos = widg->pos_down;
				break;
				
			case SEL_UP:
				curpos = widg->pos_up;
				break;
		}
		
		//find our new position
		widg = get_widg_pos(curpos,false);
		
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
					if(!item_only || widg->getItemVal()>-1)
						return curpos;
	}
}
SubscrWidget* SubscrPage::get_widg_pos(int32_t pos, bool sel_only)
{
	for(size_t q = 0; q < contents.size(); ++q)
	{
		if(!(contents[q]->flags & SUBSCRFLAG_SELECTABLE))
			continue;
		if (sel_only && contents[q]->getType() == widgITEMSLOT)
			if (static_cast<SW_CurrentItem*>(contents[q])->flags & SUBSCR_CURITM_NONEQP)
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
int32_t SubscrPage::get_sel_item(bool display)
{
	auto* w = get_sel_widg();
	if(w)
		return w->getItemVal(display);
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
		widg->replay_rand_compat(pos);
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
		if(arr[ind].type == ssoNONE || arr[ind].type == ssoNULL)
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

