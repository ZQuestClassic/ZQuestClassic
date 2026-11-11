#pragma once

struct ZCVersion {
	const char* version_string;
	int major, minor, patch;
};

const char* getVersionString();
ZCVersion getVersion();
int getAlphaState();
bool isStableRelease();
