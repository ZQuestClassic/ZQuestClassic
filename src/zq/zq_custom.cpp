#include <map>
#include <cstring>
#include <stdio.h>
#include <cstring>

#include "base/files.h"
#include "base/qrs.h"
#include "base/packfile.h"
#include "base/gui.h"
#include "qst.h"
#include "zc/zc_custom.h"
#include "zq/zq_custom.h"
#include "tiles.h"
#include "zq/zq_tiles.h"
#include "zq/zq_misc.h"
#include "zq/zquest.h"
#include "items.h"
#include "base/zsys.h"
#include "sfx.h"
#include "init.h"
#include "defdata.h"
#include "zc/ffscript.h"
#include "dialog/itemeditor.h"
#include "dialog/misc_sfx.h"
#include "dialog/misc_sprs.h"
#include "dialog/info.h"
#include "dialog/spritedata.h"
#include "dialog/info_lister.h"
#include "zinfo.h"
extern FFScript FFCore;

extern int32_t ex;
extern void reset_itembuf(itemdata *item, int32_t id);
extern const char *sfxlist(int32_t index, int32_t *list_size);
// zq_subscr.cpp
extern int32_t sstype_drop_proc(int32_t msg,DIALOG *d,int32_t c);

extern int32_t biw_cnt;

extern ZModule zcm;
extern zcmodule moduledata;


#ifdef _MSC_VER
#define stricmp _stricmp
#endif


int32_t hero_animation_speed = 1; //lower is faster animation

int32_t d_ecstile_proc(int32_t msg,DIALOG *d,int32_t c);

int32_t d_cstile_proc(int32_t msg,DIALOG *d,int32_t c)
{
	//these are here to bypass compiler warnings about unused arguments
	c=c;
	
	switch(msg)
	{
	case MSG_CLICK:
	{
		int32_t f = 0;
		int32_t t = d->d1;
		int32_t cs = d->d2;
		
		if((CHECK_CTRL_CMD)
			? select_tile_2(t,f,1,cs,true)
			: select_tile(t,f,1,cs,true))
		{
			d->d1 = t;
			d->d2 = cs;
			return D_REDRAW;
		}
	}
	break;
	
	case MSG_DRAW:
		d->w = 36;
		d->h = 36;
		
		BITMAP *buf = create_bitmap_ex(8,20,20);
		BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
		
		if(buf && bigbmp)
		{
			clear_bitmap(buf);
			
			if(d->d1)
				overtile16(buf,d->d1,2,2,d->d2,0);
				
			stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->w-2, d->h-2);
			destroy_bitmap(buf);
			jwin_draw_frame(bigbmp,0,0,d->w,d->h,FR_DEEP);
			blit(bigbmp,screen,0,0,d->x,d->y,d->w,d->h);
			destroy_bitmap(bigbmp);
		}
		
		//    text_mode(d->bg);
		FONT *fonty = font;
		textprintf_ex(screen,fonty,d->x+d->w,d->y+2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Tile: %d",d->d1);
		textprintf_ex(screen,fonty,d->x+d->w,d->y+text_height(fonty)+3,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet: %d",d->d2);
		break;
	}
	
	return D_O_K;
}

void large_dialog(DIALOG *d)
{
	large_dialog(d, 1.5f);
}

void large_dialog(DIALOG *d, float RESIZE_AMT)
{
	if(d[0].d1 == 0)
	{
		d[0].d1 = 1;
		int32_t oldwidth = d[0].w;
		int32_t oldheight = d[0].h;
		int32_t oldx = d[0].x;
		int32_t oldy = d[0].y;
		d[0].x -= int32_t(float(d[0].w)/RESIZE_AMT);
		d[0].y -= int32_t(float(d[0].h)/RESIZE_AMT);
		d[0].w = int32_t(float(d[0].w)*RESIZE_AMT);
		d[0].h = int32_t(float(d[0].h)*RESIZE_AMT);
		
		for(int32_t i=1; d[i].proc!=NULL; i++)
		{
			// Place elements horizontally
			double xpc = ((double)(d[i].x - oldx) / (double)oldwidth);
			d[i].x = int32_t(d[0].x + (xpc*double(d[0].w)));
			
			// Horizontally resize elements
			if((d[i].proc == d_maptile_proc && d[i].dp2!=(void*)1))
			{
				d[i].x += (int32_t)(float(d[i].w)/4.f);
			}
			else if(d[i].proc == d_comboframe_proc)
			{
				d[i].w *= 2;
				d[i].w -= 4;
			}
			else if(d[i].proc == d_wflag_proc || d[i].proc==d_bitmap_proc || d[i].proc == d_maptile_proc)
			{
				d[i].w *= 2;
			}
			else if(d[i].proc == jwin_button_proc || d[i].proc == jwin_swapbtn_proc)
				d[i].w = int32_t(d[i].w*1.5);
			else d[i].w = int32_t(float(d[i].w)*RESIZE_AMT);
			
			// Place elements vertically
			double ypc = ((double)(d[i].y - oldy) / (double)oldheight);
			d[i].y = int32_t(d[0].y + (ypc*double(d[0].h)));
			
			// Vertically resize elements
			if((d[i].proc == d_maptile_proc && d[i].dp2!=(void*)1))
			{
			}
			else if(d[i].proc == jwin_edit_proc || d[i].proc == jwin_check_proc || d[i].proc == jwin_checkfont_proc)
			{
				d[i].h = int32_t((double)d[i].h*1.5);
			}
			else if(d[i].proc == jwin_droplist_proc || d[i].proc == d_ndroplist_proc || d[i].proc == d_idroplist_proc || d[i].proc == d_nidroplist_proc || d[i].proc == d_dropdmaplist_proc
					|| d[i].proc == jwin_as_droplist_proc  || d[i].proc == d_ffcombolist_proc || d[i].proc == sstype_drop_proc
					|| d[i].proc == d_comboalist_proc)
			{
				d[i].y += int32_t((double)d[i].h*0.25);
				d[i].h = int32_t((double)d[i].h*1.25);
			}
			else if(d[i].proc == d_comboframe_proc)
			{
				d[i].h *= 2;
				d[i].h -= 4;
			}
			else if(d[i].proc == d_wflag_proc || d[i].proc==d_bitmap_proc || d[i].proc == d_maptile_proc)
			{
				d[i].h *= 2;
			}
			else if(d[i].proc == jwin_button_proc || d[i].proc == jwin_swapbtn_proc)
				d[i].h = int32_t(d[i].h*1.5);
			else d[i].h = int32_t(float(d[i].h)*RESIZE_AMT);
			
			// Fix frames
			if(d[i].proc == jwin_frame_proc)
			{
				d[i].x++;
				d[i].y++;
				d[i].w-=4;
				d[i].h-=4;
			}
		}
	}
	
	for(int32_t i=1; d[i].proc != NULL; i++)
	{
		if(d[i].proc==jwin_slider_proc)
			continue;
			
		// Bigger font
		bool bigfontproc = (d[i].proc != jwin_droplist_proc && d[i].proc != jwin_abclist_proc && d[i].proc != d_wlist_proc && d[i].proc != jwin_list_proc && d[i].proc != d_dmaplist_proc
							&& d[i].proc != d_dropdmaplist_proc && d[i].proc != d_warplist_proc && d[i].proc != d_warplist_proc && d[i].proc != d_wclist_proc && d[i].proc != d_ndroplist_proc
							&& d[i].proc != d_idroplist_proc && d[i].proc != d_nidroplist_proc && d[i].proc != jwin_as_droplist_proc && d[i].proc != d_ffcombolist_proc && d[i].proc != d_enelist_proc && d[i].proc != sstype_drop_proc
							&& d[i].proc != d_comboalist_proc);
							
		if(bigfontproc && !d[i].dp2)
		{
			d[i].dp2 = get_zc_font(font_lfont_l);
		}
		else if(!bigfontproc)
		{
			((ListData *) d[i].dp)->font = &a4fonts[font_lfont_l];
		}
		
		// Make checkboxes work
		if(d[i].proc == jwin_check_proc)
			d[i].proc = jwin_checkfont_proc;
		else if(d[i].proc == jwin_radio_proc)
			d[i].proc = jwin_radiofont_proc;
	}
	
	jwin_center_dialog(d);
}

static ListData weapon_list(weaponlist_num, &a4fonts[font_pfont]);

const char *defenselist(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		bound(index,0,edLAST-1);
		
		switch(index)
		{
			case 0:
				return "(None)";

			case edHALFDAMAGE:
				return "1/2 Damage";

			case edQUARTDAMAGE:
				return "1/4 Damage";


			case edSTUNONLY:
				return "Stun";

			case edSTUNORCHINK:
				return "Stun Or Block";

			case edSTUNORIGNORE:
				return "Stun Or Ignore";

			case edCHINKL1:
				return "Block If < 1";

			case edCHINKL2:
				return "Block If < 2";

			case edCHINKL4:
				return "Block If < 4";

			case edCHINKL6:
				return "Block If < 6";

			case edCHINKL8:
				return "Block If < 8";



			case edCHINK:
				return "Block";

			case edIGNOREL1:
				return "Ignore If < 1";

			case edIGNORE:
				return "Ignore";




			case ed1HKO:
				return "One-Hit-Kill";

			case edCHINKL10: //If damage is less than 10
				return "Block if Power < 10";

			case ed2x: //Double damage
				return "Double Damage";
			case ed3x: //Triple Damage
				return "Triple Damage";
			case ed4x: //4x damage
				return "Quadruple Damage";

			case edHEAL: //recover the weapon damage in HP
				return "Enemy Gains HP = Damage";

			case edTRIGGERSECRETS: //Triggers screen secrets. 
				return "Trigger Screen Secrets";

			case edSPLIT: 
				return "Split";
			case edREPLACE:
				return "Transform";

			case edSUMMON: 
				return "Summon";

			case edEXPLODESMALL: 
				return "Bomb Explosion";

			case edEXPLODELARGE: 
				return "Superbomb Explosion";

			case edEXPLODEHARMLESS: 
				return "Harmless Explosion";

			case edFREEZE: //Hit by ice.. 
				return "Freeze Solid";
				
			case edSWITCH:
				return "Switch w/ Player";
			default:
				return "[reserved]";
		}
	}
	
	*list_size = edLAST;
	return NULL;
}

static ListData defense_list(defenselist, &font);

const char *itemscriptdroplist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = biitems_cnt;
		return NULL;
	}
	
	return biitems[index].first.c_str();
}

ListData itemscript_list(itemscriptdroplist, &a4fonts[font_pfont]);

const char *itemspritescriptdroplist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = biitemsprites_cnt;
		return NULL;
	}
	
	return biditemsprites[index].first.c_str();
}

ListData itemspritescript_list(itemspritescriptdroplist, &a4fonts[font_pfont]);

const char *lweaponscriptdroplist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = bilweapons_cnt;
		return NULL;
	}
	
	return bilweapons[index].first.c_str();
}

ListData lweaponscript_list(lweaponscriptdroplist, &a4fonts[font_pfont]);

void edit_itemdata(int32_t index)
{
	call_item_editor(index);
}

int32_t readoneitem(PACKFILE *f, int32_t index)
{
	dword section_version = 0;
	dword section_cversion = 0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	itemdata tempitem;
	memset(&tempitem, 0, sizeof(itemdata));
		//reset_itembuf(&tempitem,i);
	
   
	char istring[64]={0};
	//section version info
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	
	if(!p_igetw(&section_cversion,f))
	{
		return 0;
	}
	al_trace("readoneitem section_version: %d\n", section_version);
	al_trace("readoneitem section_cversion: %d\n", section_cversion);
	
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zitem packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	else if ( ( section_version > V_ITEMS ) || ( section_version == V_ITEMS && section_cversion < CV_ITEMS ) )
	{
		al_trace("Cannot read .zitem packfile made using V_ITEMS (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zitem packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
   
	if(!pfread(&istring, 64, f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.tile,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.misc_flags,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.csets,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.frames,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.speed,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.delay,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.ltm,f))
	{	
		return 0;
	}
	
	if(!p_igetl(&tempitem.family,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.fam_type,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.power,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.flags,f))
	{
		return 0;
	}
	
	if(!p_igetw(&tempitem.script,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.count,f))
	{
		return 0;
	}
	
	if(!p_igetw(&tempitem.amount,f))
	{
		return 0;
	}
	
	if(!p_igetw(&tempitem.collect_script,f))
	{
		return 0;
	}
	
	if(!p_igetw(&tempitem.setmax,f))
	{
		return 0;
	}
	
	if(!p_igetw(&tempitem.max,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.playsound,f))
	{
		return 0;
	}
	
	for(int32_t j=0; j<8; j++)
	{
		if(!p_igetl(&tempitem.initiald[j],f))
		{
		return 0;
		}
	}
	
	for(int32_t j=0; j<2; j++)
	{
		if(!p_getc(&tempitem.initiala[j],f))
		{
			return 0;
		}
	}
	
	if(!p_getc(&tempitem.wpn,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn2,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn3,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn4,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn5,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn6,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn7,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn8,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn9,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.wpn10,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.pickup_hearts,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc1,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc2,f))
	{
		return 0;
	}
	
	if(section_version < 53)
	{
		byte tempbyte;
		if(!p_getc(&tempbyte,f))
		{
			return 0;
		}
		tempitem.cost_amount[0] = tempbyte;
		switch(tempitem.family)
		{
			case itype_arrow:
			case itype_bomb:
			case itype_sbomb:
				tempitem.cost_amount[1] = 1;
				break;
			default:
				tempitem.cost_amount[1] = 0;
		}
	}
	else
	{
		for(auto q = 0; q < 2; ++q)
		{
			if(!p_igetw(&tempitem.cost_amount[q],f))
			{
				return 0;
			}
		}
	}
	
	if(!p_igetl(&tempitem.misc3,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc4,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc5,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc6,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc7,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc8,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc9,f))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.misc10,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.usesound,f))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.usesound2,f))
	{
		return 0;
	}
	
	if ( zversion >= 0x255 )
	{
		if  ( section_version >= 45 )
		{
			//New itemdata vars -Z
			//! version 27
			
		if(!p_getc(&tempitem.useweapon,f))
			{
			return 0;
			}
			if(!p_getc(&tempitem.usedefence,f))
			{
			return 0;
			}
			if(!p_igetl(&tempitem.weaprange,f))
			{
			return 0;
			}
			if(!p_igetl(&tempitem.weapduration,f))
			{
			return 0;
			}
			for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) {
				if(!p_igetl(&tempitem.weap_pattern[q],f))
				{
				return 0;
				}
			}
			//version 28
			if(!p_igetl(&tempitem.duplicates,f))
			{
				return 0;
			}
			for ( int32_t q = 0; q < INITIAL_D; q++ )
			{
				if(!p_igetl(&tempitem.weap_initiald[q],f))
				{
					return 0;
				}
			}
			for ( int32_t q = 0; q < INITIAL_A; q++ )
			{
				if(!p_getc(&tempitem.weap_initiala[q],f))
				{
					return 0;
				}
			}

			if(!p_getc(&tempitem.drawlayer,f))
			{
				return 0;
			}


			if(!p_igetl(&tempitem.hxofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.hyofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.hxsz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.hysz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.hzsz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.xofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.yofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_hxofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_hyofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_hxsz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_hysz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_hzsz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_xofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_yofs,f))
			{
				return 0;
			}
			if(!p_igetw(&tempitem.weaponscript,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.wpnsprite,f))
			{
				return 0;
			}
			size_t num_cost_tmr = (section_version > 52 ? 2 : 1);
			for(size_t q = 0; q < num_cost_tmr; ++q)
			{
				if(!p_igetl(&tempitem.magiccosttimer[q],f))
				{
					return 0;
				}
			}
			if(!p_igetl(&tempitem.overrideFLAGS,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.tilew,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.tileh,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weapoverrideFLAGS,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_tilew,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_tileh,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.pickup,f))
			{
				return 0;
			}
			if(!p_igetw(&tempitem.pstring,f))
			{
				return 0;
			}
			if(!p_igetw(&tempitem.pickup_string_flags,f))
			{
				return 0;
			}
			
			size_t num_cost_ctr = (section_version > 52 ? 2 : 1);
			for(size_t q = 0; q < num_cost_ctr; ++q)
			{
				if(!p_getc(&tempitem.cost_counter[q],f))
				{
					return 0;
				}
			}
			
			//InitD[] labels
			for ( int32_t q = 0; q < 8; q++ )
			{
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&tempitem.initD_label[q][w],f))
					{
						return 0;
					} 
				}
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&tempitem.weapon_initD_label[q][w],f))
					{
						return 0;
					} 
				}
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&tempitem.sprite_initD_label[q][w],f))
					{
						return 0;
					} 
				}
				if(!p_igetl(&tempitem.sprite_initiald[q],f))
				{
					return 0;
				} 
			}
			for ( int32_t q = 0; q < 2; q++ )
			{
				if(!p_getc(&tempitem.sprite_initiala[q],f))
				{
					return 0;
				} 
				
			}
			if(!p_igetw(&tempitem.sprite_script,f))
			{
				return 0;
			} 
			
			if  ( section_version >= 47 )
			{
				if(!p_getc(&tempitem.pickupflag,f))
				{
					return 0;
				} 
			}	
			//read it into an item
		}
	}
	itemsbuf[index] = tempitem;
	strcpy(item_string[index], istring);
	   
	return 1;
}

int32_t writeoneitem(PACKFILE *f, int32_t i)
{
	
	dword section_version=V_ITEMS;
	dword section_cversion=CV_ITEMS;
	int32_t zversion = ZELDA_VERSION;
	int32_t zbuild = VERSION_BUILD;
	
  
	//section version info
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		new_return(2);
	}
	
	if(!p_iputw(section_cversion,f))
	{
		new_return(3);
	}
	
   
		
	   //section data        
		
			if(!pfwrite(item_string[i], 64, f))
			{
				new_return(5);
			}
		
		if(!p_iputl(itemsbuf[i].tile,f))
			{
				new_return(6);
			}
			
			if(!p_putc(itemsbuf[i].misc_flags,f))
			{
				new_return(7);
			}
			
			if(!p_putc(itemsbuf[i].csets,f))
			{
				new_return(8);
			}
			
			if(!p_putc(itemsbuf[i].frames,f))
			{
				new_return(9);
			}
			
			if(!p_putc(itemsbuf[i].speed,f))
			{
				new_return(10);
			}
			
			if(!p_putc(itemsbuf[i].delay,f))
			{
				new_return(11);
			}
			
			if(!p_iputl(itemsbuf[i].ltm,f))
			{
				new_return(12);
			}
			
			if(!p_iputl(itemsbuf[i].family,f))
			{
				new_return(13);
			}
			
			if(!p_putc(itemsbuf[i].fam_type,f))
			{
				new_return(14);
			}
			
			if(!p_iputl(itemsbuf[i].power,f))
			{
				new_return(14);
			}
			
			if(!p_iputl(itemsbuf[i].flags,f))
			{
				new_return(15);
			}
			
			if(!p_iputw(itemsbuf[i].script,f))
			{
				new_return(16);
			}
			
			if(!p_putc(itemsbuf[i].count,f))
			{
				new_return(17);
			}
			
			if(!p_iputw(itemsbuf[i].amount,f))
			{
				new_return(18);
			}
			
			if(!p_iputw(itemsbuf[i].collect_script,f))
			{
				new_return(19);
			}
			
			if(!p_iputw(itemsbuf[i].setmax,f))
			{
				new_return(21);
			}
			
			if(!p_iputw(itemsbuf[i].max,f))
			{
				new_return(22);
			}
			
			if(!p_putc(itemsbuf[i].playsound,f))
			{
				new_return(23);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_iputl(itemsbuf[i].initiald[j],f))
				{
					new_return(24);
				}
			}
			
			for(int32_t j=0; j<2; j++)
			{
				if(!p_putc(itemsbuf[i].initiala[j],f))
				{
					new_return(25);
				}
			}
			
			if(!p_putc(itemsbuf[i].wpn,f))
			{
				new_return(26);
			}
			
			if(!p_putc(itemsbuf[i].wpn2,f))
			{
				new_return(27);
			}
			
			if(!p_putc(itemsbuf[i].wpn3,f))
			{
				new_return(28);
			}
			
			if(!p_putc(itemsbuf[i].wpn4,f))
			{
				new_return(29);
			}
			
			if(!p_putc(itemsbuf[i].wpn5,f))
			{
				new_return(30);
			}
			
			if(!p_putc(itemsbuf[i].wpn6,f))
			{
				new_return(31);
			}
			
			if(!p_putc(itemsbuf[i].wpn7,f))
			{
				new_return(32);
			}
			
			if(!p_putc(itemsbuf[i].wpn8,f))
			{
				new_return(33);
			}
			
			if(!p_putc(itemsbuf[i].wpn9,f))
			{
				new_return(34);
			}
			
			if(!p_putc(itemsbuf[i].wpn10,f))
			{
				new_return(35);
			}
			
			if(!p_putc(itemsbuf[i].pickup_hearts,f))
			{
				new_return(36);
			}
			
			if(!p_iputl(itemsbuf[i].misc1,f))
			{
				new_return(37);
			}
			
			if(!p_iputl(itemsbuf[i].misc2,f))
			{
				new_return(38);
			}
			
			for(auto q = 0; q < 2; ++q)
			{
				if(!p_iputw(itemsbuf[i].cost_amount[q],f))
				{
					new_return(39);
				}
			}
			
			if(!p_iputl(itemsbuf[i].misc3,f))
			{
				new_return(40);
			}
			
			if(!p_iputl(itemsbuf[i].misc4,f))
			{
				new_return(41);
			}
			
			if(!p_iputl(itemsbuf[i].misc5,f))
			{
				new_return(42);
			}
			
			if(!p_iputl(itemsbuf[i].misc6,f))
			{
				new_return(43);
			}
			
			if(!p_iputl(itemsbuf[i].misc7,f))
			{
				new_return(44);
			}
			
			if(!p_iputl(itemsbuf[i].misc8,f))
			{
				new_return(45);
			}
			
			if(!p_iputl(itemsbuf[i].misc9,f))
			{
				new_return(46);
			}
			
			if(!p_iputl(itemsbuf[i].misc10,f))
			{
				new_return(47);
			}
			
			if(!p_putc(itemsbuf[i].usesound,f))
			{
				new_return(48);
			}
			
			if(!p_putc(itemsbuf[i].usesound2,f))
			{
				new_return(48);
			}
		
		//New itemdata vars -Z
		//! version 27
		
		if(!p_putc(itemsbuf[i].useweapon,f))
			{
				new_return(49);
			}
		if(!p_putc(itemsbuf[i].usedefence,f))
			{
				new_return(50);
			}
		if(!p_iputl(itemsbuf[i].weaprange,f))
			{
				new_return(51);
			}
		if(!p_iputl(itemsbuf[i].weapduration,f))
			{
				new_return(52);
			}
		for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) {
			if(!p_iputl(itemsbuf[i].weap_pattern[q],f))
			{
			new_return(53);
			}
		}
		//version 28
		if(!p_iputl(itemsbuf[i].duplicates,f))
		{
			new_return(54);
		}
		for ( int32_t q = 0; q < INITIAL_D; q++ )
		{
			if(!p_iputl(itemsbuf[i].weap_initiald[q],f))
			{
				new_return(55);
			}
		}
		for ( int32_t q = 0; q < INITIAL_A; q++ )
		{
			if(!p_putc(itemsbuf[i].weap_initiala[q],f))
			{
				new_return(56);
			}
		}

		if(!p_putc(itemsbuf[i].drawlayer,f))
		{
			new_return(57);
		}


		if(!p_iputl(itemsbuf[i].hxofs,f))
		{
			new_return(58);
		}
		if(!p_iputl(itemsbuf[i].hyofs,f))
		{
			new_return(59);
		}
		if(!p_iputl(itemsbuf[i].hxsz,f))
		{
			new_return(60);
		}
		if(!p_iputl(itemsbuf[i].hysz,f))
		{
			new_return(61);
		}
		if(!p_iputl(itemsbuf[i].hzsz,f))
		{
			new_return(62);
		}
		if(!p_iputl(itemsbuf[i].xofs,f))
		{
			new_return(63);
		}
		if(!p_iputl(itemsbuf[i].yofs,f))
		{
			new_return(64);
		}
		if(!p_iputl(itemsbuf[i].weap_hxofs,f))
		{
			new_return(65);
		}
		if(!p_iputl(itemsbuf[i].weap_hyofs,f))
		{
			new_return(66);
		}
		if(!p_iputl(itemsbuf[i].weap_hxsz,f))
		{
			new_return(67);
		}
		if(!p_iputl(itemsbuf[i].weap_hysz,f))
		{
			new_return(68);
		}
		if(!p_iputl(itemsbuf[i].weap_hzsz,f))
		{
			new_return(69);
		}
		if(!p_iputl(itemsbuf[i].weap_xofs,f))
		{
			new_return(70);
		}
		if(!p_iputl(itemsbuf[i].weap_yofs,f))
		{
			new_return(71);
		}
		if(!p_iputw(itemsbuf[i].weaponscript,f))
		{
			new_return(72);
		}
		if(!p_iputl(itemsbuf[i].wpnsprite,f))
		{
			new_return(73);
		}
		
		for(auto q = 0; q < 2; ++q)
		{
			if(!p_iputl(itemsbuf[i].magiccosttimer[q],f))
			{
				new_return(74);
			}
		}
		if(!p_iputl(itemsbuf[i].overrideFLAGS,f))
		{
			new_return(75);
		}
		if(!p_iputl(itemsbuf[i].tilew,f))
		{
			new_return(76);
		}
		if(!p_iputl(itemsbuf[i].tileh,f))
		{
			new_return(77);
		}
		if(!p_iputl(itemsbuf[i].weapoverrideFLAGS,f))
		{
			new_return(78);
		}
		if(!p_iputl(itemsbuf[i].weap_tilew,f))
		{
			new_return(79);
		}
		if(!p_iputl(itemsbuf[i].weap_tileh,f))
		{
			new_return(80);
		}
		if(!p_iputl(itemsbuf[i].pickup,f))
		{
			new_return(81);
		}
		if(!p_iputw(itemsbuf[i].pstring,f))
		{
			new_return(82);
		}
		if(!p_iputw(itemsbuf[i].pickup_string_flags,f))
		{
			new_return(83);
		}
		
		for(auto q = 0; q < 2; ++q)
		{
			if(!p_putc(itemsbuf[i].cost_counter[q],f))
			{
				new_return(84);
			}
		}
		
		//InitD[] labels
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(itemsbuf[i].initD_label[q][w],f))
				{
					new_return(85);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(itemsbuf[i].weapon_initD_label[q][w],f))
				{
					new_return(86);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(itemsbuf[i].sprite_initD_label[q][w],f))
				{
					new_return(87);
				} 
			}
			if(!p_iputl(itemsbuf[i].sprite_initiald[q],f))
			{
				new_return(88);
			} 
		}
		for ( int32_t q = 0; q < 2; q++ )
		{
			if(!p_putc(itemsbuf[i].sprite_initiala[q],f))
			{
				new_return(89);
			} 
			
		}
		if(!p_iputw(itemsbuf[i].sprite_script,f))
		{
			new_return(90);
		} 
		if(!p_putc(itemsbuf[i].pickupflag,f))
		{
			new_return(91);
		} 
		return 1;
}

int32_t onCustomItems()
{
	ItemListerDialog().show();
	refresh(rMAP+rCOMBOS);
	return D_O_K;
}

static DIALOG wpndata_dlg[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
	{ jwin_win_proc,     55,   40,   210,  163,  vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL, NULL, NULL },
	{ d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
	{ d_cstile_proc,     198,  84,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL, NULL, NULL },
	{ jwin_button_proc,  90,   176,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
	{ jwin_button_proc,  170,  176,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	{ jwin_check_proc,   198,  109,   65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "Auto-flash", NULL, NULL },
	{ jwin_check_proc,   198,  120,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "2P Flash", NULL, NULL },
	{ jwin_check_proc,   198,  131,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "H-Flip", NULL, NULL },
	{ jwin_check_proc,   198,  142,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "V-Flip", NULL, NULL },
	{ jwin_text_proc,    61,   88,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Flash CSet:", NULL, NULL },
	//10
	{ jwin_text_proc,    61,   106,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Animation Frames:", NULL, NULL },
	{ jwin_text_proc,    61,   124,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Animation Speed:", NULL, NULL },
	{ jwin_text_proc,    61,   142,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Miscellaneous Type:", NULL, NULL },
	{ jwin_edit_proc,    160,   88,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    160,  106,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    160,  124,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,    160,  142,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_check_proc,   198,  153,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "Behind", NULL, NULL },
	{ jwin_edit_proc,     92,  65,   155,  16,   vc(12),  vc(1),  0,       0,          64,             0,       NULL, NULL, NULL },
	{ jwin_text_proc,     61,  69,   35,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Name:", NULL, NULL },
	{ d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,          0, (void *) onSnapshot, NULL, NULL },
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};



void edit_weapondata(int32_t index)
{
	call_sprite_dlg(index); return;
	//OLD CODE
	/*
	char frm[8], spd[8], fcs[8], typ[8];
	char name[64];
	char wpnnumstr[75];
	
	sprintf(wpnnumstr, "Sprite %d: %s", index, weapon_string[index]);
	wpndata_dlg[0].dp  = wpnnumstr;
	wpndata_dlg[0].dp2 = get_zc_font(font_lfont);
	wpndata_dlg[2].d1  = wpnsbuf[index].tile;
	wpndata_dlg[2].d2  = wpnsbuf[index].csets&15;
	
	for(int32_t i=0; i<4; i++)
		wpndata_dlg[i+5].flags = (wpnsbuf[index].misc&(1<<i)) ? D_SELECTED : 0;
		
	wpndata_dlg[17].flags = (wpnsbuf[index].misc & WF_BEHIND) ? D_SELECTED : 0;
	
	sprintf(fcs,"%d",wpnsbuf[index].csets>>4);
	sprintf(frm,"%d",wpnsbuf[index].frames);
	sprintf(spd,"%d",wpnsbuf[index].speed);
	sprintf(typ,"%d",wpnsbuf[index].type);
	wpndata_dlg[13].dp = fcs;
	wpndata_dlg[14].dp = frm;
	wpndata_dlg[15].dp = spd;
	wpndata_dlg[16].dp = typ;
	sprintf(name,"%s",weapon_string[index]);
	wpndata_dlg[18].dp = name;
	
	large_dialog(wpndata_dlg);
	
	int32_t ret;
	wpndata test;
	
	do
	{
		ret = do_zqdialog(wpndata_dlg,3);
		
		test.tile  = wpndata_dlg[2].d1;
		test.csets = wpndata_dlg[2].d2;
		
		test.misc  = 0;
		
		for(int32_t i=0; i<4; i++)
			if(wpndata_dlg[i+5].flags & D_SELECTED)
				test.misc |= 1<<i;
				
		test.misc |= (wpndata_dlg[17].flags & D_SELECTED) ? WF_BEHIND : 0;
		
		test.csets  |= (atoi(fcs)&15)<<4;
		test.frames = atoi(frm);
		test.speed  = atoi(spd);
		test.type   = atoi(typ);
		test.script = 0; // Not used yet
	}
	while(ret==17);
	
	if(ret==3)
	{
		strcpy(weapon_string[index],name);
		wpnsbuf[index] = test;
		saved = false;
	}*/
	
}

int32_t onCustomWpns()
{
	/*
	  char *hold = item_string[0];
	  item_string[0] = "rupee (1)";
	  */
	
	build_biw_list();
	
	int32_t index = select_weapon("Select Weapon",biw[0].i);
	
	while(index >= 0)
	{
		edit_weapondata(index);
		index = select_weapon("Select Weapon",index);
	}
	
	refresh(rMAP+rCOMBOS);
	return D_O_K;
}

int32_t onMiscSprites()
{
	MiscSprsDialog(QMisc.sprites, 20, [](int32_t* newsprs)
	{
		saved = false;
		for(auto q = 0; q < sprMAX; ++q)
			QMisc.sprites[q] = byte(newsprs[q]);
	}).show();
	return D_O_K;
}

int32_t onMiscSFX()
{
	MiscSFXDialog(QMisc.miscsfx, 20, [](int32_t* newsfx)
	{
		saved = false;
		for(auto q = 0; q < sfxMAX; ++q)
			QMisc.miscsfx[q] = byte(newsfx[q]);
	}).show();
	return D_O_K;
}

static int32_t enedata_data_list[] =
{
	//2,3,4 --moved to EOL  as 247,248,249, to make room for tabs. -Z
	247,248,249,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
	31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,138,139,140,141,143,-1
};

static int32_t enedata_data2_list[] =
{
	54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,179,180,181,182,183,184,187,188,189,190, 288, 289, 296, 297,-1
};

static int32_t enedata_data3_list[] =
{
	// d_dummy_proc entries still must be somewhere on the editor.
	// They will be hidden, but if ther are not referenced, this causes objects
	// to refuse to click!
	// This was the cause of flags not clicking properly and Enemy Editor UI elements not responding to mouse input. -Z
	5,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,
	286,287,290,291,292,293,294,295,298,299,300,301,302,303,304,305,306,307,310,311,312,313,314,315,
	-1
};

static int32_t enedata_data4_list[] =
{
	308, 309, 316,317,318,319,320,321,322,323,324,325,326,327,328,329,330,331,332,333,
	-1
};

static int32_t enedata_flags_list[] =
{
	74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,98,99,100,101,102,103,104,105,106,120,121,400,-1
};

static int32_t enedata_flags2_list[] =
{
	90,91,92,93,94,95,96,97,401,402,403,404,405,406,407,408,409,410,411,412,413,414,-1
};

static int32_t enedata_editorflags_list[]=
{
	254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,
	-1
};

static int32_t enedata_flags3_list[] =
{
	107,108,109,110,111,112,113,114,115,116,117,118,119,185,186,399,-1
}; //REMEMBER: If you undummy anything from 400-414, make sure to remove it from enedata_flags2_list! That list gets everything in it disabled.

static int32_t enedata_defense_list[] =
{
	144,145,146,147,148,149,150,151,152,161,162,163,164,165,166,167,168,169,178,-1
};

static int32_t enedata_defense2_list[] =
{
	153,154,155,156,157,158,159,160,170,171,172,173,174,175,176,177,191,192,415,416,-1
};

static int32_t enedata_defense3_list[] =
{
	193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,-1
};

static int32_t enedata_spritesize_list[] =
{
	213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,237,238,239,240,241,242,243,244,245,246,-1
};

static int32_t enemy_defence_tabs_list[] =
{
	2,-1
};

static int32_t enemy_basic_tabs_list[] =
{
	3,-1
};

static int32_t enemy_flags_tabs_list[] =
{
	4,-1
};


static int32_t enemy_attributes_tabs_list[] =
{
	5,-1
};

static int32_t enemy_graphics_tabs_list[] =
{
	6,-1
};

static int32_t enemy_movement_tabs_list[] =
{
	250,-1
};

static int32_t enemy_script_tabs_list[] =
{
	251,-1
};


static int32_t enemy_scripts_list[] =
{
	
	334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,351,352,
	377,378,379,380,381,382,383,384,
	426,427,428,429,430,431,432,433,
	-1
};

static int32_t enemy_weapon_scripts_list[] =
{
	353,354,355,356,357,358,359,360,
	361,362,363,364,365,366,367,368,
	369,370,
	385,386,387,388,389,390,391,392,
	434,435,436,437,438,439,440,441,
	-1
};
static int32_t enemy_moveflag_list[] =
{
	371, 372, 373, 374, 375, 417, 418, 419,
	420, 421, 422, 423, 424, 425,
	-1
};
static int32_t enemy_movement_list[] =
{
	-1
};

static int32_t enemy_gfx_sprites_list[] =
{
	235,236,393,394,395,396,397,398,
	-1
};


static TABPANEL enemy_graphics_tabs[] =
{
	{ (char *)"Sprites",      D_SELECTED,      enemy_gfx_sprites_list,  0, NULL },
	{ NULL,                   0,               NULL,                    0, NULL }
};



/*



static int32_t enemy_sounds_tabs_list[] =
{
	250,-1
};


static int32_t enemy_attacks_tabs_list[] =
{
	5,-1
};

static int32_t enemy_sfx_sounds_list[] =
{
	-1
};

static int32_t enemy_sfx_bgsounds_list[] =
{
	-1
};

static int32_t enemy_attacks_list[] =
{
	-1
};
static int32_t enemy_attacks_Wmove_list[] =
{
	-1
};



*/





/*

static TABPANEL enemy_sounds_tabs[] =
{
   // { (char *)"Sound Efects",	 D_SELECTED,               enemy_sfx_sounds_list,   0, NULL },
   // { (char *)"Background Sounds",	 D_SELECTED,               enemy_sfx_bgsounds_list,   0, NULL },
	{ NULL,                   0,               NULL,                  0, NULL }
};



static TABPANEL enemy_attacks_tabs[] =
{
  //  { (char *)"Attacks",	 D_SELECTED,               enemy_attacks_list,   0, NULL },
  //  { (char *)"Weapon Movement",	 D_SELECTED,               enemy_attacks_Wmove_list,   0, NULL },
	{ NULL,                   0,               NULL,                  0, NULL }
};


*/


static int32_t enedata_defense_ranged_list[] =
{
	//ranged 1
	//brang,	arrow, 		magic, 		fire, 		byrna, 		whistle
	144, 161, 	147, 164, 	150, 167,	148, 165, 	160, 177,	191, 192,
	-1
};


//193 == text; 203 == lister
static int32_t enedata_defense_script_list[] =
{
	193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,-1
};

static int32_t enedata_defense_melee_list[] =
{
	//melee
	//sword, 	bomb, 		sbomb, 		wand, 
	153, 170, 	145, 162,	146, 163, 	149, 166,
	//hammer, 	boots, 		hookshot
	152, 169, 	159, 176, 	151, 168, 
	//set all
	178, 
};

static int32_t enedata_defense_reflected_list[] =
{
	//reflected
	//ref beam, 	ref magic, 	ref fireball, 	refl rock, 
	155, 172,	156, 173,	157, 174,	158, 175,
	-1
};

static TABPANEL enemy_movement_tabs[] =
{
	{ (char *)"Move Flags",	     D_SELECTED,               enemy_moveflag_list,   0, NULL },
	{ (char *)"Enemy Movement",	          0,               enemy_movement_list,   0, NULL },
	{ NULL,                   0,               NULL,                  0, NULL }
};

static TABPANEL enemy_script_tabs[] =
{
	{ (char *)"Action Script",	 D_SELECTED,               enemy_scripts_list,   0, NULL },
	{ (char *)"Weapon Script",	 0,               enemy_weapon_scripts_list,   0, NULL },
	{ NULL,                   0,               NULL,                  0, NULL }
};

static TABPANEL enemy_defence_tabs[] =
{
	
	{ (char *)"Defs 1",	 D_SELECTED,               enedata_defense_list,   0, NULL },
	{ (char *)"Defs 2",	 0,               enedata_defense2_list,   0, NULL },
	{ (char *)"Script",	 0,               enedata_defense3_list,   0, NULL },
	
	/*
	{ (char *)"Melee",	 D_SELECTED,               enedata_defense_melee_list,   0, NULL },
	{ (char *)"Ranged",	 0,               enedata_defense_ranged_list,   0, NULL },
	{ (char *)"Reflected",	 0,               enedata_defense_reflected_list,   0, NULL },
	{ (char *)"Script",	 0,               enedata_defense_script_list,   0, NULL },
	*/
	{ NULL,                   0,               NULL,                  0, NULL }
};

static TABPANEL enemy_flags_tabs[] =
{
	{ (char *)"Basic Flags",	 D_SELECTED,               enedata_flags_list,   0, NULL },
	{ (char *)"Behaviour",	 0,               enedata_editorflags_list,   0, NULL },
	{ (char *)"Spawn Flags",	 0,               enedata_flags3_list,   0, NULL },
	{ NULL,                   0,               NULL,                  0, NULL }
};

static TABPANEL enemy_basic_tabs[] =
{
	{ (char *)"Data 1",	 D_SELECTED,               enedata_data_list,   0, NULL },
	{ (char *)"Data 2",	 0,               enedata_data2_list,   0, NULL },
	{ (char *)"Data 3",	 0,               enedata_data3_list,   0, NULL },
	{ (char *)"Data 4",	 0,               enedata_data4_list,   0, NULL },
	{ (char *)"Size",	 0,               enedata_spritesize_list,   0, NULL },
   // { (char *)"Graphics",	 D_SELECTED,               enemy_gfx_sprites_list,   0, NULL },
   // { (char *)"Sound Efects",	 D_SELECTED,               enemy_sfx_sounds_list,   0, NULL },
	{ NULL,                   0,               NULL,                  0, NULL }
};

static TABPANEL enedata_tabs[] =
{
   // { (char *)"Data 1",       D_SELECTED,      enedata_data_list,     0, NULL },
   // { (char *)"Data 2",       0,               enedata_data2_list,    0, NULL },
	{ (char *)"Basic",       D_SELECTED,               enemy_basic_tabs_list,    0, NULL },
   // { (char *)"Misc. Flags",	 0,               enedata_flags_list,    0, NULL },
   // { (char *)"Flags 2",	    0,               enedata_flags2_list,   0, NULL },
	{ (char *)"Defenses",	 0,               enemy_defence_tabs_list,   0, NULL },
   // { (char *)"Defs 1",	 0,               enedata_defense_list,   0, NULL },
   // { (char *)"Defs 2",	 0,               enedata_defense2_list,   0, NULL },
   // { (char *)"Defs 3",	 0,               enedata_defense3_list,   0, NULL },
	{ (char *)"Flags",	 0,               enemy_flags_tabs_list,   0, NULL },
   // { (char *)"Spawn Flags",	 0,               enedata_flags3_list,   0, NULL },
  // { (char *)"Size",	 0,               enedata_spritesize_list,   0, NULL },
  //Unimplemented at present. -Z
	{ (char *)"Movement",	 0,               enemy_movement_tabs_list,   0, NULL },
   // { (char *)"Attacks",	 0,               enemy_attacks_tabs_list,   0, NULL },
   // { (char *)"Sound Effects",	 0,               enemy_sounds_tabs_list,   0, NULL },
	{ (char *)"Graphics",	 0,               enemy_graphics_tabs_list,   0, NULL },
	{ (char *)"Scripts",	 0,               enemy_script_tabs_list,   0, NULL },
	{ NULL,                   0,               NULL,                  0, NULL }
};


list_data_struct bief[eeMAX];
int32_t bief_cnt=-1;

void build_bief_list()
{
	int32_t start=bief_cnt=0;
	
	for(int32_t i=start; i<eeMAX; i++)
	{
	//Load enemy names from the module
		//if(moduledata.enem_type_names[i][0]!='-')
	if (moduledata.enem_type_names[i][0]!=0)
	{
		if(moduledata.enem_type_names[i][0]!='-')
		{
			//load these from the module
		   // bief[bief_cnt].s, = (char *)moduledata.enem_type_names[i]); //, (char *)enetype_string[i]);
			//bief[bief_cnt].s = (char *)enetype_string[i];
			bief[bief_cnt].s = (char *)moduledata.enem_type_names[i];
			bief[bief_cnt].i = i;
			++bief_cnt;
		}
	}
	else //not set in the module file, so use the default
	{
		if(enetype_string[i][0]!='-')
		{
			//load these from the module
		   // bief[bief_cnt].s, = (char *)moduledata.enem_type_names[i]); //, (char *)enetype_string[i]);
			//bief[bief_cnt].s = (char *)enetype_string[i];
			bief[bief_cnt].s = (char *)enetype_string[i];
			bief[bief_cnt].i = i;
			++bief_cnt;
		}
		
	}
		
	}
	
	// No alphabetic sorting for this list
	for(int32_t j=start+1; j<bief_cnt-1; j++)
	{
		if(!strcmp(bief[j].s,"(None)"))
		{
			for(int32_t i=j; i>0; i--)
				zc_swap(bief[i],bief[i-1]);
				
			break;
		}
	}
}

const char *enetypelist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = bief_cnt;
		return NULL;
	}
	
	return bief[index].s;
}

list_data_struct biea[aMAX];
int32_t biea_cnt=-1;

void build_biea_list()
{
	int32_t start=biea_cnt=0;
	
	for(int32_t i=start; i<aMAX; i++)
	{
		
	if ( moduledata.enem_anim_type_names[1][0] != 0 )
	{
		if(eneanim_string[i][0]!='-')
		{
			biea[biea_cnt].s = (char *)moduledata.enem_anim_type_names[i];
			biea[biea_cnt].i = i;
			++biea_cnt;
		}
	}
	else
	{
		if(eneanim_string[i][0]!='-')
		{
			biea[biea_cnt].s = (char *)eneanim_string[i];
			biea[biea_cnt].i = i;
			++biea_cnt;
		}
	}
	}
	
	for(int32_t i=start; i<biea_cnt-1; i++)
		for(int32_t j=i+1; j<biea_cnt; j++)
			if(stricmp(biea[i].s,biea[j].s)>0 && strcmp(biea[j].s,""))
				zc_swap(biea[i],biea[j]);
}


const char *eneanimlist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = biea_cnt;
		return NULL;
	}
	
	return biea[index].s;
}

const char *itemsetlist(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		bound(index,0,count_item_drop_sets()-1);
		return item_drop_sets[index].name;
		//  return itemset_string[index];
	}
	
	*list_size=count_item_drop_sets();
	return NULL;
}

list_data_struct biew[MAXWPNS];
int32_t biew_cnt=-1;

char temp_custom_ew_strings[10][40];

static int32_t enemy_weapon_types[]=
{
	128, ewFireball,ewArrow,ewBrang,ewSword,
	ewRock,ewMagic,ewBomb,ewSBomb,
	//137
	ewLitBomb,ewLitSBomb,ewFireTrail,ewFlame,
	ewWind,ewFlame2,ewFlame2Trail,
	//145
	ewIce,ewFireball2
	
};

static int32_t enemy_script_weapon_types[]=
{
	wScript1, wScript2, wScript3, wScript4,
	//35
	wScript5, wScript6, wScript7, wScript8,
	//39
	wScript9, wScript10
	
};

void build_biew_list()
{
	biew_cnt=0;

	memset(temp_custom_ew_strings, 0, sizeof(temp_custom_ew_strings));
	
	for(int32_t i=0; i<wMax-wEnemyWeapons; i++)
	{
		//if(eweapon_string[i][0]!='-')
		if(moduledata.enemy_weapon_names[i][0]!='-')
		{
			//biew[biew_cnt].s = (char *)eweapon_string[i];
			biew[biew_cnt].s = (char *)moduledata.enemy_weapon_names[i];
			biew[biew_cnt].i = enemy_weapon_types[i];
			++biew_cnt;
		}
	}
	for(int32_t i = 0; i < 10; i++)
	{
		biew[biew_cnt].s = (char *)moduledata.enemy_scriptweaponweapon_names[i];
	biew[biew_cnt].i = enemy_script_weapon_types[i];
	++biew_cnt;
	}
	al_trace("biew_cnt is: %d\n", biew_cnt);
	for ( int32_t i = 0; i < biew_cnt; i++ )
	{
	al_trace("biew[%d] id is (%d) and string is (%s)\n", i, biew[i].i, biew[i].s);
		
	}
	
}

const char *eweaponlist(int32_t index, int32_t *list_size)
{
	if(biew_cnt==-1)
		build_biew_list();
		
	if(index>=0)
	{
		bound(index,0,biew_cnt-1);
		return biew[index].s;
	}
	
	*list_size=biew_cnt;
	return NULL;
}


//
// Enemy Misc. Attribute label swapping device
//
struct EnemyNameInfo
{
	int32_t family;
	char const* misc[10];
	void* list[32];
	char *flags[16];
	char *attributes[32];
  
};

const char *walkmisc1list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		bound(index,0,e1tLAST-1);
		return walkmisc1_string[index];
	}
	
	*list_size = e1tLAST;
	return NULL;
}

const char *walkmisc2list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		bound(index,0,e2tTRIBBLE);
		return walkmisc2_string[index];
	}
	
	*list_size = e2tTRIBBLE+1;
	return NULL;
}

const char *walkmisc7list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		bound(index,0,e7tEATHURT);
		return walkmisc7_string[index];
	}
	
	*list_size = e7tEATHURT+1;
	return NULL;
}

const char *walkmisc9list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		bound(index,0,e9tARMOS);
		//return walkmisc9_string[index];
		return (char *)moduledata.walkmisc9_names[index];
	}
	
	*list_size = e9tARMOS+1;
	return NULL;
}

const char *gleeokmisc3list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "Breath" : "1 Shot");
	}
	
	*list_size = 2;
	return NULL;
}

const char *gohmamisc1list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index==2 ? "Breath" : index==1 ? "3 Shots" : "1 Shot");
	}
	
	*list_size = 3;
	return NULL;
}

const char *manhandlamisc2list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "2x2 (8 Heads)" : "1x1 (4 Heads)");
	}
	
	*list_size = 2;
	return NULL;
}

const char *aquamisc1list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "Left (Facing Right)" : "Right (Facing Left)");
	}
	
	*list_size = 2;
	return NULL;
}

const char *patramisc4list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "Oval" : "Big Circle");
	}
	
	*list_size = 2;
	return NULL;
}

const char *patramisc5list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index==3 ? "Inner + Center" : index==2 ? "Inner Eyes" : index==1 ? "Center Eye" : "None");
	}
	
	*list_size = 4;
	return NULL;
}

const char *patramisc10list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "2x2 Patra" : "1x1 Patra");
	}
	
	*list_size = 2;
	return NULL;
}

const char *patramisc20list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		switch(index)
		{
			case 4:
				return "Stream";
			case 3:
				return "Ring";
			case 2:
				return "Barrage";
			case 1:
				return "Random (Single)";
			default:
				return "Random (Any)";
		}
	}
	
	*list_size = 5;
	return NULL;
}

const char *patramisc22list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		switch(index)
		{
			case 2:
				return "Expand + Warning";
			case 1:
				return "When Expanding";
			default:
				return "Never";
		}
	}
	
	*list_size = 3;
	return NULL;
}

const char *patramisc25list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		switch(index)
		{
			case 3:
				return "When behind 1 layer";
			case 2:
				return "When Invulnerable";
			case 1:
				return "When Vulnerable";
			default:
				return "Always";
		}
	}
	
	*list_size = 4;
	return NULL;
}

const char *patramisc26list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		switch(index)
		{
			case 2:
				return "Without Outer Ring";
			case 1:
				return "With Outer Ring";
			default:
				return "Always";
		}
	}
	
	*list_size = 3;
	return NULL;
}

const char *patramisc28list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		bound(index,0,patratLAST-1);
		return patramisc28_string[index];
	}
	
	*list_size = patratLAST;
	return NULL;
}

const char *dodongomisc10list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "BS-Zelda" : "NES");
	}
	
	*list_size = 2;
	return NULL;
}

const char *digdoggermisc10list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "Kid" : "Digdogger");
	}
	
	*list_size = 2;
	return NULL;
}

const char *walkerspawnlist(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index==2 ? "Instant" : index==1 ? "Flicker" : "Puff");
	}
	
	*list_size = 3;
	return NULL;
}

const char *wizzrobemisc1list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "Phase" : "Teleport");
	}
	
	*list_size = 2;
	return NULL;
}

const char *wizzrobemisc2list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index==3 ? "Summon (Layer)" : index==2 ? "Summon" : index==1 ? "8 Shots" : "1 Shot");
	}
	
	*list_size = 4;
	return NULL;
}

const char *keesemisc1list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index > 1 ? "Keese (Fast)" : index > 0 ? "Bat" : "Keese");
	}
	
	*list_size = 3;
	return NULL;
}

const char *keesemisc2list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "Tribble" : "Normal");
	}
	
	*list_size = 2;
	return NULL;
}

const char *trapmisc2list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "Constant" : "Line Of Sight");
	}
	
	*list_size = 2;
	return NULL;
}

const char *trapmisc1list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index==2 ? "Vertical" : index==1 ? "Horizontal" : "4-Way");
	}
	
	*list_size = 3;
	return NULL;
}

const char *leevermisc1list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		if(index==0)
			return "Player's path";
		else if(index==1)
			return "In place";
		else if(index==2)
			return "Player's path + second";
	}
	
	*list_size = 3;
	return NULL;
}

const char *rockmisc1list(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "2x2" : "1x1");
	}
	
	*list_size = 2;
	return NULL;
}

// 0: no, 1: yes
const char *yesnomisclist(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "Yes" : "No");
	}
	
	*list_size = 2;
	return NULL;
}

// 0: yes, 1: no
const char *noyesmisclist(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return (index ? "No" : "Yes");
	}
	
	*list_size = 2;
	return NULL;
}

static ListData walkmisc1_list(walkmisc1list, &a4fonts[font_lfont_l]);
static ListData walkmisc2_list(walkmisc2list, &a4fonts[font_lfont_l]);
static ListData walkmisc7_list(walkmisc7list, &a4fonts[font_lfont_l]);
static ListData walkmisc9_list(walkmisc9list, &a4fonts[font_lfont_l]);

static ListData gleeokmisc3_list(gleeokmisc3list, &a4fonts[font_lfont_l]);
static ListData gohmamisc1_list(gohmamisc1list, &a4fonts[font_lfont_l]);
static ListData manhandlamisc2_list(manhandlamisc2list, &a4fonts[font_lfont_l]);
static ListData aquamisc1_list(aquamisc1list, &a4fonts[font_lfont_l]);

static ListData patramisc4_list(patramisc4list, &a4fonts[font_lfont_l]);
static ListData patramisc5_list(patramisc5list, &a4fonts[font_lfont_l]);
static ListData patramisc10_list(patramisc10list, &a4fonts[font_lfont_l]);
static ListData patramisc20_list(patramisc20list, &a4fonts[font_lfont_l]);
static ListData patramisc22_list(patramisc22list, &a4fonts[font_lfont_l]);
static ListData patramisc25_list(patramisc25list, &a4fonts[font_lfont_l]);
static ListData patramisc26_list(patramisc26list, &a4fonts[font_lfont_l]);
static ListData patramisc28_list(patramisc28list, &a4fonts[font_lfont_l]);

static ListData dodongomisc10_list(dodongomisc10list, &a4fonts[font_lfont_l]);

static ListData keesemisc1_list(keesemisc1list, &a4fonts[font_lfont_l]);
static ListData keesemisc2_list(keesemisc2list, &a4fonts[font_lfont_l]);

static ListData digdoggermisc10_list(digdoggermisc10list, &a4fonts[font_lfont_l]);

static ListData wizzrobemisc1_list(wizzrobemisc1list, &a4fonts[font_lfont_l]);
static ListData wizzrobemisc2_list(wizzrobemisc2list, &a4fonts[font_lfont_l]);

static ListData trapmisc1_list(trapmisc1list, &a4fonts[font_lfont_l]);
static ListData trapmisc2_list(trapmisc2list, &a4fonts[font_lfont_l]);

static ListData leevermisc1_list(leevermisc1list, &a4fonts[font_lfont_l]);
static ListData rockmisc1_list(rockmisc1list, &a4fonts[font_lfont_l]);

static ListData yesnomisc_list(yesnomisclist, &a4fonts[font_lfont_l]);
static ListData noyesmisc_list(noyesmisclist, &a4fonts[font_lfont_l]);

static EnemyNameInfo enameinf[]=
{
	{
		//"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility"
		eeWALK, { "Shot Type:", "Death Type:", "Death Attr. 1:", "Death Attr. 2:", "Death Attr. 3:", "Extra Shots:", "Touch Effects:", "Effect Strength:", "Walk Style:", "Walk Attr.:" },
		{ (void*)&walkmisc1_list, (void*)&walkmisc2_list, NULL, NULL, NULL, NULL, (void*)&walkmisc7_list, NULL, (void*)&walkmisc9_list, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",
		(char *)"Draw Invisible as Cloaked",(char *)"Split in Place",(char *)"BFlags[5]:",(char *)"BFlags[6]:",(char *)"BFlags[7]:",
		(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",
		(char *)"Fast Drawing",(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}
	},
	{
		eeGLEEOK, { "Heads:", "Head HP:", "Shot Type:", NULL, "Neck Segments:", "Neck Offset 1:", "Neck Offset 2:", "Head Offset:", "Fly Head Offset:", NULL },
		{ NULL, NULL, (void*)&gleeokmisc3_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"Obeys Spawn Points",(char *)"Center Spawn X Point",(char *)"Heads Block Projectiles",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeDIG, { "Enemy 1 ID:", "Enemy 2 ID:", "Enemy 3 ID:", "Enemy 4 ID:", "Enemy 1 Qty:", "Enemy 2 Qty:", "Enemy 3 Qty:", "Enemy 4 Qty:", "Unused:", "Type:" },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&digdoggermisc10_list, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eePATRA, { "Outer Eyes:", "Inner Eyes:", "Eyes' HP:", "Eye Movement:", "Shooters:", "Pattern Odds:", "Pattern Cycles:", "Eye Offset:", "Eye CSet:", "Type:" },
		{ NULL, NULL, NULL, (void*)&patramisc4_list, (void*)&patramisc5_list, NULL, NULL, NULL, NULL, (void*)&patramisc10_list, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&patramisc20_list, NULL,  //21
		(void*)&patramisc22_list, NULL, NULL, (void*)&patramisc25_list, (void*)&patramisc26_list, NULL, (void*)&patramisc28_list, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Has Firing Animation",(char *)"Draw Invisible as Cloaked",(char *)"Obeys Spawn Points",(char *)"Slow down when Firing",(char *)"Don't attack when expanding",
		(char *)"Don't expand when spawned",(char *)"Pattern Odds reset when Expanding",(char *)"Don't 'expand' without orbiters",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Shot chance (1/N per frame):",(char *)"Firing cooldown:",(char *)"Inner Eyes Firing Pattern:",
		(char *)"Warning Spins:",(char *)"Stays still:",(char *)"Outer Ring Loss Speed Boost:",(char *)"Inner Ring Loss Speed Boost:",
		(char *)"Can Fire:",(char *)"Can Expand:",(char *)"Inner Eye HP:",(char *)"Center Eye Firing Pattern:",
		(char *)"Outer Eye Radius:",(char *)"Inner Eye Radius:",(char *)"Outer Eye Expand Radius:",(char *)"Inner Eye Expand Radius:",
	}	
	},
	{
		eePROJECTILE, { "Shot Type:",  NULL, "Shot Attr. 1:", "Shot Attr. 2:", NULL, NULL, NULL, NULL, NULL, NULL  },
		{ (void*)&walkmisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeGHOMA, { "Shot Type:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ (void*)&gohmamisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"Obeys Spawn Points",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeAQUA, { "Side:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ (void*)&aquamisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"Obeys Spawn Points",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeMANHAN, { "Frame rate:",  "Size:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, (void*)&manhandlamisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"Each segment drops items",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeLANM, { "Segments:",  "Segment Lag:", "Item per segment:", NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"Obeys Spawn Points",(char *)"Each segment drops items",
		(char *)"BFlags[6]:",(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",
		(char *)"Fast Drawing",(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeMOLD, { "Segments:",  "Item per segment:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"Obeys Spawn Points",(char *)"Each segment drops items",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeWIZZ, { "Walk Style:",  "Shot Type:", "Shot Attr. 1:", "Solid Combos OK:", "Teleport Delay:", NULL, NULL, NULL, NULL, NULL  },
		{ (void*)&wizzrobemisc1_list, (void*)&wizzrobemisc2_list, NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"Old Windrobe teleport",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeDONGO,{ NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Type :"  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&dodongomisc10_list, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeKEESE, { "Walk Style:",  "Death Type:", "Enemy ID:", NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ (void*)&keesemisc1_list, (void*)&keesemisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Landing Chance (1/N):",(char *)"Landing Cooldown:",(char *)"Halt Duration",
		(char *)"Acceleration Frame Interval:",(char *)"Acceleration Step Modifier:",(char *)"Spawn Step:",(char *)"Tribble Timer:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeTEK,  { "1/n jump start:",  "1/n jump cont.:", "Jump Z velocity:", NULL, NULL, NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeLEV,  { "Emerge style:",  "Submerged CSet:", "Emerging step:", NULL, NULL, NULL, NULL, NULL, NULL, NULL },
		{ (void*)&leevermisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeWALLM,{ "Fixed distance:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
		{ (void*)&noyesmisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeTRAP, { "Direction:",  "Move Style:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ (void*)&trapmisc1_list, (void*)&trapmisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeROCK, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Rock size:" },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&rockmisc1_list, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeNONE, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Boss Death Trigger:"  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&yesnomisc_list, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeGHINI, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Misc Attr. 15:",(char *)"Halt Duration",
		(char *)"Acceleration Factor",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eePEAHAT, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Misc Attr. 15:",(char *)"Halt Duration",
		(char *)"Acceleration Factor",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeZORA, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"Lava Only",(char *)"Lava and Water",(char *)"Shallow Water",
		(char *)"Doesn't regenerate health",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSPINTILE, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFIRE, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeOTHER, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT01, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT02, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT03, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT04, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT05, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT06, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT07, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT08, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT09, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT10, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT11, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT12, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT13, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT14, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT15, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT16, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT17, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT18, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT19, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeSCRIPT20, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY01, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY02, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY03, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY04, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY05, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY06, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY07, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY08, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY09, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeFFRIENDLY10, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeGUY,  { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Doesn't always return",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		eeGANON,  { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL
		, NULL, NULL  },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Visible if the player has an Level 2 (or higher) Amulet item.",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"Uses Full Hitbox",(char *)"BFlags[3]:",
		(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",
		(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",(char *)"Use Editor ",(char *)"When Not Invisible, Also Show Weapon Shots.",(char *)"Draw Cloaked When Not Invisible" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Death Shot Item ID",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	},
	{
		-1,		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //10
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  //21
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, //32
	{ (char *)"Enemy is Completely Invisible",(char *)"Item Specified in Attributes 13 Dispels Invisibility",(char *)"BFlags[2]:",(char *)"Draw Invisible as Cloaked",(char *)"BFlags[4]:",(char *)"BFlags[5]:",(char *)"BFlags[6]:",
		(char *)"BFlags[7]:",(char *)"BFlags[8]:",(char *)"BFlags[9]:",(char *)"BFlags[10]:",(char *)"Toggle Move Offscreen",(char *)"Fast Drawing",
		(char *)"Ignore Sideview Ladders/Platforms",(char *)"Move Off-Grid (WIP)",(char *)"Render Cloaked Instead of VISIBLE" },
	{ 	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,(char*)"Misc Attr. 11:",(char*)"Misc Attr. 12:",
		(char *)"This Item Dispels Invisibility:",(char *)"Misc Attr. 14:",(char *)"Transform Effect:",(char *)"Transform Into Enemy:",
		(char *)"Transformation Inv Time:",(char *)"Misc Attr. 18:",(char *)"Misc Attr. 19:",(char *)"Misc Attr. 20:",
		(char *)"Misc Attr. 21:",(char *)"Misc Attr. 22:",(char *)"Misc Attr. 23:",(char *)"Misc Attr. 24:",
		(char *)"Misc Attr. 25:",(char *)"Misc Attr. 26:",(char *)"Misc Attr. 27:",(char *)"Misc Attr. 28:",
		(char *)"Misc Attr. 29:",(char *)"Misc Attr. 30:",(char *)"Misc Attr. 31:",(char *)"Misc Attr. 32:",
	}	
	}
};

static std::map<int32_t, EnemyNameInfo *> *enamemap = NULL;

std::map<int32_t, EnemyNameInfo *> *getEnemyNameMap()
{
	if(enamemap == NULL)
	{
		enamemap = new std::map<int32_t, EnemyNameInfo *>();
		
		for(int32_t i=0;; i++)
		{
			EnemyNameInfo *inf = &enameinf[i];
			
			if(inf->family == -1)
				break;
				
			(*enamemap)[inf->family] = inf;
		}
	}
	
	return enamemap;
}

const char *npcscriptdroplist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = binpcs_cnt;
		return NULL;
	}
	
	return binpcs[index].first.c_str();
}
ListData npcscript_list(npcscriptdroplist, &font);

static ListData itemset_list(itemsetlist, &font);
static ListData eneanim_list(eneanimlist, &font);
static ListData enetype_list(enetypelist, &font);
static ListData eweapon_list(eweaponlist, &font);


const char *eweaponscriptdroplist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = bieweapons_cnt;
		return NULL;
	}
	
	return bieweapons[index].first.c_str();
}


//droplist like the dialog proc, naming scheme for this stuff is awful...
ListData eweaponscript_list(eweaponscriptdroplist, &a4fonts[font_pfont]);


static ListData walkerspawn_list(walkerspawnlist, &font);

static ListData sfx__list(sfxlist, &font);

/*
static DIALOG enedata_dlg[] =
{
	{  jwin_win_proc,            0,      0,    320,    240,    vc(14),                 vc(1),                   0,    D_EXIT,      0,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_tab_proc,            4,     24,    312,    192,    0,                      0,                       0,    0,           0,    0, (void *) enedata_tabs,                                          NULL, (void *)enedata_dlg  },
	//2
	{  d_ecstile_proc,          16,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   NULL                 },
	{  d_ecstile_proc,          52,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   NULL                 },
	{  d_ecstile_proc,          88,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   NULL                 },
	//5
	{  jwin_button_proc,        50,    220,     61,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "OK",                                                  NULL,   NULL                 },
	{  jwin_button_proc,       130,    220,     61,     16,    vc(14),                 vc(1),                  27,    D_EXIT,      0,    0, (void *) "Cancel",                                              NULL,   NULL                 },
	//7
	{  jwin_text_proc,          21,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Old",                                                 NULL,   NULL                 },
	{  jwin_text_proc,          44,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Special",                                          NULL,   NULL                 },
	{  jwin_text_proc,          90,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "New",                                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
	{  jwin_text_proc,           6,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
	//12
	{  jwin_text_proc,          42,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
	{  jwin_text_proc,          42,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
	{  jwin_text_proc,          78,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
	{  jwin_text_proc,          78,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
	//16
	{  jwin_text_proc,         114,     54,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Name:",                                               NULL,   NULL                 },
	{  jwin_text_proc,         114,     72,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HP:",                                                 NULL,   NULL                 },
	{  jwin_text_proc,         114,     90,     50,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Damage:",                                             NULL,   NULL                 },
	{  jwin_text_proc,         114,    108,     50,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W. Damage:",                                          NULL,   NULL                 },
	{  jwin_text_proc,         140,    126,     35,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hunger:",                                             NULL,   NULL                 },
	//21
	{  jwin_text_proc,         210,     72,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Random Rate:",                                        NULL,   NULL                 },
	{  jwin_text_proc,         210,     90,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Halt Rate:",                                          NULL,   NULL                 },
	{  jwin_text_proc,         210,    108,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Homing Factor:",                                      NULL,   NULL                 },
	{  jwin_text_proc,         210,    126,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Step Speed:",                                         NULL,   NULL                 },
	//25
	{  jwin_text_proc,           8,    126,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Weapon:",                                             NULL,   NULL                 },
	{  jwin_text_proc,           8,    144,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Type:",                                               NULL,   NULL                 },
	{  jwin_text_proc,           8,    162,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "O. Anim:",                                            NULL,   NULL                 },
	{  jwin_text_proc,           8,    180,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "E. Anim:",                                            NULL,   NULL                 },
	{  jwin_text_proc,           8,    198,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Item Set:",                                           NULL,   NULL                 },
	//30
	{  jwin_edit_proc,          16,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          16,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          52,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          52,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          88,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          88,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	//36
	{  jwin_edit_proc,         144,     50,    166,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         166,     68,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         166,     86,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         166,    104,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         186,    122,     20,     16,    vc(12),                 vc(1),                   0,    0,           1,    0,  NULL,                                                           NULL,   NULL                 },
	//41
	{  jwin_edit_proc,         280,     68,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,     86,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,    104,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,    122,     30,     16,    vc(12),                 vc(1),                   0,    0,           4,    0,  NULL,                                                           NULL,   NULL                 },
	//45
	{  jwin_droplist_proc,      51,    122,     85,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eweapon_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      51,    140,    110,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    D_EXIT,      0,    0, (void *) &enetype_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      51,    158,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eneanim_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      51,    176,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eneanim_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      51,    194,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &itemset_list,                                         NULL,   NULL                 },
	//50
	{  d_dummy_proc,           240,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0,  NULL,													       NULL,   NULL                 },
	{  jwin_text_proc,         240,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Pal CSet:",                                           NULL,   NULL                 },
	{  d_dummy_proc,           280,    140,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,    140,     30,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	//54
	{  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 1:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 2:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 3:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 4:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 5:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 6:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         160,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 7:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         160,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 8:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         160,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 9:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         160,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 10:",                                  NULL,   NULL                 },
	//64
	{  jwin_edit_proc,          86,     50,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,     68,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,     86,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,    104,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,    122,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,    140,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,     50,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,     68,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,     86,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,    104,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//74
	{  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Damaged By Power 0 Weapons",                          NULL,   NULL                 },
	{  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Invisible",                                        NULL,   NULL                 },
	{  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Never Returns After Death",						   NULL,   NULL                 },
	{  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Doesn't Count As Beatable Enemy",					   NULL,   NULL                 },
	{  d_dummy_proc,             6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",										       NULL,   NULL                 },
	{  d_dummy_proc,             6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	//82
	{  d_dummy_proc,             6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	//90
	{  d_dummy_proc,             6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	//98
	{  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded In Front",                                   NULL,   NULL                 },
	{  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded On Left",                                    NULL,   NULL                 },
	{  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded On Right",                                   NULL,   NULL                 },
	{  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded In Back",                                    NULL,   NULL                 },
	{  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Hammer Can Break Shield",                             NULL,   NULL                 },
	{  d_dummy_proc,             6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shield Is Mirrored",                                  NULL,   NULL                 },
	{  d_dummy_proc,             6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  jwin_check_proc,          6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can Only Be Seen By Lens Of Truth",                   NULL,   NULL                 },
	//106
	{  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Flashing",                                         NULL,   NULL                 },
	{  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Zora\" Screen Flag",                      NULL,   NULL                 },
	{  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Falling Rocks\" Screen Flag",             NULL,   NULL                 },
	{  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Corner Traps\" Screen Flag",              NULL,   NULL                 },
	{  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Horz Trap\" Combo Type/Flag",            NULL,   NULL                 },
	{  jwin_check_proc,          6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Vert Trap\" Combo Type/Flag",            NULL,   NULL                 },
	{  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"4-Way Trap\" Combo Type/Flag",           NULL,   NULL                 },
	{  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"LR Trap\" Combo Type/Flag",              NULL,   NULL                 },
	//114
	{  jwin_check_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"UD Trap\" Combo Type/Flag",              NULL,   NULL                 },
	{  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Middle Traps\" Screen Flag",              NULL,   NULL                 },
	{  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Statue Fire\" Screen Flag",               NULL,   NULL                 },
	{  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Armos\" Combo Type",                     NULL,   NULL                 },
	{  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Grave\" Combo Type",                     NULL,   NULL                 },
	{  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Ganon\" Room Type",                      NULL,   NULL                 },
	{  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Flickering",                                       NULL,   NULL                 },
	{  jwin_check_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Translucent",                                      NULL,   NULL                 },
	//122
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	//130
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	//138
	{  jwin_text_proc,         240,    162,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "F. Rate:",                                      NULL,   NULL                 },
	{  jwin_text_proc,         240,    180,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "F. Rate:",                                      NULL,   NULL                 },
	{  jwin_edit_proc,         280,    158,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,    176,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_timer_proc,             0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_check_proc,        165,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Use Pal CSet",                                        NULL,   NULL                 },
	//143 - note: these are defenses 0-16, 17 is at 191
	{  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Boomerang Defense:",                                  NULL,   NULL                 },
	{  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Bomb Defense:",                                       NULL,   NULL                 },
	{  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Super Bomb Defense:",                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Arrow Defense:",                                      NULL,   NULL                 },
	{  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Fire Defense:",                                       NULL,   NULL                 },
	{  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Wand Melee Defense:",                                       NULL,   NULL                 },
	{  jwin_text_proc,           6,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Magic Defense:",                                      NULL,   NULL                 },
	{  jwin_text_proc,           6,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hookshot Defense:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hammer Defense:",                                     NULL,   NULL                 },
	{  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Sword Defense:",                                      NULL,   NULL                 },
	{  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Sword Beam Defense:",                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Beam Defense:",                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Magic Defense:",                                NULL,   NULL                 },
	{  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Fireball Defense:",                             NULL,   NULL                 },
	{  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Rock Defense:",                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Stomp Boots Defense:",                                NULL,   NULL                 },
	{  jwin_text_proc,           6,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Byrna Beam Defense:",                              NULL,   NULL                 },
	//161
	{  jwin_droplist_proc,         126,   54-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   72-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   90-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  108-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  126-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  144-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  162-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  180-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  198-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   54-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   72-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   90-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  108-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  126-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  144-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  162-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  180-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_button_proc,           255,    54-4,     48,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "Set All",                                            NULL,   NULL                 },
	//179
	{  jwin_text_proc,           8,    162,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "BG Sound:",                                            NULL,   NULL                 },
	{  jwin_text_proc,           8,    180,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hit Sound:",                                           NULL,   NULL                 },
	{  jwin_text_proc,           8,    198,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Death Sound:",                                         NULL,   NULL                 },
	//182
	{  jwin_droplist_proc,      86,    158,     140,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
	{  jwin_droplist_proc,      86,    176,     140,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
	{  jwin_droplist_proc,      86,    194,     140,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
	{  jwin_text_proc,          6,     184,     95,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Spawn Animation:",                               NULL,   NULL                 },
	{  jwin_droplist_proc,      86,    180,     85,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &walkerspawn_list,                                     NULL,   NULL                 },
	{  jwin_text_proc,         160,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 11:",                                  NULL,   NULL                 },
	{  jwin_text_proc,         160,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 12:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         242,    122,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,    140,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	
	// 191
	{  jwin_text_proc,           6,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 198-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  NULL,                     0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 }
};
*/

static DIALOG enedata_dlg[] =
{
	{  jwin_win_proc,            0,      0,    320,    260,    vc(14),                 vc(1),                   0,    D_EXIT,      0,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_tab_proc,            4,     17,    312,    220,    0,                      0,                       0,    0,           0,    0, (void *) enedata_tabs,                                          NULL, (void *)enedata_dlg  },
	//2
	////tab procs must be listed prior to their elements!
	{  jwin_tab_proc,             4,     32,    312,    204,    0,                      0,                       0,    0,          0,          0, (void *) enemy_defence_tabs,     NULL, (void *) enedata_dlg   },
	{  jwin_tab_proc,             4,     32,    312,    204,    0,                      0,                       0,    0,          0,          0, (void *) enemy_basic_tabs,     NULL, (void *) enedata_dlg   },
	{  jwin_tab_proc,             4,     32,    312,    204,    0,                      0,                       0,    0,          0,          0, (void *) enemy_flags_tabs,     NULL, (void *) enedata_dlg   },
// {  d_ecstile_proc,          16,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   (void *)enedata_dlg },
	
	//5
	{  d_dummy_proc,             9904,     32,    1,    1,    0,                      0,                       0,    0,          0,          0, NULL,     NULL, NULL   },
	{  jwin_tab_proc,             4,     32,    312,    204,    0,                      0,                       0,    0,          0,          0, (void *) enemy_graphics_tabs,     NULL, (void *) enedata_dlg   },
 
	//Attributes Tab
	// {  jwin_button_proc,        86,    220,     61,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "OK",                                                  NULL,   NULL                 },
	// {  jwin_button_proc,       166,    220,     61,     16,    vc(14),                 vc(1),                  27,    D_EXIT,      0,    0, (void *) "Cancel",                                              NULL,   NULL                 },
	//7
	{  jwin_text_proc,          21,     51,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Old",                                                 NULL,   NULL                 },
	{  jwin_text_proc,          53,     51,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Special",                                          NULL,   NULL                 },
	{  jwin_text_proc,          92,     51,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "New",                                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
	{  jwin_text_proc,           6,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
	//12
	{  jwin_text_proc,          42,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
	{  jwin_text_proc,          42,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
	{  jwin_text_proc,          78,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
	{  jwin_text_proc,          78,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
	//16
	{  jwin_text_proc,         118,     53,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Name:",                                               NULL,   NULL                 },
	{  jwin_text_proc,         114,     72,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HP:",                                                 NULL,   NULL                 },
	{  jwin_text_proc,         114,     90,     50,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Damage:",                                             NULL,   NULL                 },
	{  jwin_text_proc,         114,    108,     50,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W. Damage:",                                          NULL,   NULL                 },
	{  jwin_text_proc,         140,    126,     35,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hunger:",                                             NULL,   NULL                 },
	//21
	{  jwin_text_proc,         210,     72,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Random Rate:",                                        NULL,   NULL                 },
	{  jwin_text_proc,         210,     90,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Halt Rate:",                                          NULL,   NULL                 },
	{  jwin_text_proc,         210,    108,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Homing Factor:",                                      NULL,   NULL                 },
	{  jwin_text_proc,         210,    126,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Step Speed:",                                         NULL,   NULL                 },
	//25
	{  jwin_text_proc,           8,    126,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Weapon:",                                             NULL,   NULL                 },
	{  jwin_text_proc,           8,    144,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Type:",                                               NULL,   NULL                 },
	{  jwin_text_proc,           8,    162,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "O. Anim:",                                            NULL,   NULL                 },
	{  jwin_text_proc,           8,    180,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "E. Anim:",                                            NULL,   NULL                 },
	{  jwin_text_proc,           8,    198,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Item Set:",                                           NULL,   NULL                 },
	//30
	{  jwin_edit_proc,          16,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          16,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          52,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          52,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          88,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          88,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	//36
	{  jwin_edit_proc,         144,     50,    166,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         166,     68,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         166,     86,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         166,    104,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         186,    122,     20,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	//41
	{  jwin_edit_proc,         280,     68,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,     86,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,    104,     30,     16,    vc(12),                 vc(1),                   0,    0,           4,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,    122,     30,     16,    vc(12),                 vc(1),                   0,    0,           4,    0,  NULL,                                                           NULL,   NULL                 },
	//45
	{  jwin_droplist_proc,      51,    122,     85,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eweapon_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      51,    140,    110,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    D_EXIT,      0,    0, (void *) &enetype_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      51,    158,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eneanim_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      51,    176,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eneanim_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      51,    194,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &itemset_list,                                         NULL,   NULL                 },
	//50
	{  d_dummy_proc,           240,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0,  NULL,													       NULL,   NULL                 },
	{  jwin_text_proc,         240,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Pal CSet:",                                           NULL,   NULL                 },
	{  d_dummy_proc,           280,    140,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,    140,     30,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
	//54
	//The tab for these ( npc->Attributes[] ) means that fewer fit. THey need to be rearranged. -Z
	{  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 1:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 2:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 3:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 4:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 5:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 6:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         160,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 7:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         120+40,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 8:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         120+40,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 9:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         120+40,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 10:",                                  NULL,   NULL                 },
	//64 : Misc Attribute boxes. 
	{  jwin_edit_proc,          86,     50,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,     68,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,     86,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,    104,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,    122,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          86,    140,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,     50,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,     68,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,     86,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         242,    104,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//74
	{  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Damaged By Power 0 Weapons",                          NULL,   NULL                 },
	{  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Does not draw",                                        NULL,   NULL                 },
	{  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Never Returns After Death",						   NULL,   NULL                 },
	{  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Doesn't Count As Beatable Enemy",					   NULL,   NULL                 },
	{  d_dummy_proc,             6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",										       NULL,   NULL                 },
	{  d_dummy_proc,             6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	//82
	{  d_dummy_proc,             6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	//90
	{  d_dummy_proc,             6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  d_dummy_proc,             6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	//98
	{  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded In Front",                                   NULL,   NULL                 },
	{  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded On Left",                                    NULL,   NULL                 },
	{  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded On Right",                                   NULL,   NULL                 },
	{  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded In Back",                                    NULL,   NULL                 },
	{  jwin_check_proc,          6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Hammer Can Break Shield",                             NULL,   NULL                 },
	{  d_dummy_proc,             6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shield Is Mirrored",                                  NULL,   NULL                 },
	{  d_dummy_proc,             6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
	{  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can Only Be Seen By Lens Of Truth",                   NULL,   NULL                 },
	//106
	{  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Flashing",                                         NULL,   NULL                 },
	{  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Zora\" Screen Flag",                      NULL,   NULL                 },
	{  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Falling Rocks\" Screen Flag",             NULL,   NULL                 },
	{  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Corner Traps\" Screen Flag",              NULL,   NULL                 },
	{  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Horz Trap\" Combo Type/Flag",            NULL,   NULL                 },
	{  jwin_check_proc,          6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Vert Trap\" Combo Type/Flag",            NULL,   NULL                 },
	{  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"4-Way Trap\" Combo Type/Flag",           NULL,   NULL                 },
	{  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"LR Trap\" Combo Type/Flag",              NULL,   NULL                 },
	//114
	{  jwin_check_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"UD Trap\" Combo Type/Flag",              NULL,   NULL                 },
	{  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Middle Traps\" Screen Flag",              NULL,   NULL                 },
	{  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Statue Fire\" Screen Flag",               NULL,   NULL                 },
	{  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Armos\" Combo Type",                     NULL,   NULL                 },
	{  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Grave\" Combo Type",                     NULL,   NULL                 },
	{  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Ganon\" Room Type",                      NULL,   NULL                 },
	{  jwin_check_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Flickering",                                       NULL,   NULL                 },
	{  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Translucent",                                      NULL,   NULL                 },
	//122
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	//130
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	//138
	{  jwin_text_proc,         240,    162,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "F. Rate:",                                      NULL,   NULL                 },
	{  jwin_text_proc,         240,    180,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "F. Rate:",                                      NULL,   NULL                 },
	{  jwin_edit_proc,         280,    158,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         280,    176,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
	//142 timer_proc
	{  d_timer_proc,             0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_check_proc,        165,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Use Pal CSet",                                        NULL,   NULL                 },
	//144 - note: these are defenses 0-16, 17 is at 191
	{  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Boomerang Defense:",                                  NULL,   NULL                 },
	{  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Bomb Defense:",                                       NULL,   NULL                 },
	{  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Super Bomb Defense:",                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Arrow Defense:",                                      NULL,   NULL                 },
	{  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Fire Defense:",                                       NULL,   NULL                 },
	{  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Wand Melee Defense:",                                       NULL,   NULL                 },
	{  jwin_text_proc,           6,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Magic Defense:",                                      NULL,   NULL                 },
	{  jwin_text_proc,           6,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hookshot Defense:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hammer Defense:",                                     NULL,   NULL                 },
	{  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Sword Defense:",                                      NULL,   NULL                 },
	{  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Sword Beam Defense:",                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Beam Defense:",                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Magic Defense:",                                NULL,   NULL                 },
	{  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Fireball Defense:",                             NULL,   NULL                 },
	{  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Rock Defense:",                                 NULL,   NULL                 },
	{  jwin_text_proc,           6,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Stomp Boots Defense:",                                NULL,   NULL                 },
	{  jwin_text_proc,           6,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Byrna Beam Defense:",                              NULL,   NULL                 },
	//161
	{  jwin_droplist_proc,         126,   54-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   72-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   90-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  108-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  126-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  144-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  162-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  180-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  198-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   54-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   72-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,   90-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  108-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  126-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  144-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  162-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,         126,  180-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_button_proc,           255,    54-4,     48,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "Set All",                                            NULL,   NULL                 },
	//179
	{  jwin_text_proc,           8,    161,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "BG Sound:",                                            NULL,   NULL                 },
	{  jwin_text_proc,           8,    176,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hit Sound:",                                           NULL,   NULL                 },
	{  jwin_text_proc,           8,    191,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Death Sound:",                                         NULL,   NULL                 },
	//182
	{  jwin_droplist_proc,      86,    157,     100,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
	{  jwin_droplist_proc,      86,    172,     100,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
	{  jwin_droplist_proc,      86,    187,     100,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
	{  jwin_text_proc,          6,     214,     95,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Spawn Animation:",                               NULL,   NULL                 },
	{  jwin_droplist_proc,      86,    210,     85,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &walkerspawn_list,                                     NULL,   NULL                 },
//moved to attributes tabs  
	{  d_dummy_proc,         160,    126-4,     50+30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 11:",                                  NULL,   NULL                 },
	{  d_dummy_proc,         160,    144-4,     50+30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 12:",                                  NULL,   NULL                 },
	{  d_dummy_proc,         242,    122-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,         242,    140-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	
	// 191
	{  jwin_text_proc,           6,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Whistle Weapon Defense:",                              NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 198-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
 //193 - sccript 1
		{  jwin_text_proc,           6,    51,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 1 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    67,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 2 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    83,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 3 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    99,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 4 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    115,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 5 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    131,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 6 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    147,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 7 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    163,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 8 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    179,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 9 Defense:",                              NULL,   NULL                 },
	{  jwin_text_proc,           6,    196,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Custom Weapon 10 Defense:",                              NULL,   NULL                 },
	//203 script 1 pulldown
	
	 /* (dialog proc)           (x)   (y)     (w)     (h)      (fg)                   (bg)                    (key) (flags)      (d1)  (d2)  (dp)                                                         (dp2)  (dp3) */
	{  jwin_droplist_proc,      126, 51-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 67-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 83-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 99-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 115-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 131-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 147-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 163-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 179-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 196-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
	//213 HitWidth and Height
	{  jwin_text_proc,         12,      51,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "TileWidth:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    51-4,     30,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
		{  jwin_text_proc,         12,      67,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "TileHeight:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    67-4,     30,     16,    vc(12),                 vc(1),                   0,    0,           6,    0, NULL,                                                           NULL,   NULL                 },
		{  jwin_text_proc,         12,      83,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitWidth:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    83-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0, NULL,                                                           NULL,   NULL                 },
		{  jwin_text_proc,         12,      99,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitHeight:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    99-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//221 HitZHeight
	{  jwin_text_proc,         12,      115,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitZHeight:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    115-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//223 HitXOffset
	{  jwin_text_proc,         12,      131,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitXOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    131-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//225 HitYOffset
	{  jwin_text_proc,         12,      147,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HitYOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    147-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//227 HitZOffset
	{  jwin_text_proc,         12,      163,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "DrawZOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    163-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//229 DrawXOffset
	{  jwin_text_proc,         12,      179,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "DrawXOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    179-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//231 DrawYOffset
	{  jwin_text_proc,         12,      195,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "DrawYOffset:",                                  NULL,   NULL                 },
	{  jwin_edit_proc,         60,    195-4,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//233 'Tiles' annotation. 
	{ jwin_text_proc,         96,      51,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "(Tiles)",                                  NULL,   NULL                 },
	{ jwin_text_proc,         96,      67,       80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "(Tiles)",                                  NULL,   NULL                 },
	//235 : Weapon Sprite Pulldown
	{  jwin_text_proc,          9,    90-36,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Weapon Sprite:",                              NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 86-36,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
	// {  jwin_edit_proc,         86, 189-4+12,    151,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//237 HitWidth Override
	 { jwin_check_proc,        94+50,     83,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//238 HitHeight override
	 { jwin_check_proc,        94+50,    99,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//239 HitZHeight Override
	 { jwin_check_proc,        94+50,     115,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//240 HitXOffset override
	 { jwin_check_proc,        94+50,     131,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//241 HitYOffset Override
	 { jwin_check_proc,        94+50,    147,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//242 DrawZOffset Override
	 { jwin_check_proc,        94+50,     163,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//243 DrawXOffset Override
	 { jwin_check_proc,        94+50,    179,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//244 DrawYOffset Overrife
	 { jwin_check_proc,        94+50,     195,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//245 TileWidth Overrife
	 { jwin_check_proc,        94+50,     51,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//246 TileHeight Overrife
	 { jwin_check_proc,        94+50,     67,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Enable",                        NULL,   NULL                  },
	//247, used to be 2
	 {  d_ecstile_proc,          16,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   (void *)enedata_dlg },
	//248 -- used to be 3
	 {  d_ecstile_proc,          52,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   (void *)enedata_dlg },
	//249 used to be 4
	 {  d_ecstile_proc,          88,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   (void *)enedata_dlg },
	
	 /* (dialog proc)     		(x)   (y)     (w)     (h)    (fg)                   (bg)                    (key) (flags)      (d1)        (d2)  (dp)                           (dp2)  (dp3) */
	 // 250 Movement tabs
	 {  jwin_tab_proc,             4,     32,    312,    204,    0,                      0,                       0,    0,          0,          0, (void *) enemy_movement_tabs ,     NULL, (void *) enedata_dlg   },
	 //251 Scripts tabs
	 {  jwin_tab_proc,             4,     32,    312,    204,    0,                      0,                       0,    0,          0,          0, (void *) enemy_script_tabs ,     NULL, (void *) enedata_dlg   },
	// {  d_timer_proc,             0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
	//252 --was 5, OK button. 
	 {  jwin_button_proc,        86,    240,     61,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "OK",                                                  NULL,   NULL                 },
	//253 -- CANCEL  BUTTON
	 {  jwin_button_proc,       166,    240,     61,     16,    vc(14),                 vc(1),                  27,    D_EXIT,      0,    0, (void *) "Cancel",                                              NULL,   NULL                 },
	//254
	{ jwin_check_proc,        6,     52,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[0]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     62,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[1]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     72,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[2]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     82,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[3]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     92,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[4]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     102,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[5]",                        NULL,   NULL                  },
	//260  
	{ jwin_check_proc,        6,    112,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[6]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     122,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[7]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     132,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[8]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,    142,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[9]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     152,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[10]",                        NULL,   NULL                  },
	//265
	{ jwin_check_proc,        6,    162,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[11]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     172,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[12]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,     182,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[13]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,    192,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[14]",                        NULL,   NULL                  },
	{ jwin_check_proc,        6,    202,     250,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flags[15]",                        NULL,   NULL                  },
  //270
	//attributes page 1, attrib 1 to 8 move from basic tab?
	 {  d_dummy_proc,           9970,   58,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[0]",                                   NULL,   NULL                 },
	{  d_dummy_proc,           9970,     58+(18),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[1]",                                   NULL,   NULL                 },
	{  d_dummy_proc,           9970,     58+(18*2),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[2]",                                   NULL,   NULL                 },
	{  d_dummy_proc,           9970,    58+(18*3),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[3]",                                   NULL,   NULL                 },
	{  d_dummy_proc,           9970,    58+(18*4),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[4]",                                   NULL,   NULL                 },
	{  d_dummy_proc,           9970,    58+(18*5),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[5]",                                   NULL,   NULL                 },
	{  d_dummy_proc,         9970,     58+(18*6),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[6]",                                   NULL,   NULL                 },
	{  d_dummy_proc,         9970,     58+(18*7),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[7]",                                   NULL,   NULL                 },
	  //278 : Misc Attribute boxes. 
	{  d_dummy_proc,         9906,     56,     50,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,          9906,     56+(18),     50,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,          9906,     56+(18*2),     50,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,          9906,    56+(18*3),   50,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,          9906,    56+(18*4),     50,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,          9906,    56+(18*5),     50,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,        9906,     56+(18*6),     50,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,         9906,     56+(18*7),     50,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
//286 
	//attributes page 2, attrib 9 to 16
	 {  d_dummy_proc,           9970,   58,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Attributes[8]",                                   NULL,   NULL                 },
	{  d_dummy_proc,           9906,     58+(18),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Lower Attributes are on Basic->Data 2",                                   NULL,   NULL                 },
	//288
	{  jwin_text_proc,           160,     126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 11:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           160,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 12:",                                   NULL,   NULL                 },
	//290
	{  jwin_text_proc,           6,    58,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 13:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    58+(18),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 14:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         6,     58+(18*2),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 15:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         6,     58+(18*3),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 16:",                                   NULL,   NULL                 },
	  //294 : Misc Attribute boxes. 
	{  d_dummy_proc,         9986,     56,     80,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  d_dummy_proc,          9986,     56+(18),     80,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//296
	{  jwin_edit_proc,          242,     122,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          242,    140,   65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//298
	{  jwin_edit_proc,          126,    56,     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,    56+(18),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,        126,     56+(18*2),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         126,     56+(18*3),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
//302
	//attributes page 3, attrib 17 to 24
	 {  jwin_text_proc,           6,   58+(18*4),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 17:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,     58+(18*5),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 18:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,     58+(18*6),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 19:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    58+(18*7),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 20:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    58+(18*8),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 21:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    58+(18*9),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 22:",                                   NULL,   NULL                 },
	//308
	{  jwin_text_proc,         6,     58,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 23:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         6,     58+(18),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 24:",                                   NULL,   NULL                 },
	  //310 : Misc Attribute boxes. 
	{  jwin_edit_proc,         126,     56+(18*4),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,     56+(18*5),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,     56+(18*6),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,    56+(18*7),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,    56+(18*8),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,    56+(18*9),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	//316
	{  jwin_edit_proc,        126,     56,     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         126,     56+(18),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
//318
	//attributes page 4, attrib 25 to 32
	 {  jwin_text_proc,           6,   58+(18*2),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 25:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,     58+(18*3),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 26:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,     58+(18*4),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 27:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    58+(18*5),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 28:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    58+(18*6),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 29:",                                   NULL,   NULL                 },
	{  jwin_text_proc,           6,    58+(18*7),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 30:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         6,     58+(18*8),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 31:",                                   NULL,   NULL                 },
	{  jwin_text_proc,         6,     58+(18*9),     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 32:",                                   NULL,   NULL                 },
	  //327 : Misc Attribute boxes. 
	{  jwin_edit_proc,         126,     56+(18*2),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,     56+(18*3),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,     56+(18*4),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,    56+(18*5),   95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,    56+(18*6),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,          126,    56+(18*7),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,        126,     56+(18*8),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
	{  jwin_edit_proc,         126,     56+(18*9),     95,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
//334
	{ d_dummy_proc,           112+10,  47+38+10 + 18,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Action Script:",                      NULL,   NULL                  },
	
	{ jwin_droplist_proc,      182,  66,     130,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       D_EXIT,           1,    0, (void *) &npcscript_list,                   NULL,   NULL 				   },
	{ jwin_text_proc,           182,   58,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "NPC Action Script:",                      NULL,   NULL                  },
	//337
	//npc Script InitD Labels (not editable)
	// { jwin_text_proc,       6+10,   10+29+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D0:", NULL, NULL },
	// { jwin_text_proc,       6+10,   10+47+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D1:", NULL, NULL },
	// { jwin_text_proc,       6+10,   10+65+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D2:", NULL, NULL },
	// { jwin_text_proc,       6+10,   10+83+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D3:", NULL, NULL },
	// { jwin_text_proc,       6+10,  10+101+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D4:", NULL, NULL },
	// { jwin_text_proc,       6+10,  10+119+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D5:", NULL, NULL },
	// { jwin_text_proc,       6+10,  10+137+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D6:", NULL, NULL },
	// { jwin_text_proc,       6+10,  10+155+20,   24,    12,   0,        0,       0,       0,          0,             0, (void *) "D7:", NULL, NULL },
	
	//editable npc script InitD fields
	//337
	{ jwin_rtext_proc,         90,     13+29+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*) "InitD[0]",                                                           NULL,   NULL},
	{ jwin_rtext_proc,         90,     13+47+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[1]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+65+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[2]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+83+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[3]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+101+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[4]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+119+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[5]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+137+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[6]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+155+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[7]",                                                           NULL,   NULL                 },
	
	// { jwin_edit_proc,       6+10,   10+29+20,   90,    16,   0,        0,       0,       0,          0,             0, 64, NULL, NULL },
	// { jwin_edit_proc,       6+10,   10+47+20,   90,    16,   0,        0,       0,       0,          0,             0, 64, NULL, NULL },
	// { jwin_edit_proc,       6+10,   10+65+20,   90,    16,   0,        0,       0,       0,          0,             0, 64, NULL, NULL },
	// { jwin_edit_proc,       6+10,   10+83+20,   90,    16,   0,        0,       0,       0,          0,             0, 64, NULL, NULL },
	// { jwin_edit_proc,       6+10,  10+101+20,   90,    16,   0,        0,       0,       0,          0,             0, 64, NULL, NULL },
	// { jwin_edit_proc,       6+10,  10+119+20,   90,    16,   0,        0,       0,       0,          0,             0, 64, NULL, NULL },
	// { jwin_edit_proc,       6+10,  10+137+20,   90,    16,   0,        0,       0,       0,          0,             0, 64, NULL, NULL },
	// { jwin_edit_proc,       6+10,  10+155+20,   90,    16,   0,        0,       0,       0,          0,             0, 64, NULL, NULL },
   
	//NPC InitD Data fields
	//345
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+29+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+47+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+65+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+83+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+101+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,  10+119+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,  10+137+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,  10+155+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	//353
	{ jwin_rtext_proc,         90,     13+29+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[0]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+47+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[1]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+65+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[2]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+83+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[3]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+101+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[4]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+119+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[5]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+137+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[6]",                                                           NULL,   NULL                 },
	{ jwin_rtext_proc,         90,     13+155+20,    90,     16,    vc(14),                 vc(1),                   0,    0,          63,    0,  (void*)"InitD[7]",                                                           NULL,   NULL                 },
	//361
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+29+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+47+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+65+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+83+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,   10+101+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,  10+119+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,  10+137+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	{ jwin_numedit_swap_zsint_proc,      (90-24)+34+10,  10+155+20,   72-16,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
	//369
	{ jwin_text_proc,           182,  58,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "EWeapon Script:",                      NULL,   NULL                  },
	{ jwin_droplist_proc,       182,  66,     130,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       D_EXIT,           1,    0, (void *) &eweaponscript_list,                   NULL,   NULL 				   },
	//371
	{  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Obeys Gravity",                          NULL,   NULL                 },
	{  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can Fall Into Pitfalls",                          NULL,   NULL                 },
	{  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can Walk Over Pitfalls",                          NULL,   NULL                 },
	{  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can Drown In Liquid",                          NULL,   NULL                 },
	{  jwin_check_proc,          6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can Walk On Liquid",                          NULL,   NULL                 },
	/*
	  // 248 scripts
	  {  jwin_tab_proc,                        4,     34,    312,    184,    0,                      0,                       0,    0,          0,          0, (void *) enemy_script_tabs,     NULL, (void *)enedata_dlg   },
	 //249 graphics
	  {  jwin_tab_proc,                        4,     34,    312,    184,    0,                      0,                       0,    0,          0,          0, (void *) enemy_graphics_tabs,     NULL, (void *)enedata_dlg   },
	 //250 sounds
	  {  jwin_tab_proc,                        4,     34,    312,    184,    0,                      0,                       0,    0,          0,          0, (void *) enemy_sounds_tabs,     NULL, (void *)enedata_dlg   },
	//251 basic
	  {  jwin_tab_proc,                        4,     34,    312,    184,    0,                      0,                       0,    0,          0,          0, (void *) enemy_basic_tabs,     NULL, (void *)enedata_dlg   },
	//252 attacks
	  {  jwin_tab_proc,                        4,     34,    312,    184,    0,                      0,                       0,    0,          0,          0, (void *) enemy_attacks_tabs,     NULL, (void *)enedata_dlg   },
	//253 movement
	  {  jwin_tab_proc,                        4,     34,    312,    184,    0,                      0,                       0,    0,          0,          0, (void *) enemy_movement_tabs,     NULL, (void *)enedata_dlg   },
	//254 flags
	  {  jwin_tab_proc,                        4,     34,    312,    184,    0,                      0,                       0,    0,          0,          0, (void *) enemy_flags_tabs,     NULL, (void *)enedata_dlg   },
	//255 attributes
	  {  jwin_tab_proc,                        4,     34,    312,    184,    0,                      0,                       0,    0,          0,          0, (void *) enemy_attribs_tabs,     NULL, (void *)enedata_dlg   },
	*/
	{ d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,          0, (void *) onSnapshot, NULL, NULL },
	//377
	{ jwin_swapbtn_proc,    164,    59,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,    77,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,    95,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   113,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   131,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   149,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   167,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   185,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	//385
	{ jwin_swapbtn_proc,    164,    59,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,    77,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,    95,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   113,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   131,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   149,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   167,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_swapbtn_proc,    164,   185,    16,    16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	//393
	{  jwin_text_proc,        9,   108-36,    80,     8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Shadow Sprite:",                              NULL,   NULL                 },
	{  jwin_droplist_proc,  126,   104-36,   151,    16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
	{  jwin_text_proc,        9,   126-36,    80,     8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Death Sprite:",                              NULL,   NULL                 },
	{  jwin_droplist_proc,  126,   122-36,   151,    16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
	{  jwin_text_proc,        9,   144-36,    80,     8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Spawn Sprite:",                              NULL,   NULL                 },
	{  jwin_droplist_proc,  126,   140-36,   151,    16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
	//399
	{  jwin_check_proc,          6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Ignores \"Temp No Return\".",                                         NULL,   NULL                 },
	{  jwin_check_proc,          6,    90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Ignored by Kill All Enemies effects.",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x00040000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x00080000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x00100000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x00200000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x00400000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x00800000",                                         NULL,   NULL                 },
	//407
	{  d_dummy_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x01000000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x02000000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x04000000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x08000000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x10000000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x20000000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x40000000",                                         NULL,   NULL                 },
	{  d_dummy_proc,          6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Flag2 0x80000000",                                         NULL,   NULL                 },
	//415
	{  jwin_text_proc,           6,    216,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "SwitchHook Weapon Defense:",                            NULL,   NULL                 },
	{  jwin_droplist_proc,      126, 216-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                           NULL,   NULL                 },
	{  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can ONLY Walk On Liquid",                               NULL,   NULL                 },
	{  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can ONLY Walk On Shallow Liquid",                       NULL,   NULL                 },
	{  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can ONLY Walk On Pitfalls",                             NULL,   NULL                 },
	//420
	{  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can walk through solidity",                             NULL,   NULL                 },
	{  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can walk through No Enemies flags / etc",               NULL,   NULL                 },
	{  jwin_check_proc,          6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can walk through screen edge",                          NULL,   NULL                 },
	{  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Use 'scripted movement' for engine movement",           NULL,   NULL                 },
	{  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Cannot be pushed by moving solid objects",              NULL,   NULL                 },
	{  jwin_text_proc,           6,    120,    280,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Below flags only affect scripted movement functions",   NULL,   NULL                 },
	//NPC Script Help buttons
	//426
	{ jwin_button_proc,         92,   10 + 29 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL                 },
	{ jwin_button_proc,         92,   10 + 47 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 65 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 83 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 101 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 119 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 137 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 155 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	//434
	{ jwin_button_proc,         92,   10 + 29 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 47 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 65 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 83 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 101 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 119 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 137 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },
	{ jwin_button_proc,         92,   10 + 155 + 20,   16,    16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"?",                          NULL,   NULL },

	{  NULL,                     0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                            NULL,   NULL                 }
};


void setEnemyLabels(int32_t family)
{
	std::map<int32_t, EnemyNameInfo *> *nmap = getEnemyNameMap();
	std::map<int32_t, EnemyNameInfo *>::iterator it = nmap->find(family);
	EnemyNameInfo *inf = NULL;
	
	if(it != nmap->end())
		inf = it->second;
		
	// Default labels
	enedata_dlg[54].dp = (void*)"Misc. Attr. 1:";
	enedata_dlg[55].dp = (void*)"Misc. Attr. 2:";
	enedata_dlg[56].dp = (void*)"Misc. Attr. 3:";
	enedata_dlg[57].dp = (void*)"Misc. Attr. 4:";
	enedata_dlg[58].dp = (void*)"Misc. Attr. 5:";
	enedata_dlg[59].dp = (void*)"Misc. Attr. 6:";
	enedata_dlg[60].dp = (void*)"Misc. Attr. 7:";
	enedata_dlg[61].dp = (void*)"Misc. Attr. 8:";
	enedata_dlg[62].dp = (void*)"Misc. Attr. 9:";
	enedata_dlg[63].dp = (void*)"Misc. Attr. 10:";
	
	if(family==eeKEESE || family==eeGHINI || family==eePEAHAT || family==eeMANHAN
			|| family==eeGLEEOK || family==eePATRA || family==eeDIG)
		enedata_dlg[22].dp = (void*)"Turn Freq.:";
	else
		enedata_dlg[22].dp = (void*)"Halt Rate:";
		
	//set enemy editor new 2.55 flags 1 to 16 label
	for(int32_t i = 0; i < 16; i++)
	{
		if(inf != NULL)
		{
			if(inf->flags[i]!=NULL)
			{
				enedata_dlg[254+i].dp = (char*)inf->flags[i];
			}
			else
			{
				//enedata_dlg[64+i].flags = enedata_dlg[54+i].flags = D_DISABLED;
			}
		}
	
	}
	
	//Enemy Attribute Labels, starting at 11
	for ( int32_t q = 10; q < 16; q++ ) //check these numbers! -Z
	{
		if(inf != NULL && inf->attributes[q]!=NULL)
		{
			enedata_dlg[288+(q-10)].dp = (char*)inf->attributes[q];
		}
		if(inf != NULL && inf->list[q] != NULL)
		{
			enedata_dlg[296+(q-10)].proc = jwin_droplist_proc;
			enedata_dlg[296+(q-10)].fg = jwin_pal[jcTEXTFG];
			enedata_dlg[296+(q-10)].bg = jwin_pal[jcTEXTBG];
			((ListData*)inf->list[q])->font = (&a4fonts[font_lfont_l]);
			enedata_dlg[296+(q-10)].dp = inf->list[q];
			enedata_dlg[296+(q-10)].dp2 = NULL;
			enedata_dlg[296+(q-10)].h = 22;
		}
		else
		{
			enedata_dlg[296+(q-10)].proc = jwin_edit_proc;
			enedata_dlg[296+(q-10)].fg = vc(14);
			enedata_dlg[296+(q-10)].bg = vc(1);
			enedata_dlg[296+(q-10)].dp = NULL;
			enedata_dlg[296+(q-10)].d1 = 6;
			enedata_dlg[296+(q-10)].h = 24;
			enedata_dlg[296+(q-10)].dp2 = get_zc_font(font_lfont_l);
		}
	}
	for ( int32_t q = 16; q < 24; q++ ) //check these numbers! -Z
	{
		if(inf->attributes[q]!=NULL)
		{
			enedata_dlg[302+(q-16)].dp = (char*)inf->attributes[q];
		}
		if(inf != NULL && inf->list[q] != NULL)
		{
			enedata_dlg[310+(q-16)].proc = jwin_droplist_proc;
			enedata_dlg[310+(q-16)].fg = jwin_pal[jcTEXTFG];
			enedata_dlg[310+(q-16)].bg = jwin_pal[jcTEXTBG];
			((ListData*)inf->list[q])->font = (&a4fonts[font_lfont_l]);
			enedata_dlg[310+(q-16)].dp = inf->list[q];
			enedata_dlg[310+(q-16)].dp2 = NULL;
			enedata_dlg[310+(q-16)].h = 22;
		}
		else
		{
			enedata_dlg[310+(q-16)].proc = jwin_edit_proc;
			enedata_dlg[310+(q-16)].fg = vc(14);
			enedata_dlg[310+(q-16)].bg = vc(1);
			enedata_dlg[310+(q-16)].dp = NULL;
			enedata_dlg[310+(q-16)].d1 = 6;
			enedata_dlg[310+(q-16)].h = 24;
			enedata_dlg[310+(q-16)].dp2 = get_zc_font(font_lfont_l);
		}
	}
	for ( int32_t q = 24; q < 32; q++ ) //check these numbers! -Z
	{
		if(inf->attributes[q]!=NULL)
		{
			enedata_dlg[318+(q-24)].dp = (char*)inf->attributes[q]; //attr err
		}
		if(inf != NULL && inf->list[q] != NULL)
		{
			enedata_dlg[326+(q-24)].proc = jwin_droplist_proc;
			enedata_dlg[326+(q-24)].fg = jwin_pal[jcTEXTFG];
			enedata_dlg[326+(q-24)].bg = jwin_pal[jcTEXTBG];
			((ListData*)inf->list[q])->font = (&a4fonts[font_lfont_l]);
			enedata_dlg[326+(q-24)].dp = inf->list[q];
			enedata_dlg[326+(q-24)].dp2 = NULL;
			enedata_dlg[326+(q-24)].h = 22;
		}
		else
		{
			enedata_dlg[326+(q-24)].proc = jwin_edit_proc;
			enedata_dlg[326+(q-24)].fg = vc(14);
			enedata_dlg[326+(q-24)].bg = vc(1);
			enedata_dlg[326+(q-24)].dp = NULL;
			enedata_dlg[326+(q-24)].d1 = 6;
			enedata_dlg[326+(q-24)].h = 24;
			enedata_dlg[326+(q-24)].dp2 = get_zc_font(font_lfont_l);
		}
	}
	
	for(int32_t i = 0; i < 10; i++)
	{
		if(inf != NULL)
		{
			if(inf->misc[i]!=NULL)
			{
				enedata_dlg[54+i].dp = (char*)inf->misc[i];
				//enedata_dlg[54+i].flags = enedata_dlg[64+i].flags = 0;
			}
			else
			{
				//enedata_dlg[64+i].flags = enedata_dlg[54+i].flags = D_DISABLED;
			}
		}
		
		if(inf != NULL && inf->list[i] != NULL)
		{
			enedata_dlg[64+i].proc = jwin_droplist_proc;
			enedata_dlg[64+i].fg = jwin_pal[jcTEXTFG];
			enedata_dlg[64+i].bg = jwin_pal[jcTEXTBG];
			((ListData*)inf->list[i])->font = (&a4fonts[font_lfont_l]);
			enedata_dlg[64+i].dp = inf->list[i];
			enedata_dlg[64+i].dp2 = NULL;
			enedata_dlg[64+i].h = 22;
		}
		else
		{
			enedata_dlg[64+i].proc = jwin_edit_proc;
			enedata_dlg[64+i].fg = vc(14);
			enedata_dlg[64+i].bg = vc(1);
			enedata_dlg[64+i].dp = NULL;
			enedata_dlg[64+i].d1 = 6;
			enedata_dlg[64+i].h = 24;
			enedata_dlg[64+i].dp2 = get_zc_font(font_lfont_l);
		}
	}
	
	bool r = 0 != get_qr(qr_NEWENEMYTILES);
	enedata_dlg[r ? 47 : 48].flags |= D_DISABLED;
	enedata_dlg[r ? 48 : 47].flags &= ~D_DISABLED;
	enedata_dlg[r ? 140 : 141].flags |= D_DISABLED;
	enedata_dlg[r ? 141 : 140].flags &= ~D_DISABLED;
	
	if(family==eeTRAP || family==eeROCK || family==eeDONGO ) //|| family==eeGANON)
	{
		for(int32_t j=0; j <= edefBYRNA+1 /* + the Set All button*/; j++) enedata_dlg[j+161].flags |= D_DISABLED;
		
		enedata_dlg[192].flags |= D_DISABLED; //Defenses disabled for Traps. rocks,. Dodongos and Ganon. 
		//We will need to remove Ganon from this list once we give him defence properties in the EE. -Z
		
	enedata_dlg[203].d1 |= D_DISABLED;
		 enedata_dlg[204].d1 |= D_DISABLED;
		 enedata_dlg[205].d1 |= D_DISABLED;
		 enedata_dlg[206].d1 |= D_DISABLED;
		 enedata_dlg[207].d1 |= D_DISABLED;
		 enedata_dlg[208].d1 |= D_DISABLED;
		 enedata_dlg[209].d1 |= D_DISABLED;
		 enedata_dlg[210].d1 |= D_DISABLED;
		 enedata_dlg[211].d1 |= D_DISABLED;
		 enedata_dlg[212].d1 |= D_DISABLED;
	}
	else
	{
		for(int32_t j=0; j <= edefBYRNA+1 /* + the Set All button*/; j++) enedata_dlg[j+161].flags &= ~D_DISABLED;
		
		enedata_dlg[192].flags &= ~D_DISABLED;
		enedata_dlg[203].d1 &= ~D_DISABLED;
		 enedata_dlg[204].d1 &= ~D_DISABLED;
		 enedata_dlg[205].d1 &= ~D_DISABLED;
		 enedata_dlg[206].d1 &= ~D_DISABLED;
		 enedata_dlg[207].d1 &= ~D_DISABLED;
		 enedata_dlg[208].d1 &= ~D_DISABLED;
		 enedata_dlg[209].d1 &= ~D_DISABLED;
		 enedata_dlg[210].d1 &= ~D_DISABLED;
		 enedata_dlg[211].d1 &= ~D_DISABLED;
		 enedata_dlg[212].d1 &= ~D_DISABLED;
	}
	
	if(!(family==eeWALK || family==eeFIRE || family==eeOTHER))
	{
		enedata_dlg[98].flags |= D_DISABLED;
		enedata_dlg[99].flags |= D_DISABLED;
		enedata_dlg[100].flags |= D_DISABLED;
		enedata_dlg[101].flags |= D_DISABLED;
		enedata_dlg[102].flags |= D_DISABLED;
		enedata_dlg[186].flags |= D_DISABLED;
	}
	else
	{
		enedata_dlg[98].flags &= ~D_DISABLED;
		enedata_dlg[99].flags &= ~D_DISABLED;
		enedata_dlg[100].flags &= ~D_DISABLED;
		enedata_dlg[101].flags &= ~D_DISABLED;
		enedata_dlg[102].flags &= ~D_DISABLED;
		enedata_dlg[186].flags &= ~D_DISABLED;
	}
}

int32_t d_ecstile_proc(int32_t msg,DIALOG *d,int32_t c)
{
	//these are here to bypass compiler warnings about unused arguments
	c=c;
	int32_t *tempint=enedata_flags2_list;
	tempint=tempint;
	
	switch(msg)
	{
	case MSG_CLICK:
	{
		int32_t f = -8; // Suppress Flip, but in a way that the lowest 3 bits are 0. (Trust me here.) -L
		int32_t t = d->d1;
		int32_t cs = d->d2;
		
		if((CHECK_CTRL_CMD)
			? select_tile_2(t,f,1,cs,true)
			: select_tile(t,f,1,cs,true))
		{
			d->d1 = t;
			enedata_dlg[247].d2 = cs;
			enedata_dlg[248].d2 = cs;
			enedata_dlg[249].d2 = cs;
			return D_REDRAW;
		}
	}
	break;
	
	case MSG_DRAW:
		d->w = 36;
		d->h = 36;
		
		BITMAP *buf = create_bitmap_ex(8,20,20);
		BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
		
		if(buf && bigbmp)
		{
			clear_bitmap(buf);
			
			if(d->d1)
				overtile16(buf,d->d1,2,2,d->d2,0);
				
			stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->w-3, d->h-3);
			destroy_bitmap(buf);
			jwin_draw_frame(bigbmp,0,0,d->w,d->h,FR_DEEP);
			blit(bigbmp,screen,0,0,d->x,d->y,d->w,d->h);
			destroy_bitmap(bigbmp);
		}
		
		//    text_mode(d->bg);
		break;
	}
	
	return D_O_K;
}

void setEnemyScriptLabels(const guydata& test, char (&npc_initd_labels)[8][65], char (&weapon_initd_labels)[8][65])
{
	for (int32_t q = 0; q < 8; ++q)
	{
		sprintf(npc_initd_labels[q], "InitD[%d]", q);
		sprintf(weapon_initd_labels[q], "InitD[%d]", q);
		enedata_dlg[345 + q].d1 &= ~0xF;
		enedata_dlg[345 + q].d1 |= nswapDEC;
		enedata_dlg[361 + q].d1 &= ~0xF;
		enedata_dlg[361 + q].d1 |= nswapDEC;
		enedata_dlg[426 + q].flags |= D_DISABLED;
		enedata_dlg[434 + q].flags |= D_DISABLED;
	}

	if (test.script)
	{
		zasm_meta const& meta = guyscripts[test.script]->meta;
		for (int32_t q = 0; q < 8; ++q)
		{
			if (!meta.initd[q].empty())
				snprintf(npc_initd_labels[q], 64, "%s", meta.initd[q].c_str());
			if(!meta.initd_help[q].empty())
				enedata_dlg[426 + q].flags &= ~D_DISABLED;
			DIALOG* swapbtn = (DIALOG*)enedata_dlg[345 + q].dp3;
			if(meta.initd_type[q]>-1)
			{
				swapbtn->d1 &= ~0xF;
				swapbtn->d1 |= meta.initd_type[q] & 0x3;
			}
		}
	}

	if (test.weaponscript)
	{
		zasm_meta const& meta = ewpnscripts[test.weaponscript]->meta;
		for (int32_t q = 0; q < 8; ++q)
		{
			if(!meta.initd[q].empty())
				snprintf(weapon_initd_labels[q], 64, "%s", meta.initd[q].c_str());
			if (!meta.initd_help[q].empty())
				enedata_dlg[434 + q].flags &= ~D_DISABLED;
			DIALOG* swapbtn = (DIALOG*)enedata_dlg[361 + q].dp3;
			if (meta.initd_type[q] > -1)
			{
				swapbtn->d1 &= ~0xF;
				swapbtn->d1 |= meta.initd_type[q] & 0x3;
			}
		}
	}
}

void showEnemyScriptMetaHelp(const guydata& test, int32_t i)
{
	if (i < 8)
	{
		if (test.script)
		{
			zasm_meta const& meta = guyscripts[test.script]->meta;
			if(!meta.initd_help[i].empty())
				InfoDialog("Info", meta.initd_help[i].c_str()).show();
		}
	}
	else
	{
		i -= 8;
		if (test.weaponscript)
		{
			zasm_meta const& meta = ewpnscripts[test.weaponscript]->meta;
			if (!meta.initd_help[i].empty())
				InfoDialog("Info", meta.initd_help[i].c_str()).show();
		}
	}
}

void edit_enemydata(int32_t index)
{
	//guysbuf[index].script = 1;
	char hp[8], dp[8], wdp[8], rat[8], hrt[8], hom[8], grm[8], spd[8],
		 frt[8], efr[8], bsp[8];
	char w[8],h[8],sw[8],sh[8],ew[8],eh[8];
	char name[64];
	char attribs[32][8];
	char enemynumstr[75];
	char hitx[8], hity[8], hitz[8], tiley[8], tilex[8], hitofsx[8], hitofsy[8], hitofsz[8], drawofsx[8], drawofsy[8];
	char npc_initd_labels[8][65];
	char weapon_initd_labels[8][65];
	char weap_initdvals[8][13];
	
	char initdvals[8][13];
	//begin npc script
	int32_t j = 0; build_binpcs_list(); //npc scripts lister
	for(j = 0; j < binpcs_cnt; j++)
	{
		if(binpcs[j].second == guysbuf[index].script -1)
		{
			enedata_dlg[335].d1 = j; 
			break;
		} 
	}
	
	int32_t j2 = 0; 
	build_bieweapons_list(); //lweapon scripts lister
	for(j2 = 0; j2 < bieweapons_cnt; j2++)
	{
		if(bieweapons[j2].second == guysbuf[index].weaponscript -1)
		{
			enedata_dlg[370].d1 = j2; 
			break;
		}
	}
	
	for ( int32_t q = 0; q < 8; q++ )
	{
		//NPC InitD / WeaponInitD
		enedata_dlg[345+q].dp = initdvals[q];
		enedata_dlg[361+q].dp = weap_initdvals[q];
		enedata_dlg[345+q].fg = guysbuf[index].initD[q];
		enedata_dlg[361+q].fg = guysbuf[index].weap_initiald[q];
		enedata_dlg[345+q].dp3 = &(enedata_dlg[377+q]);
		enedata_dlg[361+q].dp3 = &(enedata_dlg[385+q]);

		//InitD Labels
		enedata_dlg[337 + q].dp = npc_initd_labels[q];
		enedata_dlg[353 + q].dp = weapon_initd_labels[q];
	}

	//end npc script
	
	//disable the missing dialog items!
	//else they will lurk in the background
	//stealing mouse focus -DD
	for(int32_t i=0; enedata_flags2_list[i] != -1; i++)
	{
		enedata_dlg[enedata_flags2_list[i]].proc = d_dummy_proc;
		enedata_dlg[enedata_flags2_list[i]].x = 0;
		enedata_dlg[enedata_flags2_list[i]].y = 0;
		enedata_dlg[enedata_flags2_list[i]].w = 0;
		enedata_dlg[enedata_flags2_list[i]].h = 0;
	}
	
	sprintf(enemynumstr,"Enemy %d: %s", index, guy_string[index]);
	enedata_dlg[0].dp = enemynumstr;
	enedata_dlg[0].dp2 = get_zc_font(font_lfont);
	enedata_dlg[247].d1 = guysbuf[index].tile;
	enedata_dlg[247].d2 = guysbuf[index].cset;
	enedata_dlg[248].d1 = guysbuf[index].s_tile;
	enedata_dlg[248].d2 = guysbuf[index].cset;
	enedata_dlg[249].d1 = guysbuf[index].e_tile;
	enedata_dlg[249].d2 = guysbuf[index].cset;
	
	// Enemy weapon list
	if(guysbuf[index].weapon==wNone)
	{
		enedata_dlg[45].d1 = 0;
	}
	else
	{
		if(biew_cnt==-1)
		{
			build_biew_list();
		}
		
		for(int32_t j=0; j<biew_cnt; j++)
		{
			if(biew[j].i == guysbuf[index].weapon /*- wEnemyWeapons*/)
				enedata_dlg[45].d1 = j;
		}
	}
	
	// Enemy family list
	if(bief_cnt==-1)
	{
		build_bief_list();
	}
	
	for(int32_t j=0; j<bief_cnt; j++)
	{
		if(bief[j].i == guysbuf[index].family)
			enedata_dlg[46].d1 = j;
	}
	
	// Enemy animation list
	if(biea_cnt==-1)
	{
		build_biea_list();
	}
	
	for(int32_t j=0; j<biea_cnt; j++)
	{
		if(biea[j].i == guysbuf[index].anim)
			enedata_dlg[47].d1 = j;
			
		if(biea[j].i == guysbuf[index].e_anim)
			enedata_dlg[48].d1 = j;
	}
	
	
	enedata_dlg[49].d1 = guysbuf[index].item_set;
	
	sprintf(w,"%d",guysbuf[index].width);
	sprintf(h,"%d",guysbuf[index].height);
	sprintf(sw,"%d",guysbuf[index].s_width);
	sprintf(sh,"%d",guysbuf[index].s_height);
	sprintf(ew,"%d",guysbuf[index].e_width);
	sprintf(eh,"%d",guysbuf[index].e_height);
	enedata_dlg[30].dp = w;
	enedata_dlg[31].dp = h;
	enedata_dlg[32].dp = sw;
	enedata_dlg[33].dp = sh;
	enedata_dlg[34].dp = ew;
	enedata_dlg[35].dp = eh;
	
	sprintf(hp,"%d",guysbuf[index].hp);
	sprintf(dp,"%d",guysbuf[index].dp);
	sprintf(wdp,"%d",guysbuf[index].wdp);
	sprintf(grm,"%d",guysbuf[index].grumble);
	enedata_dlg[37].dp = hp;
	enedata_dlg[38].dp = dp;
	enedata_dlg[39].dp = wdp;
	enedata_dlg[40].dp = grm;
	
	sprintf(rat,"%d",guysbuf[index].rate);
	sprintf(hrt,"%d",guysbuf[index].hrate);
	sprintf(hom,"%d",guysbuf[index].homing);
	sprintf(spd,"%d",guysbuf[index].step);
	enedata_dlg[41].dp = rat;
	enedata_dlg[42].dp = hrt;
	enedata_dlg[43].dp = hom;
	enedata_dlg[44].dp = spd;
	
	sprintf(name,"%s",guy_string[index]);
	enedata_dlg[36].dp = name;
	
	for (int q = 0; q < 32; ++q)
	{
		sprintf(attribs[q], "%d", guysbuf[index].attributes[q]);
	}
	
	
	for(int32_t j=0; j <= edefBYRNA; j++)
	{
		enedata_dlg[j+161].d1 = guysbuf[index].defense[j];
	}
	
	enedata_dlg[192].d1 = guysbuf[index].defense[edefWhistle];
	enedata_dlg[416].d1 = guysbuf[index].defense[edefSwitchHook];
	
	
	//Script Defenses
	enedata_dlg[203].d1 = guysbuf[index].defense[edefSCRIPT01];
	enedata_dlg[204].d1 = guysbuf[index].defense[edefSCRIPT02];
	enedata_dlg[205].d1 = guysbuf[index].defense[edefSCRIPT03];
	enedata_dlg[206].d1 = guysbuf[index].defense[edefSCRIPT04];
	enedata_dlg[207].d1 = guysbuf[index].defense[edefSCRIPT05];
	enedata_dlg[208].d1 = guysbuf[index].defense[edefSCRIPT06];
	enedata_dlg[209].d1 = guysbuf[index].defense[edefSCRIPT07];
	enedata_dlg[210].d1 = guysbuf[index].defense[edefSCRIPT08];
	enedata_dlg[211].d1 = guysbuf[index].defense[edefSCRIPT09];
	enedata_dlg[212].d1 = guysbuf[index].defense[edefSCRIPT10];
	
	//tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
	sprintf(tilex,"%d",guysbuf[index].txsz);
	sprintf(tiley,"%d",guysbuf[index].tysz);
	sprintf(hitx,"%d",guysbuf[index].hxsz);
	sprintf(hity,"%d",guysbuf[index].hysz);
	sprintf(hitz,"%d",guysbuf[index].hzsz);
  
	enedata_dlg[214].dp = tilex;
	enedata_dlg[216].dp = tiley;
	enedata_dlg[218].dp = hitx;
	enedata_dlg[220].dp = hity;
	enedata_dlg[222].dp = hitz;
	
	//HitXOffset, HitYOFfset, hitZOffset, DrawXOffsrt, DrawYOffset
	sprintf(hitofsx,"%d",guysbuf[index].hxofs);
	sprintf(hitofsy,"%d",guysbuf[index].hyofs);
	sprintf(hitofsz,"%d",guysbuf[index].zofs);
	sprintf(drawofsx,"%d",guysbuf[index].xofs);
	sprintf(drawofsy,"%d",guysbuf[index].yofs); //This seems to be setting to +48 or something with any value set?! -Z
	
	enedata_dlg[224].dp = hitofsx;
	enedata_dlg[226].dp = hitofsy;
	enedata_dlg[228].dp = hitofsz;
	enedata_dlg[230].dp = drawofsx;
	enedata_dlg[232].dp = drawofsy; //This seems to be setting to +48 or something with any value set?! -Z
	
	//Override flags
	enedata_dlg[237].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_HIT_WIDTH) ? D_SELECTED : 0;
	enedata_dlg[238].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) ? D_SELECTED : 0;
	enedata_dlg[239].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) ? D_SELECTED : 0;
	enedata_dlg[240].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET) ? D_SELECTED : 0;
	enedata_dlg[241].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET) ? D_SELECTED : 0;
	enedata_dlg[242].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET) ? D_SELECTED : 0;
	enedata_dlg[243].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET) ? D_SELECTED : 0;
	enedata_dlg[244].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET) ? D_SELECTED : 0;
	enedata_dlg[245].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_TILE_WIDTH) ? D_SELECTED : 0;
	enedata_dlg[246].flags = (guysbuf[index].SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) ? D_SELECTED : 0;
	
	//New Misc FLags (2.55, flag1 to flag16)
	enedata_dlg[254].flags = (guysbuf[index].editorflags&ENEMY_FLAG1) ? D_SELECTED : 0;
	enedata_dlg[255].flags = (guysbuf[index].editorflags&ENEMY_FLAG2) ? D_SELECTED : 0;
	enedata_dlg[256].flags = (guysbuf[index].editorflags&ENEMY_FLAG3) ? D_SELECTED : 0;
	enedata_dlg[257].flags = (guysbuf[index].editorflags&ENEMY_FLAG4) ? D_SELECTED : 0;
	enedata_dlg[258].flags = (guysbuf[index].editorflags&ENEMY_FLAG5) ? D_SELECTED : 0;
	enedata_dlg[259].flags = (guysbuf[index].editorflags&ENEMY_FLAG6) ? D_SELECTED : 0;
	enedata_dlg[260].flags = (guysbuf[index].editorflags&ENEMY_FLAG7) ? D_SELECTED : 0;
	enedata_dlg[261].flags = (guysbuf[index].editorflags&ENEMY_FLAG8) ? D_SELECTED : 0;
	enedata_dlg[262].flags = (guysbuf[index].editorflags&ENEMY_FLAG9) ? D_SELECTED : 0;
	enedata_dlg[263].flags = (guysbuf[index].editorflags&ENEMY_FLAG10) ? D_SELECTED : 0;
	enedata_dlg[264].flags = (guysbuf[index].editorflags&ENEMY_FLAG11) ? D_SELECTED : 0;
	enedata_dlg[265].flags = (guysbuf[index].editorflags&ENEMY_FLAG12) ? D_SELECTED : 0;
	enedata_dlg[266].flags = (guysbuf[index].editorflags&ENEMY_FLAG13) ? D_SELECTED : 0;
	enedata_dlg[267].flags = (guysbuf[index].editorflags&ENEMY_FLAG14) ? D_SELECTED : 0;
	enedata_dlg[268].flags = (guysbuf[index].editorflags&ENEMY_FLAG15) ? D_SELECTED : 0;
	enedata_dlg[269].flags = (guysbuf[index].editorflags&ENEMY_FLAG16) ? D_SELECTED : 0;
  
	
	sprintf(frt,"%d",guysbuf[index].frate);
	sprintf(efr,"%d",guysbuf[index].e_frate);
	enedata_dlg[140].dp = frt;
	enedata_dlg[141].dp = efr;
	
	//sprintf(sfx,"%d",guysbuf[index].bgsfx);
	enedata_dlg[182].d1= (int32_t)guysbuf[index].bgsfx;
	enedata_dlg[183].d1= (int32_t)guysbuf[index].hitsfx;
	if ( ( enedata_dlg[183].d1 == 0 ) && FFCore.getQuestHeaderInfo(vZelda) < 0x250 || (( FFCore.getQuestHeaderInfo(vZelda) == 0x250 ) && FFCore.getQuestHeaderInfo(vBuild) < 32 ) )
	{
		//If no user-set hit sound was in place, and the quest was made in a version before 2.53.0 Gamma 2:
		enedata_dlg[183].d1 = WAV_EHIT; //Fix quests using the wrong hit sound when loading this. 
		//Force SFX_HIT here. 
		
	}
	enedata_dlg[184].d1= (int32_t)guysbuf[index].deadsfx;
	
	// Sprites
	if(biw_cnt==-1)
	{
		build_biw_list();
	}
	for(int32_t j=0; j<biw_cnt; j++)
	{
		if(biw[j].i == guysbuf[index].wpnsprite)
			enedata_dlg[236].d1  = j;
		if(biw[j].i == guysbuf[index].spr_shadow)
			enedata_dlg[394].d1  = j;
		if(biw[j].i == guysbuf[index].spr_death)
			enedata_dlg[396].d1  = j;
		if(biw[j].i == guysbuf[index].spr_spawn)
			enedata_dlg[398].d1  = j;
	}
	
	sprintf(bsp,"%d",guysbuf[index].bosspal);
	
	if(guysbuf[index].cset == 14)
	{
		enedata_dlg[143].flags = D_SELECTED;
	}
	else
	{
		enedata_dlg[143].flags = 0;
	}
	
	enedata_dlg[53].dp = bsp;
	
	for(int32_t i=0; i<32; i++)
		enedata_dlg[74+i].flags = (guysbuf[index].flags & (1<<i)) ? D_SELECTED : 0;
		
	enedata_dlg[186].d1 = (guysbuf[index].flags & guy_fade_instant ? 2
						   : guysbuf[index].flags & guy_fade_flicker ? 1 : 0);
						   
	for(int32_t i=0; i<16; i++)
		enedata_dlg[106+i].flags = (guysbuf[index].flags & (1<<i)) ? D_SELECTED : 0;
	
	for(int32_t i=0; i<16; i++)
		enedata_dlg[399+i].flags = (guysbuf[index].flags & (1<<(i+16))) ? D_SELECTED : 0;
	
	enedata_dlg[371].flags = (guysbuf[index].moveflags & move_obeys_grav) ? D_SELECTED : 0;
	enedata_dlg[372].flags = (guysbuf[index].moveflags & move_can_pitfall) ? D_SELECTED : 0;
	enedata_dlg[373].flags = (guysbuf[index].moveflags & move_can_pitwalk) ? D_SELECTED : 0;
	enedata_dlg[374].flags = (guysbuf[index].moveflags & move_can_waterdrown) ? D_SELECTED : 0;
	enedata_dlg[375].flags = (guysbuf[index].moveflags & move_can_waterwalk) ? D_SELECTED : 0;
	enedata_dlg[417].flags = (guysbuf[index].moveflags & move_only_waterwalk) ? D_SELECTED : 0;
	enedata_dlg[418].flags = (guysbuf[index].moveflags & move_only_shallow_waterwalk) ? D_SELECTED : 0;
	enedata_dlg[419].flags = (guysbuf[index].moveflags & move_only_pitwalk) ? D_SELECTED : 0;
	enedata_dlg[420].flags = (guysbuf[index].moveflags & move_ignore_solidity) ? D_SELECTED : 0;
	enedata_dlg[421].flags = (guysbuf[index].moveflags & move_ignore_blockflags) ? D_SELECTED : 0;
	enedata_dlg[422].flags = (guysbuf[index].moveflags & move_ignore_screenedge) ? D_SELECTED : 0;
	enedata_dlg[423].flags = (guysbuf[index].moveflags & move_new_movement) ? D_SELECTED : 0;
	enedata_dlg[424].flags = (guysbuf[index].moveflags & move_not_pushable) ? D_SELECTED : 0;
	
	int32_t ret;
	guydata test;
	memset(&test, 0, sizeof(guydata));
	
	large_dialog(enedata_dlg);
	
	setEnemyLabels(guysbuf[index].family);
	setEnemyScriptLabels(guysbuf[index], npc_initd_labels, weapon_initd_labels);
	
	popup_zqdialog_start();
	do
	{
		for(int32_t i=0; i<10; i++)
		{
			if(enedata_dlg[64+i].proc==jwin_droplist_proc)
			{
				int32_t size = 0;
				((ListData*)enedata_dlg[64+i].dp)->listFunc(-1,&size);
				sprintf(attribs[i],"%d",vbound(atoi(attribs[i]), 0, size));
				enedata_dlg[64+i].d1 = atoi(attribs[i]);
			}
			else
				enedata_dlg[64+i].dp = attribs[i];
		}
		for ( int32_t q = 10; q < 16; q++ )
		{
			if (enedata_dlg[296+(q-10)].proc == jwin_droplist_proc)
			{
				int32_t size = 0;
				((ListData*)enedata_dlg[296+(q-10)].dp)->listFunc(-1,&size);
				sprintf(attribs[q],"%d",vbound(atoi(attribs[q]), 0, size));
				enedata_dlg[296+(q-10)].d1 = atoi(attribs[q]);
			}
			else 
				enedata_dlg[296+(q-10)].dp = attribs[q];
		}
		for ( int32_t q = 16; q < 24; q++ )
		{
			if (enedata_dlg[310+(q-16)].proc == jwin_droplist_proc)
			{
				int32_t size = 0;
				((ListData*)enedata_dlg[310+(q-16)].dp)->listFunc(-1,&size);
				sprintf(attribs[q],"%d",vbound(atoi(attribs[q]), 0, size));
				enedata_dlg[310+(q-16)].d1 = atoi(attribs[q]);
			}
			else 
				enedata_dlg[310+(q-16)].dp = attribs[q];
		}
		for ( int32_t q = 24; q < 32; q++ )
		{
			if (enedata_dlg[326+(q-24)].proc == jwin_droplist_proc)
			{
				int32_t size = 0;
				((ListData*)enedata_dlg[326+(q-24)].dp)->listFunc(-1,&size);
				sprintf(attribs[q],"%d",vbound(atoi(attribs[q]), 0, size));
				enedata_dlg[326+(q-24)].d1 = atoi(attribs[q]);
			}
			else 
				enedata_dlg[326+(q-24)].dp = attribs[q];
		}
		
		enedata_dlg[278].dp = attribs[0];
		enedata_dlg[279].dp = attribs[1];
		enedata_dlg[280].dp = attribs[2];
		enedata_dlg[281].dp = attribs[3];
		enedata_dlg[282].dp = attribs[4];
		enedata_dlg[283].dp = attribs[5];
		enedata_dlg[284].dp = attribs[6];
		enedata_dlg[285].dp = attribs[7];
		
		enedata_dlg[294].dp = attribs[8];
		enedata_dlg[295].dp = attribs[9];
		
		enedata_dlg[296].dp = attribs[10];
		enedata_dlg[297].dp = attribs[11];
		enedata_dlg[298].dp = attribs[12];
		enedata_dlg[299].dp = attribs[13];
		enedata_dlg[300].dp = attribs[14];
		enedata_dlg[301].dp = attribs[15];
		
		enedata_dlg[310].dp = attribs[16];
		enedata_dlg[311].dp = attribs[17];
		enedata_dlg[312].dp = attribs[18];
		enedata_dlg[313].dp = attribs[19];
		enedata_dlg[314].dp = attribs[20];
		enedata_dlg[315].dp = attribs[21];
		enedata_dlg[316].dp = attribs[22];
		enedata_dlg[317].dp = attribs[23];
		
		enedata_dlg[326].dp = attribs[24];
		enedata_dlg[327].dp = attribs[25];
		enedata_dlg[328].dp = attribs[26];
		enedata_dlg[329].dp = attribs[27];
		enedata_dlg[330].dp = attribs[28];
		enedata_dlg[331].dp = attribs[29];
		enedata_dlg[332].dp = attribs[30];
		enedata_dlg[333].dp = attribs[31];
		
		ret = do_zqdialog(enedata_dlg,3);
		
		
		
		test.tile  = enedata_dlg[247].d1;
		test.cset = enedata_dlg[247].d2;
		test.s_tile  = enedata_dlg[248].d1;
		test.e_tile  = enedata_dlg[249].d1;
		
		test.width = vbound(atoi(w),0,20);
		test.height = vbound(atoi(h),0,20);
		test.s_width = vbound(atoi(sw),0,20);
		test.s_height = vbound(atoi(sh),0,20);
		test.e_width = vbound(atoi(ew),0,20);
		test.e_height = vbound(atoi(eh),0,20);
		
		test.weapon = enedata_dlg[45].d1 != 0 ? biew[enedata_dlg[45].d1].i /*+ wEnemyWeapons*/ : wNone;
		test.family = bief[enedata_dlg[46].d1].i;
		test.anim = biea[enedata_dlg[47].d1].i;
		test.e_anim = biea[enedata_dlg[48].d1].i;
		test.item_set = enedata_dlg[49].d1;
		
		test.hp = vbound(atoi(hp), 0, 32767); //0x7FFF, not 0xFFFF?
		test.dp = vbound(atoi(dp), 0, 32767);
		test.wdp = vbound(atoi(wdp), 0, 32767);
		test.grumble = vbound(atoi(grm), -4, 4);
		
		test.rate = vbound(atoi(rat), 0, (test.family == eeFIRE || test.family == eeOTHER)?32767:16);
		test.hrate = vbound(atoi(hrt), 0, (test.family == eeFIRE || test.family == eeOTHER)?32767:16);
		test.homing = vbound(atoi(hom), -256, (test.family == eeFIRE || test.family == eeOTHER)?32767:256);
		test.step = vbound(atoi(spd),0, (test.family == eeFIRE || test.family == eeOTHER)?32767:1000);
		
		test.frate = vbound(atoi(frt),0,256);
		test.e_frate = vbound(atoi(efr),0,256);
		test.bosspal = vbound(atoi(bsp),-1,29);
		test.bgsfx = enedata_dlg[182].d1;
		test.hitsfx = enedata_dlg[183].d1;
		test.deadsfx = enedata_dlg[184].d1;
		
		// Sprites
		for(int32_t j=0; j<biw_cnt; j++)
		{
			if(enedata_dlg[236].d1 == j)
				test.wpnsprite = biw[j].i;
			if(enedata_dlg[394].d1 == j)
				test.spr_shadow = biw[j].i;
			if(enedata_dlg[396].d1 == j)
				test.spr_death = biw[j].i;
			if(enedata_dlg[398].d1 == j)
				test.spr_spawn = biw[j].i;
		}
		
		test.attributes[0] = (enedata_dlg[64].proc == jwin_droplist_proc) ? enedata_dlg[64].d1 : atol(attribs[0]);
		test.attributes[1] = (enedata_dlg[65].proc==jwin_droplist_proc) ? enedata_dlg[65].d1 : atol(attribs[1]);
		test.attributes[2] = (enedata_dlg[66].proc==jwin_droplist_proc) ? enedata_dlg[66].d1 : atol(attribs[2]);
		test.attributes[3] = (enedata_dlg[67].proc==jwin_droplist_proc) ? enedata_dlg[67].d1 : atol(attribs[3]);
		test.attributes[4] = (enedata_dlg[68].proc==jwin_droplist_proc) ? enedata_dlg[68].d1 : atol(attribs[4]);
		test.attributes[5] = (enedata_dlg[69].proc==jwin_droplist_proc) ? enedata_dlg[69].d1 : atol(attribs[5]);
		test.attributes[6] = (enedata_dlg[70].proc==jwin_droplist_proc) ? enedata_dlg[70].d1 : atol(attribs[6]);
		test.attributes[7] = (enedata_dlg[71].proc==jwin_droplist_proc) ? enedata_dlg[71].d1 : atol(attribs[7]);
		test.attributes[8] = (enedata_dlg[72].proc==jwin_droplist_proc) ? enedata_dlg[72].d1 : atol(attribs[8]);
		test.attributes[9] = (enedata_dlg[73].proc==jwin_droplist_proc) ? enedata_dlg[73].d1 : atol(attribs[9]);
		test.attributes[10] = (enedata_dlg[296].proc==jwin_droplist_proc) ? enedata_dlg[296].d1 : atol(attribs[10]);
		test.attributes[11] = (enedata_dlg[297].proc==jwin_droplist_proc) ? enedata_dlg[297].d1 : atol(attribs[11]);
		test.attributes[12] = (enedata_dlg[298].proc==jwin_droplist_proc) ? enedata_dlg[298].d1 : atol(attribs[12]);
		test.attributes[13] = (enedata_dlg[299].proc==jwin_droplist_proc) ? enedata_dlg[299].d1 : atol(attribs[13]);
		test.attributes[14] = (enedata_dlg[300].proc==jwin_droplist_proc) ? enedata_dlg[300].d1 : atol(attribs[14]);
		test.attributes[15] = (enedata_dlg[301].proc==jwin_droplist_proc) ? enedata_dlg[301].d1 : atol(attribs[15]);
		test.attributes[16] = (enedata_dlg[310].proc==jwin_droplist_proc) ? enedata_dlg[310].d1 : atol(attribs[16]);
		test.attributes[17] = (enedata_dlg[311].proc==jwin_droplist_proc) ? enedata_dlg[311].d1 : atol(attribs[17]);
		test.attributes[18] = (enedata_dlg[312].proc==jwin_droplist_proc) ? enedata_dlg[312].d1 : atol(attribs[18]);
		test.attributes[19] = (enedata_dlg[313].proc==jwin_droplist_proc) ? enedata_dlg[313].d1 : atol(attribs[19]);
		test.attributes[20] = (enedata_dlg[314].proc==jwin_droplist_proc) ? enedata_dlg[314].d1 : atol(attribs[20]);
		test.attributes[21] = (enedata_dlg[315].proc==jwin_droplist_proc) ? enedata_dlg[315].d1 : atol(attribs[21]);
		test.attributes[22] = (enedata_dlg[316].proc==jwin_droplist_proc) ? enedata_dlg[316].d1 : atol(attribs[22]);
		test.attributes[23] = (enedata_dlg[317].proc==jwin_droplist_proc) ? enedata_dlg[317].d1 : atol(attribs[23]);
		test.attributes[24] = (enedata_dlg[326].proc==jwin_droplist_proc) ? enedata_dlg[326].d1 : atol(attribs[24]);
		test.attributes[25] = (enedata_dlg[327].proc==jwin_droplist_proc) ? enedata_dlg[327].d1 : atol(attribs[25]);
		test.attributes[26] = (enedata_dlg[328].proc==jwin_droplist_proc) ? enedata_dlg[328].d1 : atol(attribs[26]);
		test.attributes[27] = (enedata_dlg[329].proc==jwin_droplist_proc) ? enedata_dlg[329].d1 : atol(attribs[27]);
		test.attributes[28] = (enedata_dlg[330].proc==jwin_droplist_proc) ? enedata_dlg[330].d1 : atol(attribs[28]);
		test.attributes[29] = (enedata_dlg[331].proc==jwin_droplist_proc) ? enedata_dlg[331].d1 : atol(attribs[29]);
		test.attributes[30] = (enedata_dlg[332].proc==jwin_droplist_proc) ? enedata_dlg[332].d1 : atol(attribs[30]);
		test.attributes[31] = (enedata_dlg[333].proc==jwin_droplist_proc) ? enedata_dlg[333].d1 : atol(attribs[31]);
		
	
	
		for(int32_t j=0; j <= edefBYRNA; j++)
		{
			test.defense[j] = enedata_dlg[j+161].d1;
		}
		
		test.defense[edefWhistle] = enedata_dlg[192].d1;
		test.defense[edefSwitchHook] = enedata_dlg[416].d1;
		//Are the new defs missing here? -Z
		
		
		for(int32_t i=0; i<32; i++)
			test.flags |= (guy_flags)((enedata_dlg[74+i].flags & D_SELECTED) ? (1<<i) : 0);
			
		test.flags &= ~(guy_fade_instant|guy_fade_flicker);
		test.flags |= (guy_flags)(enedata_dlg[186].d1==2 ? guy_fade_instant : enedata_dlg[186].d1==1 ? guy_fade_flicker : 0);
		
		for(int32_t i=0; i<16; i++)
			test.flags |= (guy_flags)((enedata_dlg[106+i].flags & D_SELECTED) ? (1<<i) : 0);
		
		for(int32_t i=0; i<16; i++)
			test.flags |= (guy_flags)((enedata_dlg[399+i].flags & D_SELECTED) ? (1<<(i+16)) : 0);
			
		if(enedata_dlg[143].flags & D_SELECTED)
		{
			test.cset = 14;
		}
		//if we disable the box, revert to cset 8 -Z.
		else if(guysbuf[index].cset == 14 || test.cset == 14)
		{
			test.cset = 8;
		}
		test.defense[edefSCRIPT01] = enedata_dlg[203].d1;
		test.defense[edefSCRIPT02] = enedata_dlg[204].d1;
		test.defense[edefSCRIPT03] = enedata_dlg[205].d1;
		test.defense[edefSCRIPT04] = enedata_dlg[206].d1;
		test.defense[edefSCRIPT05] = enedata_dlg[207].d1;
		test.defense[edefSCRIPT06] = enedata_dlg[208].d1;
		test.defense[edefSCRIPT07] = enedata_dlg[209].d1;
		test.defense[edefSCRIPT08] = enedata_dlg[210].d1;
		test.defense[edefSCRIPT09] = enedata_dlg[211].d1;
		test.defense[edefSCRIPT10] = enedata_dlg[212].d1;
		
		//tilewidth, tileheight, hitwidth, hitheight, 
		test.txsz = atoi(tilex);
		test.tysz = atoi(tiley);
		test.hxsz = atoi(hitx);
		test.hysz = atoi(hity);
		test.hzsz = atoi(hitz);
		test.hxofs = atoi(hitofsx);
		test.hyofs = atoi(hitofsy);
		test.zofs = atoi(hitofsz);
		test.xofs = atoi(drawofsx);
		test.yofs = atoi(drawofsy); //This seems to be setting to +48 or something with any value set?! -Z
		
		//override flags
		if(enedata_dlg[237].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_HIT_WIDTH;
			
		if(enedata_dlg[238].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_HIT_HEIGHT;
			
		if(enedata_dlg[239].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_HIT_Z_HEIGHT;
			
		if(enedata_dlg[240].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_HIT_X_OFFSET;
			
		if(enedata_dlg[241].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_HIT_Y_OFFSET;
			
		if(enedata_dlg[242].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_DRAW_Z_OFFSET;
		if(enedata_dlg[243].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_DRAW_X_OFFSET;
			
		if(enedata_dlg[244].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_DRAW_Y_OFFSET;
		if(enedata_dlg[245].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_TILE_WIDTH;
		if(enedata_dlg[246].flags & D_SELECTED)
			test.SIZEflags |= guyflagOVERRIDE_TILE_HEIGHT;

		//new 2.55 misc flags 1 to 16
		if(enedata_dlg[254].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG1;
		if(enedata_dlg[255].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG2;
		if(enedata_dlg[256].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG3;
		if(enedata_dlg[257].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG4;
		if(enedata_dlg[258].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG5;
		if(enedata_dlg[259].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG6;
		if(enedata_dlg[260].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG7;
		if(enedata_dlg[261].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG8;
		if(enedata_dlg[262].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG9;
		if(enedata_dlg[263].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG10;
		if(enedata_dlg[264].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG11;
		if(enedata_dlg[265].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG12;
		if(enedata_dlg[266].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG13;
		if(enedata_dlg[267].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG14;
		if(enedata_dlg[268].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG15;
		if(enedata_dlg[269].flags & D_SELECTED)
			test.editorflags |= ENEMY_FLAG16;
		
		//begin npc scripts
		test.script = binpcs[enedata_dlg[335].d1].second + 1; 
		for ( int32_t q = 0; q < 8; q++ )
		{
			test.initD[q] = enedata_dlg[345+q].fg;
			test.weap_initiald[q] = enedata_dlg[361+q].fg;
		}
		//eweapon script
		test.weaponscript = bieweapons[enedata_dlg[370].d1].second + 1; 
		
		test.moveflags = move_none;
		if(enedata_dlg[371].flags & D_SELECTED)
			test.moveflags |= move_obeys_grav;
		if(enedata_dlg[372].flags & D_SELECTED)
			test.moveflags |= move_can_pitfall;
		if(enedata_dlg[373].flags & D_SELECTED)
			test.moveflags |= move_can_pitwalk;
		if(enedata_dlg[374].flags & D_SELECTED)
			test.moveflags |= move_can_waterdrown;
		if(enedata_dlg[375].flags & D_SELECTED)
			test.moveflags |= move_can_waterwalk;
		if(enedata_dlg[417].flags & D_SELECTED)
			test.moveflags |= move_only_waterwalk;
		if(enedata_dlg[418].flags & D_SELECTED)
			test.moveflags |= move_only_shallow_waterwalk;
		if(enedata_dlg[419].flags & D_SELECTED)
			test.moveflags |= move_only_pitwalk;
		if(enedata_dlg[420].flags & D_SELECTED)
			test.moveflags |= move_ignore_solidity;
		if(enedata_dlg[421].flags & D_SELECTED)
			test.moveflags |= move_ignore_blockflags;
		if(enedata_dlg[422].flags & D_SELECTED)
			test.moveflags |= move_ignore_screenedge;
		if(enedata_dlg[423].flags & D_SELECTED)
			test.moveflags |= move_new_movement;
		if(enedata_dlg[424].flags & D_SELECTED)
			test.moveflags |= move_not_pushable;
	
		//end npc scripts
	
		if(ret==252) //OK Button
		{
			strcpy(guy_string[index],name);
			guysbuf[index] = test;
			saved = false;
		}
		else if(ret==46)
		{
			setEnemyLabels(bief[enedata_dlg[46].d1].i);
		}
		else if(ret==178)
		{
			for(int32_t j=1; j <= edefBYRNA; j++)
			{
				enedata_dlg[j+161].d1 = enedata_dlg[161].d1;
			}
			
			enedata_dlg[192].d1 = enedata_dlg[161].d1;
			 //Clear to 0
			enedata_dlg[203].d1 = enedata_dlg[161].d1;
			enedata_dlg[204].d1 = enedata_dlg[161].d1;
			enedata_dlg[205].d1 = enedata_dlg[161].d1;
			enedata_dlg[206].d1 = enedata_dlg[161].d1;
			enedata_dlg[207].d1 = enedata_dlg[161].d1;
			enedata_dlg[208].d1 = enedata_dlg[161].d1;
			enedata_dlg[209].d1 = enedata_dlg[161].d1;
			enedata_dlg[210].d1 = enedata_dlg[161].d1;
			enedata_dlg[211].d1 = enedata_dlg[161].d1;
			enedata_dlg[212].d1 = enedata_dlg[161].d1;
		}
		else if (ret == 335 || ret == 370) // Script dropdowns
		{
			setEnemyScriptLabels(test, npc_initd_labels, weapon_initd_labels);
		}
		else if (ret >= 426 && ret <= 441) // InitD help buttons
		{
			showEnemyScriptMetaHelp(test, ret - 426);
		}
	}
	//252 == 	OK button, 253 == Cancel Button
	while(ret != 252 && ret != 253 && ret != 0);
	popup_zqdialog_end();
}

extern DIALOG elist_dlg[];
static int32_t copiedGuy;

int32_t readonenpc(PACKFILE *f, int32_t index)
{
	dword section_version = 0;
	dword section_cversion = 0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	guydata tempguy;
	memset(&tempguy, 0, sizeof(guydata));
		//reset_itembuf(&tempitem,i);
	
   
	char npcstring[64]={0}; //guy_string[]
	//section version info
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	
	if(!p_igetw(&section_cversion,f))
	{
		return 0;
	}
	al_trace("readonenpc section_version: %d\n", section_version);
	al_trace("readonenpc section_cversion: %d\n", section_cversion);
	
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .znpc packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	else if ( ( section_version > V_GUYS ) || ( section_version == V_GUYS && section_cversion < CV_GUYS ) )
	{
		al_trace("Cannot read .znpc packfile made using V_GUYS (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .znpc packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
   
	if(!pfread(&npcstring, 64, f))
	{
		return 0;
	}
	
	//section data
	uint32_t flags1;
	uint32_t flags2;
	if (!p_igetl(&(flags1), f))
	{
		return qe_invalid;
	}
	if (!p_igetl(&(flags2), f))
	{
		return qe_invalid;
	}
	tempguy.flags = guy_flags(flags1) | guy_flags(uint64_t(flags2) << 32ULL);

	if(!p_igetl(&tempguy.tile,f))
	{
	return 0;
	}
			
	if(!p_getc(&tempguy.width,f))
	{
	return 0;
	}

	if(!p_getc(&tempguy.height,f))
	{
	return 0;
	}

	if(!p_igetl(&tempguy.s_tile,f))
	{
	return 0;
	}

	if(!p_getc(&tempguy.s_width,f))
	{
	return 0;
	}

	if(!p_getc(&tempguy.s_height,f))
	{
	return 0;
	}

	if(!p_igetl(&tempguy.e_tile,f))
	{
	return 0;
	}

	if(!p_getc(&tempguy.e_width,f))
	{
	return 0;
	}

	if(!p_getc(&tempguy.e_height,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.hp,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.family,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.cset,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.anim,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.e_anim,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.frate,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.e_frate,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.dp,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.wdp,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.weapon,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.rate,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.hrate,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.step,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.homing,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.grumble,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.item_set,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.bgsfx,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.bosspal,f))
	{
	return 0;
	}

	if(!p_igetw(&tempguy.extend,f))
	{
	return 0;
	}

	for(int32_t j=0; j < edefLAST; j++)
	{
	if(!p_getc(&tempguy.defense[j],f))
	{
		return 0;
	}
	}
			
	if(!p_getc(&tempguy.hitsfx,f))
	{
	return 0;
	}
			
	if(!p_getc(&tempguy.deadsfx,f))
	{
	return 0;
	}
			
	if(!p_igetl(&tempguy.attributes[10],f))
	{
	return 0;
	}
			
	if(!p_igetl(&tempguy.attributes[11],f))
	{
	return 0;
	}
	
	if ( zversion >= 0x255 )
	{
		if  ( section_version >= 41 )
		{
			//New itemdata vars -Z
			//! version 27
			
			//2.55 starts here
			for(int32_t j=edefLAST; j < edefLAST255; j++)
			{
			if(!p_getc(&tempguy.defense[j],f))
			{
				return 0;
			}
			}
			
			//tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
			if(!p_igetl(&tempguy.txsz,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.tysz,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.hxsz,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.hysz,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.hzsz,f))
			{
			return 0;
			}
			// These are not fixed types, but ints, so they are safe to use here. 
			if(!p_igetl(&tempguy.hxofs,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.hyofs,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.xofs,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.yofs,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.zofs,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.wpnsprite,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.SIZEflags,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.frozentile,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.frozencset,f))
			{
			return 0;
			}
			if(!p_igetl(&tempguy.frozenclock,f))
			{
			return 0;
			}
			
			for ( int32_t q = 0; q < 10; q++ ) 
			{
			if(!p_igetw(&tempguy.frozenmisc[q],f))
			{
				return 0;
			}
			}
			if(!p_igetw(&tempguy.firesfx,f))
			{
			return 0;
			}
			for (int32_t q = 0; q < 32; q++)
			{
				if(!p_igetl(&tempguy.movement[q],f))
				{
					return 0;
				}
			}
			for ( int32_t q = 0; q < 32; q++ )
			{
				if(!p_igetl(&tempguy.new_weapon[q],f))
				{
				return 0;
				}
			}
			if(!p_igetw(&tempguy.script,f))
			{
			return 0;
			}
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&tempguy.initD[q],f))
			{
				return 0;
			}
			}
			for ( int32_t q = 0; q < 2; q++ )
			{
			if(!p_igetl(&tempguy.initA[q],f))
			{
				return 0;
			}
			}
			if(!p_igetl(&tempguy.editorflags,f))
			{
			return 0;
			}
			
			//Enemy Editor InitD[] labels
			for ( int32_t q = 0; q < 8; q++ )
			{
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&tempguy.initD_label[q][w],f))
					{
						return 0;
					}
				}
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&tempguy.weapon_initD_label[q][w],f))
					{
						return 0;
					}
				}
			}
			if(!p_igetw(&tempguy.weaponscript,f))
			{
			return 0;
			}
			//eweapon initD
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&tempguy.weap_initiald[q],f))
			{
				return 0;
			}
			}
			//enemy editor misc merge
			for (int32_t q = 0; q < 32; q++)
			{
				if (!p_igetl(&tempguy.attributes[q], f)) return 0;
			}
		}
	}
	memcpy(&guysbuf[index], &tempguy, sizeof(guydata));
	//strcpy(item_string[index], istring);
	guysbuf[bie[index].i] = tempguy;
	strcpy(guy_string[bie[index].i], npcstring);
	   
	return 1;
}

int32_t writeonenpc(PACKFILE *f, int32_t i)
{
	
	dword section_version=V_GUYS;
	dword section_cversion=CV_GUYS;
	int32_t zversion = ZELDA_VERSION;
	int32_t zbuild = VERSION_BUILD;
	
  
	//section version info
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		new_return(2);
	}
	
	if(!p_iputw(section_cversion,f))
	{
		new_return(3);
	}
	
	if(!pfwrite(guy_string[i], 64, f))
			{
				new_return(5);
			}
		
		uint32_t flags1 = uint32_t(guysbuf[i].flags);
		uint32_t flags2 = uint32_t(guysbuf[i].flags >> 32ULL);
		if (!p_iputl(flags1, f))
		{
			return 0;
		}
		if (!p_iputl(flags2, f))
		{
			return 0;
		}

		if(!p_iputl(guysbuf[i].tile,f))
		{
		return 0;
		}
		
		if(!p_putc(guysbuf[i].width,f))
		{
		return 0;
		}
		
		if(!p_putc(guysbuf[i].height,f))
		{
		return 0;
		}
		
		if(!p_iputl(guysbuf[i].s_tile,f))
		{
		return 0;
		}
		
		if(!p_putc(guysbuf[i].s_width,f))
		{
		return 0;
		}
		
		if(!p_putc(guysbuf[i].s_height,f))
		{
		return 0;
		}
		
		if(!p_iputl(guysbuf[i].e_tile,f))
		{
		return 0;
		}
		
		if(!p_putc(guysbuf[i].e_width,f))
		{
		return 0;
		}
		
		if(!p_putc(guysbuf[i].e_height,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].hp,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].family,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].cset,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].anim,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].e_anim,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].frate,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].e_frate,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].dp,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].wdp,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].weapon,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].rate,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].hrate,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].step,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].homing,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].grumble,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].item_set,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].bgsfx,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].bosspal,f))
		{
		return 0;
		}
		
		if(!p_iputw(guysbuf[i].extend,f))
		{
		return 0;
		}
		
		for(int32_t j=0; j < edefLAST; j++)
		{
		if(!p_putc(guysbuf[i].defense[j],f))
		{
		   return 0;
		}
		}
		
		if(!p_putc(guysbuf[i].hitsfx,f))
		{
		return 0;
		}
		
		if(!p_putc(guysbuf[i].deadsfx,f))
		{
		return 0;
		}
		
		//2.55 starts here
		for(int32_t j=edefLAST; j < edefLAST255; j++)
		{
		if(!p_putc(guysbuf[i].defense[j],f))
		{
			return 0;
		}
		}
		
		//tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
		if(!p_iputl(guysbuf[i].txsz,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].tysz,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].hxsz,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].hysz,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].hzsz,f))
		{
		return 0;
		}
		// These are not fixed types, but ints, so they are safe to use here. 
		if(!p_iputl(guysbuf[i].hxofs,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].hyofs,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].xofs,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].yofs,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].zofs,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].wpnsprite,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].SIZEflags,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].frozentile,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].frozencset,f))
		{
		return 0;
		}
		if(!p_iputl(guysbuf[i].frozenclock,f))
		{
		return 0;
		}
		
		for ( int32_t q = 0; q < 10; q++ ) 
		{
		if(!p_iputw(guysbuf[i].frozenmisc[q],f))
		{
			return 0;
		}
		}
		if(!p_iputw(guysbuf[i].firesfx,f))
		{
		return 0;
		}
		for ( int32_t q = 0; q < 32; q++ )
		{
			if(!p_iputl(guysbuf[i].movement[q],f))
			{
			return 0;
			}
		}
		for ( int32_t q = 0; q < 32; q++ )
		{
			if(!p_iputl(guysbuf[i].new_weapon[q],f))
			{
			return 0;
			}
		}
		if(!p_iputw(guysbuf[i].script,f))
		{
		return 0;
		}
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(guysbuf[i].initD[q],f))
		{
			return 0;
		}
		}
		for ( int32_t q = 0; q < 2; q++ )
		{
		if(!p_iputl(guysbuf[i].initA[q],f))
		{
			return 0;
		}
		}
		if(!p_iputl(guysbuf[i].editorflags,f))
		{
		return 0;
		}

		//Enemy Editor InitD[] labels
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(guysbuf[i].initD_label[q][w],f))
				{
					return 0;
				}
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(guysbuf[i].weapon_initD_label[q][w],f))
				{
					return 0;
				}
			}
		}
		if(!p_iputw(guysbuf[i].weaponscript,f))
		{
		return 0;
		}
		//eweapon initD
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(guysbuf[i].weap_initiald[q],f))
		{
			return 0;
		}
		}
		//enemy editor misc merge
		for (int32_t q = 0; q < 32; q++)
		{
			if (!p_iputl(guysbuf[i].attributes[q], f))
			{
				return 0;
			}
		}
	return 1;
}






void paste_enemy(int32_t index)
{
	if(index < 0) index = elist_dlg[2].d1;
	if(index==0)
		return;
	if(copiedGuy<0) //Nothing copied
		return;
	guysbuf[bie[index].i]=guysbuf[copiedGuy];
	elist_dlg[2].flags|=D_DIRTY;
	saved=false;
}
void copy_enemy(int32_t index)
{
	if(index < 0) index = elist_dlg[2].d1;
	if(index==0)
		return;
	copiedGuy=bie[index].i;
}
void save_enemy(int32_t index)
{
	if(index < 0) index = elist_dlg[2].d1;
	if(index==0)
		return;
	if(!prompt_for_new_file_compat("Save NPC(.znpc)", "znpc", NULL,datapath,false))
		return;
	int32_t iid = bie[index].i;
	
	PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
	if(!f) return;
	if (!writeonenpc(f,iid))
	{
		Z_error("Could not write to .znpc packfile %s\n", temppath);
		InfoDialog("ZNPC Error", "Could not save the specified enemy.").show();
	}
	pack_fclose(f);
}
void load_enemy(int32_t index)
{
	if(index < 0) index = elist_dlg[2].d1;
	if(index==0)
		return;
	if(!prompt_for_existing_file_compat("Load NPC(.znpc)", "znpc", NULL,datapath,false))
		return;
	PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
	if(!f) return;
	
	if (!readonenpc(f,index))
	{
		al_trace("Could not read from .znpc packfile %s\n", temppath);
		InfoDialog("ZNPC Error", "Could not load the specified enemy.").show();
	}
	
	pack_fclose(f);
	elist_dlg[2].flags|=D_DIRTY;
	saved=false;
}


void elist_rclick_func(int32_t index, int32_t x, int32_t y)
{
	if(index == 0)
		return;
	
	NewMenu rcmenu {
		{ "&Copy", [&](){copy_enemy(index);} },
		{ "Paste", "&v", [&](){paste_enemy(index);}, 0, copiedGuy <= 0 },
		{ "&Save", [&](){save_enemy(index);} },
		{ "&Load", [&](){load_enemy(index);} },
	};
	rcmenu.pop(x, y);
}

int32_t onCustomEnemies()
{
	/*
	  char *hold = item_string[0];
	  item_string[0] = "rupee (1)";
	  */
	
	int32_t foo;
	int32_t index = select_enemy("Select Enemy",bie[0].i,true,true,foo);
	
	while(index >= 0)
	{
		//I can't get the fucking dialog to handle a simple copy paste so I stuck it here else I'm going to rage kill something.
		//,,.Someone feel free to fix the thing properly later on.
		//Right now creating custom enemies remains a int32_t painful process, but it shouldn't be this hard for users to use.
		//-Two cents worth. -Gleeok
		if(key[KEY_OPENBRACE])   //copy
		{
			if(index != 0)
				copiedGuy=index;
		}
		else if(key[KEY_CLOSEBRACE])   //paste
		{
			if(copiedGuy>0 && index!=0)
			{
				guysbuf[index]=guysbuf[copiedGuy];
				saved=false;
			}
		}
		else
		{
			if(index != 0)
			{
				edit_enemydata(index);
			}
		}
		
		index = select_enemy("Select Enemy",index,true,true,foo);
	}
	
	refresh(rMAP+rCOMBOS);
	return D_O_K;
}


int32_t onCustomGuys()
{
	return D_O_K;
}

int32_t d_ltile_proc(int32_t msg,DIALOG *d,int32_t c);
static int32_t herotile_land_walk_list[] =
{
	// dialog control number
	11, 12, 13, 14, 15, 16, 17, 18, -1
};

static int32_t herotile_land_slash_list[] =
{
	// dialog control number
	19, 20, 21, 22, 23, 24, 25, 26, -1
};

static int32_t herotile_land_stab_list[] =
{
	// dialog control number
	27, 28, 29, 30, 31, 32, 33, 34,  -1
};

static int32_t herotile_land_pound_list[] =
{
	// dialog control number
	35, 36, 37, 38, 39, 40, 41, 42, -1
};

static int32_t herotile_land_hold_list[] =
{
	// dialog control number
	43, 44, 45, 46, -1
};

static int32_t herotile_land_cast_list[] =
{
	// dialog control number
	47, -1
};

static int32_t herotile_land_fall_list[] =
{
	// dialog control number
	105, 106, 107, 108, 109, 110, 111, 112, -1
};

static int32_t herotile_land_liftwalk_list[] =
{
	// dialog control number
	260, 261, 262, 263, 264, 265, 266, 267, -1
};
static int32_t herotile_land_lift_list[] =
{
	// dialog control number
	268, 269, 270, 271, 272, 273, 274, 275,
	276, 277, 278, 279, 280, 281, 282, 283,
	284, 285, -1
};

static int32_t herotile_land_jump_list[] =
{
	// dialog control number
	77, 78, 79, 80, 81, 82, 83, 84, -1
};

static int32_t herotile_land_charge_list[] =
{
	// dialog control number
	85, 86, 87, 88, 89, 90, 91, 92, -1
};

static int32_t herotile_land_revslash_list[] =
{
	// dialog control number
	252, 253, 254, 255, 256, 257, 258, 259, -1
};

static TABPANEL herotile_land_tabs[] =
{
	// (text)
	{ (char *)"Walk",          D_SELECTED,  herotile_land_walk_list, 0, NULL },
	{ (char *)"Slash",         0,           herotile_land_slash_list, 0, NULL },
	{ (char *)"Slash 2",       0,           herotile_land_revslash_list, 0, NULL },
	{ (char *)"Stab",          0,           herotile_land_stab_list, 0, NULL },
	{ (char *)"Pound",         0,           herotile_land_pound_list, 0, NULL },
	{ (char *)"Jump",          0,           herotile_land_jump_list, 0, NULL },
	{ (char *)"Charge",        0,           herotile_land_charge_list, 0, NULL },
	{ (char *)"Hold",          0,           herotile_land_hold_list, 0, NULL },
	{ (char *)"Cast",          0,           herotile_land_cast_list, 0, NULL },
	{ (char *)"Falling",       0,           herotile_land_fall_list, 0, NULL },
	{ (char *)"Lifting",       0,           herotile_land_lift_list, 0, NULL },
	{ (char *)"Lift+Walk",     0,           herotile_land_liftwalk_list, 0, NULL },
	{ NULL,                    0,           NULL,                   0, NULL }
};

static int32_t herotile_water_float_list[] =
{
	// dialog control number
	48, 49, 50, 51, 52, 53, 54, 55, -1
};

static int32_t herotile_water_swim_list[] =
{
	// dialog control number
	56, 57, 58, 59, 60, 61, 62, 63, -1
};

static int32_t herotile_water_dive_list[] =
{
	// dialog control number
	64, 65, 66, 67, 68, 69, 70, 71, -1
};

static int32_t herotile_water_hold_list[] =
{
	// dialog control number
	72, 73, 74, 75, -1
};

static int32_t herotile_water_drown_list[] =
{
	// dialog control number
	97, 98, 99, 100, 101, 102, 103, 104, -1
};

static int32_t herotile_lava_drown_list[] =
{
	// dialog control number
	113, 114, 115, 116, 117, 118, 119, 120, -1
};

static TABPANEL herotile_water_tabs[] =
{
	// (text)
	{ (char *)"Float",      D_SELECTED,  herotile_water_float_list, 0, NULL },
	{ (char *)"Swim",       0,           herotile_water_swim_list, 0, NULL },
	{ (char *)"Dive",       0,           herotile_water_dive_list, 0, NULL },
	{ (char *)"Drown",      0,           herotile_water_drown_list, 0, NULL },
	{ (char *)"Hold",       0,           herotile_water_hold_list, 0, NULL },
	{ (char *)"Lava Drown", 0,           herotile_lava_drown_list, 0, NULL },
	{ NULL,                 0,           NULL,                     0, NULL }
};

static int32_t herotile_sidewater_swim_list[] =
{
	// dialog control number
	196, 197, 198, 199, 200, 201, 202, 203, -1
};

static int32_t herotile_sidewater_slash_list[] =
{
	// dialog control number
	204, 205, 206, 207, 208, 209, 210, 211, -1
};

static int32_t herotile_sidewater_stab_list[] =
{
	// dialog control number
	212, 213, 214, 215, 216, 217, 218, 219, -1
};

static int32_t herotile_sidewater_pound_list[] =
{
	// dialog control number
	220, 221, 222, 223, 224, 225, 226, 227, 236, 237, 238, 239, 240, 241, 242, 243,-1
};

static int32_t herotile_sidewater_charge_list[] =
{
	// dialog control number
	228, 229, 230, 231, 232, 233, 234, 235, -1
};

static int32_t herotile_sidewater_hold_list[] =
{
	// dialog control number
	244,245,246,247,248,249,250,251, -1
};

static TABPANEL herotile_sidewater_tabs[] =
{
	// (text)
	{ (char *)"Swim",       D_SELECTED,           herotile_sidewater_swim_list, 0, NULL },
	{ (char *)"S. Slash",       0,           herotile_sidewater_slash_list, 0, NULL },
	{ (char *)"S. Stab",      0,           herotile_sidewater_stab_list, 0, NULL },
	{ (char *)"S. Pound",       0,           herotile_sidewater_pound_list, 0, NULL },
	{ (char *)"S. Charge", 0,           herotile_sidewater_charge_list, 0, NULL },
	{ (char *)"S. Misc", 0,           herotile_sidewater_hold_list, 0, NULL },
	{ NULL,                 0,           NULL,                     0, NULL }
};

static int32_t herotile_defense_enemy1_list[] =
{
	//dialog control number
	122, 123, 124, 125, 126, 127, 128, 129, 130, 145, 146, 147, 148, 149, 150, 151, 152, 153, 168, -1
};

static int32_t herotile_defense_enemy2_list[] =
{
	//dialog control number
	131, 132, 133, 134, 135, 136, 137, 138, 154, 155, 156, 157, 158, 159, 160, 161, -1
};

static int32_t herotile_defense_other1_list[] =
{
	//dialog control number
	139, 140, 141, 142, 143, 144, 162, 163, 164, 165, 166, 167, -1
};

static int32_t herotile_defense_script_list[] =
{
	//dialog control number
	169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, -1
};

static TABPANEL herotile_defense_tabs[] =
{
	// (text)
	{ (char*)"Enemy 1",     D_SELECTED,  herotile_defense_enemy1_list, 0, NULL },
	{ (char*)"Enemy 2",     0,           herotile_defense_enemy2_list, 0, NULL },
	{ (char*)"Other",       0,           herotile_defense_other1_list, 0, NULL },
	{ (char*)"Script",      0,           herotile_defense_script_list, 0, NULL },
	{ NULL,                 0,           NULL,                     0, NULL }
};


static int32_t herotile_land_list[] =
{
	// dialog control number
	9, -1
};

static int32_t herotile_water_list[] =
{
	// dialog control number
	10, -1
};

static int32_t herotile_sidewater_list[] =
{
	// dialog control number
	195, -1
};

static int32_t herotile_defense_list[] =
{
	// dialog control number
	121, -1
};

static int32_t herotile_option_list[] =
{
	// dialog control number
	189, 190, 191, 192, 193, 194, -1
};

static TABPANEL herotile_tabs[] =
{
	// (text)
	{ (char *)"Sprites (Land)",       D_SELECTED,   herotile_land_list, 0, NULL },
	{ (char *)"Sprites (Liquid)",      0,            herotile_water_list, 0, NULL },
	{ (char *)"Sprites (Side Liquid)",      0,            herotile_sidewater_list, 0, NULL },
	{ (char *)"Defenses",             0,            herotile_defense_list, 0, NULL},
	{ (char *)"Options",              0,            herotile_option_list, 0, NULL  }, 
	{ NULL,                 0,            NULL,                0, NULL }
};

const char *animationstyles[las_max]= { "Original", "BS-Zelda", "Zelda 3", "Zelda 3 (Slow Walk)" };

const char *animationstylelist(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return animationstyles[index];
	}
	
	*list_size=las_max;
	return NULL;
}

const char *swimspeeds[2]= { "Slow", "Fast" };

const char *swimspeedlist(int32_t index, int32_t *list_size)
{
	if(index>=0)
	{
		return swimspeeds[index];
	}
	
	*list_size=2;
	return NULL;
}

int32_t jwin_as_droplist_proc(int32_t msg,DIALOG *d,int32_t c)
{
	int32_t ret = jwin_droplist_proc(msg,d,c);
	
	switch(msg)
	{
	case MSG_CHAR:
	case MSG_CLICK:
		zinit.heroAnimationStyle=d->d1;
		
		if(zinit.heroAnimationStyle==las_zelda3slow)
		{
			hero_animation_speed=2;
		}
		else
		{
			hero_animation_speed=1;
		}
	}
	
	return ret;
}

static ListData animationstyle_list(animationstylelist, &font);
static ListData swimspeed_list(swimspeedlist, &font);

static DIALOG herotile_dlg[] =
{
	// (dialog proc)                       (x)     (y)     (w)     (h)    (fg)                     (bg)                 (key)     (flags)    (d1)        (d2)              (dp)                             (dp2)   (dp3)
	{  jwin_win_proc,                        0,      0,    320,    240,    vc(14),                 vc(1),                   0,    D_EXIT,     0,          0, (void *) "Player Sprites",         NULL,   NULL                   },
	{  d_vsync_proc,                         0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   },
	{  d_keyboard_proc,                      0,      0,      0,      0,    0,                      0,                       0,    0,          KEY_F1,     0, (void *) onHelp,                 NULL,   NULL                   },
	{  jwin_button_proc,                    90,    220,     61,     21,    vc(14),                 vc(1),                  13,    D_EXIT,     0,          0, (void *) "OK",                   NULL,   NULL                   },
	{  jwin_button_proc,                   170,    220,     61,     21,    vc(14),                 vc(1),                  27,    D_EXIT,     0,          0, (void *) "Cancel",               NULL,   NULL                   },
	// 5
	{  d_dummy_proc,                       217,    200,      0,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Large Player Hit Box",   NULL,   NULL                   },
	{  d_dummy_proc,                         4,    201,     17,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Animation Style:",     NULL,   NULL                   },
	{  d_dummy_proc,                        77,    197,     78,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,          0,          0, (void *) &animationstyle_list,   NULL,   NULL                   },
	{  jwin_tab_proc,                        4,     17,    312,    200,    0,                      0,                       0,    0,          0,          0, (void *) herotile_tabs,          NULL, (void *)herotile_dlg   },
	// 9
	{  jwin_tab_proc,                        7,     33,    305,    183,    0,                      0,                       0,    0,          0,          0, (void *) herotile_land_tabs,     NULL, (void *)herotile_dlg   },
	{  jwin_tab_proc,                        7,     33,    305,    183,    0,                      0,                       0,    0,          0,          0, (void *) herotile_water_tabs,    NULL, (void *)herotile_dlg   },
	// 11 (walk sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 15 (walk sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_walk,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_walk,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_walk,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_walk,         NULL,                            NULL,   NULL                   },
	// 19 (slash spritetitles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 23 (slash sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_slash,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_slash,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_slash,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_slash,        NULL,                            NULL,   NULL                   },
	// 27 (stab sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 31 (stab sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_stab,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_stab,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_stab,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_stab,         NULL,                            NULL,   NULL                   },
	// 35 (pound sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 39 (pound sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_pound,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_pound,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_pound,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_pound,        NULL,                            NULL,   NULL                   },
	// 43 (hold sprite titles)
	{  jwin_rtext_proc,                     67,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "One Hand",             NULL,   NULL                   },
	{  jwin_rtext_proc,                     67,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Two Hands",            NULL,   NULL                   },
	// 45 (hold sprites)
	{  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_landhold1,    NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        70,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_landhold2,    NULL,                            NULL,   NULL                   },
	// 47 (casting sprites)
	{  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_cast,         NULL,                            NULL,   NULL                   },
	
	// 48 (float sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 52 (float sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_float,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_float,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_float,        NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_float,        NULL,                            NULL,   NULL                   },
	// 56 (swim sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 60 (swim sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_swim,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_swim,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_swim,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_swim,         NULL,                            NULL,   NULL                   },
	// 64 (dive sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 68 (dive sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_dive,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_dive,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_dive,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_dive,         NULL,                            NULL,   NULL                   },
	// 72 (hold sprite titles)
	{  jwin_rtext_proc,                     67,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "One Hand",             NULL,   NULL                   },
	{  jwin_rtext_proc,                     67,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Two Hands",            NULL,   NULL                   },
	// 74 (hold sprites)
	{  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_waterhold1,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        70,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_waterhold2,   NULL,                            NULL,   NULL                   },
	{  d_dummy_proc,                       217,    186,      0,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Diagonal Movement",    NULL,   NULL                   },
	// 77 (jump sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 81 (jump sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_jump,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_jump,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_jump,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_jump,         NULL,                            NULL,   NULL                   },
	// 85 (charge sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 89 (charge sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_charge,       NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_charge,       NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_charge,       NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_charge,       NULL,                            NULL,   NULL                   },
	// 93
	{  d_timer_proc,                         0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   },
	{  d_dummy_proc,                         4,    183,     17,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Swim Speed:",          NULL,   NULL                   },
	{  d_dummy_proc,                        77,    179,     78,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,          0,          0, (void *) &swimspeed_list,        NULL,   NULL                   },
	{  d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,          0, (void *) onSnapshot, NULL, NULL },
	// 97 (drown sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 101 (drown sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_drown,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_drown,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_drown,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_drown,         NULL,                            NULL,   NULL                   },
	// 105 (falling sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 109 (falling sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_falling,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_falling,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_falling,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_falling,         NULL,                            NULL,   NULL                   },

	// 113 (lavadrown sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 117 (lavadrown sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_lavadrown,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_lavadrown,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_lavadrown,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_lavadrown,         NULL,                            NULL,   NULL                   },
	
	// DEFENSE TAB BEGINS
	// 121 (Player defenses)
	{ jwin_tab_proc,                        7,      33,    305,    183,    0,                      0,                       0,    0,          0,          0, (void*)herotile_defense_tabs,    NULL, (void*)herotile_dlg },
	// 122 - Enemy weapons (currently 17)
	{ jwin_text_proc,           9,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Fireball Defense:",                                  NULL,   NULL },
	{ jwin_text_proc,           9,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Arrow Defense:",                                     NULL,   NULL },
	{ jwin_text_proc,           9,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Boomerang Defense:",                                 NULL,   NULL },
	{ jwin_text_proc,           9,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Sword Beam Defense:",                                NULL,   NULL },
	{ jwin_text_proc,           9,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Rock Defense:",                                      NULL,   NULL },
	{ jwin_text_proc,           9,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Magic Defense:",                                     NULL,   NULL },
	{ jwin_text_proc,           9,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Bomb (object) Defense:",                             NULL,   NULL },
	{ jwin_text_proc,           9,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"S. Bomb (object) Defense:",                          NULL,   NULL },
	{ jwin_text_proc,           9,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Bomb (explode) Defense:",                            NULL,   NULL },
	// 131 - Enemy weapons page 2
	{ jwin_text_proc,           9,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"S. Bomb (explode) Defense:",                          NULL,   NULL },
	{ jwin_text_proc,           9,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Flame Trail Defense:",                                 NULL,   NULL },
	{ jwin_text_proc,           9,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Flame Defense:",                                 NULL,   NULL },
	{ jwin_text_proc,           9,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Wind Defense:",                                NULL,   NULL },
	{ jwin_text_proc,           9,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Flame 2 Defense:",                             NULL,   NULL },
	{ jwin_text_proc,           9,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Flame 2 Trail Defense:",                                 NULL,   NULL },
	{ jwin_text_proc,           9,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Ice Defense:",                                NULL,   NULL },
	{ jwin_text_proc,           9,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Fireball 2 Defense:",                              NULL,   NULL },
	// 139 - Other weapons (Currently 6)
	{ jwin_text_proc,           9,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Candle Fire Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Player Bomb Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Refl. Magic Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Refl. Fireball Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Refl. Rock Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Refl. Sword Beam Defense:",                              NULL,   NULL },
	// 145 - Enemy weapons pulldown 
	{ jwin_droplist_proc,         126,   54 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,   72 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,   90 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  108 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  126 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  144 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  162 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  180 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  198 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	// 154 - Enemy weapons pulldown page 2
	{ jwin_droplist_proc,         126,   54 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,   72 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,   90 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  108 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  126 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  144 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  162 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  180 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	// 162 - Other weapon pulldown
	{ jwin_droplist_proc,         126,   54 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,   72 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,   90 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  108 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  126 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,         126,  144 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	// 168 - Set all button
	{ jwin_button_proc,           255,    54 - 4,     48,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void*)"Set All",                                            NULL,   NULL },
	
	
	// 169 - Script 1
	{ jwin_text_proc,           9,    51,      80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 1 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    67,      80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 2 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    83,      80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 3 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    99,      80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 4 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    115,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 5 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    131,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 6 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    147,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 7 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    163,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 8 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    179,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 9 Defense:",                              NULL,   NULL },
	{ jwin_text_proc,           9,    196,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void*)"Custom Weapon 10 Defense:",                              NULL,   NULL },
	//179 script 1 pulldown

		 /* (dialog proc)           (x)   (y)     (w)     (h)      (fg)                   (bg)                    (key) (flags)      (d1)  (d2)  (dp)                                                         (dp2)  (dp3) */
	{ jwin_droplist_proc,      126, 51 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 67 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 83 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 99 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 115 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 131 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 147 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 163 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 179 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	{ jwin_droplist_proc,      126, 196 - 4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void*)&defense_list,                                         NULL,   NULL },
	// 189 - Options relocated to handle new tab
	{ jwin_check_proc,                      9,    78,      0,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void*)"Large Player Hit Box",   NULL,   NULL },
	{ jwin_text_proc,                       9,    38,     17,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void*)"Animation Style:",     NULL,   NULL },
	{ jwin_as_droplist_proc,               89,    34,     78,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,          0,          0, (void*)&animationstyle_list,   NULL,   NULL },
	{ jwin_check_proc,                      9,    90,      0,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void*)"Diagonal Movement",    NULL,   NULL },
	{ jwin_text_proc,                       9,    54,     17,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void*)"Swim Speed:",          NULL,   NULL },
	{ jwin_droplist_proc,                  89,    50,     78,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,          0,          0, (void*)&swimspeed_list,        NULL,   NULL },
	//195 Sideview water tab
	{  jwin_tab_proc,                        7,     33,    305,    183,    0,                      0,                       0,    0,          0,          0, (void *) herotile_sidewater_tabs,    NULL, (void *)herotile_dlg   },
	// 196 (sideswim sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 200 (sideswim sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_sideswim,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_sideswim,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_sideswim,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_sideswim,         NULL,                            NULL,   NULL                   },
	// 204 (sideswim slash sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 208 (sideswim slash sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_sideswimslash,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_sideswimslash,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_sideswimslash,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_sideswimslash,         NULL,                            NULL,   NULL                   },
	// 212 (sideswim stab sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 216 (sideswim stab sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_sideswimstab,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_sideswimstab,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_sideswimstab,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_sideswimstab,         NULL,                            NULL,   NULL                   },
	// 220 (sideswim pound sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    142,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    142,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 224 (sideswim pound sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_sideswimpound,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_sideswimpound,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    128,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_sideswimpound,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    128,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_sideswimpound,         NULL,                            NULL,   NULL                   },
	// 228 (sideswim charge sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 232 (sideswim charge sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_sideswimcharge,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_sideswimcharge,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_sideswimcharge,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_sideswimcharge,         NULL,                            NULL,   NULL                   },
	// 236 (swim hammer offset titles)
	{  jwin_rtext_proc,                     40,    116,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Hammer Ofs.",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    108,    116,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Hammer Ofs.",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     40,    170,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Hammer Ofs.",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    108,    170,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Hammer Ofs.",                NULL,   NULL                   },
	// 240 (swim hammer offset fields)
	{  jwin_edit_proc,                     43,     113,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	{  jwin_edit_proc,                     111,    113,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	{  jwin_edit_proc,                     43,     167,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	{  jwin_edit_proc,                     111,    167,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	// 244 (hold sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "1 Hand",             NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Casting",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "2 Hands",            NULL,   NULL                   },
	{  jwin_rtext_proc,                     101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Drown",            NULL,   NULL                   },
	// 248 (hold sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_sidewaterhold1,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_sideswimcast,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_sidewaterhold2,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_sidedrown,   NULL,                            NULL,   NULL                   },
	// 252 (revslash sprite titles)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 256 (revslash sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_revslash,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_revslash,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_revslash,         NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_revslash,         NULL,                            NULL,   NULL                   },
	// 260 (lifting)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 264 (lift sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,             up,      ls_liftwalk,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,           down,      ls_liftwalk,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,           left,      ls_liftwalk,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_liftwalk,   NULL,                            NULL,   NULL                   },
	// 268 (lifting)
	{  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     33,    142,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    101,    142,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
	// 272 (lift sprites)
	{  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,             up,      ls_lifting,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,           down,      ls_lifting,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                        36,    128,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,           left,      ls_lifting,   NULL,                            NULL,   NULL                   },
	{  d_ltile_proc,                       104,    128,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_lifting,   NULL,                            NULL,   NULL                   },
	// 276 (lift frame count labels)
	{  jwin_rtext_proc,                     40,    116,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Num Frames",                   NULL,   NULL                   },
	{  jwin_rtext_proc,                    108,    116,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Num Frames",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                     40,    170,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Num Frames",                 NULL,   NULL                   },
	{  jwin_rtext_proc,                    108,    170,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Num Frames",                NULL,   NULL                   },
	// 280 (lift frame count fields)
	{  jwin_edit_proc,                      43,    113,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	{  jwin_edit_proc,                     111,    113,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	{  jwin_edit_proc,                      43,    167,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	{  jwin_edit_proc,                     111,    167,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	// 284 (lift speed preview value)
	{  jwin_rtext_proc,                     60,     58,     64,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Preview Speed",                NULL,   NULL                   },
	{  jwin_edit_proc,                      63,     55,     20,     16,    vc(12),                 vc(1),                   0,       0,           4,    0,  NULL,                                           NULL,   NULL                  },
	
	{  NULL,                                 0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   }    

};

static char liftspeed[4] = "4";
int32_t d_ltile_proc(int32_t msg,DIALOG *d,int32_t)
{
	//d1=dir
	//d2=type (determines how to animate)
	//fg=cset (6)
	enum {lt_clock, lt_tile, lt_flip, lt_extend, lt_frames};
	static int32_t bg=makecol(0, 0, 0);
	int32_t *p=(int32_t*)d->dp3;
	int32_t oldtile=0;
	int32_t oldflip=0;
	
	switch(msg)
	{
		case MSG_START:
		{
			d->dp3=(int32_t*)malloc(sizeof(int32_t)*5);
			p=(int32_t*)d->dp3;
			p[lt_clock]=0;
			p[lt_tile]=0;
			p[lt_flip]=0;
			p[lt_extend]=0;
			p[lt_frames]=0;
			herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.heroAnimationStyle);
			if(d->d2 == ls_lifting)
				p[lt_frames] = vbound(atoi((char*)herotile_dlg[280+d->d1].dp),1,255);
			break;
		}
		
		case MSG_CLICK:
		{
			int32_t t;
			int32_t f;
			int32_t extend;
			int32_t cs = 6;
			herotile(&t, &f, &extend, d->d2, d->d1, zinit.heroAnimationStyle);
			
			switch(extend)
			{
				case 0:
					if(!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2+8, d->y+2+4, d->x+32+8+2, d->y+(16+32)+2))
					{
						return D_O_K;
					}
					
					break;
					
				case 1:
					if(!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2+8, d->y+2+4, d->x+(32)+8+2, d->y+(4+64)+2))
					{
						return D_O_K;
					}
					
					break;
					
				case 2:
					if(!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2+8, d->y+4, d->x+(64)+8+2, d->y+(4+64)+2))
					{
						return D_O_K;
					}
					
					break;
			}
			
			if((CHECK_CTRL_CMD)
				? select_tile_2(t,f,2,cs,false,extend,true)
				: select_tile(t,f,2,cs,false,extend,true))
			{
				extend=ex;
				setherotile(t,f,extend,d->d2,d->d1);
				return D_REDRAW;
			}
		}
		break;
		
		case MSG_VSYNC:
		{
			oldtile=p[lt_tile];
			oldflip=p[lt_flip];
			p[lt_clock]++;
			auto lspeed = vbound(atoi(liftspeed),1,255);
			if(d->d2 == ls_lifting)
				p[lt_frames] = vbound(atoi((char*)herotile_dlg[280+d->d1].dp),1,255);
			
			switch(zinit.heroAnimationStyle)
			{
				case las_original:                                             //2-frame
					switch(d->d2)
					{
						case ls_charge:
						case ls_walk:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=6)
							{
								if(d->d1==up&&d->d2==ls_walk)
								{
									p[lt_flip]=1;                //h flip
								}
								else
								{
									p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;               //tile++
								}
							};
							
							if(p[lt_clock]>=11)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
							
						case ls_sideswim:
						case ls_sideswimcharge:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=6)
							{
								if(d->d1==up&&d->d2==ls_sideswim)
								{
									p[lt_flip]=1;				//h flip
								}
								else
								{
									p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;			   //tile++
								}
							};
							
							if(p[lt_clock]>=11)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
					
						case ls_sideswimslash:
						{
							if(p[lt_clock]<6)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimslash, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswim, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswim, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;				  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
							
						case ls_sideswimstab:
						{
							if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswim, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswim, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;				  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
							
						case ls_sideswimpound:
						{
							if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimpound, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<30)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswim, d->d1, zinit.heroAnimationStyle);
								
								if(p[lt_clock]>=31)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
					
						case ls_jump:
						{
							if(p[lt_clock]>=24)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								
								if(p[lt_clock]>=36)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=p[lt_extend]==2?((int32_t)p[lt_clock]/8)*2:((int32_t)p[lt_clock]/8);
							}
							
							break;
						}
							
						case ls_slash:
						{
							if(p[lt_clock]<6)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
							
						case ls_revslash:
						{
							if(p[lt_clock]<6)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_revslash, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
							
						case ls_stab:
						{
							if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
							
						case ls_pound:
						{
							if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<30)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.heroAnimationStyle);
								
								if(p[lt_clock]>=31)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
							
						case ls_float:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=12)
							{
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
							};
							
							if(p[lt_clock]>=23)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
							
						case ls_swim:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=12)
							{
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
							};
							
							if(p[lt_clock]>=23)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
							
						case ls_dive:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=50)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							if((p[lt_clock]/12)&1)
							{
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
							};
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_drown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_drown, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							if((p[lt_clock]/12)&1)
							{
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
							};
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						case ls_sidedrown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sidedrown, d->d1, zinit.heroAnimationStyle);
							
							
							if((p[lt_clock]/12)&1)
							{
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
							};
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=p[lt_extend]==2?((int32_t)p[lt_clock]/8)*2:((int32_t)p[lt_clock]/8);
							};
							break;
						}
					
						case ls_lavadrown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_lavadrown, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							if((p[lt_clock]/12)&1)
							{
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
							};
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
							
						case ls_falling:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_falling, d->d1, zinit.heroAnimationStyle);
							p[lt_tile] += ((p[lt_clock]%70)/10)*(p[lt_extend]==2 ? 2 : 1);
							break;
						}
							
						case ls_landhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
							
						case ls_landhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
							
						case ls_waterhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
							
						case ls_waterhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_sidewaterhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
							
						case ls_sidewaterhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
							
						case ls_cast:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<96)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.heroAnimationStyle);
							};
							
							if(p[lt_clock]>=194)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_sideswimcast:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sideswimcast, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<96)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
							};
							
							if(p[lt_clock]>=194)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_lifting:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_lifting, d->d1, zinit.heroAnimationStyle);
							size_t fr = p[lt_clock] / lspeed;
							if(fr >= p[lt_frames])
							{
								fr = 0;
								p[lt_clock] = -1;
							}
							p[lt_tile] += fr*(p[lt_extend]==2?2:1);
							break;
						}
						case ls_liftwalk:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_liftwalk, d->d1, zinit.heroAnimationStyle);
							if(p[lt_clock]>=6)
							{
								if(d->d1==up)
								{
									p[lt_flip]=1;                //h flip
								}
								else
								{
									p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;               //tile++
								}
							};
							
							if(p[lt_clock]>=11)
							{
								p[lt_clock]=-1;
							}
							break;
						}
							
						default:
							break;
					}
					
					break;
				
				case las_bszelda:                                             //3-frame BS
					switch(d->d2)
					{
						case ls_charge:
						case ls_walk:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]>=27)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_sideswim:
						case ls_sideswimcharge:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]>=27)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_sideswimslash:
						{
							if(p[lt_clock]<6)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimslash, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswim, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswim, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;				  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
						
						case ls_sideswimstab:
						{
							if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswim, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswim, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;				  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
						
						case ls_sideswimpound:
						{
							if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimpound, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<30)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sideswim, d->d1, zinit.heroAnimationStyle);
								
								if(p[lt_clock]>=31)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
						
						case ls_jump:
						{
							if(p[lt_clock]>=24)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								
								if(p[lt_clock]>=36)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=p[lt_extend]==2?((int32_t)p[lt_clock]/8)*2:((int32_t)p[lt_clock]/8);
							}
							
							break;
						}
						
						case ls_slash:
						{
							if(p[lt_clock]<6)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
						
						case ls_revslash:
						{
							if(p[lt_clock]<6)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_revslash, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
						
						case ls_stab:
						{
							if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<13)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
								
								if(p[lt_clock]>=16)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
						
						case ls_pound:
						{
							if(p[lt_clock]<12)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.heroAnimationStyle);
							}
							else if(p[lt_clock]<30)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.heroAnimationStyle);
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.heroAnimationStyle);
								
								if(p[lt_clock]>=31)
								{
									p[lt_clock]=-1;
								}
							};
							
							break;
						}
						
						case ls_float:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]>=55)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_swim:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]>=55)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_dive:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=50)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_drown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_drown, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_sidedrown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sidedrown, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=p[lt_extend]==2?((int32_t)p[lt_clock]/8)*2:((int32_t)p[lt_clock]/8);
							};
							
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_lavadrown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_lavadrown, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_falling:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_falling, d->d1, zinit.heroAnimationStyle);
							p[lt_tile] += ((p[lt_clock]%70)/10)*(p[lt_extend]==2 ? 2 : 1);
							break;
						}
						
						case ls_landhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_landhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_waterhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_waterhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_sidewaterhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_sidewaterhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_cast:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<96)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.heroAnimationStyle);
							}
							
							if(p[lt_clock]>=194)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_sideswimcast:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sideswimcast, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<96)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
							};
							
							if(p[lt_clock]>=194)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_lifting:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_lifting, d->d1, zinit.heroAnimationStyle);
							size_t fr = p[lt_clock] / lspeed;
							if(fr >= p[lt_frames])
							{
								fr = 0;
								p[lt_clock] = -1;
							}
							p[lt_tile] += fr*(p[lt_extend]==2?2:1);
							break;
						}
						case ls_liftwalk:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_liftwalk, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
							
							if(p[lt_clock]>=27)
							{
								p[lt_clock]=-1;
							}
							break;
						}
							
						default:
							break;
					}
					break;
				
				case las_zelda3slow:                                         //multi-frame Zelda 3 (slow)
				case las_zelda3:                                             //multi-frame Zelda 3
					switch(d->d2)
					{
						case ls_charge:
						case ls_walk:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=(64*(hero_animation_speed)))
							{
								p[lt_tile]+=(p[lt_extend]==2?2:1);
								
								int32_t l=((p[lt_clock]/hero_animation_speed)&15);
								l-=((l>3)?1:0)+((l>12)?1:0);
								p[lt_tile]+=(l/2)*(p[lt_extend]==2?2:1);
								
								//p[lt_tile]+=(((p[lt_clock]>>2)%8)*(p[lt_extend]==2?2:1));
								if(p[lt_clock]>=255)
								{
									p[lt_clock]=-1;
								}
							}
							
							break;
						}
						case ls_sideswim:
						case ls_sideswimcharge:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=(64*(hero_animation_speed)))
							{
								p[lt_tile]+=(p[lt_extend]==2?2:1);
								
								int32_t l=((p[lt_clock]/hero_animation_speed)&15);
								l-=((l>3)?1:0)+((l>12)?1:0);
								p[lt_tile]+=(l/2)*(p[lt_extend]==2?2:1);
								
								//p[lt_tile]+=(((p[lt_clock]>>2)%8)*(p[lt_extend]==2?2:1));
								if(p[lt_clock]>=255)
								{
									p[lt_clock]=-1;
								}
							}
							
							break;
						 
						case ls_sideswimstab:
						{
							if(p[lt_clock]>35)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
								
								if(p[lt_clock]>=47)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswim, d->d1, zinit.heroAnimationStyle);
							}
							
							break;
						}
						
						case ls_sideswimslash:
						{
							if(p[lt_clock]>23)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswimslash, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=(((p[lt_clock]>>2)%6)*(p[lt_extend]==2?2:1));
								
								if(p[lt_clock]>=47)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswim, d->d1, zinit.heroAnimationStyle);
							}
							
							break;
						}
						
						case ls_sideswimpound:
						{
							if(p[lt_clock]>35)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswimpound, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
								
								if(p[lt_clock]>=47)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sideswim, d->d1, zinit.heroAnimationStyle);
							}
							
							break;
						}
						
						case ls_jump:
						{
							if(p[lt_clock]>=24)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
								
								if(p[lt_clock]>=36)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=p[lt_extend]==2?((int32_t)p[lt_clock]/8)*2:((int32_t)p[lt_clock]/8);
							}
							
							break;
						}
						
						case ls_slash:
						{
							if(p[lt_clock]>23) //24 frames, advances by one every 4 frames, 6 tiles total
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_slash, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=(((p[lt_clock]>>2)%6)*(p[lt_extend]==2?2:1));
								
								if(p[lt_clock]>=47)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							
							break;
						}
						
						case ls_revslash:
						{
							if(p[lt_clock]>23) //24 frames, advances by one every 4 frames, 6 tiles total
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_revslash, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=(((p[lt_clock]>>2)%6)*(p[lt_extend]==2?2:1));
								
								if(p[lt_clock]>=47)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							
							break;
						}
						
						case ls_stab: //12 frames, advances by one every 4 frames, 3 tiles total.
							if(p[lt_clock]>35)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_stab, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
								
								if(p[lt_clock]>=47)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							
							break;
						}
						
						case ls_pound:
						{
							if(p[lt_clock]>35)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_pound, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
								
								if(p[lt_clock]>=47)
								{
									p[lt_clock]=-1;
								}
							}
							else
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.heroAnimationStyle);
							}
							
							break;
						}
						
						case ls_float:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
							
							if(p[lt_clock]>=23)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_swim:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=((p[lt_clock]/12)%4)<<(p[lt_extend]==2?1:0);
							
							if(p[lt_clock]>=47)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_dive:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=50)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_drown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_drown, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_sidedrown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_sidedrown, d->d1, zinit.heroAnimationStyle);
							p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.heroAnimationStyle);
								p[lt_tile]+=p[lt_extend]==2?((int32_t)p[lt_clock]/8)*2:((int32_t)p[lt_clock]/8);
							};
							
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_lavadrown:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_lavadrown, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<=4)
							{
								herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.heroAnimationStyle);
							};
							
							p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
							
							if(p[lt_clock]>=81)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_falling:
						{
							herotile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_falling, d->d1, zinit.heroAnimationStyle);
							p[lt_tile] += ((p[lt_clock]%70)/10)*(p[lt_extend]==2 ? 2 : 1);
							break;
						}
						
						case ls_landhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_landhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_waterhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_waterhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_sidewaterhold1:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold1, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_sidewaterhold2:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
							break;
						}
						
						case ls_cast:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<96)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.heroAnimationStyle);
							};
							
							if(p[lt_clock]>=194)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_sideswimcast:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_sideswimcast, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]<96)
							{
								herotile(&p[lt_tile], &p[lt_flip], ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
							};
							
							if(p[lt_clock]>=194)
							{
								p[lt_clock]=-1;
							}
							
							break;
						}
						
						case ls_lifting:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_lifting, d->d1, zinit.heroAnimationStyle);
							size_t fr = p[lt_clock] / lspeed;
							if(fr >= p[lt_frames])
							{
								fr = 0;
								p[lt_clock] = -1;
							}
							p[lt_tile] += fr*(p[lt_extend]==2?2:1);
							break;
						}
						case ls_liftwalk:
						{
							herotile(&p[lt_tile], &p[lt_flip], ls_liftwalk, d->d1, zinit.heroAnimationStyle);
							
							if(p[lt_clock]>=(64*(hero_animation_speed)))
							{
								p[lt_tile]+=(p[lt_extend]==2?2:1);
								
								int32_t l=((p[lt_clock]/hero_animation_speed)&15);
								l-=((l>3)?1:0)+((l>12)?1:0);
								p[lt_tile]+=(l/2)*(p[lt_extend]==2?2:1);
								
								if(p[lt_clock]>=255)
								{
									p[lt_clock]=-1;
								}
							}
							break;
						}
							
						default:
							break;
					}
					break;
				
				default:
					break;
			}
			
			if((p[lt_tile]!=oldtile)||(p[lt_flip]!=oldflip))
			{
				d->flags|=D_DIRTY;
			}
			
			break;
		}
			
		case MSG_DRAW:
		{
			BITMAP *buf=create_bitmap_ex(8,1,1);
			BITMAP *buf2=buf;
			int32_t dummy1, dummy2;
			int32_t extend;
			herotile(&dummy1, &dummy2, &extend, d->d2, d->d1, zinit.heroAnimationStyle);
			int32_t w = 16;
			int32_t h = 16;
			
			switch(extend)
			{
				case 0: //16x16
					break;
					
				case 1: //16x32
					w = 16;
					h = 32;
					break;
					
				case 2: //32x32
					w = 32;
					h = 32;
					break;
					
				default:
					break;
			}
			
			buf = create_bitmap_ex(8,w,h);
			
			w *= 2;
			h *= 2;
			
			BITMAP *bigbmp = create_bitmap_ex(8,w+4,h+4);
			
			if(buf && bigbmp)
			{
				clear_to_color(buf, bg);
				
				switch(extend)
				{
					case 0:
						overtile16(buf,p[lt_tile],0,0,6,p[lt_flip]);
						stretch_blit(buf, bigbmp, 0,0, 16, 16, 2, 2, w, h);
						jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
						blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
						break;
						
					case 1:
						overtile16(buf,p[lt_tile]-TILES_PER_ROW,0,0,6,p[lt_flip]);
						overtile16(buf,p[lt_tile],0,16,6,p[lt_flip]);
						stretch_blit(buf, bigbmp, 0,0, 16, 32, 2, 2, w, h);
						jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
						blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
						break;
						
					case 2:
						overtile16(buf,p[lt_tile]-TILES_PER_ROW,8,0,6,p[lt_flip]); //top middle
						overtile16(buf,p[lt_tile]-TILES_PER_ROW-(p[lt_flip]?-1:1),-8,0,6,p[lt_flip]); //top left
						overtile16(buf,p[lt_tile]-TILES_PER_ROW+(p[lt_flip]?-1:1),8+16,0,6,p[lt_flip]); //top right
						overtile16(buf,p[lt_tile]-(p[lt_flip]?-1:1),-8,16,6,p[lt_flip]); //bottom left
						overtile16(buf,p[lt_tile]+(p[lt_flip]?-1:1),8+16,16,6,p[lt_flip]); //bottom right
						overtile16(buf,p[lt_tile],8,16,6,p[lt_flip]);              //bottom middle
						stretch_blit(buf, bigbmp, 0,0, 32, 32, 2, 2, w, h);
						jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
						blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
						break;
						
					default:
						break;
				}
				
				destroy_bitmap(bigbmp);
				destroy_bitmap(buf);
				destroy_bitmap(buf2);
			}
		}
		break;
		
		case MSG_END:
		{
			free(d->dp3);
			break;
		}
	}
	
	return D_O_K;
}

int32_t onCustomHero()
{
	//setupherotiles(zinit.heroAnimationStyle);
	if(zinit.heroAnimationStyle==las_zelda3slow)
	{
		hero_animation_speed=2;
	}
	else
	{
		hero_animation_speed=1;
	}
	
	herotile_dlg[0].dp2=get_zc_font(font_lfont);
	herotile_dlg[189].flags = get_qr(qr_LTTPCOLLISION)? D_SELECTED : 0;
	herotile_dlg[192].flags = get_qr(qr_LTTPWALK)? D_SELECTED : 0;
	herotile_dlg[194].d1=(zinit.hero_swim_speed<60)?0:1;
	herotile_dlg[191].d1=zinit.heroAnimationStyle;
	
	large_dialog(herotile_dlg, 2.0);
		
	int32_t oldWalkSpr[4][3];
	int32_t oldStabSpr[4][3];
	int32_t oldSlashSpr[4][3];
	int32_t oldRevSlashSpr[4][3];
	int32_t oldFloatSpr[4][3];
	int32_t oldSwimSpr[4][3];
	int32_t oldDiveSpr[4][3];
	int32_t oldPoundSpr[4][3];
	int32_t oldJumpSpr[4][3];
	int32_t oldChargeSpr[4][3];
	int32_t oldCastSpr[3];
	int32_t oldsideswimCastSpr[3];
	int32_t oldHoldSpr[2][3][3];
	int32_t oldDrownSpr[4][3];
	int32_t oldSideDrownSpr[4][3];
	int32_t oldFallSpr[4][3];
	int32_t oldLavaDrownSpr[4][3];
	int32_t oldSideSwimSpr[4][3];
	int32_t oldSideSwimSlashSpr[4][3];
	int32_t oldSideSwimStabSpr[4][3];
	int32_t oldSideSwimPoundSpr[4][3];
	int32_t oldSideSwimChargeSpr[4][3];
	int32_t oldSideSwimHoldSpr[3][3];
	int32_t oldLiftingSpr[4][4];
	int32_t oldLiftingWalkSpr[4][3];
	char hmr1[8];
	char hmr2[8];
	char hmr3[8];
	char hmr4[8];
	sprintf(hmr1,"%d",hammeroffsets[0]);
	sprintf(hmr2,"%d",hammeroffsets[1]);
	sprintf(hmr3,"%d",hammeroffsets[2]);
	sprintf(hmr4,"%d",hammeroffsets[3]);
	herotile_dlg[240].dp = hmr1;
	herotile_dlg[241].dp = hmr2;
	herotile_dlg[242].dp = hmr3;
	herotile_dlg[243].dp = hmr4;
	char liftframes1[4];
	char liftframes2[4];
	char liftframes3[4];
	char liftframes4[4];
	sprintf(liftframes1,"%d",vbound(liftingspr[0][spr_frames],1,255));
	sprintf(liftframes2,"%d",vbound(liftingspr[1][spr_frames],1,255));
	sprintf(liftframes3,"%d",vbound(liftingspr[2][spr_frames],1,255));
	sprintf(liftframes4,"%d",vbound(liftingspr[3][spr_frames],1,255));
	herotile_dlg[280].dp = liftframes1;
	herotile_dlg[281].dp = liftframes2;
	herotile_dlg[282].dp = liftframes3;
	herotile_dlg[283].dp = liftframes4;
	herotile_dlg[285].dp = liftspeed;
	memcpy(oldWalkSpr, walkspr, 4*3*sizeof(int32_t));
	memcpy(oldStabSpr, stabspr, 4*3*sizeof(int32_t));
	memcpy(oldSlashSpr, slashspr, 4*3*sizeof(int32_t));
	memcpy(oldRevSlashSpr, revslashspr, 4*3*sizeof(int32_t));
	memcpy(oldFloatSpr, floatspr, 4*3*sizeof(int32_t));
	memcpy(oldSwimSpr, swimspr, 4*3*sizeof(int32_t));
	memcpy(oldDiveSpr, divespr, 4*3*sizeof(int32_t));
	memcpy(oldPoundSpr, poundspr, 4*3*sizeof(int32_t));
	memcpy(oldJumpSpr, jumpspr, 4*3*sizeof(int32_t));
	memcpy(oldChargeSpr, chargespr, 4*3*sizeof(int32_t));
	memcpy(oldCastSpr, castingspr, 3*sizeof(int32_t));
	memcpy(oldsideswimCastSpr, sideswimcastingspr, 3*sizeof(int32_t));
	memcpy(oldHoldSpr, holdspr, 2*3*3*sizeof(int32_t));
	memcpy(oldDrownSpr, drowningspr, 4*3*sizeof(int32_t));
	memcpy(oldSideDrownSpr, sidedrowningspr, 4*3*sizeof(int32_t));
	memcpy(oldFallSpr, fallingspr, 4*3*sizeof(int32_t));
	memcpy(oldLavaDrownSpr, drowning_lavaspr, 4*3*sizeof(int32_t));
	memcpy(oldSideSwimSpr, sideswimspr, 4*3*sizeof(int32_t));
	memcpy(oldSideSwimSlashSpr, sideswimslashspr, 4*3*sizeof(int32_t));
	memcpy(oldSideSwimStabSpr, sideswimstabspr, 4*3*sizeof(int32_t));
	memcpy(oldSideSwimPoundSpr, sideswimpoundspr, 4*3*sizeof(int32_t));
	memcpy(oldSideSwimChargeSpr, sideswimchargespr, 4*3*sizeof(int32_t));
	memcpy(oldSideSwimHoldSpr, sideswimholdspr, 3*3*sizeof(int32_t));
	memcpy(oldLiftingSpr, liftingspr, 4*4*sizeof(int32_t));
	memcpy(oldLiftingWalkSpr, liftingwalkspr, 4*3*sizeof(int32_t));
	
	//Populate Player defenses
	for (int32_t i = 0; i < wMax - wEnemyWeapons - 1; i++)
	{
		herotile_dlg[144 + i].d1 = hero_defence[wEnemyWeapons+i];
	}
	herotile_dlg[154+8].d1 = hero_defence[wFire];
	herotile_dlg[155+8].d1 = hero_defence[wBomb];
	herotile_dlg[156+8].d1 = hero_defence[wRefMagic];
	herotile_dlg[157+8].d1 = hero_defence[wRefFireball];
	herotile_dlg[158+8].d1 = hero_defence[wRefRock];
	herotile_dlg[159+8].d1 = hero_defence[wRefBeam];

	herotile_dlg[171+8].d1 = hero_defence[wScript1];
	herotile_dlg[172+8].d1 = hero_defence[wScript2];
	herotile_dlg[173+8].d1 = hero_defence[wScript3];
	herotile_dlg[174+8].d1 = hero_defence[wScript4];
	herotile_dlg[175+8].d1 = hero_defence[wScript5];
	herotile_dlg[176+8].d1 = hero_defence[wScript6];
	herotile_dlg[177+8].d1 = hero_defence[wScript7];
	herotile_dlg[178+8].d1 = hero_defence[wScript8];
	herotile_dlg[179+8].d1 = hero_defence[wScript9];
	herotile_dlg[180+8].d1 = hero_defence[wScript10];

	int32_t ret = 0;
	do
	{
		ret = do_zqdialog(herotile_dlg, 3);

		if (ret == 3)
		{
			saved = false;
			set_qr(qr_LTTPCOLLISION, (herotile_dlg[181+8].flags & D_SELECTED) ? 1 : 0);
			set_qr(qr_LTTPWALK, (herotile_dlg[184+8].flags & D_SELECTED) ? 1 : 0);
			zinit.hero_swim_speed = (herotile_dlg[186+8].d1 == 0) ? 50 : 67;

			//Save Player defenses
			for (int32_t i = 0; i < wMax - wEnemyWeapons - 1; i++)
			{
				hero_defence[wEnemyWeapons + i] = herotile_dlg[137 + 7 + i].d1;
			}
			hero_defence[wFire] = herotile_dlg[154+8].d1;
			hero_defence[wBomb] = herotile_dlg[155+8].d1;
			hero_defence[wRefMagic] = herotile_dlg[156+8].d1;
			hero_defence[wRefFireball] = herotile_dlg[157+8].d1;
			hero_defence[wRefRock] = herotile_dlg[158+8].d1;
			hero_defence[wRefBeam] = herotile_dlg[159+8].d1;

			hero_defence[wScript1] = herotile_dlg[171+8].d1;
			hero_defence[wScript2] = herotile_dlg[172+8].d1;
			hero_defence[wScript3] = herotile_dlg[173+8].d1;
			hero_defence[wScript4] = herotile_dlg[174+8].d1;
			hero_defence[wScript5] = herotile_dlg[175+8].d1;
			hero_defence[wScript6] = herotile_dlg[176+8].d1;
			hero_defence[wScript7] = herotile_dlg[177+8].d1;
			hero_defence[wScript8] = herotile_dlg[178+8].d1;
			hero_defence[wScript9] = herotile_dlg[179+8].d1;
			hero_defence[wScript10] = herotile_dlg[180+8].d1;
			hammeroffsets[0] = atoi(hmr1);
			hammeroffsets[1] = atoi(hmr2);
			hammeroffsets[2] = atoi(hmr3);
			hammeroffsets[3] = atoi(hmr4);
			liftingspr[0][spr_frames] = vbound(atoi(liftframes1),1,255);
			liftingspr[1][spr_frames] = vbound(atoi(liftframes2),1,255);
			liftingspr[2][spr_frames] = vbound(atoi(liftframes3),1,255);
			liftingspr[3][spr_frames] = vbound(atoi(liftframes4),1,255);
		}
		else if (ret == 168)
		{
			for (int32_t i = 146; i < 168; i++)
			{
				herotile_dlg[i].d1 = herotile_dlg[145].d1;
			}
			for (int32_t i = 179; i < 189; i++)
			{
				herotile_dlg[i].d1 = herotile_dlg[145].d1;
			}
		}
		else
		{
			memcpy(walkspr, oldWalkSpr, 4 * 3 * sizeof(int32_t));
			memcpy(stabspr, oldStabSpr, 4 * 3 * sizeof(int32_t));
			memcpy(slashspr, oldSlashSpr, 4 * 3 * sizeof(int32_t));
			memcpy(revslashspr, oldRevSlashSpr, 4 * 3 * sizeof(int32_t));
			memcpy(floatspr, oldFloatSpr, 4 * 3 * sizeof(int32_t));
			memcpy(swimspr, oldSwimSpr, 4 * 3 * sizeof(int32_t));
			memcpy(divespr, oldDiveSpr, 4 * 3 * sizeof(int32_t));
			memcpy(poundspr, oldPoundSpr, 4 * 3 * sizeof(int32_t));
			memcpy(jumpspr, oldJumpSpr, 4 * 3 * sizeof(int32_t));
			memcpy(chargespr, oldChargeSpr, 4 * 3 * sizeof(int32_t));
			memcpy(castingspr, oldCastSpr, 3 * sizeof(int32_t));
			memcpy(sideswimcastingspr, oldsideswimCastSpr, 3 * sizeof(int32_t));
			memcpy(holdspr, oldHoldSpr, 2*3*3*sizeof(int32_t));
			memcpy(drowningspr, oldDrownSpr, 4 * 3 * sizeof(int32_t));
			memcpy(sidedrowningspr, oldSideDrownSpr, 4 * 3 * sizeof(int32_t));
			memcpy(fallingspr, oldFallSpr, 4 * 3 * sizeof(int32_t));
			memcpy(drowning_lavaspr, oldLavaDrownSpr, 4 * 3 * sizeof(int32_t));
			memcpy(sideswimspr, oldSideSwimSpr, 4 * 3 * sizeof(int32_t));
			memcpy(sideswimslashspr, oldSideSwimSlashSpr, 4 * 3 * sizeof(int32_t));
			memcpy(sideswimstabspr, oldSideSwimStabSpr, 4 * 3 * sizeof(int32_t));
			memcpy(sideswimpoundspr, oldSideSwimPoundSpr, 4 * 3 * sizeof(int32_t));
			memcpy(sideswimchargespr, oldSideSwimChargeSpr, 4 * 3 * sizeof(int32_t));
			memcpy(sideswimholdspr, oldSideSwimHoldSpr, 3* 3 * sizeof(int32_t));
			memcpy(liftingspr, oldLiftingSpr, 4 * 4 * sizeof(int32_t));
			memcpy(liftingwalkspr, oldLiftingWalkSpr, 4 * 3 * sizeof(int32_t));
		}
	} while (ret == 168);
	
	return D_O_K;
}

void center_zq_custom_dialogs()
{
	jwin_center_dialog(herotile_dlg);
	jwin_center_dialog(wpndata_dlg);
	jwin_center_dialog(enedata_dlg);
}
