

#include <math.h>
#include "..\scriptEngine.h"


namespace ScriptMath
{
	float __cdecl Fractionf(float v)
	{
		float intPart;
		return modff(v, &intPart);
	}
}


void ScriptRegistrar::RegisterMathFunctions(asIScriptEngine* engine)
{
	int r;
	using namespace ScriptMath;

	//engine->SetDefaultNamespace("Math"); //testing

	//////////////////////////////////////////////////////////////////////////
	/// Floating-Point
	//////////////////////////////////////////////////////////////////////////

	// Trigonometric functions
	r = engine->RegisterGlobalFunction("float Cos(float)", asFUNCTIONPR(cosf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sin(float)", asFUNCTIONPR(sinf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Tan(float)", asFUNCTIONPR(tanf, (float), float), asCALL_CDECL); Assert( r >= 0 );

	r = engine->RegisterGlobalFunction("float Acos(float)", asFUNCTIONPR(acosf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Asin(float)", asFUNCTIONPR(asinf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Atan(float)", asFUNCTIONPR(atanf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Atan2(float,float)", asFUNCTIONPR(atan2f, (float, float), float), asCALL_CDECL); Assert( r >= 0 );

	// Hyberbolic functions
	r = engine->RegisterGlobalFunction("float Cosh(float)", asFUNCTIONPR(coshf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sinh(float)", asFUNCTIONPR(sinhf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Tanh(float)", asFUNCTIONPR(tanhf, (float), float), asCALL_CDECL); Assert( r >= 0 );

	// Exponential and logarithmic functions
	r = engine->RegisterGlobalFunction("float Log(float)", asFUNCTIONPR(logf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Log10(float)", asFUNCTIONPR(log10f, (float), float), asCALL_CDECL); Assert( r >= 0 );

	// Power functions
	r = engine->RegisterGlobalFunction("float Pow(float, float)", asFUNCTIONPR(powf, (float, float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Sqrt(float)", asFUNCTIONPR(sqrtf, (float), float), asCALL_CDECL); Assert( r >= 0 );

	// Nearest integer, absolute value, and remainder functions
	r = engine->RegisterGlobalFunction("float Ceil(float)", asFUNCTIONPR(ceilf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Abs(float)", asFUNCTIONPR(fabsf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Floor(float)", asFUNCTIONPR(floorf, (float), float), asCALL_CDECL); Assert( r >= 0 );
	r = engine->RegisterGlobalFunction("float Fraction(float)", asFUNCTIONPR(Fractionf, (float), float), asCALL_CDECL); Assert( r >= 0 );

}



