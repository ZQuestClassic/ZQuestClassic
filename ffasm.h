# include "ffscript.h"

#ifndef _FFASM_H_
#define _FFASM_H_

int set_argument(char *argbuf, ffscript **script, int com, int argument);
int parse_script_section(char *combuf, char *arg1buf, char *arg2buf, ffscript **script, int com, int &retcode);
int parse_script(ffscript **script);
int parse_script_file(ffscript **script, const char *path, bool report_success);
long ffparse(char *string);

#endif

