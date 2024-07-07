#ifndef AUTOPATTERN_FENCE_H_
#define AUTOPATTERN_FENCE_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"

namespace AutoPattern
{

	class autopattern_fence : public autopattern_container
	{
	public:
		explicit autopattern_fence(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource) :
			autopattern_container(ntype, nlayer, nbasescreen, nbasepos, nsource), flip(nsource->flags&ACF_FLIP) {}
		virtual bool execute(int32_t exscreen, int32_t expos) override;
		virtual bool erase(int32_t exscreen, int32_t expos) override;
		virtual int32_t get_floating_cid(int32_t s, int32_t p) override;
		virtual uint32_t slot_to_flags(int32_t slot) override;
		virtual int32_t flags_to_slot(uint32_t flags) override;
		virtual void get_turn_flags(int32_t &ret, int32_t dir, int32_t adjslot);
		virtual int32_t flip_slot(int32_t slot);
		virtual void flip_single(apcombo*& ap);
		int32_t flags_to_slot_s(int32_t curslot, uint32_t flags);
		void flip_all_connected(int32_t exscreen, int32_t expos, int32_t max);
		void form_connections(apcombo* p, bool changecombo = false, bool noadj = false);
		int32_t get_edge_flags(apcombo*& ap, int32_t dir, bool flipped = false);
		int32_t get_turn_edge_flags(apcombo*& ap, int32_t dir);
		int32_t get_num_connections(apcombo*& ap);
	private:
		bool flip;
	};

}

#endif