#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

int32_t readfavorites(PACKFILE *f, int32_t)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_favorites);

	int32_t temp_num;
	dword dummy_dword;
	word num_favorite_combos;
	word num_favorite_combo_aliases;
	word s_version=0;
	
	//section version info
	if(!p_igetw(&s_version,f))
	{
		return qe_invalid;
	}

	if (s_version > V_FAVORITES)
		return qe_version;
	
	if (!should_skip)
		FFCore.quest_format[vFavourites] = s_version;
	
	if(!read_deprecated_section_cversion(f))
	{
		return qe_invalid;
	}
	
	//section size
	if(!p_igetl(&dummy_dword,f))
	{
		return qe_invalid;
	}
	
	word per_row = FAVORITECOMBO_PER_ROW;
	word per_page = FAVORITECOMBO_PER_PAGE;
	if(s_version >= 3)
		if(!p_igetw(&per_row,f))
			return qe_invalid;
	if(s_version >= 4)
		if(!p_igetw(&per_page,f))
			return qe_invalid;
	//finally...  section data
	if(!p_igetw(&num_favorite_combos,f))
	{
		return qe_invalid;
	}
	// num_favorite_combos and per_row are file-controlled; guard against OOB writes
	// into favorite_combos/favorite_combo_modes (and a div-by-zero on per_row below).
	if(num_favorite_combos > MAXFAVORITECOMBOS)
	{
		return qe_invalid;
	}

	//Hack; port old favorite combos
	if(s_version < 3 && num_favorite_combos == 100)
		per_row = 13;

	if (!should_skip)
	for(int q = 0; q < MAXFAVORITECOMBOS; ++q)
		favorite_combos[q] = -1;
	byte favtype = 0;
	for(int32_t i=0; i<num_favorite_combos; i++)
	{
		if (s_version >= 4)
		{
			if (!p_getc(&favtype, f))
			{
				return qe_invalid;
			}
		}
		else
			favtype = 0;
		if(!p_igetl(&temp_num,f))
		{
			return qe_invalid;
		}

		if (should_skip)
			continue;
		
		if(per_row == FAVORITECOMBO_PER_ROW)
		{
			favorite_combos[i] = temp_num;
			favorite_combo_modes[i] = favtype;
		}
		else if(per_row != 0)
		{
			int new_i = (i%per_row) + (i/per_row)*FAVORITECOMBO_PER_ROW;
			if(new_i >= 0 && new_i < MAXFAVORITECOMBOS)
			{
				favorite_combos[new_i]=temp_num;
				favorite_combo_modes[new_i] = favtype;
			}
		}
	}
	
	// Discard the separate favorite aliases list from previous versions
	if(s_version<4)
	{
		if (!p_igetw(&num_favorite_combo_aliases, f))
		{
			return qe_invalid;
		}

		for (int32_t i = 0; i < num_favorite_combo_aliases; i++)
		{
			if (!p_igetl(&temp_num, f))
			{
				return qe_invalid;
			}
		}
	}
	
	word max_combo_cols = 0;
	word max_mappages = 0;
	if(s_version >= 2)
	{
		if(!p_igetw(&max_combo_cols,f))
			return qe_invalid;
		int32_t tmp = 0, tmp2 = 0, tmp3 = 0;
		for(int q = 0; q < max_combo_cols; ++q)
		{
			if(!p_igetl(&tmp,f))
				return qe_invalid;
			if(!p_igetl(&tmp2,f))
				return qe_invalid;
			if(!p_igetl(&tmp3,f))
				return qe_invalid;
			if(q < MAX_COMBO_COLS)
			{
				First[q] = tmp;
				combo_alistpos[q] = tmp2;
				combo_pool_listpos[q] = tmp3;
			}
		}
		
		if(!p_igetw(&max_mappages,f))
			return qe_invalid;
		for(int q = 0; q < max_mappages; ++q)
		{
			if(!p_igetl(&tmp,f))
				return qe_invalid;
			if(!p_igetl(&tmp2,f))
				return qe_invalid;
			if(q < MAX_MAPPAGE_BTNS)
			{
				map_page[q].map = tmp;
				map_page[q].screen = tmp2;
			}
		}
	}

	if (should_skip)
		return 0;

	for(int q = max_combo_cols; q < MAX_COMBO_COLS; ++q)
	{
		First[q] = 0;
		combo_alistpos[q] = 0;
		combo_pool_listpos[q] = 0;
	}
	for(int q = max_mappages; q < MAX_MAPPAGE_BTNS; ++q)
	{
		map_page[q].map = 0;
		map_page[q].screen = 0;
	}
	
	return 0;
}
