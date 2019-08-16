// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_OS_H__
#define __BSS_OS_H__

#include "bss_util.h" // bssDLLDelete
#include "Str.h"

#ifdef BSS_PLATFORM_WIN32
struct HWND__;
struct HKEY__; //Include WinReg.h to get access to the root key handles (e.g. HKEY_LOCAL_MACHINE)
#endif

namespace bss {
  BSS_COMPILER_DLLEXPORT extern void AlertBox(const char* text, const char* caption, int type = 0);
  BSS_COMPILER_DLLEXPORT extern std::unique_ptr<char[], bssDLLDelete<char[]>> FileDialog(bool open, unsigned long flags, const char* file, const char* filter = "All Files (*.*)\0*.*\0", const char* initdir = 0, const char* defext = 0);
  BSS_COMPILER_DLLEXPORT extern std::unique_ptr<char[], bssDLLDelete<char[]>> GetFontPath(const char* family, int weight, bool italic);

#ifdef BSS_PLATFORM_WIN32
  BSS_COMPILER_DLLEXPORT extern void AlertBoxW(const wchar_t* text, const wchar_t* caption, int type = 0);

  BSS_COMPILER_DLLEXPORT extern std::unique_ptr<char[], bssDLLDelete<char[]>> FileDialog(bool open, unsigned long flags, const wchar_t* file, const wchar_t* filter = L"All Files (*.*)\0*.*\0", const wchar_t* initdir = 0, const wchar_t* defext = 0, HWND__* owner = 0);
  BSS_COMPILER_DLLEXPORT extern int SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, const char* szData);
  BSS_COMPILER_DLLEXPORT extern int SetRegistryValueW(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, const char* szData);
  BSS_COMPILER_DLLEXPORT extern int SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, int32_t szData);
  BSS_COMPILER_DLLEXPORT extern int SetRegistryValueW(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, int32_t szData);
  BSS_COMPILER_DLLEXPORT extern int SetRegistryValue64(HKEY__*	hOpenKey, const char* szKey, const char* szValue, int64_t szData);
  BSS_COMPILER_DLLEXPORT extern int SetRegistryValue64W(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, int64_t szData);
  BSS_COMPILER_DLLEXPORT extern int DelRegistryNode(HKEY__* hKeyRoot, const char* lpSubKey);
  BSS_COMPILER_DLLEXPORT extern int DelRegistryNodeW(HKEY__* hKeyRoot, const wchar_t* lpSubKey);
  BSS_COMPILER_DLLEXPORT extern int64_t GetRegistryValue(HKEY__* hKeyRoot, const char* szKey, const char* szValue, unsigned char* data, unsigned long sz);
  BSS_COMPILER_DLLEXPORT extern int64_t GetRegistryValueW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, unsigned char* data, unsigned long sz);
  BSS_COMPILER_DLLEXPORT extern int GetRegistryValueDWORD(HKEY__* hKeyRoot, const char* szKey, const char* szValue, unsigned long* data);
  BSS_COMPILER_DLLEXPORT extern int GetRegistryValueDWORDW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, unsigned long* data);
  BSS_COMPILER_DLLEXPORT extern int GetRegistryValueQWORD(HKEY__* hKeyRoot, const char* szKey, const char* szValue, unsigned long long* data);
  BSS_COMPILER_DLLEXPORT extern int GetRegistryValueQWORDW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, unsigned long long* data);
#else
  BSS_COMPILER_DLLEXPORT extern int ListDir(const char* dir, std::vector<Str>& files, char flags); // Setting flags to 1 will do a recursive search. Setting flags to 2 will return directory+file names. Setting flags to 3 will both be recursive and return directory names.
#endif
}

#endif
