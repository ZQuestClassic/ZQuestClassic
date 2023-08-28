#ifndef _AUTOPATTERN_FLATMTN_H_
#define _AUTOPATTERN_FLATMTN_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"

namespace AutoPattern
{

	class autopattern_flatmtn : protected autopattern_container
	{
	public:
		explicit autopattern_flatmtn(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource, bool nnocrossedge) :
			autopattern_container(ntype, nlayer, nbasescreen, nbasepos, nsource, nnocrossedge) {}
		bool execute(int32_t exscreen, int32_t expos);
		bool erase(int32_t exscreen, int32_t expos);
		void calculate_connections(apcombo* p);
		uint32_t slot_to_flags(int32_t slot);
		int32_t flags_to_slot(uint32_t flags);
	};

}

#endif