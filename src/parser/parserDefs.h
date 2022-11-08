#ifndef _PARSERDEFS_H_
#define _PARSERDEFS_H_

#include "base/zdefs.h"
#undef TEXT
#undef VOID

#include "base/util.h"
using namespace util;

//FUNCFLAG values, for `Function` and `ASTFuncDecl` objects.
#define FUNCFLAG_INLINE             0x01
#define FUNCFLAG_INVALID            0x02
#define FUNCFLAG_STATIC             0x04
#define FUNCFLAG_CONSTRUCTOR        0x08
#define FUNCFLAG_DESTRUCTOR         0x10
#define FUNCFLAG_CLASSFUNC          0x20

#define IFUNCFLAG_SKIPPOINTER		0x01
#define IFUNCFLAG_REASSIGNPTR		0x02

//Option values. NOTE: Result of "lookupOption" must be '/10000.0' -V
#define OPT_OFF                     0
#define OPT_ON                      1
#define OPT_ERROR                   2
#define OPT_WARN                    3

#define OPTTYPE_QR                  0
#define OPTTYPE_CONFIG              1

#define MAX_INCLUDE_PATH_CHARS      65535

#define REGISTRATION_REC_LIMIT		50

void zconsole_db(const char *format,...);
void zconsole_warn(const char *format,...);
void zconsole_error(const char *format,...);
void zconsole_info(const char *format,...);
bool zparser_errored_out();
void zparser_error_out();

#endif

