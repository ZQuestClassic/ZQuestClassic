#include "base/about.h"

// Ex: mac, linux, windows-x86, windows-x64
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
