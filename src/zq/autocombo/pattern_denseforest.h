#ifndef AUTOPATTERN_DENSEFOREST_H_
#define AUTOPATTERN_DENSEFOREST_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"

namespace AutoPattern
{

	class autopattern_denseforest : public autopattern_container
	{
	public:
		explicit autopattern_denseforest(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource) :
			autopattern_container(ntype, nlayer, nbasescreen, nbasepos, nsource),
			offsets(nsource->getOffsets()), connectsolid(nsource->flags& ACF_CONNECTSOLID) {}
		virtual bool execute(int32_t exscreen, int32_t expos) override;
		virtual bool erase(int32_t exscreen, int32_t expos) override;
		virtual int32_t get_floating_cid(int32_t s, int32_t p) override;
		int32_t overlap_slot(int32_t slot);
		void filter_connections(apcombo* ap);
		void calculate_connections(apcombo* p);
		virtual uint32_t slot_to_flags(int32_t slot) override;
		virtual int32_t flags_to_slot(uint32_t flags) override;
	private:
		std::pair<byte, byte> offsets;
		bool connectsolid;
	};

}

#endif