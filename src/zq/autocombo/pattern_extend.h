#ifndef AUTOPATTERN_EXTEND_H_
#define AUTOPATTERN_EXTEND_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"

namespace AutoPattern
{

	class autopattern_extend : public autopattern_container
	{
	public:
		explicit autopattern_extend(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource) :
			autopattern_container(ntype, nlayer, nbasescreen, nbasepos, nsource), 
			vertical(nsource->flags&ACF_VERTICAL), nocreate(nsource->flags&ACF_NOCREATE), dogroup(nsource->flags& ACF_DOGROUP) {}
		virtual bool execute(int32_t exscreen, int32_t expos) override;
		virtual bool erase(int32_t exscreen, int32_t expos) override;
		bool execute_single(apcombo* p);
		bool erase_single(apcombo* p);
		virtual int32_t get_floating_cid(int32_t s, int32_t p) override;
		void filter_connections(apcombo* ap);
		void calculate_connections(apcombo* p);
		virtual uint32_t slot_to_flags(int32_t slot) override;
		virtual int32_t flags_to_slot(uint32_t flags) override;
	private:
		bool vertical;
		bool nocreate;
		bool dogroup;
	};
}

#endif