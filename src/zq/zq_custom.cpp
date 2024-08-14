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
#include "dialog/enemyeditor.h"
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

void edit_enemydata(int32_t index)
{
	call_enemy_editor(index);
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
}
