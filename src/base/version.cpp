#include "base/version.h"
#include "base/version_header.h"
#include <string_view>

consteval bool is_digit(char c)
{
    return c <= '9' && c >= '0';
}

consteval int stoi_impl(const char* str, int value = 0)
{
    return *str ?
            is_digit(*str) ?
                stoi_impl(str + 1, (*str - '0') + value * 10)
                : value
            : value;
}

consteval int stoi(const char* str, int start)
{
    return stoi_impl(str + start);
}

consteval ZCVersion parseVersion()
{
	int components[] = {-1, -1, -1};
	int last = 0;
	int index = 0;
	int i = 0;
    for (const auto& ch : ZC_VERSION)
	{
		if (ch == '.')
		{
			components[i++] = stoi(ZC_VERSION, last);
			last = index + 1;
		}
		else if (!is_digit(ch))
		{
			break;
		}

		if (i == 3)
			break;
		index++;
    }

	if (i == 2)
		components[2] = stoi(ZC_VERSION, last);

	return {ZC_VERSION, components[0], components[1], components[2]};
}

static constexpr auto version = parseVersion();
static_assert(version.major == 3, "version not set correctly");
static_assert(version.minor != -1, "version not set correctly");
static_assert(version.patch != -1, "version not set correctly");
static_assert(sizeof(ZC_VERSION) < 40, "");

const char* getVersionString()
{
	return version.version_string;
}

ZCVersion getVersion()
{
	return version;
}

int getAlphaState()
{
	if (version.patch == 0) return 3;
	return 0;
}

bool isStableRelease()
{
	return version.patch == 0;
}
