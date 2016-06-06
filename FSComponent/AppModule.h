#pragma once
/**
@file
AppModule definition
@author Marius Bancila
http://www.codeproject.com/Articles/654343/Authoring-and-consuming-classic-COM-components-wit
*/

// Class implements COM self registration of WRL component
// got it from CodeProject, published by free license
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