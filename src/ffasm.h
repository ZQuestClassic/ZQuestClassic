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

int set_argument(char *argbuf, script_data **script, int com, int argument);
int parse_script_section(char *combuf, char *arg1buf, char *arg2buf, script_data **script, int com, int &retcode);
int parse_script(script_data **script);
int parse_script_file(script_data **script, const char *path, bool report_success);
int parse_script_file(script_data **script, FILE* fscript, bool report_success);
long ffparse(char *string);
long ffparse2(char *string);
std::string get_meta(zasm_meta const& meta);
std::string get_script_name(int type);

#endif

