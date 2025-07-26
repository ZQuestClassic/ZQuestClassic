#ifndef ZC_MATH_H_
#define ZC_MATH_H_

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
float CalculateBezier(const float p1, const float t1, const float p2, float t);
float CalculateBezier(const float p1, const float t1, const float t2, const float p2, float t);
float CalculateBezier(const float p1, const float t1, const float t2, const float t3, const float p2, float t);
double Sin(double x);
double SinD(int16_t x);
double Cos(double x);
double CosD(int16_t x);
double Tan(double x);


} //namespace math


} //namespace zc

#endif
