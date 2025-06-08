// TODO: do not link allegro w/ zscript compiler.

#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include "fmt/core.h"
#include "parser/AST.h"
#include "parser/CompileError.h"
#include "parser/CompileOption.h"
#include "zasm/table.h"
#include "zasm/serialize.h"
#include "zc/ffscript.h"
#include "base/util.h"
#include "parser/ZScript.h"
#include "parser/config.h"
#include "zconfig.h"
#include "zconsole/ConsoleLogger.h"
#include "zscrdata.h"
#include "base/zapp.h"
#include "base/qrs.h"
#include "base/zsys.h"
#include <nlohmann/json.hpp>

using namespace std::chrono_literals;
using json = nlohmann::ordered_json;

FFScript FFCore;

std::vector<std::string> ZQincludePaths;
std::string console_path;

extern byte monochrome_console;

io_manager* ConsoleWrite;

extern std::string input_script_filename;
extern uint32_t zscript_failcode;
extern bool zscript_error_out;
extern std::vector<Diagnostic>* current_diagnostics;

const int BUILDTM_YEAR = (
    __DATE__[7] == '?' ? 1900
    : (((__DATE__[7] - '0') * 1000 )
    + (__DATE__[8] - '0') * 100
    + (__DATE__[9] - '0') * 10
    + __DATE__[10] - '0'));

const int BUILDTM_MONTH = (
    __DATE__ [2] == '?' ? 1
    : __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6)
    : __DATE__ [2] == 'b' ? 2
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4)
    : __DATE__ [2] == 'y' ? 5
    : __DATE__ [2] == 'l' ? 7
    : __DATE__ [2] == 'g' ? 8
    : __DATE__ [2] == 'p' ? 9
    : __DATE__ [2] == 't' ? 10
    : __DATE__ [2] == 'v' ? 11
    : 12);

const int BUILDTM_DAY = (
    __DATE__[4] == '?' ? 1
    : ((__DATE__[4] == ' ' ? 0 :
    ((__DATE__[4] - '0') * 10)) + __DATE__[5] - '0'));

const int BUILDTM_HOUR = (
	(__TIME__[0]-'0')*10 +
	(__TIME__[1]-'0'));

const int BUILDTM_MINUTE = (
	(__TIME__[3]-'0')*10 +
	(__TIME__[4]-'0'));

const int BUILDTM_SECOND = (
	(__TIME__[6]-'0')*10 +
	(__TIME__[7]-'0'));

bool zparser_errored_out()
{
	return zscript_error_out;
}
void zparser_error_out(std::string message)
{
	zscript_error_out = true;

	if (!current_diagnostics || curfilename != input_script_filename) return;

	ZScript::LocationData loc{};
	loc.first_line = yylloc.first_line;
	loc.last_line = yylloc.last_line;
	loc.first_column = yylloc.first_column;
	loc.last_column = yylloc.last_column;
	loc.fname = input_script_filename;
	std::string context = getErrorContext(loc);
	if (!context.empty())
		zconsole_error(context);

	auto& diag = current_diagnostics->emplace_back();
	diag.severity = DiagnosticSeverity::Error;
	diag.message = message;
	diag.range.start.line = yylloc.first_line - 1;
	diag.range.start.character = yylloc.first_column - 1;
	diag.range.end.line = yylloc.last_line - 1;
	diag.range.end.character = yylloc.last_column - 1;
}

void zparser_warn_out(std::string message)
{
	if (!current_diagnostics || curfilename != input_script_filename) return;

	ZScript::LocationData loc{};
	loc.first_line = yylloc.first_line;
	loc.last_line = yylloc.last_line;
	loc.first_column = yylloc.first_column;
	loc.last_column = yylloc.last_column;
	loc.fname = input_script_filename;
	std::string context = getErrorContext(loc);
	if (!context.empty())
		zconsole_error(context);

	auto& diag = current_diagnostics->emplace_back();
	diag.severity = DiagnosticSeverity::Warning;
	diag.message = message;
	diag.range.start.line = yylloc.first_line - 1;
	diag.range.start.character = yylloc.first_column - 1;
	diag.range.end.line = yylloc.last_line - 1;
	diag.range.end.character = yylloc.last_column - 1;
}

static const int32_t WARN_COLOR = CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_GREEN;
static const int32_t ERR_COLOR = CConsoleLoggerEx::COLOR_RED;
static const int32_t INFO_COLOR = CConsoleLoggerEx::COLOR_WHITE;

void _console_print(char const* str, int32_t code)
{
	if(console_path.size())
	{
		FILE *console=fopen(console_path.c_str(), "a");
		if(ConsoleWrite)
			fprintf(console, "%s", str);
		else
			fprintf(console, "%s\n", str);
		fclose(console);
	}
	if(ConsoleWrite)
	{
		ConsoleWrite->write(&code, sizeof(int32_t));
		ConsoleWrite->read(&code, sizeof(int32_t));
	}
	else fprintf(stderr, "%s\n", str);
}
void zconsole_db(const char *format,...)
{
	//{
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	#ifdef WIN32
	 		ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#else
	 		ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#endif
	tmp[vbound(ret,0,1023)]=0;
	
	va_end(argList);
	//}
	_console_print(tmp, ZC_CONSOLE_DB_CODE);
}
void zconsole_db(std::string const& str)
{
	_console_print(str.c_str(), ZC_CONSOLE_DB_CODE);
}
void zconsole_warn(const char *format,...)
{
	//{
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	#ifdef WIN32
	 		ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#else
	 		ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#endif
	tmp[vbound(ret,0,1023)]=0;
	
	va_end(argList);
	//}
	_console_print(tmp, ZC_CONSOLE_WARN_CODE);
}
void zconsole_warn(std::string const& str)
{
	_console_print(str.c_str(), ZC_CONSOLE_WARN_CODE);
}
void zconsole_error(const char *format,...)
{
	//{
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	#ifdef WIN32
	 		ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#else
	 		ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#endif
	tmp[vbound(ret,0,1023)]=0;
	
	va_end(argList);
	//}
	_console_print(tmp, ZC_CONSOLE_ERROR_CODE);
}
void zconsole_error(std::string const& str)
{
	_console_print(str.c_str(), ZC_CONSOLE_ERROR_CODE);
}
void zconsole_info(const char *format,...)
{
	//{
	int32_t ret;
	char tmp[1024];
	
	va_list argList;
	va_start(argList, format);
	#ifdef WIN32
	 		ret = _vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#else
	 		ret = vsnprintf(tmp,sizeof(tmp)-1,format,argList);
	#endif
	tmp[vbound(ret,0,1023)]=0;
	
	va_end(argList);
	//}
	_console_print(tmp, ZC_CONSOLE_INFO_CODE);
}
void zconsole_info(std::string const& str)
{
	_console_print(str.c_str(), ZC_CONSOLE_INFO_CODE);
}
void zconsole_idle(dword seconds)
{
	if(ConsoleWrite)
	{
		int32_t code = ZC_CONSOLE_IDLE_CODE;
		ConsoleWrite->write(&code, sizeof(int32_t));
		ConsoleWrite->write(&seconds, sizeof(dword));
		ConsoleWrite->read(&code, sizeof(int32_t));
	}
}

static bool linked = true;
std::unique_ptr<ZScript::ScriptsData> compile(std::string script_path, bool metadata, bool docs)
{
	if(linked)
		zconsole_info("Compiling the editor script buffer...");
	else zconsole_info("Compiling '%s'", script_path.c_str());

	if (!std::filesystem::exists(script_path))
	{
		zconsole_error("%s", "Cannot open specified file!");
		zscript_failcode = -404;
		return NULL;
	}
	
	std::unique_ptr<ZScript::ScriptsData> res(ZScript::compile(script_path, metadata, docs));
	return res;
}

static std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> result;
	std::stringstream ss (s);
	std::string item;

	while (getline (ss, item, delim)) {
		result.push_back (item);
	}

	return result;
}

void updateIncludePaths()
{
	FILE* f = fopen("includepaths.txt", "r");
	char includePathString[MAX_INCLUDE_PATH_CHARS] = {0};
	if(f)
	{
		int32_t pos = 0;
		int32_t c;
		do
		{
			c = fgetc(f);
			if(c!=EOF) 
				includePathString[pos++] = c;
		}
		while(c!=EOF && pos<MAX_INCLUDE_PATH_CHARS);
		if(pos<MAX_INCLUDE_PATH_CHARS)
			includePathString[pos] = '\0';
		includePathString[MAX_INCLUDE_PATH_CHARS-1] = '\0';
		fclose(f);
	}
	else strcpy(includePathString, "include/;headers/;scripts/;");

	ZQincludePaths = split(includePathString, ';');
}

static void fill_result(json& data, int code, ZScript::ScriptsData* result)
{
	data["success"] = code == 0;
	if (code)
		data["code"] = code;
	data["diagnostics"] = result->diagnostics;
	if (!result->metadata.empty())
		data["metadata"] = result->metadata;
}

bool delay_asserts = false, ignore_asserts = false, is_json_output = false;
std::vector<std::filesystem::path> force_ignores;
int32_t main(int32_t argc, char **argv)
{
	common_main_setup(App::zscript, argc, argv);

	if (used_switch(argc, argv, "-print-zasm-commands"))
	{
		for (int i = 0; i < NUMCOMMANDS; i++)
		{
			auto sc = get_script_command(i);
			if (sc && std::string(sc->name) != "(null)")
				printf("%d %s\n", i, sc->name);
		}
		return 0;
	}

	if (used_switch(argc, argv, "-print-zasm-registers"))
	{
		for (int i = 0; i < NUMVARIABLES; i++)
		{
			std::string name = zasm_var_to_string(i);
			if (name != "(null)")
				printf("%d %s\n", i, name.c_str());
		}
		return 0;
	}

	linked = true;
	if (!used_switch(argc, argv, "-linked"))
	{
		if(used_switch(argc, argv, "-unlinked"))
		{
			linked = false;
		}
		else return 1;
	}
	if(used_switch(argc, argv, "-ignore_cassert"))
		delay_asserts = ignore_asserts = true;
	else if(used_switch(argc, argv, "-delay_cassert"))
		delay_asserts = true;
	
	if(auto index = used_switch(argc, argv, "-force_ignore"))
	{
		for(int q = index+1; q < argc; ++q)
		{
			if(argv[q][0] == '-') break;
			force_ignores.push_back((std::filesystem::current_path() / std::filesystem::path(argv[q])).lexically_normal());
		}
	}
	
	int32_t zasm_out_index = used_switch(argc, argv, "-zasm");
	bool zasm_out_append = used_switch(argc, argv, "-append");
	bool zasm_commented = used_switch(argc, argv, "-commented");
	string zasm_out = "";
	if(zasm_out_index)
		zasm_out = argv[zasm_out_index + 1];
	
	child_process_handler* cph = (linked ? new child_process_handler() : nullptr);
	ConsoleWrite = cph;

	if (ZScript::is_test())
	{
		bool result = zscript_load_base_config("base_config/zscript_test.cfg");
		assert(result);
	}
	else
	{
		if (!zscript_load_base_config("base_config/zscript.cfg"))
		{
			zconsole_error("%s", "Error: failed to load base config");
			return 1;
		}

		zscript_load_user_config("zscript.cfg");
	}

	bool has_qrs = false;
	if(int32_t qr_hex_index = used_switch(argc, argv, "-qr"))
	{
		has_qrs = true;
		std::string qr_hex = argv[qr_hex_index + 1];
		if (qr_hex.size() != QUESTRULES_NEW_SIZE * 2)
		{
			zconsole_error("Error: -qr hex string must be of length %d", QUESTRULES_NEW_SIZE * 2);
			return 1;
		}

		for (int i = 0; i < QUESTRULES_NEW_SIZE; i++)
		{
			char ch0 = qr_hex[2 * i];
			char ch1 = qr_hex[2 * i + 1];
			uint8_t nib0 = (ch0 & 0xF) + (ch0 >> 6) | ((ch0 >> 3) & 0x8);
			uint8_t nib1 = (ch1 & 0xF) + (ch1 >> 6) | ((ch1 >> 3) & 0x8);
			quest_rules[i] = (nib0 << 4) | nib1;
		}
		unpack_qrs();
	}

	int32_t script_path_index = used_switch(argc, argv, "-input");
	if (!script_path_index)
	{
		zconsole_error("%s", "Error: missing required flag: -input");
		return 1;
	}
	std::string script_path = argv[script_path_index + 1];

	int32_t console_path_index = used_switch(argc, argv, "-console");
	if (linked && !console_path_index)
	{
		zconsole_error("%s", "Error: missing required flag: -console");
		return 1;
	}
	if(console_path_index)
		console_path = argv[console_path_index + 1];
	else console_path = "";
	
	if(console_path.size())
	{
		FILE *console=fopen(console_path.c_str(), "w");
		fclose(console);
	}

	int32_t syncthing = 0;
	
	if(linked)
	{
		cph->write(&syncthing, sizeof(int32_t));
	}

	int32_t include_paths_index = used_switch(argc, argv, "-include");
	if (include_paths_index)
	{
		std::string include_paths = argv[include_paths_index+1];
		ZQincludePaths = split(include_paths, ';');
	}
	else
	{
		updateIncludePaths();
	}

	// Any errors will be printed to stdout.
	if(used_switch(argc, argv, "-delay"))
	{
		zconsole_info("%s","Pausing for debugger...");
		while(true)
		{
			zconsole_idle();
			std::this_thread::sleep_for(1s);
		}
	}

	bool parse_only = used_switch(argc, argv, "-parse-only") > 0;
	if (parse_only)
	{
		auto root(ZScript::parseFile(script_path));
		if (zscript_error_out)
			exit(1);
		if (!root.get())
			log_error(ZScript::CompileError::CantOpenSource(NULL));
		fmt::println("ok");
		exit(0);
	}

	bool do_json_output = used_switch(argc, argv, "-json") > 0;
	is_json_output = do_json_output;

	bool metadata = used_switch(argc, argv, "-metadata") > 0;
	int metadata_tmp_path_idx = used_switch(argc, argv, "-metadata-tmp-path");
	int metadata_orig_path_idx = used_switch(argc, argv, "-metadata-orig-path");
	if (metadata && metadata_tmp_path_idx > 0 && metadata_orig_path_idx > 0)
	{
		extern std::string metadata_tmp_path;
		extern std::string metadata_orig_path;
		metadata_tmp_path = argv[metadata_tmp_path_idx + 1];
		metadata_orig_path = argv[metadata_orig_path_idx + 1];
	}

	bool docs = used_switch(argc, argv, "-doc") > 0;
	
	ZScript::ScriptParser::initialize(has_qrs);
	unique_ptr<ZScript::ScriptsData> result(compile(script_path, metadata, docs));
	if(!result || !result->success)
		zconsole_info("%s", "Failure!");
	int32_t res = (result && result->success ? 0 : (zscript_failcode ? zscript_failcode : -1));

	if (result && !result->docs.empty())
	{
		printf("%s\n", result->docs.c_str());
		exit(0);
	}

	if(linked)
	{
		if(!res)
		{
			write_compile_data(result->zasm, result->scriptTypes, result->theScripts);
		}
		int32_t errorcode = ZC_CONSOLE_TERM_CODE;
		cph->write(&errorcode, sizeof(int32_t));
		cph->write(&res, sizeof(int32_t));
	}
	else
	{
		if(res)
		{
			if(res == -1)
				zconsole_warn("Compile finished with exit code '-1' (compiled with warnings)");
			else zconsole_error("Compile finished with exit code '%d' (compiled with errors)", res);
		}
		else zconsole_info("Compile finished with exit code '0' (success)");

		if (result && do_json_output)
		{
			json data;
			fill_result(data, res, result.get());
			std::cout << data.dump(2);
		}
	}
	if(!zasm_out.empty() && result)
	{
		if(FILE* outfile = fopen(zasm_out.c_str(), zasm_out_append ? "a" : "w"))
		{
			string str;
			write_script(result->zasm, str, zasm_commented, &result->theScripts);
			fwrite(str.c_str(), sizeof(char), str.size(), outfile);
			fclose(outfile);
		}
	}
	
	if(cph) delete cph;
	return res;
}
END_OF_MAIN()

extern "C" int compile_script(const char* script_path)
{
	bool has_qrs = false;
	ZScript::ScriptParser::initialize(has_qrs);
	updateIncludePaths();
	ZScript::CompileOption::OPT_NO_ERROR_HALT.setDefault(ZScript::OPTION_ON);

	bool metadata = true;
	bool docs = false;
	unique_ptr<ZScript::ScriptsData> result(compile(script_path, metadata, docs));
	int32_t code = (result && result->success ? 0 : (zscript_failcode ? zscript_failcode : -1));

	json data;
	fill_result(data, code, result.get());
	std::ofstream out("out.txt");
	out << data.dump(2);
	out.close();

	if (!result)
		zconsole_info("%s", "Failure!");
	return code;
}

// TODO: make this not needed to compile...
bool DragAspect = false;
double aspect_ratio = LARGE_H / double(LARGE_W);
int window_min_width = 0, window_min_height = 0;
