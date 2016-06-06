#include "stdafx.h"
#include "AppModule.h"
#include "log.h"

static AppModule s_Module;

HRESULT __stdcall DllRegisterServer()
{
    return s_Module.RegisterServer();
}

HRESULT __stdcall DllUnregisterServer()
{
    return s_Module.UnregisterServer();
}

HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    return s_Module.GetClassObject(rclsid, riid, ppv);
}

HRESULT __stdcall DllCanUnloadNow()
{
    return s_Module.CanUnloadNow();
}

BOOL __stdcall DllMain(HINSTANCE module, DWORD reason, void*)
{
    // setup logging level
    g_level = boost::log::trivial::severity_level::error;

    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);
    }

    return TRUE;
}