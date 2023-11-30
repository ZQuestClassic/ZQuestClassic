// TODO: do not link allegro w/ zscript compiler.

#include "zc/ffscript.h"
#include "base/util.h"
#include "parser/ZScript.h"
#include "parser/config.h"
#include "parser/parser.h"
#include <string>
#include "zconfig.h"
#include "zconsole/ConsoleLogger.h"
#include "zscrdata.h"
#include "base/zapp.h"
#include "base/qrs.h"
#include "base/zsys.h"

FFScript FFCore;

std::vector<std::string> ZQincludePaths;
std::string console_path;

extern byte monochrome_console;

io_manager* ConsoleWrite;

extern uint32_t zscript_failcode;
extern bool zscript_error_out;

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
void zparser_error_out()
{
	zscript_error_out = true;
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
	else printf("%s\n", str);
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
std::unique_ptr<ZScript::ScriptsData> compile(std::string script_path)
{
	if(linked)
		zconsole_info("Compiling the ZQuest buffer...");
	else zconsole_info("Compiling '%s'", script_path.c_str());

	// copy to tmp file
	std::string zScript;
	FILE *zscript = fopen(script_path.c_str(),"r");
	if(zscript == NULL)
	{
		zconsole_error("%s", "Cannot open specified file!");
		zscript_failcode = -404;
		return NULL;
	}

	char c = fgetc(zscript);
	while(!feof(zscript))
	{
		zScript += c;
		c = fgetc(zscript);
	}
	fclose(zscript);

	char tmpfilename[L_tmpnam];
	std::tmpnam(tmpfilename);
	FILE *tempfile = fopen(tmpfilename, "w");
	if(!tempfile)
	{
		zconsole_error("%s", "Unable to create a temporary file in current directory!");
		zscript_failcode = -404;
		return NULL;
	}
	fwrite(zScript.c_str(), sizeof(char), zScript.size(), tempfile);
	fclose(tempfile);
	
	std::unique_ptr<ZScript::ScriptsData> res(ZScript::compile(tmpfilename));
	unlink(tmpfilename);
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

bool delay_asserts = false, ignore_asserts = false;
std::vector<std::filesystem::path> force_ignores;
int32_t main(int32_t argc, char **argv)
{
	common_main_setup(App::zscript, argc, argv);
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
			force_ignores.push_back(std::filesystem::path(argv[q]).lexically_normal());
		}
	}
	
	int32_t console_path_index = used_switch(argc, argv, "-console");
	if (linked && !console_path_index)
	{
		zconsole_error("%s", "Error: missing required flag: -console");
		return 1;
	}
	if(console_path_index)
		console_path = argv[console_path_index + 1];
	else console_path = "";
	
	int32_t zasm_out_index = used_switch(argc, argv, "-zasm");
	bool zasm_out_append = used_switch(argc, argv, "-append");
	string zasm_out = "";
	if(zasm_out_index)
		zasm_out = argv[zasm_out_index + 1];
	
	child_process_handler* cph = (linked ? new child_process_handler() : nullptr);
	ConsoleWrite = cph;

	if (!zscript_load_base_config("base_config/zscript.cfg"))
	{
		zconsole_error("%s", "Error: failed to load base config");
		return 1;
	}

	zscript_load_user_config("zscript.cfg");

	int32_t script_path_index = used_switch(argc, argv, "-input");
	if (!script_path_index)
	{
		zconsole_error("%s", "Error: missing required flag: -input");
		return 1;
	}
	
	if(console_path.size())
	{
		FILE *console=fopen(console_path.c_str(), "w");
		fclose(console);
	}

	int32_t qr_hex_index = used_switch(argc, argv, "-qr");
	{
		std::string qr_hex = qr_hex_index ?
			argv[qr_hex_index + 1] :
			// TODO: set to defaults in a better way.
			"B343AFAF01C281A00DA58A4211A608DFDF080001162A0410FC5306FE2A274100381B02044031300000065824000000000000D0030000000000000000000000000000000000000000000000000000000034866C3140320000000000000000000000000000";
		//printf("%s\n", qr_hex.c_str());
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

	std::string script_path = argv[script_path_index + 1];
	int32_t syncthing = 0;
	
	if(linked)
	{
		cph->write(&syncthing, sizeof(int32_t));
	}
	
	std::string runstr = zscript_get_config_string("run_string", "run");
	strncpy(FFCore.scriptRunString, runstr.c_str(), sizeof(FFCore.scriptRunString));

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
			al_rest(1); //seconds
		}
	}
	unique_ptr<ZScript::ScriptsData> result(compile(script_path));
	if(!result)
		zconsole_info("%s", "Failure!");
	int32_t res = (result ? 0 : (zscript_failcode ? zscript_failcode : -1));
	
	if(linked)
	{
		if(!res)
		{
			write_compile_data(result->scriptTypes, result->theScripts);
		}
		int32_t errorcode = ZC_CONSOLE_TERM_CODE;
		cph->write(&errorcode, sizeof(int32_t));
		cph->write(&res, sizeof(int32_t));
		/*
		if(zscript_had_warn_err)
			zconsole_warn("%s", "Leaving console open; there were errors or warnings during compile!");
		else if(used_switch(argc, argv, "-noclose"))
		{
			zconsole_info("%s", "Leaving console open; '-noclose' switch used");
		}
		else
		{
			parser_console.kill();
		}*/
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
	}
	if(!zasm_out.empty() && result)
	{
		FILE *outfile = fopen(zasm_out.c_str(), zasm_out_append ? "a" : "w");
		for(auto& p : result->theScripts)
		{
			disassembled_script_data const& data = p.second;
			data.write(outfile,false,true);
		}
		fclose(outfile);
	}
	
	if(cph) delete cph;
	return res;
}
END_OF_MAIN()

// TODO: make this not needed to compile...
bool DragAspect = false;
double aspect_ratio = LARGE_H / double(LARGE_W);
int window_min_width = 0, window_min_height = 0;
