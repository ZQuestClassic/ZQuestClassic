#ifndef BASE_VERSION_H
#define BASE_VERSION_H

struct ZCVersion {
	const char* version_string;
	int major, minor, patch;
};

const char* getVersionString();
ZCVersion getVersion();
int getAlphaState();
bool isStableRelease();

#endif
