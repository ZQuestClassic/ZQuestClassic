#ifndef PARSERDEFS_H_
#define PARSERDEFS_H_

#include "base/headers.h"
#include "components/zasm/cmp.h"

namespace ZScript
{
	class LocationData;
}

using std::shared_ptr;
#undef TEXT
#undef VOID

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
#define FUNCFLAG_UNUSED             0x00000200 //
#define FUNCFLAG_NIL                0x00000400 //Function does 'nothing' (optimizable)
#define FUNCFLAG_EXITS              0x00000800 //Function exits the current script (or game)
#define FUNCFLAG_NEVER_RETURN       0x00001000 //Function never returns
#define FUNCFLAG_READ_ONLY          0x00002000 //Function is read-only
#define FUNCFLAG_INTERNAL           0x00004000 //Function is internal
#define FUNCFLAG_CONSTEXPR          0x00008000 //Function is constexpr
#define FUNCFLAG_MAY_SET_OBJECT     0x00010000 //Function is a setter, and may or may not retain an object reference
#define FUNCFLAG_NONSTATIC          0x00020000 //'FUNCFLAG_STATIC' should be removed if was auto-applied

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
#define OPTTYPE_QST_SETTING         3

#define REGISTRATION_REC_LIMIT		50

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
void zparser_error_out(std::string message, ZScript::LocationData const& err_loc);
void zparser_warn_out(std::string message);

#endif

