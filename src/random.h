#ifndef RANDOM_H
#define RANDOM_H

#include "zdefs.h"
#include <random>

typedef std::mt19937 zc_randgen;

int zc_oldrand(zc_randgen* rng=NULL);
int zc_rand(zc_randgen* rng=NULL);
int zc_rand(int upper, int lower=0, zc_randgen* rng=NULL);
void zc_srand(long seedval, zc_randgen* rng=NULL);

#endif

