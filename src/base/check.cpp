#include "base/check.h"
#include "base/zapp.h"

#include <fmt/format.h>

#include <cstdio>
#include <string>

namespace {

void DefaultCheckHandler(const char* file, int line, const char* func, const char* condition)
{
	std::string message = fmt::format("CHECK failed at {}:{} {}: {}", file, line, func, condition);
	fprintf(stderr, "%s\n", message.c_str());
	zapp_reporting_add_breadcrumb("error", message.c_str());
}

CheckFailedHandler g_check_handler = DefaultCheckHandler;

}

void SetCheckFailedHandler(CheckFailedHandler handler)
{
	g_check_handler = handler;
}

void HandleCheckError(const char* file, int line, const char* func, const char* condition)
{
	if (g_check_handler)
		g_check_handler(file, line, func, condition);
}
