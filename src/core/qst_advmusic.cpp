#include "advanced_music.h"
#include "core/qst.h"
#include "zalleg/packfile.h"

int32_t read_adv_music(PACKFILE *f)
{
	word s_version = 0;
	dword section_size;
	
	if(!p_igetw(&s_version,f))
		return qe_invalid;
	if (s_version > V_ADVMUSIC)
		return qe_version;
	if(!read_deprecated_section_cversion(f))
		return qe_invalid;
	if(!p_igetl(&section_size,f))
		return qe_invalid;
	
	word count = 0;
	if (!p_igetw(&count, f))
		return qe_invalid;
	
	quest_music.clear();
	for (size_t q = 0; q < count; ++q)
	{
		AdvancedMusic& m = quest_music.emplace_back();
		if (auto ret = m.read(f, s_version))
			return ret;
		m.id = q + 1;
	}
	return 0;
}
