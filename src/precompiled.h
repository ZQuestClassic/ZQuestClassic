#ifndef PRECOMPILED_H
#define PRECOMPILED_H
#pragma once

#ifdef ZC_PCH

//globally remove extraneous bullshit
//

#define VLD_FORCE_ENABLE 0

// Visual Leak Detector - Alternative Mem Debug
// NB: This should always be set to 0 in mainline
// NB: To use, PCH, Windows, and installing VLD are required
//     Found here: https://kinddragon.github.io/vld/
#if (VLD_FORCE_ENABLE == 1)
#include <vld.h>
#endif // (VLD_FORCE_ENABLE == 1)

//Boost
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/move/make_unique.hpp>

//c standard lib
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

//c++ stl (these are already found in common headers)
#include <map>
#include <set>
#include <string>
#include <vector>

//allegro
#include "zc_alleg.h"

//zc
#include "zc_malloc.h"
#include "mem_debug.h"

#endif // ZC_PCH
#endif // !PRECOMPILED_H
