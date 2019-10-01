
#include "../ScriptCommon.h"

#ifdef _MSC_VER
#pragma warning ( push )
#pragma warning ( disable : 151 241 138 ) //stupid dinkumware can't fucking write libraries worth shit.
#pragma warning ( disable : 4275 4996 ) //clib "may be unsafe" warnings.
#endif

#include <direct.h>

//////////////////////////////////////////////////////////////////////////
#define ScriptString std::string

//////////////////////////////////////////////////////////////////////////

#include "../ZCMath.cpp"
#include "../Array.cpp"
#include "../Utility.cpp"
#include "../Preprocessor.cpp"

#ifdef _MSC_VER
#pragma warning ( pop )
#endif



