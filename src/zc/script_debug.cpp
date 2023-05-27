#include "script_debug.h"
#include "zc/ffscript.h"
#include "ConsoleLogger.h"
#include <fmt/format.h>

extern CConsoleLoggerEx zscript_coloured_console;
extern refInfo *ri;
extern script_data *curscript;
extern int32_t (*stack)[MAX_SCRIPT_REGISTERS];
std::string ZASMVarToString(int32_t arg);

bool DEBUG_PRINT_ZASM;
bool DEBUG_JIT_PRINT_ASM;
bool DEBUG_JIT_EXIT_ON_COMPILE_FAIL;

// 0 for off, 1 for per-script execution, 2 for per-instruction.
static int runtime_debug = 0;
static bool debug_to_file = true;
static bool debug_to_console = false;
static std::string debug_file_path;
static ALLEGRO_FILE *debug_file = nullptr;
static ALLEGRO_FILE *debug_file_cur_frame = nullptr;
static ALLEGRO_FILE *debug_file_zasm = nullptr;
static int debug_file_zasm_cur_id = -1;
static int debug_file_index = 0;

static void script_debug_print_impl(int32_t attributes, const char *str)
{
	if (debug_file)
	{
		al_fputs(debug_file, str);
	}

	if (debug_to_console && zscript_debugger)
	{
		zscript_coloured_console.safeprint(attributes, str);
	}
}

int script_debug_is_runtime_debugging()
{
	if (!debug_to_file && !debug_to_console)
	{
		return 0;
	}

	return runtime_debug;
}

void script_debug_set_file_type(int type)
{
	if (!debug_to_file)
		return;
	if (type != 0 && type != 1)
		return;

	if (debug_file)
		al_fflush(debug_file);

	if (type == 0)
	{
		int counter = replay_is_active() ? replay_get_frame() : frame;
		if (!debug_file_cur_frame || counter != debug_file_index)
		{
			if (debug_file_cur_frame)
			{
				al_fclose(debug_file_cur_frame);
			}
			std::string dir = fmt::format("zscript-debug/{}", counter / 1000);
			al_make_directory(dir.c_str());
			debug_file_path = fmt::format("{}/debug-{}.txt", dir, counter);
			debug_file_cur_frame = al_fopen(debug_file_path.c_str(), "w");
			debug_file_index = counter;
		}
		debug_file = debug_file_cur_frame;
	}
	else
	{
		if (!debug_file_zasm || debug_file_zasm_cur_id != curscript->debug_id)
		{
			if (debug_file_zasm)
			{
				al_fclose(debug_file_zasm);
			}

			std::string dir = fmt::format("zscript-debug/zasm/{}", get_filename(qstpath));
			al_make_directory(dir.c_str());
			if (curscript->meta.script_name.empty())
				debug_file_path = fmt::format("{}/zasm-{}.txt", dir, curscript->debug_id);
			else
				debug_file_path = fmt::format("{}/zasm-{}-{}.txt", dir, curscript->debug_id, curscript->meta.script_name);
			debug_file_zasm = al_fopen(debug_file_path.c_str(), "w");
			debug_file_zasm_cur_id = curscript->debug_id;
		}
		debug_file = debug_file_zasm;
	}
}

void script_debug_printf(int32_t attributes, const char *format, ...)
{
	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	int ret = vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
	buffer[vbound(ret, 0, 1023)] = 0;

	script_debug_print_impl(attributes, buffer);
}

void script_debug_printf(const char *format, ...)
{
	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	int ret = vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
	buffer[vbound(ret, 0, 1023)] = 0;

	script_debug_print_impl(0, buffer);
}

void script_debug_print(const char *str)
{
	script_debug_print_impl(0, str);
}

void script_debug_print(int32_t attributes, const char *str)
{
	script_debug_print_impl(attributes, str);
}

void script_debug_print_command(int i)
{
	word scommand = curscript->zasm[i].command;
	int32_t arg1 = curscript->zasm[i].arg1;
	int32_t arg2 = curscript->zasm[i].arg2;
	script_command c = get_script_command(scommand);

	script_debug_printf(CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
							CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
						"%14s", c.name);
	if (c.args >= 1)
	{
		if (c.arg1_type == 0)
		{
			script_debug_printf(CConsoleLoggerEx::COLOR_WHITE |
									CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
								"\t %s", ZASMVarToString(arg1).c_str());
		}
		else
		{
			script_debug_printf(CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
									CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
								"\t %d", arg1);
		}
	}
	if (c.args >= 2)
	{
		script_debug_print(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK, ", ");
		if (c.arg2_type == 0)
		{
			script_debug_printf(CConsoleLoggerEx::COLOR_WHITE |
									CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
								"%s", ZASMVarToString(arg2).c_str());
		}
		else
		{
			script_debug_printf(CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
									CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
								"%d", arg2);
		}
	}
	script_debug_print("\n");
}

std::string script_debug_command_to_string(word scommand, int32_t arg1, int32_t arg2)
{
	std::stringstream ss;
	script_command c = get_script_command(scommand);

	ss << c.name;
	if (c.args >= 1)
	{
		if (c.arg1_type == 0)
		{
			ss << "\t " << ZASMVarToString(arg1);
		}
		else
		{
			ss << "\t " << arg1;
		}
	}
	if (c.args >= 2)
	{
		ss << ", ";
		if (c.arg2_type == 0)
		{
			ss << ZASMVarToString(arg2);
		}
		else
		{
			ss << arg2;
		}
	}

	return ss.str();
}

std::string script_debug_command_to_string(word scommand)
{
	return get_script_command(scommand).name;
}

std::string script_debug_registers_and_stack_to_string()
{
	std::stringstream ss;

	ss << "D:\t";
	for (int i = 0; i < 8; i++)
	{
		ss << ri->d[i] << " ";
	}
	ss << "\n";

	ss << "A:\t";
	for (int i = 0; i < 2; i++)
	{
		ss << ri->a[i] << " ";
	}
	ss << "\n";

	ss << "stack:\t";
	if (ri->sp > 0)
	{
		for (int i = 1023; i >= ri->sp; i--)
		{
			ss << (*stack)[i] << " ";
		}
	}
	ss << "\n";

	return ss.str();
}

void script_debug_pre_command()
{
	// if (replay_get_frame() < 4034-100) return;
	// if (curscript->debug_id != 5134) return;

	// This is only to match the behavior in jitted code, where comparison instructions
	// must be grouped together.
	static bool supress_output = false;

	int command = curscript->zasm[ri->pc].command;
	if (!command_uses_comparison_result(command))
		supress_output = false;
	if (supress_output)
		return;

	int f = CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK;
	script_debug_printf(f, "pc:\t%d\n", ri->pc);
	script_debug_print(f, script_debug_registers_and_stack_to_string().c_str());

	script_debug_print_command(ri->pc);

	if (command == COMPAREV || command == COMPARER)
	{
		supress_output = true;
		for (int j = ri->pc + 1; curscript->zasm[j].command != 0xFFFF; j++)
		{
			if (!command_uses_comparison_result(curscript->zasm[j].command))
				break;
			script_debug_print_command(j);
		}
	}

	if (debug_file)
		al_fflush(debug_file);
}

void script_debug_print_zasm(script_data *script)
{
	extern byte curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	size_t size = script->size();
	script_debug_print("ZASM:\n\n");
	script_debug_printf(
		CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
		"script id: %d\nname: %s\ntype: %d\nnum: %d\nindex: %d\n\n", script->debug_id, script->meta.script_name.c_str(), (int)curScriptType, (int)curScriptNum, curScriptIndex);
	for (size_t i = 0; i < size; i++)
	{
		script_debug_printf(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_INTENSITY |
								CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
							"%4d: ", i);
		script_debug_print_command(i);
	}
	if (debug_file)
		al_fflush(debug_file);
}
