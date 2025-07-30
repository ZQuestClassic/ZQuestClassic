#ifndef ZASM_OPTIMIZE_H_
#define ZASM_OPTIMIZE_H_

#include "base/zdefs.h"

#include <string>

struct OptimizationPass
{
	std::string name;
	uint32_t instructions_saved;
	// microseconds
	uint32_t elapsed;
	bool skipped;
};

struct OptimizeResults
{
	uint32_t instructions_saved;
	// microseconds
	uint32_t elapsed;
	std::vector<OptimizationPass> passes;
};

void zasm_optimize();
bool zasm_optimize_test();
void zasm_optimize_run_for_file(std::string path);

#endif
