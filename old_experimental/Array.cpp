
#include "Array.h"

#include <string.h>
#include <stdlib.h>

#define arrayAllocFunction malloc
#define arrayReallocFunction realloc
#define arrayFreeFunction free



void ArrayCopy(void* dst, const void* source, u32 objectSize, u32 arrayCount)
{
	memcpy(dst, source, arrayCount * objectSize);
}


void ArrayBaseProxy::Allocate(u32 maxCapacity, u32 objectSize)
{
	Assert(data == NULL);

	data = (u8*)arrayAllocFunction(maxCapacity * objectSize);
	count = 0;
	capacity = maxCapacity;
}


void ArrayBaseProxy::Reallocate(u32 maxCapacity, u32 objectSize)
{
	data = (u8*)arrayReallocFunction(data, maxCapacity * objectSize);

	if(maxCapacity < count)
		count = maxCapacity;

	capacity = maxCapacity;
}


void ArrayBaseProxy::Free()
{
	if(data)
	{
		arrayFreeFunction(data);
		data = NULL;
		count = 0;
		capacity = 0;
	}
	else
	{
		Assert(count == 0);
		Assert(capacity == 0);
	}
}


void ArrayBaseProxy::InsertHole(u32 position, u32 objectSize, u32 holeCount)
{
	Assert(position + holeCount <= capacity);
	Assert(position <= count);

	if(position != count)
	{
		u8* dest = data + (position * objectSize);
		memmove(dest + (holeCount * objectSize), dest, (count - position) * objectSize);
	}

	count += holeCount;
}


void ArrayBaseProxy::InsertRange(u32 position, u32 objectSize, const void* p, u32 numElements)
{
	InsertHole(position, objectSize, numElements);
	memcpy(data + (position * objectSize), p, numElements * objectSize);
}


void ArrayBaseProxy::RemoveRange(u32 position, u32 objectSize, u32 numElements)
{
	Assert(count >= (position + numElements));

	if(position + numElements < count)
	{
		u8* dest = data + (position * objectSize);
		u32 bytesToMove = (count - (position + numElements)) * objectSize;

		memcpy(dest, dest + (objectSize * numElements), bytesToMove);
	}

	count -= numElements;
}


void ArrayBaseProxy::EnsureCapacity(u32 objectSize, u32 numElements)
{
	if(!data)
		Allocate(numElements, objectSize);

	else
	{
		if(count + numElements > capacity)
			Reallocate(count + numElements, objectSize);
	}
}


void ArrayBaseProxy::InsertHoleOrReallocate(u32 position, u32 objectSize, u32 holeCount)
{
	EnsureCapacity(objectSize, holeCount);
	InsertHole(position, objectSize, holeCount);
}


void ArrayBaseProxy::InsertRangeOrReallocate(u32 position, u32 objectSize, const void* p, u32 numElements)
{
	EnsureCapacity(objectSize, numElements);
	InsertRange(position, objectSize, p, numElements);
}






