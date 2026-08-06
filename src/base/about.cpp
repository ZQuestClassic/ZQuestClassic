#include "base/about.h"

// Ex: mac, mac-universal, linux, windows-x86, windows-x64
char const* getReleasePlatform()
{
#ifdef RELEASE_PLATFORM
	return RELEASE_PLATFORM;
#else
	return "none";
#endif
}

// The architecture this binary is running as. For a universal mac binary,
// each slice compiles its own value, so this reports the slice the OS
// actually loaded.
char const* getRunningArch()
{
#if defined(__EMSCRIPTEN__)
	return "wasm";
#elif defined(__aarch64__) || defined(_M_ARM64)
	return "arm64";
#elif defined(__x86_64__) || defined(_M_X64)
	return "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
	return "x86";
#else
	return "unknown";
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
