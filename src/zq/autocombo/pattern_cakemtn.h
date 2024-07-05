#ifndef AUTOPATTERN_CAKEMTN_H_
#define AUTOPATTERN_CAKEMTN_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"
#include "zq/autocombo/pattern_fence.h"

namespace AutoPattern
{

	class autopattern_cakemtn : public autopattern_fence
	{
	public:
		explicit autopattern_cakemtn(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource, byte nheight = 1) :
			autopattern_fence(ntype, nlayer, nbasescreen, nbasepos, nsource), height(nheight) {}
		virtual bool execute(int32_t exscreen, int32_t expos) override;
		virtual bool erase(int32_t exscreen, int32_t expos) override;
		virtual int32_t get_floating_cid(int32_t s, int32_t p) override;
		virtual void flip_single(apcombo*& ap) override;
		virtual uint32_t slot_to_flags(int32_t slot) override;
		virtual int32_t flags_to_slot(uint32_t flags) override;
		virtual int32_t flip_slot(int32_t slot) override;
		virtual void get_turn_flags(int32_t &ret, int32_t dir, int32_t adjslot) override;
		int32_t get_south_face_id(int32_t slot);
		void recalculate_height(apcombo*& ap, int32_t oldheight);
		void resize_connected(int32_t exscreen, int32_t expos, int32_t max, int32_t newsize);
	private:
		byte height;
		bool flip;
	};

}

#endif