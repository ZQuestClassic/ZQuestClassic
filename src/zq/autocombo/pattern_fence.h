#ifndef _AUTOPATTERN_FENCE_H_
#define _AUTOPATTERN_FENCE_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"

namespace AutoPattern
{

	class autopattern_fence : protected autopattern_container
	{
	public:
		explicit autopattern_fence(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource, bool nnocrossedge) :
			autopattern_container(ntype, nlayer, nbasescreen, nbasepos, nsource, nnocrossedge) {}
		bool execute(int32_t exscreen, int32_t expos);
		bool erase(int32_t exscreen, int32_t expos);
		void flip_single(apcombo*& ap);
		void flip_all_connected(int32_t exscreen, int32_t expos, int32_t max);
		void form_connections(apcombo* p, bool changecombo = false, bool noadj = false);
		uint32_t slot_to_flags(int32_t slot);
		int32_t flags_to_slot(uint32_t flags);
		int32_t flip_slot(int32_t slot);
		int32_t get_edge_flags(apcombo*& ap, int32_t dir, bool flipped = false);
		int32_t get_turn_edge_flags(apcombo*& ap, int32_t dir);
		int32_t get_num_connections(apcombo*& ap);
	};

}

#endif