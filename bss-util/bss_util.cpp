// Copyright ©2016 Black Sphere Studios
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
#include <dirent.h>
#include <unistd.h> // rmdir()
//#include <gtkmm.h> // file dialog
#endif

//typedef DWORD (WINAPI *GETFINALNAMEBYHANDLE)(HANDLE,LPWSTR,DWORD,DWORD);
//static const HMODULE bssdll_KERNAL = LoadLibraryA("Kernal32.dll");
//static const GETFINALNAMEBYHANDLE bssdll_GetFinalNameByHandle = (GETFINALNAMEBYHANDLE)GetProcAddress(bssdll_KERNAL,"GetFinalPathNameByHandleW");

#ifdef BSS_PLATFORM_WIN32
template<DWORD T_FLAG>
inline bool BSS_FASTCALL r_fexists(const wchar_t* path)
{
  assert(path!=0);
  cStrW s(L"\\\\?\\"); //You must append \\?\ to the beginning of the string to allow paths up to 32767 characters long
  if(!PathIsRelativeW(path)) // But only if its an absolute path
  {
    s+=path;
    path=s;
    s.ReplaceChar('/', '\\'); //This doesn't behave nicely if you have / in there instead of \ for some reason.
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
  HANDLE hf = CreateFileW(path,nullptr,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);
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
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FileExistsW(const wchar_t* strpath)
{
  return r_fexists<1>(strpath);
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FolderExistsW(const wchar_t* strpath)
{
  return r_fexists<0>(strpath);
}

#else // BSS_PLATFORM_POSIX
template<int T_FLAG>
inline bool BSS_FASTCALL r_fexists(const char* path)
{
  struct stat st;
  if(stat(path, &st)!=0)
    return false;
  return ((S_ISDIR(st.st_mode)!=0)^T_FLAG)!=0;
}
#endif

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FolderExists(const char* strpath)
{
  return r_fexists<0>(BSSPOSIX_WCHAR(strpath));
}

BSS_COMPILER_DLLEXPORT
extern bool BSS_FASTCALL bss_util::FileExists(const char* strpath)
{
  return r_fexists<1>(BSSPOSIX_WCHAR(strpath));
}

BSS_COMPILER_DLLEXPORT
extern void BSS_FASTCALL bss_util::SetWorkDirToCur()
{
#ifdef BSS_PLATFORM_WIN32
  cStrW commands(MAX_PATH);
  GetModuleFileNameW(0, commands.UnsafeString(), MAX_PATH);
  commands.UnsafeString()[wcsrchr(commands, '\\')-commands+1] = '\0';
  SetCurrentDirectoryW(commands);
#endif
}

BSS_COMPILER_DLLEXPORT
extern void BSS_FASTCALL bss_util::ForceWin64Crash()
{
#ifdef BSS_PLATFORM_WIN32
  typedef BOOL(WINAPI *tGetPolicy)(LPDWORD lpFlags);
  typedef BOOL(WINAPI *tSetPolicy)(DWORD dwFlags);
  const DWORD EXCEPTION_SWALLOWING = 0x1;
  DWORD dwFlags;

  HMODULE kernel32 = LoadLibraryA("kernel32.dll");
  assert(kernel32!=0);
  tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32, "GetProcessUserModeExceptionPolicy");
  tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32, "SetProcessUserModeExceptionPolicy");
  if(pGetPolicy && pSetPolicy && pGetPolicy(&dwFlags))
    pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING); // Turn off the filter 
#endif
  // Obviously in linux this function does nothing becuase linux isn't a BROKEN PIECE OF SHIT
}

BSS_COMPILER_DLLEXPORT extern unsigned long long BSS_FASTCALL bss_util::bssFileSize(const char* path)
{
#ifdef BSS_PLATFORM_WIN32
  return bssFileSize(cStrW(path).c_str());
#else // BSS_PLATFORM_POSIX
  struct stat path_stat;
  if(::stat(path, &path_stat)!=0 || !S_ISREG(path_stat.st_mode))
    return (unsigned long long)-1;

  return (unsigned long long)path_stat.st_size;
#endif
}
BSS_COMPILER_DLLEXPORT extern unsigned long long BSS_FASTCALL bss_util::bssFileSize(const wchar_t* path)
{
#ifdef BSS_PLATFORM_WIN32
  WIN32_FILE_ATTRIBUTE_DATA fad;

  if(GetFileAttributesExW(path, GetFileExInfoStandard, &fad)==FALSE || (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0)
    return (unsigned long long)-1;

  return (static_cast<unsigned long long>(fad.nFileSizeHigh) << (sizeof(fad.nFileSizeLow)*8)) + fad.nFileSizeLow;
#else // BSS_PLATFORM_POSIX
  return bssFileSize(cStr(path).c_str());
#endif
}

BSS_COMPILER_DLLEXPORT
extern std::unique_ptr<char[], bss_util::bssdll_delete<char[]>> BSS_FASTCALL bss_util::FileDialog(bool open, unsigned long flags,
const char* file, const char* filter, const char* initdir, const char* defext)
{
#ifdef BSS_PLATFORM_WIN32 //Windows function
  cStrW wfilter;
  size_t c;
  const char* i;
  for(i=filter; *((const short*)i) != 0; ++i);
  c=i-filter+1; //+1 to include null terminator
  wfilter.reserve(MultiByteToWideChar(CP_UTF8, 0, filter, (int)c, 0, 0));
  MultiByteToWideChar(CP_UTF8, 0, filter, (int)c, wfilter.UnsafeString(), (int)wfilter.capacity());
  return FileDialog(open, flags, cStrW(file), wfilter, cStrW(initdir), cStrW(defext), 0);
#else // BSS_PLATFORM_POSIX
  /*Gtk::FileChooserDialog dialog("Choose File", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  //Add filters, so that only certain file types can be selected:

  Glib::RefPtr<Gtk::FileFilter> filter_text = Gtk::FileFilter::create();
  filter_text->set_name("Text files");
  filter_text->add_mime_type("text/plain");
  dialog.add_filter(filter_text);

  Glib::RefPtr<Gtk::FileFilter> filter_cpp = Gtk::FileFilter::create();
  filter_cpp->set_name("C/C++ files");
  filter_cpp->add_mime_type("text/x-c");
  filter_cpp->add_mime_type("text/x-c++");
  filter_cpp->add_mime_type("text/x-c-header");
  dialog.add_filter(filter_cpp);

  Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
  filter_any->set_name("Any files");
  filter_any->add_pattern("*");
  dialog.add_filter(filter_any);

  //Handle the response:
  if(dialog.run() == Gtk::RESPONSE_OK)
  *out= dialog.get_filename();*/
  return std::unique_ptr<char[], bss_util::bssdll_delete<char[]>>((char*)0);
#endif
}
#ifdef BSS_PLATFORM_WIN32 //Windows function
BSS_COMPILER_DLLEXPORT
extern std::unique_ptr<char[], bss_util::bssdll_delete<char[]>> BSS_FASTCALL bss_util::FileDialog(bool open, unsigned long flags,
const wchar_t* file, const wchar_t* filter, const wchar_t* initdir, const wchar_t* defext, HWND__* owner)
{
  wchar_t buf[MAX_PATH];
  GetCurrentDirectoryW(MAX_PATH, buf);
  cStrW curdirsave(buf);
  ZeroMemory(buf, MAX_PATH);

  if(file!=0) WCSNCPY(buf, MAX_PATH, file, bssmin(wcslen(file), MAX_PATH-1));

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
  size_t len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, 0, 0, 0, 0);
  std::unique_ptr<char[], bssdll_delete<char[]>> r(new char[len]);
  WideCharToMultiByte(CP_UTF8, 0, buf, -1, r.get(), (int)len, 0, 0);
  return r;
}
#endif

extern long BSS_FASTCALL bss_util::GetTimeZoneMinutes()
{
#ifdef BSS_PLATFORM_WIN32
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

#else // BSS_PLATFORM_POSIX
  time_t rawtime;
  TIME64(&rawtime);
  tm stm;
  if(GMTIMEFUNC(&rawtime, &stm)!=0)
    return 0;
  return stm.tm_gmtoff;
#endif
}

#ifdef BSS_PLATFORM_WIN32
extern void BSS_FASTCALL bss_util::AlertBox(const char* text, const char* caption, int type)
{
  AlertBoxW(cStrW(text), cStrW(caption), type);
}
extern void BSS_FASTCALL bss_util::AlertBoxW(const wchar_t* text, const wchar_t* caption, int type)
{
  MessageBoxW(0, text, caption, type);
}
#endif

void bss_util::bssdll_delete_delfunc(void* p) { ::operator delete(p); } // operator delete[] simply calls operator delete when its void*

#ifdef BSS_PLATFORM_WIN32
extern int BSS_FASTCALL bss_util::CreateDir(const char* path, bool recursive)
{
  return CreateDirW(BSSPOSIX_WCHAR(path), recursive);
}
extern int BSS_FASTCALL bss_util::CreateDirW(const wchar_t* path, bool recursive)
{
  if(!recursive)
    return -(CreateDirectoryW(path, 0) == 0);
  cStrW hold(path);
  hold.ReplaceChar('/', '\\');
  if(hold[hold.size()-1] != '\\') hold += '\\'; //Make sure we've got a trailing \\ on the path.
  const wchar_t* pos = wcschr(hold, '\\');
  cStrW temppath;
  while(pos)
  {
    temppath = hold.substr(0, pos-(const wchar_t*)hold);
    if(!FolderExistsW(temppath) && CreateDirectoryW(temppath, 0)==0)
      return -1;
    pos = wcschr(++pos, '\\');
  }

  return 0;
}
#else
extern int BSS_FASTCALL bss_util::CreateDir(const char* path, bool recursive)
{
  if(!recursive)
    return mkdir(path, 0700);
  struct stat st ={ 0 };

  cStr hold(path);
  hold.ReplaceChar('\\', '/');
  if(hold[hold.size()-1] != '/') hold += '/'; //Make sure we've got a trailing \\ on the path.
  const char* pos = strchr(hold, '/');
  cStr temppath;
  while(pos)
  {
    temppath = hold.substr(0, pos-(const char*)hold);
    if(stat(temppath, &st) < 0 && mkdir(temppath, 0700) < 0)
      return -1;
    pos = strchr(++pos, '/');
  }
  return 0;
}
#endif

#ifdef BSS_PLATFORM_WIN32
extern int BSS_FASTCALL bss_util::DelDir(const char* cdir, bool recursive)
{
  return bss_util::DelDirW(BSSPOSIX_WCHAR(cdir), recursive);
}
extern int BSS_FASTCALL bss_util::DelDirW(const wchar_t* cdir, bool recursive)
{
  if(!recursive)
    return -(RemoveDirectoryW(cdir) == 0);

  WIN32_FIND_DATAW ffd;
  HANDLE hdir = INVALID_HANDLE_VALUE;

  cStrW dir(cdir);
  dir.ReplaceChar('/', '\\');
  if(dir[dir.length()-1] != L'\\') dir += L'\\';
  hdir = FindFirstFileW(dir+L"*", &ffd);

  while(hdir > 0 && hdir != INVALID_HANDLE_VALUE)
  {
    if(WCSICMP(ffd.cFileName, L".")!=0 && WCSICMP(ffd.cFileName, L"..")!=0)
    {
      if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if(DelDirW(dir+ffd.cFileName, true)!=0)
          return -3;
      } else if(_wremove(dir+ffd.cFileName)!=0)
        return -2;
    }
    if(FindNextFileW(hdir, &ffd) <= 0) break; //either we're done or it failed
  }

  FindClose(hdir);
  if(RemoveDirectoryW(dir.substr(0, dir.size()-1).c_str())==0)
    return -1;
  return 0;
}
#else // POSIX
#include <ftw.h>

int _deldir_func(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  assert(typeflag!=FTW_D); //shouldn't happen because we set FTW_DEPTH
  if(typeflag==FTW_DP)
    return rmdir(fpath);
  if(typeflag==FTW_F)
    return unlink(fpath);
  return remove(fpath);
}

extern int BSS_FASTCALL bss_util::DelDir(const char* cdir, bool recursive)
{
  if(!recursive)
    return rmdir(cdir);
  return nftw(cdir,&_deldir_func, 20, FTW_DEPTH);
}
#endif

#ifdef BSS_PLATFORM_WIN32 //Windows function
BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL bss_util::_listdir(const wchar_t* cdir, void(BSS_FASTCALL *fn)(const wchar_t*, std::vector<cStr>*), std::vector<cStr>* files, char flags)
{
  WIN32_FIND_DATAW ffd;
  HANDLE hdir = INVALID_HANDLE_VALUE;

  cStrW dir(cdir);
  if(dir[dir.length()-1] == '/') dir.UnsafeString()[dir.length()-1] = '\\';
  if(dir[dir.length()-1] != '\\') dir += '\\';
  hdir = FindFirstFileW(dir+"*", &ffd);

  while(hdir > 0 && hdir !=INVALID_HANDLE_VALUE)
  {
    if(WCSICMP(ffd.cFileName, L".")!=0 && WCSICMP(ffd.cFileName, L"..")!=0)
    {
      if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        cStrW fldir(dir+ffd.cFileName);
        if(flags&2) (*fn)(fldir.c_str(), files);
        if(flags&1) _listdir(fldir.c_str(), fn, files, flags);
      } else
        (*fn)((dir+ffd.cFileName).c_str(),files);
    }
    if(FindNextFileW(hdir, &ffd) <= 0) break; //either we're done or it failed
  }

  FindClose(hdir);
  return 0;
}
#else //Linux function
BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL bss_util::ListDir(const char* path, std::vector<cStr>& files, char flags) // Setting flags to 1 will do a recursive search. Setting flags to 2 will return directory+file names. Setting flags to 3 will both be recursive and return directory names.
{
  DIR* srcdir = opendir(path);

  if(!srcdir)
    return -1;

  struct stat st;
  struct dirent* dent;
  cStr dir(path);
  if(dir[dir.length()-1] == '\\') dir.UnsafeString()[dir.length()-1] = '/';
  if(dir[dir.length()-1] != '/') dir += '/';

  while((dent = readdir(srcdir))!=0)
  {
    if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
      continue;
    if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) == 0)
    {
      cStr sdir(dir+dent->d_name);
      if(S_ISDIR(st.st_mode)) {
        if(flags&2) files.push_back(sdir.c_str());
        if(flags&1) ListDir(sdir.c_str(), files, flags);
      } else
        files.push_back((dir+dent->d_name).c_str());
    }
  }
  closedir(srcdir);
  return 0;
}
#endif


#ifdef BSS_PLATFORM_WIN32
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
  if(ERROR_SUCCESS == RegCreateKeyExW(hOpenKey, szKey, dwReserved, 0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0, &hTempKey, &dwDisposition))
  {
    if(fn(hTempKey, dwReserved) == ERROR_SUCCESS)
      bRetVal = TRUE;
  }

  // close opened key
  if(hTempKey)
    ::RegCloseKey(hTempKey);

  return bRetVal;
}

int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, const char* szData)
{
  return SetRegistryValueW(hOpenKey, cStrW(szKey), cStrW(szValue), szData);
}

int BSS_FASTCALL bss_util::SetRegistryValueW(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, const char* szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue || !szData) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input

  return r_setregvalue(hOpenKey, szKey, [&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_SZ, (LPBYTE)szData, ((DWORD)strlen(szData)+1)*sizeof(wchar_t));
  });
}

int BSS_FASTCALL bss_util::SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, int32_t szData)
{
  return SetRegistryValueW(hOpenKey, cStrW(szKey), cStrW(szValue), szData);
}
int BSS_FASTCALL bss_util::SetRegistryValueW(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, int32_t szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input

  return r_setregvalue(hOpenKey, szKey, [&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_DWORD, (LPBYTE)&szData, sizeof(int32_t));
  });
}
int BSS_FASTCALL bss_util::SetRegistryValue64(HKEY__*	hOpenKey, const char* szKey, const char* szValue, int64_t szData)
{
  return SetRegistryValue64W(hOpenKey, cStrW(szKey), cStrW(szValue), szData);
}
int BSS_FASTCALL bss_util::SetRegistryValue64W(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, int64_t szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input

  return r_setregvalue(hOpenKey, szKey, [&](HKEY& hTempKey, DWORD& dwReserved) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, dwReserved, REG_QWORD, (LPBYTE)&szData, sizeof(int64_t));
  });
}

int64_t BSS_FASTCALL bss_util::GetRegistryValueW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, unsigned char* data, unsigned long sz)
{
  HKEY__* hKey;
  LRESULT e = RegOpenKeyExW(hKeyRoot, szKey, 0, KEY_READ, &hKey);
  if(!hKey) return -2;
  LSTATUS r = RegQueryValueExW(hKey, szValue, 0, 0, data, &sz);
  RegCloseKey(hKey);
  if(r == ERROR_SUCCESS)
    return sz;
  return (r == ERROR_MORE_DATA) ? sz : -1;
}
int64_t BSS_FASTCALL bss_util::GetRegistryValue(HKEY__* hKeyRoot, const char* szKey, const char* szValue, unsigned char* data, unsigned long sz)
{
  return GetRegistryValueW(hKeyRoot, cStrW(szKey), cStrW(szValue), data, sz);
}
int BSS_FASTCALL bss_util::GetRegistryValueDWORDW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, DWORD* data)
{
  HKEY__* hKey;
  RegOpenKeyExW(hKeyRoot, szKey, 0, KEY_READ, &hKey);
  if(!hKey) return -2;
  DWORD type = 0;
  DWORD sz = sizeof(DWORD);
  LSTATUS r = RegQueryValueExW(hKey, szValue, 0, &type, (LPBYTE)data, &sz);
  RegCloseKey(hKey);
  if(type != REG_DWORD)
    return -3;
  return (r == ERROR_SUCCESS) ? 0 : -1;
}
int BSS_FASTCALL bss_util::GetRegistryValueDWORD(HKEY__* hKeyRoot, const char* szKey, const char* szValue, DWORD* data)
{
  return GetRegistryValueDWORDW(hKeyRoot, cStrW(szKey), cStrW(szValue), data);
}
int BSS_FASTCALL bss_util::GetRegistryValueQWORDW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, unsigned long long* data)
{
  HKEY__* hKey;
  RegOpenKeyExW(hKeyRoot, szKey, 0, KEY_READ, &hKey);
  if(!hKey) return -2;
  DWORD type = 0;
  DWORD sz = sizeof(unsigned long long);
  LSTATUS r = RegQueryValueExW(hKey, szValue, 0, &type, (LPBYTE)data, &sz);
  RegCloseKey(hKey);
  if(type != REG_QWORD)
    return -3;
  return (r == ERROR_SUCCESS) ? 0 : -1;
}
int BSS_FASTCALL bss_util::GetRegistryValueQWORD(HKEY__* hKeyRoot, const char* szKey, const char* szValue, unsigned long long* data)
{
  return GetRegistryValueQWORDW(hKeyRoot, cStrW(szKey), cStrW(szValue), data);
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

  if(lResult == ERROR_SUCCESS)
    return TRUE;

  lResult = RegOpenKeyExW(hKeyRoot, lpEnd, 0, KEY_READ, &hKey);

  if(lResult != ERROR_SUCCESS)
  {
    if(lResult == ERROR_FILE_NOT_FOUND) {
      OutputDebugStringW(L"Key not found.\n");
      return TRUE;
    } else {
      OutputDebugStringW(L"Error opening key.\n");
      return FALSE;
    }
  }
  // Check for an ending slash and add one if it is missing.
  if(lpEnd[lpEnd.length()-1] != L'\\')
    lpEnd += L'\\';

  dwSize = MAX_PATH;
  lResult = RegEnumKeyExW(hKey, 0, szName, &dwSize, nullptr, nullptr, nullptr, &ftWrite);

  if(lResult == ERROR_SUCCESS)
  {
    do {
      //lpEnd = szName;
      if(!r_delregnode(hKeyRoot, lpEnd+szName))
        break;

      dwSize = MAX_PATH;
      lResult = RegEnumKeyExW(hKey, 0, szName, &dwSize, nullptr, nullptr, nullptr, &ftWrite);
    } while(lResult == ERROR_SUCCESS);
  }

  RegCloseKey(hKey);

  lResult = RegDeleteKeyW(hKeyRoot, lpEnd);

  return lResult == ERROR_SUCCESS?TRUE:FALSE;
}

int BSS_FASTCALL bss_util::DelRegistryNode(HKEY__* hKeyRoot, const char* lpSubKey)
{
  return r_delregnode(hKeyRoot, cStrW(lpSubKey).c_str());
}

int BSS_FASTCALL bss_util::DelRegistryNodeW(HKEY__* hKeyRoot, const wchar_t* lpSubKey)
{
  return r_delregnode(hKeyRoot, lpSubKey);
}

#endif

#ifdef BSS_DEBUG
#include "delegate.h"

struct testclass
{
  void BSS_FASTCALL f() {}
};
void BSS_FASTCALL ffff() {}

bss_util::delegate<void> rrrrr = bss_util::delegate<void>::From<testclass, &testclass::f>(0);
#endif