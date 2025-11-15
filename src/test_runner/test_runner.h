#ifndef TEST_RUNNER_H_
#define TEST_RUNNER_H_

struct TestResults
{
	int failed;
	int total;
};

bool run_tests(TestResults fn(bool verbose), const char* name, bool verbose);

#endif
