#include "base/ints.h"
#include "core/qrs.h"
#include "core/qst.h"
#include "defdata.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern byte deprecated_rules[QUESTRULES_NEW_SIZE];

void init_item_drop_sets()
{
    for(int32_t i=0; i<MAXITEMDROPSETS; i++)
    {
//    item_drop_sets[i] = default_item_drop_sets[0];
        memset(&item_drop_sets[i], 0, sizeof(item_drop_object));
    }
    
    for(int32_t i=0; i<isMAX; i++)
    {
        item_drop_sets[i] = default_item_drop_sets[i];
        
        // Deprecated: qr_NOCLOCKS and qr_ALLOW10RUPEEDROPS
        for(int32_t j=0; j<10; ++j)
        {
            int32_t it = item_drop_sets[i].item[j];
            
            if((itemsbuf.get(it).type == itype_rupee && ((itemsbuf.get(it).amount)&0xFFF) == 10)
                    && !get_bit(deprecated_rules, qr_ALLOW10RUPEEDROPS_DEP))
            {
                item_drop_sets[i].chance[j+1]=0;
            }
            else if(itemsbuf.get(it).type == itype_clock && get_bit(deprecated_rules, qr_NOCLOCKS_DEP))
            {
                item_drop_sets[i].chance[j+1]=0;
            }
            
            // From Sept 2007 to Dec 2008, non-gameplay items were prohibited.
            if(itemsbuf.get(it).type == itype_misc)
            {
                // If a non-gameplay item was selected, then item drop was aborted.
                // Reflect this by increasing the 'Nothing' chance accordingly.
                item_drop_sets[i].chance[0]+=item_drop_sets[i].chance[j+1];
                item_drop_sets[i].chance[j+1]=0;
            }
        }
    }
}

int32_t readitemdropsets(PACKFILE *f, int32_t version)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_itemdropsets);

    dword dummy_dword;
    word item_drop_sets_to_read=0;
    item_drop_object tempitemdrop;
    word s_version=0;
    
	if (!should_skip)
	for(int32_t i=0; i<MAXITEMDROPSETS; i++)
	{
		memset(&item_drop_sets[i], 0, sizeof(item_drop_object));
	}
    
    if(version > 0x192)
    {
        item_drop_sets_to_read=0;
        
        //section version info
        if(!p_igetw(&s_version,f))
        {
            return qe_invalid;
        }

		if (s_version > V_ITEMDROPSETS)
			return qe_version;
	
	FFCore.quest_format[vItemDropsets] = s_version;
        
        if(!read_deprecated_section_cversion(f))
        {
            return qe_invalid;
        }
        
        //section size
        if(!p_igetl(&dummy_dword,f))
        {
            return qe_invalid;
        }
        
        //finally...  section data
        if(!p_igetw(&item_drop_sets_to_read,f))
        {
            return qe_invalid;
        }

        if (item_drop_sets_to_read > MAXITEMDROPSETS)
        {
            return qe_invalid;
        }
    }
    else
    {
		init_item_drop_sets();
    }
    
    if(s_version>=1)
    {
        for(int32_t i=0; i<item_drop_sets_to_read; i++)
        {
            if(!p_getstr(tempitemdrop.name,sizeof(tempitemdrop.name)-1,f))
            {
                return qe_invalid;
            }
            
            for(int32_t j=0; j<10; ++j)
            {
                if(!p_igetw(&tempitemdrop.item[j],f))
                {
                    return qe_invalid;
                }
            }
            
            for(int32_t j=0; j<11; ++j)
            {
                if(!p_igetw(&tempitemdrop.chance[j],f))
                {
                    return qe_invalid;
                }
            }
            
            // Dec 2008: Addition of the 'Tall Grass' set, #12,
            // overrides the quest's set #12.
            if(s_version<2 && i==12)
                continue;
                
            // Deprecated: qr_NOCLOCKS and qr_ALLOW10RUPEEDROPS
            if(s_version<2) for(int32_t j=0; j<10; ++j)
                {
                    int32_t it = tempitemdrop.item[j];
                    
                    if((itemsbuf.get(it).type == itype_rupee
                            && ((itemsbuf.get(it).amount)&0xFFF) == 10)
                            && !get_bit(deprecated_rules, qr_ALLOW10RUPEEDROPS_DEP))
                    {
                        tempitemdrop.chance[j+1]=0;
                    }
                    else if(itemsbuf.get(it).type == itype_clock && get_bit(deprecated_rules, qr_NOCLOCKS_DEP))
                    {
                        tempitemdrop.chance[j+1]=0;
                    }
                    
                    // From Sept 2007 to Dec 2008, non-gameplay items were prohibited.
                    if(itemsbuf.get(it).type == itype_misc)
                    {
                        // If a non-gameplay item was selected, then item drop was aborted.
                        // Reflect this by increasing the 'Nothing' chance accordingly.
                        tempitemdrop.chance[0]+=tempitemdrop.chance[j+1];
                        tempitemdrop.chance[j+1]=0;
                    }
                }
            
			if (!should_skip)
				memcpy(&item_drop_sets[i], &tempitemdrop, sizeof(item_drop_object));
        }
    }
    
    return 0;
}
