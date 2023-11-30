#include "zconsole/ConsoleLogger.h"
#include "zconfig.h"
#include <allegro.h>

byte monochrome_console;

#ifdef _WIN32

static OVERLAPPED k_Over = {0};
static void WINAPI CompletionRoutine(DWORD u32_ErrorCode, DWORD u32_BytesTransfered, OVERLAPPED* pk_Overlapped)
{
    // Don't care.
}

//{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// CTOR: reset everything
CConsoleLogger::CConsoleLogger() : kill_on_close(true)
{
	InitializeCriticalSection();
	m_name[0]=0;
	m_hPipe = INVALID_HANDLE_VALUE;
}

// DTOR: delete everything
CConsoleLogger::~CConsoleLogger()
{
	DeleteCriticalSection();
	
	// Notice: Because we want the pipe to stay alive until all data is passed,
	//         it's better to avoid closing the pipe here....
	//Close();
}

//////////////////////////////////////////////////////////////////////////
// Create: create a new console (logger) with the following OPTIONAL attributes:
//
// lpszWindowTitle : window title
// buffer_size_x   : width
// buffer_size_y   : height
// logger_name     : pipe name . the default is f(this,time)
// helper_executable: which (and where) is the EXE that will write the pipe's output
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::Create(const char	*lpszWindowTitle/*=NULL*/,
							int32_t			buffer_size_x/*=-1*/,int32_t buffer_size_y/*=-1*/,
							const char	*logger_name/*=NULL*/,
							const char	*helper_executable/*=NULL*/)
{
	#define _CONSOLE_DEBUG(str) al_trace(str)
	// Ensure there's no pipe connected
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(m_hPipe);
		CloseHandle(m_hPipe);
		m_hPipe=INVALID_HANDLE_VALUE;
	}
	strcpy(m_name,"\\\\.\\pipe\\");
	
	if (!logger_name)
	{	// no name was give , create name based on the current address+time
		// (you can modify it to use PID , zc_rand() ,...
		uint32_t now = GetTickCount();
		logger_name = m_name+ strlen(m_name);
		sprintf((char*)logger_name,"logger%p_%lu",this,now);
	}
	else
	{	// just use the given name
		strcat(m_name,logger_name);
	}
	_CONSOLE_DEBUG("Creating pipe...\n");
	// Create the pipe
	m_hPipe = CreateNamedPipe( 
		  m_name,                   // pipe name 
		  PIPE_ACCESS_OUTBOUND|FILE_FLAG_OVERLAPPED,     // read/write access, we're only writing...
		  PIPE_TYPE_MESSAGE |       // message type pipe 
		  PIPE_READMODE_BYTE|       // message-read mode 
		  PIPE_WAIT,                // blocking mode 
		  1,                        // max. instances  
		  32768,                    // output buffer size 
		  0,                        // input buffer size (we don't read data, so 0 is fine)
		  1,                        // client time-out 
		  NULL);                    // no security attribute 
	if (m_hPipe==INVALID_HANDLE_VALUE)
	{	// failure
		MessageBox(NULL,"CreateNamedPipe failed","ConsoleLogger failed",MB_OK);
		return -1;
	}
	_CONSOLE_DEBUG("Created pipe!\nCreating process...\n");
	
	// Extra console : create another process , its role is to display the pipe's output
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetStartupInfo(&si);
	
	char cmdline[MAX_PATH];;
	if (!helper_executable) {
		helper_executable = DEFAULT_HELPER_EXE;
	}
	sprintf(cmdline,"%s %s",helper_executable,logger_name);
	BOOL bRet = CreateProcess(NULL,cmdline,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
	if (!bRet)
	{	// on failure - try to get the path from the environment
		char *path = getenv("ConsoleLoggerHelper");
		if (path)
		{
			sprintf(cmdline,"%s %s",path,logger_name);
			bRet = CreateProcess(NULL,cmdline,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
		}
		if (!bRet)
		{
			MessageBox(NULL,"Helper executable not found","ConsoleLogger failed",MB_OK);
			CloseHandle(m_hPipe);
			m_hPipe = INVALID_HANDLE_VALUE;
			return -1;
		}
	}
	killer.init_process(pi.hProcess);
	
	_CONSOLE_DEBUG("Created process!\nConnecting pipe...\n");
	BOOL bConnected = ConnectNamedPipe(m_hPipe, NULL) ? 
		 TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
	if (!bConnected)
	{
		MessageBox(NULL,"ConnectNamedPipe failed","ConsoleLogger failed",MB_OK);
		
		CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
		return -1;
	}
	
	DWORD cbWritten;
	
	_CONSOLE_DEBUG("Connected pipe!\n");
	//////////////////////////////////////////////////////////////////////////
	// In order to easily add new future-features , i've chosen to pass the "extra"
	// parameters just the HTTP protocol - via textual "headers" .
	// the last header should end with NULL
	//////////////////////////////////////////////////////////////////////////
	
	char buffer[128];
	// Send title
	if (!lpszWindowTitle)	lpszWindowTitle=m_name+9;
	sprintf(buffer,"TITLE: %s\r\n",lpszWindowTitle);
	WriteFile(m_hPipe,buffer,strlen(buffer),&cbWritten,NULL);
	if (cbWritten!=strlen(buffer))
	{
		MessageBox(NULL,"WriteFile failed(1)","ConsoleLogger failed",MB_OK);
		DisconnectNamedPipe(m_hPipe);
		CloseHandle(m_hPipe);
		m_hPipe=INVALID_HANDLE_VALUE;
		return -1;
	}
	
	if (buffer_size_x!=-1 && buffer_size_y!=-1)
	{	// Send buffer-size
		sprintf(buffer,"BUFFER-SIZE: %dx%d\r\n",buffer_size_x,buffer_size_y);
		WriteFile(m_hPipe,buffer,strlen(buffer),&cbWritten,NULL);
		if (cbWritten!=strlen(buffer))
		{
			MessageBox(NULL,"WriteFile failed(2)","ConsoleLogger failed",MB_OK);
			DisconnectNamedPipe(m_hPipe);
			CloseHandle(m_hPipe);
			m_hPipe=INVALID_HANDLE_VALUE;
			return -1;
		}
	}
	
	// Send more headers. you can override the AddHeaders() function to 
	// extend this class
	if (AddHeaders())
	{	
		DisconnectNamedPipe(m_hPipe);
		CloseHandle(m_hPipe);
		m_hPipe=INVALID_HANDLE_VALUE;
		return -1;
	}
	
	// send NULL as "end of header"
	buffer[0]=0;
	WriteFile(m_hPipe,buffer,1,&cbWritten,NULL);
	if (cbWritten!=1)
	{
		MessageBox(NULL,"WriteFile failed(3)","ConsoleLogger failed",MB_OK);
		DisconnectNamedPipe(m_hPipe);
		CloseHandle(m_hPipe);
		m_hPipe=INVALID_HANDLE_VALUE;
		return -1;
	}
	return 0;
}

void CConsoleLogger::kill()
{
	killer.kill();
	m_hPipe = INVALID_HANDLE_VALUE;
}

// Close and disconnect
int32_t CConsoleLogger::Close(void)
{
	int32_t ret;
	if (m_hPipe==INVALID_HANDLE_VALUE || m_hPipe==NULL)
		ret = -1;
	else
		ret = DisconnectNamedPipe( m_hPipe );
	if(kill_on_close) kill();
	return ret;
}

//////////////////////////////////////////////////////////////////////////
// print: print string lpszText with size iSize
// if iSize==-1 (default) , we'll use strlen(lpszText)
// 
// this is the fastest way to print a simple (not formatted) string
//////////////////////////////////////////////////////////////////////////
inline int32_t CConsoleLogger::print(const char *lpszText,int32_t iSize/*=-1*/)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;
	return _print(lpszText,(iSize==-1) ? strlen(lpszText) : iSize);
}

bool CConsoleLogger::valid()
{
	return m_hPipe != INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////
// printf: print a formatted string
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::printf(const char *format,...)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;
	
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	tmp[vbound(ret,0,1023)]=0;
	
	va_end(argList);
	
	return _print(tmp,ret);
}

//////////////////////////////////////////////////////////////////////////
// set the default (CRT) printf() to use this logger
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::SetAsDefaultOutput(void)
{
	int32_t hConHandle = _open_osfhandle(/*lStdHandle*/ (intptr_t)m_hPipe, _O_TEXT);
	if (hConHandle==-1)
		return -2;
	FILE *fp = _fdopen( hConHandle, "w" );
	if (!fp)
		return -3;
	*stdout = *fp;
	return setvbuf( stdout, NULL, _IONBF, 0 );
}

///////////////////////////////////////////////////////////////////////////
// Reset the CRT printf() to its default
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::ResetDefaultOutput(void)
{
	intptr_t lStdHandle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
	if (lStdHandle ==  (intptr_t)INVALID_HANDLE_VALUE)
		return -1;
	int32_t hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	if (hConHandle==-1)
		return -2;
	FILE *fp = _fdopen( hConHandle, "w" );
	if (!fp)
		return -3;
	*stdout = *fp;
	return setvbuf( stdout, NULL, _IONBF, 0 );
}

//////////////////////////////////////////////////////////////////////////
// _print: print helper
// we use the thread-safe funtion "SafeWriteFile()" to output the data
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::_print(const char *lpszText,int32_t iSize)
{
	DWORD dwWritten=(DWORD)-1;
	
	return (!SafeWriteFile( m_hPipe,lpszText,iSize,&dwWritten,NULL)
		|| (int32_t)dwWritten!=iSize) ? -1 : (int32_t)dwWritten;
}

//////////////////////////////////////////////////////////////////////////
// Implementation of the derived class: CConsoleLoggerEx
//////////////////////////////////////////////////////////////////////////

// ctor: just set the default color
CConsoleLoggerEx::CConsoleLoggerEx()
{
	m_dwCurrentAttributes = COLOR_WHITE | COLOR_BACKGROUND_BLACK;
}

//////////////////////////////////////////////////////////////////////////
// override the _print.
// first output the "command" (which is COMMAND_PRINT) and the size,
// and than output the string itself	
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLoggerEx::_print(const char *lpszText,int32_t iSize)
{
	DWORD dwWritten=(DWORD)-1;
	// we assume that in iSize < 2^24 , because we're using only 3 bytes of iSize 
	// 32BIT: send DWORD = 4bytes: one byte is the command (COMMAND_PRINT) , and 3 bytes for size
	
	DWORD command_plus_size = (COMMAND_PRINT <<24)| iSize;
	EnterCriticalSection();
	if ( !WriteFile (m_hPipe, &command_plus_size,sizeof(DWORD),&dwWritten,NULL) 
		|| dwWritten != sizeof(DWORD))
	{
		LeaveCriticalSection();
		return -1;
	}
	
	int32_t iRet = (!WriteFile( m_hPipe,lpszText,iSize,&dwWritten,NULL)
		|| (int32_t)dwWritten!=iSize) ? -1 : (int32_t)dwWritten;
	LeaveCriticalSection();
	return iRet;
}

//////////////////////////////////////////////////////////////////////////
// cls: clear screen  (just sends the COMMAND_CLEAR_SCREEN)
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::cls(void)
{
	DWORD dwWritten=(DWORD)-1;
	// we assume that in iSize < 2^24 , because we're using only 3 bytes of iSize 
	// 32BIT: send DWORD = 4bytes: one byte is the command (COMMAND_PRINT) , and 3 bytes for size
	DWORD command = COMMAND_CLEAR_SCREEN<<24;
	SafeWriteFile (m_hPipe, &command,sizeof(DWORD),&dwWritten,NULL);
}	

//////////////////////////////////////////////////////////////////////////
// cls(DWORD) : clear screen with specific color
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::cls(DWORD color)
{
	DWORD dwWritten=(DWORD)-1;
	// we assume that in iSize < 2^24 , because we're using only 3 bytes of iSize 
	// 32BIT: send DWORD = 4bytes: one byte is the command (COMMAND_PRINT) , and 3 bytes for size
	DWORD command = COMMAND_COLORED_CLEAR_SCREEN<<24;
	EnterCriticalSection();
	WriteFile (m_hPipe, &command,sizeof(DWORD),&dwWritten,NULL);
	WriteFile (m_hPipe, &color,sizeof(DWORD),&dwWritten,NULL);
	LeaveCriticalSection();
}	

//////////////////////////////////////////////////////////////////////////
// clear_eol() : clear till the end of current line
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::clear_eol(void)
{
	DWORD dwWritten=(DWORD)-1;
	// we assume that in iSize < 2^24 , because we're using only 3 bytes of iSize 
	// 32BIT: send DWORD = 4bytes: one byte is the command (COMMAND_PRINT) , and 3 bytes for size
	DWORD command = COMMAND_CLEAR_EOL<<24;
	SafeWriteFile (m_hPipe, &command,sizeof(DWORD),&dwWritten,NULL);
}	

//////////////////////////////////////////////////////////////////////////
// clear_eol(DWORD) : clear till the end of current line with specific color
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::clear_eol(DWORD color)
{
	DWORD dwWritten=(DWORD)-1;
	// we assume that in iSize < 2^24 , because we're using only 3 bytes of iSize 
	// 32BIT: send DWORD = 4bytes: one byte is the command (COMMAND_PRINT) , and 3 bytes for size
	DWORD command = COMMAND_COLORED_CLEAR_EOL<<24;
	EnterCriticalSection();
	WriteFile (m_hPipe, &command,sizeof(DWORD),&dwWritten,NULL);
	WriteFile (m_hPipe, &color,sizeof(DWORD),&dwWritten,NULL);
	LeaveCriticalSection();
}	

//////////////////////////////////////////////////////////////////////////
// gotoxy(x,y) : sets the cursor to x,y location
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::gotoxy(int32_t x,int32_t y)
{
	DWORD dwWritten=(DWORD)-1;
	// we assume that in iSize < 2^24 , because we're using only 3 bytes of iSize 
	// 32BIT: send DWORD = 4bytes: one byte is the command (COMMAND_PRINT) , and 3 bytes for size
	DWORD command = COMMAND_GOTOXY<<24;
	EnterCriticalSection();
	WriteFile (m_hPipe, &command,sizeof(DWORD),&dwWritten,NULL);
	command = (x<<16)  | y;
	WriteFile (m_hPipe, &command,sizeof(DWORD),&dwWritten,NULL);
	LeaveCriticalSection();
}	

//////////////////////////////////////////////////////////////////////////
// cprintf(attr,str,...) : prints a formatted string with the "attributes" color
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLoggerEx::cprintf(int32_t attributes,const char *format,...)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;
	
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	tmp[vbound(ret, 0, 1023)]=0;
	
	va_end(argList);
	
	if ( monochrome_console ) return _cprint(CConsoleLoggerEx::COLOR_BACKGROUND_BLACK | CConsoleLoggerEx::COLOR_WHITE,tmp,ret);
	else	return _cprint(attributes,tmp,ret);
}
int32_t CConsoleLoggerEx::safeprint(int32_t attributes,const char *str)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;
	
	int32_t sz = strlen(str);
	
	if ( monochrome_console ) return _cprint(CConsoleLoggerEx::COLOR_BACKGROUND_BLACK | CConsoleLoggerEx::COLOR_WHITE,str,sz);
	else return _cprint(attributes,str,sz);
}

//////////////////////////////////////////////////////////////////////////
// cprintf(str,...) : prints a formatted string with current color
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLoggerEx::cprintf(const char *format,...)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;
	
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	tmp[vbound(ret, 0, 1023)]=0;
	
	va_end(argList);
	
	if ( monochrome_console ) return _cprint(CConsoleLoggerEx::COLOR_BACKGROUND_BLACK | CConsoleLoggerEx::COLOR_WHITE,tmp,ret);
	else return _cprint(m_dwCurrentAttributes,tmp,ret);

}
int32_t CConsoleLoggerEx::safeprint(const char *str)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;
	
	int32_t sz = strlen(str);
	
	if ( monochrome_console ) return _cprint(CConsoleLoggerEx::COLOR_BACKGROUND_BLACK | CConsoleLoggerEx::COLOR_WHITE,str,sz);
	else return _cprint(m_dwCurrentAttributes,str,sz);
}

//////////////////////////////////////////////////////////////////////////
// the _cprintf() helper . do the actual output
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLoggerEx::_cprint(int32_t attributes,const char *lpszText,int32_t iSize)
{
	// we assume that in iSize < 2^24 , because we're using only 3 bytes of iSize 
	// 32BIT: send DWORD = 4bytes: one byte is the command (COMMAND_CPRINT) , and 3 bytes for size
	DWORD command_plus_size = (COMMAND_CPRINT <<24)| iSize;
	EnterCriticalSection();
	if ( !WriteFileEx (m_hPipe, &command_plus_size,sizeof(DWORD),&k_Over,&CompletionRoutine))
	{
		LeaveCriticalSection();
		return -1;
	}
	
	command_plus_size = attributes;	// reuse of the prev variable
	if ( !WriteFileEx (m_hPipe, &command_plus_size,sizeof(DWORD),&k_Over,&CompletionRoutine))
	{
		LeaveCriticalSection();
		return -1;
	}
	
	int32_t iRet = WriteFileEx( m_hPipe,lpszText,iSize,&k_Over,&CompletionRoutine);
	LeaveCriticalSection();
	return iRet;
}
//}
#else

#include <cstdio>
//{Unix

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// CTOR: reset everything
CConsoleLogger::CConsoleLogger() : kill_on_close(true)
{
}

// DTOR: delete everything
CConsoleLogger::~CConsoleLogger()
{

}

//////////////////////////////////////////////////////////////////////////
// Create: create a new console (logger) with the following OPTIONAL attributes:
//
// lpszWindowTitle : window title
// buffer_size_x   : width
// buffer_size_y   : height
// logger_name     : pipe name . the default is f(this,time)
// helper_executable: which (and where) is the EXE that will write the pipe's output
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::Create(const char	*lpszWindowTitle/*=NULL*/,
							int32_t			buffer_size_x/*=-1*/,int32_t buffer_size_y/*=-1*/,
							const char	*logger_name/*=NULL*/,
							const char	*helper_executable/*=NULL*/)
{
#ifdef __EMSCRIPTEN__
	return 0;
#else
	if (m_textlog) {
		kill();
	}

	m_textlog = al_open_native_text_log("ZScript", ALLEGRO_TEXTLOG_MONOSPACE);
	return m_textlog == nullptr ? 1 : 0;
#endif
}

void CConsoleLogger::kill()
{
#ifndef __EMSCRIPTEN__
	if (m_textlog) {
		al_close_native_text_log(m_textlog);
		m_textlog = NULL;
	}
#endif
}

// Close and disconnect
int32_t CConsoleLogger::Close(void)
{
	if(kill_on_close) kill();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// print: print string lpszText with size iSize
// if iSize==-1 (default) , we'll use strlen(lpszText)
// 
// this is the fastest way to print a simple (not formatted) string
//////////////////////////////////////////////////////////////////////////
inline int32_t CConsoleLogger::print(const char *lpszText,int32_t iSize/*=-1*/)
{
#ifdef __EMSCRIPTEN__
	::printf("%s", lpszText);
#else
	al_append_native_text_log(m_textlog, "%s", lpszText);
#endif
	return 0;
}

bool CConsoleLogger::valid()
{
#ifdef __EMSCRIPTEN__
	return 1;
#else
	return m_textlog != NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
// printf: print a formatted string
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::printf(const char *format,...)
{
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	tmp[vbound(ret,0,1023)]=0;
	
	va_end(argList);
	
#ifdef __EMSCRIPTEN__
	::printf("%s", tmp);
#else
	al_append_native_text_log(m_textlog, "%s", tmp);
#endif
	return ret;
}

//////////////////////////////////////////////////////////////////////////
// set the default (CRT) printf() to use this logger
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::SetAsDefaultOutput(void)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Reset the CRT printf() to its default
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::ResetDefaultOutput(void)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// _print: print helper
// we use the thread-safe funtion "SafeWriteFile()" to output the data
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLogger::_print(const char *lpszText,int32_t iSize)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Implementation of the derived class: CConsoleLoggerEx
//////////////////////////////////////////////////////////////////////////

// ctor: just set the default color
CConsoleLoggerEx::CConsoleLoggerEx()
{
}

//////////////////////////////////////////////////////////////////////////
// override the _print.
// first output the "command" (which is COMMAND_PRINT) and the size,
// and than output the string itself	
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLoggerEx::_print(const char *lpszText,int32_t iSize)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// cls: clear screen  (just sends the COMMAND_CLEAR_SCREEN)
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::cls(void)
{

}	

//////////////////////////////////////////////////////////////////////////
// cls(DWORD) : clear screen with specific color
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::cls(word color)
{

}	

//////////////////////////////////////////////////////////////////////////
// clear_eol() : clear till the end of current line
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::clear_eol(void)
{

}	

//////////////////////////////////////////////////////////////////////////
// clear_eol(DWORD) : clear till the end of current line with specific color
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::clear_eol(word color)
{

}	

//////////////////////////////////////////////////////////////////////////
// gotoxy(x,y) : sets the cursor to x,y location
//////////////////////////////////////////////////////////////////////////
void CConsoleLoggerEx::gotoxy(int32_t x,int32_t y)
{

}	

//////////////////////////////////////////////////////////////////////////
// cprintf(attr,str,...) : prints a formatted string with the "attributes" color
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLoggerEx::cprintf(int32_t attributes,const char *format,...)
{
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	tmp[vbound(ret,0,1023)]=0;
	
	va_end(argList);

#ifdef __EMSCRIPTEN__
	::printf("%s", tmp);
#else
	al_append_native_text_log(m_textlog, "%s", tmp);
#endif
	return ret;
}
int32_t CConsoleLoggerEx::safeprint(int32_t attributes,const char *str)
{
	int32_t sz = strlen(str);

#ifdef __EMSCRIPTEN__
	::printf("%s", str);
#else
	al_append_native_text_log(m_textlog, "%s", str);
#endif
	return sz;
}

//////////////////////////////////////////////////////////////////////////
// cprintf(str,...) : prints a formatted string with current color
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLoggerEx::cprintf(const char *format,...)
{
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	tmp[vbound(ret,0,1023)]=0;
	
	va_end(argList);
	
#ifdef __EMSCRIPTEN__
	::printf("%s", tmp);
#else
	al_append_native_text_log(m_textlog, "%s", tmp);
#endif
	return ret;
}
int32_t CConsoleLoggerEx::safeprint(const char *str)
{
	int32_t sz = strlen(str);

#ifdef __EMSCRIPTEN__
	::printf("%s", str);
#else
	al_append_native_text_log(m_textlog, "%s", str);
#endif
	return sz;
}
//////////////////////////////////////////////////////////////////////////
// the _cprintf() helper . do the actual output
//////////////////////////////////////////////////////////////////////////
int32_t CConsoleLoggerEx::_cprint(int32_t attributes,const char *lpszText,int32_t iSize)
{
	return 0;
}
//}
#endif

