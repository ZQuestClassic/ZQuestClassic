

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Sun Dec 23 23:12:05 2018
 */
/* Compiler settings for .\algif_1.3\algif_1.3\src\algif\algif\algif.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_algifLib,0x2F027A9E,0x3BD8,0x40AF,0x82,0xD4,0xA2,0x1D,0xA2,0xF9,0xE8,0xC6);


MIDL_DEFINE_GUID(IID, DIID__Dalgif,0x3CF2A73C,0xDA1B,0x4642,0xB9,0x4D,0xE7,0x79,0xDD,0xFF,0x5C,0x35);


MIDL_DEFINE_GUID(IID, DIID__DalgifEvents,0x35FB3AB6,0x5F05,0x4B3E,0x8A,0x82,0xFC,0x21,0xED,0x68,0xA2,0xD1);


MIDL_DEFINE_GUID(CLSID, CLSID_algif,0xA81C8A96,0xE5AE,0x4299,0xB1,0xFF,0x1D,0x1E,0x7E,0x88,0x3F,0x95);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



