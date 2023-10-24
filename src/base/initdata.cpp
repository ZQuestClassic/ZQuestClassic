#include "base/initdata.h"

zinitdata zinit;

void zinitdata::clear_genscript()
{
	gen_doscript.clear();
	gen_exitState.clear();
	gen_reloadState.clear();
	gen_eventstate.clear();
	gen_initd.clear();
	gen_data.clear();
}
void zinitdata::clear()
{
	static zinitdata zinitdata_default = {};
	*this = zinitdata_default;
}

void zinitdata::copy(zinitdata const& other)
{
	*this = other;
}

