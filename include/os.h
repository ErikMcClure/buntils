// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_OS_H__
#define __BSS_OS_H__

#include <vector>
#include "bss_dlldef.h"
#include "cStr.h"
struct HWND__;
struct HKEY__; //Include WinReg.h to get access to the root key handles (e.g. HKEY_LOCAL_MACHINE)

namespace bss_util {
  //__declspec(dllexport) extern int BSS_FASTCALL GetDirFiles(const char* cdir, std::vector<cStr>* files);
  //__declspec(dllexport) extern int BSS_FASTCALL GetDirFiles(const wchar_t* cdir, std::vector<cStrW>* files);
  //__declspec(dllexport) extern int BSS_FASTCALL DelDir(const char* cdir, bool files=true);
  //__declspec(dllexport) extern int BSS_FASTCALL DelDir(const wchar_t* cdir, bool files=true);
  //__declspec(dllexport) extern int BSS_FASTCALL CreateDir(const char* cdir);
  //__declspec(dllexport) extern int BSS_FASTCALL CreateDir(const wchar_t* cdir);
  __declspec(dllexport) extern bool BSS_FASTCALL FolderExists(const char* strpath); 
  __declspec(dllexport) extern bool BSS_FASTCALL FolderExists(const wchar_t* strpath); 
  __declspec(dllexport) extern bool BSS_FASTCALL FileExists(const char* strpath);
  __declspec(dllexport) extern bool BSS_FASTCALL FileExists(const wchar_t* strpath);
  __declspec(dllexport) extern void BSS_FASTCALL FileDialog(char (&buf)[260], bool open, int flags, const char* file=0, const char* filter="All Files (*.*)\0*.*\0", HWND__* owner=0, const char* initdir=0, const char* defext=0);
  __declspec(dllexport) extern void BSS_FASTCALL FileDialog(wchar_t (&buf)[260], bool open, int flags, const wchar_t* file=0, const wchar_t* filter=L"All Files (*.*)\0*.*\0", HWND__* owner=0, const wchar_t* initdir=0, const wchar_t* defext=0);
  
#if defined(WIN32) || defined(_WINDOWS)
  __declspec(dllexport) extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, const char* szData);
  __declspec(dllexport) extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, const wchar_t* szData);
  __declspec(dllexport) extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int32 szData);
  __declspec(dllexport) extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int32 szData);
  __declspec(dllexport) extern int BSS_FASTCALL SetRegistryValue64(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int64 szData);
  __declspec(dllexport) extern int BSS_FASTCALL SetRegistryValue64(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int64 szData);
  __declspec(dllexport) extern int BSS_FASTCALL DelRegistryNode(HKEY__* hKeyRoot, const char* lpSubKey);
  __declspec(dllexport) extern int BSS_FASTCALL DelRegistryNode(HKEY__* hKeyRoot, const wchar_t* lpSubKey);
#endif
}

#endif