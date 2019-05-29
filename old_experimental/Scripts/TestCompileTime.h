
#include "Defines.h"

int gv;

namespace NS
{
	int globalVar;

	class INSClass
	{
		private int b;
		int gv;

		INSClass()
		{
			b = 9;
			//gv = b;
		}
	}
}

enum Enum1
{
	Value = 3
}

class IClass
{
	private int a;

	int PropertyAccessor
	{
		get const { return a; }
		set { a = value; }
	}

	void SetGlobal(int val)
	{
		gv = Enum1::Value;
	}
}

#define DECLARE_CLASS(name, type, pad) \
class Class##type##name : IClass { \
	type v0; \
	type v1; \
	string str = #type; \
	type Foo0() { PropertyAccessor = PropertyAccessor + int(v0 * v0 + v1 - v0 * v1); return PropertyAccessor; }  \
	type Foo1(type a) { return v1 + a - v0 * 39 + 2*v0 + v1; }  \
	string ToString() { return str + " hello"; } \
} \
	enum Enum##name##type##pad { \
	EnumValueLongName1, \
	EnumValueLongName2, \
	EnumValueLongName3, \
	EnumValueLongName4, \
	EnumValueLongName5, \
	EnumValueLongName6, \
	EnumValueLongName7, \
	EnumValueLongName8 \
}


#define DECLARE_CLASS_GROUP(x) \
DECLARE_CLASS(TS##x, int8, x) \
DECLARE_CLASS(TS##x, int16, x) \
DECLARE_CLASS(TS##x, int32, x) \
DECLARE_CLASS(TS##x, int64, x) \
DECLARE_CLASS(TS##x, uint8, x) \
DECLARE_CLASS(TS##x, uint16, x) \
DECLARE_CLASS(TS##x, uint32, x) \
DECLARE_CLASS(TS##x, float, x) \


//Define 80 classes + 640 enums

DECLARE_CLASS_GROUP(1);
DECLARE_CLASS_GROUP(2);
DECLARE_CLASS_GROUP(4);
DECLARE_CLASS_GROUP(8);
DECLARE_CLASS_GROUP(16);
DECLARE_CLASS_GROUP(32);
DECLARE_CLASS_GROUP(64);
DECLARE_CLASS_GROUP(128);
DECLARE_CLASS_GROUP(256);
DECLARE_CLASS_GROUP(777);

