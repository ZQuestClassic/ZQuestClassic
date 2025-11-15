// This runs just the tests defined in cpp files.
// For all other tests, see the `tests` folder.

#include "base/zc_alleg.h"
#include "test_runner/test_runner.h"
#include <allegro5/allegro.h>

#include <cstdint>

int32_t main(int32_t argc, char* argv[])
{
	bool success = false;
	bool verbose = argc >= 2 && (strcmp(argv[1], "-verbose") == 0 || strcmp(argv[1], "-v") == 0);

	extern TestResults test_scc(bool);

	run_tests(test_scc, "test_scc", verbose);

	return success ? 1 : 0;
}
END_OF_MAIN()
