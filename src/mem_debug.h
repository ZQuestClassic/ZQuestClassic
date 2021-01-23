
/* For stupid memory management bullshit */
#ifndef __trapperkeeper_h_
#define __trapperkeeper_h_

#include "gc.h"
#include "zc_malloc.h"


#ifndef TK_NO_CRTDBG
#ifdef _MSC_VER


#pragma once

#if (VLD_FORCE_ENABLE == 0)

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>


#ifndef TK_NO_NEW_REMAP
//taken from msdn
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif // _DEBUG
#endif // !TK_NO_NEW_REMAP


#ifdef _DEBUG
inline static void InitCrtDebug(int flags = 0)
{
    if (flags == 0)
        flags =
        _CRTDBG_ALLOC_MEM_DF
        |
        _CRTDBG_LEAK_CHECK_DF
        //|
        //_CRTDBG_REPORT_FLAG
        ;

    _CrtSetDbgFlag(flags);

}
#else // NOT _DEBUG
/*inline static void InitCrtDebug( int flags = 0 )
    {
        flags = flags; //do nothing
    }*/
#endif

#else // NOT _MSC_VER
/*static void InitCrtDebug( int flags = 0 )
    {
        flags = flags; //do nothing
    }*/
#endif // _MSC_VER
#endif // TK_HEADER_NO_DEBUG_REMAP
#endif // !(VLD_FORCE_ENABLE == 0)


#endif // __trapperkeeper_h_
