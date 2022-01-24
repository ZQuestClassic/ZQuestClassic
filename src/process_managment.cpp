#include "process_managment.h"
#include "util.h"
using namespace util;

void process_killer::kill(uint32_t exitcode)
{
#ifdef _WIN32
	if(process_handle)
	{
		TerminateProcess(process_handle, exitcode);
		process_handle = NULL;
	}
#endif
}

process_killer launch_process(char const* relative_path)
{
#ifdef _WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetStartupInfo(&si);
	char path[MAX_PATH];
	strcpy(path, relative_path);
	CreateProcess(NULL,path,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
	return process_killer(pi.hProcess);
#else
	return process_killer();
#endif
}

process_manager* launch_piped_process(char const* relative_path)
{
#ifdef _WIN32
	process_manager* pm = new process_manager();
	#define ERR_EXIT(str) \
	do \
	{ \
		delete pm; \
		char buf[2048] = {0}; \
		sprintf(buf, "[PROCESS_LAUNCH: '%s' ERROR]: %s\n", relative_path, str); \
		safe_al_trace(buf); \
		return NULL; \
	} while(false)
	
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if ( !CreatePipe(&(pm->child_out_read), &(pm->child_out_write), &saAttr, 0))
		ERR_EXIT("Failed to create child output pipe");
	if ( !SetHandleInformation(pm->child_out_read, HANDLE_FLAG_INHERIT, 0))
		ERR_EXIT("Failed to set handle information for child output"); 
	if ( !CreatePipe(&(pm->child_in_read), &(pm->child_in_write), &saAttr, 0))
		ERR_EXIT("Failed to create child input pipe");
	if ( !SetHandleInformation(pm->child_in_write, HANDLE_FLAG_INHERIT, 0))
		ERR_EXIT("Failed to set handle information for child input"); 
	
	
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	BOOL bSuccess = FALSE;
	
	ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	
	si.hStdError = pm->child_out_write;
	si.hStdOutput = pm->child_out_write;
	si.hStdInput = pm->child_in_read;
	si.dwFlags |= STARTF_USESTDHANDLES;
	bSuccess = CreateProcess(NULL, 
		(LPSTR)relative_path, // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&si, &pi);
	if(!bSuccess)
		ERR_EXIT("Failed to create process");
	pm->pk.init(pi.hProcess);
	return pm;
#else
	return NULL;
#endif
}

