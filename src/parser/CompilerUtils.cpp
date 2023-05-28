#include "CompilerUtils.h"

#include <cstdarg>
#include <cstdio>
#include <string>

std::string XTableHelper::formatStr(std::string const* format, ...)
{
	static char buffer[formatBufferSize];
	
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format->c_str(), args);
	va_end(args);

	return std::string(buffer);
}

