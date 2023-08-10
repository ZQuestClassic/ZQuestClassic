#ifndef RANDOM_H
#define RANDOM_H

#include "base/zdefs.h"
#include <random>

#ifdef IS_PLAYER
#define DEBUG_RAND false
#else
#define DEBUG_RAND false //Can only work when IS_PLAYER
#endif

typedef std::mt19937 zc_randgen;
zc_randgen* zc_get_default_rand();

#if !DEBUG_RAND || defined(_ZC_DBRNG)
int32_t zc_oldrand(zc_randgen* rng=NULL);
int32_t zc_rand(zc_randgen* rng=NULL);
int32_t zc_rand(int32_t upper, int32_t lower=0, zc_randgen* rng=NULL);
void zc_srand(int32_t seedval, zc_randgen* rng=NULL);
#else

#define zc_oldrand(...) db_oldrand(__FILE__,__LINE__,__VA_ARGS__)
int32_t db_oldrand(char const* f, int l,zc_randgen* rng=NULL);
#define zc_rand(...) db_rand(__FILE__,__LINE__,__VA_ARGS__)
int32_t db_rand(char const* f, int l,zc_randgen* rng=NULL);
int32_t db_rand(char const* f, int l,int32_t upper,int32_t lower=0,zc_randgen* rng=NULL);
#define zc_srand(...) db_srand(__FILE__,__LINE__,__VA_ARGS__)
void db_srand(char const* f, int l, int32_t seedval, zc_randgen* rng=NULL);

#endif

#endif

