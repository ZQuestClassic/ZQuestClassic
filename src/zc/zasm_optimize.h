#ifndef ZASM_OPTIMIZE_H_
#define ZASM_OPTIMIZE_H_

#include "core/zdefs.h"

#include <string>

struct OptimizationPass
{
	std::string name;
	uint32_t instructions_saved;
	// microseconds
	uint32_t elapsed;
	bool skipped;
	// For passes that rewrite instructions in place to make them faster (rather than
	// removing them): how many instances were improved. Reported instead of "instr saved".
	uint32_t instances;
	bool counts_instances;
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
void zasm_optimize_run_for_file(std::string path);

#endif
