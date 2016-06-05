#include "stdafx.h"
#include "AppModule.h"
#include "Registration.h"

#include <string>

#include <KtmW32.h>
#pragma comment(lib, "KtmW32")

#include <wrl\module.h>
using namespace Microsoft::WRL;

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

BEGIN_COMPONENTS_TABLE
   COMPONENT_ENTRY(
      L"{CBD410A7-C5D8-480F-9E68-A6DBD6D7E537}", 
      L"FSComponent", 
      L"Apartment", 
      L"{36DCC467-4C00-448A-A25D-3F99BF0E633E}", 
      L"1.0")
END_COMPONENTS_TABLE

class registry_handle
{
   HKEY handle;

public:
   registry_handle(HKEY const & key): handle(key)
   {
   }

   registry_handle(registry_handle&& rh)
   {
      handle = rh.handle;
      rh.handle = nullptr;
   }

   ~registry_handle()
   {
      if(handle != nullptr)
         RegCloseKey(handle);
   }

   registry_handle& operator=(registry_handle&& rh)
   {
      if(this != &rh)
      {
         if(handle != nullptr)
            RegCloseKey(handle);

         handle = rh.handle;
         rh.handle = nullptr;
      }

      return *this;
   }

   HKEY* get() throw()
   {
      return &handle;
   }

   operator HKEY() const 
   {
      return handle;
   }
};

registry_handle RegistryCreateKey(wchar_t const * keyPath, HANDLE hTransaction)
{
   registry_handle hKey = nullptr;
   auto result = ::RegCreateKeyTransacted(
      HKEY_LOCAL_MACHINE,
      keyPath,
      0,
      nullptr,
      REG_OPTION_NON_VOLATILE,
      KEY_WRITE,
      nullptr,
      hKey.get(),
      nullptr,
      hTransaction,
      nullptr);

   if (ERROR_SUCCESS != result)
   {
      SetLastError(result);
      hKey = nullptr;
   }

   return const_cast<registry_handle&&>(hKey);
}

bool RegistryCreateNameValue(HKEY hKey, wchar_t const * name, wchar_t const * value)
{
   auto result = ::RegSetValueEx(
      hKey,
      name,
      0,
      REG_SZ,
      reinterpret_cast<BYTE const*>(value),
      static_cast<DWORD>(sizeof(wchar_t)*(wcslen(value) + 1)));

   if (ERROR_SUCCESS != result)
   {
      ::SetLastError(result);
      return false;
   }

   return true;
}

bool RegistryDeleteTree(wchar_t const * keyPath, HANDLE hTransaction)
{
   registry_handle hKey = nullptr;

   auto result = ::RegOpenKeyTransacted(
      HKEY_LOCAL_MACHINE,
      keyPath,
      0,
      DELETE | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE,
      hKey.get(),
      hTransaction,
      nullptr);

   if (ERROR_SUCCESS != result && ERROR_FILE_NOT_FOUND != result)
   {
      SetLastError(result);
      return false;
   }

   if(ERROR_SUCCESS == result)
   {
      result = ::RegDeleteTree(hKey, nullptr);

      if (ERROR_SUCCESS != result)
      {
         ::SetLastError(result);
         return false;
      }
   }

   return true;
}

bool AppModule::Unregister(HANDLE hTransaction)
{
   for(auto const & entry : s_regTable)
   {
      if(!RegistryDeleteTree((std::wstring(L"Software\\Classes\\CLSID\\") + entry.Guid).data(), 
                              hTransaction))
         return false;
   }

   return true;
}

bool AppModule::Register(HANDLE hTransaction)
{
   if(!Unregister(hTransaction))
      return false;

   wchar_t filename[MAX_PATH] = { 0 };
   auto const length = ::GetModuleFileName(
      reinterpret_cast<HMODULE>(&__ImageBase), 
      filename, 
      _countof(filename));

   if(length == 0)
      return false;

   for(auto const & entry : s_regTable)
   {
      auto keyPath = std::wstring(L"Software\\Classes\\CLSID\\") + entry.Guid;

      registry_handle hKey = RegistryCreateKey(keyPath.data(), hTransaction);
      if(hKey == nullptr)
         return false;

      if(!RegistryCreateNameValue(hKey, nullptr, entry.Name))
         return false;

      registry_handle hKey2 = RegistryCreateKey((keyPath + L"\\InProcServer32").data(), hTransaction);
      if(hKey2 == nullptr)
         return false;

      if(!RegistryCreateNameValue(hKey2, nullptr, filename))
         return false;
      if(!RegistryCreateNameValue(hKey2, L"ThreadingModel", entry.ThreadingModel))
         return false;

      if (!RegistryCreateNameValue(hKey2, L"StoragePath", L"%TEMP%\\demo.bin"))
          return false;

      if(entry.TypelibGuid != nullptr)
      {
         registry_handle hKey3 = RegistryCreateKey((keyPath + L"\\TypeLib").data(), hTransaction);
         if(hKey3 == nullptr)
            return false;

         if(!RegistryCreateNameValue(hKey3, nullptr, entry.TypelibGuid))
            return false;
      }

      if(entry.Version != nullptr)
      {
         registry_handle hKey3 = RegistryCreateKey((keyPath + L"\\Version").data(), hTransaction);
         if(hKey3 == nullptr)
            return false;

         if(!RegistryCreateNameValue(hKey3, nullptr, entry.Version))
            return false;
      }
   }

   return true;
}

HRESULT AppModule::RegisterServer()
{
   HANDLE hTransaction = ::CreateTransaction(
      nullptr,                      // security attributes
      nullptr,                      // reserved
      TRANSACTION_DO_NOT_PROMOTE,   // options
      0,                            // isolation level (reserved)
      0,                            // isolation flags (reserved)
      INFINITE,                     // timeout
      nullptr                       // transaction description
      );   

   if(INVALID_HANDLE_VALUE == hTransaction)
   {
      auto lastError = ::GetLastError();
      ::CloseHandle(hTransaction);
      return HRESULT_FROM_WIN32(lastError);
   }

   if(!Register(hTransaction))
   {
      auto lastError = ::GetLastError();
      ::CloseHandle(hTransaction);
      return HRESULT_FROM_WIN32(lastError);
   }

   if(!::CommitTransaction(hTransaction))
   {
      auto lastError = ::GetLastError();
      ::CloseHandle(hTransaction);
      return HRESULT_FROM_WIN32(lastError);
   }

   ::CloseHandle(hTransaction);

   return S_OK;
}

HRESULT AppModule::UnregisterServer()
{
   HANDLE hTransaction = ::CreateTransaction(
      nullptr,                      // security attributes
      nullptr,                      // reserved
      TRANSACTION_DO_NOT_PROMOTE,   // options
      0,                            // isolation level (reserved)
      0,                            // isolation flags (reserved)
      INFINITE,                     // timeout
      nullptr                       // transaction description
      );

   if(INVALID_HANDLE_VALUE == hTransaction)
   {
      auto lastError = ::GetLastError();
      ::CloseHandle(hTransaction);
      return HRESULT_FROM_WIN32(lastError);
   }

   if(!Unregister(hTransaction))
   {
      auto lastError = ::GetLastError();
      ::CloseHandle(hTransaction);
      return HRESULT_FROM_WIN32(lastError);
   }

   if(!::CommitTransaction(hTransaction))
   {
      auto lastError = ::GetLastError();
      ::CloseHandle(hTransaction);
      return HRESULT_FROM_WIN32(lastError);
   }

   ::CloseHandle(hTransaction);

   return S_OK;
}

HRESULT AppModule::GetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
   return Module<InProc>::GetModule().GetClassObject(rclsid, riid, ppv);
}

HRESULT AppModule::CanUnloadNow()
{
   return Module<InProc>::GetModule().Terminate() ? S_OK : S_FALSE;
}

