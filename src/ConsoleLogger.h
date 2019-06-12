// ConsoleLogger.h: interface for the CConsoleLogger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONSOLELOGGER_H__294FDF9B_F91E_4F6A_A953_700181DD1996__INCLUDED_)
#define AFX_CONSOLELOGGER_H__294FDF9B_F91E_4F6A_A953_700181DD1996__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "windows.h"
#include <time.h>
#include <stdio.h>
#include "stdlib.h"
#include <fcntl.h>
#include "io.h"
#include "direct.h"


#include "ntverp.h"
#if !defined(VER_PRODUCTBUILD) ||  VER_PRODUCTBUILD<3790
#pragma message ("********************************************************************************************")
#pragma message ("Notice (performance-warning): you are not using the Microsoft Platform SDK,")
#pragma message ("                              we'll use CRITICAL_SECTION instead of InterLocked operation")
#pragma message ("********************************************************************************************")
#else
#define CONSOLE_LOGGER_USING_MS_SDK
#endif


// If no "helper_executable" location was specify -
// search for the DEFAULT_HELPER_EXE 
#define DEFAULT_HELPER_EXE	"ConsoleLoggerHelper.exe"


class CConsoleLogger
{
public:

	// ctor,dtor
	CConsoleLogger();
	virtual ~CConsoleLogger();
	
	// create a logger: starts a pipe+create the child process
	long Create(const char *lpszWindowTitle=NULL,
				int buffer_size_x=-1,int buffer_size_y=-1,
				const char *logger_name=NULL,
				const char *helper_executable=NULL);

	// close everything
	long Close(void);
	
	// output functions
	inline int print(const char *lpszText,int iSize=-1);
	int printf(const char *format,...);
	
	// play with the CRT output functions
	int SetAsDefaultOutput(void);
	static int ResetDefaultOutput(void);

protected:
	char	m_name[64];
	HANDLE	m_hPipe;
	
#ifdef CONSOLE_LOGGER_USING_MS_SDK
	// we'll use this DWORD as VERY fast critical-section . for more info:
	// * "Understand the Impact of Low-Lock Techniques in Multithreaded Apps"
	//		Vance Morrison , MSDN Magazine  October 2005
	// * "Performance-Conscious Thread Synchronization" , Jeffrey Richter , MSDN Magazine  October 2005
	volatile long m_fast_critical_section;

	inline void InitializeCriticalSection(void)
	{ m_fast_critical_section=0; }
	
	inline void DeleteCriticalSection(void)
	{ m_fast_critical_section=0; }

	// our own LOCK function
	inline void EnterCriticalSection(void)
	{ while ( InterlockedCompareExchange(&m_fast_critical_section,1,0)!=0) Sleep(0); }

	// our own UNLOCK function
	inline void LeaveCriticalSection(void)
	{ m_fast_critical_section=0; }

#else
	CRITICAL_SECTION	m_cs;
	inline void InitializeCriticalSection(void)
	{ ::InitializeCriticalSection(&m_cs); }
	
	inline void DeleteCriticalSection(void)
	{ ::DeleteCriticalSection(&m_cs); }

	// our own LOCK function
	inline void EnterCriticalSection(void)
	{ ::EnterCriticalSection(&m_cs); }

	// our own UNLOCK function
	inline void LeaveCriticalSection(void)
	{ ::LeaveCriticalSection(&m_cs); }

#endif

	// you can extend this class by overriding the function
	virtual long	AddHeaders(void)
	{ return 0;}

	// the _print() helper function
	virtual int _print(const char *lpszText,int iSize);

	


	// SafeWriteFile : write safely to the pipe
	inline BOOL SafeWriteFile(
		/*__in*/ HANDLE hFile,
		/*__in_bcount(nNumberOfBytesToWrite)*/	LPCVOID lpBuffer,
		/*__in        */ DWORD nNumberOfBytesToWrite,
		/*__out_opt   */ LPDWORD lpNumberOfBytesWritten,
		/*__inout_opt */ LPOVERLAPPED lpOverlapped
		)
	{
		EnterCriticalSection();
		BOOL bRet = ::WriteFile(hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped);
		LeaveCriticalSection();
		return bRet;
	}

};


///////////////////////////////////////////////////////////////////////////
// CConsoleLoggerEx: same as CConsoleLogger,
//                   but with COLORS and more functionality (cls,gotoxy,...)
//
// the drawback - we first send the "command" and than the data,
//                so it's little bit slower . (i don't believe that anyone 
//                is going to notice the differences , it's not measurable)
//
//////////////////////////////////////////////////////////////////////////
class CConsoleLoggerEx : public CConsoleLogger
{
	DWORD	m_dwCurrentAttributes;
	enum enumCommands
	{
		COMMAND_PRINT,
		COMMAND_CPRINT,
		COMMAND_CLEAR_SCREEN,
		COMMAND_COLORED_CLEAR_SCREEN,
		COMMAND_GOTOXY,
		COMMAND_CLEAR_EOL,
		COMMAND_COLORED_CLEAR_EOL
	};
public:
	CConsoleLoggerEx();

	enum enumColors
	{
		COLOR_BLACK=0,
		COLOR_BLUE=FOREGROUND_BLUE,
		COLOR_GREEN=FOREGROUND_GREEN ,
		COLOR_RED =FOREGROUND_RED ,
		COLOR_WHITE=COLOR_RED|COLOR_GREEN|COLOR_BLUE,
		COLOR_INTENSITY =FOREGROUND_INTENSITY ,
		COLOR_BACKGROUND_BLACK=0,
		COLOR_BACKGROUND_BLUE =BACKGROUND_BLUE ,
		COLOR_BACKGROUND_GREEN =BACKGROUND_GREEN ,
		COLOR_BACKGROUND_RED =BACKGROUND_RED ,
		COLOR_BACKGROUND_WHITE =COLOR_BACKGROUND_RED|COLOR_BACKGROUND_GREEN|COLOR_BACKGROUND_BLUE ,
		COLOR_BACKGROUND_INTENSITY =BACKGROUND_INTENSITY ,
		COLOR_COMMON_LVB_LEADING_BYTE =COMMON_LVB_LEADING_BYTE ,
		COLOR_COMMON_LVB_TRAILING_BYTE =COMMON_LVB_TRAILING_BYTE ,
		COLOR_COMMON_LVB_GRID_HORIZONTAL =COMMON_LVB_GRID_HORIZONTAL ,
		COLOR_COMMON_LVB_GRID_LVERTICAL =COMMON_LVB_GRID_LVERTICAL ,
		COLOR_COMMON_LVB_GRID_RVERTICAL =COMMON_LVB_GRID_RVERTICAL ,
		COLOR_COMMON_LVB_REVERSE_VIDEO =COMMON_LVB_REVERSE_VIDEO ,
		COLOR_COMMON_LVB_UNDERSCORE=COMMON_LVB_UNDERSCORE

		
	};
	
	// Clear screen , use default color (black&white)
	void cls(void);
	
	// Clear screen use specific color
	void cls(DWORD color);

	// Clear till End Of Line , use default color (black&white)
	void clear_eol(void);
	
	// Clear till End Of Line , use specified color
	void clear_eol(DWORD color);
	
	// write string , use specified color
	int cprintf(int attributes,const char *format,...);
	
	// write string , use current color
	int cprintf(const char *format,...);
	
	// goto(x,y)
	void gotoxy(int x,int y);



	DWORD	GetCurrentColor(void)
	{ return m_dwCurrentAttributes; }
	
	void	SetCurrentColor(DWORD dwColor)
	{ m_dwCurrentAttributes=dwColor; }
	

protected:
	virtual long	AddHeaders(void)
	{	// Thnx to this function, the "Helper" can see that we are "extended" logger !!!
		// (so we can use the same helper-child-application for both loggers
		DWORD cbWritten=0;
		char *ptr="Extended-Console: TRUE\r\n";
		WriteFile(m_hPipe,ptr,strlen(ptr),&cbWritten,NULL);
		return (cbWritten==strlen(ptr)) ? 0 : -1;
	}
	
	virtual int _print(const char *lpszText,int iSize);
	virtual int _cprint(int attributes,const char *lpszText,int iSize);


};

#endif