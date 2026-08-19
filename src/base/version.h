#ifndef BASE_VERSION_H_
#define BASE_VERSION_H_

struct ZCVersion {
	const char* version_string;
	int major, minor, patch;
};

const char* getVersionString();
ZCVersion getVersion();
int getAlphaState();
bool isStableRelease();

#endif
