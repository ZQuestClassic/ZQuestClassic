#include <chrono>
#include "base/ints.h"
#include "base/headers.h"
#include "base/containers.h"

#define TIMEIT_SETUP() \
    static vector<uint> __timeit_store; \
    auto __timeit_start = std::chrono::steady_clock::now()
#define TIMEIT_ENDCLOCK(ty) \
    uint __timeit_elapsed = std::chrono::duration_cast<ty>(std::chrono::steady_clock::now() - __timeit_start).count(); \
    __timeit_store.push_back(__timeit_elapsed); \
    uint __timeit_total = 0, __timeit_avg; \
    do { \
        for(auto v : __timeit_store) \
            __timeit_total += v; \
        __timeit_avg = (__timeit_total / __timeit_store.size()); \
    } while(false)
#define TIMEIT_RESET_COUNTS() \
    __timeit_store.clear()
#define TIMEIT_AVG __timeit_avg
#define TIMEIT_TOTAL __timeit_total
#define TIMEIT_COUNT __timeit_store.size()
#define TIMEIT_ELAPSED __timeit_elapsed

/* Example use:
 * TIMEIT_SETUP();
 * //Insert something to time here
 * TIMEIT_ENDCLOCK(std::chrono::milliseconds);
 * zprint2("%dms (%d avg, %d total, %d count)\n", TIMEIT_ELAPSED, TIMEIT_AVG, TIMEIT_TOTAL, TIMEIT_COUNT);
 * 
 * If this section is run repeatedly, the average/total/runcount are accumulated via static variable.
 * 'TIMEIT_RESET_COUNTS();' will reset the static accumulator
 */
