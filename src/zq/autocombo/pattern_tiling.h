#ifndef _AUTOPATTERN_TILING_H_
#define _AUTOPATTERN_TILING_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"

namespace AutoPattern
{

	class autopattern_tiling : public autopattern_container
	{
	public:
		explicit autopattern_tiling(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource, bool nnocrossedge, byte nsize, std::pair<byte,byte> noffsets) :
			autopattern_container(ntype, nlayer, nbasescreen, nbasepos, nsource, nnocrossedge),
			size(std::make_pair((nsize & 0xF) + 1, ((nsize >> 4) & 0xF) + 1)), offsets(noffsets) {}
		virtual bool execute(int32_t exscreen, int32_t expos) override;
		virtual bool erase(int32_t exscreen, int32_t expos) override;
		virtual uint32_t slot_to_flags(int32_t slot) override;
		virtual int32_t flags_to_slot(uint32_t flags) override;
	private:
		std::pair<byte, byte> size;
		std::pair<byte, byte> offsets;
	};

}

#endif