#include "stdafx.h"
#include "file_system.h"
#include "FSComponent.h"


STDMETHODIMP FSComponent::CreateFile(unsigned char* name)
{
    try
    {
        FileSystem fs;
        fs.create_file(reinterpret_cast<const char*>(name));
    }
    // TODO: Make macros for standard exception handling in this class
    catch (boost::exception const& e)
    {
       return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP FSComponent::CreateDirectory(unsigned char* name)
{
    try
    {
        FileSystem fs;
        fs.create_directory(reinterpret_cast<const char*>(name));
    }
    catch (boost::exception const& e)
    {
        return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}


STDMETHODIMP FSComponent::RemoveDirectory(unsigned char* name)
{
    try
    {
        FileSystem fs;
        fs.remove_directory(reinterpret_cast<const char*>(name));
    }
    catch (boost::exception const& e)
    {
        return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}


STDMETHODIMP FSComponent::RemoveFile(unsigned char* name)
{
    try
    {
        FileSystem fs;
        fs.remove_file(reinterpret_cast<const char*>(name));
    }
    catch (boost::exception const& e)
    {
        return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}


STDMETHODIMP FSComponent::WriteFile(unsigned char* name, unsigned char* buffer, unsigned long long cb, unsigned long long write_position,
    unsigned long long* written_bytes, unsigned long long* after_position)
{
    try
    {
        FileSystem fs;
        if (!after_position)
        {
            BOOST_THROW_EXCEPTION(common_exception()
                << errinfo_fs_code(fs_error::INVALID_PARAMETER)
                << errinfo_message("Argument after_position is NULL"));
        }
        *after_position = fs.write_file((const char*)name, (const char*)buffer, cb, write_position, written_bytes);
    }
    catch (boost::exception const& e)
    {
        return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}


STDMETHODIMP FSComponent::ReadFile(unsigned char* name, unsigned char* buffer, unsigned long long cb, unsigned long long position,
    unsigned long long* readed_bytes, unsigned long long* after_position)
{
    try
    {
        FileSystem fs;
        if (!after_position)
        {
            BOOST_THROW_EXCEPTION(common_exception()
                << errinfo_fs_code(fs_error::INVALID_PARAMETER)
                << errinfo_message("Argument after_position is NULL"));
        }
        *after_position = fs.read_file((const char*)name, (char*)buffer, cb, position, readed_bytes);
    }
    catch (boost::exception const& e)
    {
        return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP FSComponent::RemoveStorage()
{
    try
    {
        FileSystem fs;
        fs.remove_storage();
    }
    catch (boost::exception const& e)
    {
        return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP FSComponent::GetFileDescription(unsigned char* name, FSComFileDescriptionStruct** out_desc)
{
    // TODO: allocate struct CoTaskMemAlloc();
    try
    {
        FileSystem fs;
        FileDescription desc = fs.get_file_desc((const char*)name);
        FSComFileDescription* fs_desc = (FSComFileDescription*)CoTaskMemAlloc(sizeof(FSComFileDescription));
        if(fs_desc == NULL)
            BOOST_THROW_EXCEPTION(common_exception()
                << errinfo_fs_code(fs_error::UNKNOWN_ERROR)
                << errinfo_message("Cannot allocate memory for FSComFileDescription."));

        // fill the description
        copy(*fs_desc, desc);
        *out_desc = fs_desc;
    }
    catch (boost::exception const& e)
    {
        return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}


STDMETHODIMP FSComponent::GetDirectoryList(unsigned char* name, FSComFileDescriptionStruct** descriptions,
    unsigned long long* count)
{
    try
    {
        FileSystem fs;
        FSComFileDescription* desc_arr = NULL;
        std::vector<FileDescription> vec = fs.get_directory_list((const char*)name);
        if (vec.size() > 0)
        {
            desc_arr = (FSComFileDescription*)CoTaskMemAlloc(sizeof(FSComFileDescription) * vec.size());
            FSComFileDescription* cur = desc_arr;
            for (FileDescription& it : vec)
            {
                copy(*cur, it);
                cur++;
            }
        }

        *descriptions = desc_arr;
        *count = vec.size();
    }
    catch (boost::exception const& e)
    {
        return ExceptionToComError(e, _uuidof(IFileSystemComponent));
    }// TODO: process Unknown exceptions
    catch (...)
    {
        return E_FAIL;
    }
    return S_OK;
}

//catch (boost::exception const& e) \
//{ if (int const * c = boost::get_error_info<errinfo_fs_code>(e)) \
//reply.Error(*c, boost::diagnostic_information(e)); \
//else reply.Error(::rpc_error::UNKNOWN_ERROR, boost::diagnostic_information(e)); \
//return; } \
//catch (std::exception const& e) \
//{ reply.Error(::rpc_error::UNKNOWN_ERROR, e.what()); return; }


HRESULT FSComponent::ExceptionToComError(boost::exception const& e, REFIID iid)
{
    HRESULT hr;

    int code = *boost::get_error_info<errinfo_fs_code>(e);
    std::string short_desc = *boost::get_error_info<errinfo_message>(e);
    std::string desc = boost::diagnostic_information(e);

    ComPtr<ICreateErrorInfo> createErrorInfo;
    hr = CreateErrorInfo(&createErrorInfo);
    if (FAILED(hr))
        return hr;

    // Set rich error information.
    createErrorInfo->SetDescription(
        (LPOLESTR)(wchar_t*)_bstr_t(short_desc.c_str()));
    createErrorInfo->SetGUID(iid);
    createErrorInfo->SetSource(L"FileSystem.FSComponent");

    // Exchange ICreateErrorInfo for IErrorInfo.
    ComPtr<IErrorInfo> errorInfo;
    hr = createErrorInfo.As(&errorInfo);
    if (FAILED(hr))
        return hr;

    // Make the error information available to the client.
    hr = SetErrorInfo(0, errorInfo.Detach());
    if (FAILED(hr))
        return hr;
    
    // Return the actual error code.
    return fs_error::fs_hr(code);
}

void FSComponent::copy(FSComFileDescription& c_desc, FileDescription& f_desc)
{
    c_desc.size = f_desc.size;

    strncpy_s((char*)c_desc.name, sizeof(c_desc.name),
        (const char*)f_desc.name, sizeof(f_desc.name));

    c_desc.type = (FSComDescTypeEnum)f_desc.type;
}