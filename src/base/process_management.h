#ifndef ZC_PROCESS_MANAGEMENT_H_
#define ZC_PROCESS_MANAGEMENT_H_

#include <cstdio>
#include <string>
#include <stdint.h>
#include <vector>
#include <map>
#include "base/ints.h"

static uint32_t __dummy_;

#ifdef _WIN32
#define NOGDI
#define WinMain __win_main
#include "windows.h"
#undef WinMain
#undef NOGDI
#else
#include <unistd.h>
#endif

#ifdef _WIN32
	#define ZPLAYER_FILE "zplayer.exe"
	#define ZEDITOR_FILE "zeditor.exe"
	#define ZSCRIPT_FILE "zscript.exe"
	#define ZLAUNCHER_FILE "zlauncher.exe"
	#define ZUPDATER_FILE "zupdater.exe"
	#define PYTHON "pythonw"
#elif defined(ALLEGRO_LINUX)
	#define ZPLAYER_FILE "zplayer"
	#define ZEDITOR_FILE "zeditor"
	#define ZSCRIPT_FILE "zscript"
	#define ZLAUNCHER_FILE "zlauncher"
	#define ZUPDATER_FILE "zupdater"
	#define PYTHON "python"
#else
	#define ZPLAYER_FILE "./zplayer"
	#define ZEDITOR_FILE "./zeditor"
	#define ZSCRIPT_FILE "./zscript"
	#define ZLAUNCHER_FILE "./zlauncher"
	#define ZUPDATER_FILE "./zupdater"
	#define PYTHON "python"
#endif

class zc_io_exception : public std::exception
{
public:
	enum
	{
		IO_NIL,
		IO_TIMEOUT,
		IO_DEAD,
		IO_UNKNOWN,
		IO_MAX
	};
	const char * what() const noexcept override {
		return msg.c_str();
	}
	zc_io_exception(std::string const& msg, int type) : msg(msg), type(type)
	{}
	static zc_io_exception dead()
	{
		return zc_io_exception("Error: Process Dead",IO_TIMEOUT);
	}
	static zc_io_exception timeout()
	{
		return zc_io_exception("Error: IO Timeout Ocurred",IO_TIMEOUT);
	}
	static zc_io_exception unknown()
	{
		return zc_io_exception("Error: Unknown IO error",IO_UNKNOWN);
	}
	int getType() const {return type;}
private:
	std::string msg;
	int type;
};

//manages killing a process
struct process_killer
{
	#ifdef _WIN32
	void* process_handle;
	bool kill(uint32_t exitcode = 0);
	void init_process(void* h, uint32_t exitcode = 0);
	bool is_alive() const;
	process_killer(void* h) : process_handle(h) {}
	process_killer() : process_handle(NULL) {}
	#else
	int32_t pid;
	bool kill();
	void init_process(int32_t pr_id);
	process_killer(int32_t pr_id) : pid(pr_id) {}
	process_killer() : pid(0) {}
	bool is_alive() const;
	#endif
};

//virtual handling for io
struct io_manager
{
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL) = 0;
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL) = 0;
	virtual bool is_alive() const = 0;
	word timeout_seconds = 30;
protected:
#ifdef _WIN32
	bool ProcessReadFile(HANDLE read_handle, LPVOID buf, DWORD bytes_to_read, LPDWORD bytes_read);
	bool ProcessWriteFile(HANDLE write_handle, LPVOID buf, DWORD bytes_to_write, LPDWORD bytes_written);
#endif
};


//The parent process's way of spawning and managing a child
struct process_manager : public io_manager, public process_killer
{
	bool kill_on_destructor;
	virtual bool is_alive() const
	{
		return process_killer::is_alive();
	}
	#ifdef _WIN32
	//{ Windows
	HANDLE write_handle;
	HANDLE read_handle;
	HANDLE wr_2;
	HANDLE re_2;
	PROCESS_INFORMATION pi;
	
	process_manager() : process_killer(), io_manager(),
		write_handle(NULL), read_handle(NULL),
		wr_2(NULL), re_2(NULL), kill_on_destructor(true)
	{}
	~process_manager();
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL);
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL);
	//}
	#else
	//{ Unix
	int write_handle;
	int read_handle;
	
	process_manager() : io_manager(), process_killer(),
		kill_on_destructor(true)
	{}
	
	~process_manager();
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL);
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL);
	//}
	#endif
};

//The child process's way of communicating with the parent
struct child_process_handler : public io_manager
{
	virtual bool is_alive() const
	{
		return true;
	}
	#ifdef _WIN32
	//{
	HANDLE in;
	HANDLE out;
	
	bool init();
	
	child_process_handler()
	{
		init();
	}
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL);
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL);
	//}
	#else
	//{
	int read_handle, write_handle;
	
	bool init();
	
	child_process_handler()
	{
		init();
	}
	
	virtual bool read(void* buf, uint32_t bytes_to_read, uint32_t* bytes_read = NULL);
	virtual bool write(void* buf, uint32_t bytes_to_write, uint32_t* bytes_written = NULL);
	//}
	#endif
};

process_killer launch_process(std::string file, const std::vector<std::string>& args = {});
process_manager* launch_piped_process(std::string file, std::string pipename, const std::vector<std::string>& args = {});
void launch_file(std::string const& file);
bool run_and_get_output(std::string file, const std::vector<std::string>& args, std::string& output);
std::map<std::string, std::string> parse_output_map(std::string output);

#endif

