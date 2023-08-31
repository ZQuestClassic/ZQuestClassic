#ifndef _PARSERDEFS_H_
#define _PARSERDEFS_H_

#include "base/zdefs.h"
#undef TEXT
#undef VOID

#include "base/util.h"
using namespace util;

//FUNCFLAG values, for `Function` and `ASTFuncDecl` objects.
#define FUNCFLAG_INLINE             0x00000001
#define FUNCFLAG_INVALID            0x00000002
#define FUNCFLAG_STATIC             0x00000004
#define FUNCFLAG_CONSTRUCTOR        0x00000008
#define FUNCFLAG_DESTRUCTOR         0x00000010
#define FUNCFLAG_CLASSFUNC          0x00000020
#define FUNCFLAG_VARARGS            0x00000040
#define FUNCFLAG_DEPRECATED         0x00000080
#define FUNCFLAG_NOCAST             0x00000100
#define FUNCFLAG_INTARRAY           0x00000200
#define FUNCFLAG_NIL                0x00000400

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

void zconsole_db(const char *format,...);
void zconsole_warn(const char *format,...);
void zconsole_error(const char *format,...);
void zconsole_info(const char *format,...);

void zconsole_db(std::string const& str);
void zconsole_warn(std::string const& str);
void zconsole_error(std::string const& str);
void zconsole_info(std::string const& str);

bool zparser_errored_out();
void zparser_error_out();

#endif

