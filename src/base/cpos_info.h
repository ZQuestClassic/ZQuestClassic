#ifndef _CPOS_INFO_H_
#define _CPOS_INFO_H_

#include "base/ints.h"

struct cpos_info
{
	int32_t data;
	byte clk;
	word shootrclk;
	byte trig_cd;
	byte pushes[4];
	bool appeared;
	
	byte sfx_onchange, spr_onchange;
	
	void push(int dir, bool cancel = false);
	word sumpush() const;
	void clear();
	void updateData(int32_t newdata);
};

#endif
