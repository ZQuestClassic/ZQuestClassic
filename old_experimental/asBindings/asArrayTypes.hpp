// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// This file is MIT licensed.

#pragma once

#include "ScriptCommon.h"
#include "Array.h"
#include "ZCString.h"


template<class T>
class ScriptArray : public Array<T>
{
public:
	typedef Array<T> BaseType;

	ScriptArray()
		: BaseType(), m_refCount(1)
	{
	}

	ScriptArray(int size)
		: BaseType(), m_refCount(1)
	{
		Allocate(size);
	}

	ScriptArray(const ScriptArray& other)
		: BaseType(other.data, other.count), m_refCount(1)
	{
	}

	ScriptArray& operator =(const ScriptArray& array)
	{
		BaseType::Clear();
		BaseType::InsertRange(0, array.data, array.count);
		return *this;
	}

	bool OutOfBounds(int index)
	{
		if((u32)index >= BaseType::Size())
		{
			asIScriptContext* context = asGetActiveContext();
			if(context)
				context->SetException("Array index out of bounds.");

			return false;
		}

		return true;
	}

	void Add(const T& val)
	{
		BaseType::Add(val);
	}

	void RemoveFromEnd()
	{
		if(count)
			BaseType::RemoveFromEnd();
	}

	void PushFront(const T& value)
	{
		BaseType::Insert(0, value);
	}

	void PopFront()
	{
		if(count)
			BaseType::Remove(0);
	}

	void Insert(int position, const T& value)
	{
		if(OutOfBounds(position))
			return;

		BaseType::Insert((u32)position, value);
	}

	void Remove(int position)
	{
		if(OutOfBounds(position))
			return;

		BaseType::Remove((u32)position);
	}

	void RemoveRange(int position, int n)
	{
		if(OutOfBounds(position))
			return;

		BaseType::RemoveRange(position, n);
	}

	T* At(int index)
	{
		if(OutOfBounds(index))
			return 0;

		return &BaseType::operator[](index);
	}

	void Reserve(int newCapacity)
	{
		if((u32)newCapacity < 4096)
			BaseType::Reserve((u32)newCapacity);
	}

	void Resize(int newSize)
	{
		//todo
	}

	void Clear()
	{
		BaseType::Clear();
	}

	bool Empty()
	{
		return BaseType::Empty();
	}

	int Size()
	{
		return BaseType::Size();
	}

	// GC behaviors
	void AddRef()
	{
		++m_refCount;
	}

	void Release()
	{
		if(--m_refCount <= 0)
		{
			delete this;
		}
	}

protected:
	int m_refCount;

};


namespace ScriptArrayType
{
	// TODO: Factories for script objects should use a custom allocator instead of new.

	template <class T>
	ScriptArray<T>* ScriptArrayFactory()
	{
		ScriptArray<T> *ptr = new ScriptArray<T>();
		return ptr;
	}

	template <class T>
	ScriptArray<T>* ScriptArrayFactoryReserve(int x)
	{
		ScriptArray<T> *ptr = new ScriptArray<T>(x);
		return ptr;
	}

	template <class T>
	ScriptArray<T>* ScriptArrayFactoryCopy(const ScriptArray<T>& v)
	{
		ScriptArray<T> *ptr = new ScriptArray<T>(v);
		return ptr;
	}

} // namespace ScriptArrayType


void ScriptRegistrar::RegisterArrayTemplateSpecializations(asIScriptEngine* engine)
{
	// Default template array must be registered first.

	RegisterArraySpecialization<int8>(engine, "Array<int8>", "int8", "int8");
	RegisterArraySpecialization<int16>(engine, "Array<int16>", "int16", "int16");
	RegisterArraySpecialization<int32>(engine, "Array<int>", "int", "int");
	RegisterArraySpecialization<int64>(engine, "Array<int64>", "int64", "int64");
	RegisterArraySpecialization<u8>(engine, "Array<uint8>", "uint8", "uint8");
	RegisterArraySpecialization<u16>(engine, "Array<uint16>", "uint16", "uint16");
	RegisterArraySpecialization<u32>(engine, "Array<uint>", "uint", "uint");
	RegisterArraySpecialization<u64>(engine, "Array<uint64>", "uint64", "uint64");
	RegisterArraySpecialization<float>(engine, "Array<float>", "float", "float");
	//RegisterArraySpecialization<float64>(engine, "Array<float64>", "float64");
	//RegisterArraySpecialization<Vector2i>(engine, "Array<vec2i>", "vec2i", "vec2i");
	//RegisterArraySpecialization<Vector2f>(engine, "Array<vec2>", "vec2", "vec2");
	//RegisterArraySpecialization<Rect>(engine, "Array<Rect>", "Rect", "const Rect& in");
}


template <class T>
void ScriptRegistrar::RegisterArraySpecialization(asIScriptEngine* engine, const char* decl, const char* type, const char* paramDecl)
{
	using namespace ScriptArrayType;
	int r;

	char buf[96];
	char declBuffer[96];
	char typeBuffer[96];

	StaticBufferedString d = StaticBufferedString(declBuffer) + decl;
	StaticBufferedString t = StaticBufferedString(typeBuffer) + type;


	// Register the array type
	r = engine->RegisterObjectType(decl, 0, asOBJ_REF); Assert(r >= 0);

	// Constructors
	r = engine->RegisterObjectBehaviour(decl, asBEHAVE_FACTORY,
		d + " @f()", 
		asFUNCTION(ScriptArrayFactory<T>), asCALL_CDECL);
	Assert(r >= 0);
	
	r = engine->RegisterObjectBehaviour(decl, asBEHAVE_FACTORY,
		d + " @f(" + paramDecl + ")",
		asFUNCTION(ScriptArrayFactoryReserve<T>), asCALL_CDECL);
	Assert(r >= 0);

	r = engine->RegisterObjectBehaviour(decl, asBEHAVE_FACTORY,
		d + " @f(" + decl + " @)",
		asFUNCTION(ScriptArrayFactoryCopy<T>), asCALL_CDECL);
	Assert(r >= 0);

	// operator []
	r = engine->RegisterObjectMethod(decl,
		t + " opIndex(int) const",
		asMETHOD(ScriptArray<T>, At), asCALL_THISCALL);
	Assert(r >= 0);

	r = engine->RegisterObjectMethod(decl,
		t + "& opIndex(int)",
		asMETHOD(ScriptArray<T>, At), asCALL_THISCALL);
	Assert(r >= 0);

	// operator =
	r = engine->RegisterObjectMethod(decl,
		d + "& opAssign(const " + paramDecl + ")",
		asMETHOD(ScriptArray<T>, operator=), asCALL_THISCALL);
	Assert(r >= 0);

	// Resize
	r = engine->RegisterObjectMethod(decl,
		"void Resize(int)",
		asMETHOD(ScriptArray<T>, Resize), asCALL_THISCALL);
	Assert(r >= 0);

	// Empty
	r = engine->RegisterObjectMethod(decl,
		"bool Empty() const",
		asMETHOD(ScriptArray<T>, Empty), asCALL_THISCALL);
	Assert(r >= 0);

	// Size
	r = engine->RegisterObjectMethod(decl,
		"int Size() const",
		asMETHOD(ScriptArray<T>, Size), asCALL_THISCALL);
	Assert(r >= 0);

	// Add
	r = engine->RegisterObjectMethod(decl,
		StaticBufferedString(buf) + "void Add(" + paramDecl + ")",
		asMETHOD(ScriptArray<T>, Add), asCALL_THISCALL);
	Assert(r >= 0);

	// RemoveFromEnd
	r = engine->RegisterObjectMethod(decl,
		"void RemoveFromEnd()",
		asMETHOD(ScriptArray<T>, RemoveFromEnd), asCALL_THISCALL);
	Assert(r >= 0);

	// Insert
	r = engine->RegisterObjectMethod(decl,
		StaticBufferedString(buf) + "void Insert(int," + paramDecl + ")",
		asMETHOD(ScriptArray<T>, Insert), asCALL_THISCALL);
	Assert(r >= 0);

	//r = engine->RegisterObjectMethod(decl,
	//	StaticBufferedString(buf) + "void Insert(int, int," + paramDecl + ")",
	//	asMETHOD(ScriptArray<T>, InsertRange), asCALL_THISCALL);
	//Assert(r >= 0);

	// Remove
	r = engine->RegisterObjectMethod(decl,
		"void Remove(int)",
		asMETHOD(ScriptArray<T>, Remove), asCALL_THISCALL);
	Assert(r >= 0);

	// RemoveRange
	r = engine->RegisterObjectMethod(decl,
		"void RemoveRange(int, int)",
		asMETHOD(ScriptArray<T>, RemoveRange), asCALL_THISCALL);
	Assert(r >= 0);

	// Clear
	r = engine->RegisterObjectMethod(decl,
		"void Clear()",
		asMETHOD(ScriptArray<T>, Clear), asCALL_THISCALL);
	Assert(r >= 0);

	// Memory management
	r = engine->RegisterObjectBehaviour(decl, asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptArray<T>, AddRef), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour(decl, asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptArray<T>, Release), asCALL_THISCALL); Assert(r >= 0);
}

