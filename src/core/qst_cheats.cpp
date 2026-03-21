#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

int32_t readcheatcodes(PACKFILE *f, zquestheader *Header)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_cheats);

    int32_t dummy;
    ZCHEATS tempzcheats;
    char temp_use_cheats=1;
    memset(&tempzcheats, 0, sizeof(tempzcheats));
    word s_version = 0;
    
    if(Header->zelda_version > 0x192)
    {
        //section version info
        if(!p_igetw(&s_version,f))
        {
            return qe_invalid;
        }

		if (s_version > V_CHEATS)
			return qe_version;
        
	FFCore.quest_format[vCheats] = s_version;
        if(!p_igetw(&dummy,f))
        {
            return qe_invalid;
        }
        
        //section size
        if(!p_igetl(&dummy,f))
        {
            return qe_invalid;
        }
        
        //finally...  section data
        if(!p_getc(&temp_use_cheats,f))
        {
            return qe_invalid;
        }
    }
    
    if(Header->data_flags[ZQ_CHEATS2])
    {
        if(!p_igetl(&tempzcheats.flags,f))
        {
            return qe_invalid;
        }
        
        if(!pfread(&tempzcheats.codes, sizeof(tempzcheats.codes),f))
        {
            return qe_invalid;
        }
    }
    
	if (should_skip)
		return 0;

	memcpy(&zcheats, &tempzcheats, sizeof(tempzcheats));
	Header->data_flags[ZQ_CHEATS2]=temp_use_cheats;
    
    return 0;
}
