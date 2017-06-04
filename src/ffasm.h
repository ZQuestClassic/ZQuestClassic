# include "ffscript.h"

#ifndef _FFASM_H_
#define _FFASM_H_

int set_argument(char *argbuf, zasm **script, int com, int argument);
int parse_script_section(char *combuf, char *arg1buf, char *arg2buf, zasm **script, int com, int &retcode);
int parse_script(ZAsmScript &script, int type);
int parse_script_file(zasm **script, const char *path, bool report_success);
long ffparse(char *string);
std::string to_string(zasm const& command);


#endif

