#include "base/process_management.h"
#include "base/util.h"
#include <fmt/format.h>
#include <sstream>

#ifndef _WIN32
	#include <csignal>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <spawn.h>
	#include <pstream.h>
#endif

using namespace util;

#define ERR_EXIT(str, pm, errcode) \
	do \
	{ \
		if(pm) \
			delete pm; \
		zprint2("[PROCESS_LAUNCH: '%s' ERROR %d]: %s\n", file.c_str(), errcode, str); \
		return NULL; \
	} while(false)

#ifdef _WIN32
void process_killer::init_process(void* h, uint32_t exitcode)
{
	if(process_handle)
		kill(exitcode);
	process_handle = h;
}
#else
void process_killer::init_process(int32_t pr_id)
{
	if(pid)
		kill();
	pid = pr_id;
}
#endif
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
#ifdef _WIN32
bool process_killer::kill(uint32_t exitcode)
{
	if(process_handle)
	{
		if(TerminateProcess(process_handle, exitcode))
		{
			process_handle = NULL;
			return true;
		}
		return false;
	}
	return true;
}
#else
bool process_killer::kill()
{
	if(pid)
	{
		if(::kill(pid,SIGKILL) == 0)
		{
			pid = 0;
			return true;
		}
		return false;
	}
	return true;
}
#endif

//

#ifdef _WIN32
static size_t callback_complete_count = 0;
static bool callback_error = false;
static VOID WINAPI FileCallback(DWORD error_code, DWORD bytes_transferred, LPOVERLAPPED ptr_overlapped)
{
	if(error_code)
		callback_error = true;
}

bool io_manager::ProcessReadFile(HANDLE read_handle, LPVOID buf, DWORD bytes_to_read, LPDWORD bytes_read)
{
	OVERLAPPED ov;
	LPOVERLAPPED p_ov = &ov;
	LPOVERLAPPED_COMPLETION_ROUTINE callback = FileCallback;
	memset(&ov, 0, sizeof(OVERLAPPED));
	
	callback_error = false;

	typedef BOOL (WINAPI *GetOverlappedResultExPROC)(HANDLE, LPOVERLAPPED, LPDWORD, DWORD, BOOL);
	HMODULE kernel32 = LoadLibraryW(L"kernel32");
	GetOverlappedResultExPROC imp_GetOverlappedResultEx =
		(GetOverlappedResultExPROC) GetProcAddress(kernel32, "GetOverlappedResultEx");

	// Windows 7 compat: GetOverlappedResultEx was added in Windows 8.
	if (!kernel32 || kernel32 == INVALID_HANDLE_VALUE)
	{
		DWORD num_read = 0;
		// if(!ReadFile(read_handle, buf, bytes_to_read, &num_read, p_ov))
		if(!ReadFile(read_handle, buf, bytes_to_read, NULL, NULL))
		{
			zprint2("READ FAILURE: %d\n",GetLastError());
			return false;
		}

		while(!GetOverlappedResult(read_handle, p_ov, bytes_read, true))
		{
			if(!is_alive())
				throw zc_io_exception::dead();
			auto error = GetLastError();
			switch(error)
			{
				case WAIT_IO_COMPLETION:
				case ERROR_IO_INCOMPLETE:
					continue;
				default:
					throw zc_io_exception::unknown();
			}
		}

		return !callback_error;
	}
	
	if (!ReadFileEx(read_handle, buf, bytes_to_read, p_ov, callback))
	{
		zprint2("READ FAILURE: %d\n",GetLastError());
		return false;
	}
	
	dword timeout_ms = timeout_seconds*1000;
	if(!timeout_ms) timeout_ms = INFINITE;
	while (!imp_GetOverlappedResultEx(read_handle, p_ov, bytes_read, timeout_ms, true))
	{
		if (!is_alive())
		{
			FreeLibrary(kernel32);
			throw zc_io_exception::dead();
		}
		auto error = GetLastError();
		switch(error)
		{
			case WAIT_TIMEOUT:
			{
				FreeLibrary(kernel32);
				throw zc_io_exception::timeout();
			}
			break;

			case WAIT_IO_COMPLETION:
			case ERROR_IO_INCOMPLETE:
				continue;
			default:
				FreeLibrary(kernel32);
				throw zc_io_exception::unknown();
		}
	}

	FreeLibrary(kernel32);
	return !callback_error;
}
bool io_manager::ProcessWriteFile(HANDLE write_handle, LPVOID buf, DWORD bytes_to_write, LPDWORD bytes_written)
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

bool process_manager::read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read)
{
#ifdef _WIN32
	if(!bytes_read) bytes_read = &__dummy_;
	*bytes_read = 0;

	if(!read_handle) return false;

	if(!is_alive())
		throw zc_io_exception::dead();

	bool ret = ProcessReadFile((HANDLE)read_handle, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read);
	
	if(!is_alive())
		throw zc_io_exception::dead();

	return ret;
#else
	if(!read_handle) return false;
	// if(!is_alive())
		// throw zc_io_exception::dead();
	size_t ret = ::read(read_handle, buf, bytes_to_read); //!TODO handle timeouts
	if(bytes_read) *bytes_read = ret;
	// if(!is_alive())
		// throw zc_io_exception::dead();
	return ret>0;
#endif
}

bool process_manager::write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written)
{
#ifdef _WIN32
	if(!write_handle) return false;
	if(!is_alive())
		throw zc_io_exception::dead();
	if(!bytes_written) bytes_written = &__dummy_;
	bool ret = ProcessWriteFile((HANDLE)write_handle, (LPVOID)buf, (DWORD)bytes_to_write, (LPDWORD)bytes_written);
	if(!is_alive())
		throw zc_io_exception::dead();
	return ret;
#else
	if(!write_handle) return false;
	// if(!is_alive())
		// throw zc_io_exception::dead();
	size_t ret = ::write(write_handle, buf, bytes_to_write); //!TODO handle timeouts
	if(bytes_written) *bytes_written = ret;
	// if(!is_alive())
		// throw zc_io_exception::dead();
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

bool child_process_handler::read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read)
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

bool child_process_handler::write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written)
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

#ifdef _WIN32
/******************************************************************
* https://groups.google.com/g/microsoft.public.win32.programmer.kernel/c/5UX78EXhyz0/m/oA-_hfk3_A0J
*
* BOOL RunOneShot(UseDos, szCmd)
*
* Spawn a child process and capture the redirect output.
*
* UseDos = Set TRUE to use OS command interpreter
* cmd = command to run (if DOS command, set UseDos=TRUE)
*
* return TRUE if successful. Otherwise FALSE, read extended error.
******************************************************************/

BOOL RunOneShot(const char *szCmd, const BOOL UseDos, std::string& output) {
  SECURITY_ATTRIBUTES sa;
  ZeroMemory(&sa, sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;

  HANDLE hOut = INVALID_HANDLE_VALUE;
  HANDLE hRedir = INVALID_HANDLE_VALUE;

  // Create Temp File for redirection

  char szTempPath[MAX_PATH];
  char szOutput[MAX_PATH];
  GetTempPath(sizeof(szTempPath), szTempPath);
  GetTempFileName(szTempPath, "tmp", 0, szOutput);

  // setup redirection handles
  // output handle must be WRITE mode, share READ
  // redirect handle must be READ mode, share WRITE

  hOut = CreateFile(szOutput, GENERIC_WRITE, FILE_SHARE_READ, &sa,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, 0);

  if (hOut == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  hRedir = CreateFile(szOutput, GENERIC_READ, FILE_SHARE_WRITE, 0,
                      OPEN_EXISTING, 0, 0);

  if (hRedir == INVALID_HANDLE_VALUE) {
    CloseHandle(hOut);
    return FALSE;
  }

  // setup startup info, set std out/err handles
  // hide window

  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  if (hOut != INVALID_HANDLE_VALUE) {
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = hOut;
    si.hStdError = hOut;
    si.wShowWindow = SW_HIDE;
  }

  // use the current OS comspec for DOS commands, i.e., DIR

  char cmd[MAX_PATH] = {0};
  if (UseDos) {
    strcpy(cmd, getenv("comspec"));
    strcat(cmd, " /c ");
  }
  strcat(cmd, szCmd);
  printf("-Process: %s\n", cmd);

  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  DWORD dwFlags = CREATE_NO_WINDOW; // NT/2000 only

  if (!CreateProcess(NULL, (char *)cmd, NULL, NULL, TRUE, dwFlags, NULL, NULL,
                     &si, &pi)) {

    int err = GetLastError(); // preserve the CreateProcess error

    if (hOut != INVALID_HANDLE_VALUE) {
      CloseHandle(hOut);
      CloseHandle(hRedir);
    }

    SetLastError(err);
    return FALSE;
  }

  // let go of the child thread
  CloseHandle(pi.hThread);

  // wait for process to finish, capture redirection and
  // send it to the parent window/console

  std::stringstream ss;

  DWORD dw;
  char buf[256];
  do {
    ZeroMemory(&buf, sizeof(buf));
    while (ReadFile(hRedir, &buf, sizeof(buf) - 1, &dw, NULL)) {
      if (dw == 0)
        break;
	  ss << buf;
      ZeroMemory(&buf, sizeof(buf));
    }
  } while (WaitForSingleObject(pi.hProcess, 0) != WAIT_OBJECT_0);

  // perform any final flushing

  while (ReadFile(hRedir, &buf, sizeof(buf) - 1, &dw, NULL)) {
    if (dw == 0)
      break;
    ss << buf;
    ZeroMemory(&buf, sizeof(buf));
  }

  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(hOut);
  CloseHandle(hRedir);
  DeleteFile(szOutput);

  output = ss.str();
  return true;
}
#endif

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
	extern char** environ;
	int s = posix_spawnp(&pid, file.c_str(), NULL, NULL, argv.data(), environ);
#endif
	for (auto arg : argv) free(arg);
	if (s != 0) ERR_EXIT("Failed posix_spawnp", (process_manager*)0, 0);
	return process_killer(pid);
#endif
}

process_manager* launch_piped_process(std::string file, std::string pipename, const std::vector<std::string>& args)
{
	process_manager* pm = new process_manager();
#ifdef _WIN32
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	#define PIPE_BUFFER_SZ 256
	
	std::string pipename_1 = fmt::format("\\\\.\\pipe\\{}",pipename),
		pipename_2 = pipename_1+"2";
	
	// if ( !CreatePipe(&(pm->read_handle), &(pm->re_2), &saAttr, 0))
		// ERR_EXIT("Failed to create child output pipe", pm);
	HANDLE PipeHandle1 = CreateNamedPipeA(pipename_1.c_str(),PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,PIPE_TYPE_BYTE,1,PIPE_BUFFER_SZ,PIPE_BUFFER_SZ,0,&saAttr);
	if(PipeHandle1==INVALID_HANDLE_VALUE)
		ERR_EXIT("Failed to create child output pipe", pm, GetLastError());
	if(!WaitNamedPipeA(pipename_1.c_str(), 1000*5))
		ERR_EXIT("Failed to recognize opened child output pipe", pm, 0);
	pm->read_handle = CreateFileA(pipename_1.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&saAttr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
	if(pm->read_handle==INVALID_HANDLE_VALUE)
		ERR_EXIT("Failed to open child output pipe read end", pm, GetLastError());
	pm->re_2 = PipeHandle1;//CreateFileA(pipename_1.c_str(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&saAttr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
	// if(pm->re_2==INVALID_HANDLE_VALUE)
		// ERR_EXIT("Failed to open child output pipe write end", pm, GetLastError());
	
	if ( !SetHandleInformation(pm->read_handle, HANDLE_FLAG_INHERIT, 0))
		ERR_EXIT("Failed to set handle information for child output", pm, GetLastError()); 
	
	// if ( !CreatePipe(&(pm->wr_2), &(pm->write_handle), &saAttr, 0))
		// ERR_EXIT("Failed to create child input pipe", pm);
	HANDLE PipeHandle2 = CreateNamedPipeA(pipename_2.c_str(),PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,PIPE_TYPE_BYTE,1,PIPE_BUFFER_SZ,PIPE_BUFFER_SZ,0,&saAttr);
	if(PipeHandle2==INVALID_HANDLE_VALUE)
		ERR_EXIT("Failed to create child input pipe", pm, GetLastError());
	if(!WaitNamedPipeA(pipename_2.c_str(), 1000*5))
		ERR_EXIT("Failed to recognize opened child input pipe", pm, 0);
	pm->wr_2 = PipeHandle2;//CreateFileA(pipename_2.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&saAttr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
	// if(pm->wr_2==INVALID_HANDLE_VALUE)
		// ERR_EXIT("Failed to open child input pipe read end", pm, GetLastError());
	pm->write_handle = CreateFileA(pipename_2.c_str(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&saAttr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
	if(pm->write_handle==INVALID_HANDLE_VALUE)
		ERR_EXIT("Failed to open child input pipe write end", pm, GetLastError());
	
	if ( !SetHandleInformation(pm->write_handle, HANDLE_FLAG_INHERIT, 0))
		ERR_EXIT("Failed to set handle information for child input", pm, GetLastError()); 
	
	
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
		ERR_EXIT("Failed to create process", pm, GetLastError());
	pm->init_process(pi.hProcess);
	return pm;
#else
	int32_t pdes_r[2], pdes_w[2], pid;
	posix_spawn_file_actions_t file_actions;
	int s;

	pipe(pdes_r);
	pipe(pdes_w);

	s = posix_spawn_file_actions_init(&file_actions);
	if (s != 0) ERR_EXIT("Failed posix_spawn_file_actions_init", pm, 0);
	
	posix_spawn_file_actions_adddup2(&file_actions, pdes_r[1], fileno(stdout));
	posix_spawn_file_actions_addclose(&file_actions, pdes_r[1]);
	posix_spawn_file_actions_addclose(&file_actions, pdes_r[0]);

	posix_spawn_file_actions_adddup2(&file_actions, pdes_w[0], fileno(stdin));
	posix_spawn_file_actions_addclose(&file_actions, pdes_w[0]);
	posix_spawn_file_actions_addclose(&file_actions, pdes_w[1]);

	std::vector<char*> argv = create_argv_unix(file, args);
	pid_t child_pid;
#ifdef ALLEGRO_LINUX
	s = posix_spawnp(&child_pid, file.c_str(), &file_actions, NULL, argv.data(), environ);
#else
	extern char** environ;
	s = posix_spawnp(&child_pid, file.c_str(), &file_actions, NULL, argv.data(), environ);
#endif
	if (s != 0) ERR_EXIT("Failed posix_spawnp", pm, 0);
	
	pm->read_handle = pdes_r[0];
	pm->write_handle = pdes_w[1];
	close(pdes_r[1]);
	close(pdes_w[0]);
	pm->init_process(child_pid);
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

// TODO: also need to read stderr in windows
bool run_and_get_output(std::string file, const std::vector<std::string>& args, std::string& output)
{
#ifndef _WIN32
	// For unix, bypass the whole pipe thing (which breaks terribly when zlauncher calls zupdater) by
	// using pstream library. Bonus, gives us stderr too.
	std::stringstream result;
	std::vector args2 = args;
	args2.insert(args2.begin(), file);
	redi::ipstream proc(file, args2, redi::pstreams::pstdout | redi::pstreams::pstderr);
	if (proc.bad())
		return false;

	std::string line;
	while (std::getline(proc.out(), line))
		result << line << '\n';
	if (proc.eof() && proc.fail())
		proc.clear();
	while (std::getline(proc.err(), line))
		result << "[stderr] " << line << '\n';

	output = result.str();
	return true;
#else
	char path_buf[2048];
	sprintf(path_buf, "\"%s\"", file.c_str());
	for (auto q : args)
	{
		strcat(path_buf, " \"");
		strcat(path_buf, q.c_str());
		strcat(path_buf, "\"");
	}
	return RunOneShot(path_buf, false, output);
#endif
}

// Utility function for parsing structured data from process output.
// Example usage is updater.py
std::map<std::string, std::string> parse_output_map(std::string output)
{
	std::map<std::string, std::string> result;

	util::removechar(output, '\r');
	std::vector<std::string> lines;
	util::split(output, lines, '\n');

	for (auto line : lines)
	{
		size_t pos = line.find_first_of(' ');
		if (pos != std::string::npos)
		{
			auto key = line.substr(0, pos);
			auto value = line.substr(pos + 1);
			result[key] = value;
		}
	}

	return result;
}
