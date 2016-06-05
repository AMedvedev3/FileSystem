

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Sun Jun 05 22:59:25 2016
 */
/* Compiler settings for FSComponent.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __FSComponent_h_h__
#define __FSComponent_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IFileSystemComponent_FWD_DEFINED__
#define __IFileSystemComponent_FWD_DEFINED__
typedef interface IFileSystemComponent IFileSystemComponent;

#endif 	/* __IFileSystemComponent_FWD_DEFINED__ */


#ifndef __FSComponent_FWD_DEFINED__
#define __FSComponent_FWD_DEFINED__

#ifdef __cplusplus
typedef class FSComponent FSComponent;
#else
typedef struct FSComponent FSComponent;
#endif /* __cplusplus */

#endif 	/* __FSComponent_FWD_DEFINED__ */


/* header files for imported files */
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_FSComponent_0000_0000 */
/* [local] */ 

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_FSComponent_0000_0000_0001
    {
        FSComDirectoryType	= 0,
        FSComFileType	= 1
    } 	FSComDescTypeEnum;

typedef struct FSComFileDescription
    {
    unsigned char name[ 256 ];
    unsigned char path[ 256 ];
    unsigned long long size;
    FSComDescTypeEnum type;
    } 	FSComFileDescriptionStruct;



extern RPC_IF_HANDLE __MIDL_itf_FSComponent_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_FSComponent_0000_0000_v0_0_s_ifspec;

#ifndef __IFileSystemComponent_INTERFACE_DEFINED__
#define __IFileSystemComponent_INTERFACE_DEFINED__

/* interface IFileSystemComponent */
/* [object][version][uuid] */ 


EXTERN_C const IID IID_IFileSystemComponent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CEDAE3D8-F3D8-427D-AA1D-5DB1741FF917")
    IFileSystemComponent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RemoveStorage( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFile( 
            /* [in][ref] */ unsigned char *name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateDirectory( 
            /* [in][ref] */ unsigned char *name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveDirectory( 
            /* [in][ref] */ unsigned char *name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveFile( 
            /* [in][ref] */ unsigned char *name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteFile( 
            /* [in][ref] */ unsigned char *name,
            /* [in][ref] */ unsigned char *buffer,
            /* [in] */ unsigned long long cb,
            /* [in] */ unsigned long long write_position,
            /* [out][ref] */ unsigned long long *written_bytes,
            unsigned long long *after_position) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadFile( 
            /* [in][ref] */ unsigned char *name,
            /* [out][ref] */ unsigned char *buffer,
            /* [in] */ unsigned long long cb,
            /* [in] */ unsigned long long position,
            /* [out][ref] */ unsigned long long *readed_bytes,
            /* [out][ref] */ unsigned long long *after_position) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFileDescription( 
            /* [in][ref] */ unsigned char *name,
            /* [out][in][ref] */ FSComFileDescriptionStruct **desc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDirectoryList( 
            /* [in][ref] */ unsigned char *name,
            /* [out][in][ref] */ FSComFileDescriptionStruct **descriptions,
            /* [out] */ unsigned long long *count) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFileSystemComponentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFileSystemComponent * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFileSystemComponent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFileSystemComponent * This);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveStorage )( 
            IFileSystemComponent * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFile )( 
            IFileSystemComponent * This,
            /* [in][ref] */ unsigned char *name);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDirectory )( 
            IFileSystemComponent * This,
            /* [in][ref] */ unsigned char *name);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveDirectory )( 
            IFileSystemComponent * This,
            /* [in][ref] */ unsigned char *name);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveFile )( 
            IFileSystemComponent * This,
            /* [in][ref] */ unsigned char *name);
        
        HRESULT ( STDMETHODCALLTYPE *WriteFile )( 
            IFileSystemComponent * This,
            /* [in][ref] */ unsigned char *name,
            /* [in][ref] */ unsigned char *buffer,
            /* [in] */ unsigned long long cb,
            /* [in] */ unsigned long long write_position,
            /* [out][ref] */ unsigned long long *written_bytes,
            unsigned long long *after_position);
        
        HRESULT ( STDMETHODCALLTYPE *ReadFile )( 
            IFileSystemComponent * This,
            /* [in][ref] */ unsigned char *name,
            /* [out][ref] */ unsigned char *buffer,
            /* [in] */ unsigned long long cb,
            /* [in] */ unsigned long long position,
            /* [out][ref] */ unsigned long long *readed_bytes,
            /* [out][ref] */ unsigned long long *after_position);
        
        HRESULT ( STDMETHODCALLTYPE *GetFileDescription )( 
            IFileSystemComponent * This,
            /* [in][ref] */ unsigned char *name,
            /* [out][in][ref] */ FSComFileDescriptionStruct **desc);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectoryList )( 
            IFileSystemComponent * This,
            /* [in][ref] */ unsigned char *name,
            /* [out][in][ref] */ FSComFileDescriptionStruct **descriptions,
            /* [out] */ unsigned long long *count);
        
        END_INTERFACE
    } IFileSystemComponentVtbl;

    interface IFileSystemComponent
    {
        CONST_VTBL struct IFileSystemComponentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFileSystemComponent_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFileSystemComponent_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFileSystemComponent_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFileSystemComponent_RemoveStorage(This)	\
    ( (This)->lpVtbl -> RemoveStorage(This) ) 

#define IFileSystemComponent_CreateFile(This,name)	\
    ( (This)->lpVtbl -> CreateFile(This,name) ) 

#define IFileSystemComponent_CreateDirectory(This,name)	\
    ( (This)->lpVtbl -> CreateDirectory(This,name) ) 

#define IFileSystemComponent_RemoveDirectory(This,name)	\
    ( (This)->lpVtbl -> RemoveDirectory(This,name) ) 

#define IFileSystemComponent_RemoveFile(This,name)	\
    ( (This)->lpVtbl -> RemoveFile(This,name) ) 

#define IFileSystemComponent_WriteFile(This,name,buffer,cb,write_position,written_bytes,after_position)	\
    ( (This)->lpVtbl -> WriteFile(This,name,buffer,cb,write_position,written_bytes,after_position) ) 

#define IFileSystemComponent_ReadFile(This,name,buffer,cb,position,readed_bytes,after_position)	\
    ( (This)->lpVtbl -> ReadFile(This,name,buffer,cb,position,readed_bytes,after_position) ) 

#define IFileSystemComponent_GetFileDescription(This,name,desc)	\
    ( (This)->lpVtbl -> GetFileDescription(This,name,desc) ) 

#define IFileSystemComponent_GetDirectoryList(This,name,descriptions,count)	\
    ( (This)->lpVtbl -> GetDirectoryList(This,name,descriptions,count) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFileSystemComponent_INTERFACE_DEFINED__ */



#ifndef __FSComponentLib_LIBRARY_DEFINED__
#define __FSComponentLib_LIBRARY_DEFINED__

/* library FSComponentLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_FSComponentLib;

EXTERN_C const CLSID CLSID_FSComponent;

#ifdef __cplusplus

class DECLSPEC_UUID("CBD410A7-C5D8-480F-9E68-A6DBD6D7E537")
FSComponent;
#endif
#endif /* __FSComponentLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


