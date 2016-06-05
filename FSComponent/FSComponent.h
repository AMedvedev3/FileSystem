#pragma once

#include "FSComponent_h.h"
#include <wrl.h>
#include <exception.h>
#include <comdef.h>
#include "file_system.h"

using namespace Microsoft::WRL;

class FSComponent : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IFileSystemComponent, ISupportErrorInfo>
{
public:
    FSComponent()
    {
    }

    STDMETHODIMP RemoveStorage();
    STDMETHODIMP CreateFile(unsigned char* name);
    STDMETHODIMP CreateDirectory(unsigned char* name);
    STDMETHODIMP RemoveDirectory(unsigned char* name);
    STDMETHODIMP RemoveFile(unsigned char* name);

    STDMETHODIMP WriteFile(unsigned char* name, unsigned char* buffer, unsigned long long cb, unsigned long long write_position,
        unsigned long long* written_bytes, unsigned long long* after_position);

    STDMETHODIMP ReadFile(unsigned char* name, unsigned char* buffer, unsigned long long cb, unsigned long long position,
        unsigned long long* readed_bytes, unsigned long long* after_position);

    STDMETHODIMP GetFileDescription(unsigned char* name, FSComFileDescriptionStruct** desc);

    STDMETHODIMP GetDirectoryList(unsigned char* name, FSComFileDescriptionStruct** descriptions,
        unsigned long long* count);

    // TODO: SetStoragePath
    // additional maybe
    // TODO: MoveFile
    // TODO: CopyFile
    // TODO: MoveDirectory
    // TODO: DirectoryExists
    // TODO: FileExists
    // TODO: Lock
    // TODO: Unlock


    STDMETHODIMP InterfaceSupportsErrorInfo(REFIID riid)
    {
        if (riid == __uuidof(IFileSystemComponent))
            return S_OK;
        else
            return S_FALSE;
    }

protected:

    HRESULT ExceptionToComError(boost::exception const& e, REFIID iid);
    void copy(FSComFileDescription& c_desc, FileDescription& f_desc);
};

CoCreatableClass(FSComponent);

