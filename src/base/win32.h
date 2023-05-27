
#pragma once

#ifdef _WIN32


#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include "base/zc_alleg.h"
#include <winalleg.h>
#include <allegro5/allegro_windows.h>



class Win32Data
{
public:
    volatile HWND hWnd;
    volatile LONG_PTR hInstance;
    volatile LONG_PTR hId;
    
    static LRESULT CALLBACK zqWindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static int32_t zqSetDefaultThreadPriority(HANDLE _thread);
    static int32_t zqSetCustomCallbackProc(HWND hWnd);
    
    static LRESULT CALLBACK zcWindowsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static int32_t zcSetCustomCallbackProc(HWND hWnd);
    
    volatile bool hasFocus;
    volatile bool isValid;
    
    //call from main thread.
    void Update(int32_t frameskip);
};
extern Win32Data win32data;



class Win32Mutex
{
public:
    Win32Mutex(bool lock = false) : m_locked(false)
    {
        ::InitializeCriticalSection(&m_criticalSection);
        
        if(lock)
            Lock();
    }
    
    ~Win32Mutex()
    {
        Unlock();
        ::DeleteCriticalSection(&m_criticalSection);
    }
    
    void Lock()
    {
        if(!m_locked)
        {
            m_locked = true;
            ::EnterCriticalSection(&m_criticalSection);
        }
    }
    
    void Unlock()
    {
        if(m_locked)
        {
            m_locked = false;
            ::LeaveCriticalSection(&m_criticalSection);
        }
    }
    
protected:
    CRITICAL_SECTION m_criticalSection;
    bool m_locked;
};













#endif //_WIN32
