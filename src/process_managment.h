#ifndef ZC_PROCESS_MANAGEMENT_H
#define ZC_PROCESS_MANAGEMENT_H

#include <cstdio>
#include <string>

static uint32_t __dummy_;

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

struct io_manager
{
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL) = 0;
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL) = 0;
};

struct process_manager : public io_manager
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
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL)
	{
		if(!bytes_read) bytes_read = &__dummy_;
		if(child_out_read)
			return ReadFile((HANDLE)child_out_read, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read, NULL);
		return false;
	}
	
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL)
	{
		if(!bytes_written) bytes_written = &__dummy_;
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
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL)
	{
		return false;
	}
	
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL)
	{
		return false;
	}
	//}
	#endif
};

struct child_process_handler : public io_manager
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
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL)
	{
		if(!bytes_read) bytes_read = &__dummy_;
		if(valid && in)
			return ReadFile((HANDLE)in, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read, NULL);
		return false;
	}
	
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL)
	{
		if(!bytes_written) bytes_written = &__dummy_;
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
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL)
	{
		return false;
	}
	
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL)
	{
		return false;
	}
	//}
	#endif
};

process_killer launch_process(char const* relative_path);
process_manager* launch_piped_process(char const* relative_path);

#endif

