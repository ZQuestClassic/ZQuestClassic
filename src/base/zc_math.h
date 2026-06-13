#ifndef __zc_math_h_
#define __zc_math_h_

#include <cstdint>

#ifndef PI
#define PI 3.14159265358979323846
#endif

namespace zc
{


namespace math
{


bool IsPowerOfTwo(uint32_t x);
uint32_t NextPowerOfTwo(uint32_t x);
int32_t SafeMod(int32_t x, int32_t y);
float ToDegrees(float radians);
float ToRadians(float degrees);
float Round(float x);
float Lerp(float a, float b, float t);
double Sin(double x);
double SinD(int16_t x);
double Cos(double x);
double CosD(int16_t x);
double Tan(double x);
double ArcSin(double x);
double ArcCos(double x);
double ArcTan2(double y, double x);
double Ln(double x);
double Log10(double x);
double Pow(double base, double exponent);

// Which implementation the functions above use. The non-New modes reproduce
// pre-replay-version-59 behavior exactly; replay_compat_setup_zc_maths() maps
// the active replay's version to a mode, so this header needs no knowledge of
// the replay system (and the legacy tiers stay unit-testable in isolation).
enum class MathsMode
{
	// The deterministic tables (replay version 59+). The default.
	New,
	// Pre-59 while a replay was active: Q15 sin1 trig (libm for everything
	// but trig), with the rounding tier introduced by that replay version.
	LegacyReplayV0,
	LegacyReplayV4,
	LegacyReplayV21,
};
void set_maths_mode(MathsMode mode);


} //namespace math


} //namespace zc

#endif
