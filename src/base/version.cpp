#include "base/version.h"
#include "base/about.h"
#include "metadata/versionsig.h"

static ZCVersion version = {getReleaseTag(), V_ZC_FIRST, V_ZC_SECOND, V_ZC_THIRD};

const char* getVersionString()
{
	return version.version_string;
}

ZCVersion getVersion()
{
	return version;
}
