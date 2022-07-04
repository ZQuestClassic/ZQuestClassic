#include "../ffscript.h"
#include "util.h"
#include "parser/ZScript.h"
#include "parser/parser.h"
#include <string>
#include "zconfig.h"
#include "ConsoleLogger.h"
#include "zscrdata.h"
#include "zapp.h"

FFScript FFCore;

std::vector<std::string> ZQincludePaths;
std::string console_path;
byte quest_rules[QUESTRULES_NEW_SIZE];

extern byte monochrome_console;

io_manager* ConsoleWrite;

extern uint32_t zscript_failcode;
extern bool zscript_had_warn_err;

int32_t get_bit(byte const* bitstr,int32_t bit)
{
	bitstr += bit>>3;
	return ((*bitstr) >> (bit&7))&1;
}

int32_t used_switch(int32_t argc, char* argv[], const char* s)
{
	// assumes a switch won't be in argv[0]
	for (int32_t i = 1; i < argc; i++)
		if (stricmp(argv[i], s) == 0)
			return i;

	return 0;
}

static const int32_t WARN_COLOR = CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_GREEN;
static const int32_t ERR_COLOR = CConsoleLoggerEx::COLOR_RED;
static const int32_t INFO_COLOR = CConsoleLoggerEx::COLOR_WHITE;

void zconsole_warn(const char *format,...)
{
	zscript_had_warn_err = true;
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
	if(console_path.size())
	{
		FILE *console=fopen(console_path.c_str(), "a");
		fprintf(console, "%s", tmp);
		fclose(console);
	}
	if(ConsoleWrite)
	{
		int32_t errorcode = ZC_CONSOLE_WARN_CODE;
		ConsoleWrite->write(&errorcode, sizeof(int32_t));
		ConsoleWrite->read(&errorcode, sizeof(int32_t));
	}
	else printf("%s\n", tmp);
}
void zconsole_error(const char *format,...)
{
	zscript_had_warn_err = true;
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
	if(console_path.size())
	{
		FILE *console=fopen(console_path.c_str(), "a");
		fprintf(console, "%s", tmp);
		fclose(console);
	}
	if(ConsoleWrite)
	{
		int32_t errorcode = ZC_CONSOLE_ERROR_CODE;
		ConsoleWrite->write(&errorcode, sizeof(int32_t));
		ConsoleWrite->read(&errorcode, sizeof(int32_t));
	}
	else printf("%s\n", tmp);
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
	if(console_path.size())
	{
		FILE *console=fopen(console_path.c_str(), "a");
		fprintf(console, "%s", tmp);
		fclose(console);
	}
	if(ConsoleWrite)
	{
		int32_t errorcode = ZC_CONSOLE_INFO_CODE;
		ConsoleWrite->write(&errorcode, sizeof(int32_t));
		ConsoleWrite->read(&errorcode, sizeof(int32_t));
	}
	else printf("%s\n", tmp);
}

std::unique_ptr<ZScript::ScriptsData> compile(std::string script_path)
{
	zconsole_info("Compiling '%s'", script_path.c_str());

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
	ZQincludePaths.clear();
	int32_t pos = 0; int32_t pathnumber = 0;
	for ( int32_t q = 0; includePathString[pos]; ++q )
	{
		int32_t dest = 0;
		char buf[2048] = {0};
		while(includePathString[pos] != ';' && includePathString[pos])
		{
			buf[dest] = includePathString[pos];
			++pos;
			++dest;
		}
		++pos;
		std::string str(buf);
		ZQincludePaths.push_back(str);
	}
}

int32_t main(int32_t argc, char **argv)
{
	common_main_setup(argc, argv);
	bool linked = true;
	if (!used_switch(argc, argv, "-linked"))
	{
		if(used_switch(argc, argv, "-unlinked"))
		{
			linked = false;
		}
		else return 1;
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
	
	child_process_handler cph;
	ConsoleWrite = linked ? &cph : nullptr;
	if(allegro_init() != 0)
	{
		zconsole_error("%s", "Failed Init!");
		exit(1);
	}
	
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
	
	std::string script_path = argv[script_path_index + 1];
	int32_t syncthing = 0;
	
	if(linked)
	{
		cph.read(quest_rules, QUESTRULES_NEW_SIZE);
		cph.write(&syncthing, sizeof(int32_t));
	}
	
	zc_set_config_standard();
	memset(FFCore.scriptRunString,0,sizeof(FFCore.scriptRunString));
	char const* runstr = zc_get_config(STANDARD_CFG,"Compiler","run_string","run");
	strcpy(FFCore.scriptRunString, runstr);
	updateIncludePaths();
	// Any errors will be printed to stdout.
	/*
	for(auto q = 0; q < 2147483647; ++q)
	{
		if(!(rand()%10))
			--q;
	} //*/
	unique_ptr<ZScript::ScriptsData> result(compile(script_path));
	
	int32_t res = (result ? 0 : (zscript_failcode ? zscript_failcode : -1));
	
	if(linked)
	{
		if(!res)
		{
			write_compile_data(result->scriptTypes, result->theScripts);
		}
		int32_t errorcode = -9995;
		cph.write(&errorcode, sizeof(int32_t));
		cph.write(&res, sizeof(int32_t));
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
	return res;
}
END_OF_MAIN()


