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

void process_killer::init_process(void* h, uint32_t exitcode)
{
#ifdef _WIN32
	if(process_handle)
		kill(exitcode);
	process_handle = h;
#else
	if(pid)
		kill(exitcode);
	pid = pr_id;
#endif
}
bool process_killer::is_alive() const
{
#ifdef _WIN32
	return process_handle && WaitForSingleObject(process_handle,0) == WAIT_TIMEOUT;
#else
	waitpid(pid, nullptr, WNOHANG);
	if(::kill(pid,0) == -1)
	{
		if(errno != ESRCH)
			return true;
		return false;
	}
	return true;
#endif
}
bool process_killer::kill(uint32_t exitcode)
{
#ifdef _WIN32
	if(process_handle)
	{
		if(TerminateProcess(process_handle, exitcode))
		{
			process_handle = NULL;
			return true;
		}
		return false;
	}
#else
	if(pid)
	{
		if(::kill(pid,SIGKILL) == 0)
		{
			pid = 0;
			return true;
		}
		return false
	}
#endif
	return true;
}

//

#ifdef _WIN32
static size_t callback_complete_count = 0;
static bool callback_error = false;
static VOID WINAPI FileCallback(DWORD error_code, DWORD bytes_transferred, LPOVERLAPPED ptr_overlapped)
{
	if(error_code)
		callback_error = true;
}

bool io_manager::ProcessReadFile(HANDLE read_handle, LPVOID buf, DWORD bytes_to_read, LPDWORD bytes_read, bool throw_timeout)
{
	OVERLAPPED ov;
	LPOVERLAPPED p_ov = &ov;
	LPOVERLAPPED_COMPLETION_ROUTINE callback = FileCallback;
	memset(&ov, 0, sizeof(OVERLAPPED));
	
	callback_error = false;
	
	zprint2("STARTING READ!\n");
	//! Why does this sometimes block, not returning? It's supposed to be async and continue to the GetOverlappedResultEx calls... -Em
	if(!ReadFileEx(read_handle, buf, bytes_to_read, p_ov, callback))
	{
		zprint2("READ FAILURE: %d\n",GetLastError());
		return false;
	}
	if(auto w = GetLastError())
		zprint2("READ WARNING: %d\n", w);
	
	int waitcount = 0;
	while(!GetOverlappedResultEx(read_handle, p_ov, bytes_read, 30000, true))
	{
		auto error = GetLastError();
		zprint2("...waiting %d (err %d)\n",++waitcount, error);
		switch(error)
		{
			case WAIT_TIMEOUT:
			{
				if(throw_timeout)
					throw zc_io_exception::timeout();
				else return false;
			}
			case WAIT_IO_COMPLETION:
			case ERROR_IO_INCOMPLETE:
				continue;
			default:
				throw zc_io_exception::unknown();
		}
	}
	
	return !callback_error;
}
bool io_manager::ProcessWriteFile(HANDLE write_handle, LPVOID buf, DWORD bytes_to_write, LPDWORD bytes_written, bool throw_timeout)
{
	return WriteFile(write_handle, buf, bytes_to_write, bytes_written, NULL);
}
#endif

//

process_manager::~process_manager()
{
#ifdef _WIN32
	if(write_handle)
	{
		CloseHandle(write_handle);
		write_handle = NULL;
	}
	if(read_handle)
	{
		CloseHandle(read_handle);
		read_handle = NULL;
	}
	if(wr_2)
	{
		CloseHandle(wr_2);
		wr_2 = NULL;
	}
	if(re_2)
	{
		CloseHandle(re_2);
		re_2 = NULL;
	}
#endif
	if(kill_on_destructor)
		kill();
}

bool process_manager::read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read, bool throw_timeout)
{
#ifdef _WIN32
	if(!read_handle) return false;
	if(throw_timeout && !is_alive())
		throw zc_io_exception::dead();
	if(!bytes_read) bytes_read = &__dummy_;
	bool ret = ProcessReadFile((HANDLE)read_handle, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read, throw_timeout);
	
	zprint2("READ: %d %d '%s'[%d]\n",callback_error?1:0,is_alive()?1:0,buf,*bytes_read);
	if(throw_timeout && !is_alive())
		throw zc_io_exception::dead();
	return ret;
#else
	if(!read_handle) return false;
	if(throw_timeout && !is_alive())
		throw zc_io_exception::dead();
	size_t ret = ::read(read_handle, buf, bytes_to_read); //!TODO handle timeouts (if throw_timeout, throw zc_io_exception::timeout())
	if(bytes_read) *bytes_read = ret;
	if(throw_timeout && !is_alive())
		throw zc_io_exception::dead();
	return ret>0;
#endif
}

bool process_manager::write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written, bool throw_timeout)
{
#ifdef _WIN32
	if(!write_handle) return false;
	if(throw_timeout && !is_alive())
		throw zc_io_exception::dead();
	if(!bytes_written) bytes_written = &__dummy_;
	bool ret = ProcessWriteFile((HANDLE)write_handle, (LPVOID)buf, (DWORD)bytes_to_write, (LPDWORD)bytes_written, throw_timeout);
	if(throw_timeout && !is_alive())
		throw zc_io_exception::dead();
	return ret;
#else
	if(!write_handle) return false;
	if(throw_timeout && !is_alive())
		throw zc_io_exception::dead();
	size_t ret = ::write(write_handle, buf, bytes_to_write); //!TODO handle timeouts (if throw_timeout, throw zc_io_exception::timeout())
	if(bytes_written) *bytes_written = ret;
	if(throw_timeout && !is_alive())
		throw zc_io_exception::dead();
	return ret==bytes_to_write;
#endif
}

//

bool child_process_handler::init()
{
#ifdef _WIN32
	out = GetStdHandle(STD_OUTPUT_HANDLE);
	in = GetStdHandle(STD_INPUT_HANDLE);
	return (out != INVALID_HANDLE_VALUE && in != INVALID_HANDLE_VALUE);
#else
	read_handle = fileno(stdin);
	write_handle = fileno(stdout);
	return true;
#endif
}

bool child_process_handler::read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read, bool throw_timeout)
{
#ifdef _WIN32
	if(!bytes_read) bytes_read = &__dummy_;
	if(in != INVALID_HANDLE_VALUE)
		return ReadFile((HANDLE)in, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read, NULL);
	return false;
#else
	if(!read_handle) return false;
	size_t ret = ::read(read_handle, buf, bytes_to_read);
	if(bytes_read) *bytes_read = ret;
	return ret>0;
#endif
}

bool child_process_handler::write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written, bool throw_timeout)
{
#ifdef _WIN32
	if(!bytes_written) bytes_written = &__dummy_;
	if(out != INVALID_HANDLE_VALUE)
		return WriteFile((HANDLE)out, (LPVOID)buf, (DWORD)bytes_to_write, (LPDWORD)bytes_written, NULL);
	return false;
#else
	if(!write_handle) return false;
	size_t ret = ::write(write_handle, buf, bytes_to_write);
	if(bytes_written) *bytes_written = ret;
	return ret==bytes_to_write;
#endif
}

//

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
	int s = posix_spawn(&pid, file.c_str(), NULL, NULL, argv.data(), environ);
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
	pm->init_process(pi.hProcess);
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
	pm->init_process(pid);
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
