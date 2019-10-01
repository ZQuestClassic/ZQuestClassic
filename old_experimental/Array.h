
#pragma once

#include "Config.h"


void ArrayCopy(void* dst, const void* source, u32 objectSize, u32 arrayCount);

template <class T> inline
u32 ArrayFind(const T* data, u32 arrayCount, const T& value)
{
	u32 i = 0;
	for( ; i != arrayCount; ++i)
	{
		if(data[i] == value)
			break;
	}

	return i;
}


template <class T> inline
void ArrayFill(T* data, u32 arrayCount, const T& value)
{
	for(u32 i(0); i != arrayCount; ++i)
		data[i] = value;
}


template <class T> inline
void ArrayConstruct(T* data, u32 arrayCount)
{
	for(u32 i(0); i != arrayCount; ++i)
		new (data + i) T();
}


template <class T, class U> inline
void ArrayConstructAs(T* data, u32 arrayCount, const U& value)
{
	for(u32 i(0); i != arrayCount; ++i)
		new (data + i) T(value);
}


template <class T> inline
void ArrayDestruct(T* data, u32 arrayCount)
{
	while(arrayCount)
		data[--arrayCount].~T();
}


/// @ArrayBaseProxy
/// The actual array class that does all the work.
///
struct ArrayBaseProxy
{
	u8* data;
	u32 count;
	u32 capacity;

	void Allocate(u32 maxCapacity, u32 objectSize);
	void Reallocate(u32 maxCapacity, u32 objectSize);
	void Free();

	void InsertHole(u32 position, u32 objectSize, u32 holeCount);
	void InsertRange(u32 position, u32 objectSize, const void* p, u32 numElements);
	void RemoveRange(u32 position, u32 objectSize, u32 numElements);

	void EnsureCapacity(u32 objectSize, u32 numElements);
	void InsertHoleOrReallocate(u32 position, u32 objectSize, u32 holeCount);
	void InsertRangeOrReallocate(u32 position, u32 objectSize, const void* p, u32 numElements);

};


///
/// @PODArray
/// Simple POD array that only operates on POD types. Designed to not cripple compile and link times,
/// and be faster and generate much smaller machine code than std::vector<>.
///
/// Differences between PODArray and c++ vector<>:
/// 1) PODArray will never resize, allocate, reallocate, or copy memory behind your back. You
/// must manually call Allocate(), Reallocate(), and Free().
/// 2) To deep-copy a PODArray you must explicitly call PODArray.Copy();
///
template <class T>
struct Array
{
	typedef T			ElementType;
	typedef T*			Iterator;
	typedef const T*	ConstIterator;

	T* data;
	u32 count;
	u32 capacity;

	void Allocate(u32 maxCapacity)
	{
		((ArrayBaseProxy*)this)->Allocate(maxCapacity, sizeof(T));
	}

	void Reallocate(u32 maxCapacity)
	{
		((ArrayBaseProxy*)this)->Reallocate(maxCapacity, sizeof(T));
	}

	void Free()
	{
		((ArrayBaseProxy*)this)->Free();
	}

	inline Array()
		: data(NULL), count(0), capacity(0)
	{}

	explicit Array(u32 numElements, const T& value = T())
	{
		((ArrayBaseProxy*)this)->Allocate(numElements, sizeof(T));
		Fill(0, numElements, value);
	}

	explicit Array(T* ptr, u32 numElements)
	{
		((ArrayBaseProxy*)this)->Allocate(numElements, sizeof(T));
		ArrayCopy(data, ptr, sizeof(T), numElements);
		count = numElements;
	}

	inline u32 Size() const { return count; }
	inline u32 Capacity() const { return capacity; }
	inline void Clear() { count = 0; }
	inline bool Empty() const { return count == 0; }
	inline bool Full() const { return count == capacity; }

	inline Iterator begin() { return data; }
	inline Iterator end() { return data + count; }
	inline ConstIterator begin() const { return data; }
	inline ConstIterator end() const { return data + count; }

	inline ElementType& operator[](u32 index) { return data[index]; }
	inline ElementType& operator[](u32 index) const { return data[index]; }

	inline void Reserve(u32 newCapacity)
	{
		if(newCapacity > capacity)
			Reallocate(newCapacity);
	}

	inline void Add(const T& value)
	{
		if(UNLIKELY(count == capacity))
			Reallocate(capacity * 2);

		data[count++] = value;
	}

	inline void AddUnchecked(const T& value)
	{
		data[count++] = value;
	}

	inline void Insert(u32 position, const T& value)
	{
		((ArrayBaseProxy*)this)->InsertHoleOrReallocate(position, sizeof(T), 1);
		data[position] = value;
	}

	inline Iterator InsertHole(u32 position, u32 numElements)
	{
		((ArrayBaseProxy*)this)->InsertHoleOrReallocate(position, sizeof(T), numElements);
		return data + position;
	}

	inline void InsertRange(u32 position, const T* p, u32 numElements)
	{
		((ArrayBaseProxy*)this)->InsertRangeOrReallocate(position, sizeof(T), p, numElements);
	}

	inline void RemoveFromEnd()
	{
		Assert(count != 0);
		--count;
	}

	inline void Remove(u32 position)
	{
		((ArrayBaseProxy*)this)->RemoveRange(position, sizeof(T), 1);
	}

	inline void RemoveRange(u32 position, u32 numElements)
	{
		((ArrayBaseProxy*)this)->RemoveRange(position, sizeof(T), numElements);
	}

	// Swaps position with the element at the end of the array.
	void RemoveUnordered(u32 position)
	{
		ASSERT(count != 0 && position < count);

		--count;
		if(position != count)
			data[position] = data[count];
	}

	u32 RemoveValue(const T& value)
	{
		u32 position = ArrayFind(data, count, value);
		if(position < count)
			Remove(position);

		return position;
	}

	inline void Fill(u32 position, u32 numElements, const T& value)
	{
		for(u32 i(0); i != count; ++i)
			data[i] = value;
	}

	inline u32 Find(const T& value)
	{
		return ArrayFind(data, count, value);
	}

	inline bool Contains(const T& value)
	{
		return ArrayFind(data, count, value) != count;
	}

	inline Array Copy()
	{
		return Array(data, count);
	}
};





