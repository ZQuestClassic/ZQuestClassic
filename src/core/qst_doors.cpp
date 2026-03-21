#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

int32_t readdoorcombosets(PACKFILE *f, zquestheader *Header)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_doors);

    if((Header->zelda_version < 0x192)||
            ((Header->zelda_version == 0x192)&&(Header->build<158)))
    {
        return 0;
    }
    
    word temp_door_combo_set_count=0;
    DoorComboSet tempDoorComboSet;
    word dummy_word;
    int32_t dummy_long;
    byte padding;
    int32_t s_version = 0;
    
	if (!should_skip)
	{
		DoorComboSets = {};
		DoorComboSetNames = {};
	}
    
    if(Header->zelda_version > 0x192)
    {
        //section version info
        if(!p_igetw(&s_version,f))
        {
            return qe_invalid;
        }

		if (s_version > V_DOORS)
			return qe_version;
	
        FFCore.quest_format[vDoors] = s_version;
	
        if(!p_igetw(&dummy_word,f))
        {
            return qe_invalid;
        }
        
        //section size
        if(!p_igetl(&dummy_long,f))
        {
            return qe_invalid;
        }
    }
    
    //finally...  section data
    if(!p_igetw(&temp_door_combo_set_count,f))
    {
        return qe_invalid;
    }

	if (temp_door_combo_set_count > MAXDOORCOMBOSETS)
	{
		return qe_invalid;
	}
    
    for(int32_t i=0; i<temp_door_combo_set_count; i++)
    {
        memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
        
        //name
		char name[21];
        if(!pfread(&name,sizeof(name),f))
        {
            return qe_invalid;
        }
		if (!should_skip)
			DoorComboSetNames[i] = name;
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&padding,f))
            {
                return qe_invalid;
            }
        }
        
        //up door
        for(int32_t j=0; j<9; j++)
        {
            for(int32_t k=0; k<4; k++)
            {
                if(!p_igetw(&tempDoorComboSet.doorcombo_u[j][k],f))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int32_t j=0; j<9; j++)
        {
            for(int32_t k=0; k<4; k++)
            {
                if(!p_getc(&tempDoorComboSet.doorcset_u[j][k],f))
                {
                    return qe_invalid;
                }
            }
        }
        
        //down door
        for(int32_t j=0; j<9; j++)
        {
            for(int32_t k=0; k<4; k++)
            {
                if(!p_igetw(&tempDoorComboSet.doorcombo_d[j][k],f))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int32_t j=0; j<9; j++)
        {
            for(int32_t k=0; k<4; k++)
            {
                if(!p_getc(&tempDoorComboSet.doorcset_d[j][k],f))
                {
                    return qe_invalid;
                }
            }
        }
        
        //left door
        for(int32_t j=0; j<9; j++)
        {
            for(int32_t k=0; k<6; k++)
            {
                if(!p_igetw(&tempDoorComboSet.doorcombo_l[j][k],f))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int32_t j=0; j<9; j++)
        {
            for(int32_t k=0; k<6; k++)
            {
                if(!p_getc(&tempDoorComboSet.doorcset_l[j][k],f))
                {
                    return qe_invalid;
                }
            }
        }
        
        //right door
        for(int32_t j=0; j<9; j++)
        {
            for(int32_t k=0; k<6; k++)
            {
                if(!p_igetw(&tempDoorComboSet.doorcombo_r[j][k],f))
                {
                    return qe_invalid;
                }
            }
        }
        
        for(int32_t j=0; j<9; j++)
        {
            for(int32_t k=0; k<6; k++)
            {
                if(!p_getc(&tempDoorComboSet.doorcset_r[j][k],f))
                {
                    return qe_invalid;
                }
            }
        }
        
        //up bomb rubble
        for(int32_t j=0; j<2; j++)
        {
            if(!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j],f))
            {
                return qe_invalid;
            }
        }
        
        for(int32_t j=0; j<2; j++)
        {
            if(!p_getc(&tempDoorComboSet.bombdoorcset_u[j],f))
            {
                return qe_invalid;
            }
        }
        
        //down bomb rubble
        for(int32_t j=0; j<2; j++)
        {
            if(!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j],f))
            {
                return qe_invalid;
            }
        }
        
        for(int32_t j=0; j<2; j++)
        {
            if(!p_getc(&tempDoorComboSet.bombdoorcset_d[j],f))
            {
                return qe_invalid;
            }
        }
        
        //left bomb rubble
        for(int32_t j=0; j<3; j++)
        {
            if(!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j],f))
            {
                return qe_invalid;
            }
        }
        
        for(int32_t j=0; j<3; j++)
        {
            if(!p_getc(&tempDoorComboSet.bombdoorcset_l[j],f))
            {
                return qe_invalid;
            }
        }
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&padding,f))
            {
                return qe_invalid;
            }
            
        }
        
        //right bomb rubble
        for(int32_t j=0; j<3; j++)
        {
            if(!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j],f))
            {
                return qe_invalid;
            }
        }
        
        for(int32_t j=0; j<3; j++)
        {
            if(!p_getc(&tempDoorComboSet.bombdoorcset_r[j],f))
            {
                return qe_invalid;
            }
        }
        
        if(Header->zelda_version < 0x193)
        {
            if(!p_getc(&padding,f))
            {
                return qe_invalid;
            }
        }
        
        //walkthrough stuff
        for(int32_t j=0; j<4; j++)
        {
            if(!p_igetw(&tempDoorComboSet.walkthroughcombo[j],f))
            {
                return qe_invalid;
            }
        }
        
        for(int32_t j=0; j<4; j++)
        {
            if(!p_getc(&tempDoorComboSet.walkthroughcset[j],f))
            {
                return qe_invalid;
            }
        }
        
        //flags
        for(int32_t j=0; j<2; j++)
        {
            if(!p_getc(&tempDoorComboSet.flags[j],f))
            {
                return qe_invalid;
            }
        }
        
        if(Header->zelda_version < 0x193)
        {
            if(!pfread(&tempDoorComboSet.expansion,sizeof(tempDoorComboSet.expansion),f))
            {
                return qe_invalid;
            }
        }
        
        if (!should_skip)
            memcpy(&DoorComboSets[i], &tempDoorComboSet, sizeof(tempDoorComboSet));
    }
    
	if (!should_skip)
		door_combo_set_count=temp_door_combo_set_count;
    
    return 0;
}
