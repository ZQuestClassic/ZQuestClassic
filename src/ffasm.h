#ifndef _FFASM_H_
#define _FFASM_H_

#include "scripting/ZASMdefs.h"
#include "scripting/ZAsmCommands.h"
#include "scripting/ZAsmVariables.h"
#include <string>

struct script_variable
{
	char name[30];
	long id;
	word maxcount;
	byte multiple;
};

int set_argument(char *argbuf, zasm **script, int com, int argument);
int parse_script_section(char *combuf, char *arg1buf, char *arg2buf, zasm **script, int com, int &retcode);
int parse_script(ZAsmScript &script, int type);
int parse_script_file(zasm **script, const char *path, bool report_success);
long ffparse(char *string);
std::string to_string(zasm const& command);


#endif

