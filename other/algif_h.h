

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __algif_h_h__
#define __algif_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ___Dalgif_FWD_DEFINED__
#define ___Dalgif_FWD_DEFINED__
typedef interface _Dalgif _Dalgif;
#endif 	/* ___Dalgif_FWD_DEFINED__ */


#ifndef ___DalgifEvents_FWD_DEFINED__
#define ___DalgifEvents_FWD_DEFINED__
typedef interface _DalgifEvents _DalgifEvents;
#endif 	/* ___DalgifEvents_FWD_DEFINED__ */


#ifndef __algif_FWD_DEFINED__
#define __algif_FWD_DEFINED__

#ifdef __cplusplus
typedef class algif algif;
#else
typedef struct algif algif;
#endif /* __cplusplus */

#endif 	/* __algif_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __algifLib_LIBRARY_DEFINED__
#define __algifLib_LIBRARY_DEFINED__

/* library algifLib */
/* [control][helpstring][helpfile][version][uuid] */ 


EXTERN_C const IID LIBID_algifLib;

#ifndef ___Dalgif_DISPINTERFACE_DEFINED__
#define ___Dalgif_DISPINTERFACE_DEFINED__

/* dispinterface _Dalgif */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__Dalgif;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3CF2A73C-DA1B-4642-B94D-E779DDFF5C35")
    _Dalgif : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DalgifVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _Dalgif * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _Dalgif * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _Dalgif * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _Dalgif * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _Dalgif * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _Dalgif * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _Dalgif * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DalgifVtbl;

    interface _Dalgif
    {
        CONST_VTBL struct _DalgifVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _Dalgif_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _Dalgif_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _Dalgif_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _Dalgif_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _Dalgif_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _Dalgif_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _Dalgif_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___Dalgif_DISPINTERFACE_DEFINED__ */


#ifndef ___DalgifEvents_DISPINTERFACE_DEFINED__
#define ___DalgifEvents_DISPINTERFACE_DEFINED__

/* dispinterface _DalgifEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DalgifEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("35FB3AB6-5F05-4B3E-8A82-FC21ED68A2D1")
    _DalgifEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DalgifEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DalgifEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DalgifEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DalgifEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DalgifEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DalgifEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DalgifEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DalgifEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DalgifEventsVtbl;

    interface _DalgifEvents
    {
        CONST_VTBL struct _DalgifEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DalgifEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DalgifEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DalgifEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DalgifEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DalgifEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DalgifEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DalgifEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DalgifEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_algif;

#ifdef __cplusplus

class DECLSPEC_UUID("A81C8A96-E5AE-4299-B1FF-1D1E7E883F95")
algif;
#endif
#endif /* __algifLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


