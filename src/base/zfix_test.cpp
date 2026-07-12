// Tests for zfix, focused on the INT32_MIN boundary.
//
// Negating or abs-ing INT32_MIN is signed overflow, so zfix::operator-() and
// zfix::doAbs() perform unsigned negation, which wraps INT32_MIN to itself.
// Before that, gcc used the UB to conclude a negation result could never be
// negative and folded the sign out of str() for MIN_SCC_ARG-like values,
// printing "214748.3648" instead of "-214748.3648" (only on gcc - clang
// happened to wrap). These tests pin the boundary behavior on every
// platform/compiler CI runs.

#include "base/zfix.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"

#include <fmt/format.h>

#include <cstdint>
#include <string>

static void test_two_arg_ctor()
{
	assertEqual(zfix(1, 5000).getZLong(), 15000);
	assertEqual(zfix(-1, -5000).getZLong(), -15000);
	assertEqual(zfix(0, -5000).getZLong(), -5000);
	// The extremes are exactly representable from components.
	assertEqual(zfix(214748, 3647).getZLong(), INT32_MAX);
	assertEqual(zfix(-214748, -3648).getZLong(), INT32_MIN);
}

static void test_literals_at_bounds()
{
	// There are no negative literals in C++: this is 214748.3648_zf (which
	// wraps to INT32_MIN) negated by zfix::operator-().
	assertEqual((-214748.3648_zf).getZLong(), INT32_MIN);
	assertEqual((214748.3647_zf).getZLong(), INT32_MAX);
}

static void test_negation()
{
	assertEqual((-(1.5_zf)).getZLong(), -15000);
	assertEqual((-(-1.5_zf)).getZLong(), 15000);
	assertEqual((-(0_zf)).getZLong(), 0);
	// Negating INT32_MIN wraps to itself (defined via unsigned negation).
	zfix min_zf = zfix(-214748, -3648);
	assertEqual((-min_zf).getZLong(), INT32_MIN);
	assertEqual((-(-min_zf)).getZLong(), INT32_MIN);
}

static void test_abs()
{
	assertEqual((-1.5_zf).getAbs().getZLong(), 15000);
	assertEqual((1.5_zf).getAbs().getZLong(), 15000);
	// abs of INT32_MIN wraps to itself and so remains negative - the same
	// result abs() produced before, but no longer UB.
	assertEqual(zfix(-214748, -3648).getAbs().getZLong(), INT32_MIN);
}

static void test_atozfix()
{
	assertEqual(atozfix("1.5").getZLong(), 15000);
	assertEqual(atozfix("-1.5").getZLong(), -15000);
	assertEqual(atozfix("-0.5").getZLong(), -5000);
	assertEqual(atozfix("0").getZLong(), 0);
	// The extremes parse exactly (the negative one must be built from
	// negative components internally, not negated after the fact).
	assertEqual(atozfix("214748.3647").getZLong(), INT32_MAX);
	assertEqual(atozfix("-214748.3648").getZLong(), INT32_MIN);
}

static void test_str()
{
	assertEqual((1.5_zf).str(), std::string("1.5000"));
	assertEqual((-1.5_zf).str(), std::string("-1.5000"));
	assertEqual((1.5_zf).str_trim(), std::string("1.5"));
	assertEqual((26_zf).str_trim(), std::string("26"));
	// The exact strings from the gcc regression: the sign must survive.
	assertEqual((-214748.3648_zf).str(), std::string("-214748.3648"));
	assertEqual((214748.3647_zf).str(), std::string("214748.3647"));
	assertEqual(zfix(-214748, -3648).str(), std::string("-214748.3648"));
}

TestResults test_zfix(bool verbose)
{
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "two_arg_ctor", test_two_arg_ctor },
		{ "literals_at_bounds", test_literals_at_bounds },
		{ "negation", test_negation },
		{ "abs", test_abs },
		{ "atozfix", test_atozfix },
		{ "str", test_str },
	};

	int failed = 0, total = 0;
	for (auto& test : tests)
	{
		total++;
		try
		{
			test.fn();
			if (verbose)
				fmt::println("  [PASS] {}", test.name);
		}
		catch (const std::exception& e)
		{
			failed++;
			fmt::println("  [FAIL] {}\n{}", test.name, e.what());
		}
	}

	return TestResults{failed, total};
}
