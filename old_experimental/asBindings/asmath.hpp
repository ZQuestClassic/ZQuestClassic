
#include "ZCMath.h"
#include "Utility.h"

#include <angelscript.h>


namespace ScriptMath
{
	float Fractionf(float v)
	{
		float intPart;
		return modff(v, &intPart);
	}

	int32 Rand(int32 x)
	{
		return x ? rand() % x : 0;
	}

	int32 Rand(int low, int high)
	{
		return low + Rand((1 + high) - low);
	}
}


void ScriptRegistrar::RegisterMathFunctions(asIScriptEngine* engine)
{
	int r;
	using namespace ScriptMath;

	//engine->SetDefaultNamespace("Math"); //testing

	//////////////////////////////////////////////////////////////////////////
	/// Floating-Vector2i
	//////////////////////////////////////////////////////////////////////////

	// Random Functions
	r = engine->RegisterGlobalFunction("int Rand(int)", asFUNCTIONPR(Rand, (int32), int32), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int Rand(int, int)", asFUNCTIONPR(Rand, (int32, int32), int32), asCALL_CDECL); Assert(r >= 0);

	// Utility Functions
	r = engine->RegisterGlobalFunction("float Min(float, float)", asFUNCTIONPR(Min, (float, float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Max(float, float)", asFUNCTIONPR(Max, (float, float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int Min(int, int)", asFUNCTIONPR(Min, (int32, int32), int32), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int Max(int, int)", asFUNCTIONPR(Max, (int32, int32), int32), asCALL_CDECL); Assert(r >= 0);

	r = engine->RegisterGlobalFunction("float SmoothStep(float, float, float)", asFUNCTIONPR(SmoothStep, (float, float, float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Lerp(float, float, float)", asFUNCTIONPR(Lerp, (float, float, float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int Clamp(int, int, int)", asFUNCTIONPR(Clamp, (int32, int32, int32), int32), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Clamp(float, float, float)", asFUNCTIONPR(Clamp, (float, float, float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Clamp01(float)", asFUNCTIONPR(Clamp01, (float), float), asCALL_CDECL); Assert(r >= 0);

	r = engine->RegisterGlobalFunction("float ToDegrees(float)", asFUNCTIONPR(ToDegrees, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float ToRadians(float)", asFUNCTIONPR(ToRadians, (float), float), asCALL_CDECL); Assert(r >= 0);

	r = engine->RegisterGlobalFunction("int SnapToGrid(int, int)", asFUNCTIONPR(SnapToGrid, (int32, int32), int32), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float SnapToGrid(float, float)", asFUNCTIONPR(SnapToGrid, (float, float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Distance(float, float, float, float)", asFUNCTIONPR(Distance, (float, float, float, float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float DistanceSquared(float, float, float, float)", asFUNCTIONPR(DistanceSquared, (float, float, float, float), float), asCALL_CDECL); Assert(r >= 0);

	// Trigonometric functions
	r = engine->RegisterGlobalFunction("float Cos(float)", asFUNCTIONPR(cosf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Sin(float)", asFUNCTIONPR(sinf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Tan(float)", asFUNCTIONPR(tanf, (float), float), asCALL_CDECL); Assert(r >= 0);

	r = engine->RegisterGlobalFunction("float Acos(float)", asFUNCTIONPR(acosf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Asin(float)", asFUNCTIONPR(asinf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Atan(float)", asFUNCTIONPR(atanf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Atan2(float,float)", asFUNCTIONPR(atan2f, (float, float), float), asCALL_CDECL); Assert(r >= 0);

	// Hyberbolic functions
	r = engine->RegisterGlobalFunction("float Cosh(float)", asFUNCTIONPR(coshf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Sinh(float)", asFUNCTIONPR(sinhf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Tanh(float)", asFUNCTIONPR(tanhf, (float), float), asCALL_CDECL); Assert(r >= 0);

	// Exponential and logarithmic functions
	r = engine->RegisterGlobalFunction("float Log(float)", asFUNCTIONPR(logf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Log10(float)", asFUNCTIONPR(log10f, (float), float), asCALL_CDECL); Assert(r >= 0);

	// Power functions
	r = engine->RegisterGlobalFunction("float Pow(float, float)", asFUNCTIONPR(powf, (float, float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Sqrt(float)", asFUNCTIONPR(sqrtf, (float), float), asCALL_CDECL); Assert(r >= 0);

	// Nearest integer, absolute value, and remainder functions
	r = engine->RegisterGlobalFunction("float Ceil(float)", asFUNCTIONPR(ceilf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Abs(float)", asFUNCTIONPR(fabsf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Floor(float)", asFUNCTIONPR(floorf, (float), float), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("float Fraction(float)", asFUNCTIONPR(Fractionf, (float), float), asCALL_CDECL); Assert(r >= 0);

}


namespace ScriptBindings
{
	void Vector2iDefaultConstructor(Vector2i *self)
	{
		// Initialize all script math-types to 0.
		new(self) Vector2i(0);
	}

	void Vector2iCopyConstructor(const Vector2i &other, Vector2i* self)
	{
		new(self) Vector2i(other);
	}

	void Vector2iInitConstructor(int32 x, int32 y, Vector2i* self)
	{
		new(self) Vector2i(x, y);
	}

	void Vector2iSingleInitConstructor(int32 value, Vector2i* self)
	{
		new(self) Vector2i(value);
	}

	std::string Vector2iToString(Vector2i* self)
	{
		char buffer[64];
		u32 position = 0;

		position = Itoa(self->x, buffer + position);
		buffer[position++] = ',';
		buffer[position++] = ' ';
		position = Itoa(self->y, buffer + position);

		return std::string(buffer, position);
	}
}


void ScriptRegistrar::RegisterVector2i(asIScriptEngine* engine)
{
	int r;
	using namespace ScriptBindings;

	// Register the type
	r = engine->RegisterObjectType("Vector2i", sizeof(Vector2i), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); Assert(r >= 0);

	// Register the object properties
	r = engine->RegisterObjectProperty("Vector2i", "int x", asOFFSET(Vector2i, x)); Assert(r >= 0);
	r = engine->RegisterObjectProperty("Vector2i", "int y", asOFFSET(Vector2i, y)); Assert(r >= 0);

	r = engine->RegisterObjectBehaviour("Vector2i", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector2iDefaultConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2i", asBEHAVE_CONSTRUCT, "void f(const Vector2i& in)", asFUNCTION(Vector2iCopyConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2i", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(Vector2iSingleInitConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2i", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(Vector2iInitConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);

	// Register the operator overloads
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i& opAddAssign(const Vector2i& in)", asMETHODPR(Vector2i, operator+=, (const Vector2i&), Vector2i&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i& opSubAssign(const Vector2i& in)", asMETHODPR(Vector2i, operator-=, (const Vector2i&), Vector2i&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i& opMulAssign(const Vector2i& in)", asMETHODPR(Vector2i, operator*=, (const Vector2i&), Vector2i&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i& opDivAssign(const Vector2i& in)", asMETHODPR(Vector2i, operator/=, (const Vector2i&), Vector2i&), asCALL_THISCALL); Assert(r >= 0);

	//r = engine->RegisterObjectMethod("Vector2i", "Vector2i opAdd(const Vector2i& in) const", asMETHODPR(Vector2i, operator +, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2i", "Vector2i opSub(const Vector2i& in) const", asMETHODPR(Vector2i, operator -, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2i", "Vector2i opMul(const Vector2i& in) const", asMETHODPR(Vector2i, operator *, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2i", "Vector2i opDiv(const Vector2i& in) const", asMETHODPR(Vector2i, operator /, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);

	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opAdd(const Vector2i& in) const", asMETHODPR(Vector2i, operator +, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opSub(const Vector2i& in) const", asMETHODPR(Vector2i, operator -, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opMul(const Vector2i& in) const", asMETHODPR(Vector2i, operator *, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opDiv(const Vector2i& in) const", asMETHODPR(Vector2i, operator /, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opAdd(int) const", asMETHODPR(Vector2i, operator +, (int32) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opSub(int) const", asMETHODPR(Vector2i, operator -, (int32) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opMul(int) const", asMETHODPR(Vector2i, operator *, (int32) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opDiv(int) const", asMETHODPR(Vector2i, operator /, (int32) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opAdd(int, const Vector2i& in) const", asFUNCTIONPR(operator +, (int32, const Vector2i&), Vector2i), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opSub(int, const Vector2i& in) const", asFUNCTIONPR(operator -, (int32, const Vector2i&), Vector2i), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opMul(int, const Vector2i& in) const", asFUNCTIONPR(operator *, (int32, const Vector2i&), Vector2i), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opDiv(int, const Vector2i& in) const", asFUNCTIONPR(operator /, (int32, const Vector2i&), Vector2i), asCALL_CDECL_OBJLAST); Assert(r >= 0);

}



namespace ScriptBindings
{
	void Vector2fDefaultConstructor(Vector2f *self)
	{
		// Initialize all script math-types to 0.
		new(self) Vector2f(0.0f);
	}

	void Vector2fCopyConstructor(const Vector2f &other, Vector2f* self)
	{
		new(self) Vector2f(other);
	}

	void Vector2fInitConstructor(float x, float y, Vector2f* self)
	{
		new(self) Vector2f(x, y);
	}

	void Vector2fSingleInitConstructor(float value, Vector2f* self)
	{
		new(self) Vector2f(value);
	}

	std::string Vector2fToString(Vector2f* self)
	{
		char buffer[64];
		u32 position = 0;

		position = Ftoa(self->x, buffer + position);
		buffer[position++] = ',';
		buffer[position++] = ' ';
		position = Ftoa(self->y, buffer + position);

		return std::string(buffer, position);
	}
}


void ScriptRegistrar::RegisterVector2f(asIScriptEngine* engine)
{
	int r;
	using namespace ScriptBindings;

	r = engine->RegisterObjectType("Vector2f", sizeof(Vector2f), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); Assert(r >= 0);

	/* members */
	r = engine->RegisterObjectProperty("Vector2f", "float x", offsetof(Vector2f, x)); Assert(r >= 0);
	r = engine->RegisterObjectProperty("Vector2f", "float y", offsetof(Vector2f, y)); Assert(r >= 0);

	/* constructors */
	r = engine->RegisterObjectBehaviour("Vector2f", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector2fDefaultConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2f", asBEHAVE_CONSTRUCT, "void f(const Vector2f& in)", asFUNCTION(Vector2fCopyConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2f", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(Vector2fSingleInitConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2f", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(Vector2fInitConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);

	/* operator overloads */
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opAssign(const Vector2f& in)", asMETHODPR(Vector2f, operator =, (const Vector2f&), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opAssign(const Vector2f& in)", asFUNCTION(V2ass), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opAssign(float)", asMETHODPR(Vector2f, operator =, (float), Vector2f&), asCALL_THISCALL); Assert(r >= 0);


	r = engine->RegisterObjectMethod("Vector2f", "bool opEquals(const Vector2f& in) const", asMETHODPR(Vector2f, operator ==, (const Vector2f&) const, bool), asCALL_THISCALL); Assert(r >= 0);
//	r = engine->RegisterObjectMethod("Vector2f", "bool opEquals(const Vector2f& in) const", asMETHODPR(Vector2f, operator !=, (const Vector2f&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "bool opCmp(const Vector2f& in) const", asMETHODPR(Vector2f, operator >, (const Vector2f&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "bool opCmp(const Vector2f& in) const", asMETHODPR(Vector2f, operator <, (const Vector2f&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "bool opCmp(const Vector2f& in) const", asMETHODPR(Vector2f, operator >=, (const Vector2f&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "bool opCmp(const Vector2f& in) const", asMETHODPR(Vector2f, operator <=, (const Vector2f&) const, bool), asCALL_THISCALL); Assert(r >= 0);

	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opAddAssign(const Vector2f& in)", asMETHODPR(Vector2f, operator +=, (const Vector2f&), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opSubAssign(const Vector2f& in)", asMETHODPR(Vector2f, operator -=, (const Vector2f&), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opMulAssign(const Vector2f& in)", asMETHODPR(Vector2f, operator *=, (const Vector2f&), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opDivAssign(const Vector2f& in)", asMETHODPR(Vector2f, operator /=, (const Vector2f&), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opAddAssign(float)", asMETHODPR(Vector2f, operator +=, (float), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opSubAssign(float)", asMETHODPR(Vector2f, operator -=, (float), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opMulAssign(float)", asMETHODPR(Vector2f, operator *=, (float), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opDivAssign(float)", asMETHODPR(Vector2f, operator /=, (float), Vector2f&), asCALL_THISCALL); Assert(r >= 0);

	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opAdd(const Vector2f& in) const", asMETHODPR(Vector2f, operator +, (const Vector2f&) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opSub(const Vector2f& in) const", asMETHODPR(Vector2f, operator -, (const Vector2f&) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opMul(const Vector2f& in) const", asMETHODPR(Vector2f, operator *, (const Vector2f&) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opDiv(const Vector2f& in) const", asMETHODPR(Vector2f, operator /, (const Vector2f&) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opAdd(float) const", asMETHODPR(Vector2f, operator +, (float) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opSub(float) const", asMETHODPR(Vector2f, operator -, (float) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opMul(float) const", asMETHODPR(Vector2f, operator *, (float) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opDiv(float) const", asMETHODPR(Vector2f, operator /, (float) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opAdd(float, const Vector2f& in) const", asFUNCTIONPR(operator +, (float, const Vector2f&), Vector2f), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opSub(float, const Vector2f& in) const", asFUNCTIONPR(operator -, (float, const Vector2f&), Vector2f), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opMul(float, const Vector2f& in) const", asFUNCTIONPR(operator *, (float, const Vector2f&), Vector2f), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f opDiv(float, const Vector2f& in) const", asFUNCTIONPR(operator /, (float, const Vector2f&), Vector2f), asCALL_CDECL_OBJLAST); Assert(r >= 0);

	// Register the object methods
	r = engine->RegisterObjectMethod("Vector2f", "void Normalize()", asMETHODPR(Vector2f, Normalize, (void), void), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f Normalized() const", asMETHODPR(Vector2f, Normalized, (void) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "float Length() const", asMETHODPR(Vector2f, Length, (void) const, float), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "float LengthSquared() const", asMETHODPR(Vector2f, LengthSquared, (void) const, float), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "float Distance(const Vector2f& in) const", asMETHODPR(Vector2f, Distance, (const Vector2f& v) const, float), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "float DistanceSquared(const Vector2f& in) const", asMETHODPR(Vector2f, DistanceSquared, (const Vector2f& v) const, float), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "float Dot(const Vector2f& in) const", asMETHODPR(Vector2f, Dot, (const Vector2f& v) const, float), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "float Cross(const Vector2f& in) const", asMETHODPR(Vector2f, Cross, (const Vector2f& v) const, float), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "float Angle() const", asMETHODPR(Vector2f, Angle, (void) const, float), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "float AngleTo(const Vector2f& in) const", asMETHODPR(Vector2f, Angle, (const Vector2f& v) const, float), asCALL_THISCALL); Assert(r >= 0);

	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f min(const Vector2f& in) const", asMETHODPR(Vector2f, Min, (const Vector2f&) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f max(const Vector2f& in) const", asMETHODPR(Vector2f, Max, (const Vector2f&) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);

	r = engine->RegisterObjectMethod("Vector2f", "Vector2f Abs() const", asMETHODPR(Vector2f, Abs, (void) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f Ceil() const", asMETHODPR(Vector2f, Ceil, (void) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f Round() const", asMETHODPR(Vector2f, Round, (void) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f Floor() const", asMETHODPR(Vector2f, Floor, (void) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f Negate() const", asMETHODPR(Vector2f, Negate, (void) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f Polar(const Vector2f& in,float) const", asMETHODPR(Vector2f, Polar, (const Vector2f&, float) const, Vector2f), asCALL_THISCALL); Assert(r >= 0);

	//r = engine->RegisterObjectMethod("Vector2f", "String ToString()", asFUNCTION(Vector2ToString), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
}




namespace ScriptBindings
{
	void RectDefaultConstructor(Rect *self)
	{
		new(self) Rect();
	}

	void RectCopyConstructor(const Rect &other, Rect *self)
	{
		new(self) Rect(other);
	}

	void RectInitConstructorp(const Vector2i &p, const Vector2i &s, Rect *self)
	{
		new(self) Rect(p, s);
	}

	void RectInitConstructor(int x, int y, int sx, int sy, Rect *self)
	{
		new(self) Rect(x, y, sx, sy);
	}

	std::string RectToString(Vector2i* self)
	{
		char buffer[80];
		u32 position = 0;

		position = Itoa(self->x, buffer + position);
		buffer[position++] = ',';
		buffer[position++] = ' ';
		position = Itoa(self->y, buffer + position);
		buffer[position++] = ',';
		buffer[position++] = ' ';
		position = Itoa(self->y, buffer + position);
		buffer[position++] = ',';
		buffer[position++] = ' ';
		position = Itoa(self->y, buffer + position);

		return std::string(buffer, position);
	}
}


void ScriptRegistrar::RegisterRect(asIScriptEngine* engine)
{
	int r;
	using namespace ScriptBindings;
	
	r = engine->RegisterObjectType("Rect", sizeof(Rect), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); Assert(r >= 0);

	// Members
	r = engine->RegisterObjectProperty("Rect", "int x", offsetof(Rect, x)); Assert(r >= 0);
	r = engine->RegisterObjectProperty("Rect", "int y", offsetof(Rect, y)); Assert(r >= 0);
	r = engine->RegisterObjectProperty("Rect", "Vector2i Position", offsetof(Rect, position)); Assert(r >= 0);
	r = engine->RegisterObjectProperty("Rect", "Vector2i Size", offsetof(Rect, size)); Assert(r >= 0);

	// Constructors
	r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(RectDefaultConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(const Rect &in)", asFUNCTION(RectCopyConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(int, int, int, int)", asFUNCTION(RectInitConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(const Vector2i &in, const Vector2i &in)", asFUNCTION(RectInitConstructorp), asCALL_CDECL_OBJLAST); Assert(r >= 0);

	// Operator Overloads
	r = engine->RegisterObjectMethod("Rect", "Rect &opAssign(const Rect &in)", asMETHODPR(Rect, operator =, (const Rect&), Rect&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "bool opEquals(const Rect &in) const", asMETHODPR(Rect, operator ==, (const Rect&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Rect", "bool opEquals(const Rect &in) const", asMETHODPR(Rect, operator !=, (const Rect&) const, bool), asCALL_THISCALL); Assert(r >= 0);

	/*
	r = engine->RegisterObjectMethod("Rect", "bool opCmp(const Rect &in) const", asMETHODPR(Rect, operator >, (const Rect&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "bool opCmp(const Rect &in) const", asMETHODPR(Rect, operator <, (const Rect&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "bool opCmp(const Rect &in) const", asMETHODPR(Rect, operator >=, (const Rect&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "bool opCmp(const Rect &in) const", asMETHODPR(Rect, operator <=, (const Rect&) const, bool), asCALL_THISCALL); Assert(r >= 0);
*/

/*
	r = engine->RegisterObjectMethod("Rect", "Rect &opAddAssign(const Rect &in)", asMETHODPR(Rect, operator +=, (const Rect&), Rect&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "Rect &opSubAssign(const Rect &in)", asMETHODPR(Rect, operator -=, (const Rect&), Rect&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "Rect &opMulAssign(const Rect &in)", asMETHODPR(Rect, operator *=, (const Rect&), Rect&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "Rect &opDivAssign(const Rect &in)", asMETHODPR(Rect, operator /=, (const Rect&), Rect&), asCALL_THISCALL); Assert(r >= 0);
*/
	//r = engine->RegisterObjectMethod("Rect", "Rect opAdd(const Rect &in) const", asMETHODPR(Rect, operator +, (const Rect&) const, Rect), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Rect", "Rect opSub(const Rect &in) const", asMETHODPR(Rect, operator -, (const Rect&) const, Rect), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Rect", "Rect opMul(const Rect &in) const", asMETHODPR(Rect, operator *, (const Rect&) const, Rect), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Rect", "Rect opDiv(const Rect &in) const", asMETHODPR(Rect, operator /, (const Rect&) const, Rect), asCALL_THISCALL); Assert(r >= 0);


	r = engine->RegisterObjectMethod("Rect", "int Top() const", asMETHOD(Rect, Top), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "int Bottom() const", asMETHOD(Rect, Bottom), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "int Left() const", asMETHOD(Rect, Left), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "int Right() const", asMETHOD(Rect, Right), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "Vector2i TopLeft() const", asMETHOD(Rect, TopLeft), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "Vector2i BottomLeft() const", asMETHOD(Rect, BottomLeft), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "Vector2i BottomRight() const", asMETHOD(Rect, BottomRight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "Vector2i TopRight() const", asMETHOD(Rect, TopRight), asCALL_THISCALL); Assert(r >= 0);


}



void ScriptRegistrar::RegisterMathDependencies(asIScriptEngine* engine)
{
	int r;
	using namespace ScriptBindings;

	r = engine->RegisterObjectMethod("Vector2i", "string ToString()", asFUNCTION(Vector2iToString), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "string ToString()", asFUNCTION(Vector2fToString), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Rect", "string ToString()", asFUNCTION(RectToString), asCALL_CDECL_OBJLAST); Assert(r >= 0);
}


//todo: not math!
namespace ScriptBindings
{
	void ScriptPrintCString(const char* str)
	{
		//printf(str);
		ScriptLog(str);
	}

	void ScriptPrintString(const ScriptString& str)
	{
		ScriptPrintCString(str.c_str());
	}

	void ScriptPrintInt(int32 value)
	{
		char buffer[32];
		Itoa(value, buffer);
		ScriptPrintCString(buffer);
	}

	void ScriptPrintFloat(float value)
	{
		char buffer[32];
		Ftoa(value, buffer);
		ScriptPrintCString(buffer);
	}

	void ScriptAssert(const std::string& str)
	{
		ScriptPrintCString(str.c_str());

		asIScriptContext* scriptContext = asGetActiveContext();
		if(scriptContext)
		{
			scriptContext->SetException(str.c_str());
			//LogScriptException(scriptContext);
		}
	}

	// Only valid if called from a script object instance and not a callback function.
	void Waitframes(int32 frames)
	{
		asIScriptContext *scriptContext = asGetActiveContext();
		if(scriptContext)
		{
			Script* activeScript = (Script*)scriptContext->GetUserData();
			if(activeScript)
			{
				activeScript->Suspend(frames);

				// Suspend the script object.
				// The script class will be notified and resume execution on a later frame.
				scriptContext->Suspend(); 
			}
			else //script-writer messed up.
			{
				ScriptLog("Waitframes(%i) error dummy-head!", frames);
			}
		}
	}

	void CreateGlobalScript(void** p)
	{
	}

	void SetGlobalCallback(int32 id, asIScriptFunction* callbackFunc)
	{
		if(id < MAX_SCRIPT_GLOBAL_CALLBACKS)
		{
			if(globalScriptCallbackCache[id])
				globalScriptCallbackCache[id]->Release(); // Release the previous callback

			// Store the received handle for later use
			globalScriptCallbackCache[id] = callbackFunc; // Works also if NULL.
		}
	}

}


void ScriptRegistrar::RegisterGlobalFunctions(asIScriptEngine* engine)
{
	int r;
	using namespace ScriptBindings;

	r = engine->RegisterGlobalFunction("void Print(int)", asFUNCTION(ScriptPrintInt), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void Print(float)", asFUNCTION(ScriptPrintFloat), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void Print(const string& in)", asFUNCTION(ScriptPrintString), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void ThrowException(const string& in)", asFUNCTION(ScriptAssert), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void Waitframes(int)", asFUNCTION(Waitframes), asCALL_CDECL); Assert(r >= 0);


	// Global Callbacks ---- Game namespace?
	engine->RegisterFuncdef("void CallbackFunc()");
	engine->RegisterGlobalFunction("void SetGlobalCallback(int, CallbackFunc @)", asFUNCTION(SetGlobalCallback), asCALL_CDECL);


	//r = engine->RegisterGlobalFunction("void CreateGlobalScript(ref @)", asFUNCTION(CreateGlobalScript), asCALL_CDECL); Assert(r >= 0);

}

