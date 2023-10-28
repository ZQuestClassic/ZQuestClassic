#ifndef _CPOS_INFO_H_
#define _CPOS_INFO_H_

#include "base/ints.h"
#include "base/containers.h"

enum
{
	CPOS_FL_APPEARED,
	CPOS_FL_TYPE01,
	CPOS_FL_MAX
};
#define CPOS_CRUMBLE_BREAKING CPOS_FL_TYPE01
struct cpos_info
{
	int32_t data;
	byte clk;
	word type_clk;
	byte trig_cd;
	byte pushes[4];
	bitstring flags;
	
	byte sfx_onchange, spr_onchange;
	
	void push(int dir, bool cancel = false);
	word sumpush() const;
	void clear();
	void updateData(int32_t newdata);
};

#endif
