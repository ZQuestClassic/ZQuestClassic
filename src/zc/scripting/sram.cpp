#include "zc/scripting/sram.h"
#include "base/dmap.h"
#include "base/general.h"
#include "base/ints.h"
#include "base/packfile.h"
#include "base/qst.h"
#include "zc/ffscript.h"
#include "zc/zc_ffc.h"
#include "zc/zelda.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

#define SRAM_VERSION 2

namespace {

void write_dmaps(PACKFILE *f, int32_t vers_id)
{
	word dmap_count=count_dmaps();
  
		dmap_count=zc_min(dmap_count, 512);
		dmap_count=zc_min(dmap_count, MAXDMAPS-0);
		
		//finally...  section data
		if(!p_iputw(dmap_count,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",5);
		}
		
		
		for(int32_t i=0; i<dmap_count; i++)
		{
			if(!p_putc(DMaps[i].map,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",6);
			}
			
			if(!p_iputw(DMaps[i].level,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",7);
			}
			
			if(!p_putc(DMaps[i].xoff,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",8);
			}
			
			if(!p_putc(DMaps[i].compass,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",9);
			}
			
			if(!p_iputw(DMaps[i].color,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",10);
			}
			
			if(!p_putc(DMaps[i].midi,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",11);
			}
			
			if(!p_putc(DMaps[i].cont,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",12);
			}
			
			if(!p_putc(DMaps[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",13);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_putc(DMaps[i].grid[j],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",14);
				}
			}
			
			//16
			if(!pfwrite(&DMaps[i].name,sizeof(DMaps[0].name),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",15);
			}
			
			if(!p_putwstr(DMaps[i].title,f))
            {
                Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",16);
            }
			
			if(!pfwrite(&DMaps[i].intro,sizeof(DMaps[0].intro),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",17);
			}
			
			if(!p_iputl(DMaps[i].minimap_tile[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",18);
			}
			
			if(!p_putc(DMaps[i].minimap_cset[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",19);
			}
			
			if(!p_iputl(DMaps[i].minimap_tile[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",20);
			}
			
			if(!p_putc(DMaps[i].minimap_cset[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",21);
			}
			
			if(!p_iputl(DMaps[i].largemap_tile[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",22);
			}
			
			if(!p_putc(DMaps[i].largemap_cset[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",23);
			}
			
			if(!p_iputl(DMaps[i].largemap_tile[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",24);
			}
			
			if(!p_putc(DMaps[i].largemap_cset[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",25);
			}
			
			if(!pfwrite(&DMaps[i].tmusic,sizeof(DMaps[0].tmusic),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",26);
			}
			
			if(!p_putc(DMaps[i].tmusictrack,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",25);
			}
			
			if(!p_putc(DMaps[i].active_subscreen,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",26);
			}
			
			if(!p_putc(DMaps[i].passive_subscreen,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",27);
			}
			
			byte disabled[32];
			memset(disabled,0,32);
			
			for(int32_t j=0; j<MAXITEMS; j++)
			{
				if(DMaps[i].disableditems[j])
				{
					disabled[j/8] |= (1 << (j%8));
				}
			}
			
			if(!pfwrite(disabled,32,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",28);
			}
			
			if(!p_iputl(DMaps[i].flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",29);
			}
		if(!p_putc(DMaps[i].sideview,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",30);
			}
		if(!p_iputw(DMaps[i].script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",31);
			}
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(DMaps[i].initD[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",32);
		}
			
		}
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
			if (!p_putc(DMaps[i].initD_label[q][w],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",33);
			}
		}
		}
		}
}

void read_dmaps(PACKFILE *f, int32_t vers_id)
{
	word dmap_count=count_dmaps();
  
		dmap_count=zc_min(dmap_count, 512);
		dmap_count=zc_min(dmap_count, MAXDMAPS-0);
		
		//finally...  section data
		if(!p_igetw(&dmap_count,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",5);
		}
		
		
		for(int32_t i=0; i<dmap_count; i++)
		{
			if(!p_getc(&DMaps[i].map,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",6);
			}
			
			if(!p_igetw(&DMaps[i].level,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",7);
			}
			
			if(!p_getc(&DMaps[i].xoff,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",8);
			}
			
			if(!p_getc(&DMaps[i].compass,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",9);
			}
			
			if(!p_igetw(&DMaps[i].color,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",10);
			}
			
			if(!p_getc(&DMaps[i].midi,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",11);
			}
			
			if(!p_getc(&DMaps[i].cont,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",12);
			}
			
			if(!p_getc(&DMaps[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",13);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_getc(&DMaps[i].grid[j],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",14);
				}
			}
			
			//16
			if(!pfread((&DMaps[i].name),sizeof(DMaps[0].name),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",15);
			}
			
			if (!p_getwstr(&DMaps[i].title, f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",16);
			}
			
			if(!pfread((&DMaps[i].intro),sizeof(DMaps[0].intro),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",17);
			}
			
			if(!p_igetl(&DMaps[i].minimap_tile[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",18);
			}
			
			if(!p_getc(&DMaps[i].minimap_cset[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",19);
			}
			
			if(!p_igetl(&DMaps[i].minimap_tile[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",20);
			}
			
			if(!p_getc(&DMaps[i].minimap_cset[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",21);
			}
			
			if(!p_igetl(&DMaps[i].largemap_tile[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",22);
			}
			
			if(!p_getc(&DMaps[i].largemap_cset[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",23);
			}
			
			if(!p_igetl(&DMaps[i].largemap_tile[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",24);
			}
			
			if(!p_getc(&DMaps[i].largemap_cset[1],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",25);
			}
			
			if(!pfread((&DMaps[i].tmusic),sizeof(DMaps[0].tmusic),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",26);
			}
			
			if(!p_getc(&DMaps[i].tmusictrack,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",25);
			}
			
			if(!p_getc(&DMaps[i].active_subscreen,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",26);
			}
			
			if(!p_getc(&DMaps[i].passive_subscreen,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",27);
			}
			
			byte disabled[32];
			memset(disabled,0,32);
			
			for(int32_t j=0; j<MAXITEMS; j++)
			{
				if(&DMaps[i].disableditems[j])
				{
					disabled[j/8] |= (1 << (j%8));
				}
			}
			
			if(!pfread(disabled,32,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",28);
			}
			
			if(!p_igetl(&DMaps[i].flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",29);
			}
		if(!p_getc(&DMaps[i].sideview,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",30);
			}
		if(!p_igetw(&DMaps[i].script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",31);
			}
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_igetl(&DMaps[i].initD[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",32);
		}
			
		}
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
			if (!p_getc(&DMaps[i].initD_label[q][w],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read DMAP NODE: %d",33);
			}
		}
		}
		}
}

void read_combos(PACKFILE *f, int32_t version_id)
{
	
	word combos_used = 0;
	
		if(!p_igetw(&combos_used,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",5);
		}
		
		for(int32_t i=0; i<combos_used; i++)
		{
			if(!p_igetl(&combobuf[i].tile,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",6);
			}
			
			if(!p_getc(&combobuf[i].flip,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",7);
			}
			
			if(!p_getc(&combobuf[i].walk,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",8);
			}
			
			if(!p_getc(&combobuf[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",9);
			}
			
			if(!p_getc(&combobuf[i].csets,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",10);
			}
			
			if(!p_getc(&combobuf[i].frames,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",11);
			}
			
			if(!p_getc(&combobuf[i].speed,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",12);
			}
			
			if(!p_igetw(&combobuf[i].nextcombo,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",13);
			}
			
			if(!p_getc(&combobuf[i].nextcset,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",14);
			}
			
			if(!p_getc(&combobuf[i].flag,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",15);
			}
			
			if(!p_getc(&combobuf[i].skipanim,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",16);
			}
			
			if(!p_igetw(&combobuf[i].nexttimer,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",17);
			}
			
			if(!p_getc(&combobuf[i].skipanimy,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",18);
			}
			
			if(!p_getc(&combobuf[i].animflags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",19);
			}
		
		for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
		if(!p_igetl(&combobuf[i].attributes[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",20);
		}
		}
		if(!p_igetl(&combobuf[i].usrflags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",21);
		}	 
		if(combobuf[i].triggers.empty())
			combobuf[i].triggers.emplace_back();
		if(!p_getbitstr(&combobuf[i].triggers[0].trigger_flags,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",22);
		
		if(!p_igetl(&combobuf[i].triggers[0].triggerlevel,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",23);
		}	
		for ( int32_t q = 0; q < 11; q++ ) 
		{
			if(!p_getc(&combobuf[i].label[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",24);
			}
		}
		for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
			if(!p_getc(&combobuf[i].attribytes[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",25);
			}
		}
		if(!p_igetw(&combobuf[i].script,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",26);
		for ( int32_t q = 0; q < 2; q++ )
		{
			if(!p_igetl(&combobuf[i].initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",27);
			}
		}
		if(!p_igetl(&combobuf[i].o_tile,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",28);
		if(!p_getc(&combobuf[i].cur_frame,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",29);
		if(!p_getc(&combobuf[i].aclk,f))
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",30);
		}

	combo_caches::refresh();
}

void write_combos(PACKFILE *f, int32_t version_id)
{
	
	word combos_used = 0;
	
		//finally...  section data
		combos_used=count_combos()-0;
		combos_used=zc_min(combos_used, MAXCOMBOS);
		
		if(!p_iputw(combos_used,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",5);
		}
		
		for(int32_t i=0; i<combos_used; i++)
		{
			if(!p_iputl(combobuf[i].tile,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",6);
			}
			
			if(!p_putc(combobuf[i].flip,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",7);
			}
			
			if(!p_putc(combobuf[i].walk,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",8);
			}
			
			if(!p_putc(combobuf[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",9);
			}
			
			if(!p_putc(combobuf[i].csets,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",10);
			}
			
			if(!p_putc(combobuf[i].frames,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",11);
			}
			
			if(!p_putc(combobuf[i].speed,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",12);
			}
			
			if(!p_iputw(combobuf[i].nextcombo,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",13);
			}
			
			if(!p_putc(combobuf[i].nextcset,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",14);
			}
			
			if(!p_putc(combobuf[i].flag,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",15);
			}
			
			if(!p_putc(combobuf[i].skipanim,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",16);
			}
			
			if(!p_iputw(combobuf[i].nexttimer,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",17);
			}
			
			if(!p_putc(combobuf[i].skipanimy,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",18);
			}
			
			if(!p_putc(combobuf[i].animflags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",19);
			}
		
		for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
		if(!p_iputl(combobuf[i].attributes[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",20);
		}
		}
		if(!p_iputl(combobuf[i].usrflags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",21);
		}	 
		if(combobuf[i].triggers.empty())
			combobuf[i].triggers.emplace_back();
		if(!p_putbitstr(combobuf[i].triggers[0].trigger_flags,f));
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",22);
		
		if(!p_iputl(combobuf[i].triggers[0].triggerlevel,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",23);
		}	
		for ( int32_t q = 0; q < 11; q++ ) 
		{
			if(!p_putc(combobuf[i].label[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",24);
			}
		}
		for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
		{
			if(!p_putc(combobuf[i].attribytes[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",25);
			}
		}
		if(!p_iputw(combobuf[i].script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",26);
		}
		for ( int32_t q = 0; q < 2; q++ )
		{
			if(!p_iputl(combobuf[i].initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",27);
			}
		}
		if(!p_iputl(combobuf[i].o_tile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",28);
		}
		if(!p_putc(combobuf[i].cur_frame,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",29);
		}
		if(!p_putc(combobuf[i].aclk,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read COMBO NODE: %d",30);
		}
			
		}
}

void read_weaponsprtites(PACKFILE *f, int32_t vers_id)
{   
	for(int32_t i=0; i<MAXWPNS; i++)
	{
		word oldtile = 0;
		if(!p_igetw(&oldtile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",6);
		}
			
		if(!p_getc(&wpnsbuf[i].misc,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",7);
		}
			
		if(!p_getc(&wpnsbuf[i].csets,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",8);
		}
			
		if(!p_getc(&wpnsbuf[i].frames,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",9);
		}
			
		if(!p_getc(&wpnsbuf[i].speed,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",10);
		}
			
		if(!p_getc(&wpnsbuf[i].type,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",11);
		}
		
		if(!p_igetw(&wpnsbuf[i].script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",12);
		}

		if(!p_igetl(&wpnsbuf[i].tile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",12);
		}
	}
}

void write_weaponsprtites(PACKFILE *f, int32_t vers_id)
{   
	for(int32_t i=0; i<MAXWPNS; i++)
	{
		if(!p_iputw(wpnsbuf[i].tile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",6);
		}
			
		if(!p_putc(wpnsbuf[i].misc,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",7);
		}
			
		if(!p_putc(wpnsbuf[i].csets,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",8);
		}
			
		if(!p_putc(wpnsbuf[i].frames,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",9);
		}
			
		if(!p_putc(wpnsbuf[i].speed,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",10);
		}
			
		if(!p_putc(wpnsbuf[i].type,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",11);
		}
		
		if(!p_iputw(wpnsbuf[i].script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",12);
		}
		
		if(!p_iputl(wpnsbuf[i].tile,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read WPNSPRITE NODE: %d",12);
		}
	}
}

void read_enemies(PACKFILE *f, int32_t vers_id)
{
	if ( !f ) return;
	for(int32_t i=0; i<MAXGUYS; i++)
	{
			uint32_t flags1;
			uint32_t flags2;
			if (!p_igetl(&(flags1), f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 6);
			}
			if (!p_igetl(&(flags2), f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 7);;
			}
			guysbuf[i].flags = guy_flags(flags1) | guy_flags(uint64_t(flags2) << 32ULL);
			
			if(!p_igetl(&guysbuf[i].tile,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",8);
			}
			
			if(!p_getc(&guysbuf[i].width,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",9);
			}
			
			if(!p_getc(&guysbuf[i].height,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",10);
			}
			
			if(!p_igetl(&guysbuf[i].s_tile,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",11);
			}
			
			if(!p_getc(&guysbuf[i].s_width,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",12);
			}
			
			if(!p_getc(&guysbuf[i].s_height,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",13);
			}
			
			if(!p_igetl(&guysbuf[i].e_tile,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",14);
			}
			
			if(!p_getc(&guysbuf[i].e_width,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",15);
			}
			
			if(!p_getc(&guysbuf[i].e_height,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",16);
			}
			
			if(!p_igetw(&guysbuf[i].hp,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",17);
			}
			
			if(!p_igetw(&guysbuf[i].type,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",18);
			}
			
			if(!p_igetw(&guysbuf[i].cset,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",19);
			}
			
			if(!p_igetw(&guysbuf[i].anim,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",20);
			}
			
			if(!p_igetw(&guysbuf[i].e_anim,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",21);
			}
			
			if(!p_igetw(&guysbuf[i].frate,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",22);
			}
			
			if(!p_igetw(&guysbuf[i].e_frate,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",23);
			}
			
			if(!p_igetw(&guysbuf[i].dp,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",24);
			}
			
			if(!p_igetw(&guysbuf[i].wdp,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",25);
			}
			
			if(!p_igetw(&guysbuf[i].weapon,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",26);
			}
			
			if(!p_igetw(&guysbuf[i].rate,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",27);
			}
			
			if(!p_igetw(&guysbuf[i].hrate,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",28);
			}
			
			if(!p_igetw(&guysbuf[i].step,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",29);
			}
			
			if(!p_igetw(&guysbuf[i].homing,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",30);
			}
			
			if(!p_igetw(&guysbuf[i].grumble,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",31);
			}
			
			if(!p_igetw(&guysbuf[i].item_set,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",32);
			}
			//misc 1-10
			for (int q = 0; q < 10; ++q)
			{
				if (!p_igetl(&guysbuf[i].attributes[q], f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 33 + q);
				}
			}
			
			if(!p_igetw(&guysbuf[i].bgsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",43);
			}
			
			if(!p_igetw(&guysbuf[i].bosspal,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",44);
			}
			
			if(!p_igetw(&guysbuf[i].extend,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",45);
			}
			
			for(int32_t j=0; j < edefLAST; j++)
			{
			if(!p_getc(&guysbuf[i].defense[j],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",46);
			}
			}
			
			if(!p_getc(&guysbuf[i].hitsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",47);
			}
			
			if(!p_getc(&guysbuf[i].deadsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",48);
			}
			//misc 11-12
			for (int q = 0; q < 2; ++q)
			{
				if (!p_igetl(&guysbuf[i].attributes[10+q], f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 49 + q);
				}
			}
			
			//New 2.6 defences
			for(int32_t j=edefLAST; j < edefLAST255; j++)
			{
			if(!p_getc(&guysbuf[i].defense[j],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",51);
			}
			}
			
			//tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
			if(!p_igetl(&guysbuf[i].txsz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",52);
			}
			if(!p_igetl(&guysbuf[i].tysz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",53);
			}
			if(!p_igetl(&guysbuf[i].hxsz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",54);
			}
			if(!p_igetl(&guysbuf[i].hysz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",55);
			}
			if(!p_igetl(&guysbuf[i].hzsz,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",56);
			}
			// These are not fixed types, but ints, so they are safe to use here. 
			if(!p_igetl(&guysbuf[i].hxofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",57);
			}
			if(!p_igetl(&guysbuf[i].hyofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",58);
			}
			if(!p_igetl(&guysbuf[i].xofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",59);
			}
			if(!p_igetl(&guysbuf[i].yofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",60);
			}
			if(!p_igetl(&guysbuf[i].zofs,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",61);
			}
			if(!p_igetl(&guysbuf[i].wpnsprite,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",62);
			}
			if(!p_igetl(&guysbuf[i].SIZEflags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",63);
			}
			if(!p_igetl(&guysbuf[i].frozentile,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",64);
			}
			if(!p_igetl(&guysbuf[i].frozencset,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",65);
			}
			if(!p_igetl(&guysbuf[i].frozenclock,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",66);
			}
			
			for ( int32_t q = 0; q < 10; q++ ) 
			{
			if(!p_igetw(&guysbuf[i].frozenmisc[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",67);
			}
			}
			if(!p_igetw(&guysbuf[i].firesfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",68);
			}
			//misc 16->32
			for (int q = 0; q < 17; ++q)
			{
				if (!p_igetl(&guysbuf[i].attributes[15 + q], f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 69 + q);
				}
			}
			for ( int32_t q = 0; q < 32; q++ )
			{
				if(!p_igetl(&guysbuf[i].movement[q],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",86);
				}
			}
			for ( int32_t q = 0; q < 32; q++ )
			{
				if(!p_igetl(&guysbuf[i].new_weapon[q],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",87);
				}
			}
			if(!p_igetw(&guysbuf[i].script,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",88);
			}
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&guysbuf[i].initD[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",89);
			}
			}
			if(!p_igetl(&guysbuf[i].editorflags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",91);
			}
			//somehow forgot these in the older builds -Z
			for (int q = 0; q < 3; ++q)
			{
				if (!p_igetl(&guysbuf[i].attributes[12 + q], f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d", 92 + q);
				}
			}
			
			//Enemy Editor InitD[] labels
			for ( int32_t q = 0; q < 8; q++ )
			{
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&guysbuf[i].initD_label[q][w],f))
					{
						Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",95);
					} 
				}
				byte dummy;
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&dummy,f))
					{
						Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",96);
					} 
				}
			}
			if(!p_igetw(&guysbuf[i].weap_data.script,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",97);
			}
			//eweapon initD
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&guysbuf[i].weap_data.initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read GUY NODE: %d",98);
			}
			}

			
	}
}

void write_enemies(PACKFILE *f, int32_t vers_id)
{
	if ( !f ) return;
	for(int32_t i=0; i<MAXGUYS; i++)
	{
		uint32_t flags1 = uint32_t(guysbuf[i].flags);
		uint32_t flags2 = uint32_t(guysbuf[i].flags >> 32ULL);
		if (!p_iputl(flags1, f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 6);
		}
		if (!p_iputl(flags2, f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 7);
		}
		
		if(!p_iputl(guysbuf[i].tile,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",8);
		}
		
		if(!p_putc(guysbuf[i].width,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",9);
		}
		
		if(!p_putc(guysbuf[i].height,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",10);
		}
		
		if(!p_iputl(guysbuf[i].s_tile,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",11);
		}
		
		if(!p_putc(guysbuf[i].s_width,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",12);
		}
		
		if(!p_putc(guysbuf[i].s_height,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",13);
		}
		
		if(!p_iputl(guysbuf[i].e_tile,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",14);
		}
		
		if(!p_putc(guysbuf[i].e_width,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",15);
		}
		
		if(!p_putc(guysbuf[i].e_height,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",16);
		}
		
		if(!p_iputw(guysbuf[i].hp,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",17);
		}
		
		if(!p_iputw(guysbuf[i].type,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",18);
		}
		
		if(!p_iputw(guysbuf[i].cset,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",19);
		}
		
		if(!p_iputw(guysbuf[i].anim,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",20);
		}
		
		if(!p_iputw(guysbuf[i].e_anim,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",21);
		}
		
		if(!p_iputw(guysbuf[i].frate,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",22);
		}
		
		if(!p_iputw(guysbuf[i].e_frate,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",23);
		}
		
		if(!p_iputw(guysbuf[i].dp,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",24);
		}
		
		if(!p_iputw(guysbuf[i].wdp,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",25);
		}
		
		if(!p_iputw(guysbuf[i].weapon,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",26);
		}
		
		if(!p_iputw(guysbuf[i].rate,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",27);
		}
		
		if(!p_iputw(guysbuf[i].hrate,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",28);
		}
		
		if(!p_iputw(guysbuf[i].step,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",29);
		}
		
		if(!p_iputw(guysbuf[i].homing,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",30);
		}
		
		if(!p_iputw(guysbuf[i].grumble,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",31);
		}
		
		if(!p_iputw(guysbuf[i].item_set,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",32);
		}
		
		//misc 1-10
		for (int q = 0; q < 10; ++q)
		{
			if (!p_iputl(guysbuf[i].attributes[q], f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 33+q);
			}
		}
		
		if(!p_iputw(guysbuf[i].bgsfx,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",43);
		}
		
		if(!p_iputw(guysbuf[i].bosspal,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",44);
		}
		
		if(!p_iputw(guysbuf[i].extend,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",45);
		}
		
		for(int32_t j=0; j < edefLAST; j++)
		{
		if(!p_putc(guysbuf[i].defense[j],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",46);
		}
		}
		
		if(!p_putc(guysbuf[i].hitsfx,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",47);
		}
		
		if(!p_putc(guysbuf[i].deadsfx,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",48);
		}
		
		//misc 11-12
		for (int q = 0; q < 2; ++q)
		{
			if (!p_iputl(guysbuf[i].attributes[10+q], f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 49 + q);
			}
		}
		
		//New 2.6 defences
		for(int32_t j=edefLAST; j < edefLAST255; j++)
		{
		if(!p_putc(guysbuf[i].defense[j],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",51);
		}
		}
		
		//tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
		if(!p_iputl(guysbuf[i].txsz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",52);
		}
		if(!p_iputl(guysbuf[i].tysz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",53);
		}
		if(!p_iputl(guysbuf[i].hxsz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",54);
		}
		if(!p_iputl(guysbuf[i].hysz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",55);
		}
		if(!p_iputl(guysbuf[i].hzsz,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",56);
		}
		// These are not fixed types, but ints, so they are safe to use here. 
		if(!p_iputl(guysbuf[i].hxofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",57);
		}
		if(!p_iputl(guysbuf[i].hyofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",58);
		}
		if(!p_iputl(guysbuf[i].xofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",59);
		}
		if(!p_iputl(guysbuf[i].yofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",60);
		}
		if(!p_iputl(guysbuf[i].zofs,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",61);
		}
		if(!p_iputl(guysbuf[i].wpnsprite,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",62);
		}
		if(!p_iputl(guysbuf[i].SIZEflags,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",63);
		}
		if(!p_iputl(guysbuf[i].frozentile,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",64);
		}
		if(!p_iputl(guysbuf[i].frozencset,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",65);
		}
		if(!p_iputl(guysbuf[i].frozenclock,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",66);
		}
		
		for ( int32_t q = 0; q < 10; q++ ) 
		{
		if(!p_iputw(guysbuf[i].frozenmisc[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",67);
		}
		}

		if(!p_iputw(guysbuf[i].firesfx,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",68);
		}
		//misc 16->32
		for (int q=0; q < 17; ++q)
		{
			if (!p_iputl(guysbuf[i].attributes[15 + q], f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 69 + q);
			}
		}
		for ( int32_t q = 0; q < 32; q++ )
		{
			if(!p_iputl(guysbuf[i].movement[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",86);
			}
		}
		for ( int32_t q = 0; q < 32; q++ )
		{
			if(!p_iputl(guysbuf[i].new_weapon[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",87);
			}
		}
		if(!p_iputw(guysbuf[i].script,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",88);
		}
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(guysbuf[i].initD[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",89);
		}
		}
		if(!p_iputl(guysbuf[i].editorflags,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",91);
		}
		//misc 13-15
		for (int q = 0; q < 4; ++q)
		{
			if (!p_iputl(guysbuf[i].attributes[12 + q], f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d", 92 + q);
			}
		}
		
		//Enemy Editor InitD[] labels
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(guysbuf[i].initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",95);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(0,f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",96);
				} 
			}
		}
		if(!p_iputw(guysbuf[i].weap_data.script,f))
		{
		Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",97);
		}
		//eweapon initD
		for ( int32_t q = 0; q < 8; q++ )
		{
		if(!p_iputl(guysbuf[i].weap_data.initd[q],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write GUY NODE: %d",98);
		}
		}
		
	}
}

void write_items(PACKFILE *f, int32_t vers_id)
{
		for(int32_t i=0; i<MAXITEMS; i++)
		{
			if(!p_iputl(itemsbuf[i].tile,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",6);
			}
			
			if(!p_putc(itemsbuf[i].misc_flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",7);
			}
			
			if(!p_putc(itemsbuf[i].csets,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",8);
			}
			
			if(!p_putc(itemsbuf[i].frames,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",9);
			}
			
			if(!p_putc(itemsbuf[i].speed,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",10);
			}
			
			if(!p_putc(itemsbuf[i].delay,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",11);
			}
			
			if(!p_iputl(itemsbuf[i].ltm,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",12);
			}
			
			if(!p_iputl(itemsbuf[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",13);
			}
			
			if(!p_putc(itemsbuf[i].level,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",14);
			}
			
			if(!p_iputl(itemsbuf[i].power,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",14);
			}
			
			if(!p_iputl(itemsbuf[i].flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",15);
			}
			
			if(!p_iputw(itemsbuf[i].script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",16);
			}
			
			if(!p_putc(itemsbuf[i].count,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",17);
			}
			
			if(!p_iputw(itemsbuf[i].amount,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",18);
			}
			
			if(!p_iputw(itemsbuf[i].collect_script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",19);
			}
			
			if(!p_iputw(itemsbuf[i].setmax,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",21);
			}
			
			if(!p_iputw(itemsbuf[i].max,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",22);
			}
			
			if(!p_putc(itemsbuf[i].playsound,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",23);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_iputl(itemsbuf[i].initiald[j],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",24);
				}
			}
			
			if(!p_putc(itemsbuf[i].wpn,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",26);
			}
			
			if(!p_putc(itemsbuf[i].wpn2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",27);
			}
			
			if(!p_putc(itemsbuf[i].wpn3,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",28);
			}
			
			if(!p_putc(itemsbuf[i].wpn4,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",29);
			}
			
			if(!p_putc(itemsbuf[i].wpn5,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",30);
			}
			
			if(!p_putc(itemsbuf[i].wpn6,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",31);
			}
			
			if(!p_putc(itemsbuf[i].wpn7,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",32);
			}
			
			if(!p_putc(itemsbuf[i].wpn8,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",33);
			}
			
			if(!p_putc(itemsbuf[i].wpn9,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",34);
			}
			
			if(!p_putc(itemsbuf[i].wpn10,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",35);
			}
			
			if(!p_putc(itemsbuf[i].pickup_hearts,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",36);
			}
			
			if(!p_iputl(itemsbuf[i].misc1,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",37);
			}
			
			if(!p_iputl(itemsbuf[i].misc2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",38);
			}
			
			if(!p_putc(itemsbuf[i].cost_amount[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",39);
			}
			
			if(!p_iputl(itemsbuf[i].misc3,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",40);
			}
			
			if(!p_iputl(itemsbuf[i].misc4,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",41);
			}
			
			if(!p_iputl(itemsbuf[i].misc5,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",42);
			}
			
			if(!p_iputl(itemsbuf[i].misc6,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",43);
			}
			
			if(!p_iputl(itemsbuf[i].misc7,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",44);
			}
			
			if(!p_iputl(itemsbuf[i].misc8,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",45);
			}
			
			if(!p_iputl(itemsbuf[i].misc9,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",46);
			}
			
			if(!p_iputl(itemsbuf[i].misc10,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",47);
			}
			
			if(!p_putc(itemsbuf[i].usesound,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",48);
			}
			
			if(!p_putc(itemsbuf[i].usesound2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",48);
			}
		
		//New itemdata vars -Z
		//! version 27
		
		if(!p_putc(itemsbuf[i].weap_data.imitate_weapon,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",49);
			}
		if(!p_putc(itemsbuf[i].weap_data.default_defense,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",50);
			}
		if(!p_iputl(itemsbuf[i].weaprange,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",51);
			}
		if(!p_iputl(itemsbuf[i].weapduration,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",52);
			}
		for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) {
			if(!p_iputl(itemsbuf[i].weap_pattern[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",53);
			}
		}
		//version 28
		if(!p_iputl(itemsbuf[i].duplicates,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",54);
		}
		for ( int32_t q = 0; q < INITIAL_D; q++ )
		{
			if(!p_iputl(itemsbuf[i].weap_data.initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",55);
			}
		}

		if(!p_putc(itemsbuf[i].drawlayer,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",57);
		}


		if(!p_iputl(itemsbuf[i].hxofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",58);
		}
		if(!p_iputl(itemsbuf[i].hyofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",59);
		}
		if(!p_iputl(itemsbuf[i].hxsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",60);
		}
		if(!p_iputl(itemsbuf[i].hysz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",61);
		}
		if(!p_iputl(itemsbuf[i].hzsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",62);
		}
		if(!p_iputl(itemsbuf[i].xofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",63);
		}
		if(!p_iputl(itemsbuf[i].yofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",64);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hxofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",65);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hyofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",66);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hxsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",67);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hysz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",68);
		}
		if(!p_iputl(itemsbuf[i].weap_data.hzsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",69);
		}
		if(!p_iputl(itemsbuf[i].weap_data.xofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",70);
		}
		if(!p_iputl(itemsbuf[i].weap_data.yofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",71);
		}
		if(!p_iputw(itemsbuf[i].weap_data.script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",72);
		}
		if(!p_iputl(itemsbuf[i].wpnsprite,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",73);
		}
		if(!p_iputl(itemsbuf[i].magiccosttimer[0],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",74);
		}
		if(!p_iputl(itemsbuf[i].overrideFLAGS,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",75);
		}
		if(!p_iputl(itemsbuf[i].tilew,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",76);
		}
		if(!p_iputl(itemsbuf[i].tileh,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",77);
		}
		if(!p_iputl(itemsbuf[i].weap_data.override_flags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",78);
		}
		if(!p_iputl(itemsbuf[i].weap_data.tilew,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",79);
		}
		if(!p_iputl(itemsbuf[i].weap_data.tileh,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",80);
		}
		if(!p_iputl(itemsbuf[i].pickup,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",81);
		}
		if(!p_iputw(itemsbuf[i].pstring,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",82);
		}
		if(!p_iputw(itemsbuf[i].pickup_string_flags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",83);
		}
		
		if(!p_putc(itemsbuf[i].cost_counter[0],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",84);
		}
		
		//InitD[] labels
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(itemsbuf[i].initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",85);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(0,f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",86);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_putc(itemsbuf[i].sprite_initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",87);
				} 
			}
			if(!p_iputl(itemsbuf[i].sprite_initiald[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",88);
			} 
		}

		if(!p_iputw(itemsbuf[i].sprite_script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to read ITEM NODE: %d",90);
		} 
		
		
		}
}

void read_items(PACKFILE *f, int32_t vers_id)
{
		for(int32_t i=0; i<MAXITEMS; i++)
		{
			if(!p_igetl(&itemsbuf[i].tile,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",6);
			}
			
			if(!p_getc(&itemsbuf[i].misc_flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",7);
			}
			
			if(!p_getc(&itemsbuf[i].csets,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",8);
			}
			
			if(!p_getc(&itemsbuf[i].frames,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",9);
			}
			
			if(!p_getc(&itemsbuf[i].speed,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",10);
			}
			
			if(!p_getc(&itemsbuf[i].delay,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",11);
			}
			
			if(!p_igetl(&itemsbuf[i].ltm,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",12);
			}
			
			if(!p_igetl(&itemsbuf[i].type,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",13);
			}
			
			if(!p_getc(&itemsbuf[i].level,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",14);
			}
			
			if(!p_igetl(&itemsbuf[i].power,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",14);
			}
			
			if(!p_igetl(&itemsbuf[i].flags,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",15);
			}
			
			if(!p_igetw(&itemsbuf[i].script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",16);
			}
			
			if(!p_getc(&itemsbuf[i].count,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",17);
			}
			
			if(!p_igetw(&itemsbuf[i].amount,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",18);
			}
			
			if(!p_igetw(&itemsbuf[i].collect_script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",19);
			}
			
			if(!p_igetw(&itemsbuf[i].setmax,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",21);
			}
			
			if(!p_igetw(&itemsbuf[i].max,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",22);
			}
			
			if(!p_getc(&itemsbuf[i].playsound,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",23);
			}
			
			for(int32_t j=0; j<8; j++)
			{
				if(!p_igetl(&itemsbuf[i].initiald[j],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",24);
				}
			}
			
			if(!p_getc(&itemsbuf[i].wpn,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",26);
			}
			
			if(!p_getc(&itemsbuf[i].wpn2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",27);
			}
			
			if(!p_getc(&itemsbuf[i].wpn3,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",28);
			}
			
			if(!p_getc(&itemsbuf[i].wpn4,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",29);
			}
			
			if(!p_getc(&itemsbuf[i].wpn5,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",30);
			}
			
			if(!p_getc(&itemsbuf[i].wpn6,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",31);
			}
			
			if(!p_getc(&itemsbuf[i].wpn7,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",32);
			}
			
			if(!p_getc(&itemsbuf[i].wpn8,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",33);
			}
			
			if(!p_getc(&itemsbuf[i].wpn9,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",34);
			}
			
			if(!p_getc(&itemsbuf[i].wpn10,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",35);
			}
			
			if(!p_getc(&itemsbuf[i].pickup_hearts,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",36);
			}
			
			if(!p_igetl(&itemsbuf[i].misc1,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",37);
			}
			
			if(!p_igetl(&itemsbuf[i].misc2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",38);
			}
			
			if(!p_getc(&itemsbuf[i].cost_amount[0],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",39);
			}
			
			if(!p_igetl(&itemsbuf[i].misc3,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",40);
			}
			
			if(!p_igetl(&itemsbuf[i].misc4,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",41);
			}
			
			if(!p_igetl(&itemsbuf[i].misc5,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",42);
			}
			
			if(!p_igetl(&itemsbuf[i].misc6,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",43);
			}
			
			if(!p_igetl(&itemsbuf[i].misc7,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",44);
			}
			
			if(!p_igetl(&itemsbuf[i].misc8,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",45);
			}
			
			if(!p_igetl(&itemsbuf[i].misc9,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",46);
			}
			
			if(!p_igetl(&itemsbuf[i].misc10,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",47);
			}
			
			if(!p_getc(&itemsbuf[i].usesound,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",48);
			}
			
			if(!p_getc(&itemsbuf[i].usesound2,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",48);
			}
		
		//New itemdata vars -Z
		//! version 27
		
		if(!p_getc(&itemsbuf[i].weap_data.imitate_weapon,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",49);
			}
		if(!p_getc(&itemsbuf[i].weap_data.default_defense,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",50);
			}
		if(!p_igetl(&itemsbuf[i].weaprange,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",51);
			}
		if(!p_igetl(&itemsbuf[i].weapduration,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",52);
			}
		for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) {
			if(!p_igetl(&itemsbuf[i].weap_pattern[q],f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",53);
			}
		}
		//version 28
		if(!p_igetl(&itemsbuf[i].duplicates,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",54);
		}
		for ( int32_t q = 0; q < INITIAL_D; q++ )
		{
			if(!p_igetl(&itemsbuf[i].weap_data.initd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",55);
			}
		}

		if(!p_getc(&itemsbuf[i].drawlayer,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",57);
		}


		if(!p_igetl(&itemsbuf[i].hxofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",58);
		}
		if(!p_igetl(&itemsbuf[i].hyofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",59);
		}
		if(!p_igetl(&itemsbuf[i].hxsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",60);
		}
		if(!p_igetl(&itemsbuf[i].hysz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",61);
		}
		if(!p_igetl(&itemsbuf[i].hzsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",62);
		}
		if(!p_igetl(&itemsbuf[i].xofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",63);
		}
		if(!p_igetl(&itemsbuf[i].yofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",64);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hxofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",65);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hyofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",66);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hxsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",67);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hysz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",68);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.hzsz,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",69);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.xofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",70);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.yofs,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",71);
		}
		if(!p_igetw(&itemsbuf[i].weap_data.script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",72);
		}
		if(!p_igetl(&itemsbuf[i].wpnsprite,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",73);
		}
		if(!p_igetl(&itemsbuf[i].magiccosttimer[0],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",74);
		}
		if(!p_igetl(&itemsbuf[i].overrideFLAGS,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",75);
		}
		if(!p_igetl(&itemsbuf[i].tilew,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",76);
		}
		if(!p_igetl(&itemsbuf[i].tileh,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",77);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.override_flags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",78);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.tilew,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",79);
		}
		if(!p_igetl(&itemsbuf[i].weap_data.tileh,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",80);
		}
		if(!p_igetl(&itemsbuf[i].pickup,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",81);
		}
		if(!p_igetw(&itemsbuf[i].pstring,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",82);
		}
		if(!p_igetw(&itemsbuf[i].pickup_string_flags,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",83);
		}
		
		if(!p_getc(&itemsbuf[i].cost_counter[0],f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",84);
		}
		
		//InitD[] labels
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_getc(&itemsbuf[i].initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",85);
				} 
			}
			byte dummy;
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_getc(&dummy,f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",86);
				} 
			}
			for ( int32_t w = 0; w < 65; w++ )
			{
				if(!p_getc(&itemsbuf[i].sprite_initD_label[q][w],f))
				{
					Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",87);
				} 
			}
			if(!p_igetl(&itemsbuf[i].sprite_initiald[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",88);
			} 
		}

		if(!p_igetw(&itemsbuf[i].sprite_script,f))
		{
			Z_scripterrlog("do_savegamestructs FAILED to write ITEM NODE: %d",90);
		} 
		
		
		}
}
	
void write_mapscreens(PACKFILE *f,int32_t vers_id)
{
	for(int32_t i=0; i<map_count && i<MAXMAPS; i++)
		{
		for(int32_t j=0; j<MAPSCRS; j++)
		{
			mapscr *m = &TheMaps[i*MAPSCRS+j];
			
			if(!p_putc(m->valid,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->guy,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			{
			if(!p_iputw(m->str,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_putc(m->room,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->item,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->hasitem, f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->tilewarptype[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_iputw(m->door_combo_set,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->warpreturnx[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->warpreturny[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_iputw(m->warpreturnc,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->stairx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->stairy,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->itemx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->itemy,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputw(m->color,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags11,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->door[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_iputw(m->tilewarpdmap[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->tilewarpscr[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_putc(m->tilewarpoverlayflags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->exitdir,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<10; k++)
			{
			{
				if(!p_iputw(m->enemy[k],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			}
			}
			
			if(!p_putc(m->pattern,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->sidewarptype[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_putc(m->sidewarpoverlayflags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->warparrivalx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->warparrivaly,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->path[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_putc(m->sidewarpscr[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_iputw(m->sidewarpdmap[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_putc(m->sidewarpindex,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputw(m->undercombo,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->undercset,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputw(m->catchall,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags2,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags3,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags4,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags5,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputw(m->noreset,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_iputl(m->nocarry,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags6,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags7,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags8,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags9,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->flags10,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->csensitive,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->oceansfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->bosssfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->secretsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->holdupsfx,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_putc(m->layermap[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_putc(m->layerscreen[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_putc(m->layeropacity[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_iputw(m->timedwarptics,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->nextmap,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->nextscr,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_iputw(m->secretcombo[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_putc(m->secretcset[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_putc(m->secretflag[k],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_iputw(m->data[k],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_putc(m->sflag[k], f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_putc(m->cset[k],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			}
			
			if(!p_iputw(m->screen_midi,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->lens_layer,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			m->ensureFFC(32);
			for(int32_t k=0; k<32; k++)
			{
				ffcdata& ffc = m->ffcs[k];
				if(!p_iputw(ffc.data,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_putc(ffc.cset,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputw(ffc.delay,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.x,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.y,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.vx,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.vy,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.ax,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputzf(ffc.ay,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_putc(ffc.link,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.hit_width,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.hit_height,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_putc(ffc.txsz,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_putc(ffc.tysz,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.flags,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputw(ffc.script,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[0],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[1],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[2],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[3],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[4],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[5],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[6],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
				
				if(!p_iputl(ffc.initd[7],f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
				}
			
			}
			
			if(!p_iputw(m->script,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			} 
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_iputl(m->screeninitd[q],f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			} 
				
			}
			if(!p_putc(m->preloadscript,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}
			
			if(!p_putc(m->hidelayers,f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}  
			if(!p_putc(m->hidescriptlayers,f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to write MAPSCR NODEz\n"); return;
			}    
				
			
		} //end mapscr for loop
	}
}

void read_mapscreens(PACKFILE *f,int32_t vers_id)
{
	for(int32_t i=0; i<map_count && i<MAXMAPS; i++)
		{
		for(int32_t j=0; j<MAPSCRS; j++)
		{
			mapscr *m = &TheMaps[i*MAPSCRS+j];
			
			if(!p_getc(&(m->valid),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->guy),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			{
			if(!p_igetw(&(m->str),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_getc(&(m->room),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->item),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->hasitem), f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->tilewarptype[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_igetw(&(m->door_combo_set),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->warpreturnx[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->warpreturny[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_igetw(&(m->warpreturnc),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->stairx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->stairy),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->itemx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->itemy),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetw(&(m->color),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags11),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->door[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_igetw(&(m->tilewarpdmap[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->tilewarpscr[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_getc(&(m->tilewarpoverlayflags),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->exitdir),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<10; k++)
			{
			{
				if(!p_igetw(&(m->enemy[k]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			}
			}
			
			if(!p_getc(&(m->pattern),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->sidewarptype[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_getc(&(m->sidewarpoverlayflags),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->warparrivalx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->warparrivaly),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->path[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_getc(&(m->sidewarpscr[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<4; k++)
			{
			if(!p_igetw(&(m->sidewarpdmap[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_getc(&(m->sidewarpindex),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetw(&(m->undercombo),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->undercset),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetw(&(m->catchall),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags2),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags3),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags4),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags5),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetw(&(m->noreset),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_igetl(&(m->nocarry),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags6),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags7),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags8),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags9),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->flags10),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->csensitive),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->oceansfx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->bosssfx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->secretsfx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->holdupsfx),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_getc(&(m->layermap[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_getc(&(m->layerscreen[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<6; k++)
			{
			if(!p_getc(&(m->layeropacity[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_igetw(&(m->timedwarptics),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->nextmap),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->nextscr),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_igetw(&(m->secretcombo[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_getc(&(m->secretcset[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<128; k++)
			{
			if(!p_getc(&(m->secretflag[k]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_igetw(&(m->data[k]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_getc(&(m->sflag[k]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			for(int32_t k=0; k<176; k++)
			{
			try
			{
				if(!p_getc(&(m->cset[k]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			}
			catch(std::out_of_range& )
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			}
			
			if(!p_igetw(&(m->screen_midi),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if(!p_getc(&(m->lens_layer),f))
			{
			Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}

			m->ensureFFC(32);
			word tempw;
			for(int32_t k=0; k<32; k++)
			{
				ffcdata& ffc = m->ffcs[k];
				if(!p_igetw(&tempw,f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				zc_ffc_set(ffc, tempw);
				
				if(!p_getc(&(ffc.cset),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetw(&(ffc.delay),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.x),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.y),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.vx),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.vy),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.ax),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetzf(&(ffc.ay),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_getc(&(ffc.link),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.hit_width),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.hit_height),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_getc(&(ffc.txsz),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_getc(&(ffc.tysz),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.flags),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetw(&(ffc.script),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[0]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[1]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[2]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[3]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[4]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[5]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[6]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
				
				if(!p_igetl(&(ffc.initd[7]),f))
				{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
				}
			
			}
			
			if(!p_igetw(&(m->script),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			} 
			for ( int32_t q = 0; q < 8; q++ )
			{
			if(!p_igetl(&(m->screeninitd[q]),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			} 
				
			}
			if(!p_getc(&(m->preloadscript),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}
			
			if ( vers_id >= 2 )
			{
			if(!p_getc(&(m->hidelayers),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}  
			if(!p_getc(&(m->hidescriptlayers),f))
			{
				Z_scripterrlog("do_savegamestructs FAILED to read MAPSCR NODE\n"); return;
			}    
				
			}
			
			
		}//end mapscr all for loop
		
	}
}

} // namespace

void do_loadgamestructs(const bool v, const bool v2)
{
	int32_t arrayptr = SH::get_arg(sarg1, v);
	int32_t section_id = SH::get_arg(sarg2, v2) / 10000;
	//Bitwise OR sections together
	string strA;
	ArrayH::getString(arrayptr, strA, 256);
	int32_t temp_flags = section_id; int32_t version = 0;

	if ( FFCore.checkExtension(strA, ".zcsram") )
	{
		PACKFILE *f = pack_fopen_password(strA.c_str(),F_READ, "");
		if (f)
		{
			p_igetl(&version,f);
			p_igetl(&section_id,f);
			if ( version > SRAM_VERSION ) //file version is greater than programme current version.
			{
				Z_scripterrlog("SRAM Version is from a version of ZC newer than the running version and cannot be loaded.\n");
				return;
			}
			if ( section_id != temp_flags )
			{
				Z_scripterrlog("Reading an SRAM file with a section flag mismatch!\nThe file section flags are (%d) and the specified flagset is (%d).\nThis may cause errors!\n", section_id, temp_flags);
			}
			
			if ( !section_id || section_id&svGUYS ) read_enemies(f,version);
			if ( !section_id || section_id&svITEMS )read_items(f,version);
			if ( !section_id || section_id&svWEAPONS ) read_weaponsprtites(f,version);
			if ( !section_id || section_id&svCOMBOS ) 
			{
				reset_all_combo_animations();
				read_combos(f,version);
			}
			if ( !section_id || section_id&svDMAPS ) read_dmaps(f,version);
			if ( !section_id || section_id&svMAPSCR ) read_mapscreens(f,version);
			pack_fclose(f);
			
			set_register(sarg1, 10000);
		}
		else 
		{
			Z_scripterrlog("do_loadgamestructs could not read packfile!");
			set_register(sarg1, -10000);
		}
	}
	else
	{
		Z_scripterrlog("Tried to read a .zcsram file, but the file lacked the ..zcsram extension!\n");
		set_register(sarg1, -20000);
		
	}
}

void do_savegamestructs(const bool v, const bool v2)
{
	int32_t arrayptr = SH::get_arg(sarg1, v);
	int32_t section_id = SH::get_arg(sarg2, v2) / 10000;
	//Bitwise OR sections together
	string strA;
	ArrayH::getString(arrayptr, strA, 256);
	int32_t cycles = 0;

	if ( FFCore.checkExtension(strA, ".zcsram") )
	{
		PACKFILE *f = pack_fopen_password(strA.c_str(),F_WRITE, "");
		if (f)
		{
			p_iputl(SRAM_VERSION,f);
			p_iputl(section_id,f);
			
			if ( !section_id || section_id&svGUYS ) write_enemies(f,SRAM_VERSION);
			if ( !section_id || section_id&svITEMS ) write_items(f,SRAM_VERSION);
			if ( !section_id || section_id&svWEAPONS ) write_weaponsprtites(f,SRAM_VERSION);
			if ( !section_id || section_id&svCOMBOS ) 
			{
				reset_all_combo_animations();
				write_combos(f,SRAM_VERSION);
			}
			if ( !section_id || section_id&svDMAPS ) write_dmaps(f,SRAM_VERSION);
			if ( !section_id || section_id&svMAPSCR ) write_mapscreens(f,SRAM_VERSION);
			pack_fclose(f);
			set_register(sarg1, 10000);
		}
		else 
		{
			Z_scripterrlog("FFCore.do_loadgamestructs could not read packfile!");
			set_register(sarg1, -10000);
		}
	}
	else
	{
		Z_scripterrlog("Tried to write a .zcsram file, but the file lacked the ..zcsram extension!\n");
		set_register(sarg1, -20000);
	}
}
