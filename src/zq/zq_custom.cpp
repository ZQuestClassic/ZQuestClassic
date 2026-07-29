#include <map>
#include <cstring>
#include <stdio.h>
#include <cstring>

#include "zalleg/files.h"
#include "core/qrs.h"
#include "zalleg/packfile.h"
#include "zalleg/gui.h"
#include "core/qst.h"
#include "zq/zq_custom.h"
#include "tiles.h"
#include "zq/zq_tiles.h"
#include "zq/zq_misc.h"
#include "zq/zquest.h"
#include "items.h"
#include "zalleg/zsys.h"
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
			else if(d[i].proc == jwin_droplist_proc || d[i].proc == d_ndroplist_proc || d[i].proc == d_idroplist_proc || d[i].proc == d_dropdmaplist_proc)
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
							&& d[i].proc != d_idroplist_proc && d[i].proc);
							
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
	
	update_old_item(section_version, index, zversion, zbuild);
	
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
			QMisc.sprites[q] = word(newsprs[q]);
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
		if (test.scrconfig.script)
		{
			zasm_meta const& meta = guyscripts[test.scrconfig.script]->meta;
			if(!meta.initd_help[i].empty())
				InfoDialog("Info", meta.initd_help[i].c_str()).show();
		}
	}
	else
	{
		i -= 8;
		if (test.weap_data.scrconfig.script)
		{
			zasm_meta const& meta = ewpnscripts[test.weap_data.scrconfig.script]->meta;
			if (!meta.initd_help[i].empty())
				InfoDialog("Info", meta.initd_help[i].c_str()).show();
		}
	}
}

static int32_t copiedGuy;

int32_t readonenpc(PACKFILE *f, int32_t index)
{
	int32_t zversion = 0;
	int32_t zbuild = 0;
	word section_version = 0;
	word cversion = 0;
	char npcstring[64] = {0};

	if(!p_igetl(&zversion, f)) return 0;
	if(!p_igetl(&zbuild, f)) return 0;
	if(!p_igetw(&section_version, f)) return 0;
	if(!p_igetw(&cversion, f)) return 0;

	al_trace("readonenpc section_version: %d\n", section_version);
	
	if(zversion > ZELDA_VERSION)
	{
		al_trace("Cannot read .znpc packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	if(section_version > V_GUYS)
	{
		al_trace("Cannot read .znpc packfile made using V_GUYS (%d)\n", section_version);
		return 0;
	}
	if(section_version < V_GUYS_ZNPC_MIN)
	{
		al_trace("Cannot read .znpc packfile with old format (V_GUYS %d < %d); please re-export from the original quest\n", section_version, V_GUYS_ZNPC_MIN);
		return 0;
	}
	al_trace("Reading a .znpc packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);

	if(!pfread(npcstring, 64, f)) return 0;

	zquestheader tmp_header = {};
	tmp_header.zelda_version = (int16_t)zversion;
	tmp_header.build = (byte)zbuild;

	guydata tempguy;
	if(readguy_single(f, section_version, cversion, &tmp_header, index, tempguy) != 0)
		return 0;

	guysbuf[index] = tempguy;
	strcpy(guy_string[index], npcstring);

	return 1;
}

int32_t writeonenpc(PACKFILE *f, int32_t i)
{
	if(!p_iputl(ZELDA_VERSION, f)) return 0;
	if(!p_iputl(VERSION_BUILD, f)) return 0;
	if(!p_iputw(V_GUYS, f)) return 0;
	if(!write_deprecated_section_cversion(V_GUYS, f)) return 0;
	if(!pfwrite(guy_string[i], 64, f)) return 0;

	return writeguy_single(f, guysbuf[i]) == 0 ? 1 : 0;
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