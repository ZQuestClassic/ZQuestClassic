#include "base/process_management.h"
#include "base/util.h"
#include <fmt/format.h>
#ifndef _WIN32
	#include <csignal>
	#include <sys/types.h>
	#include <spawn.h>
#endif
using namespace util;

#define ERR_EXIT(str, pm) \
	do \
	{ \
		if(pm) \
			delete pm; \
		safe_al_trace(fmt::format("[PROCESS_LAUNCH: '{}' ERROR]: {}\n", file, str)); \
		return NULL; \
	} while(false)

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
		::kill(pid,SIGKILL);
		pid = 0;
	}
#endif
}

static std::vector<char*> create_argv_unix(std::string file, const std::vector<std::string>& args)
{
	std::vector<char*> argv;
	argv.reserve(args.size() + 2);
	argv.push_back(strcpy(new char[file.length() + 1], file.c_str()));
	std::transform(args.begin(), args.end(), std::back_inserter(argv), [](const std::string& s) {
		char *pc = new char[s.size()+1];
		std::strcpy(pc, s.c_str());
		return pc; 
	});
	argv.push_back(nullptr);
	return argv;
}

process_killer launch_process(std::string file, const std::vector<std::string>& args)
{
#ifdef _WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	char path_buf[2048];
	sprintf(path_buf, "\"%s\"", file.c_str());
	for (auto q : args)
	{
		strcat(path_buf, " \"");
		strcat(path_buf, q.c_str());
		strcat(path_buf, "\"");
	}

	GetStartupInfo(&si);
	CreateProcess(NULL,path_buf,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
	return process_killer(pi.hProcess);
#else
	std::vector<char*> argv = create_argv_unix(file, args);
	pid_t pid;
#ifdef ALLEGRO_LINUX
	int s = posix_spawnp(&pid, file.c_str(), NULL, NULL, argv.data(), environ);
#else
	int s = posix_spawn(&pid, file.c_str(), NULL, NULL, argv.data(), NULL);
#endif
	for (auto arg : argv) free(arg);
	if (s != 0) ERR_EXIT("Failed posix_spawn", (process_manager*)0);
	return process_killer(pid);
#endif
}

process_manager* launch_piped_process(std::string file, const std::vector<std::string>& args)
{
	process_manager* pm = new process_manager();
#ifdef _WIN32
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if ( !CreatePipe(&(pm->read_handle), &(pm->re_2), &saAttr, 0))
		ERR_EXIT("Failed to create child output pipe", pm);
	if ( !SetHandleInformation(pm->read_handle, HANDLE_FLAG_INHERIT, 0))
		ERR_EXIT("Failed to set handle information for child output", pm); 
	if ( !CreatePipe(&(pm->wr_2), &(pm->write_handle), &saAttr, 0))
		ERR_EXIT("Failed to create child input pipe", pm);
	if ( !SetHandleInformation(pm->write_handle, HANDLE_FLAG_INHERIT, 0))
		ERR_EXIT("Failed to set handle information for child input", pm); 
	
	
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
	sprintf(path_buf, "\"%s\"", file.c_str());
	for (auto q : args)
	{
		strcat(path_buf, " \"");
		strcat(path_buf, q.c_str());
		strcat(path_buf, "\"");
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
		ERR_EXIT("Failed to create process", pm);
	pm->pk.init(pi.hProcess);
	return pm;
#else
	int32_t pdes_r[2], pdes_w[2], pid;
	posix_spawn_file_actions_t file_actions;
	int s;

	pipe(pdes_r);
	pipe(pdes_w);

	s = posix_spawn_file_actions_init(&file_actions);
	if (s != 0) ERR_EXIT("Failed posix_spawn_file_actions_init", pm);
	
	posix_spawn_file_actions_adddup2(&file_actions, pdes_r[1], fileno(stdout));
	posix_spawn_file_actions_addclose(&file_actions, pdes_r[1]);
	posix_spawn_file_actions_addclose(&file_actions, pdes_r[0]);

	posix_spawn_file_actions_adddup2(&file_actions, pdes_w[0], fileno(stdin));
	posix_spawn_file_actions_addclose(&file_actions, pdes_w[0]);
	posix_spawn_file_actions_addclose(&file_actions, pdes_w[1]);

	std::vector<char*> argv = create_argv_unix(file, args);
	pid_t child_pid;
#ifdef ALLEGRO_LINUX
	s = posix_spawn(&child_pid, file.c_str(), &file_actions, NULL, argv.data(), environ);
#else
	s = posix_spawn(&child_pid, file.c_str(), &file_actions, NULL, argv.data(), NULL);
#endif
	if (s != 0) ERR_EXIT("Failed posix_spawn", pm);
	
	pm->read_handle = pdes_r[0];
	pm->write_handle = pdes_w[1];
	close(pdes_r[1]);
	close(pdes_w[0]);
	pm->pk.init(pid);
	return pm;
#endif
}

void launch_file(std::string const& file)
{
#ifdef _WIN32
	char path_buf[2048];
	GetCurrentDirectory(2047, path_buf);
	//trim trailing slashes
	for (int32_t q = strlen(path_buf) - 1; q >= 0; --q)
	{
		if (path_buf[q] == '/' || path_buf[q] == '\\')
			path_buf[q] = 0;
		else break;
	}
	strcat(path_buf, "\\\\");
	strcat(path_buf, file.c_str());
	ShellExecute(0, 0, path_buf, 0, 0, SW_NORMAL);
#elif defined(__EMSCRIPTEN__)
	// TODO
#else
	std::string command = "open " + file;
	system(command.c_str());
#endif
}
