#include "base/cpos_info.h"
#include "base/zdefs.h"
#include "base/combo.h"

void cpos_update_cache(int32_t oldid, int32_t newid);

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

//Clear the info with no regard for the cpos cache
void cpos_info::clear()
{
	*this = cpos_info();
}

//Clear the info, updating the cpos cache to the change
void cpos_info::clearInfo()
{
	cpos_update_cache(data,0);
	clear();
}

//Change the data of the combo, updating the cpos cache and triggering other effects
void cpos_info::updateData(int32_t newdata)
{
	if(data != newdata)
	{
		byte csfx = sfx_onchange, cspr = spr_onchange;
		if(flags.get(CPOS_FL_APPEARED))
		{
			newcombo const& cmb = combobuf[data];
			csfx = cmb.sfx_disappear;
			cspr = cmb.spr_disappear;
		}
		
		cpos_update_cache(data,newdata);
		clear();
		data = newdata;
		
		sfx_onchange = csfx;
		spr_onchange = cspr;
	}
}

//Copy from 'other', updating the cpos cache with the changes
void cpos_info::updateInfo(cpos_info const& other)
{
	if(data != other.data)
		cpos_update_cache(data,other.data);
	*this = other;
}

