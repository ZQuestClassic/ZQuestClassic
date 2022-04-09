#include "../ffscript.h"
#include "util.h"
#include "parser/ZScript.h"
#include "parser/parser.h"
#include <string>
#include "zconfig.h"
#include "ConsoleLogger.h"
#include "zscrdata.h"

FFScript FFCore;

std::vector<std::string> ZQincludePaths;
byte quest_rules[QUESTRULES_NEW_SIZE];

extern byte monochrome_console;

CConsoleLoggerEx parser_console;

extern uint32_t zscript_failcode;
extern bool zscript_had_warn_err;

int32_t get_bit(byte const* bitstr,int32_t bit)
{
	bitstr += bit>>3;
	return ((*bitstr) >> (bit&7))&1;
}

static const int32_t WARN_COLOR = CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_GREEN;
static const int32_t ERR_COLOR = CConsoleLoggerEx::COLOR_RED;
static const int32_t INFO_COLOR = CConsoleLoggerEx::COLOR_WHITE;

void zconsole_warn(const char *format,...)
{
	zscript_had_warn_err = true;
	int32_t v = parser_console.cprintf( WARN_COLOR, "[Warn] ");
	if(v < 0) return; //Failed to print
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
	al_trace("%s\n", tmp);
	parser_console.cprintf( WARN_COLOR, "%s\n", tmp);
}
void zconsole_error(const char *format,...)
{
	zscript_had_warn_err = true;
	int32_t v = parser_console.cprintf( ERR_COLOR,"[Error] ");
	if(v < 0) return; //Failed to print
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
	al_trace("%s\n", tmp);
	parser_console.cprintf( ERR_COLOR, "%s\n", tmp);
}
void zconsole_info(const char *format,...)
{
	int32_t v = parser_console.cprintf( INFO_COLOR,"[Info] ");
	if(v < 0) return; //Failed to print
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
	al_trace("%s\n", tmp);
	parser_console.cprintf( INFO_COLOR, "%s\n", tmp);
}

int32_t used_switch(int32_t argc,char *argv[],const char *s)
{
	// assumes a switch won't be in argv[0]
	for(int32_t i=1; i<argc; i++)
		if(stricmp(argv[i],s)==0)
			return i;

	return 0;
}

std::unique_ptr<ZScript::ScriptsData> compile(std::string script_path)
{
	zconsole_info("Compiling '%s'", script_path.c_str());

	// copy to tmp file
	std::string zScript;
	FILE *zscript = fopen(script_path.c_str(),"r");
	if(zscript == NULL)
	{
		zconsole_error(" Cannot open specified file!");
		zscript_failcode = -404;
		return NULL;
	}

	if(strcmp(script_path.c_str(), "tmp")) // if path matches "tmp", no reason to copy it to "tmp"
	{
		char c = fgetc(zscript);
		while(!feof(zscript))
		{
			zScript += c;
			c = fgetc(zscript);
		}
		fclose(zscript);

		FILE *tempfile = fopen("tmp","w");
		if(!tempfile)
		{
			zconsole_error("Unable to create a temporary file in current directory!");
			zscript_failcode = -404;
			return NULL;
		}
		fwrite(zScript.c_str(), sizeof(char), zScript.size(), tempfile);
		fclose(tempfile);
	}
	else fclose(zscript);
	
	std::unique_ptr<ZScript::ScriptsData> res(ZScript::compile("tmp"));
	unlink("tmp");
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
	if (!used_switch(argc, argv, "-linked"))
	{
		return 1;
	}
	
	child_process_handler cph;
	allegro_init();
	
	parser_console.Create("ZScript Parser Output", 600, 200, NULL, "ZConsole.exe");
	parser_console.cls(CConsoleLoggerEx::COLOR_BACKGROUND_BLACK);
	parser_console.gotoxy(0,0);
	zconsole_info("External ZScript Parser\n");
	
	int32_t script_path_index = used_switch(argc, argv, "-input");
	if (!script_path_index)
	{
		zconsole_error("Error: missing required flag: -input");
		return 1;
	}
	
	std::string script_path = argv[script_path_index + 1];

	cph.read(quest_rules, QUESTRULES_NEW_SIZE);
	
	set_config_file("zscript.cfg");
	memset(FFCore.scriptRunString,0,sizeof(FFCore.scriptRunString));
	char const* runstr = zc_get_config("zquest.cfg","Compiler","run_string","run");
	strcpy(FFCore.scriptRunString, runstr);
	updateIncludePaths();
	// Any errors will be printed to stdout.
	unique_ptr<ZScript::ScriptsData> result(compile(script_path));
	
	int32_t res = (result ? 0 : (zscript_failcode ? zscript_failcode : -1));
	
	if(!res)
	{
		write_compile_data(result->scriptTypes, result->theScripts);
	}
	cph.write(&res, sizeof(int32_t));
	
	if(zscript_had_warn_err)
		zconsole_warn("Leaving console open; there were errors or warnings during compile!");
	else if(used_switch(argc, argv, "-noclose"))
	{
		zconsole_info("Leaving console open; '-noclose' switch used");
	}
	else
	{
		parser_console.kill();
	}
	allegro_exit();
	return res;
}
END_OF_MAIN()


