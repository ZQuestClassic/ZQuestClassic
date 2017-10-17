
#pragma once

#ifdef _WIN32


#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <allegro.h>
#include <winalleg.h>



class Win32Data
{
public:
	HWND hWnd;
	LONG_PTR hInstance;
	LONG_PTR hId;

	static LRESULT CALLBACK zqWindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static int zqSetDefaultThreadPriority(HANDLE _thread);
	static int zqSetCustomCallbackProc(HWND hWnd);

	static LRESULT CALLBACK zcWindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static int zcSetCustomCallbackProc(HWND hWnd);

	bool hasFocus;

	//call from main thread.
	void Update(int frameskip);
};
extern Win32Data win32data;



#endif //_WIN32
