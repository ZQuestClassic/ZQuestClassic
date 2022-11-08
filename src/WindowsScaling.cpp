#ifdef _WIN32
#include "base/zdefs.h"
#include <windows.h>
#include <iostream>
#include <shellscalingapi.h>
#include <allegro5/platform/aintwin.h>
#endif

#include "WindowsScaling.h"

double gethorizontalscale()
{
#ifdef _WIN32
	HDC screen = GetDC(NULL);
	int32_t dpiX = GetDeviceCaps (screen, LOGPIXELSX);
	ReleaseDC(NULL, GetActiveWindow());
	double scalingFactor = dpiX / 96.0;
	return scalingFactor;
#else
	return 1.0;
#endif
}

double getverticalscale()
{
#ifdef _WIN32
	HDC screen = GetDC(NULL);
	int32_t dpiY = GetDeviceCaps (screen, LOGPIXELSY);
	ReleaseDC(NULL, GetActiveWindow());
	double scalingFactor = dpiY / 96.0;
	return scalingFactor;
#else
	return 1.0;
#endif
}







