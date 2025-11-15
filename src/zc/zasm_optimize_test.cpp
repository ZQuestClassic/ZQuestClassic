#include "test_runner/test_runner.h"
#include "zc/zasm_optimize.h"

TestResults test_zasm_optimize(bool verbose)
{
	TestResults tr{};

	tr.total++;
	// TODO: convert to test runner.
	if (!zasm_optimize_test())
		tr.failed++;

	return tr;
}
