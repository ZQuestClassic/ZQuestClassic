#include "random.h"

zc_randgen default_rng;

int zc_oldrand(zc_randgen* rng)
{
	return zc_rand(RAND_MAX, 0, rng);
}

int zc_rand(zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	return (*rng)();
}

int zc_rand(int upper, int lower, zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	if(lower > upper)
	{
		int t = lower;
		lower = upper;
		upper = t;
	}
	std::uniform_int_distribution<int> dist(lower,upper);
	return dist(*rng);
}

void zc_srand(long seedval, zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	rng->seed(seedval);
}