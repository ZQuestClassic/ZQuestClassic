
#pragma once

// todo: this needs to go in a zc_config.h file but the 2.50 branch doesn't have one yet.
#ifdef _MSC_VER
	#define ALLEGRO_MSVC
#endif

#if defined(ZC_PCH)

//globally remove extraneous bullshit
//


//c standard lib
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


//c++ stl (these are already found in common headers)
#include <string>
#include <map>
#include <vector>
#include <set>

//allegro
#include "zc_alleg.h"

//zc
#include "zc_malloc.h"



#endif

