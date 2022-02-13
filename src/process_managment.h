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
	#ifdef _WIN32
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
	#else
	int32_t pid;
	void kill(uint32_t exitcode = 0);
	void init(int32_t pr_id, uint32_t exitcode = 0)
	{
		if(pid)
			kill(exitcode);
		pid = pr_id;
	}
	process_killer(int32_t pr_id) : pid(pr_id) {}
	process_killer() : pid(0) {}
	#endif
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
	HANDLE write_handle;
	HANDLE read_handle;
	HANDLE wr_2;
	HANDLE re_2;
	PROCESS_INFORMATION pi;
	
	process_killer pk;
	
	bool kill_on_destructor;
	
	process_manager() : write_handle(NULL),
		read_handle(NULL), kill_on_destructor(true),
		wr_2(NULL), re_2(NULL)
	{}
	
	void kill(uint32_t exitcode = 0)
	{
		pk.kill(exitcode);
	}
	
	~process_manager()
	{
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
		if(kill_on_destructor)
			kill();
	}
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL)
	{
		if(!bytes_read) bytes_read = &__dummy_;
		if(read_handle)
			return ReadFile((HANDLE)read_handle, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read, NULL);
		return false;
	}
	
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL)
	{
		if(!bytes_written) bytes_written = &__dummy_;
		if(write_handle)
			return WriteFile((HANDLE)write_handle, (LPVOID)buf, (DWORD)bytes_to_write, (LPDWORD)bytes_written, NULL);
		return false;
	}
	//}
	#else
	//{ Unix
	FILE* write_handle, read_handle;
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
		if(!read_handle) return false;
		size_t ret = fread(buf, 1, bytes_to_read, read_handle);
		if(bytes_read) *bytes_read = ret;
		return ret>0;
	}
	
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL)
	{
		if(!write_handle) return false;
		size_t ret = fwrite(buf, 1, bytes_to_write, write_handle);
		if(bytes_written) *bytes_written = ret;
		return ret==bytes_to_write;
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
	
	bool init()
	{
		out = GetStdHandle(STD_OUTPUT_HANDLE);
		in = GetStdHandle(STD_INPUT_HANDLE);
		return (out != INVALID_HANDLE_VALUE && in != INVALID_HANDLE_VALUE);
	}
	
	child_process_handler()
	{
		init();
	}
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL)
	{
		if(!bytes_read) bytes_read = &__dummy_;
		if(in != INVALID_HANDLE_VALUE)
			return ReadFile((HANDLE)in, (LPVOID)buf, (DWORD)bytes_to_read, (LPDWORD)bytes_read, NULL);
		return false;
	}
	
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL)
	{
		if(!bytes_written) bytes_written = &__dummy_;
		if(out != INVALID_HANDLE_VALUE)
			return WriteFile((HANDLE)out, (LPVOID)buf, (DWORD)bytes_to_write, (LPDWORD)bytes_written, NULL);
		return false;
	}
	//}
	#else
	//{
	FILE *read_handle, *write_handle;
	
	bool init()
	{
		read_handle = stdin;
		write_handle = stdout;
		return (out && in);
	}
	
	child_process_handler()
	{
		init();
	}
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL)
	{
		if(!read_handle) return false;
		size_t ret = fread(buf, 1, bytes_to_read, read_handle);
		if(bytes_read) *bytes_read = ret;
		return ret>0;
	}
	
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL)
	{
		if(!write_handle) return false;
		size_t ret = fwrite(buf, 1, bytes_to_write, write_handle);
		if(bytes_written) *bytes_written = ret;
		return ret==bytes_to_write;
	}
	//}
	#endif
};

process_killer launch_process(char const* relative_path, char const** argv = NULL);
process_manager* launch_piped_process(char const* relative_path, char const** argv = NULL);

#endif

