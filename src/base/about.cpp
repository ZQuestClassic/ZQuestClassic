#include "base/about.h"
#include "metadata/versionsig.h"
#include <cstdio>
#include <cstring>

int getProgramAlphaVer()
{
	if(V_ZC_RELEASE) return V_ZC_RELEASE;
	else if(V_ZC_GAMMA) return V_ZC_GAMMA;
	else if(V_ZC_BETA) return V_ZC_BETA;
	else if(V_ZC_ALPHA) return V_ZC_ALPHA;
	return 0;
}

char const* getProgramAlphaStr(bool ignoreNightly = false)
{
	static char buf[60] = "";
	char format[20] = "%s";
	if(!ignoreNightly && ZC_IS_NIGHTLY) strcpy(format, "Nightly (%s)");
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
	if(ZC_IS_NIGHTLY)
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

char const* getProgramVerStr()
{
	static char buf[120] = "";
	if(V_ZC_FOURTH > 0)
		sprintf(buf, "%d.%d.%d.%d %s", V_ZC_FIRST, V_ZC_SECOND,
			V_ZC_THIRD, V_ZC_FOURTH, getProgramAlphaVerStr());
	else sprintf(buf, "%d.%d.%d %s", V_ZC_FIRST, V_ZC_SECOND,
			V_ZC_THIRD, getProgramAlphaVerStr());
	return buf;
}

char const* getReleaseTag()
{
#ifdef RELEASE_TAG
	return RELEASE_TAG;
#else
	return getProgramVerStr();
#endif
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
	return ".*";
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
