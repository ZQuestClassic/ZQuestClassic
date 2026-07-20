#include "core/initdata.h"

zinitdata zinit;

zinitdata::zinitdata()
{
	for (word q = 0; q < NUMSCRIPTGLOBAL; ++q)
		((script_config&)(global_scrconfig[q])) = script_config(q);
	for (word q = 0; q < NUMSCRIPTHERO; ++q)
		((script_config&)(hero_scrconfig[q])) = script_config(q);
}

void zinitdata::clear_genscript()
{
	gen_doscript.clear();
	gen_exitState.clear();
	gen_reloadState.clear();
	gen_eventstate.clear();
	gen_initd.clear();
	gen_inst_init.clear();
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

