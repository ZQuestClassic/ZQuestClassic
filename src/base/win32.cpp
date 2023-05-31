

#ifdef _WIN32

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "base/win32.h"


bool is_only_instance(const char* name)
{
    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, name);
	
    if (!hMutex)
	{
		hMutex = CreateMutex(0, 0, name);
		return true;
	}
    else
		return false;
}


Win32Data win32data;
WNDPROC hAllegroProc = 0;


#define FATAL_ERROR(message) \
	set_gfx_mode(GFX_TEXT,0,0,0,0); \
	allegro_message(message); \
	exit(1)


/*
	These are unfinished, though the windows stuff is here if it is needed. -Gleeok.
	//todo* The return values and pointers NEED to be checked.
*/


#define Z_WIN32NOUPDATE


void Win32Data::Update(int32_t frameskip)
{
    if(!win32data.isValid)
        return;
        
    if(frameskip > 0)
    {
    }
    
    while(!win32data.hasFocus)
    {
        rest(17);
    }
}


// Should be called from winmain, and alleg-event threads.
int32_t Win32Data::zqSetDefaultThreadPriority(HANDLE _thread)
{
    (void*)_thread;
    /*
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
    */
    
    return 0;
}


int32_t Win32Data::zqSetCustomCallbackProc(HWND hWnd)
{
    Win32Mutex mutex;
    mutex.Lock();
    
    memset((void*)&win32data, 0, sizeof(Win32Data));
    win32data.hasFocus = true;
    
    win32data.hWnd = hWnd;
    
    if(win32data.hWnd)
        win32data.isValid = true;
    else return -1;
    
    hAllegroProc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    
    if(!hAllegroProc)
    {
        win32data.isValid = false;
        return -1;
    }
    
    win32data.hInstance = ::GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
    
    if(!win32data.hInstance)
    {
        win32data.isValid = false;
        return -1;
    }
    
    win32data.hId = ::GetWindowLongPtr(hWnd, GWLP_ID);
    
    //assign a new callback
    LONG_PTR pcb = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)Win32Data::zqWindowsProc);
    
    if(!pcb)
    {
        al_trace("Error: Cannot Set Windows Procedure.");
        win32data.isValid = false;
        return -1;
    }
    
    return 0;
}



LRESULT CALLBACK Win32Data::zqWindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    assert(win32data.isValid && "ZQuest-Windows Fatal Error: Set \"zq_win_proc_fix = 0\" in config file.");
    
    static bool initPriority = false;
    
    if(!initPriority)
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






int32_t Win32Data::zcSetCustomCallbackProc(HWND hWnd)
{
    Win32Mutex mutex;
    mutex.Lock();
    
    memset((void*)&win32data, 0, sizeof(Win32Data));
    win32data.hasFocus = true;
    
    win32data.hWnd = hWnd;
    
    if(win32data.hWnd)
        win32data.isValid = true;
    else return -1;
    
    hAllegroProc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    
    if(!hAllegroProc)
    {
        win32data.isValid = false;
        return -1;
    }
    
    win32data.hInstance = ::GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
    
    if(!win32data.hInstance)
    {
        win32data.isValid = false;
        return -1;
    }
    
    win32data.hId = ::GetWindowLongPtr(hWnd, GWLP_ID);
    
    //assign a new callback
    LONG_PTR pcb = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)Win32Data::zcWindowsProc);
    
    if(!pcb)
    {
        al_trace("Error: Cannot Set Windows Procedure.");
        win32data.isValid = false;
        return -1;
    }
    
    return 0;
}



LRESULT CALLBACK Win32Data::zcWindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    assert(win32data.isValid && "ZC-Win32 Fatal Error: Set \"zc_win_proc_fix = 0\" in config file.");
    
    /*
    HWND tmp_hwnd = win_get_window();
    if( win32data.hWnd != hwnd || win32data.hWnd != tmp_hwnd )
    {
    	win32data.hWnd = hwnd; //happens at shutdown
    }
    */
    
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
	    
		case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
				// *Don't let the monitor enter screen-save mode.*
				// -this is actually an annoying bug on my laptop: what happens is if allegro
				// is running and we go into powersave, (joystick input is not considered activity),
				// the damn thing locks up sometimes and I have to enter stand-by mode... :/ -Gleeok
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				//fall through
				return 0;
			}
	        
			break;
		}
    }
    
    //ship the rest for allegro to handle
    LRESULT result = hAllegroProc(hwnd, uMsg, wParam, lParam);
    
    return result;
}


#endif //_WIN32
