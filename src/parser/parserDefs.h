#ifndef _PARSERDEFS_H_
#define _PARSERDEFS_H_

#include "base/zdefs.h"
#include "base/headers.h"
#undef TEXT
#undef VOID

#include "base/util.h"
using namespace util;

//FUNCFLAG values, for `Function` and `ASTFuncDecl` objects.
#define FUNCFLAG_INLINE             0x00000001 //Can be heavily optimized
#define FUNCFLAG_INVALID            0x00000002 //Needs to throw an error, later in compile
#define FUNCFLAG_STATIC             0x00000004 //Is static (not per-object in classes)
#define FUNCFLAG_CONSTRUCTOR        0x00000008 //Is a user class constructor
#define FUNCFLAG_DESTRUCTOR         0x00000010 //Is a user class destructor
#define FUNCFLAG_CLASSFUNC          0x00000020 //Belongs to a user class
#define FUNCFLAG_VARARGS            0x00000040 //Uses variadic arguments
#define FUNCFLAG_DEPRECATED         0x00000080 //Gives a deprecated warning/error on use
#define FUNCFLAG_NOCAST             0x00000100 //Affects function's overloading
#define FUNCFLAG_INTARRAY           0x00000200 //Function represents a special internal array
#define FUNCFLAG_NIL                0x00000400 //Function does 'nothing' (optimizable)
#define FUNCFLAG_EXITS              0x00000800 //Function exits the current script (or game)
#define FUNCFLAG_NEVER_RETURN       0x00001000 //Function never returns

#define IFUNCFLAG_SKIPPOINTER	      0x01
#define IFUNCFLAG_REASSIGNPTR	      0x02

//Option values. NOTE: Result of "lookupOption" must be '/10000.0' -V
#define OPT_OFF                     0
#define OPT_ON                      1
#define OPT_ERROR                   2
#define OPT_WARN                    3

#define OPTTYPE_QR                  0
#define OPTTYPE_CONFIG              1
#define OPTTYPE_CONFIG_FLOAT        2

#define MAX_INCLUDE_PATH_CHARS      65535

#define REGISTRATION_REC_LIMIT		50

#define CMP_MORE  0x01
#define CMP_LESS  0x02
#define CMP_EQ    0x04
#define CMP_FLAGS 0x07
#define CMP_SETI  0x08
inline string CMP_STR(uint cmpval)
{
	switch(cmpval&CMP_FLAGS)
	{
		case 0: default:
			return "Never";
		case CMP_MORE:
			return ">";
		case CMP_MORE|CMP_EQ:
			return ">=";
		case CMP_LESS:
			return "<";
		case CMP_LESS|CMP_EQ:
			return "<=";
		case CMP_EQ:
			return "==";
		case CMP_LESS|CMP_MORE:
			return "!=";
		case CMP_LESS|CMP_MORE|CMP_EQ:
			return "Always";
	}
}

void zconsole_db(const char *format,...);
void zconsole_warn(const char *format,...);
void zconsole_error(const char *format,...);
void zconsole_info(const char *format,...);
void zconsole_idle(dword seconds = 2);

void zconsole_db(std::string const& str);
void zconsole_warn(std::string const& str);
void zconsole_error(std::string const& str);
void zconsole_info(std::string const& str);

bool zparser_errored_out();
void zparser_error_out();

#endif

