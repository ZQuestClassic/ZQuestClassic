#include "base/random.h"

zc_randgen default_rng;

zc_randgen* zc_get_default_rand()
{
	return &default_rng;
}

int32_t zc_rand(zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	return (*rng)();
}

int32_t zc_rand(int32_t upper, int32_t lower, zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	if(lower > upper)
	{
		int32_t t = lower;
		lower = upper;
		upper = t;
	}
	return signed(unsigned(zc_rand(rng))%unsigned((upper-lower)+1))+lower;
}

void zc_srand(int32_t seedval, zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	rng->seed(seedval);
}

int32_t zc_oldrand(zc_randgen* rng)
{
	// RAND_MAX can't be used because it is platform dependent, and we need
	// reproducible randomness. 0x7fff is the value MSVC uses.
	return zc_rand(0x7fff, 0, rng);
}
