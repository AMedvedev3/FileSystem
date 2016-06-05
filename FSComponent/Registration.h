#pragma once

struct RegistryEntry
{
   wchar_t const *      Guid;
   wchar_t const *      Name;
   wchar_t const *      ThreadingModel;
   wchar_t const *      TypelibGuid;
   wchar_t const *      Version;
};

#define BEGIN_COMPONENTS_TABLE static RegistryEntry s_regTable [] = {
#define END_COMPONENTS_TABLE };
#define COMPONENT_ENTRY(clsid,name,model,typelib,ver)    {clsid,name,model,typelib,ver},

