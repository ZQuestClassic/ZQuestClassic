#include "base/zc_math.h"
#include "base/sin1.h"
#include "base/zc_math_tables.h"
#include <fmt/format.h>
#include <cstdint>
#include <math.h>

// Math that produces bit-identical results on every platform and compiler,
// accurate enough (error < 1e-6) that swapping it for the libm equivalents is
// invisible at the script layer, where values truncate to 1e-4. This is the
// only trig used for gameplay since replay version 59 - whether or not a
// replay is active - so recording a replay can never change behavior.
//
// Angles are reduced to integer "turns" (TURN_BITS per full circle), then a
// Q30 quarter-wave table is interpolated with int64 math. The only floating
// point operations are the fmod/multiply/round on input (each individually
// exact or correctly rounded per IEEE 754, so deterministic even under
// -ffp-model=fast) and the final scale by a power of two (exact).
//
// Determinism invariant: every multiply that sits next to an add must be a
// power-of-two scale (exact), so FMA contraction under -ffp-model=fast cannot
// change results. Keep that property when editing.
//
// History: before version 59 (and only while a replay was active), trig used a
// 32-entry Q15 table with results rounded to 3 decimals. That error (~1e-3)
// was amplified by scripts that derive collision axes from differences of
// nearby trig values, producing hitboxes tens of pixels off.
// See https://discord.com/channels/876899628556091432/1509803675605008384
#define TURN_BITS 24
#define TURN (1 << TURN_BITS)
#define QUARTER_TURN (TURN / 4)

namespace
{

// Whether the deterministic tables below are used, vs a pre-version-59
// legacy mode. Defaults to New so binaries with no replay system (zeditor,
// base_test_runner) get the deterministic math without any setup; zplayer
// keeps it current via replay_compat_setup_zc_maths() on every replay state
// transition.
zc::math::MathsMode maths_mode = zc::math::MathsMode::New;

int64_t radians_to_turns(double x)
{
	// fmod is exact, so this bounds the multiply below without losing
	// precision or determinism.
	x = std::fmod(x, 2*PI);
	// Round (don't truncate) so critical angles survive the round trip
	// through radians: 90 degrees arrives as a double a hair off PI/2, but
	// still lands on exactly QUARTER_TURN, making Sin(DegtoRad(90)) == 1.0.
	return (int64_t)llround(x * (TURN / (2*PI)));
}

double sin_turns(int64_t turns)
{
	uint32_t u = (uint32_t)turns & (TURN - 1);
	uint32_t quad = u >> (TURN_BITS - 2);
	uint32_t r = u & (QUARTER_TURN - 1);
	if (quad & 1) r = QUARTER_TURN - r;
	// 1024 table entries per quarter turn leaves 12 bits for interpolation.
	uint32_t index = r >> 12;
	uint32_t frac = r & 0xFFF;
	int64_t v = (int64_t)SIN_TABLE_Q30[index] * (0x1000 - frac)
	          + (int64_t)SIN_TABLE_Q30[index + 1] * frac;
	// v <= 2^42, so both the int64 -> double conversion and the power-of-two
	// scale are exact.
	double s = v * (1.0 / (double)(1LL << 42));
	return quad & 2 ? -s : s;
}

// The inverse-trig lookups follow the same recipe as sin_turns: quantize the
// input with one multiply + llround, interpolate a Q30 table in int64, scale
// by a power of two. The handful of double operations in their callers' range
// reductions are each individually exact or correctly rounded, so results are
// bit-identical everywhere.

// asin over [0, 0.5]. Larger inputs are reduced into this range by the
// callers, dodging asin's vertical tangent at 1.
double asin_lookup(double x)
{
	int64_t q = llround(x * (double)(1 << 24)); // [0, 2^23]
	uint32_t index = (uint32_t)(q >> 13);
	uint32_t frac = (uint32_t)(q & 0x1FFF);
	int64_t v = (int64_t)ASIN_TABLE_Q30[index] * (0x2000 - frac)
	          + (int64_t)ASIN_TABLE_Q30[index + 1] * frac;
	// v <= asin(0.5) * 2^43 < 2^53, so conversion and scale are exact.
	return v * (1.0 / (double)(1LL << 43));
}

// atan over [0, 1]. atan2 reduces every octant into this range.
double atan_lookup(double t)
{
	int64_t q = llround(t * (double)(1 << 24)); // [0, 2^24]
	uint32_t index = (uint32_t)(q >> 14);
	uint32_t frac = (uint32_t)(q & 0x3FFF);
	int64_t v = (int64_t)ATAN_TABLE_Q30[index] * (0x4000 - frac)
	          + (int64_t)ATAN_TABLE_Q30[index + 1] * frac;
	// v <= (pi/4) * 2^44 < 2^53, so conversion and scale are exact.
	return v * (1.0 / (double)(1LL << 44));
}

// The logarithms decompose x = 2^(e-1) * (1 + m1) with m1 in [0, 1): frexp is
// exact, doubling its mantissa is a power-of-two scale, and subtracting 1 is
// exact by Sterbenz. The mantissa's log comes from a Q30 table and the
// exponent contributes (e-1) * log(2) via the matching Q44 integer constant,
// so the two parts combine in int64 with no float add to contract.

// log(1 + m1) in the table's base, as Q44. m1 in [0, 1).
int64_t log_mantissa_q44(const int32_t* table, double m1)
{
	int64_t q = llround(m1 * (double)(1 << 24)); // [0, 2^24]
	uint32_t index = (uint32_t)(q >> 14);
	uint32_t frac = (uint32_t)(q & 0x3FFF);
	return (int64_t)table[index] * (0x4000 - frac)
	     + (int64_t)table[index + 1] * frac;
}

// 2^f for f in [0, 1). The table stores 2^f - 1 so it fits int32; the 1 is
// added back as an exact Q44 integer.
double exp2_frac(double f)
{
	int64_t q = llround(f * (double)(1 << 24)); // [0, 2^24]
	uint32_t index = (uint32_t)(q >> 14);
	uint32_t frac = (uint32_t)(q & 0x3FFF);
	int64_t v = (int64_t)EXP2M1_TABLE_Q30[index] * (0x4000 - frac)
	          + (int64_t)EXP2M1_TABLE_Q30[index + 1] * frac;
	// v + 2^44 <= 2^45 < 2^53: conversion and scale are exact.
	return (double)(v + (1LL << 44)) * (1.0 / (double)(1LL << 44));
}

// Exponentiation by squaring: fixed operation order, each multiply correctly
// rounded, so deterministic - and exact where the products are representable,
// which covers the Pow(2, n) and Pow(10, n) that scripts depend on.
double pow_integer(double base, double exponent)
{
	uint64_t n = (uint64_t)std::fabs(exponent);
	double r = 1.0;
	while (n)
	{
		if (n & 1) r *= base;
		base *= base;
		n >>= 1;
	}
	return exponent < 0 ? 1.0 / r : r;
}

} // namespace

namespace zc
{


namespace math
{


bool IsPowerOfTwo(uint32_t x)
{
    return (x != 0 && !((x-1) & x));
}

uint32_t NextPowerOfTwo(uint32_t x)
{
    uint32_t y(1);
    
    while(y < x) y <<= 1;
    
    return y;
}

int32_t SafeMod(int32_t x, int32_t y)
{
	if (y == 0)
		return 0;

	return x % y;
}

float ToDegrees(float radians)
{
    return (radians * 57.29578f);
}

float ToRadians(float degrees)
{
    return (degrees * 0.0174533f);
}

float Round(float x)
{
    return floorf(x + 0.5f);
}

float Lerp(float a, float b, float t)
{
    return (a + (b - a) * t);
}

#define Q15 (1.0/(double)((1<<15)-1))

void set_maths_mode(MathsMode mode)
{
	maths_mode = mode;
}

double Sin(double x)
{
	if (maths_mode != MathsMode::New)
	{
		// x needs to be converted from radians -> angles -> sin1 domain
		x = x * (180/PI * 32768.0/360.0);
		x = (long)x % 0x8000;
		double r = sin1((int16_t)x) * Q15;
		// round to fewer decimal places, otherwise some "critical" values
		// will be slightly off (ex: Cos(0) == 0.99996, instead of 1)
		// initially did not reduce precision enough, which meant cos(PI/180) == 0.0002 instead of 0
		if (maths_mode == MathsMode::LegacyReplayV21)
			return std::round(r * 1000.0) / 1000.0;
		if (maths_mode == MathsMode::LegacyReplayV4)
			return std::round(r * 10000.0) / 10000.0;
		return r;
	}

	return sin_turns(radians_to_turns(x));
}

double SinD(int16_t x)
{
	return Sin(x * PI/180);
}

double Cos(double x)
{
	if (maths_mode != MathsMode::New)
	{
		x = x * (180/PI * 32768.0/360.0);
		x = (long)x % 0x8000;
		double r = cos1((int16_t)x) * Q15;
		if (maths_mode == MathsMode::LegacyReplayV21)
			return std::round(r * 1000.0) / 1000.0;
		if (maths_mode == MathsMode::LegacyReplayV4)
			return std::round(r * 10000.0) / 10000.0;
		return r;
	}

	// The quarter turn phase shift is an exact integer, unlike the legacy
	// cos1(), whose Q15 shift was off by one angle unit.
	return sin_turns(radians_to_turns(x) + QUARTER_TURN);
}

double CosD(int16_t x)
{
	return Cos(x * PI/180);
}

double Tan(double x)
{
	if (maths_mode != MathsMode::New)
	{
		x = x * (180/PI * 32768.0/360.0);
		x = (long)x % 0x8000;
		double r = (sin1((int16_t)x) * Q15) / (cos1((int16_t)x) * Q15);
		if (maths_mode == MathsMode::LegacyReplayV21)
			return std::round(r * 1000.0) / 1000.0;
		if (maths_mode == MathsMode::LegacyReplayV4)
			return std::round(r * 10000.0) / 10000.0;
		return r;
	}

	int64_t turns = radians_to_turns(x);
	double s = sin_turns(turns);
	double c = sin_turns(turns + QUARTER_TURN);
	// Unlike std::tan, the table hits the asymptotes dead-on; return a large
	// finite value comparable to std::tan just shy of them, rather than inf,
	// which scripts would truncate to int32 (UB).
	if (c == 0)
		return s > 0 ? 1e16 : -1e16;
	// IEEE division is correctly rounded, so this stays deterministic.
	return s / c;
}

double ArcSin(double x)
{
	if (maths_mode != MathsMode::New)
		return std::asin(x);

	double ax = std::fabs(x);
	double r;
	if (ax <= 0.5)
		r = asin_lookup(ax);
	else if (ax <= 1.0)
		// 1 - ax is exact (Sterbenz), * 0.5 is exact, sqrt is correctly
		// rounded: the lookup sees a deterministic argument in [0, 0.5].
		r = PI/2 - 2.0 * asin_lookup(std::sqrt((1.0 - ax) * 0.5));
	else
		return std::asin(x); // out of domain: NaN, same as before
	return x < 0 ? -r : r;
}

double ArcCos(double x)
{
	if (maths_mode != MathsMode::New)
		return std::acos(x);

	// Not written as PI/2 - ArcSin(x): for x near 1 that nests two
	// cancelling PI/2 terms, which unsafe-math reassociation may fold.
	// These branches keep every step a single rounding.
	if (x > 0.5 && x <= 1.0)
		return 2.0 * asin_lookup(std::sqrt((1.0 - x) * 0.5));
	if (x < -0.5 && x >= -1.0)
		return PI - 2.0 * asin_lookup(std::sqrt((1.0 + x) * 0.5));
	if (x >= -0.5 && x <= 0.5)
		return PI/2 - asin_lookup(std::fabs(x)) * (x < 0 ? -1.0 : 1.0);
	return std::acos(x); // out of domain: NaN, same as before
}

double ArcTan2(double y, double x)
{
	if (maths_mode != MathsMode::New)
		return std::atan2(y, x);

	if (y == 0)
		return x < 0 ? PI : 0.0;
	if (x == 0)
		return y < 0 ? -(PI/2) : PI/2;

	double ay = std::fabs(y), ax = std::fabs(x);
	// min/max division keeps the lookup argument in [0, 1]; one correctly
	// rounded IEEE division, so still deterministic.
	double r;
	if (ay <= ax)
		r = atan_lookup(ay / ax);
	else
		r = PI/2 - atan_lookup(ax / ay);
	if (x < 0)
		r = PI - r;
	return y < 0 ? -r : r;
}

double Ln(double x)
{
	if (maths_mode != MathsMode::New)
		return std::log(x);
	if (!(x > 0) || !std::isfinite(x))
		return std::log(x); // 0 -> -inf, negative -> NaN, inf -> inf: unchanged

	int e;
	double m1 = std::frexp(x, &e) * 2.0 - 1.0;
	int64_t v = (int64_t)(e - 1) * LN2_Q44 + log_mantissa_q44(LN_TABLE_Q30, m1);
	// |v| < 2^55, so the conversion rounds (correctly, deterministically)
	// rather than being exact like the trig paths; that costs < 1e-13.
	return (double)v * (1.0 / (double)(1LL << 44));
}

double Log10(double x)
{
	if (maths_mode != MathsMode::New)
		return std::log10(x);
	if (!(x > 0) || !std::isfinite(x))
		return std::log10(x);

	// Exact powers of ten return exact integers (as std::log10 does in
	// practice), so digit-counting scripts get Log10(100) == 2, not 1.9999.
	// 1e-22..1e22 covers every power of ten a script can produce or store.
	static const double pow10[] = {
		1e-22,1e-21,1e-20,1e-19,1e-18,1e-17,1e-16,1e-15,1e-14,1e-13,1e-12,
		1e-11,1e-10,1e-9,1e-8,1e-7,1e-6,1e-5,1e-4,1e-3,1e-2,1e-1,1e0,1e1,
		1e2,1e3,1e4,1e5,1e6,1e7,1e8,1e9,1e10,1e11,1e12,1e13,1e14,1e15,1e16,
		1e17,1e18,1e19,1e20,1e21,1e22,
	};
	if (x >= 1e-22 && x <= 1e22)
		for (int i = 0; i < 45; i++)
			if (x == pow10[i])
				return (double)(i - 22);

	int e;
	double m1 = std::frexp(x, &e) * 2.0 - 1.0;
	int64_t v = (int64_t)(e - 1) * LOG10_2_Q44 + log_mantissa_q44(LOG10_TABLE_Q30, m1);
	return (double)v * (1.0 / (double)(1LL << 44));
}

double Pow(double base, double exponent)
{
	if (maths_mode != MathsMode::New)
		return std::pow(base, exponent);
	// IEEE 754 fully specifies pow for these, so std::pow is already
	// deterministic: 0 or inf base, 0 exponent (-> 1), nan operands.
	if (base == 0 || exponent == 0 || !std::isfinite(base) || !std::isfinite(exponent))
		return std::pow(base, exponent);
	if (exponent == std::floor(exponent) && std::fabs(exponent) < 9007199254740992.0)
		return pow_integer(base, exponent);
	if (base < 0)
		return std::pow(base, exponent); // non-integer power of negative: NaN

	// base^exponent = 2^(exponent * log2(base)). log2 needs no Q44 constant:
	// the exponent part is exactly (e-1) << 44.
	int e;
	double m1 = std::frexp(base, &e) * 2.0 - 1.0;
	int64_t lg = ((int64_t)(e - 1) << 44) + log_mantissa_q44(LOG2_TABLE_Q30, m1);
	// Power-of-two scales commute with rounding, so even if unsafe-math
	// reassociates these two multiplies, the bits cannot change.
	double y = exponent * ((double)lg * (1.0 / (double)(1LL << 44)));
	if (y >= 1100.0)
		return HUGE_VAL; // 2^1024 already overflows
	if (y <= -1100.0)
		return 0.0;
	double k = std::floor(y);
	double f = y - k; // exact: k is within one of y
	return std::ldexp(exp2_frac(f), (int)k);
}



} //namespace math


} //namespace zc
