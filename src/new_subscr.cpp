#include "base/zdefs.h"
#include "new_subscr.h"
#include "subscr.h"
#include "base/misctypes.h"
#include "base/fonts.h"
#include "base/zsys.h"
#include "base/dmap.h"
#include "base/initdata.h"
#include "base/qrs.h"
#include "base/mapscr.h"
#include "base/packfile.h"
#include "base/qst.h"
#include "drawing.h"
#include "tiles.h"
#include "items.h"
#include "sprite.h"
#include <set>
#include <fmt/format.h>
#include "zc/maps.h"
#include "zc/ffscript.h"
#include "zc/zelda.h"
#include "zc_list_data.h"

#ifdef IS_PLAYER
#include "zc/hero.h"
extern sprite_list Lwpns;
extern bool msg_onscreen;
void verifyBothWeapons();
bool zq_ignore_item_ownership = true, zq_view_fullctr = false, zq_view_maxctr = false,
	zq_view_noinf = false, zq_view_allinf = false;

#define ALLOW_NULL_WIDGET replay_version_check(0,19)
#else
#define ALLOW_NULL_WIDGET is_zq_replay_test
extern bool is_zq_replay_test;
extern bool zq_ignore_item_ownership, zq_view_fullctr, zq_view_maxctr,
	zq_view_noinf, zq_view_allinf;
extern int zq_subscr_override_dmap;
#endif

extern dword loading_tileset_flags;

extern const GUI::ListData subscrWidgets;

int32_t get_dlevel();
int32_t get_currdmap();
int32_t get_homescr();
bool has_item(int32_t item_type, int32_t item);

//!TODO subscr.h/subscr.cpp trim
int32_t subscreen_color(int32_t c1, int32_t c2);
void draw_textbox(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, FONT *tempfont, char const*thetext, bool wword, int32_t tabsize, int32_t alignment, int32_t textstyle, int32_t color, int32_t shadowcolor, int32_t backcolor);
void magicgauge(BITMAP *dest,int32_t x,int32_t y, int32_t container, int32_t notlast_tile, int32_t notlast_cset, bool notlast_mod, int32_t last_tile, int32_t last_cset, bool last_mod,
				int32_t cap_tile, int32_t cap_cset, bool cap_mod, int32_t aftercap_tile, int32_t aftercap_cset, bool aftercap_mod, int32_t frames, int32_t speed, int32_t delay, bool unique_last, int32_t show);

const std::string subwidg_internal_names[widgMAX] =
{
	"SUBWIDG_NULL", "SUBWIDG_FRAME", "SUBWIDG_TEXT", "SUBWIDG_LINE", "SUBWIDG_RECT",
	"SUBWIDG_TIME", "SUBWIDG_MMETER", "SUBWIDG_LMETER", "SUBWIDG_BTNITM", "SUBWIDG_COUNTER",
	"SUBWIDG_OLDCTR", "SUBWIDG_MMAPTITLE", "SUBWIDG_MMAP", "SUBWIDG_LMAP", "SUBWIDG_BGCOLOR",
	"SUBWIDG_ITEMSLOT", "SUBWIDG_MCGUFF_FRAME", "SUBWIDG_MCGUFF", "SUBWIDG_TILEBLOCK", "SUBWIDG_MINITILE",
	"SUBWIDG_SELECTOR", "SUBWIDG_LGAUGE", "SUBWIDG_MGAUGE", "SUBWIDG_TEXTBOX", "SUBWIDG_SELECTEDTEXT",
	"SUBWIDG_MISCGAUGE", "SUBWIDG_BTNCOUNTER",
};
const std::string subscr_names[sstMAX] = {"Active","Passive","Overlay"};
const std::string subscr_infos[sstMAX] = {
	"The subscreen that actively opens when you press 'Start'",
	"The subscreen visible at the top of the screen normally, which moves down when the active opens.",
	"Like the passive, but visible across the whole screen and does NOT move down for the active opening."
	};

SubscrTransition subscr_pg_transition;
int subscr_item_clk = 0, subscr_pg_clk = 0;
byte subscr_pg_from, subscr_pg_to;
static ZCSubscreen* subscr_anim = nullptr;

int subscr_override_clkoffsets[MAXITEMS];
bool subscr_itemless = false, subscr_pg_animating = false;
int btnitem_clks[4] = {0};
int btnitem_ids[4] = {-1,-1,-1,-1};
BITMAP *subscr_pg_bmp1 = nullptr, *subscr_pg_bmp2 = nullptr, *subscr_pg_subbmp = nullptr;


void refresh_subscr_buttonitems()
{
	for(int q = 0; q < 4; ++q)
	{
		btnitem_clks[q] = 0;
		btnitem_ids[q] = -1;
	}
}

void animate_subscr_buttonitems()
{
	int nullval = get_qr(qr_ITM_0_INVIS_ON_BTNS) ? 0 : -1;
	int ids[] = {Awpn,Bwpn,Xwpn,Ywpn};
	for(int q = 0; q < 4; ++q)
	{
		if(btnitem_ids[q] != ids[q])
		{
			btnitem_ids[q] = ids[q];
			btnitem_clks[q] = 0;
		}
		if(ids[q] > nullval)
			++btnitem_clks[q];
	}
}

static int get_subscr_item_id(int family, bool compat = false)
{
	if(compat && replay_version_check(0,24))
		return current_item_id(family,true,false,false);
	return current_item_id(family,false,false,false);
}

static int calc_item_from_class_id_button(int specific_item_id, int button_id, int item_class)
{
	int item_id = -1;
	if (specific_item_id > -1)
		item_id = specific_item_id;
	else if (button_id > -1)
	{
		if (button_id >= 4) return 0;
		int ids[] = { Awpn,Bwpn,Xwpn,Ywpn };
		item_id = NEG_OR_MASK(ids[button_id], 0xFF);
	}
	else
	{
		auto family = -1;
		switch (item_class)
		{
			case itype_bowandarrow:
			case itype_arrow:
#ifdef IS_PLAYER
				if (get_subscr_item_id(itype_bow) > -1
					&& get_subscr_item_id(itype_arrow) > -1)
					family = itype_arrow;
#else
				family = itype_arrow;
#endif
				break;
			case itype_letterpotion:
#ifdef IS_PLAYER
				if (get_subscr_item_id(itype_potion) > -1)
					family = itype_potion;
				else if (get_subscr_item_id(itype_letter) > -1)
					family = itype_letter;
#else
				if (get_subscr_item_id(itype_potion) > -1)
					family = itype_potion;
				else family = itype_letter;
#endif
				break;
			default:
				family = item_class;
				break;
		}
		if (family < 0) return 0;
		item_id = get_subscr_item_id(family);
	}
	return item_id;
}

void refresh_subscr_items()
{
	subscr_itemless = false;
	if(replay_version_check(0,19))
	{
		subscr_item_clk = 0;
		//This needs to be here for the item cache to be correct...
		for(int i = 0; i < itype_max; ++i)
		{
			switch(i)
			{
				case itype_map:
				case itype_compass:
				case itype_bosskey:
					continue;
			}
			get_subscr_item_id(i);
		}
	}
}

void kill_subscr_items()
{
	subscr_itemless = true;
	for(int q = 0; q < MAXITEMS; ++q)
	{
		subscr_override_clkoffsets[q] = -1;
	}
}

bool is_counter_item(int32_t itmid, int32_t ctr)
{
	itemdata const& itm = itemsbuf[itmid];
	if(ctr == crNONE) return false;
	if(ctr == itm.cost_counter[0] ||
		ctr == itm.cost_counter[1])
		return true;
    return false;
}

int old_ssc_to_new_ctr(int ssc)
{
	switch(ssc)
	{
		case 0: return crMONEY;
		case 1: return crBOMBS;
		case 2: return crSBOMBS;
		case 3: return crARROWS;
		case 4: return sscGENKEYMAGIC;
		case 5: return sscGENKEYNOMAGIC;
		case 6: return sscLEVKEYMAGIC;
		case 7: return sscLEVKEYNOMAGIC;
		case 8: return sscANYKEYMAGIC;
		case 9: return sscANYKEYNOMAGIC;
		case 35: return crLIFE;
		case 36: return crMAGIC;
		case 37: return sscMAXHP;
		case 38: return sscMAXMP;
		default:
			if(ssc < 0) return ssc;
			if(ssc >= 10 && ssc <= 34)
				return crCUSTOM1+(ssc-10);
			if(ssc >= 39 && ssc <= 104)
				return crCUSTOM26+(ssc-39);
			return crNONE;
	}
}
static int simplify_counter(int ctr)
{
	switch(ctr)
	{
		case sscBTNCTRA_0:
		{
			itemdata const& itm = itemsbuf[Awpn&0xFF];
			if(!itm.cost_amount[0]) return crNONE;
			return itm.cost_counter[0];
		}
		case sscBTNCTRA_1:
		{
			itemdata const& itm = itemsbuf[Awpn&0xFF];
			if(!itm.cost_amount[1]) return crNONE;
			return itm.cost_counter[1];
		}
		case sscBTNCTRB_0:
		{
			itemdata const& itm = itemsbuf[Bwpn&0xFF];
			if(!itm.cost_amount[0]) return crNONE;
			return itm.cost_counter[0];
		}
		case sscBTNCTRB_1:
		{
			itemdata const& itm = itemsbuf[Bwpn&0xFF];
			if(!itm.cost_amount[1]) return crNONE;
			return itm.cost_counter[1];
		}
		case sscBTNCTRX_0:
		{
			itemdata const& itm = itemsbuf[Xwpn&0xFF];
			if(!itm.cost_amount[0]) return crNONE;
			return itm.cost_counter[0];
		}
		case sscBTNCTRX_1:
		{
			itemdata const& itm = itemsbuf[Xwpn&0xFF];
			if(!itm.cost_amount[1]) return crNONE;
			return itm.cost_counter[1];
		}
		case sscBTNCTRY_0:
		{
			itemdata const& itm = itemsbuf[Ywpn&0xFF];
			if(!itm.cost_amount[0]) return crNONE;
			return itm.cost_counter[0];
		}
		case sscBTNCTRY_1:
		{
			itemdata const& itm = itemsbuf[Ywpn&0xFF];
			if(!itm.cost_amount[1]) return crNONE;
			return itm.cost_counter[1];
		}
	}
	return ctr;
}
word get_ssc_ctrmax(int ctr)
{
	ctr = simplify_counter(ctr);
	if(ctr == crNONE)
		return 0;
	if(zq_view_maxctr)
		return 65535;
	dword ret = 0;
	switch(ctr)
	{
		case crARROWS:
			if(!get_qr(qr_TRUEARROWS))
				ctr = crMONEY;
			break;
		
		case sscMAXHP:
		{
			ret = game->get_maxlife();
			break;
		}
		case sscMAXMP:
		{
			ret = game->get_maxmagic();
			break;
		}
		case sscGENKEYMAGIC:
		case sscLEVKEYMAGIC:
		case sscANYKEYMAGIC:
		case sscANYKEYNOMAGIC:
		case sscLEVKEYNOMAGIC:
		case sscGENKEYNOMAGIC:
			if(ctr == sscGENKEYNOMAGIC || ctr == sscANYKEYNOMAGIC
					|| ctr == sscGENKEYMAGIC || ctr == sscANYKEYMAGIC)
				ret = game->get_maxcounter(crKEYS);
				
			if(ctr == sscLEVKEYNOMAGIC || ctr == sscANYKEYNOMAGIC
					|| ctr == sscLEVKEYMAGIC || ctr == sscANYKEYMAGIC)
				ret = 65535;
				
			break;
	}
	if(ctr > -1)
		return game->get_maxcounter(ctr);
	return zc_min(65535,ret);
}
word get_ssc_ctr(int ctr, bool* infptr)
{
	ctr = simplify_counter(ctr);
	if(ctr == crNONE)
		return 0;
	dword ret = 0;
	bool inf = false;
	switch(ctr)
	{
		case crMONEY:
			if(current_item_power(itype_wallet))
				inf = true;
			break;
		case crBOMBS:
			if(current_item_power(itype_bombbag))
				inf = true;
			break;
		case crSBOMBS:
		{
			int32_t itemid = get_subscr_item_id(itype_bombbag, true);
			if(itemid>-1 && itemsbuf[itemid].power>0 && itemsbuf[itemid].flags & item_flag1)
				inf = true;
			break;
		}
		case crARROWS:
			if(current_item_power(itype_quiver))
				inf = true;
			if(!get_qr(qr_TRUEARROWS))
			{
				if(current_item_power(itype_wallet))
					inf = true;
				ctr = crMONEY;
			}
			break;
		
		case sscMAXHP:
		{
			ret = game->get_maxlife();
			break;
		}
		case sscMAXMP:
		{
			ret = game->get_maxmagic();
			break;
		}
		case sscGENKEYMAGIC:
		case sscLEVKEYMAGIC:
		case sscANYKEYMAGIC:
		{
			int32_t itemid = get_subscr_item_id(itype_magickey, true);
			if(itemid>-1)
			{
				if(itemsbuf[itemid].flags&item_flag1)
					inf = itemsbuf[itemid].power>=get_dlevel();
				else
					inf = itemsbuf[itemid].power==get_dlevel();
			}
		}
		[[fallthrough]];
		case sscANYKEYNOMAGIC:
		case sscLEVKEYNOMAGIC:
		case sscGENKEYNOMAGIC:
			if(ctr == sscGENKEYNOMAGIC || ctr == sscANYKEYNOMAGIC
					|| ctr == sscGENKEYMAGIC || ctr == sscANYKEYMAGIC)
				ret += game->get_keys();
				
			if(ctr == sscLEVKEYNOMAGIC || ctr == sscANYKEYNOMAGIC
					|| ctr == sscLEVKEYMAGIC || ctr == sscANYKEYMAGIC)
				ret += game->get_lkeys();
				
			break;
	}
	if(infptr)
		*infptr = inf;
	if(zq_view_fullctr) return get_ssc_ctrmax(ctr);
	if(ctr > -1)
		return inf ? game->get_maxcounter(ctr) : game->get_counter(ctr);
	return inf ? 65535 : zc_min(65535,ret);
}
void modify_ssc_ctr(int ctr, int amnt, bool gradual)
{
	ctr = simplify_counter(ctr);
	if(ctr == crNONE)
		return;
	bool inf = false;
	
	switch(ctr)
	{
		case crMONEY:
			if(current_item_power(itype_wallet))
				inf = true;
			break;
		case crBOMBS:
			if(current_item_power(itype_bombbag))
				inf = true;
			break;
		case crSBOMBS:
		{
			int32_t itemid = get_subscr_item_id(itype_bombbag, true);
			if(itemid>-1 && itemsbuf[itemid].power>0 && itemsbuf[itemid].flags & item_flag1)
				inf = true;
			break;
		}
		case crARROWS:
			if(current_item_power(itype_quiver))
				inf = true;
			if(!get_qr(qr_TRUEARROWS))
			{
				if(current_item_power(itype_wallet))
					inf = true;
				ctr = crMONEY;
			}
			break;
		
		case sscMAXHP:
		{
			game->change_maxlife(amnt);
			break;
		}
		case sscMAXMP:
		{
			game->change_maxmagic(amnt);
			break;
		}
		case sscGENKEYMAGIC:
		case sscLEVKEYMAGIC:
		case sscANYKEYMAGIC:
		{
			int32_t itemid = get_subscr_item_id(itype_magickey, true);
			if(itemid>-1)
			{
				if(itemsbuf[itemid].flags&item_flag1)
					inf = itemsbuf[itemid].power>=get_dlevel();
				else
					inf = itemsbuf[itemid].power==get_dlevel();
			}
		}
		[[fallthrough]];
		case sscANYKEYNOMAGIC:
		case sscLEVKEYNOMAGIC:
		case sscGENKEYNOMAGIC:
			if(inf && amnt < 0) return;
				
			if(ctr == sscLEVKEYNOMAGIC || ctr == sscANYKEYNOMAGIC
				|| ctr == sscLEVKEYMAGIC || ctr == sscANYKEYMAGIC)
			{
				if(amnt > 0 || -amnt < game->lvlkeys[dlevel])
				{
					game->lvlkeys[dlevel] += amnt;
					amnt = 0;
					break; // modification fulfilled
				}
				else
				{
					amnt += game->lvlkeys[dlevel];
					game->lvlkeys[dlevel] = 0;
					// fallthrough to general keys
				}
			}
			
			if(ctr == sscGENKEYNOMAGIC || ctr == sscANYKEYNOMAGIC
				|| ctr == sscGENKEYMAGIC || ctr == sscANYKEYMAGIC)
			{
				ctr = crKEYS;
			}
				
			break;
	}
	if(inf && amnt < 0) return;
	if(ctr > -1)
	{
		if(gradual)
			game->change_dcounter(amnt, ctr);
		else game->change_counter(amnt, ctr);
	}
}
void add_ssc_ctr(int ctr, bool& infinite, int32_t& value)
{
	bool inf = false;
	value += get_ssc_ctr(ctr, &inf);
	if(inf) infinite = true;
}
bool is_full_ssc_ctr(int ctr)
{
	return get_ssc_ctr(ctr) >= get_ssc_ctrmax(ctr);
}
bool can_inf(int ctr, int infitm = -1)
{
	switch(ctr)
	{
		case crMONEY:
		case crBOMBS:
		case crSBOMBS:
		case crARROWS:
		case sscGENKEYMAGIC:
		case sscLEVKEYMAGIC:
		case sscANYKEYMAGIC:
			return true;
	}
	return infitm > -1 && infitm < MAXITEMS;
}

int32_t to_real_font(int32_t ss_font)
{
	switch(ss_font)
	{
		case ssfSMALL: return font_sfont;
		case ssfSMALLPROP: return font_spfont;
		case ssfPROP: return font_nfont;
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

int wrap_iid(int iid)
{
	if(unsigned(iid) >= MAXITEMS)
		return -1;
	return iid;
}

int get_sub_dmap()
{
#if IS_EDITOR
	if(zq_subscr_override_dmap > -1)
		return zq_subscr_override_dmap;
#endif
	return get_currdmap();
}

void subscrpg_clear_animation()
{
	subscr_pg_animating = false;
	subscr_pg_clk = subscr_pg_from = subscr_pg_to = 0;
	subscr_pg_transition.clear();
	subscr_anim = nullptr;
	if(subscr_pg_bmp1)
		destroy_bitmap(subscr_pg_bmp1);
	if(subscr_pg_bmp2)
		destroy_bitmap(subscr_pg_bmp2);
	if(subscr_pg_subbmp)
		destroy_bitmap(subscr_pg_subbmp);
	subscr_pg_bmp1 = subscr_pg_bmp2 = subscr_pg_subbmp = nullptr;
}

#define SUBSCR_ACTIVE_WIDTH 256
#define SUBSCR_ACTIVE_HEIGHT 168
bool subscrpg_animate(byte from, byte to, SubscrTransition const& transition, ZCSubscreen& parent)
{
	if(subscr_pg_animating)
		return false;
	subscrpg_clear_animation();
	if(transition.tr_sfx)
		sfx(transition.tr_sfx);
	subscr_pg_from = from;
	subscr_pg_to = to;
	subscr_pg_transition = transition;
	subscr_pg_animating = true;
	subscr_anim = &parent;
	if(transition.type != sstrINSTANT)
	{
		subscr_pg_bmp1 = create_bitmap_ex(8,SUBSCR_ACTIVE_WIDTH,SUBSCR_ACTIVE_HEIGHT);
		subscr_pg_bmp2 = create_bitmap_ex(8,SUBSCR_ACTIVE_WIDTH,SUBSCR_ACTIVE_HEIGHT);
		subscr_pg_subbmp = create_bitmap_ex(8,SUBSCR_ACTIVE_WIDTH,SUBSCR_ACTIVE_HEIGHT);
	}
	return true;
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
			if(color == -1)
				ret = -1;
			else if(get_qr(qr_BROKEN_SYSTEM_COLORS))
				ret = vc(color);
			else
			{
				switch(color)
				{
					case 0:
						ret = makecol(0, 0, 0);
						break;
					case 1:
						ret = makecol(0, 0, 170);
						break;
					case 2:
						ret = makecol(0, 157, 0);
						break;
					case 3:
						ret = makecol(0, 170, 170);
						break;
					case 4:
						ret = makecol(178, 36, 36);
						break;
					case 5:
						ret = makecol(170, 0, 170);
						break;
					case 6:
						ret = makecol(165, 105, 8);
						break;
					case 7:
						ret = makecol(170, 170, 170);
						break;
					case 8:
						ret = makecol(85, 85, 85);
						break;
					case 9:
						ret = makecol(85, 85, 255);
						break;
					case 10:
						ret = makecol(85, 255, 85);
						break;
					case 11:
						ret = makecol(85, 255, 255);
						break;
					case 12:
						ret = makecol(255, 85, 85);
						break;
					case 13:
						ret = makecol(255, 85, 255);
						break;
					case 14:
						ret = makecol(255, 255, 85);
						break;
					case 15:
						ret = makecol(255, 255, 255);
						break;
				}
			}
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
int32_t SubscrColorInfo::get_int_color() const
{
	if(type == ssctSYSTEM)
	{
		if(color == -1)
			return SUB_COLOR_TRANSPARENT;
		return -(color+1);
	}
	if(type == ssctMISC)
		return -(color+1+NUM_SYS_COLORS);
	if(type < 16)
		return (type*16)+color;
	return 0;
}
void SubscrColorInfo::set_int_color(int32_t val)
{
	if(val > 255 || val < -ssctMAX-NUM_SYS_COLORS) return;
	if(val >= 0)
	{
		type = (val&0xF0)>>4;
		color = (val&0x0F);
	}
	else if(val == SUB_COLOR_TRANSPARENT)
	{
		type = ssctSYSTEM;
		color = -1;
	}
	else if(val >= -NUM_SYS_COLORS)
	{
		type = ssctSYSTEM;
		color = (-val)-1;
	}
	else
	{
		type = ssctMISC;
		color = (-val)-1-NUM_SYS_COLORS;
	}
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
int32_t SubscrColorInfo::get_int_cset() const
{
	if(type == ssctMISC)
		return -(type+1);
	if(type < 16)
		return type;
	return 0;
}
void SubscrColorInfo::set_int_cset(int32_t val)
{
	if(val > 15 || val < -sscsMAX) return;
	if(val >= 0)
	{
		type = val;
		color = 0;
	}
	else
	{
		type = ssctMISC;
		color = (-val)-1;
	}
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
		new_return(1);
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
	return mt_tile;
}
byte SubscrMTInfo::crn() const
{
	return mt_crn;
}
int32_t SubscrMTInfo::getTileCrn() const
{
	return (mt_tile<<2)|(mt_crn%4);
}
void SubscrMTInfo::setTileCrn(int32_t tile, byte crn)
{
	mt_tile = tile;
	mt_crn = crn;
}
void SubscrMTInfo::setTileCrn(int32_t tilecrn)
{
	mt_tile = tilecrn>>2;
	mt_crn = tilecrn%4;
}
void SubscrMTInfo::setTile(int32_t tile)
{
	mt_tile = tile;
}
void SubscrMTInfo::setCrn(byte crn)
{
	mt_crn = crn;
}
int32_t SubscrMTInfo::read(PACKFILE *f, word s_version)
{
	int32_t tilecrn;
	if(!p_igetl(&tilecrn,f))
		return qe_invalid;
	setTileCrn(tilecrn);
	if(!p_getc(&cset,f))
		return qe_invalid;
	return 0;
}
int32_t SubscrMTInfo::write(PACKFILE *f) const
{
	if(!p_iputl(getTileCrn(),f))
		new_return(1);
	if(!p_putc(cset,f))
		new_return(1);
	return 0;
}

void SubscrSelectorTileInfo::clear()
{
	*this = SubscrSelectorTileInfo();
}
int32_t SubscrSelectorTileInfo::read(PACKFILE *f, word s_version)
{
	if(!p_igetw(&sw,f))
		return qe_invalid;
	if(!p_igetw(&sh,f))
		return qe_invalid;
	if(!p_igetl(&tile,f))
		return qe_invalid;
	if(!p_getc(&cset,f))
		return qe_invalid;
	if(!p_getc(&frames,f))
		return qe_invalid;
	if(!p_getc(&speed,f))
		return qe_invalid;
	if(!p_getc(&delay,f))
		return qe_invalid;
	return 0;
}
int32_t SubscrSelectorTileInfo::write(PACKFILE *f) const
{
	if(!p_iputw(sw,f))
		new_return(1);
	if(!p_iputw(sh,f))
		new_return(1);
	if(!p_iputl(tile,f))
		new_return(1);
	if(!p_putc(cset,f))
		new_return(1);
	if(!p_putc(frames,f))
		new_return(1);
	if(!p_putc(speed,f))
		new_return(1);
	if(!p_putc(delay,f))
		new_return(1);
	return 0;
}

void SubscrSelectorInfo::clear()
{
	*this = SubscrSelectorInfo();
}
int32_t SubscrSelectorInfo::read(PACKFILE *f, word s_version)
{
	if(!p_igetw(&x,f))
		return qe_invalid;
	if(!p_igetw(&y,f))
		return qe_invalid;
	if(!p_igetw(&w,f))
		return qe_invalid;
	if(!p_igetw(&h,f))
		return qe_invalid;
	byte sz;
	if(!p_getc(&sz,f))
		return qe_invalid;
	SubscrSelectorTileInfo dummy;
	for(byte q = 0; q < sz; ++q)
	{
		SubscrSelectorTileInfo& info = (unsigned(q)>=2) ? dummy : tileinfo[q];
		if(auto ret = info.read(f,s_version))
			return ret;
	}
	return 0;
}
int32_t SubscrSelectorInfo::write(PACKFILE *f) const
{
	if(!p_iputw(x,f))
		new_return(1);
	if(!p_iputw(y,f))
		new_return(1);
	if(!p_iputw(w,f))
		new_return(1);
	if(!p_iputw(h,f))
		new_return(1);
	byte sz = 2;
	if(!p_putc(sz,f))
		new_return(1);
	SubscrSelectorTileInfo dummy;
	for(byte q = 0; q < sz; ++q)
	{
		SubscrSelectorTileInfo const& info = tileinfo[q];
		if(auto ret = info.write(f))
			return ret;
	}
	return 0;
}



bool SubscrTransition::draw(BITMAP* dest, BITMAP* p1, BITMAP* p2, int dx, int dy)
{	//Returns true for failure/end of animation
	if(type <= 0 || type >= sstrMAX)
		return true; //invalid animation
	if(!subscr_pg_subbmp)
		subscr_pg_subbmp = create_bitmap_ex(8,SUBSCR_ACTIVE_WIDTH,SUBSCR_ACTIVE_HEIGHT);
	clear_bitmap(subscr_pg_subbmp);
	switch(type)
	{
		case sstrSLIDE:
		{
			zfix spd = zslongToFix(arg[1]);
			int dir = NORMAL_DIR(arg[0]);
			int diff = subscr_pg_clk*spd;
			int x1=0, y1=0, x2=0, y2=0;
			switch(dir)
			{
				case up:
					if(diff >= SUBSCR_ACTIVE_HEIGHT)
						return true; //end of animation
					y1 = -diff;
					y2 = SUBSCR_ACTIVE_HEIGHT - diff;
					break;
				case down:
					if(diff >= SUBSCR_ACTIVE_HEIGHT)
						return true; //end of animation
					y1 = diff;
					y2 = diff - SUBSCR_ACTIVE_HEIGHT;
					break;
				case left:
					if(diff >= SUBSCR_ACTIVE_WIDTH)
						return true; //end of animation
					x1 = -diff;
					x2 = SUBSCR_ACTIVE_WIDTH - diff;
					break;
				case right:
					if(diff >= SUBSCR_ACTIVE_WIDTH)
						return true; //end of animation
					x1 = diff;
					x2 = diff - SUBSCR_ACTIVE_WIDTH;
					break;
			}
			blit(p1, subscr_pg_subbmp, 0, 0, x1, y1, SUBSCR_ACTIVE_WIDTH, SUBSCR_ACTIVE_HEIGHT);
			blit(p2, subscr_pg_subbmp, 0, 0, x2, y2, SUBSCR_ACTIVE_WIDTH, SUBSCR_ACTIVE_HEIGHT);
			break;
		}
		case sstrPIXEL:
		{
			int dur = arg[1];
			if(subscr_pg_clk > dur)
				return true;
			int pat_xofs = arg[2], pat_yofs = arg[3];
			double perc = subscr_pg_clk / double(dur+1);
			double iperc = 1.0-perc;
			bool inv = arg[0]&TR_PIXELATE_INVERT;
			if(inv)
				zc_swap(perc,iperc);
			custom_bmp_dither(subscr_pg_subbmp, p1, [&](int x, int y, int, int)
				{
					return !dither_staticcheck(x+pat_xofs,y+pat_yofs,iperc) != !inv;
				});
			custom_bmp_dither(subscr_pg_subbmp, p2, [&](int x, int y, int, int)
				{
					return !dither_staticcheck(x+pat_xofs,y+pat_yofs,perc) != !inv;
				});
			break;
		}
		default:
			return true; //unrecognized animation type
	}
	blit(subscr_pg_subbmp, dest, 0, 0, dx, dy, SUBSCR_ACTIVE_WIDTH, SUBSCR_ACTIVE_HEIGHT);
	return false;
}
void SubscrTransition::clear()
{
	*this = SubscrTransition();
}
int32_t SubscrTransition::read(PACKFILE *f, word s_version)
{
	if(!p_getc(&type,f))
		return qe_invalid;
	if(!p_getc(&tr_sfx,f))
		return qe_invalid;
	if(!p_igetw(&flags,f))
		return qe_invalid;
	byte args = 3;
	if(!p_getc(&args,f))
		return qe_invalid;
	for(int q = 0; q < args; ++q)
		if(!p_igetl(&arg[q],f))
			return qe_invalid;
	for(int q = args; q < SUBSCR_TRANSITION_MAXARG; ++q)
		arg[q] = 0;
	return 0;
}
int32_t SubscrTransition::write(PACKFILE *f) const
{
	if(!p_putc(type,f))
		new_return(1);
	if(!p_putc(tr_sfx,f))
		new_return(1);
	if(!p_iputw(flags,f))
		new_return(1);
	byte args = num_args(type);
	if(!p_putc(args,f))
		new_return(1);
	for(int q = 0; q < args; ++q)
		if(!p_iputl(arg[q],f))
			new_return(1);
	return 0;
}
byte SubscrTransition::num_args(byte ty)
{
	switch(ty)
	{
		case sstrINSTANT:
			return 0;
		case sstrSLIDE:
			return 2;
		case sstrPIXEL:
			return 4;
	}
	return 0;
}
int32_t SubscrTransition::argScale(byte ty, byte ind)
{
	switch(ty)
	{
		case sstrSLIDE:
			switch(ind)
			{
				case 1:
					return 1;
			}
			break;
	}
	return 10000;
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
	posflags = old.pos&0x7;
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
byte SubscrWidget::getType() const
{
	return widgNULL;
}
int32_t SubscrWidget::getItemVal() const
{
	return -1;
}
int32_t SubscrWidget::getDisplayItem() const
{
	return -1;
}
void SubscrWidget::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	
}
bool SubscrWidget::visible(byte pos, bool showtime) const
{
	#ifdef IS_PLAYER
	if(msg_onscreen && (posflags&sspNOMSGSTR))
		return false;
	#endif
	if(!check_conditions())
		return false;
	return !pos || (posflags&pos);
}
SubscrWidget* SubscrWidget::clone() const
{
	return new SubscrWidget(*this);
}
bool SubscrWidget::copy_prop(SubscrWidget const* src, bool all)
{
	if((src->getType() == widgNULL && getType() != widgNULL) || src == this)
		return false;
	flags = src->flags;
	genflags = src->genflags;
	posflags = src->posflags;
	req_owned_items = src->req_owned_items;
	req_unowned_items = src->req_unowned_items;
	req_counter = src->req_counter;
	req_counter_val = src->req_counter_val;
	req_counter_cond_type = src->req_counter_cond_type;
	req_litems = src->req_litems;
	req_litem_level = src->req_litem_level;
	is_disabled = src->is_disabled;
	if(all)
	{
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
		for(int q = 0; q < 8; ++q)
			generic_initd[q] = src->generic_initd[q];
		type = src->type;
		pg_btns = src->pg_btns;
		pg_mode = src->pg_mode;
		if(!pg_mode || !pg_btns)
		{
			pg_mode = pg_btns = pg_targ = 0;
			pg_trans.clear();
		}
		else
		{
			pg_targ = src->pg_targ;
			pg_trans = src->pg_trans;
		}
		if(genflags & SUBSCRFLAG_SELOVERRIDE)
			selector_override = src->selector_override;
		else
			selector_override.clear();
		label = src->label;
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
	if(!p_igetl(&genflags,f))
		return qe_invalid;
	if(!p_igetl(&flags,f))
		return qe_invalid;
	if(s_version >= 9)
		if(!p_getwstr(&label,f))
			return qe_invalid;
	if(s_version >= 11)
		if(!p_getc(&compat_flags,f))
			return qe_invalid;
	if(genflags & SUBSCRFLAG_SELECTABLE)
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
		if(!p_getwstr(&override_text,f))
			return qe_invalid;
		if(genflags & SUBSCRFLAG_SELOVERRIDE)
			if(auto ret = selector_override.read(f,s_version))
				return ret;
		if(!p_igetw(&generic_script,f))
			return qe_invalid;
		if(generic_script)
		{
			if(!p_getc(&gen_script_btns,f))
				return qe_invalid;
			for(int q = 0; q < 8; ++q)
				if(!p_igetl(&generic_initd[q],f))
					return qe_invalid;
		}
		if(!p_getc(&pg_mode,f))
			return qe_invalid;
		if(pg_mode)
		{
			if(!p_getc(&pg_btns,f))
				return qe_invalid;
			if(!p_getc(&pg_targ,f))
				return qe_invalid;
			if(auto ret = pg_trans.read(f, s_version))
				return ret;
		}
	}
	if(s_version >= 14)
	{
		word count;
		byte iid;
		if(!p_igetw(&count,f))
			return qe_invalid;
		req_owned_items.clear();
		for(word q = 0; q < count; ++q)
		{
			if(!p_getc(&iid,f))
				return qe_invalid;
			req_owned_items.insert(iid);
		}
		if(!p_igetw(&count,f))
			return qe_invalid;
		req_unowned_items.clear();
		for(word q = 0; q < count; ++q)
		{
			if(!p_getc(&iid,f))
				return qe_invalid;
			req_unowned_items.insert(iid);
		}
		if(!p_igetw(&req_counter,f))
			return qe_invalid;
		if(!p_igetw(&req_counter_val,f))
			return qe_invalid;
		if(!p_getc(&req_counter_cond_type,f))
			return qe_invalid;
		if(s_version >= 15)
		{
			if(!p_igetw(&req_litems,f))
				return qe_invalid;
		}
		else
		{
			byte b;
			if(!p_getc(&b,f))
				return qe_invalid;
			req_litems = word(b);
		}
		if(!p_igetw(&req_litem_level,f))
			return qe_invalid;
		byte tempb;
		if(!p_getc(&tempb,f))
			return qe_invalid;
		is_disabled = tempb != 0;
	}
	
	if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
	{
		generic_script = 0;
		gen_script_btns = 0;
		for(int q = 0; q < 8; ++q)
			generic_initd[q] = 0;
	}
	return 0;
}
int32_t SubscrWidget::write(PACKFILE *f) const
{
	if(!p_putc(getType(),f))
		new_return(1);
	if(!p_putc(type,f))
		new_return(1);
	if(!p_putc(posflags,f))
		new_return(1);
	if(!p_iputw(x,f))
		new_return(1);
	if(!p_iputw(y,f))
		new_return(1);
	if(!p_iputw(w,f))
		new_return(1);
	if(!p_iputw(h,f))
		new_return(1);
	if(!p_iputl(genflags,f))
		new_return(1);
	if(!p_iputl(flags,f))
		new_return(1);
	if(!p_putwstr(label,f))
		new_return(1);
	if(!p_putc(compat_flags,f))
		new_return(1);
	if(genflags & SUBSCRFLAG_SELECTABLE)
	{
		if(!p_iputl(pos,f))
			new_return(1);
		if(!p_iputl(pos_up,f))
			new_return(1);
		if(!p_iputl(pos_down,f))
			new_return(1);
		if(!p_iputl(pos_left,f))
			new_return(1);
		if(!p_iputl(pos_right,f))
			new_return(1);
		if(!p_putwstr(override_text,f))
			new_return(1);
		if(genflags & SUBSCRFLAG_SELOVERRIDE)
			if(auto ret = selector_override.write(f))
				return ret;
		if(!p_iputw(generic_script,f))
			new_return(1);
		if(generic_script)
		{
			if(!p_putc(gen_script_btns,f))
				new_return(1);
			for(int q = 0; q < 8; ++q)
				if(!p_iputl(generic_initd[q],f))
					new_return(1);
		}
		if(!p_putc(pg_mode,f))
			new_return(1);
		if(pg_mode)
		{
			if(!p_putc(pg_btns,f))
				new_return(1);
			if(!p_putc(pg_targ,f))
				new_return(1);
			if(auto ret = pg_trans.write(f))
				return ret;
		}
	}
	if(!p_iputw(req_owned_items.size(),f))
		new_return(1);
	for(byte iid : req_owned_items)
		if(!p_putc(iid,f))
			new_return(1);
	if(!p_iputw(req_unowned_items.size(),f))
		new_return(1);
	for(byte iid : req_unowned_items)
		if(!p_putc(iid,f))
			new_return(1);
	if(!p_iputw(req_counter,f))
		new_return(1);
	if(!p_iputw(req_counter_val,f))
		new_return(1);
	if(!p_putc(req_counter_cond_type,f))
		new_return(1);
	if(!p_iputw(req_litems,f))
		new_return(1);
	if(!p_iputw(req_litem_level,f))
		new_return(1);
	if(!p_putc(is_disabled?1:0,f))
		new_return(1);
	return 0;
}
void SubscrWidget::check_btns(byte btnflgs, ZCSubscreen& parent) const
{
	if(pg_mode && (btnflgs&pg_btns))
		parent.page_change(pg_mode, pg_targ, pg_trans, genflags&SUBSCRFLAG_PGGOTO_NOWRAP);
}
std::string SubscrWidget::getTypeName() const
{
	return GUI::ZCListData::subscr_widgets().findText(getType());
}
bool SubscrWidget::check_conditions() const
{
	// handle editor preview of conditions somehow?
#ifdef IS_PLAYER
	if(is_disabled) // script-disable condition
		return false;
	for(auto iid : req_owned_items)
	{
		if(!game->get_item(iid))
			return false;
	}
	for(auto iid : req_unowned_items)
	{
		if(game->get_item(iid))
			return false;
	}
	if(req_counter != crNONE && req_counter_cond_type != CONDTY_NONE)
	{
		zfix val = get_ssc_ctr(req_counter);
		if(genflags&SUBSCRFLAG_REQ_COUNTER_PERC)
			val = (val / get_ssc_ctrmax(req_counter)) * 100_zf;
		else if(genflags&SUBSCRFLAG_REQ_COUNTER_MAX)
			val = get_ssc_ctrmax(req_counter);
		zfix targ_val = req_counter_val;
		switch(req_counter_cond_type)
		{
			case CONDTY_EQ:
				if(!(val == targ_val))
					return false;
				break;
			case CONDTY_NEQ:
				if(!(val != targ_val))
					return false;
				break;
			case CONDTY_GREATER:
				if(!(val > targ_val))
					return false;
				break;
			case CONDTY_GREATEREQ:
				if(!(val >= targ_val))
					return false;
				break;
			case CONDTY_LESS:
				if(!(val < targ_val))
					return false;
				break;
			case CONDTY_LESSEQ:
				if(!(val <= targ_val))
					return false;
				break;
		}
	}
	if(req_litems)
	{
		auto target_lvl = req_litem_level < 0 ? get_dlevel() : req_litem_level;
		if(!(target_lvl < 0 || target_lvl >= MAXLEVELS))
		{
			bool inverted = genflags&SUBSCRFLAG_REQ_INVERT_LITEM;
			auto litems = game->lvlitems[target_lvl]&req_litems;
			if(inverted ? litems != 0 : litems != req_litems)
				return false;
		}
	}
#endif
	return true;
}
void SubscrWidget::replay_rand_compat(byte pos) const
{
	if((compat_flags & SUBSCRCOMPAT_FONT_RAND) && (posflags&pos) && replay_version_check(0,19))
		zc_oldrand();
}
SubscrPage const* SubscrWidget::getParentPage() const
{
	return parentPage;
}
ZCSubscreen const* SubscrWidget::getParentSub() const
{
	return parentPage ? parentPage->getParent() : nullptr;
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
	auto tx = x+shadow_x(shadtype);
	switch(align)
	{
		case sstaCENTER:
			return tx-getW()/2;
		case sstaRIGHT:
			return tx-getW();
	}
	return tx;
}
int16_t SW_Text::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Text::getW() const
{
	int32_t len = text_length(get_zc_font(fontid), text.c_str());
	return len == 0 ? 8 : len;
}
word SW_Text::getH() const
{
	return text_height(get_zc_font(fontid));
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(1);
	if(!p_putwstr(text,f))
		new_return(1);
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
	auto tx = x+shadow_x(shadtype);
	switch(align)
	{
		case sstaCENTER:
			return tx-getW()/2;
		case sstaRIGHT:
			return tx-getW();
	}
	return tx;
}
int16_t SW_Time::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Time::getW() const
{
	char *ts;
	auto tm = game ? game->get_time() : 0;
	if(flags&SUBSCR_TIME_ALTSTR)
		ts = time_str_short2(tm);
	else
		ts = time_str_med(tm);
	return text_length(get_zc_font(fontid), ts) + shadow_w(shadtype);
}
word SW_Time::getH() const
{
	return text_height(get_zc_font(fontid)) + shadow_h(shadtype);
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(1);
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
	lifemeter(dest, x+xofs, y+yofs, 1, flags&SUBSCR_LIFEMET_BOT);
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
	if(!show_subscreen_items) return;
	if(flags&SUBSCR_BTNITM_TRANSP)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	
	int nullval = get_qr(qr_ITM_0_INVIS_ON_BTNS) ? 0 : -1;
#ifdef IS_PLAYER
	if(replay_version_check(19))
		verifyBothWeapons();
#endif
	int ids[] = {Awpn,Bwpn,Xwpn,Ywpn};
	
	if(replay_version_check(19) && btnitem_ids[btn] != ids[btn])
	{
		btnitem_ids[btn] = ids[btn];
		btnitem_clks[btn] = 0;
	}
	if(btnitem_ids[btn] > nullval)
	{
		bool dodraw = true;
		switch(itemsbuf[btnitem_ids[btn]&0xFF].type)
		{
			case itype_arrow:
				if(btnitem_ids[btn]&0xF000)
				{
					int bow = get_subscr_item_id(itype_bow, true);
					if(bow>-1)
					{
						if(replay_version_check(0,19))
							putitem3(dest,x+xofs,y+yofs,bow,subscr_item_clk);
						if(!get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN)
							&& !checkmagiccost(btnitem_ids[btn]&0xFF))
							dodraw = false;
					}
				}
				break;
		}
		if(dodraw)
			putitem3(dest,x+xofs,y+yofs,btnitem_ids[btn]&0xFF,btnitem_clks[btn]);
	}
	
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
	ctrs[0] = old_ssc_to_new_ctr(old.d7);
	ctrs[1] = old_ssc_to_new_ctr(old.d8);
	ctrs[2] = old_ssc_to_new_ctr(old.d9);
	for(int q = 0; q < 3; ++q)
		for(int p = 0; p < q; ++p) //prune duplicates
			if(ctrs[p]==ctrs[q])
				ctrs[q] = crNONE;
	if(ctrs[1] == crMONEY)
		ctrs[1] = crNONE;
	if(ctrs[2] == crMONEY)
		ctrs[2] = crNONE;
	SETFLAG(flags,SUBSCR_COUNTER_SHOW0,old.d6&0b01);
	SETFLAG(flags,SUBSCR_COUNTER_ONLYSEL,old.d6&0b10);
	mindigits = old.d4;
	maxdigits = 0;
	infitm = old.d10;
	infchar = old.d5;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	c_text2 = c_text;
	c_shadow2 = c_shadow;
	c_bg2 = c_bg;
	return true;
}
int16_t SW_Counter::getX() const
{
	auto tx = x+shadow_x(shadtype);
	switch(align)
	{
		case sstaCENTER:
			return tx-getW()/2;
		case sstaRIGHT:
			return tx-getW();
	}
	return tx;
}
int16_t SW_Counter::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Counter::getW() const
{
	return text_length(get_zc_font(fontid), "0")*(maxdigits>0?maxdigits:zc_max(1,mindigits)) + shadow_w(shadtype);
}
word SW_Counter::getH() const
{
	return text_height(get_zc_font(fontid)) + shadow_h(shadtype);
}
byte SW_Counter::getType() const
{
	return widgCOUNTER;
}
void SW_Counter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	auto b = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	{
		int32_t value=0;
		bool infinite = false, draw = true;
		
		if(zq_view_allinf && (can_inf(ctrs[0],infitm)||can_inf(ctrs[1])||can_inf(ctrs[2])))
			infinite = true;
		else if(game != NULL && infitm > -1 && game->get_item(infitm) && !item_disabled(infitm))
			infinite = true;
		
		char valstring[80];
		char formatstring[80];
		sprintf(valstring,"01234567890123456789");
		sprintf(formatstring, "%%0%dd", mindigits);
		
		if(flags&SUBSCR_COUNTER_ONLYSEL)
		{
			draw = false;
			for(int q = 0; q < 3; ++q)
			{
				if((Bwpn>-1&&is_counter_item(Bwpn&0xFF,ctrs[q]))
					|| (Awpn>-1&&is_counter_item(Awpn&0xFF,ctrs[q]))
					|| (Xwpn>-1&&is_counter_item(Xwpn&0xFF,ctrs[q]))
					|| (Ywpn>-1&&is_counter_item(Ywpn&0xFF,ctrs[q])))
				{
					draw = true;
					break;
				}
			}
		}
		
		if(draw)
		{
			int maxty = 1;
			if (( FFCore.getQuestHeaderInfo(vZelda) == 0x250 && FFCore.getQuestHeaderInfo(vBuild) >= 33 )
					|| ( FFCore.getQuestHeaderInfo(vZelda) > 0x250  ) )
				maxty = 3;
			
			bool is_full = true;
			for(int q = 0; q < maxty; ++q)
			{
				int ty = ctrs[q];
				for(int p = 0; p < q; ++p) //prune duplicates
					if(ctrs[p]==ty)
						ty = crNONE;
				if (ty == crNONE)
					continue;
				if(q>0 && get_qr(qr_OLD_SUBSCR))
					switch(ctrs[q])
					{
						case crMONEY:
						case crLIFE:
							continue;
					}
				add_ssc_ctr(ctrs[q],infinite,value);
				if(!is_full_ssc_ctr(ctrs[q]))
					is_full = false;
			}
			
			if(zq_view_noinf)
				infinite = false;
			
			if(!(flags&SUBSCR_COUNTER_SHOW0)&&!value&&!infinite)
			{
				zq_ignore_item_ownership = b;
				return;
			}
			
			if(infinite)
				sprintf(valstring, "%c", infchar);
			else
			{
				if(maxdigits)
				{
					auto mval = pow(10,maxdigits);
					if(value >= mval)
						value = mval-1;
				}
				sprintf(valstring, formatstring, value);
			}
			int col_text = c_text.get_color(), col_shadow = c_shadow.get_color(), col_bg = c_bg.get_color();
			if(is_full)
			{
				col_text = c_text2.get_color();
				col_shadow = c_shadow2.get_color();
				col_bg = c_bg2.get_color();
			}
			
			textout_styled_aligned_ex(dest, tempfont, valstring, x+xofs, y+yofs, shadtype, align, col_text, col_shadow, col_bg);
		}
	}
	
	zq_ignore_item_ownership = b;
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	fontid = other->fontid;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	c_text2 = other->c_text2;
	c_shadow2 = other->c_shadow2;
	c_bg2 = other->c_bg2;
	mindigits = other->mindigits;
	maxdigits = other->maxdigits;
	infchar = other->infchar;
	if(all)
	{
		ctrs[0] = other->ctrs[0];
		ctrs[1] = other->ctrs[1];
		ctrs[2] = other->ctrs[2];
		infitm = other->infitm;
	}
	for(int q = 0; q < 3; ++q)
		for(int p = 0; p < q; ++p) //prune duplicates
			if(ctrs[p]==ctrs[q])
				ctrs[q] = crNONE;
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
	if(!p_getc(&mindigits,f))
		return qe_invalid;
	if(!p_getc(&maxdigits,f))
		return qe_invalid;
	if(!p_igetl(&infitm,f))
		return qe_invalid;
	if(!p_getc(&infchar,f))
		return qe_invalid;
	if(s_version >= 12)
	{
		if(auto ret = c_text2.read(f,s_version))
			return ret;
		if(auto ret = c_shadow2.read(f,s_version))
			return ret;
		if(auto ret = c_bg2.read(f,s_version))
			return ret;
	}
	else
	{
		c_text2 = c_text;
		c_shadow2 = c_shadow;
		c_bg2 = c_bg;
	}
	return 0;
}
int32_t SW_Counter::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(1);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	if(!p_iputl(ctrs[0],f))
		new_return(1);
	if(!p_iputl(ctrs[1],f))
		new_return(1);
	if(!p_iputl(ctrs[2],f))
		new_return(1);
	if(!p_putc(mindigits,f))
		new_return(1);
	if(!p_putc(maxdigits,f))
		new_return(1);
	if(!p_iputl(infitm,f))
		new_return(1);
	if(!p_putc(infchar,f))
		new_return(1);
	if(auto ret = c_text2.write(f))
		return ret;
	if(auto ret = c_shadow2.write(f))
		return ret;
	if(auto ret = c_bg2.write(f))
		return ret;
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
		new_return(1);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	if(!p_putc(digits,f))
		new_return(1);
	if(!p_iputl(infitm,f))
		new_return(1);
	if(!p_putc(infchar,f))
		new_return(1);
	return 0;
}

int16_t SW_BtnCounter::getX() const
{
	auto tx = x+shadow_x(shadtype);
	switch(align)
	{
		case sstaCENTER:
			return tx-getW()/2;
		case sstaRIGHT:
			return tx-getW();
	}
	return tx;
}
int16_t SW_BtnCounter::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_BtnCounter::getW() const
{
	return text_length(get_zc_font(fontid), "0")*(maxdigits>0?maxdigits:zc_max(1,mindigits)) + shadow_w(shadtype);
}
word SW_BtnCounter::getH() const
{
	return text_height(get_zc_font(fontid)) + shadow_h(shadtype);
}
byte SW_BtnCounter::getType() const
{
	return widgBTNCOUNTER;
}
void SW_BtnCounter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	int32_t counter;
	int ids[] = {Awpn,Bwpn,Xwpn,Ywpn};
	if(ids[btn] > -1)
	{
		itemdata const& itm = itemsbuf[ids[btn]&0xFF];
		int costs[2];
		for(int q = 0; q < 2; ++q)
		{
			if(itm.cost_amount[q])
				costs[q] = itm.cost_counter[q];
			else costs[q] = crNONE;
		}
		if(!(flags&SUBSCR_BTNCOUNTER_NOCOLLAPSE))
			if(costs[0] == crNONE && costs[1] != crNONE)
				zc_swap(costs[0],costs[1]);
		if(costs[costind] != crNONE)
			counter = costs[costind];
		else return;
	}
	else return;
	
	FONT* tempfont = get_zc_font(fontid);
	auto b = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	{
		int32_t value=0;
		bool infinite = false;
		
		if(zq_view_allinf && (can_inf(counter)))
			infinite = true;
		
		char valstring[80];
		char formatstring[80];
		sprintf(valstring,"01234567890123456789");
		sprintf(formatstring, "%%0%dd", mindigits);
		
		add_ssc_ctr(counter,infinite,value);
		bool is_full = is_full_ssc_ctr(counter);
		
		if(zq_view_noinf)
			infinite = false;
		
		if(!(flags&SUBSCR_BTNCOUNTER_SHOW0)&&!value&&!infinite)
		{
			zq_ignore_item_ownership = b;
			return;
		}
		
		if(infinite)
			sprintf(valstring, "%c", infchar);
		else
		{
			if(maxdigits)
			{
				auto mval = pow(10,maxdigits);
				if(value >= mval)
					value = mval-1;
			}
			sprintf(valstring, formatstring, value);
		}
		int col_text = c_text.get_color(), col_shadow = c_shadow.get_color(), col_bg = c_bg.get_color();
		if(is_full)
		{
			col_text = c_text2.get_color();
			col_shadow = c_shadow2.get_color();
			col_bg = c_bg2.get_color();
		}
		
		textout_styled_aligned_ex(dest, tempfont, valstring, x+xofs, y+yofs, shadtype, align, col_text, col_shadow, col_bg);
	}
	
	zq_ignore_item_ownership = b;
}
SubscrWidget* SW_BtnCounter::clone() const
{
	return new SW_BtnCounter(*this);
}
bool SW_BtnCounter::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_BtnCounter const* other = dynamic_cast<SW_BtnCounter const*>(src);
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	fontid = other->fontid;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	c_text2 = other->c_text2;
	c_shadow2 = other->c_shadow2;
	c_bg2 = other->c_bg2;
	mindigits = other->mindigits;
	maxdigits = other->maxdigits;
	infchar = other->infchar;
	if(all)
	{
		btn = other->btn;
		costind = other->costind;
	}
	return true;
}
int32_t SW_BtnCounter::read(PACKFILE *f, word s_version)
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
	if(!p_getc(&mindigits,f))
		return qe_invalid;
	if(!p_getc(&maxdigits,f))
		return qe_invalid;
	if(!p_getc(&infchar,f))
		return qe_invalid;
	if(!p_getc(&btn,f))
		return qe_invalid;
	if(!p_getc(&costind,f))
		return qe_invalid;
	if(s_version >= 12)
	{
		if(auto ret = c_text2.read(f,s_version))
			return ret;
		if(auto ret = c_shadow2.read(f,s_version))
			return ret;
		if(auto ret = c_bg2.read(f,s_version))
			return ret;
	}
	else
	{
		c_text2 = c_text;
		c_shadow2 = c_shadow;
		c_bg2 = c_bg;
	}
	return 0;
}
int32_t SW_BtnCounter::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(1);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	if(!p_putc(mindigits,f))
		new_return(1);
	if(!p_putc(maxdigits,f))
		new_return(1);
	if(!p_putc(infchar,f))
		new_return(1);
	if(!p_putc(btn,f))
		new_return(1);
	if(!p_putc(costind,f))
		new_return(1);
	if(auto ret = c_text2.write(f))
		return ret;
	if(auto ret = c_shadow2.write(f))
		return ret;
	if(auto ret = c_bg2.write(f))
		return ret;
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
int16_t SW_MMapTitle::getX() const
{
	if (!get_qr(qr_OLD_DMAP_INTRO_STRINGS))
		return x;
	switch(align)
	{
		case sstaCENTER:
			return x-getW()/2;
		case sstaRIGHT:
			return x-getW();
	}
	return x;
}
word SW_MMapTitle::getW() const
{
	if (!get_qr(qr_OLD_DMAP_INTRO_STRINGS))
		return w;
	word ret = (flags&SUBSCR_MMAPTIT_ONELINE)?100:50;
	char bufs[2][21] = {0};
	auto linecnt = get_strs(bufs[0],bufs[1]);
	if(linecnt)
	{
		FONT* tfont = get_zc_font(fontid);
		word len1 = text_length(tfont, bufs[0]);
		word len2 = text_length(tfont, bufs[1]);
		if(len1 > ret)
			ret = len1;
		if(len2 > ret)
			ret = len2;
	}
	return ret;
}
word SW_MMapTitle::getH() const
{
	if (!get_qr(qr_OLD_DMAP_INTRO_STRINGS))
		return h;
	return ((flags&SUBSCR_MMAPTIT_ONELINE)?1:2)*text_height(get_zc_font(fontid));
}
byte SW_MMapTitle::getType() const
{
	return widgMMAPTITLE;
}
byte SW_MMapTitle::get_strs(char* line1, char* line2) const
{
	line1[0] = line2[0] = 0;
	char dmaptitlesource[2][11];
	char dmaptitle[2][11];
	std::string legacy_title = DMaps[get_sub_dmap()].title;
	legacy_title.resize(21, ' ');
	const char* title = legacy_title.c_str();
	sprintf(dmaptitlesource[0], "%.10s", title);
	sprintf(dmaptitlesource[1], "%.10s", title+10);
	bool l1 = stripspaces(dmaptitlesource[0], dmaptitle[0], 10) > 0;
	bool l2 = stripspaces(dmaptitlesource[1], dmaptitle[1], 10) > 0;
	int linecnt = (l1 ? 1 : 0) + (l2 ? 1 : 0);
	switch(linecnt)
	{
		default:
		case 0:
			return 0;
		case 1:
			sprintf(line1,"%s",dmaptitle[l1?0:1]);
			return 1;
		case 2:
			if(flags&SUBSCR_MMAPTIT_ONELINE)
			{
				char spacebuf[2] = {0};
				if(dmaptitlesource[0][9]==' '||dmaptitlesource[1][0]==' ')
					spacebuf[0] = ' ';
				sprintf(line1,"%s%s%s",dmaptitle[0],spacebuf,dmaptitle[1]);
				return 1;
			}
			else
			{
				sprintf(line1,"%s",dmaptitle[0]);
				sprintf(line2,"%s",dmaptitle[1]);
				return 2;
			}
	}
}

void SW_MMapTitle::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if((flags&SUBSCR_MMAPTIT_REQMAP) && !has_item(itype_map, -1))
		return;
	if (get_qr(qr_OLD_DMAP_INTRO_STRINGS))
		draw_old(dest, xofs, yofs, page);
	else
		draw_new(dest, xofs, yofs, page);
}
void SW_MMapTitle::draw_new(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	draw_textbox(dest, x + xofs, y + yofs, w, h, tempfont, DMaps[get_sub_dmap()].title.c_str(),
		flags & SUBSCR_MMAPTIT_WORDWRAP, tabsize, align, shadtype,
		c_text.get_color(), c_shadow.get_color(), c_bg.get_color());
}
void SW_MMapTitle::draw_old(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	auto y1 = y+yofs, y2 = y1+8;
	if(flags&SUBSCR_MMAPTIT_ONELINE)
		y2 = y1;
	char bufs[2][21] = {0};
	auto linecnt = get_strs(bufs[0],bufs[1]);
	if(linecnt == 1)
	{
		textprintf_styled_aligned_ex(dest,tempfont,x+xofs,y2,shadtype,
			align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color(),
			"%s",bufs[0]);
	}
	else if(linecnt == 2)
	{
		textprintf_styled_aligned_ex(dest,tempfont,x+xofs,y2,shadtype,
			align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color(),
			"%s",bufs[1]);
		textprintf_styled_aligned_ex(dest,tempfont,x+xofs,y1,shadtype,
			align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color(),
			"%s",bufs[0]);
	}
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	fontid = other->fontid;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	tabsize = other->tabsize;
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
	if (s_version >= 10)
	{
		if(!p_getc(&tabsize,f))
			return qe_invalid;
	}
	return 0;
}
int32_t SW_MMapTitle::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(1);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	if(!p_putc(tabsize,f))
		new_return(1);
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
	auto const& thedmap = DMaps[get_sub_dmap()];
	bool showplr = (flags&SUBSCR_MMAP_SHOWPLR) && !(TheMaps[(thedmap.map*MAPSCRS)+get_homescr()].flags7&fNOHEROMARK);
	bool showcmp = (flags&SUBSCR_MMAP_SHOWCMP) && !(thedmap.flags&dmfNOCOMPASS);
	zcolors const& c = QMisc.colors;
	int32_t type = (thedmap.type&dmfTYPE);
	
	auto tx = x+xofs, ty = y+yofs;
	if(flags&SUBSCR_MMAP_SHOWMAP)
	{
		switch(type)
		{
		case dmOVERW:
		case dmBSOVERW:
		{
			int32_t maptile=(!get_qr(qr_BROKEN_OVERWORLD_MINIMAP) && has_item(itype_map, -1))?thedmap.minimap_tile[1]:thedmap.minimap_tile[0];
			int32_t mapcset=(!get_qr(qr_BROKEN_OVERWORLD_MINIMAP) && has_item(itype_map, -1))?thedmap.minimap_cset[1]:thedmap.minimap_cset[0];
			//What a mess. The map drawing is based on a variable that can change states during a scrolling transition when warping. -Z
			if(maptile)
			{
				draw_block(dest,tx,ty,maptile,mapcset,5,3);
			}
			else if(c.overworld_map_tile || c.overworld_map_tile)
			{
				draw_block(dest,tx,ty,(c.overworld_map_tile!=0?c.overworld_map_tile:c.overworld_map_tile),c.overworld_map_cset,5,3);
			}
			else
			{
				rectfill(dest,tx+8,ty+8,tx+71,ty+39,c.overw_bg);
			}
			
			if(!thedmap.minimap_tile[0] && ((thedmap.type&dmfTYPE) == dmBSOVERW))
			{
				drawgrid(dest,tx+8,ty+8,c.bs_goal,c.bs_dk);
			}
			
			break;
		}
		case dmDNGN:
		case dmCAVE:
		{
			int32_t maptile=has_item(itype_map, -1)?thedmap.minimap_tile[1]:thedmap.minimap_tile[0];
			int32_t mapcset=has_item(itype_map, -1)?thedmap.minimap_cset[1]:thedmap.minimap_cset[0];
			//What a mess. The map drawing is based on a variable that can change states during a scrolling transition when warping. -Z
			if(maptile)
			{
				draw_block(dest,tx,ty,maptile,mapcset,5,3);
			}
			else if(c.dungeon_map_tile||c.dungeon_map_tile)
			{
				draw_block(dest,tx,ty,(c.dungeon_map_tile!=0?c.dungeon_map_tile:c.dungeon_map_tile),c.dungeon_map_cset,5,3);
			}
			else
			{
				rectfill(dest,tx+8,ty+8,tx+71,ty+39,c.dngn_bg);
			}
			//Marking this as a possible area for the scrolling warp map bug reported by Lut. -Z
			if(!thedmap.minimap_tile[1] && has_item(itype_map, -1))
			{
				if((thedmap.flags&dmfMINIMAPCOLORFIX) != 0)
				{
					drawgrid(dest,tx+8,ty+8,c.cave_fg,-1);
				}
				else
				{
					drawgrid(dest,tx+8,ty+8,c.dngn_fg,-1);
				}
			}
			
			break;
		}
		}
	}
	
	if(showcmp)
	{
		if(type==dmDNGN || type==dmCAVE)
		{
			if(show_subscreen_dmap_dots&&has_item(itype_compass, -1))
			{
				int32_t c2 = c_cmp_off.get_color();
				
				if(frame&16)
				{
					if((game->lvlitems[get_dlevel()] & compass_litems) != compass_litems) // if you don't have all of them, keep blinking
						c2 = c_cmp_blink.get_color();
				}
					
				int32_t cx = ((thedmap.compass&15)<<3)+tx+10;
				int32_t cy = ((thedmap.compass&0xF0)>>2)+ty+8;
				putdot(dest,cx,cy,c2);
			}
		}
	}

#ifdef IS_PLAYER
	if (get_currscr() == 0x81 && Hero.specialcave == PASSAGEWAY)
		showplr = false;
#endif
	
	if(showplr)
	{
		if(show_subscreen_dmap_dots && c_plr.get_color() != 255)
		{
			int screen = get_homescr();
#ifdef IS_PLAYER
			if (Hero.current_screen < 0x80)
				screen = Hero.current_screen;
#endif

			if(type==dmOVERW)
			{
				putdot(dest,((screen&15)<<2)+tx+9,((screen&0xF0)>>2)+ty+8,c_plr.get_color());
			}
			else if(type==dmBSOVERW || ((type==dmDNGN || type==dmCAVE) && screen<128))
			{
				putdot(dest,(((screen&15)-thedmap.xoff)<<3)+tx+10,((screen&0xF0)>>2)+ty+8,c_plr.get_color());
			}
		}
	}
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	compass_litems = other->compass_litems;
	c_plr = other->c_plr;
	c_cmp_blink = other->c_cmp_blink;
	c_cmp_off = other->c_cmp_off;
	return true;
}
int32_t SW_MMap::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(s_version >= 13)
	{
		if (s_version >= 15)
		{
			if(!p_igetw(&compass_litems,f))
				return qe_invalid;
		}
		else
		{
			byte b;
			if(!p_getc(&b,f))
				return qe_invalid;
			compass_litems = word(b);
		}
	}
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
	if(!p_iputw(compass_litems,f))
		new_return(1);
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
	putBmap(dest, x+xofs, y+yofs, flags&SUBSCR_LMAP_SHOWMAP,
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
	return 256;
}
word SW_Clear::getH() const
{
	int hei = 0;
	auto sub = getParentSub();
	switch(sub ? sub->sub_type : sstPASSIVE)
	{
		case sstACTIVE:
			hei = 168;
			break;
		case sstPASSIVE:
			hei = 56;
			break;
		case sstOVERLAY:
			hei = 224 + (get_qr(qr_HIDE_BOTTOM_8_PIXELS) ? 0 : 8);
			break;
	}
	return hei;
}
byte SW_Clear::getType() const
{
	return widgBGCOLOR;
}
void SW_Clear::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(replay_version_check(0,19))
		clear_to_color(dest,c_bg.get_color());
	else rectfill(dest, xofs, yofs, xofs+getW()-1, yofs+getH()-1,c_bg.get_color());
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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

SW_ItemSlot::SW_ItemSlot(subscreen_object const& old) : SW_ItemSlot()
{
	load_old(old);
}
bool SW_ItemSlot::load_old(subscreen_object const& old)
{
	if(old.type != ssoCURRENTITEM)
		return false;
	SubscrWidget::load_old(old);
	iclass = old.d1;
	iid = wrap_iid(old.d8-1);
	pos = old.d3;
	pos_up = old.d4;
	pos_down = old.d5;
	pos_left = old.d6;
	pos_right = old.d7;
	SETFLAG(genflags,SUBSCRFLAG_SELECTABLE,pos>=0);
	SETFLAG(flags,SUBSCR_CURITM_INVIS,!(old.d2&0x1));
	SETFLAG(flags,SUBSCR_CURITM_NONEQP,old.d2&0x2);
	return true;
}
word SW_ItemSlot::getW() const
{
	return 16;
}
word SW_ItemSlot::getH() const
{
	return 16;
}
byte SW_ItemSlot::getType() const
{
	return widgITEMSLOT;
}
static bool check_bomb(optional<int> iid = nullopt)
{
#ifdef IS_PLAYER
	if(get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN))
		return true;
	if(current_item_power(itype_bombbag))
		return true;
	if(get_qr(qr_BROKEN_BOMB_AMMO_COSTS) ? game->get_bombs() : (iid ? checkmagiccost(*iid) : current_item_id(itype_bomb,true) > -1))
		return true;
	auto bombid = iid ? *iid : get_subscr_item_id(itype_bomb, true);
	if(bombid>-1 && itemsbuf[bombid].misc1==0 && Lwpns.idCount(wLitBomb)>0)
		return true; // Remote Bombs - still usable without cost
	return false;
#else
	return true;
#endif
}
static bool check_sbomb(optional<int> iid = nullopt)
{
#ifdef IS_PLAYER
	if(get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN))
		return true;
	auto bombbagid = get_subscr_item_id(itype_bombbag, true);
	if(bombbagid > -1 && itemsbuf[bombbagid].power && (itemsbuf[bombbagid].flags & item_flag1))
		return true;
	if(get_qr(qr_BROKEN_BOMB_AMMO_COSTS) ? game->get_sbombs() : (iid ? checkmagiccost(*iid) : current_item_id(itype_sbomb,true) > -1))
		return true;
	auto sbombid = iid ? *iid : get_subscr_item_id(itype_sbomb);
	if(sbombid >- 1 && itemsbuf[sbombid].misc1==0 && Lwpns.idCount(wLitSBomb) > 0)
		return true; // Remote Bombs - still usable without cost
	return false;
#else
	return true;
#endif
}
int32_t SW_ItemSlot::getItemVal() const
{
#ifdef IS_PLAYER
	if(iid > -1)
	{
		bool select = false;
		switch(itemsbuf[iid].type)
		{
			case itype_bomb:
				if(check_bomb(iid))
					select=true;
				break;
			case itype_bowandarrow:
			case itype_arrow:
				if(get_subscr_item_id(itype_bow, true) > -1)
					select=true;
				break;
			case itype_letterpotion:
				break;
			case itype_sbomb:
				if(check_sbomb(iid))
					select = true;
				break;
			case itype_sword:
				if(get_qr(qr_SELECTAWPN))
					select=true;
				break;
			default:
				select = true;
				break;
		}
		if (select && !item_disabled(iid) && game->get_item(iid))
		{
			int32_t ret = iid;
			if(ret>-1 && itemsbuf[ret].type == itype_arrow)
				ret += 0xF000; //bow
			return ret;
		}
		else return -1;
	}
	int32_t family = -1;
	switch(iclass)
	{
		case itype_bomb:
			if(check_bomb())
				family = itype_bomb;
			break;
		case itype_bowandarrow:
		case itype_arrow:
			if(get_subscr_item_id(itype_bow, true)>-1
				&& get_subscr_item_id(itype_arrow, true)>-1)
				family=itype_arrow;
			break;
		case itype_letterpotion:
			if(get_subscr_item_id(itype_potion, true)>-1)
				family=itype_potion;
			else if(get_subscr_item_id(itype_letter, true)>-1)
				family=itype_letter;
			break;
		case itype_sbomb:
			if(check_sbomb())
				family = itype_sbomb;
			break;
		case itype_sword:
			if(get_qr(qr_SELECTAWPN))
				family=itype_sword;
			break;
		default:
			family = iclass;
			break;
	}
	if(family < 0)
		return -1;
	int32_t itemid = get_subscr_item_id(family);
	if(item_disabled(itemid))
		return -1;
	if(wrap_iid(itemid) < 0)
		return -1;
	if(iclass == itype_bowandarrow)
		return itemid|0xF000;
	return itemid;
#else
	if(iid > -1) return iid;
	int fam = iclass;
	switch(fam)
	{
		case itype_letterpotion:
			if(get_subscr_item_id(itype_potion, true)==-1)
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
	int itemid = get_subscr_item_id(fam);
	if(itemid == -1) return -1;
	if(fam == itype_bowandarrow)
		itemid |= 0xF000;
	return itemid;
#endif
}
int32_t SW_ItemSlot::getDisplayItem() const
{
	bool nosp = flags&SUBSCR_CURITM_IGNR_SP_DISPLAY;
#ifdef IS_PLAYER
	if(iid > -1)
	{
		bool select = false;
		switch(itemsbuf[iid].type)
		{
			case itype_bomb:
				if(check_bomb(iid))
					select = true;
				break;
			case itype_bowandarrow:
			case itype_arrow:
				select=true;
				break;
			case itype_letterpotion:
				break;
			case itype_sbomb:
				if(check_sbomb(iid))
					select=true;
				break;
			
			//Super Special Cases for display
			case itype_map:
				if(nosp) break;
				return has_item(itype_map, -1) ? iid : -1;
			case itype_compass:
				if(nosp) break;
				return has_item(itype_compass, -1) ? iid : -1;
			case itype_bosskey:
				if(nosp) break;
				return has_item(itype_bosskey, -1) ? iid : -1;
			case itype_heartpiece:
				if(nosp) break;
				if(QMisc.colors.HCpieces_tile)
					return iid;
				break;
			default:
				select = true;
				break;
		}
		
		if (select && !item_disabled(iid) && game->get_item(iid))
		{
			auto ret = iid;
			if(ret>-1 && itemsbuf[ret].type == itype_arrow)
				ret += 0xF000; //bow
			return ret;
		}
		else return -1;
	}
	int32_t family = -1;
	switch(iclass)
	{
		case itype_bomb:
			if(check_bomb())
				family=itype_bomb;
			break;
		case itype_bowandarrow:
		case itype_arrow:
			family=itype_arrow;
			break;
		case itype_letterpotion:
			if(get_subscr_item_id(itype_potion, true)>-1)
				family=itype_potion;
			else if(get_subscr_item_id(itype_letter, true)>-1)
				family=itype_letter;
			break;
		case itype_sbomb:
			if(check_sbomb())
				family=itype_sbomb;
			break;
		//Super Special Cases for display
		case itype_map:
			if(nosp) break;
			return has_item(itype_map, -1) ? iMap : -1;
		case itype_compass:
			if(nosp) break;
			return has_item(itype_compass, -1) ? iCompass : -1;
		case itype_bosskey:
			if(nosp) break;
			return has_item(itype_bosskey, -1) ? iBossKey : -1;
		case itype_heartpiece:
			if(nosp) break;
			if(QMisc.colors.HCpieces_tile)
				return iHCPiece;
			break;
		
		default:
			family = iclass;
			break;
	}
	if(family < 0)
		return -1;
	int32_t itemid = get_subscr_item_id(family);
	if(item_disabled(itemid))
		return -1;
	if(wrap_iid(itemid) < 0)
		return -1;
	if(iclass == itype_bowandarrow)
		return itemid|0xF000;
	return itemid;
#else
	if(iid > -1) return iid;
	int fam = iclass;
	switch(fam)
	{
		case itype_letterpotion:
			if(get_subscr_item_id(itype_potion, true)==-1)
				fam = itype_letter;
			else fam = itype_potion;
			break;
		case itype_bowandarrow:
			fam = itype_arrow;
			break;
		case itype_map:
			if(nosp) break;
			return iMap;
		case itype_compass:
			if(nosp) break;
			return iCompass;
		case itype_bosskey:
			if(nosp) break;
			return iBossKey;
		case itype_heartpiece:
			if(nosp) break;
			return iHCPiece;
	}
	int itemid = get_subscr_item_id(fam);
	if(itemid == -1) return -1;
	if(fam == itype_bowandarrow)
		itemid |= 0xF000;
	return itemid;
#endif
}
void SW_ItemSlot::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(subscr_itemless && iid < 0) return;
	#ifdef IS_PLAYER
	if(flags&SUBSCR_CURITM_INVIS)
		return;
	#else
	if((flags&SUBSCR_CURITM_INVIS) && !(zinit.ss_flags&ssflagSHOWINVIS))
		return;
	#endif
	int id = getDisplayItem();
	if(id > -1)
	{
		bool nosp = iid > -1 || (flags&SUBSCR_CURITM_IGNR_SP_DISPLAY);
		if(!nosp && QMisc.colors.HCpieces_tile && id == iHCPiece)
		{
			int hcpphc =  game->get_hcp_per_hc();
			int numhpc = vbound(game->get_HCpieces(),0,hcpphc > 0 ? hcpphc-1 : 0);
			int t = QMisc.colors.HCpieces_tile + numhpc;
			overtile16(dest,t,x+xofs,y+yofs,QMisc.colors.HCpieces_cset,0);
		}
		else
		{
			auto clk = subscr_item_clk;
			auto itemid = id&0xFF;
			if(iid > -1 && replay_version_check(0,19))
			{
				if(subscr_override_clkoffsets[itemid] < 0)
					subscr_override_clkoffsets[itemid] = subscr_item_clk;
				clk -= subscr_override_clkoffsets[itemid];
			}
			putitem3(dest,x+xofs,y+yofs,itemid,clk);
			if(!nosp && (id&0xF000))
			{
				int id2 = get_subscr_item_id(itype_bow);
				if(id2 > -1)
					putitem3(dest,x+xofs,y+yofs,id2,clk);
			}
		}
	}
}
SubscrWidget* SW_ItemSlot::clone() const
{
	return new SW_ItemSlot(*this);
}
bool SW_ItemSlot::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_ItemSlot const* other = dynamic_cast<SW_ItemSlot const*>(src);
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	iid = wrap_iid(other->iid);
	iclass = other->iclass;
	return true;
}
int32_t SW_ItemSlot::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	if(!p_igetl(&iclass,f))
		return qe_invalid;
	if(!p_igetl(&iid,f))
		return qe_invalid;
	iid = wrap_iid(iid);
	return 0;
}
int32_t SW_ItemSlot::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(iclass,f))
		new_return(1);
	if(!p_iputl(iid,f))
		new_return(1);
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
	puttriframe(dest, x+xofs,y+yofs, c_outline.get_color(), c_number.get_color(),
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
		new_return(1);
	if(!p_putc(frame_cset,f))
		new_return(1);
	if(!p_putc(piece_cset,f))
		new_return(1);
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
	flip = old.d2;
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
	puttriforce(dest,x+xofs,y+yofs,tile,cs.get_cset(),w,h,
		flip,flags&SUBSCR_MCGUF_OVERLAY,flags&SUBSCR_MCGUF_TRANSP,number);
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	tile = other->tile;
	number = other->number;
	flip = other->flip;
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
	if(!p_getc(&flip,f))
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
		new_return(1);
	if(!p_putc(flip,f))
		new_return(1);
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
	draw_block_flip(dest,x+xofs,y+yofs,tile,cs.get_cset(),
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
		new_return(1);
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
int32_t SW_MiniTile::get_int_tile() const
{
	if(tile == -1)
	{
		if(special_tile >= ssmstMAX)
			return 0;
		return -(special_tile+1);
	}
	else return tile;
}
void SW_MiniTile::set_int_tile(int32_t val)
{
	if(val < -ssmstMAX || val >= NEWMAXTILES) return;
	if(val < 0)
	{
		tile = -1;
		special_tile = -val-1;
	}
	else
	{
		tile = val;
		special_tile = -1;
	}
}
void SW_MiniTile::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	auto t = (get_tile()<<2)+crn;
	auto tx = x+xofs, ty = y+yofs;
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
		new_return(1);
	if(!p_putc(crn,f))
		new_return(1);
	if(!p_putc(flip,f))
		new_return(1);
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
	if(subscr_pg_animating && !(subscr_pg_transition.flags&SUBSCR_TRANS_NOHIDESELECTOR))
		return;
	SubscrWidget* widg = page.get_sel_widg();
	if(!widg)
	{
		if(page.cursor_pos)
			if(widg = page.get_widg_pos(0, false))
				page.cursor_pos = 0;
		if(!widg)
			return;
	}
	
	bool big_sel=flags&SUBSCR_SELECTOR_LARGE;
	int selector_type = (flags&SUBSCR_SELECTOR_USEB) ? 1 : 0;
	item tempsel(0,0,0,(flags&SUBSCR_SELECTOR_USEB)?iSelectB:iSelectA,0,0,true);
	tempsel.subscreenItem=true;
	tempsel.hide_hitbox = true;
	tempsel.xofs = tempsel.yofs = 0;
	tempsel.drawstyle = (flags&SUBSCR_SELECTOR_TRANSP) ? 1 : 0;
	int32_t id = widg->getItemVal();
	if(id > -1) //Mask out the bow&arrow flag
		id &= 0xFF;
	itemdata const& tmpitm = itemsbuf[id];
	bool oldsel = get_qr(qr_SUBSCR_OLD_SELECTOR);
	if(!oldsel) big_sel = false;
	int sw, sh, dw, dh;
	int sxofs, syofs, dxofs, dyofs;
	ZCSubscreen const* parentsub = getParentSub();
	bool new_animate = true;
	if(widg->genflags & SUBSCRFLAG_SELOVERRIDE)
	{
		auto const& selectile = widg->selector_override.tileinfo[selector_type];
		sw = selectile.sw;
		sh = selectile.sh;
		sxofs = syofs = 0;
		dw = widg->selector_override.w;
		dh = widg->selector_override.h;
		dxofs = widg->selector_override.x;
		dyofs = widg->selector_override.y;
		//
		big_sel = false;
		tempsel.tile = tempsel.o_tile = selectile.tile;
		tempsel.o_cset = selectile.cset;
		tempsel.cs = tempsel.o_cset&0xF;
		tempsel.frames = selectile.frames;
		tempsel.o_speed = selectile.speed;
		tempsel.o_delay = selectile.delay;
		tempsel.extend = 3;
		tempsel.txsz = ((sw%16)?1:0)+(sw/16);
		tempsel.tysz = ((sh%16)?1:0)+(sh/16);
	}
	else if(parentsub->sub_type == sstACTIVE
		&& (parentsub->flags & SUBFLAG_ACT_OVERRIDESEL))
	{
		auto const& selectile = parentsub->selector_setting.tileinfo[selector_type];
		sw = selectile.sw;
		sh = selectile.sh;
		sxofs = syofs = 0;
		if(widg->getType() == widgITEMSLOT && id > -1)
		{
			dw = ((tmpitm.overrideFLAGS & OVERRIDE_HIT_WIDTH) ? tmpitm.hxsz : 16);
			dh = ((tmpitm.overrideFLAGS & OVERRIDE_HIT_HEIGHT) ? tmpitm.hysz : 16);
			dxofs = widg->x+((tmpitm.overrideFLAGS & OVERRIDE_HIT_X_OFFSET) ? tmpitm.hxofs : 0) + (tempsel.extend > 2 ? (int)tempsel.xofs : 0);
			dyofs = widg->y+((tmpitm.overrideFLAGS & OVERRIDE_HIT_Y_OFFSET) ? tmpitm.hyofs : 0) + (tempsel.extend > 2 ? (int)tempsel.yofs : 0);
		}
		else
		{
			dw = widg->getW();
			dh = widg->getH();
			dxofs = widg->getX();
			dyofs = widg->getY();
		}
		dw += parentsub->selector_setting.w;
		dh += parentsub->selector_setting.h;
		dxofs += parentsub->selector_setting.x;
		dyofs += parentsub->selector_setting.y;
		//
		big_sel = false;
		tempsel.tile = tempsel.o_tile = selectile.tile;
		tempsel.o_cset = selectile.cset;
		tempsel.cs = tempsel.o_cset&0xF;
		tempsel.frames = selectile.frames;
		tempsel.o_speed = selectile.speed;
		tempsel.o_delay = selectile.delay;
		tempsel.extend = 3;
		tempsel.txsz = ((sw%16)?1:0)+(sw/16);
		tempsel.tysz = ((sh%16)?1:0)+(sh/16);
	}
	else new_animate = false;
	
	dummyitem_animate(&tempsel,subscr_item_clk);
	if(!tempsel.tile) return;
	
	if(!new_animate)
	{
		if(oldsel)
		{
			sw = (tempsel.extend > 2 ? tempsel.txsz*16 : 16);
			sh = (tempsel.extend > 2 ? tempsel.tysz*16 : 16);
			sxofs = 0;
			syofs = 0;
			dw = (tempsel.extend > 2 ? tempsel.txsz*16 : 16);
			dh = (tempsel.extend > 2 ? tempsel.tysz*16 : 16);
			dxofs = widg->x+(tempsel.extend > 2 ? (int)tempsel.xofs : 0);
			dyofs = widg->y+(tempsel.extend > 2 ? (int)tempsel.yofs : 0);
			if(replay_version_check(0,19) && tempsel.extend > 2)
				sh = dh = tempsel.txsz*16;
		}
		else
		{
			sw = (tempsel.extend > 2 ? tempsel.hit_width : 16);
			sh = (tempsel.extend > 2 ? tempsel.hit_height : 16);
			sxofs = (tempsel.extend > 2 ? tempsel.hxofs : 0);
			syofs = (tempsel.extend > 2 ? tempsel.hyofs : 0);
			if(widg->getType() == widgITEMSLOT && id > -1)
			{
				dw = ((tmpitm.overrideFLAGS & OVERRIDE_HIT_WIDTH) ? tmpitm.hxsz : 16);
				dh = ((tmpitm.overrideFLAGS & OVERRIDE_HIT_HEIGHT) ? tmpitm.hysz : 16);
				dxofs = widg->x+((tmpitm.overrideFLAGS & OVERRIDE_HIT_X_OFFSET) ? tmpitm.hxofs : 0) + (tempsel.extend > 2 ? (int)tempsel.xofs : 0);
				dyofs = widg->y+((tmpitm.overrideFLAGS & OVERRIDE_HIT_Y_OFFSET) ? tmpitm.hyofs : 0) + (tempsel.extend > 2 ? (int)tempsel.yofs : 0);
			}
			else
			{
				dw = widg->getW();
				dh = widg->getH();
				dxofs = widg->getX();
				dyofs = widg->getY();
			}
		}
	}
	BITMAP* tmpbmp = create_bitmap_ex(8,sw,sh);
	for(int32_t j=0; j<4; ++j)
	{
		clear_bitmap(tmpbmp);
		tempsel.x=0;
		tempsel.y=0;
		tempsel.draw(tmpbmp);

		int32_t tmpx = xofs+(big_sel?(j%2?8:-8):0);
		int32_t tmpy = yofs+(big_sel?(j>1?8:-8):0);
		masked_stretch_blit(tmpbmp, dest, vbound(sxofs, 0, sw), vbound(syofs, 0, sh), sw-vbound(sxofs, 0, sw), sh-vbound(syofs, 0, sh), tmpx+dxofs, tmpy+dyofs, dw, dh);
		
		if(!big_sel)
			break;
		tempsel.tile+=zc_max(itemsbuf[tempsel.id].frames,1);
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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

bool SW_GaugePiece::infinite() const
{
	return (inf_item > -1 && (!game || (game->get_item(inf_item) && !item_disabled(inf_item))));
}
int16_t SW_GaugePiece::getX() const
{
	return x + (grid_xoff < 0 ? gauge_hei*grid_xoff : 0);
}
int16_t SW_GaugePiece::getY() const
{
	auto sz = ((flags&SUBSCR_GAUGE_FULLTILE)?16:8);
	return y + (grid_yoff < 0 ? gauge_hei*grid_yoff : 0);
}
word SW_GaugePiece::getW() const
{
	auto sz = ((flags&SUBSCR_GAUGE_FULLTILE)?16:8);
	return (gauge_wid+1) * sz + gauge_wid * hspace + gauge_hei * abs(grid_xoff);
}
word SW_GaugePiece::getH() const
{
	auto sz = ((flags&SUBSCR_GAUGE_FULLTILE)?16:8);
	return (gauge_hei+1) * sz + gauge_hei * vspace + gauge_wid * abs(grid_yoff);
}

void SW_GaugePiece::draw_piece(BITMAP* dest, int dx, int dy, int container, int anim_offs) const
{
	dword ctr_cur = get_ctr(), ctr_max = get_ctr_max(),
		ctr_per_cont = get_per_container();
	int containers=ctr_max/ctr_per_cont;
	int fr = frames ? frames : 1;
	
	int ind = 3;
	if(container<containers)
		ind = 0;
	else if(container==containers)
		ind = 1;
	else if(container==containers+1)
		ind = 2;
	//else if (container>containers+1)
	
	int mtile = mts[ind].getTileCrn();
	int cset = mts[ind].cset;
	bool mod_value = (flags&(SUBSCR_GAUGE_MOD1<<ind));
	int tile = mtile>>2;
	
	bool fulltile = (flags&SUBSCR_GAUGE_FULLTILE);
	
	int ctr_ofs = 0;
	if(mod_value) //Change the tile based on ctr
	{
		bool full = false;
		if(!fulltile && get_qr(qr_OLD_GAUGE_TILE_LAYOUT))
		{
			int offs_0 = (fr + (4-(fr%4)));
			if(ctr_cur>=container*ctr_per_cont)
				full = true;
			else if(((container-1)*ctr_per_cont)>ctr_cur)
				ctr_ofs=0;
			else
				ctr_ofs=((ctr_cur-((container-1)*ctr_per_cont))%ctr_per_cont);
			
			ctr_ofs /= (unit_per_frame+1);
			if(full)
			{
				if((flags&SUBSCR_GAUGE_UNQLAST) && ctr_cur==container*ctr_per_cont)
					ctr_ofs = ctr_per_cont / (unit_per_frame+1) + 3 + offs_0;
			}
			else ctr_ofs += offs_0;
		}
		else
		{
			if(((container-1)*ctr_per_cont)>ctr_cur)
				ctr_ofs = 0;
			else if(full = ctr_cur>=container*ctr_per_cont)
				ctr_ofs = ctr_per_cont+unit_per_frame;
			else
				ctr_ofs = (ctr_cur-((container-1)*ctr_per_cont))%ctr_per_cont;
			ctr_ofs /= (unit_per_frame+1);
			if(full && (flags&SUBSCR_GAUGE_UNQLAST) && ctr_cur==container*ctr_per_cont)
				++ctr_ofs;
		}
	}
	
	int offs = (ctr_ofs*fr)+anim_offs;
	
	if(fulltile)
		overtile16(dest,tile+offs,dx,dy,cset,0);
	else overtile8(dest,mtile+offs,dx,dy,cset,0);
}
void SW_GaugePiece::draw(BITMAP* dest, int xofs, int yofs, SubscrPage& page) const
{
	auto b = zq_ignore_item_ownership;
	
	bool inf = infinite();
	if(flags & (inf ? SUBSCR_GAUGE_INFITM_BAN : SUBSCR_GAUGE_INFITM_REQ))
		return;
	
	int anim_offs = 0;
	bool animate = true;
	auto ctr = get_ctr();
	auto ctr_max = get_ctr_max();
	bool skipanim = frames > 1 && (flags&SUBSCR_GAUGE_ANIM_SKIP);
	if(flags&SUBSCR_GAUGE_ANIM_PERCENT)
	{
		if((flags&SUBSCR_GAUGE_ANIM_UNDER) && ((1000*ctr)/ctr_max) > (10*anim_val))
			animate = false;
		if((flags&SUBSCR_GAUGE_ANIM_OVER) && ((1000*ctr)/ctr_max) < (10*anim_val))
			animate = false;
	}
	else
	{
		if((flags&SUBSCR_GAUGE_ANIM_UNDER) && ctr > anim_val)
			animate = false;
		if((flags&SUBSCR_GAUGE_ANIM_OVER) && ctr < anim_val)
			animate = false;
	}
	if(animate)
	{
		int fr = frames ? frames : 1;
		int spd = speed ? speed : 1;
		if(skipanim) --fr;
		if(fr > 1)
		{
			int clkwid = spd*(fr+delay);
			auto t = (subscr_item_clk%clkwid)-(delay*spd);
			if(t > -1)
				anim_offs = t/spd;
		}
		if(skipanim) ++anim_offs;
	}

	if(!gauge_hei && !gauge_wid) //1x1
	{
		draw_piece(dest, x+xofs, y+yofs, container, anim_offs);
	}
	else
	{
		bool colbased = (flags&SUBSCR_GAUGE_GRID_COLUMN1ST) || !gauge_wid;
		bool rtol = (flags&SUBSCR_GAUGE_GRID_RTOL), ttob = (flags&SUBSCR_GAUGE_GRID_TTOB),
			snake = (flags&SUBSCR_GAUGE_GRID_SNAKE);
		auto sz = (flags&SUBSCR_GAUGE_FULLTILE)?16:8;
		bool snakeoffs = false;
		if(colbased) //columns then rows
		{
			for(int x2 = 0, offs = 0; x2 <= gauge_wid; ++x2)
			{
				if(snakeoffs)
					for(int y2 = gauge_hei; y2 >= 0; --y2, ++offs)
					{
						auto curx = (rtol ? gauge_wid-x2 : x2), cury = (ttob ? y2 : gauge_hei-y2);
						int xo = ((sz+hspace) * curx) + (grid_xoff * cury);
						int yo = ((sz+vspace) * cury) + (grid_yoff * curx);
						draw_piece(dest, x+xofs+xo, y+yofs+yo, container+offs, anim_offs);
					}
				else
					for(int y2 = 0; y2 <= gauge_hei; ++y2, ++offs)
					{
						auto curx = (rtol ? gauge_wid-x2 : x2), cury = (ttob ? y2 : gauge_hei-y2);
						int xo = ((sz+hspace) * curx) + (grid_xoff * cury);
						int yo = ((sz+vspace) * cury) + (grid_yoff * curx);
						draw_piece(dest, x+xofs+xo, y+yofs+yo, container+offs, anim_offs);
					}
				if(snake) snakeoffs = !snakeoffs;
			}
		}
		else
		{
			for(int y2 = 0, offs = 0; y2 <= gauge_hei; ++y2)
			{
				if(snakeoffs)
					for(int x2 = gauge_wid; x2 >= 0; --x2, ++offs)
					{
						auto curx = (rtol ? gauge_wid-x2 : x2), cury = (ttob ? y2 : gauge_hei-y2);
						int xo = ((sz+hspace) * curx) + (grid_xoff * cury);
						int yo = ((sz+vspace) * cury) + (grid_yoff * curx);
						draw_piece(dest, x+xofs+xo, y+yofs+yo, container+offs, anim_offs);
					}
				else
					for(int x2 = 0; x2 <= gauge_wid; ++x2, ++offs)
					{
						auto curx = (rtol ? gauge_wid-x2 : x2), cury = (ttob ? y2 : gauge_hei-y2);
						int xo = ((sz+hspace) * curx) + (grid_xoff * cury);
						int yo = ((sz+vspace) * cury) + (grid_yoff * curx);
						draw_piece(dest, x+xofs+xo, y+yofs+yo, container+offs, anim_offs);
					}
				if(snake) snakeoffs = !snakeoffs;
			}
		}
	}
}
bool SW_GaugePiece::copy_prop(SubscrWidget const* src, bool all)
{
	if(src == this)
		return false;
	switch(src->getType())
	{
		case widgLGAUGE:
		case widgMGAUGE:
		case widgMISCGAUGE:
		case widgITMCOOLDOWNGAUGE:
			break;
		default:
			return false;
	}
	SW_GaugePiece const* other = dynamic_cast<SW_GaugePiece const*>(src);
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	mts[0] = other->mts[0];
	mts[1] = other->mts[1];
	mts[2] = other->mts[2];
	mts[3] = other->mts[3];
	frames = other->frames;
	speed = other->speed;
	delay = other->delay;
	unit_per_frame = other->unit_per_frame;
	gauge_wid = other->gauge_wid;
	gauge_hei = other->gauge_hei;
	anim_val = other->anim_val;
	inf_item = other->inf_item;
	if(all)
	{
		container = other->container;
		hspace = other->hspace;
		vspace = other->vspace;
		grid_xoff = other->grid_xoff;
		grid_yoff = other->grid_yoff;
	}
	bool frcond = frames <= 1;
	bool acond = frcond || !(flags & (SUBSCR_GAUGE_ANIM_UNDER|SUBSCR_GAUGE_ANIM_OVER));
	bool frcond2 = frcond || (!acond && frames <= 2 && (flags & SUBSCR_GAUGE_ANIM_SKIP));
	bool infcond = !(flags & (SUBSCR_GAUGE_INFITM_REQ|SUBSCR_GAUGE_INFITM_BAN));
	if(frcond)
	{
		SETFLAG(flags, SUBSCR_GAUGE_ANIM_UNDER, false);
		SETFLAG(flags, SUBSCR_GAUGE_ANIM_OVER, false);
		SETFLAG(flags, SUBSCR_GAUGE_ANIM_PERCENT, false);
		SETFLAG(flags, SUBSCR_GAUGE_ANIM_SKIP, false);
	}
	if(acond)
	{
		anim_val = 0;
	}
	if(frcond2)
	{
		speed = 1;
		delay = 0;
	}
	if(infcond)
		inf_item = -1;
	return true;
}
int32_t SW_GaugePiece::read(PACKFILE* f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	for(auto q = 0; q < 4; ++q)
		if(auto ret = mts[q].read(f,s_version))
			return ret;
	if(!p_igetw(&frames, f))
		return qe_invalid;
	bool frcond = frames <= 1;
	bool acond = frcond || !(flags & (SUBSCR_GAUGE_ANIM_UNDER|SUBSCR_GAUGE_ANIM_OVER));
	bool frcond2 = frcond || (!acond && frames <= 2 && (flags & SUBSCR_GAUGE_ANIM_SKIP));
	bool infcond = !(flags & (SUBSCR_GAUGE_INFITM_REQ|SUBSCR_GAUGE_INFITM_BAN));
	if(!frcond2)
	{
		if(!p_igetw(&speed, f))
			return qe_invalid;
		if(!p_igetw(&delay, f))
			return qe_invalid;
	}
	if(!p_igetw(&container, f))
		return qe_invalid;
	if(!p_getc(&unit_per_frame, f))
		return qe_invalid;
	if(!p_getc(&gauge_wid, f))
		return qe_invalid;
	if(!p_getc(&gauge_hei, f))
		return qe_invalid;
	if(gauge_wid || gauge_hei)
	{
		if(!p_getc(&hspace, f))
			return qe_invalid;
		if(!p_getc(&vspace, f))
			return qe_invalid;
		if(!p_igetw(&grid_xoff, f))
			return qe_invalid;
		if(!p_igetw(&grid_yoff, f))
			return qe_invalid;
	}
	if(!acond)
	{
		if(!p_igetw(&anim_val, f))
			return qe_invalid;
	}
	if(!infcond)
	{
		if(!p_igetw(&inf_item, f))
			return qe_invalid;
	}
	return 0;
}
int32_t SW_GaugePiece::write(PACKFILE* f) const
{
	bool frcond = frames <= 1;
	bool acond = frcond || !(flags & (SUBSCR_GAUGE_ANIM_UNDER|SUBSCR_GAUGE_ANIM_OVER));
	bool frcond2 = frcond || (!acond && frames <= 2 && (flags & SUBSCR_GAUGE_ANIM_SKIP));
	bool infcond = !(flags & (SUBSCR_GAUGE_INFITM_REQ|SUBSCR_GAUGE_INFITM_BAN));
	if(auto ret = SubscrWidget::write(f))
		return ret;
	for(auto q = 0; q < 4; ++q)
		if(auto ret = mts[q].write(f))
			return ret;
	if(!p_iputw(frames, f))
		new_return(1);
	if(!frcond2)
	{
		if(!p_iputw(speed, f))
			new_return(1);
		if(!p_iputw(delay, f))
			new_return(1);
	}
	if(!p_iputw(container, f))
		new_return(1);
	if(!p_putc(unit_per_frame, f))
		new_return(1);
	if(!p_putc(gauge_wid, f))
		new_return(1);
	if(!p_putc(gauge_hei, f))
		new_return(1);
	if(gauge_wid || gauge_hei)
	{
		if(!p_putc(hspace, f))
			new_return(1);
		if(!p_putc(vspace, f))
			new_return(1);
		if(!p_iputw(grid_xoff, f))
			new_return(1);
		if(!p_iputw(grid_yoff, f))
			new_return(1);
	}
	if(!acond)
	{
		if(!p_iputw(anim_val, f))
			new_return(1);
	}
	if(!infcond)
	{
		if(!p_iputw(inf_item, f))
			new_return(1);
	}
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
	mts[0].setTileCrn(old.d2);
	mts[0].cset = old.colortype1;
	mts[1].setTileCrn(old.d3);
	mts[1].cset = old.color1;
	mts[2].setTileCrn(old.d4);
	mts[2].cset = old.colortype2;
	mts[3].setTileCrn(old.d5);
	mts[3].cset = old.color2;
	SETFLAG(flags, SUBSCR_GAUGE_MOD1, old.d10&0x01);
	SETFLAG(flags, SUBSCR_GAUGE_MOD2, old.d10&0x02);
	SETFLAG(flags, SUBSCR_GAUGE_MOD3, old.d10&0x04);
	SETFLAG(flags, SUBSCR_GAUGE_MOD4, old.d10&0x08);
	SETFLAG(flags, SUBSCR_GAUGE_UNQLAST, old.d10&0x10);
	frames = 1;
	speed = 0;
	delay = 0;
	container = old.d1;
	return true;
}
byte SW_LifeGaugePiece::getType() const
{
	return widgLGAUGE;
}

dword SW_LifeGaugePiece::get_ctr() const
{
	auto old_ign_ownership = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	auto ret = get_ssc_ctr(crLIFE);
	
	zq_ignore_item_ownership = old_ign_ownership;
	
	return ret;
}
dword SW_LifeGaugePiece::get_ctr_max() const
{
	auto old_ign_ownership = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	auto ret = get_ssc_ctrmax(crLIFE);
	
	zq_ignore_item_ownership = old_ign_ownership;
	
	return ret;
}
bool SW_LifeGaugePiece::infinite() const
{
	if(zq_view_allinf && can_inf(crLIFE,inf_item)) return true;
	if(zq_view_noinf) return false;
	
	return SW_GaugePiece::infinite();
}
dword SW_LifeGaugePiece::get_per_container() const
{
	return game ? game->get_hp_per_heart() : zinit.hp_per_heart;
}
void SW_LifeGaugePiece::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(replay_version_check(0,19))
	{
		zc_oldrand();zc_oldrand();zc_oldrand();
	}
	
	SW_GaugePiece::draw(dest, xofs, yofs, page);
}
SubscrWidget* SW_LifeGaugePiece::clone() const
{
	return new SW_LifeGaugePiece(*this);
}
bool SW_LifeGaugePiece::copy_prop(SubscrWidget const* src, bool all)
{
	return SW_GaugePiece::copy_prop(src,all);
}
int32_t SW_LifeGaugePiece::read(PACKFILE *f, word s_version)
{
	return SW_GaugePiece::read(f, s_version);
}
int32_t SW_LifeGaugePiece::write(PACKFILE *f) const
{
	return SW_GaugePiece::write(f);
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
	mts[0].setTileCrn(old.d2);
	mts[0].cset = old.colortype1;
	mts[1].setTileCrn(old.d3);
	mts[1].cset = old.color1;
	mts[2].setTileCrn(old.d4);
	mts[2].cset = old.colortype2;
	mts[3].setTileCrn(old.d5);
	mts[3].cset = old.color2;
	SETFLAG(flags, SUBSCR_GAUGE_MOD1, old.d10&0x01);
	SETFLAG(flags, SUBSCR_GAUGE_MOD2, old.d10&0x02);
	SETFLAG(flags, SUBSCR_GAUGE_MOD3, old.d10&0x04);
	SETFLAG(flags, SUBSCR_GAUGE_MOD4, old.d10&0x08);
	SETFLAG(flags, SUBSCR_GAUGE_UNQLAST, old.d10&0x10);
	frames = old.d6;
	speed = old.d7;
	delay = old.d8;
	container = old.d1;
	showdrain = old.d9;
	return true;
}
byte SW_MagicGaugePiece::getType() const
{
	return widgMGAUGE;
}

dword SW_MagicGaugePiece::get_ctr() const
{
	auto old_ign_ownership = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	auto ret = get_ssc_ctr(crMAGIC);
	
	zq_ignore_item_ownership = old_ign_ownership;
	
	return ret;
}
dword SW_MagicGaugePiece::get_ctr_max() const
{
	auto old_ign_ownership = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	auto ret = get_ssc_ctrmax(crMAGIC);
	
	zq_ignore_item_ownership = old_ign_ownership;
	
	return ret;
}
bool SW_MagicGaugePiece::infinite() const
{
	if(zq_view_allinf && can_inf(crMAGIC,inf_item)) return true;
	if(zq_view_noinf) return false;
	
	auto old_ign_ownership = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	bool ret = false;
	get_ssc_ctr(crMAGIC, &ret);
	ret = ret || SW_GaugePiece::infinite();
	
	zq_ignore_item_ownership = old_ign_ownership;
	
	return ret;
}
dword SW_MagicGaugePiece::get_per_container() const
{
	return game ? game->get_mp_per_block() : zinit.magic_per_block;
}
void SW_MagicGaugePiece::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(showdrain > -1 && showdrain != game->get_magicdrainrate())
		return;
	
	if(replay_version_check(0,19))
	{
		zc_oldrand();zc_oldrand();zc_oldrand();
	}
	
	SW_GaugePiece::draw(dest, xofs, yofs, page);
}
SubscrWidget* SW_MagicGaugePiece::clone() const
{
	return new SW_MagicGaugePiece(*this);
}
bool SW_MagicGaugePiece::copy_prop(SubscrWidget const* src, bool all)
{
	if(!SW_GaugePiece::copy_prop(src,all))
		return false;
	if(src->getType() != getType() || src == this)
		return false;
	SW_MagicGaugePiece const* other = dynamic_cast<SW_MagicGaugePiece const*>(src);
	showdrain = other->showdrain;
	return true;
}
int32_t SW_MagicGaugePiece::read(PACKFILE *f, word s_version)
{
	if(auto ret = SW_GaugePiece::read(f, s_version))
		return ret;
	if(!p_igetw(&showdrain, f))
		return qe_invalid;
	return 0;
}
int32_t SW_MagicGaugePiece::write(PACKFILE *f) const
{
	if(auto ret = SW_GaugePiece::write(f))
		return ret;
	if(!p_iputw(showdrain,f))
		new_return(1);
	return 0;
}

byte SW_MiscGaugePiece::getType() const
{
	return widgMISCGAUGE;
}

dword SW_MiscGaugePiece::get_ctr() const
{
	auto old_ign_ownership = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	auto ret = get_ssc_ctr(counter);
	
	zq_ignore_item_ownership = old_ign_ownership;
	
	return ret;
}
dword SW_MiscGaugePiece::get_ctr_max() const
{
	auto old_ign_ownership = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	auto ret = get_ssc_ctrmax(counter);
	
	zq_ignore_item_ownership = old_ign_ownership;
	
	return ret;
}
bool SW_MiscGaugePiece::infinite() const
{
	if(zq_view_allinf && can_inf(counter,inf_item)) return true;
	if(zq_view_noinf) return false;
	
	auto old_ign_ownership = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	bool ret = false;
	get_ssc_ctr(counter, &ret);
	ret = ret || SW_GaugePiece::infinite();
	
	zq_ignore_item_ownership = old_ign_ownership;
	
	return ret;
}
dword SW_MiscGaugePiece::get_per_container() const
{
	return per_container;
}
void SW_MiscGaugePiece::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	SW_GaugePiece::draw(dest, xofs, yofs, page);
}
SubscrWidget* SW_MiscGaugePiece::clone() const
{
	return new SW_MiscGaugePiece(*this);
}
bool SW_MiscGaugePiece::copy_prop(SubscrWidget const* src, bool all)
{
	if(!SW_GaugePiece::copy_prop(src,all))
		return false;
	if(src->getType() != getType() || src == this)
		return false;
	SW_MiscGaugePiece const* other = dynamic_cast<SW_MiscGaugePiece const*>(src);
	counter = other->counter;
	per_container = other->per_container;
	return true;
}
int32_t SW_MiscGaugePiece::read(PACKFILE *f, word s_version)
{
	if(auto ret = SW_GaugePiece::read(f, s_version))
		return ret;
	if(!p_igetw(&counter, f))
		return qe_invalid;
	if(!p_igetw(&per_container,f))
		return qe_invalid;
	return 0;
}
int32_t SW_MiscGaugePiece::write(PACKFILE *f) const
{
	if(auto ret = SW_GaugePiece::write(f))
		return ret;
	if(!p_iputw(counter, f))
		new_return(1);
	if(!p_iputw(per_container,f))
		new_return(1);
	return 0;
}

byte SW_ItemCooldownGauge::getType() const
{
	return widgITMCOOLDOWNGAUGE;
}

dword SW_ItemCooldownGauge::get_ctr() const
{
	int item_id = calc_item_from_class_id_button(specific_item_id, button_id, item_class);
	if (unsigned(item_id) >= MAXITEMS)
		return 0;
	
	auto b = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	auto cd_data = calc_item_cooldown(item_id);
	
	zq_ignore_item_ownership = b;
	
	zfix cooldown = cd_data.cooldown;
	zfix max_cooldown = cd_data.max_cooldown;
	zfix perc = cooldown < 0 ? 1.0_zf : (cooldown / max_cooldown);
	perc = vbound(perc, 0_zf, 1_zf);
	
	return (perc * zfix(total_points)).getCeil(); 
}
dword SW_ItemCooldownGauge::get_ctr_max() const
{
	return total_points;
}
bool SW_ItemCooldownGauge::infinite() const
{
	return false;
}
dword SW_ItemCooldownGauge::get_per_container() const
{
	return per_container;
}
void SW_ItemCooldownGauge::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	SW_GaugePiece::draw(dest, xofs, yofs, page);
}
SubscrWidget* SW_ItemCooldownGauge::clone() const
{
	return new SW_ItemCooldownGauge(*this);
}
bool SW_ItemCooldownGauge::copy_prop(SubscrWidget const* src, bool all)
{
	if (!SW_GaugePiece::copy_prop(src, all))
		return false;
	if (src->getType() != getType() || src == this)
		return false;
	SW_ItemCooldownGauge const* other = dynamic_cast<SW_ItemCooldownGauge const*>(src);
	item_class = other->item_class;
	specific_item_id = other->specific_item_id;
	button_id = other->button_id;
	total_points = other->total_points;
	per_container = other->per_container;
	return true;
}
int32_t SW_ItemCooldownGauge::read(PACKFILE* f, word s_version)
{
	if (auto ret = SW_GaugePiece::read(f, s_version))
		return ret;
	if (!p_igetl(&item_class, f))
		return qe_invalid;
	if (!p_igetl(&specific_item_id, f))
		return qe_invalid;
	if (!p_getc(&button_id, f))
		return qe_invalid;
	if (!p_igetl(&total_points, f))
		return qe_invalid;
	if (!p_igetl(&per_container, f))
		return qe_invalid;
	return 0;
}
int32_t SW_ItemCooldownGauge::write(PACKFILE* f) const
{
	if (auto ret = SW_GaugePiece::write(f))
		return ret;
	if (!p_iputl(item_class, f))
		new_return(1);
	if (!p_iputl(specific_item_id, f))
		new_return(1);
	if (!p_putc(button_id, f))
		new_return(1);
	if (!p_iputl(total_points, f))
		new_return(1);
	if (!p_iputl(per_container, f))
		new_return(1);
	return 0;
}

int16_t SW_ItemCooldownText::getX() const
{
	auto tx = x+shadow_x(shadtype);
	switch(align)
	{
		case sstaCENTER:
			return tx-getW()/2;
		case sstaRIGHT:
			return tx-getW();
	}
	return tx;
}
int16_t SW_ItemCooldownText::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_ItemCooldownText::getW() const
{
	int32_t len = text_length(get_zc_font(fontid), get_text().c_str());
	return len == 0 ? 8 : len;
}
word SW_ItemCooldownText::getH() const
{
	return text_height(get_zc_font(fontid));
}
byte SW_ItemCooldownText::getType() const
{
	return widgITMCOOLDOWNTEXT;
}
string SW_ItemCooldownText::format_text(int cd) const
{
	int frames = cd % 60;
	int seconds = (cd / 60) % 60;
	int minutes = ((cd / 60) / 60) % 60;
	
	string text;
	if (flags & SUBSCR_COOLDOWNTEXT_ALTSTYLE)
	{
		if (minutes)
			text += fmt::format("{}m{:02}", minutes, seconds);
		else
			text += fmt::format("{}", seconds);
		
		text += fmt::format(".{:02}s", frames);
	}
	else
	{
		if (minutes)
			text += fmt::format("{}:{:02}", minutes, seconds);
		else
			text += fmt::format("{}", seconds);
		
		text += fmt::format(".{:02}", frames);
	}
	return text;
}
string SW_ItemCooldownText::get_text() const
{
	int item_id = calc_item_from_class_id_button(specific_item_id, button_id, item_class);
	if (unsigned(item_id) >= MAXITEMS)
		return "";
	
	auto b = zq_ignore_item_ownership;
	zq_ignore_item_ownership = false;
	
	auto cd_data = calc_item_cooldown(item_id);
	
	zq_ignore_item_ownership = b;
	
	auto cd = cd_data.cooldown;
	if (!cd)
		return "";
	if (cd < 0)
	{
		cd = cd_data.max_cooldown; // display max time for 'infinite' cooldowns
		if (!cd) cd = 60; // default display 1s
	}
	return format_text(cd);
}
void SW_ItemCooldownText::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	textout_styled_aligned_ex(dest,tempfont,get_text().c_str(),x+xofs,y+yofs,
		shadtype,align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color());
}
SubscrWidget* SW_ItemCooldownText::clone() const
{
	return new SW_ItemCooldownText(*this);
}
bool SW_ItemCooldownText::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_ItemCooldownText const* other = dynamic_cast<SW_ItemCooldownText const*>(src);
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	fontid = other->fontid;
	align = other->align;
	shadtype = other->shadtype;
	c_text = other->c_text;
	c_shadow = other->c_shadow;
	c_bg = other->c_bg;
	item_class = other->item_class;
	specific_item_id = other->specific_item_id;
	button_id = other->button_id;
	return true;
}
int32_t SW_ItemCooldownText::read(PACKFILE *f, word s_version)
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
	if (!p_igetl(&item_class, f))
		return qe_invalid;
	if (!p_igetl(&specific_item_id, f))
		return qe_invalid;
	if (!p_getc(&button_id, f))
		return qe_invalid;
	return 0;
}
int32_t SW_ItemCooldownText::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputl(fontid,f))
		new_return(1);
	if(!p_putc(align,f))
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(1);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	if (!p_iputl(item_class, f))
		new_return(1);
	if (!p_iputl(specific_item_id, f))
		new_return(1);
	if (!p_putc(button_id, f))
		new_return(1);
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
	draw_textbox(dest, x+xofs, y+yofs, w, h, tempfont, text.c_str(),
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(1);
	if(!p_putc(tabsize,f))
		new_return(1);
	if(!p_putwstr(text,f))
		new_return(1);
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
	SubscrWidget* widg = page.get_sel_widg();
	if(!widg) return;
	FONT* tempfont = get_zc_font(fontid);
	std::string str;
	if(widg->override_text.size())
		str = widg->override_text;
	else
	{
		int32_t itemid=widg->getDisplayItem();
		if(itemid > -1)
		{
			// If its a combined bow and arrow, the item ID will have 0xF000 added.
			bool bowarrow = itemid&0xF000;
			if(bowarrow)
				itemid&=~0xF000;
			
			#if IS_PLAYER
			if(replay_version_check(0,19) && !game->get_item(itemid))
				return;
			#endif
			
			itemdata const& itm = itemsbuf[itemid];
			str = itm.get_name(false,itm.type==itype_arrow && !bowarrow);
			if(widg->getType() == widgITEMSLOT && (widg->flags&SUBSCR_CURITM_IGNR_SP_SELTEXT))
			{
				//leave the name as-is
			}
			else if(QMisc.colors.HCpieces_tile && itemid == iHCPiece)
			{
				int hcpphc =  game->get_hcp_per_hc();
				int numhpc = vbound(game->get_HCpieces(),0,hcpphc > 0 ? hcpphc-1 : 0);
				str = fmt::format("{}/{} {}{}", numhpc, hcpphc, str, hcpphc==1?"":"s");
			}
		}
	}
	if(str.size())
		draw_textbox(dest, x+xofs, y+yofs, w, h, tempfont,
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
	if(!SubscrWidget::copy_prop(other,all))
		return false;
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
		new_return(1);
	if(!p_putc(shadtype,f))
		new_return(1);
	if(!p_putc(tabsize,f))
		new_return(1);
	if(auto ret = c_text.write(f))
		return ret;
	if(auto ret = c_shadow.write(f))
		return ret;
	if(auto ret = c_bg.write(f))
		return ret;
	return 0;
}

byte SW_CounterPercentBar::getType() const
{
	return widgCOUNTERPERCBAR;
}
void SW_CounterPercentBar::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	auto c1 = c_fill.get_color();
	auto c2 = c_bg.get_color();
	if(!c1 && !c2) return;
	
	if(flags&SUBSCR_COUNTERPERCBAR_TRANSP)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	
	auto cur = get_ssc_ctr(counter);
	auto max = get_ssc_ctrmax(counter);
	zfix perc = max ? vbound((zfix(cur)/max), 0_zf, 1_zf) : 0_zf;
	auto x2 = x+xofs, y2 = y+yofs;
	bool vertical = (flags&SUBSCR_COUNTERPERCBAR_VERTICAL);
	bool invert = bool(flags&SUBSCR_COUNTERPERCBAR_INVERT) != vertical; // vertical naturally inverts
	if(invert)
	{
		perc = 1_zf - perc;
		zc_swap(c1, c2);
	}
	if(flags&SUBSCR_COUNTERPERCBAR_VERTICAL)
	{
		word ys = word((perc * zfix(h)).getInt());
		if(c1 > -1 && ys)
			rectfill(dest, x2, y2, x2+w-1, y2+ys-1, c1);
		if(c2 > -1)
			rectfill(dest, x2, y2+ys, x2+w-1, y2+h-ys-1, c2);
	}
	else
	{
		word xs = word((perc * zfix(w)).getInt());
		if(c1 > -1 && xs)
			rectfill(dest, x2, y2, x2+xs-1, y2+h-1, c1);
		if(c2 > -1)
			rectfill(dest, x2+xs, y2, x2+w-xs-1, y2+h-1, c2);
	}
	
	if(flags&SUBSCR_COUNTERPERCBAR_TRANSP)
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}
SubscrWidget* SW_CounterPercentBar::clone() const
{
	return new SW_CounterPercentBar(*this);
}
bool SW_CounterPercentBar::copy_prop(SubscrWidget const* src, bool all)
{
	if(src->getType() != getType() || src == this)
		return false;
	SW_CounterPercentBar const* other = dynamic_cast<SW_CounterPercentBar const*>(src);
	if(!SubscrWidget::copy_prop(other,all))
		return false;
	
	counter = other->counter;
	c_fill = other->c_fill;
	c_bg = other->c_bg;
	return true;
}
int32_t SW_CounterPercentBar::read(PACKFILE *f, word s_version)
{
	if(auto ret = SubscrWidget::read(f,s_version))
		return ret;
	
	if(!p_igetw(&counter, f))
		return qe_invalid;
	if(auto ret = c_fill.read(f,s_version))
		return ret;
	if(auto ret = c_bg.read(f,s_version))
		return ret;
	return 0;
}
int32_t SW_CounterPercentBar::write(PACKFILE *f) const
{
	if(auto ret = SubscrWidget::write(f))
		return ret;
	if(!p_iputw(counter, f))
		new_return(1);
	if(auto ret = c_fill.write(f))
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
			return new SW_ItemSlot(old);
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
			if(!ALLOW_NULL_WIDGET) break;
			SubscrWidget* ret = new SubscrWidget(old);
			ret->w = 16;
			ret->h = 16;
			return ret;
		}
		case ssoICON:
		{
			if(!ALLOW_NULL_WIDGET) break;
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
		case widgBTNCOUNTER:
			widg = new SW_BtnCounter();
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
			widg = new SW_ItemSlot();
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
		case widgMISCGAUGE:
			widg = new SW_MiscGaugePiece();
			break;
		case widgITMCOOLDOWNGAUGE:
			widg = new SW_ItemCooldownGauge();
			break;
		case widgITMCOOLDOWNTEXT:
			widg = new SW_ItemCooldownText();
			break;
		case widgTEXTBOX:
			widg = new SW_TextBox();
			break;
		case widgSELECTEDTEXT:
			widg = new SW_SelectedText();
			break;
		case widgCOUNTERPERCBAR:
			widg = new SW_CounterPercentBar();
			break;
		case widgNULL:
			if(!ALLOW_NULL_WIDGET) break;
			widg = new SubscrWidget();
			break;
	}
	return widg;
}
byte SubscrWidget::numFlags(byte type)
{
	switch(type)
	{
		case widgFRAME:
			return SUBSCR_NUMFLAG_2X2FR;
		case widgTEXT:
			return SUBSCR_NUMFLAG_TEXT;
		case widgLINE:
			return SUBSCR_NUMFLAG_LINE;
		case widgRECT:
			return SUBSCR_NUMFLAG_RECT;
		case widgTIME:
			return SUBSCR_NUMFLAG_TIME;
		case widgMMETER:
			return SUBSCR_NUMFLAG_MAGICMET;
		case widgLMETER:
			return SUBSCR_NUMFLAG_LIFEMET;
		case widgBTNITM:
			return SUBSCR_NUMFLAG_BTNITM;
		case widgCOUNTER:
			return SUBSCR_NUMFLAG_COUNTER;
		case widgOLDCTR:
			return SUBSCR_NUMFLAG_COUNTERS;
		case widgMMAPTITLE:
			return SUBSCR_NUMFLAG_MMAPTIT;
		case widgMMAP:
			return SUBSCR_NUMFLAG_MMAP;
		case widgLMAP:
			return SUBSCR_NUMFLAG_LMAP;
		case widgBGCOLOR:
			return SUBSCR_NUMFLAG_CLEAR;
		case widgITEMSLOT:
			return SUBSCR_NUMFLAG_CURITM;
		case widgMCGUFF_FRAME:
			return SUBSCR_NUMFLAG_TRIFR;
		case widgMCGUFF:
			return SUBSCR_NUMFLAG_MCGUF;
		case widgTILEBLOCK:
			return SUBSCR_NUMFLAG_TILEBL;
		case widgMINITILE:
			return SUBSCR_NUMFLAG_MINITL;
		case widgSELECTOR:
			return SUBSCR_NUMFLAG_SELECTOR;
		case widgLGAUGE:
			return SUBSCR_NUMFLAG_LGAUGE;
		case widgMGAUGE:
			return SUBSCR_NUMFLAG_MGAUGE;
		case widgTEXTBOX:
			return SUBSCR_NUMFLAG_TEXTBOX;
		case widgSELECTEDTEXT:
			return SUBSCR_NUMFLAG_SELTEXT;
		case widgMISCGAUGE:
			return SUBSCR_NUMFLAG_MISCGAUGE;
		case widgBTNCOUNTER:
			return SUBSCR_NUMFLAG_BTNCOUNTER;
		case widgITMCOOLDOWNGAUGE:
			return SUBSCR_NUMFLAG_COOLDOWNGAUGE;
		case widgITMCOOLDOWNTEXT:
			return SUBSCR_NUMFLAG_COOLDOWNTEXT;
	}
	return 0;
}
byte SubscrWidget::numFlags()
{
	return numFlags(getType());
}
//For moving on the subscreen. Never called with 'VERIFY' options, so don't worry about them.
void SubscrPage::move_cursor(int dir, bool item_only)
{
	// verify startpos
	if(cursor_pos == 0xFF)
		cursor_pos = 0;
	
	item_only = item_only || !get_qr(qr_FREEFORM_SUBSCREEN_CURSOR);
	
	if(dir==SEL_VERIFY_RIGHT || dir==SEL_VERIFY_LEFT)
		return;
	
	int32_t p=-1;
	int32_t curpos = cursor_pos;
	int32_t firstValidPos=-1;
	
	for(int32_t i=0; i < contents.size(); ++i)
	{
		if((contents[i]->genflags&SUBSCRFLAG_SELECTABLE) && contents[i]->check_conditions())
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
		widg = get_widg_pos(curpos,item_only);
		
		if(!widg)
			return;
		
		//if we've already been here, give up
		if(oldPositions.find(curpos) != oldPositions.end())
			return;
		
		//else, remember we've been here
		oldPositions.insert(curpos);
		
		//Valid stop point?
		if((widg->genflags & SUBSCRFLAG_SELECTABLE) && (!item_only || widg->getItemVal() > -1) && widg->check_conditions())
		{
			cursor_pos = curpos;
			return;
		}
	}
}
int32_t SubscrPage::movepos_legacy(int dir, word startp, word fp, word fp2, word fp3, bool equip_only, bool item_only, bool stay_on_page)
{
	//what will be returned when all else fails.
	//don't return the forbiddenpos... no matter what -DD
	int32_t failpos(0);
	bool start_empty = (startp&0xFF)==0xFF;
	if(start_empty)
	{
		failpos = startp;
		startp = 0;
	}
	else if(startp == fp || startp == fp2 || startp == fp3)
		failpos = 0xFF;
	else failpos = startp;
	
	item_only = item_only || !get_qr(qr_FREEFORM_SUBSCREEN_CURSOR);
	bool verify = dir==SEL_VERIFY_RIGHT || dir==SEL_VERIFY_LEFT;
	
	if(verify)
	{
		equip_only = true;
		item_only = !get_qr(qr_NO_BUTTON_VERIFY);
		if(start_empty && !item_only)
			return startp; //empty is valid
		if(startp != fp && startp != fp2 && startp != fp3)
			if(SubscrWidget* widg = get_widg_pos(startp>>8,item_only))
				if(widg->getType() == widgITEMSLOT
					&& widg->check_conditions()
					&& !(widg->flags&SUBSCR_CURITM_NONEQP)
					&& (widg->genflags & SUBSCRFLAG_SELECTABLE)
					&& (!item_only || widg->getItemVal() > -1))
					return startp; //Valid selectable slot
	}
	
	int32_t p=-1;
	byte curpos = startp>>8;
	word cp2 = (curpos<<8)|index; //curpos is the index for this page, cp2 includes the page index
	int32_t firstValidPos=-1, firstValidEquipPos=-1;
	
	for(int32_t i=0; i < contents.size(); ++i)
	{
		if(contents[i]->getType()==widgITEMSLOT && (contents[i]->genflags&SUBSCRFLAG_SELECTABLE)
			&& contents[i]->check_conditions())
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
			return (contents[firstValidEquipPos]->pos<<8)|index;
		if(firstValidPos>=0)
			return (contents[firstValidPos]->pos<<8)|index;
		//FAILURE
		else return failpos;
	}
	
	//remember we've been here
	std::set<int32_t> oldPositions;
	oldPositions.insert(cp2);
	
	//1. Perform any shifts required by the above
	//2. If that's not possible, go to position 1 and reset the b weapon.
	//2a.  -if we arrive at a position we've already visited, give up and stay there
	//3. Get the weapon at the new slot
	//4. If it's not possible, go to step 1.
	SubscrPage const* pg = this;
	SubscrWidget const* widg = contents[p];
	for(;;)
	{
		//stay_on_page currently unused; nothing changes the current pg in a cycle yet.
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
		cp2 = (curpos<<8)|pg->index;
		//find our new position
		widg = pg->get_widg_pos(curpos,false);
		
		if(!widg)
			return failpos;
		
		//if we've already been here, give up
		if(oldPositions.find(cp2) != oldPositions.end())
			return failpos;
		
		//else, remember we've been here
		oldPositions.insert(cp2);
		
		//Valid stop point?
		if((!stay_on_page||(cp2&0xFF)==index)
			&& (widg->genflags & SUBSCRFLAG_SELECTABLE)
			&& widg->check_conditions()
			&& cp2 != fp && cp2 != fp2 && cp2 != fp3
			&& (!equip_only || widg->getType()!=widgITEMSLOT || !(widg->flags & SUBSCR_CURITM_NONEQP))
			&& (!item_only || widg->getItemVal()>-1))
			return cp2;
	}
}
void SubscrPage::move_legacy(int dir, bool equip_only, bool item_only)
{
	cursor_pos = movepos_legacy(dir,(cursor_pos<<8)|index,255,255,255,equip_only,item_only,true)>>8;
}
SubscrWidget* SubscrPage::get_widg_pos(byte pos, bool item_only) const
{
	for(size_t q = 0; q < contents.size(); ++q)
	{
		if(!(contents[q]->genflags & SUBSCRFLAG_SELECTABLE))
			continue;
		if (item_only && contents[q]->getType() == widgITEMSLOT)
			if (contents[q]->flags & SUBSCR_CURITM_NONEQP)
				continue;
		if(contents[q]->pos == pos)
			return contents[q];
	}
	return nullptr;
}
int32_t SubscrPage::get_item_pos(byte pos, bool item_only)
{
	auto* w = get_widg_pos(pos,item_only);
	if(w)
		return w->getItemVal();
	return -1;
}
SubscrWidget* SubscrPage::get_sel_widg() const
{
	return get_widg_pos(cursor_pos,false);
}
int32_t SubscrPage::get_sel_item(bool display)
{
	auto* w = get_sel_widg();
	if(w)
		return display ? w->getDisplayItem() : w->getItemVal();
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
int32_t SubscrPage::find_label_index(std::string const& lbl) const
{
	if(lbl.empty()) return -1;
	int32_t indx = 0;
	for(SubscrWidget* widg : contents)
	{
		if(!lbl.compare(widg->label))
			return indx;
		++indx;
	}
	return -1;
}
int32_t SubscrPage::widget_index(SubscrWidget* widg) const
{
	int32_t indx = 0;
	for(SubscrWidget* w : contents)
	{
		if(w == widg)
			return indx;
		++indx;
	}
	return -1;
}

SubscrWidget* SubscrPage::get_widget(int indx)
{
	if(unsigned(indx) >= contents.size())
		return nullptr;
	
	return contents[indx];
}
void SubscrPage::delete_widg(word ind)
{
	if(ind >= contents.size())
		return;
	for(auto it = contents.begin(); it != contents.end();)
	{
		if(ind--) ++it;
		else
		{
			it = contents.erase(it);
			break;
		}
	}
	//curr_subscreen_object is handled outside of here
}
void SubscrPage::swap_widg(word ind1, word ind2)
{
	if(ind1 >= contents.size() || ind2 >= contents.size())
		return;
	zc_swap(contents[ind1],contents[ind2]);
	//curr_subscreen_object is handled outside of here
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
void SubscrPage::swap(SubscrPage& other)
{
	contents.swap(other.contents);
	zc_swap(cursor_pos,other.cursor_pos);
	zc_swap(index,other.index);
	zc_swap(parent,other.parent);
	for(SubscrWidget* w : contents)
		w->parentPage = this;
	for(SubscrWidget* w : other.contents)
		w->parentPage = &other;
}
SubscrPage::~SubscrPage()
{
	clear();
}
SubscrPage& SubscrPage::operator=(SubscrPage const& other)
{
	clear();
	cursor_pos = other.cursor_pos;
	index = other.index;
	parent = other.parent;
	for(SubscrWidget* widg : other.contents)
	{
		push_back(widg->clone());
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
		push_back(widg);
	}
	return 0;
}
int32_t SubscrPage::write(PACKFILE *f) const
{
    if(!p_iputl(cursor_pos,f))
        new_return(1);
	word sz = zc_min(65535,contents.size());
	if(!p_iputw(sz,f))
		new_return(1);
	for(word q = 0; q < sz; ++q)
		if(auto ret = contents[q]->write(f))
			return ret;
	
	return 0;
}
word SubscrPage::getIndex() const
{
	return index;
}
void SubscrPage::setParent(ZCSubscreen const* newparent)
{
	parent = newparent;
}
ZCSubscreen const* SubscrPage::getParent() const
{
	return parent;
}
void SubscrPage::push_back(SubscrWidget* widg)
{
	widg->parentPage = this;
	contents.push_back(widg);
}
size_t SubscrPage::size() const
{
	return contents.size();
}
bool SubscrPage::empty() const
{
	return contents.empty();
}
SubscrWidget* SubscrPage::at(size_t ind)
{
	return contents.at(ind);
}
SubscrWidget* const& SubscrPage::operator[](size_t ind) const
{
	return contents[ind];
}
void SubscrPage::force_update()
{
	for(SubscrWidget* w : contents)
		w->parentPage = this;
}
SubscrPage& ZCSubscreen::cur_page()
{
	if(pages.empty())
		pages.emplace_back();
	curpage = vbound(curpage,0,pages.size()-1);
	return pages[curpage];
}
SubscrPage* ZCSubscreen::get_page(byte id)
{
	if(id >= pages.size()) return nullptr;
	return &pages[id];
}
bool ZCSubscreen::get_page_pos(int32_t itmid, word& pgpos)
{
	for(byte q = 0; q < pages.size(); ++q)
	{
		byte p = pages[q].get_pos_of_item(itmid);
		if(p > -1)
		{
			pgpos = q | (p<<8);
			return true;
		}
	}
	pgpos = 255;
	return false;
}
int32_t ZCSubscreen::get_item_pos(word pgpos)
{
	if((pgpos&0xFF) >= pages.size()) return -1;
	return pages[pgpos&0xFF].get_item_pos(pgpos>>8, false);
}
void ZCSubscreen::delete_page(byte id)
{
	if(id >= pages.size()) return;
	
	if(pages.size()==1)
	{
		pages[0].clear();
		pages[0].index = 0;
		pages[0].parent = this;
	}
	else
	{
		if(curpage >= id)
			--curpage;
		for(int q = 0; q < 4; ++q)
		{
			if((def_btns[q]&0xFF) > id)
				--def_btns[q];
			else if((def_btns[q]&0xFF) == id)
				def_btns[q] = 255;
		}
		auto ind = 0;
		for(auto it = pages.begin(); it != pages.end();)
		{
			it->parent = this;
			if(ind < id)
				++it;
			else if(ind == id)
				it = pages.erase(it);
			else it->index--;
			++ind;
		}
	}
}
bool ZCSubscreen::add_page(byte id)
{
	if(id > pages.size())
		id = pages.size(); //add new page at end
	if(id >= MAX_SUBSCR_PAGES) //no more room!
		return false;
	auto& pg = pages.emplace_back();
	pg.index = pages.size()-1;
	pg.parent = this;
	for(byte ind = pages.size()-1; ind > id; --ind)
		swap_pages(ind,ind-1);
	curpage = id;
	return true;
}
void ZCSubscreen::swap_pages(byte ind1, byte ind2)
{
	if(ind1 >= pages.size() || ind2 >= pages.size())
		return;
	pages[ind1].swap(pages[ind2]);
	if(curpage == ind1) curpage = ind2;
	else if(curpage == ind2) curpage = ind1;
	for(int q = 0; q < 4; ++q)
	{
		if((def_btns[q]&0xFF) == ind1)
			def_btns[q] = (def_btns[q]&0xFF00)|ind2;
		else if((def_btns[q]&0xFF) == ind2)
			def_btns[q] = (def_btns[q]&0xFF00)|ind1;
	}
}
void ZCSubscreen::clear()
{
	*this = ZCSubscreen();
}
void ZCSubscreen::copy_settings(const ZCSubscreen& src, bool all)
{
	if(all)
	{
		curpage = src.curpage;
		sub_type = src.sub_type;
		name = src.name;
		for(int q = 0; q < 4; ++q)
			def_btns[q] = src.def_btns[q];
		pages.clear();
		pages = src.pages;
		for(size_t q = 0; q < pages.size(); ++q)
		{
			pages[q].index = q;
			pages[q].parent = this;
			pages[q].force_update();
		}
	}
	script = src.script;
	for(int q = 0; q < 8; ++q)
		initd[q] = src.initd[q];
	btn_left = src.btn_left;
	btn_right = src.btn_right;
	flags = src.flags;
	trans_left = src.trans_left;
	trans_right = src.trans_right;
	if(flags & SUBFLAG_ACT_OVERRIDESEL)
		selector_setting = src.selector_setting;
	else selector_setting.clear();
}
void ZCSubscreen::draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime)
{
	if(pages.empty()) return;
	
	if(sub_type == sstACTIVE && subscr_pg_animating && subscr_anim == this)
	{
		if(subscr_pg_to >= pages.size())
			; //fail animation
		else if(subscr_pg_transition.type == sstrINSTANT)
			curpage = subscr_pg_to; //instant anim
		else if(subscr_pg_from >= pages.size() || !subscr_pg_bmp1
			|| !subscr_pg_bmp2 || !subscr_pg_subbmp)
			; //fail animation
		else //progress through animation
		{
			clear_bitmap(subscr_pg_bmp1);
			clear_bitmap(subscr_pg_bmp2);
			//Draw both pages to their respective subbitmaps
			pages[subscr_pg_from].draw(subscr_pg_bmp1,0,0,pos,showtime);
			pages[subscr_pg_to].draw(subscr_pg_bmp2,0,0,pos,showtime);
			//Draw to the screen animation-dependently
			if(subscr_pg_transition.draw(dest,subscr_pg_bmp1,subscr_pg_bmp2,xofs,yofs))
				//animation finished
				curpage = subscr_pg_to;
			else
			{
				++subscr_pg_clk;
				return;
			}
		}
		subscrpg_clear_animation();
	}
	
	size_t page = curpage >= pages.size() ? 0 : curpage;
	pages[page].draw(dest,xofs,yofs,pos,showtime);
}
void ZCSubscreen::load_old(subscreen_group const& g)
{
	name = g.name;
	sub_type = g.ss_type;
	pages.clear();
	SubscrPage& p = pages.emplace_back();
	p.index = 0;
	p.parent = this;
	for(int ind = 0; ind < MAXSUBSCREENITEMS && g.objects[ind].type != ssoNULL; ++ind)
	{
		auto* w = SubscrWidget::fromOld(g.objects[ind]);
		if(w)
		{
			p.push_back(w);
		}
	}
}
void ZCSubscreen::load_old(subscreen_object const* arr)
{
	pages.clear();
	SubscrPage& p = pages.emplace_back();
	p.index = 0;
	p.parent = this;
	for(int ind = 0; ind < MAXSUBSCREENITEMS && arr[ind].type != ssoNULL; ++ind)
	{
		SubscrWidget* w = SubscrWidget::fromOld(arr[ind]);
		if(!w) continue;
		if(arr[ind].type == ssoNONE || arr[ind].type == ssoNULL)
		{
			delete w;
			continue;
		}
		p.push_back(w);
	}
}
int32_t ZCSubscreen::read(PACKFILE *f, word s_version)
{
    if(!p_getcstr(&name,f))
        return qe_invalid;
	if(!p_getc(&sub_type,f))
        return qe_invalid;
	if(!p_igetl(&flags,f))
		new_return(1);
	bool active = sub_type == sstACTIVE;
	if(active)
	{
		if(flags & SUBFLAG_ACT_OVERRIDESEL)
			if(auto ret = selector_setting.read(f,s_version))
				return ret;
		for(int q = 0; q < 4; ++q)
			if(!p_igetw(&def_btns[q],f))
				return qe_invalid;
		if(!p_getc(&btn_left,f))
			new_return(1);
		if(!p_getc(&btn_right,f))
			new_return(1);
		if(auto ret = trans_left.read(f, s_version))
			return ret;
		if(auto ret = trans_right.read(f, s_version))
			return ret;
		if(!p_igetw(&script,f))
			return qe_invalid;
		if(script)
		{
			for(int q = 0; q < 8; ++q)
				if(!p_igetl(&initd[q],f))
					return qe_invalid;
		}
	}
	byte pagecnt;
	if(!p_getc(&pagecnt,f))
        return qe_invalid;
	pages.clear();
	for(byte q = 0; q < pagecnt; ++q)
	{
		SubscrPage pg;
		if(auto ret = pg.read(f, s_version))
			return ret;
		pg.index = q;
		pg.parent = this;
		pages.push_back(pg);
	}
	for(byte q = 0; q < 4; ++q)
		if((def_btns[q] & 0xFF) >= pagecnt)
			def_btns[q] = 0xFF;
	if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
	{
		script = 0;
		for(int q = 0; q < 8; ++q)
			initd[q] = 0;
	}
	return 0;
}
int32_t ZCSubscreen::write(PACKFILE *f) const
{
    if(!p_putcstr(name,f))
        new_return(1);
	if(!p_putc(sub_type,f))
		new_return(1);
	if(!p_iputl(flags,f))
		new_return(1);
	bool active = sub_type == sstACTIVE;
	byte pagecnt = zc_min(MAX_SUBSCR_PAGES,pages.size());
	if(active)
	{
		if(flags & SUBFLAG_ACT_OVERRIDESEL)
			if(auto ret = selector_setting.write(f))
				return ret;
		for(int q = 0; q < 4; ++q)
		{
			word val = def_btns[q];
			if((val & 0xFF) >= pagecnt)
				val = 0xFF;
			if(!p_iputw(val,f))
				new_return(1);
		}
		if(!p_putc(btn_left,f))
			new_return(1);
		if(!p_putc(btn_right,f))
			new_return(1);
		if(auto ret = trans_left.write(f))
			return ret;
		if(auto ret = trans_right.write(f))
			return ret;
		if(!p_iputw(script,f))
			new_return(1);
		if(script)
		{
			for(int q = 0; q < 8; ++q)
				if(!p_iputl(initd[q],f))
					new_return(1);
		}
	}
	if(pagecnt && !active)
		pagecnt = 1;
	if(!p_putc(pagecnt,f))
		new_return(1);
	for(byte q = 0; q < pagecnt; ++q)
		if(auto ret = pages[q].write(f))
			return ret;
	
	return 0;
}

bool ZCSubscreen::wrap_pg(int& pg, bool nowrap)
{
	if(pg < 0)
	{
		if(nowrap)
			return false;
		pg = pages.size()-1;
	}
	else if(pg >= pages.size())
	{
		if(nowrap)
			return false;
		pg = 0;
	}
	return true;
}
void ZCSubscreen::check_btns(byte btnflgs)
{
	if(subscr_pg_animating) return;
	int pg = curpage;
	SubscrTransition* tr = nullptr;
	if(btn_right&btnflgs)
	{
		++pg;
		tr = &trans_right;
	}
	else if(btn_left&btnflgs)
	{
		--pg;
		tr = &trans_left;
	}
	else return;
	if(!wrap_pg(pg,flags&SUBFLAG_ACT_NOPAGEWRAP))
		return;
	subscrpg_animate(curpage,pg,*tr,*this);
}
void ZCSubscreen::page_change(byte mode, byte targ, SubscrTransition const& trans, bool nowrap)
{
	if(subscr_pg_animating) return;
	int pg = curpage;
	switch(mode)
	{
		case PGGOTO_NEXT:
			++pg;
			break;
		case PGGOTO_PREV:
			--pg;
			break;
		case PGGOTO_TRG:
			if(targ < pages.size())
				pg = targ;
			break;
	}
	if(pg == curpage)
		return;
	if(!wrap_pg(pg,nowrap))
		return;
	subscrpg_animate(curpage,pg,trans,*this);
}

ZCSubscreen::ZCSubscreen(ZCSubscreen const& other)
{
	copy_settings(other,true);
}
ZCSubscreen& ZCSubscreen::operator=(ZCSubscreen const& other)
{
	copy_settings(other,true);
	return *this;
}
