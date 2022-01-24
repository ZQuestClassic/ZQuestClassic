#ifndef ZC_PROCESS_MANAGEMENT_H
#define ZC_PROCESS_MANAGEMENT_H

#include <cstdio>
#include <string>

#ifdef _WIN32
#define NOGDI
#define WinMain __win_main
#include "windows.h"
#undef WinMain
#undef NOGDI
#endif

struct process_killer
{
	void* process_handle;
	void kill(uint32_t exitcode = 0);
	void init(void* h, uint32_t exitcode = 0)
	{
		if(process_handle)
			kill(exitcode);
		process_handle = h;
	}
	process_killer(void* h) : process_handle(h) {}
	process_killer() : process_handle(NULL) {}
};

struct process_manager
{
	#ifdef _WIN32
	//{ Windows
	HANDLE child_in_read;
	HANDLE child_in_write;
	HANDLE child_out_read;
	HANDLE child_out_write;
	PROCESS_INFORMATION pi;
	
	process_killer pk;
	
	bool kill_on_destructor;
	
	process_manager() : child_in_read(NULL), child_in_write(NULL),
		child_out_read(NULL), child_out_write(NULL),
		kill_on_destructor(true)
	{}
	
	void kill(uint32_t exitcode = 0)
	{
		pk.kill(exitcode);
	}
	
	~process_manager()
	{
		if(child_in_read)
		{
			CloseHandle(child_in_read);
			child_in_read = NULL;
		}
		if(child_in_write)
		{
			CloseHandle(child_in_write);
			child_in_write = NULL;
		}
		if(child_out_read)
		{
			CloseHandle(child_out_read);
			child_out_read = NULL;
		}
		if(child_out_write)
		{
			CloseHandle(child_out_write);
			child_out_write = NULL;
		}
		if(kill_on_destructor)
			kill();
	}
	
	bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read)
	{
		if(child_out_read)
			return ReadFile((HANDLE)child_out_read, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read, NULL);
		return false;
	}
	
	bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written)
	{
		if(child_in_write)
			return WriteFile((HANDLE)child_in_write, (LPVOID)buf, (DWORD)bytes_to_write, (LPDWORD)bytes_written, NULL);
		return false;
	}
	//}
	#else
	//{ Unix
	process_killer pk;
	
	bool kill_on_destructor;
	
	process_manager() : kill_on_destructor(true) {}
	
	void kill(uint32_t exitcode = 0)
	{
		pk.kill(exitcode);
	}
	
	~process_manager()
	{
		if(kill_on_destructor)
			kill();
	}
	
	bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read)
	{
		return false;
	}
	
	bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written)
	{
		return false;
	}
	//}
	#endif
};

struct child_process_handler
{
	#ifdef _WIN32
	//{
	HANDLE in;
	HANDLE out;
	
	bool valid;
	
	bool init()
	{
		out = GetStdHandle(STD_OUTPUT_HANDLE);
		in = GetStdHandle(STD_INPUT_HANDLE);
		valid = (out != INVALID_HANDLE_VALUE && in != INVALID_HANDLE_VALUE);
		return valid;
	}
	
	child_process_handler()
	{
		init();
	}
	
	bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read)
	{
		if(valid && in)
			return ReadFile((HANDLE)in, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read, NULL);
		return false;
	}
	
	bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written)
	{
		if(valid && out)
			return WriteFile((HANDLE)out, (LPVOID)buf, (DWORD)bytes_to_write, (LPDWORD)bytes_written, NULL);
		return false;
	}
	//}
	#else
	//{
	bool valid;
	
	bool init()
	{
		return valid = false;
	}
	
	child_process_handler()
	{
		init();
	}
	
	bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read)
	{
		return false;
	}
	
	bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written)
	{
		return false;
	}
	//}
	#endif
};

process_killer launch_process(char const* relative_path);
process_manager* launch_piped_process(char const* relative_path);

#endif

