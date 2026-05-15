#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

int32_t write_door_entry(PACKFILE *f, int32_t i)
{
	char name[21];
	memset(name, 0, sizeof(name));
	strncpy(name, DoorComboSetNames[i].c_str(), sizeof(name) - 1);
	if (!pfwrite(name, sizeof(name), f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 4; k++)
			if (!p_iputw(DoorComboSets[i].doorcombo_u[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 4; k++)
			if (!p_putc(DoorComboSets[i].doorcset_u[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 4; k++)
			if (!p_iputw(DoorComboSets[i].doorcombo_d[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 4; k++)
			if (!p_putc(DoorComboSets[i].doorcset_d[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 6; k++)
			if (!p_iputw(DoorComboSets[i].doorcombo_l[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 6; k++)
			if (!p_putc(DoorComboSets[i].doorcset_l[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 6; k++)
			if (!p_iputw(DoorComboSets[i].doorcombo_r[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 6; k++)
			if (!p_putc(DoorComboSets[i].doorcset_r[j][k], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_iputw(DoorComboSets[i].bombdoorcombo_u[j], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_putc(DoorComboSets[i].bombdoorcset_u[j], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_iputw(DoorComboSets[i].bombdoorcombo_d[j], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_putc(DoorComboSets[i].bombdoorcset_d[j], f)) return 1;
	for (int32_t j = 0; j < 3; j++)
		if (!p_iputw(DoorComboSets[i].bombdoorcombo_l[j], f)) return 1;
	for (int32_t j = 0; j < 3; j++)
		if (!p_putc(DoorComboSets[i].bombdoorcset_l[j], f)) return 1;
	for (int32_t j = 0; j < 3; j++)
		if (!p_iputw(DoorComboSets[i].bombdoorcombo_r[j], f)) return 1;
	for (int32_t j = 0; j < 3; j++)
		if (!p_putc(DoorComboSets[i].bombdoorcset_r[j], f)) return 1;
	for (int32_t j = 0; j < 4; j++)
		if (!p_iputw(DoorComboSets[i].walkthroughcombo[j], f)) return 1;
	for (int32_t j = 0; j < 4; j++)
		if (!p_putc(DoorComboSets[i].walkthroughcset[j], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_putc(DoorComboSets[i].flags[j], f)) return 1;
	return 0;
}

int32_t read_door_entry(PACKFILE *f, int32_t i, int32_t zelda_version, bool skip)
{
	DoorComboSet tempDoorComboSet;
	memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
	byte padding;

	char name[21];
	if (!pfread(name, sizeof(name), f)) return 1;
	if (zelda_version < 0x193)
		if (!p_getc(&padding, f)) return 1;

	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 4; k++)
			if (!p_igetw(&tempDoorComboSet.doorcombo_u[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 4; k++)
			if (!p_getc(&tempDoorComboSet.doorcset_u[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 4; k++)
			if (!p_igetw(&tempDoorComboSet.doorcombo_d[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 4; k++)
			if (!p_getc(&tempDoorComboSet.doorcset_d[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 6; k++)
			if (!p_igetw(&tempDoorComboSet.doorcombo_l[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 6; k++)
			if (!p_getc(&tempDoorComboSet.doorcset_l[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 6; k++)
			if (!p_igetw(&tempDoorComboSet.doorcombo_r[j][k], f)) return 1;
	for (int32_t j = 0; j < 9; j++)
		for (int32_t k = 0; k < 6; k++)
			if (!p_getc(&tempDoorComboSet.doorcset_r[j][k], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_getc(&tempDoorComboSet.bombdoorcset_u[j], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_getc(&tempDoorComboSet.bombdoorcset_d[j], f)) return 1;
	for (int32_t j = 0; j < 3; j++)
		if (!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j], f)) return 1;
	for (int32_t j = 0; j < 3; j++)
		if (!p_getc(&tempDoorComboSet.bombdoorcset_l[j], f)) return 1;
	if (zelda_version < 0x193)
		if (!p_getc(&padding, f)) return 1;
	for (int32_t j = 0; j < 3; j++)
		if (!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j], f)) return 1;
	for (int32_t j = 0; j < 3; j++)
		if (!p_getc(&tempDoorComboSet.bombdoorcset_r[j], f)) return 1;
	if (zelda_version < 0x193)
		if (!p_getc(&padding, f)) return 1;
	for (int32_t j = 0; j < 4; j++)
		if (!p_igetw(&tempDoorComboSet.walkthroughcombo[j], f)) return 1;
	for (int32_t j = 0; j < 4; j++)
		if (!p_getc(&tempDoorComboSet.walkthroughcset[j], f)) return 1;
	for (int32_t j = 0; j < 2; j++)
		if (!p_getc(&tempDoorComboSet.flags[j], f)) return 1;
	if (zelda_version < 0x193)
		if (!pfread(&tempDoorComboSet.expansion, sizeof(tempDoorComboSet.expansion), f)) return 1;

	if (!skip)
	{
		memcpy(&DoorComboSets[i], &tempDoorComboSet, sizeof(tempDoorComboSet));
		DoorComboSetNames[i] = name;
	}
	return 0;
}

int32_t readdoorcombosets(PACKFILE *f, zquestheader *Header)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_doors);

    if((Header->zelda_version < 0x192)||
            ((Header->zelda_version == 0x192)&&(Header->build<158)))
    {
        return 0;
    }
    
    word temp_door_combo_set_count=0;
    word dummy_word;
    int32_t dummy_long;
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
        if (read_door_entry(f, i, Header->zelda_version, should_skip))
            return qe_invalid;
    }
    
	if (!should_skip)
		door_combo_set_count=temp_door_combo_set_count;
    
    return 0;
}
