#include "core/autocombo.h"
#include "core/cpool.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

int32_t readcomboaliases(PACKFILE *f, zquestheader *Header, word version, word build)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    version=version;
    build=build;
    
    int32_t dummy;
    word sversion=0, c_sversion;
    
    //section version info
    if(!p_igetw(&sversion,f))
    {
        return qe_invalid;
    }

	if (sversion > V_COMBOALIASES)
			return qe_version;
    
    FFCore.quest_format[vComboAliases] = sversion;
    
    if(!p_igetw(&c_sversion,f))
    {
        return qe_invalid;
    }
    
    //section size
    if(!p_igetl(&dummy,f))
    {
        return qe_invalid;
    }
    
    int32_t max_num_combo_aliases = MAXCOMBOALIASES;
    
    if(sversion < 3) // max saved combo alias' upped from 256 to 2048.
    {
        max_num_combo_aliases = MAX250COMBOALIASES;
    }
    if(sversion < 2) // max saved combo alias' upped from 256 to 2048.
    {
        max_num_combo_aliases = OLDMAXCOMBOALIASES;
    }
    
    for(int32_t j=0; j<max_num_combo_aliases; j++)
    {
        byte width,height,mask,tempcset;
        int32_t count;
        word tempword;
        byte tempbyte;
        
        if(!p_igetw(&tempword,f))
        {
            return qe_invalid;
        }
        
		combo_aliases[j].combo = tempword;
        
        if(!p_getc(&tempbyte,f))
        {
            return qe_invalid;
        }
        
		combo_aliases[j].cset = tempbyte;
        
        if(!p_getc(&width,f))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&height,f))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&mask,f))
        {
            return qe_invalid;
        }
        
        count=(width+1)*(height+1)*(comboa_lmasktotal(mask)+1);
        
		combo_aliases[j].width = width;
		combo_aliases[j].height = height;
		combo_aliases[j].layermask = mask;
		combo_aliases[j].combos.clear();
		combo_aliases[j].csets.clear();
        
        for(int32_t k=0; k<count; k++)
        {
            if(!p_igetw(&tempword,f))
            {
                return qe_invalid;
            }
            
			combo_aliases[j].combos[k] = tempword;
        }
        
        for(int32_t k=0; k<count; k++)
        {
            if(!p_getc(&tempcset,f))
            {
                return qe_invalid;
            }
            
			combo_aliases[j].csets[k] = tempcset;
        }
		combo_aliases[j].combos.normalize();
		combo_aliases[j].csets.normalize();
    }
    
	//Combo pools!
	word num_combo_pools = 0;
	if(sversion >= 4)
	{
		if(!p_igetw(&num_combo_pools,f))
		{
			return qe_invalid;
		}
	}
	
	for(combo_pool& pool : combo_pools)
	{
		pool.clear();
	}
	
	combo_pool temp_cpool;
	for(word cp = 0; cp < num_combo_pools; ++cp)
	{
		int32_t num_combos_in_pool = 0;
		if(!p_igetl(&num_combos_in_pool,f))
		{
			return qe_invalid;
		}
		if(num_combos_in_pool < 1) continue; //nothing to read
		
		temp_cpool.clear();
		
		int32_t cp_cid; int8_t cp_cs; word cp_quant;
		for(auto q = 0; q < num_combos_in_pool; ++q)
		{
			if(!p_igetl(&cp_cid,f))
			{
				return qe_invalid;
			}
			if(!p_getc(&cp_cs,f))
			{
				return qe_invalid;
			}
			if(!p_igetw(&cp_quant,f))
			{
				return qe_invalid;
			}
			temp_cpool.add(cp_cid, cp_cs, cp_quant);
		}
		
		combo_pools[cp] = temp_cpool;
	}

	//Autocombos!
	word num_combo_autos = 0;
	if (sversion >= 5)
	{
		if (!p_igetw(&num_combo_autos, f))
		{
			return qe_invalid;
		}
	}

	for (combo_auto& cauto : combo_autos)
	{
		cauto.clear(true);
	}

	combo_auto temp_cauto;
	for (word ca = 0; ca < num_combo_autos; ++ca)
	{
		byte type;
		int32_t display_cid, erase_cid;
		byte flags, arg;
		if (!p_getc(&type, f))
		{
			return qe_invalid;
		}
		if (!p_igetl(&display_cid, f))
		{
			return qe_invalid;
		}
		if (!p_igetl(&erase_cid, f))
		{
			return qe_invalid;
		}
		if (!p_getc(&flags, f))
		{
			return qe_invalid;
		}
		if (!p_getc(&arg, f))
		{
			return qe_invalid;
		}
		int32_t num_combos_in_cauto = 0;
		if (!p_igetl(&num_combos_in_cauto, f))
		{
			return qe_invalid;
		}
		if (num_combos_in_cauto < 1) continue; //nothing to read

		temp_cauto.clear();

		temp_cauto.setType(type);
		temp_cauto.setDisplay(display_cid);
		temp_cauto.setEraseCombo(erase_cid);
		temp_cauto.setFlags(flags);
		temp_cauto.setArg(arg);

		int32_t ca_cid; byte ca_ctype;  int16_t ca_offset; int16_t ca_engrave_offset;
		for (auto q = 0; q < num_combos_in_cauto; ++q)
		{
			if (!p_getc(&ca_ctype, f))
			{
				return qe_invalid;
			}
			if (!p_igetl(&ca_cid, f))
			{
				return qe_invalid;
			}
			temp_cauto.addEntry(ca_cid, ca_ctype, q, -1);
		}

		combo_autos[ca] = temp_cauto;
	}
	
    return 0;
}
