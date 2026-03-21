#include "core/misctypes.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

int32_t readmisccolors(PACKFILE *f, zquestheader *Header, miscQdata *Misc)
{
	//these are here to bypass compiler warnings about unused arguments
	Header=Header;
	
	miscQdata temp_misc;
	word s_version=0;
	int32_t tempsize=0;
	word dummyw;
	
	temp_misc = *Misc;
	
	//section version info
	if(!p_igetw(&s_version,f))
	{
		return qe_invalid;
	}

	if (s_version > V_COLORS)
		return qe_version;
	
	FFCore.quest_format[vColours] = s_version;
	
	al_trace("Misc Colours section version: %d\n", s_version);
	
	if(!read_deprecated_section_cversion(f))
	{
		return qe_invalid;
	}
	
	
	//section size
	if(!p_igetl(&tempsize,f))
	{
		return qe_invalid;
	}
	
	//finally...  section data
	readsize=0;
	
	if(!p_getc(&temp_misc.colors.text,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.caption,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.overw_bg,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.dngn_bg,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.dngn_fg,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.cave_fg,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.bs_dk,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.bs_goal,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.compass_lt,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.compass_dk,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.subscr_bg,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.triframe_color,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.hero_dot,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.bmap_bg,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.bmap_fg,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.triforce_cset,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.triframe_cset,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.overworld_map_cset,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.dungeon_map_cset,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.blueframe_cset,f))
	{
		return qe_invalid;
	}
	if(s_version < 4)
	{
		if(!p_igetw(&dummyw,f))
			return qe_invalid;
		temp_misc.colors.triforce_tile = dummyw;
		
		if(!p_igetw(&dummyw,f))
			return qe_invalid;
		temp_misc.colors.triframe_tile = dummyw;
		
		if(!p_igetw(&dummyw,f))
			return qe_invalid;
		temp_misc.colors.overworld_map_tile = dummyw;
		
		if(!p_igetw(&dummyw,f))
			return qe_invalid;
		temp_misc.colors.dungeon_map_tile = dummyw;
		
		if(!p_igetw(&dummyw,f))
			return qe_invalid;
		temp_misc.colors.blueframe_tile = dummyw;
		
		if(!p_igetw(&dummyw,f))
			return qe_invalid;
		temp_misc.colors.HCpieces_tile = dummyw;
	}
	
	if(!p_getc(&temp_misc.colors.HCpieces_cset,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&temp_misc.colors.subscr_shadow,f))
	{
		return qe_invalid;
	}
	
	if(s_version < 2)
	{
		temp_misc.colors.msgtext = 0x01;
	}
	else
	{
		if(!p_getc(&temp_misc.colors.msgtext, f))
		{
			return qe_invalid;
		}
	}
	
	if ( s_version >= 3 ) //expanded tile pages to 825
	{
		if(!p_igetl(&temp_misc.colors.triforce_tile,f))
		{
			 return qe_invalid;
		}
		
		if(!p_igetl(&temp_misc.colors.triframe_tile,f))
		{
			 return qe_invalid;
		}
		
		if(!p_igetl(&temp_misc.colors.overworld_map_tile,f))
		{
			 return qe_invalid;
		}
		
		if(!p_igetl(&temp_misc.colors.dungeon_map_tile,f))
		{
			 return qe_invalid;
		}
		
		if(!p_igetl(&temp_misc.colors.blueframe_tile,f))
		{
			 return qe_invalid;
		}
		
		if(!p_igetl(&temp_misc.colors.HCpieces_tile,f))
		{
			 return qe_invalid;
		}
	}
	
	*Misc = temp_misc;
	
	return 0;
}
