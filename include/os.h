// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_OS_H__
#define __BSS_OS_H__

//#include <vector>
#include "bss_util.h" // bssdll_delete

#ifdef BSS_PLATFORM_WIN32
struct HWND__;
struct HKEY__; //Include WinReg.h to get access to the root key handles (e.g. HKEY_LOCAL_MACHINE)
#endif

namespace bss_util {
  //BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL GetDirFiles(const char* cdir, std::vector<cStr>* files);
  //BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL GetDirFiles(const wchar_t* cdir, std::vector<cStrW>* files);
  //BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL DelDir(const char* cdir, bool files=true);
  //BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL DelDir(const wchar_t* cdir, bool files=true);
  //BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL CreateDir(const char* cdir);
  //BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL CreateDir(const wchar_t* cdir);
  BSS_COMPILER_DLLEXPORT extern bool BSS_FASTCALL FolderExists(const char* strpath); 
  BSS_COMPILER_DLLEXPORT extern bool BSS_FASTCALL FileExists(const char* strpath);
  BSS_COMPILER_DLLEXPORT extern void BSS_FASTCALL AlertBox(const char* text, const char* caption, int type=0);
  BSS_COMPILER_DLLEXPORT extern std::unique_ptr<char[],bssdll_delete<char[]>> BSS_FASTCALL FileDialog(bool open, unsigned long flags, const char* file, const char* filter="All Files (*.*)\0*.*\0", const char* initdir=0, const char* defext=0);
  
#ifdef BSS_PLATFORM_WIN32
  BSS_COMPILER_DLLEXPORT extern bool BSS_FASTCALL FolderExists(const wchar_t* strpath); 
  BSS_COMPILER_DLLEXPORT extern bool BSS_FASTCALL FileExists(const wchar_t* strpath);
  BSS_COMPILER_DLLEXPORT extern void BSS_FASTCALL AlertBox(const wchar_t* text, const wchar_t* caption, int type=0);

  BSS_COMPILER_DLLEXPORT extern std::unique_ptr<char[],bssdll_delete<char[]>> BSS_FASTCALL FileDialog(bool open, unsigned long flags, const wchar_t* file, const wchar_t* filter=L"All Files (*.*)\0*.*\0", const wchar_t* initdir=0, const wchar_t* defext=0, HWND__* owner=0);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, const char* szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, const char* szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int32 szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int32 szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue64(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int64 szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue64(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int64 szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL DelRegistryNode(HKEY__* hKeyRoot, const char* lpSubKey);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL DelRegistryNode(HKEY__* hKeyRoot, const wchar_t* lpSubKey);
#endif
}

#endif
