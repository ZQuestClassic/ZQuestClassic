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
#else
	if(pid)
	{
		kill(pid,SIGKILL);
		pid = 0;
	}
#endif
}

process_killer launch_process(char const* relative_path, char const** argv)
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
	int32_t pid;
	
	char path_buf[2048];
	sprintf(path_buf, "./%s", relative_path);
	switch(pid = vfork())
	{
		case -1:
			ERR_EXIT("Failed to fork process");
		case 0: //child
			execv(path_buf, argv);
			ERR_EXIT("Failed execv");
	}
	
	return process_killer(pid);
#endif
}

process_manager* launch_piped_process(char const* relative_path, char const** argv)
{
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
	
#ifdef _WIN32
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if ( !CreatePipe(&(pm->read_handle), &(pm->re_2), &saAttr, 0))
		ERR_EXIT("Failed to create child output pipe");
	if ( !SetHandleInformation(pm->read_handle, HANDLE_FLAG_INHERIT, 0))
		ERR_EXIT("Failed to set handle information for child output"); 
	if ( !CreatePipe(&(pm->wr_2), &(pm->write_handle), &saAttr, 0))
		ERR_EXIT("Failed to create child input pipe");
	if ( !SetHandleInformation(pm->write_handle, HANDLE_FLAG_INHERIT, 0))
		ERR_EXIT("Failed to set handle information for child input"); 
	
	
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	BOOL bSuccess = FALSE;
	
	ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	
	si.hStdError = pm->re_2;
	si.hStdOutput = pm->re_2;
	si.hStdInput = pm->wr_2;
	si.dwFlags |= STARTF_USESTDHANDLES;
	char path_buf[2048];
	sprintf(path_buf, "\"%s\"", relative_path);
	if(argv)
	{
		for(auto q = 0; argv[q]; ++q)
		{
			strcat(path_buf, " ");
			strcat(path_buf, argv[q]);
		}
	}
	bSuccess = CreateProcess(NULL, 
		(LPSTR)path_buf, // command line 
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
	int32_t pdes_r[2], pdes_w[2], pid;
	
	pipe(pdes_r);
	pipe(pdes_w);
	char path_buf[2048];
	sprintf(path_buf, "./%s", relative_path);
	switch(pid = vfork())
	{
		case -1:
			ERR_EXIT("Failed to fork process");
		case 0: //child
			dup2(pdes_r[1], fileno(stdout));
			close(pdes_r[1]);
			close(pdes_r[0]);
			dup2(pdes_w[0], fileno(stdin));
			close(pdes_w[0]);
			close(pdes_w[1]);
			execv(path_buf, argv);
			ERR_EXIT("Failed execv");
	}
	
	pm->read_handle = (FILE*)(pdes_r[0]);
	pm->write_handle = (FILE*)(pdes_w[1]);
	close(pdes_r[1]);
	close(pdes_w[0]);
	pm->pk.init(pid);
	return pm;
#endif
}

