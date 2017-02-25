
#pragma once

#include "Config.h"


inline u32 StringCat(char* RESTRICT buffer, const char* nullTerminatedCString)
{
	char* RESTRICT p = buffer;
	while(*p = *nullTerminatedCString++) ++p;
	return u32(p - buffer);
}



///
/// @BufferedString
///
/// A null terminated string class that writes to and reads from an external buffer.
/// Useful for doing string operations in-situ or on stack-allocated memory.
///
/// @Example usage:
///   char buffer[1024];
///   StaticBufferedString str(buffer);
///   str += "hello " + "world" + "!";
///   printf("%s", str);
///
struct StaticBufferedString
{
	char* data;
	u32 length;
	//u32 capacity;

	StaticBufferedString(char* str)
		: data(str), length(0)
	{
		data[0] = 0; // null terminate
	}

	inline char* begin() { return data; }
	inline const char* begin() const { return data; }
	inline char* end() { return data + length; }
	inline const char* end() const { return data + length; }

	inline operator char*() const { return data; }

	inline bool Empty() const { return length == 0; }
	inline void Clear() { data[0] = 0; length = 0; }

	inline StaticBufferedString& operator +=(const char* s)
	{
		length += StringCat(data + length, s);
	}

	inline StaticBufferedString& operator +=(StaticBufferedString s)
	{
		length += StringCat(data + length, s.data);
	}
};


inline StaticBufferedString operator +(StaticBufferedString a, StaticBufferedString b)
{
	a.length += StringCat(a.data + a.length, b.data);
	return a;
}

inline StaticBufferedString operator +(StaticBufferedString a, const char* b)
{
	a.length += StringCat(a.data + a.length, b);
	return a;
}

inline StaticBufferedString operator +(const char* a, StaticBufferedString b)
{
	b.length += StringCat(b.data + b.length, a);
	return b;
}
