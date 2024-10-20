#ifndef _AUTOPATTERN_DGNCARVE_H_
#define _AUTOPATTERN_DGNCARVE_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"

namespace AutoPattern
{

	class autopattern_dungeoncarve : public autopattern_container
	{
	public:
		explicit autopattern_dungeoncarve(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource) :
			autopattern_container(ntype, nlayer, nbasescreen, nbasepos, nsource), uniquecorners(nsource->flags&ACF_UNIQUECORNER) {}
		virtual bool execute(int32_t exscreen, int32_t expos) override;
		virtual bool erase(int32_t exscreen, int32_t expos) override;
		virtual int32_t get_floating_cid(int32_t s, int32_t p) override;
		void convert_corner(apcombo* p, bool rem);
		void place_corner(apcombo* p, int32_t xo, int32_t yo, int32_t slots[], int32_t topleft, bool rem);
		void form_connections(apcombo* p, bool rem);
		void calculate_connections(apcombo* p);
		int32_t get_alt_slot(int32_t slot);
		virtual uint32_t slot_to_flags(int32_t slot) override;
		virtual int32_t flags_to_slot(uint32_t flags) override;
	private:
		bool uniquecorners;
	};

}

#endif