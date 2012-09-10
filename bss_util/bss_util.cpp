// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// WINDOWS ONLY (right now)

#include "bss_util.h"
#include "cStr.h"
#include "os.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#include <Commdlg.h>
#include <Shlwapi.h>
#else
#include <sys/types.h>  // stat().
#include <sys/stat.h>   // stat().
#include <dirent.h> //Linux
#endif

//typedef DWORD (WINAPI *GETFINALNAMEBYHANDLE)(HANDLE,LPWSTR,DWORD,DWORD);
//static const HMODULE bssdll_KERNAL = LoadLibraryA("Kernal32.dll");
//static const GETFINALNAMEBYHANDLE bssdll_GetFinalNameByHandle = (GETFINALNAMEBYHANDLE)GetProcAddress(bssdll_KERNAL,"GetFinalPathNameByHandleW");

template<DWORD T_FLAG>
inline bool BSS_FASTCALL r_fexists(const wchar_t* path)
{
  assert(path!=0);
#ifdef BSS_PLATFORM_WIN32
  cStrW s(L"\\\\?\\"); //You must append \\?\ to the beginning of the string to allow paths up to 32767 characters long
  if(!PathIsRelativeW(path)) // But only if its an absolute path
  {
     s+=path;
     path=s;
     s.ReplaceChar('/','\\'); //This doesn't behave nicely if you have / in there instead of \ for some reason.
  }
  
  DWORD attr = GetFileAttributesW(path);

  if(attr==INVALID_FILE_ATTRIBUTES)
    return false;

  /*if(attr&FILE_ATTRIBUTE_REPARSE_POINT !=0) // Navigate through symlink
  { // This refuses to properly resolve attributes.
    assert(bssdll_GetFinalNameByHandle!=0);
    WIN32_FIND_DATAW dat;
    if(FindFirstFile(path,&dat)==INVALID_HANDLE_VALUE)
      return false;
    if(dat.dwReserved0 != IO_REPARSE_TAG_SYMLINK)
      return false;
    HANDLE hf = CreateFileW(path,NULL,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
    if(hf == INVALID_HANDLE_VALUE)
      return false;
    size_t ln = bssdll_GetFinalNameByHandle(hf,0,0,VOLUME_NAME_DOS);
    if(!ln) return false;
    s.reserve(ln);
    ln = bssdll_GetFinalNameByHandle(hf,s.UnsafeString(),ln-1,VOLUME_NAME_DOS);
    if(!ln) return false;
    s.UnsafeString()[ln]=0; //insert null terminator
    attr = GetFileAttributesW(s); // Get attributes on final path
    if(attr==INVALID_FILE_ATTRIBUTES)
      return false;
  }*/

  return ((attr&FILE_ATTRIBUTE_DIRECTORY)^T_FLAG) !=0;
#elif BSS_PLATFORM_POSIX

#endif
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FolderExists(const char* strpath)
{
  return r_fexists<0>(cStrW(strpath).c_str());
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FolderExists(const wchar_t* strpath)
{
  return r_fexists<0>(strpath);
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FileExists(const char* strpath)
{
  return r_fexists<1>(cStrW(strpath).c_str());
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FileExists(const wchar_t* strpath)
{
  return r_fexists<1>(strpath);
}

#ifdef BSS_PLATFORM_WIN32
BSS_COMPILER_DLLEXPORT
extern void BSS_FASTCALL bss_util::SetWorkDirToCur()
{
  cStrW commands(MAX_PATH);
  GetModuleFileNameW(0, commands.UnsafeString(), MAX_PATH);
  commands.UnsafeString()[wcsrchr(commands, '\\')-commands+1] = '\0';
  SetCurrentDirectoryW(commands);
}
#elif BSS_PLATFORM_POSIX

#endif

#ifdef BSS_PLATFORM_WIN32
BSS_COMPILER_DLLEXPORT
extern void BSS_FASTCALL bss_util::ForceWin64Crash() 
{ 
    typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags); 
    typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags); 
    const DWORD EXCEPTION_SWALLOWING = 0x1;
    DWORD dwFlags; 

    HMODULE kernel32 = LoadLibraryA("kernel32.dll"); 
    tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32, "GetProcessUserModeExceptionPolicy"); 
    tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32, "SetProcessUserModeExceptionPolicy"); 
    if (pGetPolicy && pSetPolicy && pGetPolicy(&dwFlags)) 
      pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING); // Turn off the filter 
}
#else
extern void BSS_FASTCALL bss_util::ForceWin32Crash() 
{ // Obviously in linux this function does nothing becuase linux isn't a BROKEN PIECE OF SHIT
}
#endif

BSS_COMPILER_DLLEXPORT extern unsigned long long BSS_FASTCALL bss_util::bssFileSize(const char* path)
{
  return bssFileSize(cStrW(path).c_str());
}
BSS_COMPILER_DLLEXPORT extern unsigned long long BSS_FASTCALL bss_util::bssFileSize(const wchar_t* path)
{
#ifdef BSS_PLATFORM_WIN32
  WIN32_FILE_ATTRIBUTE_DATA fad;

  if(GetFileAttributesExW(path, GetFileExInfoStandard, &fad)==FALSE || (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0)
    return (unsigned long long)-1;

  return (static_cast<unsigned long long>(fad.nFileSizeHigh) << (sizeof(fad.nFileSizeLow)*8)) + fad.nFileSizeLow;
#elif BSS_PLATFORM_POSIX
    struct stat path_stat;
    if(::stat(p.c_str(), &path_stat)!=0 || !S_ISREG(path_stat.st_mode))
    return (unsigned long long)-1;

    return (unsigned long long)path_stat.st_size;
#endif
}

BSS_COMPILER_DLLEXPORT
#ifdef BSS_PLATFORM_WIN32 //Windows function
extern void BSS_FASTCALL bss_util::FileDialog(wchar_t (&buf)[MAX_PATH], bool open, unsigned long flags, const char* file, const char* filter, HWND__* owner, const char* initdir, const char* defext)
{
  cStrW wfilter;
  size_t c;
  const char* i;
  for(i=filter; *((const short*)i) != 0; ++i);
  c=i-filter+1; //+1 to include null terminator
  wfilter.reserve(MultiByteToWideChar(CP_UTF8, 0, filter, c, 0, 0));
  MultiByteToWideChar(CP_UTF8, 0, filter, c, wfilter.UnsafeString(), wfilter.capacity());
  FileDialog(buf,open,flags,cStrW(file),wfilter,owner,cStrW(initdir),cStrW(defext));
}
extern void BSS_FASTCALL bss_util::FileDialog(wchar_t (&buf)[MAX_PATH], bool open, unsigned long flags, const wchar_t* file, const wchar_t* filter, HWND__* owner, const wchar_t* initdir, const wchar_t* defext)
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
//#ifdef BSS_PLATFORM_WIN32 //Windows function
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

template<class _Fn>
inline int BSS_FASTCALL r_setregvalue(HKEY__*	hOpenKey, const wchar_t* szKey, _Fn fn)
{
	BOOL 	bRetVal = FALSE;
	DWORD	dwDisposition;
	DWORD	dwReserved = 0;
	HKEY  	hTempKey = (HKEY)0;

	// Open key of interest
	// Assume all access is okay and that all keys will be stored to file
	// Utilize the default security attributes
	//if( ERROR_SUCCESS == ::RegCreateKeyEx(hOpenKey, szKey, dwReserved,(LPTSTR)0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,&hTempKey, &dwDisposition))
	if(ERROR_SUCCESS == RegCreateKeyExW(hOpenKey, szKey, dwReserved,0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,&hTempKey, &dwDisposition))
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
  return SetRegistryValue(hOpenKey, cStrW(szKey),cStrW(szValue),szData);
}

int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, const char* szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue || !szData) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue(hOpenKey,szKey,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_SZ, (LPBYTE)szData, ((DWORD)strlen(szData)+1)*sizeof(wchar_t));
  });
}

int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int32 szData)
{
  return SetRegistryValue(hOpenKey, cStrW(szKey),cStrW(szValue),szData);
}
int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int32 szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue(hOpenKey,szKey,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_DWORD, (LPBYTE)&szData, sizeof(__int32));
  });
}
int BSS_FASTCALL bss_util::SetRegistryValue64(HKEY__*	hOpenKey, const char* szKey, const char* szValue, __int64 szData)
{
  return SetRegistryValue64(hOpenKey, cStrW(szKey),cStrW(szValue),szData);
}
int BSS_FASTCALL bss_util::SetRegistryValue64(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, __int64 szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input
  
  return r_setregvalue(hOpenKey,szKey,[&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_QWORD, (LPBYTE)&szData, sizeof(__int64));
  });
}

int BSS_FASTCALL r_delregnode(HKEY__* hKeyRoot, const wchar_t* lpSubKey)
{
  LONG lResult;
  DWORD dwSize;
  wchar_t szName[MAX_PATH];
  HKEY hKey;
  FILETIME ftWrite;

  cStrW lpEnd = lpSubKey;
  lResult = RegDeleteKeyW(hKeyRoot, lpEnd);

  if (lResult == ERROR_SUCCESS) 
      return TRUE;

  lResult = RegOpenKeyExW(hKeyRoot, lpEnd, 0, KEY_READ, &hKey);

  if (lResult != ERROR_SUCCESS) 
  {
      if (lResult == ERROR_FILE_NOT_FOUND) {
          OutputDebugStringW(L"Key not found.\n");
          return TRUE;
      } 
      else {
          OutputDebugStringW(L"Error opening key.\n");
          return FALSE;
      }
  }
  // Check for an ending slash and add one if it is missing.
  if (lpEnd[lpEnd.length()-1] != L'\\') 
    lpEnd += L'\\';

  dwSize = MAX_PATH;
  lResult = RegEnumKeyExW(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);

  if (lResult == ERROR_SUCCESS) 
  {
      do {
          //lpEnd = szName;
        if (!r_delregnode(hKeyRoot, lpEnd+szName))
              break;

          dwSize = MAX_PATH;
          lResult = RegEnumKeyExW(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);
      } while (lResult == ERROR_SUCCESS);
  }

  RegCloseKey (hKey);

  lResult = RegDeleteKeyW(hKeyRoot, lpEnd);

  return lResult == ERROR_SUCCESS?TRUE:FALSE;
}

int BSS_FASTCALL bss_util::DelRegistryNode(HKEY__* hKeyRoot, const char* lpSubKey)
{
  return r_delregnode(hKeyRoot, cStrW(lpSubKey).c_str());
}

int BSS_FASTCALL bss_util::DelRegistryNode(HKEY__* hKeyRoot, const wchar_t* lpSubKey)
{
  return r_delregnode(hKeyRoot,lpSubKey);
}

#endif