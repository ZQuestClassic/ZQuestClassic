#include "base/cpos_info.h"
#include "base/zdefs.h"

void cpos_info::push(int dir, bool cancel)
{
	if(unsigned(dir) < 4)
	{
		if(cancel && pushes[oppositeDir[dir]])
			pushes[oppositeDir[dir]] -= 1;
		else pushes[dir] += 1;
	}
}

word cpos_info::sumpush() const
{
	return pushes[0]+pushes[1]+pushes[2]+pushes[3];
}

void cpos_info::clear()
{
	*this = cpos_info();
}

void cpos_info::updateData(int32_t newdata)
{
	if(data != newdata)
	{
		clear();
		data = newdata;
	}
}
