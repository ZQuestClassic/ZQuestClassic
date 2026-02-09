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

extern int32_t biw_cnt;


#ifdef _MSC_VER
#define stricmp _stricmp
#endif

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
			if(d[i].proc == d_comboframe_proc)
			{
				d[i].w *= 2;
				d[i].w -= 4;
			}
			else if(d[i].proc == d_wflag_proc || d[i].proc==d_bitmap_proc)
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
			if(d[i].proc == jwin_edit_proc || d[i].proc == jwin_check_proc || d[i].proc == jwin_checkfont_proc)
			{
				d[i].h = int32_t((double)d[i].h*1.5);
			}
			else if(d[i].proc == jwin_droplist_proc || d[i].proc == d_ndroplist_proc || d[i].proc == d_idroplist_proc || d[i].proc == d_nidroplist_proc || d[i].proc == d_dropdmaplist_proc)
			{
				d[i].y += int32_t((double)d[i].h*0.25);
				d[i].h = int32_t((double)d[i].h*1.25);
			}
			else if(d[i].proc == d_comboframe_proc)
			{
				d[i].h *= 2;
				d[i].h -= 4;
			}
			else if(d[i].proc == d_wflag_proc || d[i].proc==d_bitmap_proc)
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
		bool bigfontproc = (d[i].proc != jwin_droplist_proc && d[i].proc != jwin_abclist_proc && d[i].proc != jwin_list_proc
							&& d[i].proc != d_dropdmaplist_proc && d[i].proc != d_warplist_proc && d[i].proc != d_wclist_proc && d[i].proc != d_ndroplist_proc
							&& d[i].proc != d_idroplist_proc && d[i].proc != d_nidroplist_proc && d[i].proc);
							
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

int32_t readoneitem(PACKFILE *f, word index)
{
	dword section_version = 0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	byte tempbyte;
	
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
	
	if (section_version < 67)
	{
		char buf[64] = {0};
		if (!pfread(buf, 64, f))
			return 0;
		itemsbuf[index].name = buf;
	}
	
	if (read_single_item(f, section_version, index, zversion, zbuild))
		return 0;
	
	update_old_item(section_version, index, zversion);
	
	return 1;
}

int32_t writeoneitem(PACKFILE *f, word i)
{
	dword section_version = V_ITEMS;
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
	
	if (write_single_item(f, i))
		return 0;
	
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
		mark_save_dirty();
		for(auto q = 0; q < sprMAX; ++q)
			QMisc.sprites[q] = byte(newsprs[q]);
	}).show();
	return D_O_K;
}

int32_t onMiscSFX()
{
	MiscSFXDialog(QMisc.miscsfx, 20, [](int32_t* newsfx)
	{
		mark_save_dirty();
		for(auto q = 0; q < sfxMAX; ++q)
			QMisc.miscsfx[q] = byte(newsfx[q]);
	}).show();
	return D_O_K;
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
	byte tempbyte;
   
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

	if(!p_igetw(&tempguy.type,f))
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
	
	if (section_version < 55)
	{
		if(!p_getc(&tempbyte, f))
			return qe_invalid;
		tempguy.hitsfx = tempbyte;
		if(!p_getc(&tempbyte, f))
			return qe_invalid;
		tempguy.deadsfx = tempbyte;
	}
	else
	{
		if(!p_igetw(&(tempguy.hitsfx),f))
			return qe_invalid;
		if(!p_igetw(&(tempguy.deadsfx),f))
			return qe_invalid;
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
			if (section_version < 55)
			{
				if (!p_getc(&tempbyte, f))
					return 0;
				tempguy.specialsfx = tempbyte;
			}
			else
			{
				if (!p_igetw(&tempguy.specialsfx, f))
					return 0;
			}

		}
	}
	guysbuf[index] = tempguy;
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
		
		if(!p_iputw(guysbuf[i].type,f))
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
		
		if(!p_iputw(guysbuf[i].hitsfx,f))
			return 0;
		
		if(!p_iputw(guysbuf[i].deadsfx,f))
			return 0;
		
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
		if (!p_iputw(guysbuf[i].specialsfx, f))
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