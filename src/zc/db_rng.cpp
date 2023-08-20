#define _ZC_DBRNG
#include "base/random.h"

#if DEBUG_RAND
#include "zc/replay.h"
#include <fmt/format.h>

#undef zc_oldrand
#undef zc_rand
#undef zc_srand

static void db_print(char const* fname, int l, std::string const& extra)
{
	if(FILE* f = fopen("__db_rand.txt","a"))
	{
		char replstr[128] = {0};
		char buf[512] = {0};
		if(replay_is_replaying()) sprintf(replstr, "%06d ", replay_get_frame());
		sprintf(buf, "%s %s%s LINE %d\n", extra.c_str(), replstr, fname, l);
		fwrite(buf, 1, strlen(buf), f);
		fclose(f);
	}
}

int32_t db_oldrand(char const* fname, int l, zc_randgen* rng)
{
	if(!rng) db_print(fname,l,"OLD");
	return zc_oldrand(rng);
}
int32_t db_rand(char const* fname, int l,zc_randgen* rng)
{
	if(!rng) db_print(fname,l,"NEW");
	return zc_rand(rng);
}
int32_t db_rand(char const* fname, int l,int32_t upper,int32_t lower,zc_randgen* rng)
{
	if(!rng) db_print(fname,l,fmt::format("NEW {}-{}",lower,upper));
	return zc_rand(upper,lower,rng);
}
void db_srand(char const* fname, int l, int32_t seedval, zc_randgen* rng)
{
	if(!rng) db_print(fname,l,fmt::format("SEED {}",seedval));
	zc_srand(seedval,rng);
}
#endif

