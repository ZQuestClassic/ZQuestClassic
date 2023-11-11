#include "base/about.h"
#include "base/version.h"
#include "metadata/versionsig.h"
#include <cstdio>
#include <cstring>

int getProgramAlphaVer()
{
	return getVersion().minor;
}

char const* getProgramAlphaStr(bool ignoreNightly = false)
{
	static char buf[60] = "";
	char format[20] = "%s";
	if(!ignoreNightly && !isStableRelease()) strcpy(format, "Nightly (%s)");
	if(V_ZC_RELEASE) sprintf(buf, format, "Release");
	else if(V_ZC_GAMMA) sprintf(buf, format, "Gamma");
	else if(V_ZC_BETA) sprintf(buf, format, "Beta");
	else if(V_ZC_ALPHA) sprintf(buf, format, "Alpha");
	else sprintf(buf, format, "Unknown");
	return buf;
}

char const* getProgramAlphaVerStr()
{
	static char buf[100] = "";
	if(!isStableRelease())
	{
		if(getProgramAlphaVer() < 0)
			sprintf(buf, "Nightly (%s ?\?)", getProgramAlphaStr(true));
		else sprintf(buf, "Nightly (%s %d/%d)", getProgramAlphaStr(true), getProgramAlphaVer()-1, getProgramAlphaVer());
	}
	else
	{
		if(getProgramAlphaVer() < 0)
			sprintf(buf, "%s ?\?", getProgramAlphaStr(true));
		else sprintf(buf, "%s %d", getProgramAlphaStr(true), getProgramAlphaVer());
	}
	return buf;
}

char const* getReleasePlatform()
{
#ifdef RELEASE_PLATFORM
	return RELEASE_PLATFORM;
#else
	return "none";
#endif
}

char const* getReleaseChannel()
{
#ifdef RELEASE_CHANNEL
	return RELEASE_CHANNEL;
#else
	return "latest";
#endif
}

char const* getRepo()
{
#ifdef REPO
	return REPO;
#else
	return "ZQuestClassic/ZQuestClassic";
#endif
}
