// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// WINDOWS ONLY (right now)

#include "bss_util.h"
#include "cStr.h"
#include "os.h"
#include <sstream>
#include <iomanip>
#include <sys/types.h>
#include <stdio.h>
#include <io.h>   // access().
#include <sys/types.h>  // stat().
#include <sys/stat.h>   // stat().
#if defined(WIN32) || defined(_WIN32)
#include "bss_win32_includes.h"
#include <Commdlg.h>
#include <tchar.h> 
#else
#include <dirent.h> //Linux
#endif
#include "cAVLtree.h"
#include "cMiniList.h"
#include "cBinaryHeap.h"
#include <string.h>
#include "bss_deprecated.h"

//#if defined(DEBUG) || defined(_DEBUG)
//#pragma comment(lib, "libboost_filesystem-vc100-mt-sgd.lib")
//#pragma comment(lib, "libboost_system-vc100-mt-sgd.lib")
//#else
//#pragma comment(lib, "libboost_filesystem-vc100-mt-s.lib")
//#pragma comment(lib, "libboost_system-vc100-mt-s.lib")
//#endif

#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_ALL_NO_DEPRECATED
#include <boost/filesystem.hpp>

using namespace boost;

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FolderExists(const char* strpath)
{
  filesystem3::path p(strpath);
  if(!filesystem3::is_directory(p)) return false; //the folder can't exist if its not a folder
  return filesystem3::exists(p,system::error_code());
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FolderExists(const wchar_t* strpath)
{
  filesystem3::path p(strpath);
  if(!filesystem3::is_directory(p)) return false; //the folder can't exist if its not a folder
  return filesystem3::exists(p,system::error_code());
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FileExists(const char* strpath)
{
  filesystem3::path p(strpath);
  if(filesystem3::is_directory(p)) return false; //the file can't exist if its not a file
  return filesystem3::exists(p,system::error_code());
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FileExists(const wchar_t* strpath)
{
  filesystem3::path p(strpath);
  if(filesystem3::is_directory(p)) return false; //the file can't exist if its not a file
  return filesystem3::exists(p,system::error_code());
}

BSS_COMPILER_DLLEXPORT
extern void BSS_FASTCALL bss_util::SetWorkDirToCur()
{
  cStrW commands(MAX_PATH);
  GetModuleFileNameW(0, commands.UnsafeString(), MAX_PATH);
  commands.UnsafeString()[wcsrchr(commands, '\\')-commands+1] = '\0';
  SetCurrentDirectoryW(commands);
}

BSS_COMPILER_DLLEXPORT extern unsigned long long BSS_FASTCALL bss_util::bssFileSize(const char* path)
{
  filesystem3::path p(path);
  return filesystem3::file_size(path);
}
BSS_COMPILER_DLLEXPORT extern unsigned long long BSS_FASTCALL bss_util::bssFileSize(const wchar_t* path)
{
  filesystem3::path p(path);
  return filesystem3::file_size(path);
}

BSS_COMPILER_DLLEXPORT
#if defined(WIN32) || defined(_WIN32) //Windows function
extern void BSS_FASTCALL bss_util::FileDialog(char (&buf)[MAX_PATH], bool open, int flags, const char* file, const char* filter, HWND__* owner, const char* initdir, const char* defext)
{
  //char* buf = (char*)calloc(MAX_PATH,1);
  GetCurrentDirectoryA(MAX_PATH,buf);
  cStr curdirsave(buf);
  ZeroMemory(buf, MAX_PATH);

  if(file!=0) STRNCPY(buf, MAX_PATH, file, bssmin(strlen(file),MAX_PATH-1));

  OPENFILENAMEA ofn;
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(OPENFILENAMEA);
  ofn.hwndOwner = owner;
  ofn.lpstrFilter = filter;
  ofn.lpstrFile = buf;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = (open?OFN_EXPLORER|OFN_FILEMUSTEXIST:OFN_EXPLORER) | flags;
  ofn.lpstrDefExt = defext;
  ofn.lpstrInitialDir = initdir;

  BOOL res = open?GetOpenFileNameA(&ofn):GetSaveFileNameA(&ofn);

  SetCurrentDirectoryA(curdirsave); //There is actually a flag that's supposed to do this for us but it doesn't work on XP for file open, so we have to do it manually just to be sure
  if(!res) buf[0]='\0';
  //return (void*)buf;
}
extern void BSS_FASTCALL bss_util::FileDialog(wchar_t (&buf)[MAX_PATH], bool open, int flags, const wchar_t* file, const wchar_t* filter, HWND__* owner, const wchar_t* initdir, const wchar_t* defext)
{
  //char* buf = (char*)calloc(MAX_PATH,1);
  GetCurrentDirectoryW(MAX_PATH,buf);
  cStrW curdirsave(buf);
  ZeroMemory(buf, MAX_PATH);

  if(file!=0) WCSNCPY(buf, MAX_PATH, file, bssmin(wcslen(file),MAX_PATH-1));

  OPENFILENAMEW ofn;
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(OPENFILENAMEW);
  ofn.hwndOwner = owner;
  ofn.lpstrFilter = filter;
  ofn.lpstrFile = buf;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = (open?OFN_EXPLORER|OFN_FILEMUSTEXIST:OFN_EXPLORER) | flags;
  ofn.lpstrDefExt = defext;
  ofn.lpstrInitialDir = initdir;

  BOOL res = open?GetOpenFileNameW(&ofn):GetSaveFileNameW(&ofn);

  SetCurrentDirectoryW(curdirsave); //There is actually a flag that's supposed to do this for us but it doesn't work on XP for file open, so we have to do it manually just to be sure
  if(!res) buf[0]='\0';
  //return (void*)buf;
}

extern long BSS_FASTCALL bss_util::GetTimeZoneMinutes()
{
  TIME_ZONE_INFORMATION dtime;
  DWORD r=GetTimeZoneInformation(&dtime);
  switch(r)
  {
  case 0:
  case 1: //None or unknown daylight savings time
    return -(dtime.Bias+dtime.StandardBias); //This should be negated because the equation is UTC = local time + bias, so that means UTC - bias = local time
  case 2: //Using daylight savings time
    return -(dtime.Bias+dtime.DaylightBias);
  }
  return 0; //error
}

//bss_util::FREEPTRDLL::FREEPTRDLL(void* ptr) : _ptr(ptr)
//{
//}
//
//bss_util::FREEPTRDLL::~FREEPTRDLL()
//{
//  if(_ptr) free(_ptr);
//}


//namespace bss_util {
//  bool BSS_FASTCALL _exists(const char* path, bool isdir)
//  {
//    if (_access_s(path,0) == 0)
//    {
//        struct stat status;
//        if(stat(path, &status)<0)
//          return false; //fail
//        
//        return isdir?(status.st_mode & S_IFDIR)!=0:(status.st_mode & S_IFDIR)==0; //if isdir is true it should be a directory, if false then it shouldn't
//    }
//    return false;
//  }
//  bool BSS_FASTCALL _exists(const wchar_t* path, bool isdir)
//  {
//    if (_waccess_s(path,0) == 0)
//    {
//        struct _stat64i32 status;
//        if(_wstat(path, &status)<0)
//          return false; //fail
//        
//        return isdir?(status.st_mode & S_IFDIR)!=0:(status.st_mode & S_IFDIR)==0; //if isdir is true it should be a directory, if false then it shouldn't
//    }
//    return false;
//  }
//}
//
//extern int BSS_FASTCALL bss_util::DelDir(const char* cdir, bool files)
//{
//  WIN32_FIND_DATAA ffd;
//  HANDLE hdir = INVALID_HANDLE_VALUE;
//
//  cStr dir(cdir);
//  if(dir[dir.length()-1] != '\\') dir += '\\';
//  hdir = FindFirstFile(dir+"*", &ffd);
//
//  while(hdir > 0 && hdir != INVALID_HANDLE_VALUE)
//  {
//    if(STRICMP(ffd.cFileName, ".")!=0 && STRICMP(ffd.cFileName, "..")!=0)
//    {
//      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//        DelDir(dir+ffd.cFileName, files);
//      else if(files)
//        remove(dir+ffd.cFileName);
//    }
//    if(FindNextFileA(hdir, &ffd) <= 0) break; //either we're done or it failed
//  }
//
//  RemoveDirectoryA(dir.substr(0, dir.size()-1).c_str());
//  FindClose(hdir);
//  return 0;
//}
//
//extern int BSS_FASTCALL bss_util::DelDir(const wchar_t* cdir, bool files)
//{
//  WIN32_FIND_DATAW ffd;
//  HANDLE hdir = INVALID_HANDLE_VALUE;
//
//  cStrW dir(cdir);
//  if(dir[dir.length()-1] != L'\\') dir += L'\\';
//  hdir = FindFirstFileW(dir+L"*", &ffd);
//
//  while(hdir > 0 && hdir != INVALID_HANDLE_VALUE)
//  {
//    if(WCSICMP(ffd.cFileName, L".")!=0 && WCSICMP(ffd.cFileName, L"..")!=0)
//    {
//      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//        DelDir(dir+ffd.cFileName, files);
//      else if(files)
//        _wremove(dir+ffd.cFileName);
//    }
//    if(FindNextFileW(hdir, &ffd) <= 0) break; //either we're done or it failed
//  }
//
//  RemoveDirectoryW(dir.substr(0, dir.size()-1).c_str());
//  FindClose(hdir);
//  return 0;
//}
//
//BSS_COMPILER_DLLEXPORT
//#if defined(WIN32) || defined(_WIN32) //Windows function
//extern int BSS_FASTCALL bss_util::GetDirFiles(const char* cdir, std::vector<cStr>* files)
//{
//  WIN32_FIND_DATAA ffd;
//  HANDLE hdir = INVALID_HANDLE_VALUE;
//
//  cStr dir(cdir);
//  if(dir[dir.length()-1] != '\\') dir += '\\';
//  hdir = FindFirstFileA(dir+"*", &ffd);
//
//  while(hdir > 0 && hdir !=INVALID_HANDLE_VALUE)
//  {
//    if(STRICMP(ffd.cFileName, ".")!=0 && STRICMP(ffd.cFileName, "..")!=0)
//    {
//      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//        GetDirFiles(dir+ffd.cFileName, files);
//      else
//        files->push_back(cStr(dir,ffd.cFileName).c_str());
//    }
//    if(FindNextFileA(hdir, &ffd) <= 0) break; //either we're done or it failed
//  }
//
//  FindClose(hdir);
//  return 0;
//}
//extern int BSS_FASTCALL bss_util::GetDirFiles(const wchar_t* cdir, std::vector<cStrW>* files)
//{
//  WIN32_FIND_DATAW ffd;
//  HANDLE hdir = INVALID_HANDLE_VALUE;
//
//  cStrW dir(cdir);
//  if(dir[dir.length()-1] != L'\\') dir += L'\\';
//  hdir = FindFirstFileW(dir+L"*", &ffd);
//
//  while(hdir > 0 && hdir !=INVALID_HANDLE_VALUE)
//  {
//    if(WCSICMP(ffd.cFileName, L".")!=0 && WCSICMP(ffd.cFileName, L"..")!=0)
//    {
//      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//        GetDirFiles(dir+ffd.cFileName, files);
//      else
//        files->push_back(cStrW(dir,ffd.cFileName).c_str());
//    }
//    if(FindNextFileW(hdir, &ffd) <= 0) break; //either we're done or it failed
//  }
//
//  FindClose(hdir);
//  return 0;
//}
//#else //Linux function
//extern int BSS_FASTCALL bss_util::GetDirFiles(const char* cdir, std::vector<cStr>* files)
//{
//  DIR *dp;
//  struct dirent *dirp;
//  if((dp = opendir(cdir)) == NULL)
//    return errno;
//  cStr dir(cdir);
//  if(dir[dir.length()-1] != '\\') dir += '\\';
//
//  while((dirp = readdir(dp)) != NULL)
//  {
//    struct stat _stat;
//    if(lstat(dirp->d_name, &_stat) != 0 && stricmp(dirp->d_name, ".")!=0 && stricmp(dirp->d_name, "..")!=0)
//    {
//      if(S_ISDIR(_stat.st_mode))
//        GetDirFiles(dir+dirp->d_name,files);
//      else
//        files->push_back(cStr(dir, dirp->d_name).c_str());
//    }
//  }
//
//  closedir(dp);
//  return 0;
//}
//#endif
//
//extern int BSS_FASTCALL bss_util::CreateDir(const char* path)
//{
//  cStr hold(path);
//  hold.ReplaceChar('/', '\\');
//  if(strchr(hold, '.') == NULL && hold[hold.size()-1] != '\\') hold += '\\';
//  const char* pos = strchr(hold,'\\');
//  while(pos)
//  {
//    cStr temppath = hold.substr(0, pos-(const char*)hold);
//    if(!FolderExists(temppath))
//      CreateDirectoryA(temppath, 0);
//    pos = strchr(++pos,'\\');
//  }
//
//  return 0;
//}
//extern int BSS_FASTCALL bss_util::CreateDir(const wchar_t* path)
//{
//  cStrW hold(path);
//  hold.ReplaceChar('/', '\\');
//  if(wcschr(hold, '.') == NULL && hold[hold.size()-1] != '\\') hold += '\\';
//  const wchar_t* pos = wcschr(hold,'\\');
//  while(pos)
//  {
//    cStrW temppath = hold.substr(0, pos-(const wchar_t*)hold);
//    if(!FolderExists(temppath))
//      CreateDirectoryW(temppath, 0);
//    pos = wcschr(++pos,'\\');
//  }
//
//  return 0;
//}

template<typename C, LSTATUS (BSS_COMPILER_STDCALL *REGCREATEKEYEX)(HKEY,const C*,DWORD,C*,DWORD,REGSAM,const LPSECURITY_ATTRIBUTES,PHKEY,LPDWORD),class _Fn>
inline int BSS_FASTCALL r_setregvalue(HKEY__*	hOpenKey, const C* szKey, const C* szValue, _Fn fn)
{
	BOOL 	bRetVal = FALSE;
	DWORD	dwDisposition;
	DWORD	dwReserved = 0;
	HKEY  	hTempKey = (HKEY)0;

	// Open key of interest
	// Assume all access is okay and that all keys will be stored to file
	// Utilize the default security attributes
	//if( ERROR_SUCCESS == ::RegCreateKeyEx(hOpenKey, szKey, dwReserved,(LPTSTR)0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,&hTempKey, &dwDisposition))
	if(ERROR_SUCCESS == REGCREATEKEYEX(hOpenKey, szKey, dwReserved,0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,&hTempKey, &dwDisposition))
  {
    if(fn(hTempKey, dwReserved) == ERROR_SUCCESS)
      bRetVal = TRUE;
	}

	// close opened key
	if( hTempKey )
		::RegCloseKey(hTempKey);

	return bRetVal;
}

int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, const char* szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue || !szData) { ::SetLastError(E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue<char,&RegCreateKeyExA>(hOpenKey,szKey,szValue,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExA(hTempKey, (LPSTR)szValue, dwReserved, REG_SZ, (LPBYTE)szData, (lstrlenA(szData)+1)*sizeof(char));
  });
}

int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, const wchar_t* szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue || !szData) { ::SetLastError(E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue<wchar_t,&RegCreateKeyExW>(hOpenKey,szKey,szValue,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_SZ, (LPBYTE)szData, (lstrlenW(szData)+1)*sizeof(wchar_t));
  });
}

int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int32 szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError(E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue<char,&RegCreateKeyExA>(hOpenKey,szKey,szValue,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExA(hTempKey, (LPSTR)szValue, dwReserved, REG_DWORD, (LPBYTE)&szData, sizeof(__int32));
  });
}
int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int32 szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError(E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue<wchar_t,&RegCreateKeyExW>(hOpenKey,szKey,szValue,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_DWORD, (LPBYTE)&szData, sizeof(__int32));
  });
}
int BSS_FASTCALL bss_util::SetRegistryValue64(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int64 szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError(E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue<char,&RegCreateKeyExA>(hOpenKey,szKey,szValue,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExA(hTempKey, (LPSTR)szValue, dwReserved, REG_QWORD, (LPBYTE)&szData, sizeof(__int64));
  });
}
int BSS_FASTCALL bss_util::SetRegistryValue64(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int64 szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError(E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue<wchar_t,&RegCreateKeyExW>(hOpenKey,szKey,szValue,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_QWORD, (LPBYTE)&szData, sizeof(__int64));
  });
}
  
template<typename C, LSTATUS (BSS_COMPILER_STDCALL *REGOPENKEYEX)(HKEY,const C*,DWORD,REGSAM,PHKEY),
  LSTATUS (BSS_COMPILER_STDCALL *REGDELETEKEY)(HKEY,const C*), 
  LSTATUS (BSS_COMPILER_STDCALL *REGENUMKEYEX)(HKEY,DWORD,C*,LPDWORD,LPDWORD,C*,LPDWORD,PFILETIME)>
int BSS_FASTCALL r_delregnode(HKEY__* hKeyRoot, const C* lpSubKey)
{
  LONG lResult;
  DWORD dwSize;
  C szName[MAX_PATH];
  HKEY hKey;
  FILETIME ftWrite;

  lResult = REGDELETEKEY(hKeyRoot, lpSubKey);

  if (lResult == ERROR_SUCCESS) 
      return TRUE;

  lResult = REGOPENKEYEX(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

  if (lResult != ERROR_SUCCESS) 
  {
      if (lResult == ERROR_FILE_NOT_FOUND) {
          OutputDebugString("Key not found.\n");
          return TRUE;
      } 
      else {
          OutputDebugString("Error opening key.\n");
          return FALSE;
      }
  }
  // Check for an ending slash and add one if it is missing.
  cStrT<C> lpEnd = lpSubKey;
  if (lpEnd[lpEnd.length()-1] != '\\') 
    lpEnd += '\\';

  dwSize = MAX_PATH;
  lResult = REGENUMKEYEX(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);

  if (lResult == ERROR_SUCCESS) 
  {
      do {
          //lpEnd = szName;
          if (!r_delregnode<C,REGOPENKEYEX,REGDELETEKEY,REGENUMKEYEX>(hKeyRoot, lpEnd+szName))
              break;

          dwSize = MAX_PATH;
          lResult = REGENUMKEYEX(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);
      } while (lResult == ERROR_SUCCESS);
  }

  lpEnd.SetSize(lpEnd.length());
  RegCloseKey (hKey);

  lResult = REGDELETEKEY(hKeyRoot, lpEnd);

  return lResult == ERROR_SUCCESS?TRUE:FALSE;
}

int BSS_FASTCALL bss_util::DelRegistryNode(HKEY__* hKeyRoot, const char* lpSubKey)
{
  return r_delregnode<char, &RegOpenKeyExA,&RegDeleteKeyA, &RegEnumKeyExA>(hKeyRoot,lpSubKey);
}

int BSS_FASTCALL bss_util::DelRegistryNode(HKEY__* hKeyRoot, const wchar_t* lpSubKey)
{
  return r_delregnode<wchar_t, &RegOpenKeyExW,&RegDeleteKeyW, &RegEnumKeyExW>(hKeyRoot,lpSubKey);
}

#endif