// This runs just the tests defined in cpp files.
// For all other tests, see the `tests` folder.

#include "test_runner/test_runner.h"
#include <fmt/format.h>

#if defined(_WIN32)
// Windows doesn't support ANSI codes in cmd.exe by default
#define RESET ""
#define RED ""
#define GREEN ""
#define YELLOW ""
#define BLUE ""
#else
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#endif

bool run_tests(TestResults fn(bool verbose), const char* name, bool verbose)
{
	fmt::println("========================================");
	fmt::println("                {}", name);
	fmt::println("========================================");

	auto tr = fn(verbose);
	if (tr.failed)
	{
		fmt::println("[{}{}{}] {} / {} tests failed.", RED, name, RESET, tr.failed, tr.total);
		return false;
	}
	else
	{
		fmt::println("[{}{}{}] All {} tests passed.", GREEN, name, RESET, tr.total);
		return true;
	}
}
