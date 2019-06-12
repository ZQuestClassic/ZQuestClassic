// ConsoleLogger.cpp: implementation of the CConsoleLogger class.
//
//////////////////////////////////////////////////////////////////////

#include "ConsoleLogger.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



// CTOR: reset everything
CConsoleLogger::CConsoleLogger()
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
long CConsoleLogger::Create(const char	*lpszWindowTitle/*=NULL*/,
							int			buffer_size_x/*=-1*/,int buffer_size_y/*=-1*/,
							const char	*logger_name/*=NULL*/,
							const char	*helper_executable/*=NULL*/)
{
	
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
		// (you can modify it to use PID , rand() ,...
		unsigned long now = GetTickCount();
		logger_name = m_name+ strlen(m_name);
		sprintf((char*)logger_name,"logger%d_%lu",(int)this,now);
	}
	else
	{	// just use the given name
		strcat(m_name,logger_name);
	}

	
	// Create the pipe
	m_hPipe = CreateNamedPipe( 
		  m_name,					// pipe name 
		  PIPE_ACCESS_OUTBOUND,		// read/write access, we're only writing...
		  PIPE_TYPE_MESSAGE |       // message type pipe 
		  PIPE_READMODE_BYTE|		// message-read mode 
		  PIPE_WAIT,                // blocking mode 
		  1,						// max. instances  
		  4096,						// output buffer size 
		  0,						// input buffer size (we don't read data, so 0 is fine)
		  1,						// client time-out 
		  NULL);                    // no security attribute 
	if (m_hPipe==INVALID_HANDLE_VALUE)
	{	// failure
		MessageBox(NULL,"CreateNamedPipe failed","ConsoleLogger failed",MB_OK);
		return -1;
	}

	// Extra console : create another process , it's role is to display the pipe's output
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetStartupInfo(&si);
	
	char cmdline[MAX_PATH];;
	if (!helper_executable)
		helper_executable=DEFAULT_HELPER_EXE;

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


// Close and disconnect
long CConsoleLogger::Close(void)
{
	if (m_hPipe==INVALID_HANDLE_VALUE || m_hPipe==NULL)
		return -1;
	else
		return DisconnectNamedPipe( m_hPipe );
}


//////////////////////////////////////////////////////////////////////////
// print: print string lpszText with size iSize
// if iSize==-1 (default) , we'll use strlen(lpszText)
// 
// this is the fastest way to print a simple (not formatted) string
//////////////////////////////////////////////////////////////////////////
inline int CConsoleLogger::print(const char *lpszText,int iSize/*=-1*/)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;
	return _print(lpszText,(iSize==-1) ? strlen(lpszText) : iSize);
}

//////////////////////////////////////////////////////////////////////////
// printf: print a formatted string
//////////////////////////////////////////////////////////////////////////
int CConsoleLogger::printf(const char *format,...)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;

	int ret;
	char tmp[1024];

	va_list argList;
	va_start(argList, format);
	#ifdef WIN32
	 		ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#else
	 		ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#endif
	tmp[ret]=0;


	va_end(argList);


	return _print(tmp,ret);

}



//////////////////////////////////////////////////////////////////////////
// set the default (CRT) printf() to use this logger
//////////////////////////////////////////////////////////////////////////
int CConsoleLogger::SetAsDefaultOutput(void)
{
	int hConHandle = _open_osfhandle(/*lStdHandle*/ (long)m_hPipe, _O_TEXT);
	if (hConHandle==-1)
		return -2;
	FILE *fp = _fdopen( hConHandle, "w" );
	if (!fp)
		return -3;
	*stdout = *fp;
	return setvbuf( stdout, NULL, _IONBF, 0 );
}

///////////////////////////////////////////////////////////////////////////
// Reset the CRT printf() to it's default
//////////////////////////////////////////////////////////////////////////
int CConsoleLogger::ResetDefaultOutput(void)
{
	long lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	if (lStdHandle ==  (long)INVALID_HANDLE_VALUE)
		return -1;
	int hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
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
int CConsoleLogger::_print(const char *lpszText,int iSize)
{
	DWORD dwWritten=(DWORD)-1;
	
	return (!SafeWriteFile( m_hPipe,lpszText,iSize,&dwWritten,NULL)
		|| (int)dwWritten!=iSize) ? -1 : (int)dwWritten;
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
int CConsoleLoggerEx::_print(const char *lpszText,int iSize)
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
	
	int iRet = (!WriteFile( m_hPipe,lpszText,iSize,&dwWritten,NULL)
		|| (int)dwWritten!=iSize) ? -1 : (int)dwWritten;
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
void CConsoleLoggerEx::gotoxy(int x,int y)
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
int CConsoleLoggerEx::cprintf(int attributes,const char *format,...)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;

	int ret;
	char tmp[1024];

	va_list argList;
	va_start(argList, format);
	#ifdef WIN32
	 		ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#else
	 		ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#endif
	tmp[ret]=0;


	va_end(argList);


	return _cprint(attributes,tmp,ret);

}


//////////////////////////////////////////////////////////////////////////
// cprintf(str,...) : prints a formatted string with current color
//////////////////////////////////////////////////////////////////////////
int CConsoleLoggerEx::cprintf(const char *format,...)
{
	if (m_hPipe==INVALID_HANDLE_VALUE)
		return -1;

	int ret;
	char tmp[1024];

	va_list argList;
	va_start(argList, format);
	#ifdef WIN32
	 		ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#else
	 		ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#endif
	tmp[ret]=0;


	va_end(argList);


	return _cprint(m_dwCurrentAttributes,tmp,ret);

}


//////////////////////////////////////////////////////////////////////////
// the _cprintf() helper . do the actual output
//////////////////////////////////////////////////////////////////////////
int CConsoleLoggerEx::_cprint(int attributes,const char *lpszText,int iSize)
{
	
	DWORD dwWritten=(DWORD)-1;
	// we assume that in iSize < 2^24 , because we're using only 3 bytes of iSize 
	// 32BIT: send DWORD = 4bytes: one byte is the command (COMMAND_CPRINT) , and 3 bytes for size
	DWORD command_plus_size = (COMMAND_CPRINT <<24)| iSize;
	EnterCriticalSection();
	if ( !WriteFile (m_hPipe, &command_plus_size,sizeof(DWORD),&dwWritten,NULL) 
		|| dwWritten != sizeof(DWORD))
	{
		LeaveCriticalSection();
		return -1;
	}
	
	command_plus_size = attributes;	// reuse of the prev variable
	if ( !WriteFile (m_hPipe, &command_plus_size,sizeof(DWORD),&dwWritten,NULL) 
		|| dwWritten != sizeof(DWORD))
	{
		LeaveCriticalSection();
		return -1;
	}
	
	int iRet = (!WriteFile( m_hPipe,lpszText,iSize,&dwWritten,NULL)
		|| (int)dwWritten!=iSize) ? -1 : (int)dwWritten;
	LeaveCriticalSection();
	return iRet;
}

