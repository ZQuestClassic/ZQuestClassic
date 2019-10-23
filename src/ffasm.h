//# include "ffscript.h"

#ifndef _FFASM_H_
#define _FFASM_H_

#include "zdefs.h"
#include <utility>
#include <string>
#include <list>
#include "zelda.h"

//What are these for exactly?
//#define fflong(x,y,z)       (((x[(y)][(z)])<<24)+((x[(y)][(z)+1])<<16)+((x[(y)][(z)+2])<<8)+(x[(y)][(z)+3]))
//#define ffword(x,y,z)       (((x[(y)][(z)])<<8)+(x[(y)][(z)+1]))

extern script_command command_list[];
extern script_variable variable_list[];

int set_argument(char *argbuf, ffscript **script, int com, int argument);
int parse_script_section(char *combuf, char *arg1buf, char *arg2buf, ffscript **script, int com, int &retcode);
int parse_script(ffscript **script);
int parse_script_file(ffscript **script, const char *path, bool report_success);
long ffparse(char *string);

#endif

