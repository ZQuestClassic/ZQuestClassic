
#include "Utility.h"

#include <time.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>



void SimpleTimer::Start()
{
	startTime = clock();
}


SMinuteSecondsInfo SimpleTimer::GetElapsedTime()
{
	long finishTime = clock();
	double elapsedSeconds = (finishTime - startTime) / (double)CLOCKS_PER_SEC;

	SMinuteSecondsInfo result = {
		int32(elapsedSeconds / 60.0),
		float(fmod(elapsedSeconds, 60.0))
	};

	return result;
}


u32 Strlen(const char* str)
{
	return (u32)(str ? strlen(str) : 0);
}


u32 FindFirstWhitespaceOrLE(const char* str, u32 stringLength, u32 position)
{
	for(u32 i(position); i < stringLength; ++i)
	{
		const char c = str[i];

		if( c == ' ' || c == '\t' || c == 10 || c == 13)
			return i;
	}

	return u32(-1);
}


u32 FindFirstNonWhitespaceOrLE(const char* str, u32 stringLength, u32 position)
{
	u32 i(position);
	for( ; i < stringLength; ++i)
	{
		const char c = str[i];

		if( c == ' ' || c == '\t' || c == 10 || c == 13)
			;
		else break;
	}

	return i;
}


bool SaveMemoryToFileWithMode(const char* filename, const void* data, u32 size, const char* mode)
{
	bool result = false;

	if(filename && data)
	{
		FILE* f = fopen(filename, mode);
		if(f)
		{
			fwrite(data, 1, size, f);
			fclose(f);

			result = true;
		}
		else
		{
			printf("Error, could not create or open file \"%s\".", filename);
		}
	}

	return result;
}

bool SaveMemoryToFile(const char* filename, const void* data, u32 size)
{
	return SaveMemoryToFileWithMode(filename, data, size, "wb");
}

bool SaveStringToFile(const char* filename, const char* str, u32 stringLength)
{
	return SaveMemoryToFileWithMode(filename, str, stringLength * sizeof(char), "w");
}


void* LoadFileIntoMemory(const char* filename, u32& filesize, bool addTerminatingNull /*= true*/)
{
	void* buffer = NULL;

	if(filename)
	{
		FILE* f = fopen(filename, "rb");
		if(f)
		{
			fseek(f, 0, SEEK_END);
			u32 actualFileSize = ftell(f);
			fseek(f, 0, SEEK_SET);

			u32 bufferSize = actualFileSize + (addTerminatingNull ? 1 : 0);
			buffer = malloc(bufferSize);

			if(buffer)
			{
				fread(buffer, 1, actualFileSize, f);
				fclose(f);

				if(addTerminatingNull)
					((u8*)buffer)[actualFileSize] = 0; //null terminate

				filesize = bufferSize;
			}
		}
	}

	return buffer;
}


u32 StringNormalizeLineEndingsCRLF(char* RESTRICT buffer, const char* RESTRICT str, u32 stringLength)
{
	char* RESTRICT p = buffer;
	for(u32 i(0); (*p = *str++) && (i < stringLength); ++p, ++i)
	{
		if(*p == 13)
		{
			*++p = 10;
			if(*str == 10)
			{
				++str;
				++i;
			}
		}
		else if(*p == 10)
		{
			*p = 13;
			*++p = 10;
		}
	}

	return u32(p - buffer);
}






