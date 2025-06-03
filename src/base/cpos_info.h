#ifndef CPOS_INFO_H_
#define CPOS_INFO_H_

#include "base/ints.h"
#include "base/containers.h"

struct cpos_trig_info
{
	byte clk;
	byte cooldown;
	bool operator==(cpos_trig_info const& other) const = default;
};

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
	word type_clk;
	byte pushes[4];
	bool crumbling, appeared;
	
	byte sfx_onchange, spr_onchange;
	
	bounded_vec<byte,cpos_trig_info> trig_data = {255};
	
	void push(int dir, bool cancel = false);
	word sumpush() const;
	void clear();
	void updateData(int32_t newdata);
	void updateInfo(cpos_info const& other);
	void clearInfo();
};

#endif
