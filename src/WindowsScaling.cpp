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
	auto activeWindow = GetActiveWindow();
	HMONITOR hMonitor = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);
	DEVICE_SCALE_FACTOR devScaleFactor;
	HMODULE shcore_dll = _al_win_safe_load_library("shcore.dll");
	if (shcore_dll) {
		typedef HRESULT (WINAPI *GetScaleFactorForMonitorPROC)(HMONITOR, DEVICE_SCALE_FACTOR*);
		GetScaleFactorForMonitorPROC imp_GetScaleFactorForMonitor =
			(GetScaleFactorForMonitorPROC)GetProcAddress(shcore_dll, "GetScaleFactorForMonitor");
		if (imp_GetScaleFactorForMonitor) {
			/* Try setting the per-monitor awareness first. It might fail on Win 8.1. */
			imp_GetScaleFactorForMonitor(hMonitor, &devScaleFactor);
		}
		else return 1.0;
		FreeLibrary(shcore_dll);
	}
	else return 1.0;

	// Calculate the scaling factor
	auto horizontalScale = devScaleFactor/100.0;
	
	//al_trace(horizontalScale);

	return horizontalScale;
#else
	return 1.0;
#endif
}

double getverticalscale()
{
#ifdef _WIN32
	auto activeWindow = GetActiveWindow();
	HMONITOR hMonitor = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);
	DEVICE_SCALE_FACTOR devScaleFactor;
	HMODULE shcore_dll = _al_win_safe_load_library("shcore.dll");
	if (shcore_dll) {
		typedef HRESULT (WINAPI *GetScaleFactorForMonitorPROC)(HMONITOR, DEVICE_SCALE_FACTOR*);
		GetScaleFactorForMonitorPROC imp_GetScaleFactorForMonitor =
			(GetScaleFactorForMonitorPROC)GetProcAddress(shcore_dll, "GetScaleFactorForMonitor");
		if (imp_GetScaleFactorForMonitor) {
			/* Try setting the per-monitor awareness first. It might fail on Win 8.1. */
			imp_GetScaleFactorForMonitor(hMonitor, &devScaleFactor);
		}
		else return 1.0;
		FreeLibrary(shcore_dll);
	}
	else return 1.0;


	// Calculate the scaling factor
	auto verticalScale = devScaleFactor/100.0;
	
	//al_trace(horizontalScale);

	return verticalScale;
#else
	return 1.0;
#endif
}







