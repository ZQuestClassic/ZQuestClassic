#include <map>
#include <cstring>
#include <stdio.h>
#include <cstring>

#include "base/files.h"
#include "base/qrs.h"
#include "base/packfile.h"
#include "base/gui.h"
#include "base/qst.h"
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
#include "dialog/heroeditor.h"
#include "zinfo.h"

extern int32_t ex;
extern void reset_itembuf(itemdata *item, int32_t id);
extern const char *sfxlist(int32_t index, int32_t *list_size);
// zq_subscr.cpp
extern int32_t sstype_drop_proc(int32_t msg,DIALOG *d,int32_t c);

extern int32_t biw_cnt;


#ifdef _MSC_VER
#define stricmp _stricmp
#endif

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
					|| d[i].proc == d_ffcombolist_proc || d[i].proc == sstype_drop_proc
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
		bool bigfontproc = (d[i].proc != jwin_droplist_proc && d[i].proc != jwin_abclist_proc && d[i].proc != jwin_list_proc && d[i].proc != d_dmaplist_proc
							&& d[i].proc != d_dropdmaplist_proc && d[i].proc != d_warplist_proc && d[i].proc != d_warplist_proc && d[i].proc != d_wclist_proc && d[i].proc != d_ndroplist_proc
							&& d[i].proc != d_idroplist_proc && d[i].proc != d_nidroplist_proc && d[i].proc && d[i].proc != d_ffcombolist_proc && d[i].proc != sstype_drop_proc
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
				return "Switch w/ Hero";
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
	int32_t zversion = 0;
	int32_t zbuild = 0;
	itemdata tempitem;
   
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
	
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	al_trace("readoneitem section_version: %d\n", section_version);
	
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zitem packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	else if ( ( section_version > V_ITEMS ) )
	{
		al_trace("Cannot read .zitem packfile made using V_ITEMS (%d)\n", section_version);
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
		byte temp;
		if(!p_getc(&temp,f))
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
			
		if(!p_getc(&tempitem.weap_data.imitate_weapon,f))
			{
			return 0;
			}
			if(!p_getc(&tempitem.weap_data.default_defense,f))
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
				if(!p_igetl(&tempitem.weap_data.initd[q],f))
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
			if(!p_igetl(&tempitem.weap_data.hxofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_data.hyofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_data.hxsz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_data.hysz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_data.hzsz,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_data.xofs,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_data.yofs,f))
			{
				return 0;
			}
			if(!p_igetw(&tempitem.weap_data.script,f))
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
			if(!p_igetl(&tempitem.weap_data.override_flags,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_data.tilew,f))
			{
				return 0;
			}
			if(!p_igetl(&tempitem.weap_data.tileh,f))
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
				byte dummy;
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&dummy,f))
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
				byte temp;
				if(!p_getc(&temp,f))
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
	
	if(!write_deprecated_section_cversion(section_version,f))
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
				if(!p_putc(0,f))
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
		
		if(!p_putc(itemsbuf[i].weap_data.imitate_weapon,f))
			{
				new_return(49);
			}
		if(!p_putc(itemsbuf[i].weap_data.default_defense,f))
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
			if(!p_iputl(itemsbuf[i].weap_data.initd[q],f))
			{
				new_return(55);
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
		if(!p_iputl(itemsbuf[i].weap_data.hxofs,f))
		{
			new_return(65);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hyofs,f))
		{
			new_return(66);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hxsz,f))
		{
			new_return(67);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hysz,f))
		{
			new_return(68);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hzsz,f))
		{
			new_return(69);
		}
		if(!p_iputl(itemsbuf[i].weap_data.xofs,f))
		{
			new_return(70);
		}
		if(!p_iputl(itemsbuf[i].weap_data.yofs,f))
		{
			new_return(71);
		}
		if(!p_iputw(itemsbuf[i].weap_data.script,f))
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
		if(!p_iputl(itemsbuf[i].weap_data.override_flags,f))
		{
			new_return(78);
		}
		if(!p_iputl(itemsbuf[i].weap_data.tilew,f))
		{
			new_return(79);
		}
		if(!p_iputl(itemsbuf[i].weap_data.tileh,f))
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
				if(!p_putc(0,f))
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
			if(!p_putc(0,f))
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

int32_t onCustomWpns()
{
	SpriteListerDialog().show();
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

//do we still use this?
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

static ListData sfx__list(sfxlist, &font);

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
		if (test.weap_data.script)
		{
			zasm_meta const& meta = ewpnscripts[test.weap_data.script]->meta;
			if (!meta.initd_help[i].empty())
				InfoDialog("Info", meta.initd_help[i].c_str()).show();
		}
	}
}

static int32_t copiedGuy;

int32_t readonenpc(PACKFILE *f, int32_t index)
{
	dword section_version = 0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	guydata tempguy;
   
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
	
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	al_trace("readonenpc section_version: %d\n", section_version);
	
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .znpc packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	else if ( ( section_version > V_GUYS ) )
	{
		al_trace("Cannot read .znpc packfile made using V_GUYS (%d)\n", section_version);
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
				int32_t temp;
			if(!p_igetl(&temp,f))
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
				byte dummy;
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&dummy,f))
					{
						return 0;
					}
				}
			}
			if(!p_igetw(&tempguy.weap_data.script,f))
			{
			return 0;
			}
			//eweapon initD
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&tempguy.weap_data.initd[q],f))
			{
				return 0;
			}
			}
			//enemy editor misc merge
			for (int32_t q = 0; q < 32; q++)
			{
				if (!p_igetl(&tempguy.attributes[q], f)) return 0;
			}

			//was missing for some reason has been added
			if (!p_getc(&tempguy.spr_shadow, f))
				return 0;
			if (!p_getc(&tempguy.spr_death, f))
				return 0;
			if (!p_getc(&tempguy.spr_spawn, f))
				return 0;
			if (!p_igetl(&tempguy.moveflags, f))
				return 0;
			
			//enemy editor attacktab
			if (!p_igetl(&tempguy.weap_data.moveflags, f))
				return 0;
			if (!p_getc(&tempguy.weap_data.unblockable, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.override_flags, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.hxofs, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.hyofs, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.hxsz, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.hysz, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.hzsz, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.xofs, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.yofs, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.tilew, f))
				return 0;
			if (!p_igetl(&tempguy.weap_data.tileh, f))
				return 0;
			int32_t temp_step;
			if (!p_igetl(&temp_step, f))
				return 0;
			tempguy.weap_data.step = zslongToFix(temp_step*100);
			for (int q=0; q < WPNSPR_MAX; ++q)
			{
				if (!p_getc(&tempguy.weap_data.burnsprs[q], f))
					return 0;
				if (!p_getc(&tempguy.weap_data.light_rads[q], f))
					return 0;
			}
			if (!p_getc(&tempguy.specialsfx, f))
				return 0;

		}
	}
	guysbuf[index] = tempguy;
	//strcpy(item_string[index], istring);
	guysbuf[bie[index].i] = tempguy;
	strcpy(guy_string[bie[index].i], npcstring);
	   
	return 1;
}

int32_t writeonenpc(PACKFILE *f, int32_t i)
{
	
	dword section_version=V_GUYS;
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
	
	if(!write_deprecated_section_cversion(section_version,f))
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
		if(!p_iputl(0,f))
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
				if(!p_putc(0,f))
				{
					return 0;
				}
			}
		}
		if(!p_iputw(guysbuf[i].weap_data.script,f))
		{
		return 0;
		}
		//eweapon initD
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(guysbuf[i].weap_data.initd[q],f))
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
		if (!p_putc(guysbuf[i].spr_shadow, f))
			return 0;
		if (!p_putc(guysbuf[i].spr_death, f))
			return 0;
		if (!p_putc(guysbuf[i].spr_spawn, f))
			return 0;
		if (!p_iputl(guysbuf[i].moveflags, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.moveflags, f))
			return 0;
		if (!p_putc(guysbuf[i].weap_data.unblockable, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.override_flags, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.hxofs, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.hyofs, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.hxsz, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.hysz, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.hzsz, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.xofs, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.yofs, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.tilew, f))
			return 0;
		if (!p_iputl(guysbuf[i].weap_data.tileh, f))
			return 0;
		int32_t temp_step = guysbuf[i].weap_data.step.getZLong() / 100;
		if (!p_iputl(temp_step, f))
			return 0;
		for (int q=0; q < WPNSPR_MAX; ++q)
		{
			if (!p_putc(guysbuf[i].weap_data.burnsprs[q], f))
				return 0;
			if (!p_putc(guysbuf[i].weap_data.light_rads[q], f))
				return 0;
		}
		if (!p_putc(guysbuf[i].specialsfx, f))
			return 0;

	return 1;
}

int32_t onCustomEnemies()
{
	EnemyListerDialog().show();
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

int32_t onCustomHero() {
	call_hero_editor();
	return D_O_K;
}