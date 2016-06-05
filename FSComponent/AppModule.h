#pragma once

class AppModule
{
   bool Unregister(HANDLE hTransaction);
   bool Register(HANDLE hTransaction);
public:
   HRESULT RegisterServer();
   HRESULT UnregisterServer();
   HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, void** ppv);
   HRESULT CanUnloadNow();
};