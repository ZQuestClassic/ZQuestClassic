#include "random.h"

zc_randgen default_rng;

int zc_rand(zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	return (*rng)();
}

void zc_srand(long seedval, zc_randgen* rng)
{
	if(!rng) rng = &default_rng;
	rng->seed(seedval);
}