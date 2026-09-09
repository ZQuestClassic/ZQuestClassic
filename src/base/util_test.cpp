#include "base/util.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"

#include <string>

static void test_fuzzy_match_score()
{
	auto score = util::fuzzy_match_score;

	// Every pattern character must appear, in order, ignoring case.
	assertTrue(score("lttz", "link_to_the_zelda.qst").has_value());
	assertTrue(score("LTTZ", "link_to_the_zelda.qst").has_value());
	assertTrue(!score("zttl", "link_to_the_zelda.qst").has_value());
	assertTrue(!score("x", "link_to_the_zelda.qst").has_value());
	assertTrue(!score("a", "").has_value());

	// An empty pattern matches everything.
	assertEqual(*score("", "anything"), 0);
	assertEqual(*score("", ""), 0);

	// Closer matches score higher: adjacent letters beat scattered ones,
	// and a shorter string beats a longer one.
	assertTrue(*score("zelda", "zelda.qst") > *score("zelda", "z_e_l_d_a.qst"));
	assertTrue(*score("zelda", "zelda.qst") > *score("zelda", "the legend of zelda.qst"));
	assertTrue(*score("zelda", "zelda.qst") > *score("zelda", "zelda_remastered.qst"));

	// Matching the start of a word after a separator beats the middle of one.
	assertTrue(*score("tz", "the_zelda.qst") > *score("tz", "thezelda.qst"));

	// Many candidate positions per character must not blow up.
	std::string many_a(300, 'a');
	assertTrue(score(std::string(40, 'a'), many_a).has_value());
	assertTrue(!score(std::string(40, 'a') + "b", many_a).has_value());
}

TestResults test_util(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "fuzzy_match_score", test_fuzzy_match_score },
	};

	for (auto& test : tests)
	{
		++tr.total;
		try
		{
			test.fn();
			if (verbose)
				fmt::println("  [PASS] {}", test.name);
		}
		catch (const std::exception& e)
		{
			++tr.failed;
			fmt::println("  [FAIL] {}\n{}", test.name, e.what());
		}
	}

	return tr;
}
