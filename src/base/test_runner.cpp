// This runs just the tests defined in cpp files.
// For all other tests, see the `tests` folder.

#include "base/zc_alleg.h"
#include <allegro5/allegro.h>

#include <cstdint>

void test_scc();

int32_t main(int32_t argc, char* argv[])
{
	test_scc();
	return 0;
}
END_OF_MAIN()
