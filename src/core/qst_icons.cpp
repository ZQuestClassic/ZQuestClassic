#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

int32_t readgameicons(PACKFILE *f, zquestheader *, miscQdata *Misc)
{
    miscQdata temp_misc;
    word s_version=0;
    byte icons;
    int32_t tempsize=0;
    
    temp_misc = *Misc;
    
    //section version info
    if(!p_igetw(&s_version,f))
    {
        return qe_invalid;
    }

	if (s_version > V_ICONS)
		return qe_version;
    
    FFCore.quest_format[vIcons] = s_version;
    
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
    
    icons=4;
    
    if ( s_version >= 10 )
    {
	    for(int32_t i=0; i<icons; i++)
	    {
		if(!p_igetl(&temp_misc.icons[i],f))
		{
		    return qe_invalid;
		}
	    }	   
    }
    else
    {	    
	    for(int32_t i=0; i<icons; i++)
	    {
		if(!p_igetw(&temp_misc.icons[i],f))
		{
		    return qe_invalid;
		}
	    }
    }

	*Misc = temp_misc;
    
    return 0;
}
