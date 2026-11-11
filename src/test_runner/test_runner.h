#pragma once

struct TestResults
{
	int failed;
	int total;
};

bool run_tests(TestResults fn(bool verbose), const char* name, bool verbose);
