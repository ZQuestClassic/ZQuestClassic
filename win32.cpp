
#ifdef _WIN32

#include <stdio.h>
#include <math.h>

#include "win32.h"


Win32Data win32data;
WNDPROC hAllegroProc = 0;


/* 
	allegro is having numurous problems on newer windows platforms
	and is likely to only get worse as time goes by, so the following 
	needs to be aggresive. -Gleeok.
*/


void Win32Data::Update(int frameskip)
{
	if(frameskip > 0)
	{
		//this hack doesn't really do anything useful.. :(
		//to get actual frameskipping we need to ..uhh, bad thoughts, nvrmd. 
		int fs = frameskip > 10 ? 10 : frameskip;
		unsigned int val = (unsigned int)ceil(1.67f * (float)fs);
		rest(val);
	}

	while(!hasFocus)
	{
		rest(17);
	}
}


// Should be called from winmain, and alleg-event threads.
int Win32Data::zqSetDefaultThreadPriority(HANDLE _thread)
{
	if(!_thread)
		_thread = ::GetCurrentThread();

	DWORD priority = ::GetThreadPriority( _thread );
	if( priority != THREAD_PRIORITY_NORMAL )
	{
		printf("Thread Priority (%ld) set to normal.\n",  priority);
		priority = THREAD_PRIORITY_NORMAL;

		if( !::SetThreadPriority(_thread, priority) )
		{
			printf("Error: Thread Priority not changed.\n");
			return 1;
		}
	}

	return 0;
}


int Win32Data::zqSetCustomCallbackProc(HWND hWnd)
{
	memset((void*)&win32data, 0, sizeof(Win32Data));
	win32data.hasFocus = true;

	win32data.hWnd = hWnd;
	hAllegroProc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
	if(!hAllegroProc)
		return -1;

	win32data.hInstance = ::GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	win32data.hId = ::GetWindowLongPtr(hWnd, GWLP_ID);

	//assign a new callback
	::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)Win32Data::zqWindowsProc);

	return 0;
}



LRESULT CALLBACK Win32Data::zqWindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool initPriority = false;

	if( !initPriority )
	{
		//sets the allegro worker threads' priority
		zqSetDefaultThreadPriority(0);
	}

	switch(uMsg)
	{
		case WM_SETFOCUS:
			{
				win32data.hasFocus = true;
			}
			break;
		case WM_KILLFOCUS:
			{
				win32data.hasFocus = false;
			}
			break;
	}

	//ship the rest for allegro to handle
	return hAllegroProc(hwnd, uMsg, wParam, lParam);
}






int Win32Data::zcSetCustomCallbackProc(HWND hWnd)
{
	memset((void*)&win32data, 0, sizeof(Win32Data));
	win32data.hasFocus = true;

	win32data.hWnd = hWnd;
	hAllegroProc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
	if(!hAllegroProc)
		return -1;

	win32data.hInstance = ::GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	win32data.hId = ::GetWindowLongPtr(hWnd, GWLP_ID);

	//assign a new callback
	::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)Win32Data::zcWindowsProc);

	return 0;
}



LRESULT CALLBACK Win32Data::zcWindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_SETFOCUS:
			{
				win32data.hasFocus = true;
			}
			break;
		case WM_KILLFOCUS:
			{
				win32data.hasFocus = false;
			}
			break;
	}

	//ship the rest for allegro to handle
	return hAllegroProc(hwnd, uMsg, wParam, lParam);
}
#endif //_WIN32
