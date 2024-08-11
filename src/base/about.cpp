#include "base/about.h"
#include "metadata/versionsig.h"

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
