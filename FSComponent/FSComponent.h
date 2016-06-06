#pragma once
/**
@file
FileSystem COM component
@author Alexey Medvedev
*/
#include "FSComponent_h.h"
#include <wrl.h>
#include <exception.h>
#include <comdef.h>
#include "file_system.h"

using namespace Microsoft::WRL;

// COM component wrapper for public FileSystem functions
// Implements COM interface to FileSystem
class FSComponent : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IFileSystemComponent, ISupportErrorInfo>
{
public:
    FSComponent()
    {
    }
    
    // Removes storage
    STDMETHODIMP RemoveStorage();

    // Creates file
    STDMETHODIMP CreateFile(unsigned char* name);

    // Creates directory
    STDMETHODIMP CreateDirectory(unsigned char* name);

    // Removes directory
    STDMETHODIMP RemoveDirectory(unsigned char* name);

    // Remove file
    STDMETHODIMP RemoveFile(unsigned char* name);

    // Write file
    STDMETHODIMP WriteFile(unsigned char* name, unsigned char* buffer, unsigned long long cb, unsigned long long write_position,
        unsigned long long* written_bytes, unsigned long long* after_position);
    
    // Read file
    STDMETHODIMP ReadFile(unsigned char* name, unsigned char* buffer, unsigned long long cb, unsigned long long position,
        unsigned long long* readed_bytes, unsigned long long* after_position);

    // Get file description
    STDMETHODIMP GetFileDescription(unsigned char* name, FSComFileDescriptionStruct** desc);

    // Returns array with list of directories and files in given directory
    STDMETHODIMP GetDirectoryList(unsigned char* name, FSComFileDescriptionStruct** descriptions,
        unsigned long long* count);

    // TODO: SetStoragePath
    // TODO: MoveFile
    // TODO: CopyFile
    // TODO: MoveDirectory

    // additional maybe
    // TODO: Lock
    // TODO: Unlock
    // TODO: RenameFile
    // TODO: RenameDirectory
    
    // Indicates that component provides extended error information in IErrorInfo
    STDMETHODIMP InterfaceSupportsErrorInfo(REFIID riid)
    {
        if (riid == __uuidof(IFileSystemComponent))
            return S_OK;
        else
            return S_FALSE;
    }

protected:
    
    // Converts FileSystem exception to IErrorInfo
    HRESULT ExceptionToComError(boost::exception const& e, REFIID iid);

    // Converts FileSystem exception to IErrorInfo
    HRESULT ExceptionToComError(int code, const std::string& short_desc, REFIID iid);

    // Helper function to convert File Description to the COM version
    void copy(FSComFileDescription& c_desc, FileDescription& f_desc);
};

CoCreatableClass(FSComponent);

