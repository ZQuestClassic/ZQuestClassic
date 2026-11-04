#include "base/crash_backtrace.h"

#ifdef _WIN32

#include <windows.h>
#include <dbghelp.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Everything the handler needs is set up front, since the crashed process may not be able to
// allocate or read the environment safely.
static char backtrace_file_path[MAX_PATH];
static wchar_t symbol_search_path[MAX_PATH];
static LPTOP_LEVEL_EXCEPTION_FILTER previous_filter;
static volatile LONG handling_crash;

// DbgHelp is loaded at runtime, so the executables don't import functions from it that
// Windows 7's copy may lack (see scripts/check_windows_imports.py) just for CI.
static decltype(&SymSetOptions) pSymSetOptions;
static decltype(&SymInitializeW) pSymInitializeW;
static decltype(&StackWalk64) pStackWalk64;
static decltype(&SymFunctionTableAccess64) pSymFunctionTableAccess64;
static decltype(&SymGetModuleBase64) pSymGetModuleBase64;
static decltype(&SymFromAddr) pSymFromAddr;
static decltype(&SymGetLineFromAddr64) pSymGetLineFromAddr64;

static bool load_dbghelp()
{
	HMODULE dbghelp = LoadLibraryW(L"dbghelp.dll");
	if (!dbghelp)
		return false;

	pSymSetOptions = (decltype(pSymSetOptions))GetProcAddress(dbghelp, "SymSetOptions");
	pSymInitializeW = (decltype(pSymInitializeW))GetProcAddress(dbghelp, "SymInitializeW");
	pStackWalk64 = (decltype(pStackWalk64))GetProcAddress(dbghelp, "StackWalk64");
	pSymFunctionTableAccess64 = (decltype(pSymFunctionTableAccess64))GetProcAddress(dbghelp, "SymFunctionTableAccess64");
	pSymGetModuleBase64 = (decltype(pSymGetModuleBase64))GetProcAddress(dbghelp, "SymGetModuleBase64");
	pSymFromAddr = (decltype(pSymFromAddr))GetProcAddress(dbghelp, "SymFromAddr");
	pSymGetLineFromAddr64 = (decltype(pSymGetLineFromAddr64))GetProcAddress(dbghelp, "SymGetLineFromAddr64");
	return pSymSetOptions && pSymInitializeW && pStackWalk64 && pSymFunctionTableAccess64
		&& pSymGetModuleBase64 && pSymFromAddr && pSymGetLineFromAddr64;
}

static void write_str(HANDLE out, const char* str)
{
	DWORD written;
	WriteFile(out, str, (DWORD)strlen(str), &written, nullptr);
}

static void print_backtrace(const CONTEXT* crash_context, const char* reason)
{
	// An abort() from inside the exception filter (or a second crashing thread) would print again.
	if (InterlockedExchange(&handling_crash, 1))
		return;

	HANDLE out = INVALID_HANDLE_VALUE;
	if (backtrace_file_path[0])
		out = CreateFileA(backtrace_file_path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (out == INVALID_HANDLE_VALUE)
		out = GetStdHandle(STD_ERROR_HANDLE);

	char line[2048];
	snprintf(line, sizeof(line), "crashed: %s\n", reason);
	write_str(out, line);

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	pSymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
	if (!pSymInitializeW(process, symbol_search_path[0] ? symbol_search_path : nullptr, TRUE))
		write_str(out, "(could not load symbols)\n");

	// StackWalk64 updates the context as it unwinds.
	CONTEXT context = *crash_context;
	STACKFRAME64 frame = {};
	DWORD machine;
#if defined(_M_X64)
	machine = IMAGE_FILE_MACHINE_AMD64;
	frame.AddrPC.Offset = context.Rip;
	frame.AddrFrame.Offset = context.Rbp;
	frame.AddrStack.Offset = context.Rsp;
#elif defined(_M_ARM64)
	machine = IMAGE_FILE_MACHINE_ARM64;
	frame.AddrPC.Offset = context.Pc;
	frame.AddrFrame.Offset = context.Fp;
	frame.AddrStack.Offset = context.Sp;
#elif defined(_M_IX86)
	machine = IMAGE_FILE_MACHINE_I386;
	frame.AddrPC.Offset = context.Eip;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrStack.Offset = context.Esp;
#else
#error "unknown architecture"
#endif
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;

	for (int i = 0; i < 64; i++)
	{
		if (!pStackWalk64(machine, process, thread, &frame, &context, nullptr, pSymFunctionTableAccess64, pSymGetModuleBase64, nullptr))
			break;

		DWORD64 address = frame.AddrPC.Offset;
		if (!address)
			break;

		// Every frame but the first is a return address, which points past the call.
		DWORD64 lookup_address = i == 0 ? address : address - 1;

		char module_name[MAX_PATH] = "?";
		DWORD64 module_base = pSymGetModuleBase64(process, address);
		if (module_base && GetModuleFileNameA((HMODULE)module_base, module_name, sizeof(module_name)))
		{
			if (const char* slash = strrchr(module_name, '\\'))
				memmove(module_name, slash + 1, strlen(slash + 1) + 1);
		}

		alignas(SYMBOL_INFO) char symbol_buffer[sizeof(SYMBOL_INFO) + 512] = {};
		SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbol_buffer;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = 512;
		DWORD64 displacement = 0;

		IMAGEHLP_LINE64 source = {};
		source.SizeOfStruct = sizeof(source);
		DWORD line_displacement = 0;

		if (pSymFromAddr(process, lookup_address, &displacement, symbol))
		{
			if (pSymGetLineFromAddr64(process, lookup_address, &line_displacement, &source))
				snprintf(line, sizeof(line), "#%-2d %s!%s [%s:%lu]\n", i, module_name, symbol->Name, source.FileName, source.LineNumber);
			else
				snprintf(line, sizeof(line), "#%-2d %s!%s+0x%llx\n", i, module_name, symbol->Name, (unsigned long long)displacement);
		}
		else
		{
			snprintf(line, sizeof(line), "#%-2d %s+0x%llx\n", i, module_name, (unsigned long long)(address - module_base));
		}
		write_str(out, line);
	}

	if (out != GetStdHandle(STD_ERROR_HANDLE))
		CloseHandle(out);

	// The C runtime drops whatever is still buffered when the process dies this way, which on
	// CI (where stdout is a pipe, so fully buffered) is usually all of the program's output.
	fflush(nullptr);
}

static LONG WINAPI on_unhandled_exception(EXCEPTION_POINTERS* info)
{
	char reason[128];
	snprintf(reason, sizeof(reason), "unhandled exception 0x%08lX at %p",
		info->ExceptionRecord->ExceptionCode, info->ExceptionRecord->ExceptionAddress);
	print_backtrace(info->ContextRecord, reason);

	// Let the C runtime's filter carry on as usual (for a C++ exception, it calls std::terminate).
	if (previous_filter)
		return previous_filter(info);
	return EXCEPTION_CONTINUE_SEARCH;
}

static void on_abort(int)
{
	CONTEXT context;
	RtlCaptureContext(&context);
	print_backtrace(&context, "abort() was called");
}

void install_crash_backtrace_handler()
{
	if (!load_dbghelp())
		return;

	if (const char* path = std::getenv("ZC_CRASH_BACKTRACE_FILE"))
		strncpy(backtrace_file_path, path, sizeof(backtrace_file_path) - 1);

	// The .pdb files are next to the executable. DbgHelp would otherwise only look where the
	// linker wrote them, which is a different machine for CI's test jobs.
	DWORD len = GetModuleFileNameW(nullptr, symbol_search_path, MAX_PATH);
	if (len && len < MAX_PATH)
	{
		if (wchar_t* slash = wcsrchr(symbol_search_path, L'\\'))
			*slash = L'\0';
	}
	else
	{
		symbol_search_path[0] = L'\0';
	}

	previous_filter = SetUnhandledExceptionFilter(on_unhandled_exception);
	signal(SIGABRT, on_abort);
}

#else

void install_crash_backtrace_handler()
{
}

#endif
