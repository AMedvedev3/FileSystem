#include "stdafx.h"
#include <comdef.h>
#include <comutil.h>
#include <string>
#include <wrl.h>
#include <iostream>

#include "..\FSComponent\FSComponent_h.h"

using namespace Microsoft::WRL;

const IID IID_ICalculatorComponent = { 0xcedae3d8, 0xf3d8, 0x427d, 0xaa, 0x1d, 0x5d, 0xb1, 0x74, 0x1f, 0xf9, 0x17 };
const CLSID CLSID_CalculatorComponent = { 0xcbd410a7, 0xc5d8, 0x480f, 0x9e, 0x68, 0xa6, 0xdb, 0xd6, 0xd7, 0xe5, 0x37 };

#define CHECK_COM_RET(fs,  res) if(FAILED(res))\
{\
    std::wstring desc = GetComErrorDescription(fs);\
    return PrintError(__LINE__, hr, desc.c_str()); \
}\

// Prints an error string for the provided source code line and HRESULT
// value and returns the HRESULT value as an int.
int PrintError(unsigned int line, HRESULT hr, const wchar_t* desc)
{
    std::wcout << L"ERROR: Line: " << line;
    std::wcout << std::hex << "HRESULT: 0x" << hr;
    std::wcout << "Error: " << desc << std::endl;
    return hr;
}


// TODO: return only description as string others as a hr
std::wstring GetComErrorDescription(ComPtr<IFileSystemComponent>& fs)
{
    ComPtr<ISupportErrorInfo> supportError;
    HRESULT hr = fs.As(&supportError);
    if (FAILED(hr))
        return L"Support Info is not accessible.";
    hr = supportError->InterfaceSupportsErrorInfo(_uuidof(IFileSystemComponent));
    if (FAILED(hr))
        return L"IFileSystemComponent doesn't support ErrorInfo";
    ComPtr<IErrorInfo> errorInfo;
    // TODO: if error info is not available it returns S_FALSE and 
    // errorInfo is null that leads to AV in next call
    hr = GetErrorInfo(0, &errorInfo);
    if (FAILED(hr) && errorInfo != NULL)
        return L"Error info is not available.";
    _bstr_t bstr;
    hr = errorInfo->GetDescription(bstr.GetAddress());
    if (FAILED(hr))
        return L"Error description is not available";
    return (const wchar_t*)bstr;
}

int wmain()
{
    HRESULT hr;

    // RAII wrapper for managing the lifetime of the COM library.
    class CoInitializeWrapper
    {
        HRESULT _hr;
    public:
        CoInitializeWrapper(DWORD flags)
        {
            _hr = CoInitializeEx(nullptr, flags);
        }
        ~CoInitializeWrapper()
        {
            if (SUCCEEDED(_hr))
            {
                CoUninitialize();
            }
        }
        operator HRESULT()
        {
            return _hr;
        }

    };

    // Initialize the COM library.
    CoInitializeWrapper initialize(COINIT_APARTMENTTHREADED);
    if (FAILED(initialize))
    {
        return PrintError(__LINE__, initialize, L"");
    }

    ComPtr<IFileSystemComponent> fs; // Interface to COM component.

                                       // Create the CalculatorComponent object.
    hr = CoCreateInstance(CLSID_CalculatorComponent, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(fs.GetAddressOf()));
    if (SUCCEEDED(hr))
    {
        unsigned char* testDirectory = reinterpret_cast<unsigned char*>("./TestDirectory/");
        unsigned char* testSubDirectory  = reinterpret_cast<unsigned char*>("./TestDirectory/SubDirectory/");
        unsigned char* testFile = reinterpret_cast<unsigned char*>("./TestDirectory/test.txt");
        unsigned char* testFile2 = reinterpret_cast<unsigned char*>("./TestDirectory/simple.txt");
        unsigned char* testFragment = reinterpret_cast<unsigned char*>("Simple Test Text");
        
        fs->RemoveStorage();

        hr = fs->CreateDirectoryW(testDirectory);
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }

        hr = fs->CreateDirectoryW(testSubDirectory);
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }

        hr = fs->CreateFileW(reinterpret_cast<unsigned char*>(testFile));
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }

        hr = fs->CreateFileW(reinterpret_cast<unsigned char*>(testFile2));
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }

        // TODO: do logging conditional to increaese speed of client
        unsigned long long last_pos = 0;
        unsigned long long  written_bytes = 0;
        for (int i = 0; i < 10/*1000*/; i++)
        {
            hr = fs->WriteFile(testFile, testFragment, sizeof(testFragment), last_pos, &written_bytes, &last_pos);
            if (FAILED(hr))
            {
                std::wstring desc = GetComErrorDescription(fs);
                return PrintError(__LINE__, hr, desc.c_str());
            }
        }

        // TODO: get file description before. check file size and change time
        FSComFileDescription* desc = NULL;
        hr = fs->GetFileDescription(testFile, &desc);
        if(FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }
        // check file size
        if (std::string((char*)desc->name) != "test.txt")
            return PrintError(__LINE__, hr, L"File name doesn't coincide with written file.");
        if(desc->size != last_pos)
            return PrintError(__LINE__, hr, L"File size doesn't coincide with written data size.");
        // free memory after usage
        CoTaskMemFree(desc);

        // read all file
        // allocate buffer 

        unsigned char* read_buffer = new unsigned char[last_pos];
        unsigned long long readed_bytes = 0;
        unsigned long long end_reading_pos = 0;
        if(read_buffer == NULL)
            return PrintError(__LINE__, E_FAIL, L"Not enough memory to allocate read buffer.");
        hr = fs->ReadFile(testFile, read_buffer, last_pos, 0, &readed_bytes, &end_reading_pos);
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }
        if(readed_bytes != last_pos)
            return PrintError(__LINE__, E_FAIL, L"Written and readed bytes number are not equals");

        // TODO: check random access and file consistency in unit tests

        FSComFileDescription* descriptions = NULL;
        unsigned long long desc_count = 0;
        hr = fs->GetDirectoryList(testDirectory, &descriptions, &desc_count);
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }
        if (desc_count > 0)
        {
            assert(descriptions != NULL);
            for (int i = 0; i < desc_count; i++)
            {
                switch (descriptions[i].type)
                {
                case FSComDirectoryType:
                    {
                        std::cout << "Directory \"" << descriptions[i].name << "\" " << std::endl;
                    }
                    break;
                case FSComFileType:
                    {
                        std::cout << "File \"" << descriptions[i].name << "\" ";
                        std::cout << "size: " << descriptions[i].size << std::endl;
                    }
                    break;
                default:
                    std::cout << "Unknown descritption" << std::endl;
                }
            }
            CoTaskMemFree(descriptions);
        }

        // clean up - remove files and directorys
        hr = fs->RemoveFile(testFile);
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }

        hr = fs->RemoveFile(testFile2);
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }

        hr = fs->RemoveDirectoryW(testSubDirectory);
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }

        hr = fs->RemoveDirectoryW(testDirectory);
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }
        
        hr = fs->RemoveStorage();
        if (FAILED(hr))
        {
            std::wstring desc = GetComErrorDescription(fs);
            return PrintError(__LINE__, hr, desc.c_str());
        }
    }
    else
    {
        // Object creation failed. Print a message.
        return PrintError(__LINE__, hr, L"");
    }

    return 0;
}