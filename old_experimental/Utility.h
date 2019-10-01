
#pragma once

#include "Config.h"


struct SMinuteSecondsInfo
{
	int32 minutes;
	float seconds;

	SMinuteSecondsInfo operator +(SMinuteSecondsInfo msi);
};

struct SimpleTimer
{
	long startTime;

	void Start();
	SMinuteSecondsInfo GetElapsedTime();
};


u32 Strlen(const char* str); // Safe for NULL pointers.

u32 FindFirstWhitespaceOrLE(const char* str, u32 stringLength, u32 position);
u32 FindFirstNonWhitespaceOrLE(const char* str, u32 stringLength, u32 position);

bool SaveMemoryToFileWithMode(const char* filename, const void* data, u32 size, const char* mode);
bool SaveMemoryToFile(const char* filename, const void* data, u32 size);
bool SaveStringToFile(const char* filename, const char* str, u32 stringLength);

void* LoadFileIntoMemory(const char* filename, u32& filesize, bool addTerminatingNull = true);




u32 StringNormalizeLineEndingsCRLF(char* RESTRICT buffer, const char* RESTRICT str, u32 stringLength);



u32 Itoa(u32 value, char* bufptr);
u32 Itoa(int32 value, char* bufptr);
u32 Dtoa(double value, char* bufptr);
inline u32 Ftoa(float value, char* bufptr) { return Dtoa((double)value, bufptr); }



//
void VPrintf(char const* fmt, void* va);
int VSprintf(char* buf, char const* fmt, void* va);
int VSnprintf(char* buf, int count, char const* fmt, void* va);
void Printf(char const* fmt, ...);
int Sprintf(char* buf, char const* fmt, ...);
int Snprintf(char* buf, int count, char const* fmt, ...);

