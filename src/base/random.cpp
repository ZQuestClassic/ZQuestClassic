#include "base/random.h"

zc_randgen default_rng;

int32_t zc_oldrand(zc_randgen* rng)
{
	return zc_rand(RAND_MAX, 0, rng);
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
	// std::uniform_int_distribution<int32_t> dist(lower,upper);
	// return dist(*rng);
	return signed(unsigned(zc_rand(rng))%unsigned((upper-lower)+1))+lower;
}

void zc_srand(int32_t seedval, zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	rng->seed(seedval);
}