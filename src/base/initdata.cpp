#include "base/initdata.h"

zinitdata zinit;

void zinitdata::clear_genscript()
{
	memset(gen_doscript, 0, sizeof(gen_doscript));
	memset(gen_exitState, 0, sizeof(gen_exitState));
	memset(gen_reloadState, 0, sizeof(gen_reloadState));
	memset(gen_eventstate, 0, sizeof(gen_eventstate));
	memset(gen_initd, 0, sizeof(gen_initd));
	for(size_t q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		gen_data[q].clear();
		gen_data[q].resize(0);
	}
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

