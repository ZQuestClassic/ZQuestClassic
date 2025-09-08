#ifndef ZASM_OPTIMIZE_H_
#define ZASM_OPTIMIZE_H_

#include "base/zdefs.h"
#include "zc/zasm_utils.h"

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
	uint32_t instructions_processed;
	// microseconds
	uint32_t elapsed;
	std::vector<OptimizationPass> passes;
};

bool zasm_optimize_is_enabled();
void zasm_optimize_sync();
void zasm_optimize();
OptimizeResults zasm_optimize_script(zasm_script* script);
bool zasm_optimize_test();
void zasm_optimize_run_for_file(std::string path);

#endif
