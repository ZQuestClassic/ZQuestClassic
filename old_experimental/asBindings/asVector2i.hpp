
#include "../ZCMath.h"


namespace scriptpoint
{

	void PointDefaultConstructor(Vector2i *self)
	{
		new (self) Vector2i();
	}


	void PointCopyConstructor(const Vector2i& other, Vector2i *self)
	{
		new (self) Vector2i(other);
	}


	void PointInitConstructor(int x, int y, Vector2i *self)
	{
		new (self) Vector2i(x,y);
	}



} //namespace scriptpoint


void ScriptRegistrar::RegisterVector2i(asIScriptEngine* engine)
{
	int r;
	//using namespace scriptpoint;

	// Register the type
	r = engine->RegisterObjectType("Vector2i", sizeof(Vector2i), asOBJ_VALUE | asOBJ_POD /*| asOBJ_APP_CLASS_CA*/); Assert(r >= 0);

	// Register the object properties
	r = engine->RegisterObjectProperty("Vector2i", "int x", asOFFSET(Vector2i, x)); Assert(r >= 0);
	r = engine->RegisterObjectProperty("Vector2i", "int y", asOFFSET(Vector2i, y)); Assert(r >= 0);

	// Register the constructors
	//r = engine->RegisterObjectBehaviour("Vector2i", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(PointDefaultConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	//r = engine->RegisterObjectBehaviour("Vector2i", asBEHAVE_CONSTRUCT, "void f(const Vector2i& in)", asFUNCTION(PointCopyConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	//r = engine->RegisterObjectBehaviour("Vector2i", asBEHAVE_CONSTRUCT, "void f(int, int)",  asFUNCTION(PointInitConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);

	// Register the operator overloads
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i& opAddAssign(const Vector2i& in)", asMETHODPR(Vector2i, operator+=, (const Vector2i&), Vector2i&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i& opSubAssign(const Vector2i& in)", asMETHODPR(Vector2i, operator-=, (const Vector2i&), Vector2i&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i& opMulAssign(const Vector2i& in)", asMETHODPR(Vector2i, operator*=, (const Vector2i&), Vector2i&), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i& opDivAssign(const Vector2i& in)", asMETHODPR(Vector2i, operator/=, (const Vector2i&), Vector2i&), asCALL_THISCALL); Assert(r >= 0);

	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opAdd(const Vector2i& in) const", asMETHODPR(Vector2i, operator +, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opSub(const Vector2i& in) const", asMETHODPR(Vector2i, operator -, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opMul(const Vector2i& in) const", asMETHODPR(Vector2i, operator *, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2i", "Vector2i opDiv(const Vector2i& in) const", asMETHODPR(Vector2i, operator /, (const Vector2i&) const, Vector2i), asCALL_THISCALL); Assert(r >= 0);

}



void ScriptRegistrar::RegisterVector2f(asIScriptEngine* engine)
{
	int r;
	//using namespace scriptvec2;

	r = engine->RegisterObjectType("Vector2f", sizeof(Vector2f), asOBJ_VALUE | asOBJ_POD/* | asOBJ_APP_CLASS_CA*/); Assert(r >= 0);

	/* members */
	r = engine->RegisterObjectProperty("Vector2f", "float x", offsetof(Vector2f, x)); Assert(r >= 0);
	r = engine->RegisterObjectProperty("Vector2f", "float y", offsetof(Vector2f, y)); Assert(r >= 0);

	/* constructors */
	r = engine->RegisterObjectBehaviour("Vector2f", asBEHAVE_CONSTRUCT,  "void f()", asFUNCTION(Vector2DefaultConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2f", asBEHAVE_CONSTRUCT,  "void f(const Vector2f& in)", asFUNCTION(Vector2CopyConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2f", asBEHAVE_CONSTRUCT,  "void f(float)", asFUNCTION(Vector2SingleInitConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Vector2f", asBEHAVE_CONSTRUCT,  "void f(float, float)", asFUNCTION(Vector2InitConstructor), asCALL_CDECL_OBJLAST); Assert(r >= 0);

	/* operator overloads */
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opAssign(const Vector2f& in)", asMETHODPR(Vector2f, operator =, (const Vector2f&), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opAssign(const Vector2f& in)", asFUNCTION(V2ass), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
	//r = engine->RegisterObjectMethod("Vector2f", "Vector2f& opAssign(float)", asMETHODPR(Vector2f, operator =, (float), Vector2f&), asCALL_THISCALL); Assert(r >= 0);


	r = engine->RegisterObjectMethod("Vector2f", "bool opEquals(const Vector2f& in) const", asMETHODPR(Vector2f, operator ==, (const Vector2f&) const, bool), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Vector2f", "bool opEquals(const Vector2f& in) const", asMETHODPR(Vector2f, operator !=, (const Vector2f&) const, bool), asCALL_THISCALL); Assert(r >= 0);
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
	r = engine->RegisterObjectMethod("Vector2f", "Vector2f& Normalize()", asMETHODPR(Vector2f, Normalize, (void), Vector2f&), asCALL_THISCALL); Assert(r >= 0);
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



namespace scriptrecti
{

	void RectDefaultConstructor( Rect *self )
	{
		new(self) Rect();
	}


	void RectCopyConstructor( const Rect &other, Rect *self )
	{
		new(self) Rect(other);
	}


	void RectInitConstructorp( const Point &p, const Point &s, Rect *self )
	{
		new(self) Rect(p, s);
	}


	void RectInitConstructor( int x, int y, int sx, int sy, Rect *self )
	{
		new(self) Rect(x, y, sx, sy);
	}

} //namespace scriptrecti


void ScriptRegistrar::RegisterRect(asIScriptEngine* engine)
{
	int r(0);
	using namespace scriptrecti;
	

	r = engine->RegisterObjectType( "Recti", sizeof(Rect), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert( r >= 0 );

	/* members */
	r = engine->RegisterObjectProperty( "Recti", "Point pos", offsetof(Rect, pos)); assert( r >= 0 );
	r = engine->RegisterObjectProperty( "Recti", "Point size", offsetof(Rect, size)); assert( r >= 0 );

	/* constructors */
	r = engine->RegisterObjectBehaviour( "Recti", asBEHAVE_CONSTRUCT,  "void f()",                asFUNCTION(RectDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour( "Recti", asBEHAVE_CONSTRUCT,  "void f(const Recti &in)", asFUNCTION(RectCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour( "Recti", asBEHAVE_CONSTRUCT,  "void f(int, int, int, int)", asFUNCTION(RectInitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour( "Recti", asBEHAVE_CONSTRUCT,  "void f(const Vector2 &in, const Vector2 &in)",  asFUNCTION(RectInitConstructorp), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	/* operator overloads */
	r = engine->RegisterObjectMethod( "Recti", "Recti &opAssign(const Recti &in)", asMETHODPR(Rect, operator =, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );

	r = engine->RegisterObjectMethod( "Recti", "bool opEquals(const Recti &in) const", asMETHODPR(Rect, operator ==, (const Rect&) const, bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "bool opEquals(const Recti &in) const", asMETHODPR(Rect, operator !=, (const Rect&) const, bool), asCALL_THISCALL); assert( r >= 0 );
/*
	r = engine->RegisterObjectMethod( "Recti", "bool opCmp(const Recti &in) const", asMETHODPR(Rect, operator >, (const Rect&) const, bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "bool opCmp(const Recti &in) const", asMETHODPR(Rect, operator <, (const Rect&) const, bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "bool opCmp(const Recti &in) const", asMETHODPR(Rect, operator >=, (const Rect&) const, bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "bool opCmp(const Recti &in) const", asMETHODPR(Rect, operator <=, (const Rect&) const, bool), asCALL_THISCALL); assert( r >= 0 );
*/
/*
	r = engine->RegisterObjectMethod( "Recti", "Recti &opAddAssign(const Recti &in)", asMETHODPR(Rect, operator +=, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "Recti &opSubAssign(const Recti &in)", asMETHODPR(Rect, operator -=, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "Recti &opMulAssign(const Recti &in)", asMETHODPR(Rect, operator *=, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "Recti &opDivAssign(const Recti &in)", asMETHODPR(Rect, operator /=, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );
*/
	//r = engine->RegisterObjectMethod( "Recti", "Recti opAdd(const Recti &in) const", asMETHODPR(Rect, operator +, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
	//r = engine->RegisterObjectMethod( "Recti", "Recti opSub(const Recti &in) const", asMETHODPR(Rect, operator -, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
	//r = engine->RegisterObjectMethod( "Recti", "Recti opMul(const Recti &in) const", asMETHODPR(Rect, operator *, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
	//r = engine->RegisterObjectMethod( "Recti", "Recti opDiv(const Recti &in) const", asMETHODPR(Rect, operator /, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );


	r = engine->RegisterObjectMethod( "Recti", "float top() const", asMETHOD(Rect, Top), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "float bottom() const", asMETHOD(Rect, Bottom), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "float left() const", asMETHOD(Rect, Left), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "float right() const", asMETHOD(Rect, Right), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "Point top_left() const", asMETHOD(Rect, TopLeft), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "Point bottom_left() const", asMETHOD(Rect, BottomLeft), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "Point bottom_right() const", asMETHOD(Rect, BottomRight), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod( "Recti", "Point top_right() const", asMETHOD(Rect, TopRight), asCALL_THISCALL); assert( r >= 0 );


	r = engine->RegisterObjectMethod( "Recti", "String to_string()", asFUNCTION(ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );


}


AS_NAMESPACE_END


