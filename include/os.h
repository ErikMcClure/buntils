// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_OS_H__
#define __BSS_OS_H__

#include "bss_util.h" // bssdll_delete
#include "cStr.h"

#ifdef BSS_PLATFORM_WIN32
struct HWND__;
struct HKEY__; //Include WinReg.h to get access to the root key handles (e.g. HKEY_LOCAL_MACHINE)
#endif

namespace bss_util {
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL DelDir(const char* dir, bool recursive = true);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL CreateDir(const char* dir, bool recursive = true);
  BSS_COMPILER_DLLEXPORT extern bool BSS_FASTCALL FolderExists(const char* strpath); 
  BSS_COMPILER_DLLEXPORT extern bool BSS_FASTCALL FileExists(const char* strpath);
  BSS_COMPILER_DLLEXPORT extern void BSS_FASTCALL AlertBox(const char* text, const char* caption, int type = 0);
  BSS_COMPILER_DLLEXPORT extern std::unique_ptr<char[], bssdll_delete<char[]>> BSS_FASTCALL FileDialog(bool open, unsigned long flags, const char* file, const char* filter = "All Files (*.*)\0*.*\0", const char* initdir = 0, const char* defext = 0);

#ifdef BSS_PLATFORM_WIN32
  BSS_COMPILER_DLLEXPORT extern bool BSS_FASTCALL FolderExistsW(const wchar_t* strpath);
  BSS_COMPILER_DLLEXPORT extern bool BSS_FASTCALL FileExistsW(const wchar_t* strpath);
  BSS_COMPILER_DLLEXPORT extern void BSS_FASTCALL AlertBoxW(const wchar_t* text, const wchar_t* caption, int type=0);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL DelDirW(const wchar_t* dir, bool recursive = true);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL CreateDirW(const wchar_t* dir, bool recursive = true);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL _listdir(const wchar_t* cdir, void(BSS_FASTCALL *fn)(const wchar_t*, std::vector<cStr>*), std::vector<cStr>* files, char flags);
  static void BSS_FASTCALL _listdir_r(const wchar_t* f, std::vector<cStr>* files) { files->push_back(f); } // a stupidly roundabout way of sidestepping the DLL boundary problem
  BSS_FORCEINLINE int BSS_FASTCALL ListDirW(const wchar_t* dir, std::vector<cStr>& files, char flags) { return _listdir(dir, &_listdir_r, &files, flags); } // Setting flags to 1 will do a recursive search. Setting flags to 2 will return directory+file names.
  BSS_FORCEINLINE int BSS_FASTCALL ListDir(const char* dir, std::vector<cStr>& files, char flags) { return ListDirW(BSSPOSIX_WCHAR(dir), files, flags); }   // Setting flags to 3 will both be recursive and return directory names.

  BSS_COMPILER_DLLEXPORT extern std::unique_ptr<char[],bssdll_delete<char[]>> BSS_FASTCALL FileDialog(bool open, unsigned long flags, const wchar_t* file, const wchar_t* filter=L"All Files (*.*)\0*.*\0", const wchar_t* initdir=0, const wchar_t* defext=0, HWND__* owner=0);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, const char* szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValueW(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, const char* szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int32 szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValueW(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int32 szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue64(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int64 szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL SetRegistryValue64W(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int64 szData);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL DelRegistryNode(HKEY__* hKeyRoot, const char* lpSubKey);
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL DelRegistryNodeW(HKEY__* hKeyRoot, const wchar_t* lpSubKey);
#else
  BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL ListDir(const char* dir, std::vector<cStr>& files, char flags); // Setting flags to 1 will do a recursive search. Setting flags to 2 will return directory+file names. Setting flags to 3 will both be recursive and return directory names.
#endif
}

#endif
