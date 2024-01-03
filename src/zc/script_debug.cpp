#include "zc/script_debug.h"
#include "allegro5/file.h"
#include "base/zapp.h"
#include "zc/ffscript.h"
#include "zc/replay.h"
#include "zc/zasm_utils.h"
#include "zconsole/ConsoleLogger.h"
#include <fmt/format.h>
#include <iomanip>

extern refInfo *ri;
std::string ZASMVarToString(int32_t arg);
std::string ZASMArgToString(int32_t arg, int32_t arg_ty);

bool DEBUG_JIT_PRINT_ASM;
bool DEBUG_JIT_EXIT_ON_COMPILE_FAIL;

bool DEBUG_PRINT_TO_FILE;
bool DEBUG_PRINT_TO_CONSOLE;

ScriptDebugHandle::ScriptDebugHandle(OutputSplit output_split, script_data* script)
{
	this->output_split = output_split;
	this->script = script;
	this->file = nullptr;
	this->file_counter = 0;

	update_file();
}

ScriptDebugHandle::ScriptDebugHandle(ScriptDebugHandle&& rhs)
	: file { std::exchange(rhs.file, nullptr) }
{
	output_split = rhs.output_split;
	script = rhs.script;
	file_counter = rhs.file_counter;
}

ScriptDebugHandle& ScriptDebugHandle::operator=(ScriptDebugHandle&& rhs)
{
	if (this != &rhs)
	{
		if (file)
			al_fclose(file);
        file = std::exchange(rhs.file, nullptr);
		output_split = rhs.output_split;
		script = rhs.script;
		file_counter = rhs.file_counter;
	}

	return *this;
}

ScriptDebugHandle::~ScriptDebugHandle()
{
	if (file)
		al_fclose(file);
}

void ScriptDebugHandle::update_file()
{
	if (!DEBUG_PRINT_TO_FILE)
		return;

	static auto output_folder = get_flag_string("-script-runtime-debug-folder").value_or("zscript-debug");

	if (file)
		al_fflush(file);

	if (output_split == OutputSplit::ByFrame)
	{
		int counter = replay_is_active() ? replay_get_frame() : frame;
		if (!file || counter != file_counter)
		{
			if (file)
			{
				al_fclose(file);
			}

			std::string script_name = zasm_script_unique_name(script);
			std::string dir = fmt::format("{}/{}/{}", output_folder, script_name, counter / 1000);

			al_make_directory(dir.c_str());
			std::string path = fmt::format("{}/debug-{}.txt", dir, counter);

			file = al_fopen(path.c_str(), "w");
			file_counter = counter;
		}
	}
	else if (output_split == OutputSplit::ByScript && !file)
	{
		std::string dir = fmt::format("{}/zasm/{}", output_folder, get_filename(qstpath));
		std::string path;
		
		if (script->meta.script_name.empty())
			path = fmt::format("{}/zasm-{}-{}.txt", dir, ScriptTypeToString(script->id.type), script->id.index);
		else
			path = fmt::format("{}/zasm-{}-{}-{}.txt", dir, ScriptTypeToString(script->id.type), script->id.index, script->meta.script_name);
		al_make_directory(dir.c_str());
		file = al_fopen(path.c_str(), "w");
	}
}

void ScriptDebugHandle::_print(int32_t attributes, const char *str)
{
	extern CConsoleLoggerEx zscript_coloured_console;

	if (file)
	{
		al_fputs(file, str);
	}

	if (DEBUG_PRINT_TO_CONSOLE && zscript_debugger)
	{
		zscript_coloured_console.safeprint(attributes, str);
	}
}

void ScriptDebugHandle::printf(int32_t attributes, const char *format, ...)
{
	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	int ret = vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
	buffer[vbound(ret, 0, 1023)] = 0;

	_print(attributes, buffer);
}

void ScriptDebugHandle::printf(const char *format, ...)
{
	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	int ret = vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
	buffer[vbound(ret, 0, 1023)] = 0;

	_print(0, buffer);
}

void ScriptDebugHandle::print(const char *str)
{
	_print(0, str);
}

void ScriptDebugHandle::print(int32_t attributes, const char *str)
{
	_print(attributes, str);
}

void ScriptDebugHandle::print_command(int i)
{
	auto& op = script->zasm[i];
	word scommand = op.command;
	int args[] = {op.arg1, op.arg2, op.arg3};
	script_command c = get_script_command(scommand);

	printf(CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY |
							CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
						"%14s", c.name);
	if (c.args >= 1)
	{
		printf(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
			"\t %s", ZASMArgToString(args[0], c.arg_type[0]).c_str());
	}
	for(int q = 1; q < c.args; ++q)
	{
		print(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK, ", ");
		printf(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
			"\t %s", ZASMArgToString(args[q], c.arg_type[q]).c_str());
	}
	if (c.arr_type)
	{
		print(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK, ", ");
		if(c.arr_type == 1)
		{
			printf(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
				"\t %s", op.strptr->c_str());
		}
		else //if(c.arr_type == 2)
		{
			printf(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
				"\t %s", fmt::format("{{ {} }}", fmt::join(*op.vecptr, ", ")).c_str());
		}
	}
	print("\n");
}

void ScriptDebugHandle::print_zasm()
{
	printf(
		CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY |
			CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
		"script type: %s\nindex: %d\nname: %s\n\n", ScriptTypeToString(script->id.type), script->id.index, script->meta.script_name.c_str());
	for (size_t i = 0; i < script->size; i++)
	{
		printf(CConsoleLoggerEx::COLOR_WHITE | CConsoleLoggerEx::COLOR_INTENSITY |
								CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,
							"%4d: ", i);
		print_command(i);
	}
	if (file)
		al_fflush(file);
}

void ScriptDebugHandle::pre_command()
{
	static int frame = get_flag_int("-script-runtime-debug-frame").value_or(-1);
	if (frame != -1 && replay_get_frame() != frame)
		return;
	// if (script->id != {ScriptType::Player, 2}) return;

	// This is only to match the behavior in jitted code, where comparison instructions
	// must be grouped together.
	static bool supress_output = false;

	int command = script->zasm[ri->pc].command;
	if (!command_uses_comparison_result(command))
		supress_output = false;
	if (supress_output)
		return;

	if (frame != -1)
	{
		int i = ri->pc;
		auto& op = script->zasm[i];

		std::string line = script_debug_registers_and_stack_to_string();
		util::replchar(line, '\n', ' ');

		replay_step_comment(fmt::format("{} {} | {}", i, script_debug_command_to_string(command, op.arg1, op.arg2, op.arg3, op.vecptr, op.strptr), line));

		if (command == COMPAREV || command == COMPARER)
		{
			supress_output = true;
		}
		return;
	}

	int f = CConsoleLoggerEx::COLOR_GREEN | CConsoleLoggerEx::COLOR_INTENSITY | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK;
	printf(f, "pc:\t%d\n", ri->pc);
	print(f, script_debug_registers_and_stack_to_string().c_str());

	print_command(ri->pc);

	if (command == COMPARER || command == COMPAREV || command == COMPAREV2)
	{
		supress_output = true;
		for (int j = ri->pc + 1; script->zasm[j].command != 0xFFFF; j++)
		{
			if (!command_uses_comparison_result(script->zasm[j].command))
				break;
			print_command(j);
		}
	}

	if (file)
		al_fflush(file);
}

// 0 for off, 1 for per-script execution, 2 for per-instruction.
int script_debug_is_runtime_debugging()
{
	if (!DEBUG_PRINT_TO_FILE && !DEBUG_PRINT_TO_CONSOLE)
	{
		return 0;
	}

	static int runtime_debug = get_flag_int("-script-runtime-debug").value_or(0);
	return runtime_debug;
}

std::string script_debug_command_to_string(word scommand, int32_t arg1, int32_t arg2, int32_t arg3, vector<int>* argvec, string* argstr)
{
	std::stringstream ss;
	script_command c = get_script_command(scommand);

	int args[] = {arg1, arg2, arg3};
	#define SS_WIDTH(w) std::setw(w) << std::setfill(' ') << std::left
	ss << SS_WIDTH(15) << c.name;
	if (c.args >= 1)
	{
		ss << " " << SS_WIDTH(16) << ZASMArgToString(args[0], c.arg_type[0]);
	}
	for(int q = 1; q < c.args; ++q)
	{
		ss << SS_WIDTH(7) << ZASMArgToString(args[q], c.arg_type[q]);
	}
	if (c.arr_type)
	{
		ss << SS_WIDTH(7);
		if(c.arr_type == 1)
		{
			ss << *argstr;
		}
		else //if(c.arr_type == 2)
		{
			ss << fmt::format("{{ {} }}", fmt::join(*argvec, ", "));
		}
	}

	return ss.str();
}

std::string script_debug_command_to_string(word scommand, int32_t arg1, int32_t arg2, int32_t arg3)
{
	std::stringstream ss;
	script_command c = get_script_command(scommand);
	
	int args[] = {arg1, arg2, arg3};
	#define SS_WIDTH(w) std::setw(w) << std::setfill(' ') << std::left
	ss << SS_WIDTH(15) << c.name;
	if (c.args >= 1)
	{
		ss << " " << SS_WIDTH(16) << ZASMArgToString(args[0], c.arg_type[0]);
	}
	for(int q = 1; q < c.args; ++q)
	{
		ss << SS_WIDTH(7) << ZASMArgToString(args[q], c.arg_type[q]);
	}

	return ss.str();
}

std::string script_debug_command_to_string(const ffscript& c)
{
	return script_debug_command_to_string(c.command, c.arg1, c.arg2, c.arg3);
}

std::string script_debug_command_to_string(word scommand)
{
	return get_script_command(scommand).name;
}

std::string script_debug_registers_and_stack_to_string()
{
	extern int32_t (*stack)[MAX_SCRIPT_REGISTERS];

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
