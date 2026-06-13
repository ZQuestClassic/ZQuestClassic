// Tests for the deterministic trig in zc_math.cpp.
//
// Two properties matter and are asserted separately:
//
//  1. Accuracy: within 1e-6 of std::sin/cos for every angle a script can pass,
//     so the deterministic path is indistinguishable from the platform libm at
//     the script layer (which truncates to 1e-4).
//  2. Determinism: the exact output bits never change. The golden hash below
//     was computed once; CI runs this on every platform/compiler, so any
//     bit-level divergence (new compiler, new flags, new platform) fails here
//     instead of as a replay desync.

#include "base/zc_math.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"

#include <fmt/format.h>

#include <cmath>
#include <cstdint>
#include <cstring>

// FNV-1a of the output bits of Sin/Cos/Tan over every script-representable
// angle. See test_determinism for what to do if this fails.
#define GOLDEN_TRIG_HASH 7783015035862141493ULL
// FNV-1a of the output bits of ArcSin/ArcCos over every script-representable
// input, and ArcTan2 over a fixed grid. Same rules as GOLDEN_TRIG_HASH.
#define GOLDEN_ARC_HASH 6163329520349183757ULL
// FNV-1a of the output bits of Ln/Log10/Pow over fixed grids. Same rules.
#define GOLDEN_LOG_HASH 12873033877482162720ULL

// Every angle reachable from ZScript: degrees in fixed point steps of 0.0001,
// converted to radians exactly as do_trig() does.
static double script_degrees_to_radians(int32_t deg_fixed)
{
	return (deg_fixed / 10000.0) * PI / 180.0;
}

static void fnv1a_mix(uint64_t& h, double v)
{
	uint64_t bits;
	memcpy(&bits, &v, sizeof(bits));
	for (int i = 0; i < 8; i++)
	{
		h ^= (bits >> (i * 8)) & 0xFF;
		h *= 1099511628211ULL;
	}
}

static void test_critical_values()
{
	// These must be exact, not just close: scripts rely on Sin(90) == 1
	// surviving the conversion to fixed point without truncating to 0.9999.
	assertEqual(zc::math::Sin(script_degrees_to_radians(0)), 0.0);
	assertEqual(zc::math::Sin(script_degrees_to_radians(900000)), 1.0);
	assertEqual(zc::math::Sin(script_degrees_to_radians(1800000)), 0.0);
	assertEqual(zc::math::Sin(script_degrees_to_radians(2700000)), -1.0);
	assertEqual(zc::math::Cos(script_degrees_to_radians(0)), 1.0);
	assertEqual(zc::math::Cos(script_degrees_to_radians(900000)), 0.0);
	assertEqual(zc::math::Cos(script_degrees_to_radians(1800000)), -1.0);
	assertEqual(zc::math::Cos(script_degrees_to_radians(2700000)), 0.0);
	assertEqual(zc::math::Tan(script_degrees_to_radians(0)), 0.0);
	assertEqual(zc::math::Tan(script_degrees_to_radians(450000)), 1.0);

	assertEqual(zc::math::ArcSin(0.0), 0.0);
	assertEqual(zc::math::ArcSin(1.0), PI/2);
	assertEqual(zc::math::ArcSin(-1.0), -(PI/2));
	assertEqual(zc::math::ArcCos(1.0), 0.0);
	assertEqual(zc::math::ArcCos(0.0), PI/2);
	assertEqual(zc::math::ArcCos(-1.0), PI);
	assertEqual(zc::math::ArcTan2(0.0, 1.0), 0.0);
	assertEqual(zc::math::ArcTan2(1.0, 0.0), PI/2);
	assertEqual(zc::math::ArcTan2(-1.0, 0.0), -(PI/2));
	assertEqual(zc::math::ArcTan2(0.0, -1.0), PI);

	assertEqual(zc::math::Ln(1.0), 0.0);
	assertEqual(zc::math::Log10(1.0), 0.0);
	// Digit-counting scripts depend on exact powers of ten.
	assertEqual(zc::math::Log10(10.0), 1.0);
	assertEqual(zc::math::Log10(100.0), 2.0);
	assertEqual(zc::math::Log10(10000.0), 4.0);
	assertEqual(zc::math::Log10(0.1), -1.0);
	assertEqual(zc::math::Log10(0.001), -3.0);
	// Integer exponents go through exponentiation by squaring: exact for the
	// powers of 2 and 10 scripts rely on.
	assertEqual(zc::math::Pow(2.0, 10.0), 1024.0);
	assertEqual(zc::math::Pow(2.0, 30.0), 1073741824.0);
	assertEqual(zc::math::Pow(10.0, 3.0), 1000.0);
	assertEqual(zc::math::Pow(10.0, 6.0), 1000000.0);
	assertEqual(zc::math::Pow(3.0, 4.0), 81.0);
	assertEqual(zc::math::Pow(-3.0, 3.0), -27.0);
	assertEqual(zc::math::Pow(2.0, -2.0), 0.25);
	assertEqual(zc::math::Pow(7.0, 1.0), 7.0);
	assertEqual(zc::math::Pow(123.0, 0.0), 1.0);
	assertEqual(zc::math::Pow(1.0, 0.5), 1.0);
}

static void test_maths_mode_dispatch()
{
	using namespace zc::math;

	// Restore the default mode no matter how this test exits, so the golden
	// hash tests that follow always run in New.
	struct Restore { ~Restore() { set_maths_mode(MathsMode::New); } } restore;

	// New (the default) is the deterministic table path.
	set_maths_mode(MathsMode::New);
	double new_sin = Sin(PI/180);

	// The legacy Q15 modes (pre-version-59 replay playback) must still dispatch
	// after the LegacyStd mode was removed - they are what old replays play back
	// with. They are deliberately not golden-hashed, so assert only their
	// defining behavior: a coarse value near 1 at 90 degrees, and the per-tier
	// decimal rounding. Each mode produces a distinct result, proving the
	// dispatch routes to four separate paths.
	set_maths_mode(MathsMode::LegacyReplayV0);
	double v0_one = Sin(PI/2);
	assertTrue(v0_one > 0.99 && v0_one <= 1.0);
	double v0 = Sin(PI/180);

	set_maths_mode(MathsMode::LegacyReplayV4);
	double v4 = Sin(PI/180);
	assertEqual(v4, std::round(v4 * 10000.0) / 10000.0);

	set_maths_mode(MathsMode::LegacyReplayV21);
	double v21 = Sin(PI/180);
	assertEqual(v21, std::round(v21 * 1000.0) / 1000.0);

	assertTrue(new_sin != v0);
	assertTrue(v0 != v4);
	assertTrue(v4 != v21);
}

static void test_accuracy()
{
	double worst_sin = 0, worst_cos = 0;
	for (int32_t deg = -3600000; deg < 3600000; deg++)
	{
		double rad = script_degrees_to_radians(deg);
		worst_sin = std::max(worst_sin, std::abs(zc::math::Sin(rad) - std::sin(rad)));
		worst_cos = std::max(worst_cos, std::abs(zc::math::Cos(rad) - std::cos(rad)));
	}
	assertTrue(worst_sin < 1e-6);
	assertTrue(worst_cos < 1e-6);
}

static void test_accuracy_arc()
{
	// Every script-representable ArcSin/ArcCos input: [-1, 1] in 1e-4 steps.
	double worst_asin = 0, worst_acos = 0;
	for (int32_t i = -10000; i <= 10000; i++)
	{
		double x = i / 10000.0;
		worst_asin = std::max(worst_asin, std::abs(zc::math::ArcSin(x) - std::asin(x)));
		worst_acos = std::max(worst_acos, std::abs(zc::math::ArcCos(x) - std::acos(x)));
	}
	assertTrue(worst_asin < 1e-6);
	assertTrue(worst_acos < 1e-6);

	// ArcTan2 over a coordinate grid (script grain is far coarser than this).
	double worst_atan2 = 0;
	for (int32_t yi = -800; yi <= 800; yi++)
	for (int32_t xi = -800; xi <= 800; xi++)
	{
		if (xi == 0 && yi == 0)
			continue;
		double y = yi / 3.0, x = xi / 3.0;
		worst_atan2 = std::max(worst_atan2, std::abs(zc::math::ArcTan2(y, x) - std::atan2(y, x)));
	}
	assertTrue(worst_atan2 < 1e-6);
}

static void test_accuracy_log_pow()
{
	// Ln/Log10 over script-representable inputs, dense for small x plus a
	// coarse sweep to the top of the script range (214748.3647).
	double worst_ln = 0, worst_log10 = 0;
	for (int32_t i = 1; i <= 2000000; i++)
	{
		double x = i / 10000.0;
		worst_ln = std::max(worst_ln, std::abs(zc::math::Ln(x) - std::log(x)));
		worst_log10 = std::max(worst_log10, std::abs(zc::math::Log10(x) - std::log10(x)));
	}
	for (int32_t i = 1; i <= 21474; i++)
	{
		double x = i * 10.0001;
		worst_ln = std::max(worst_ln, std::abs(zc::math::Ln(x) - std::log(x)));
		worst_log10 = std::max(worst_log10, std::abs(zc::math::Log10(x) - std::log10(x)));
	}
	assertTrue(worst_ln < 1e-6);
	assertTrue(worst_log10 < 1e-6);

	// Pow error scales with the exponent (it multiplies log2's error), so the
	// bound is relative and exponent-scaled.
	double bases[] = { 0.0625, 0.1234, 0.5, 1.5, 2.7183, 10.0, 123.4567, 9999.9999 };
	double worst_pow_scaled = 0;
	for (double a : bases)
	for (int32_t bi = -60000; bi <= 60000; bi += 7)
	{
		double b = bi / 10000.0;
		double got = zc::math::Pow(a, b);
		double want = std::pow(a, b);
		double rel = std::abs(got - want) / want;
		worst_pow_scaled = std::max(worst_pow_scaled, rel / (std::abs(b) + 2.0));
	}
	assertTrue(worst_pow_scaled < 1e-6);
}

static void test_determinism()
{
	uint64_t h = 14695981039346656037ULL;
	for (int32_t deg = -3600000; deg < 3600000; deg++)
	{
		double rad = script_degrees_to_radians(deg);
		fnv1a_mix(h, zc::math::Sin(rad));
		fnv1a_mix(h, zc::math::Cos(rad));
		fnv1a_mix(h, zc::math::Tan(rad));
	}
	// If this fails and you did not change the trig implementation, the compiler or platform is
	// producing different bits and replays recorded elsewhere would desync - do not just update the
	// constant.
	assertEqual(h, (uint64_t)GOLDEN_TRIG_HASH);
}

static void test_determinism_arc()
{
	uint64_t h = 14695981039346656037ULL;
	for (int32_t i = -10000; i <= 10000; i++)
	{
		double x = i / 10000.0;
		fnv1a_mix(h, zc::math::ArcSin(x));
		fnv1a_mix(h, zc::math::ArcCos(x));
	}
	for (int32_t yi = -800; yi <= 800; yi++)
	for (int32_t xi = -800; xi <= 800; xi++)
	{
		fnv1a_mix(h, zc::math::ArcTan2(yi / 3.0, xi / 3.0));
	}
	// Same rules as test_determinism: do not just update the constant.
	assertEqual(h, (uint64_t)GOLDEN_ARC_HASH);
}

static void test_determinism_log_pow()
{
	uint64_t h = 14695981039346656037ULL;
	for (int32_t i = 1; i <= 2000000; i += 3)
	{
		double x = i / 10000.0;
		fnv1a_mix(h, zc::math::Ln(x));
		fnv1a_mix(h, zc::math::Log10(x));
	}
	double bases[] = { 0.0625, 0.1234, 0.5, 1.5, 2.7183, 10.0, 123.4567, 9999.9999 };
	for (double a : bases)
	for (int32_t bi = -60000; bi <= 60000; bi += 7)
	{
		fnv1a_mix(h, zc::math::Pow(a, bi / 10000.0));
	}
	// Same rules as test_determinism: do not just update the constant.
	assertEqual(h, (uint64_t)GOLDEN_LOG_HASH);
}

// The legacy (pre-replay-version-59) Q15 trig is deliberately not golden-hashed.
// Unlike the new tables, it was never bit-identical across compilers: it uses
// plain floating-point divisions that clang keeps as divisions but gcc and MSVC
// rewrite as reciprocal multiplies under fast-math, so each toolchain produced
// different bits historically. It only runs when playing back pre-59 replays,
// which were recorded per-platform, so there is no single correct value to
// assert against.

TestResults test_zc_math(bool verbose)
{
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "critical_values", test_critical_values },
		{ "maths_mode_dispatch", test_maths_mode_dispatch },
		{ "accuracy_vs_std", test_accuracy },
		{ "accuracy_arc_vs_std", test_accuracy_arc },
		{ "accuracy_log_pow_vs_std", test_accuracy_log_pow },
		{ "determinism_golden_hash", test_determinism },
		{ "determinism_arc_golden_hash", test_determinism_arc },
		{ "determinism_log_pow_golden_hash", test_determinism_log_pow },
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
