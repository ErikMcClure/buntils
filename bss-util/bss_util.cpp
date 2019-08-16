// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss-util/bss_util.h"
#include "bss-util/Str.h"
#include "bss-util/os.h"
#include "bss-util/ArraySort.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss-util/win32_includes.h"
#include <Commdlg.h>
#include <Shlwapi.h>
#include <Shlobj.h>
#else
#include <sys/types.h>  // stat().
#include <sys/stat.h>   // stat().
#include <dirent.h>
#include <unistd.h> // rmdir()
#include "fontconfig/fontconfig.h"
//#include <gtkmm.h> // file dialog
#endif

void SetWorkDirToCur()
{
#ifdef BSS_PLATFORM_WIN32
  bss::StrW commands(MAX_PATH);
  GetModuleFileNameW(0, commands.UnsafeString(), MAX_PATH);
  commands.UnsafeString()[wcsrchr(commands, '\\') - commands + 1] = '\0';
  SetCurrentDirectoryW(commands);
#endif
}

void ForceWin64Crash()
{
#ifdef BSS_PLATFORM_WIN32
  typedef BOOL(WINAPI *tGetPolicy)(LPDWORD lpFlags);
  typedef BOOL(WINAPI *tSetPolicy)(DWORD dwFlags);
  const DWORD EXCEPTION_SWALLOWING = 0x1;
  DWORD dwFlags;

  HMODULE kernel32 = LoadLibraryA("kernel32.dll");
  assert(kernel32 != 0);
  tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32, "GetProcessUserModeExceptionPolicy");
  tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32, "SetProcessUserModeExceptionPolicy");
  if(pGetPolicy && pSetPolicy && pGetPolicy(&dwFlags))
    pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING); // Turn off the filter 
#endif
  // Obviously in linux this function does nothing becuase linux isn't a BROKEN PIECE OF SHIT
}

unsigned long long bssFileSize(const char* path)
{
#ifdef BSS_PLATFORM_WIN32
  return bssFileSizeW(bss::StrW(path).c_str());
#else // BSS_PLATFORM_POSIX
  struct stat path_stat;
  if(::stat(path, &path_stat) != 0 || !S_ISREG(path_stat.st_mode))
    return (unsigned long long) - 1;

  return (unsigned long long)path_stat.st_size;
#endif
}

unsigned long long bssFileSizeW(const wchar_t* path)
{
#ifdef BSS_PLATFORM_WIN32
  WIN32_FILE_ATTRIBUTE_DATA fad;

  if(GetFileAttributesExW(path, GetFileExInfoStandard, &fad) == FALSE || (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
    return (unsigned long long) - 1;

  return (static_cast<unsigned long long>(fad.nFileSizeHigh) << (sizeof(fad.nFileSizeLow) * 8)) + fad.nFileSizeLow;
#else // BSS_PLATFORM_POSIX
  return bssFileSize(bss::Str(path).c_str());
#endif
}

std::unique_ptr<char[], bss::bssDLLDelete<char[]>> bss::FileDialog(bool open, unsigned long flags,
  const char* file, const char* filter, const char* initdir, const char* defext)
{
#ifdef BSS_PLATFORM_WIN32 //Windows function
  StrW wfilter;
  size_t c;
  const char* i;
  for(i = filter; *((const short*)i) != 0; ++i);
  c = i - filter + 1; //+1 to include null terminator
  wfilter.reserve(MultiByteToWideChar(CP_UTF8, 0, filter, (int)c, 0, 0));
  MultiByteToWideChar(CP_UTF8, 0, filter, (int)c, wfilter.UnsafeString(), (int)wfilter.capacity());
  return FileDialog(open, flags, StrW(file), wfilter, StrW(initdir), StrW(defext), 0);
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
  return std::unique_ptr<char[], bss::bssDLLDelete<char[]>>((char*)0);
#endif
}
#ifdef BSS_PLATFORM_WIN32 //Windows function

std::unique_ptr<char[], bss::bssDLLDelete<char[]>> bss::FileDialog(bool open, unsigned long flags,
  const wchar_t* file, const wchar_t* filter, const wchar_t* initdir, const wchar_t* defext, HWND__* owner)
{
  wchar_t buf[MAX_PATH];
  GetCurrentDirectoryW(MAX_PATH, buf);
  StrW curdirsave(buf);
  bssFill(buf);

  if(file != 0) WCSNCPY(buf, MAX_PATH, file, std::min<size_t>(wcslen(file), MAX_PATH - 1));

  OPENFILENAMEW ofn;
  bssFill(ofn);

  ofn.lStructSize = sizeof(OPENFILENAMEW);
  ofn.hwndOwner = owner;
  ofn.lpstrFilter = filter;
  ofn.lpstrFile = buf;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = (open ? OFN_EXPLORER | OFN_FILEMUSTEXIST : OFN_EXPLORER) | flags;
  ofn.lpstrDefExt = defext;
  ofn.lpstrInitialDir = initdir;

  BOOL res = open ? GetOpenFileNameW(&ofn) : GetSaveFileNameW(&ofn);

  SetCurrentDirectoryW(curdirsave); //There is actually a flag that's supposed to do this for us but it doesn't work on XP for file open, so we have to do it manually just to be sure
  if(!res) buf[0] = '\0';
  size_t len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, 0, 0, 0, 0);
  std::unique_ptr<char[], bssDLLDelete<char[]>> r(new char[len]);
  WideCharToMultiByte(CP_UTF8, 0, buf, -1, r.get(), (int)len, 0, 0);
  return r;
}
#endif

long GetTimeZoneMinutes()
{
#ifdef BSS_PLATFORM_WIN32
  TIME_ZONE_INFORMATION dtime;
  DWORD r = GetTimeZoneInformation(&dtime);
  switch(r)
  {
  case 0:
  case 1: //None or unknown daylight savings time
    return -(dtime.Bias + dtime.StandardBias); //This should be negated because the equation is UTC = local time + bias, so that means UTC - bias = local time
  case 2: //Using daylight savings time
    return -(dtime.Bias + dtime.DaylightBias);
  }
  return 0; //error

#else // BSS_PLATFORM_POSIX
  time_t rawtime;
  TIME64(&rawtime);
  tm stm;
  if(GMTIMEFUNC(&rawtime, &stm) != 0)
    return 0;
  return stm.tm_gmtoff;
#endif
}

#ifdef BSS_PLATFORM_WIN32
void bss::AlertBox(const char* text, const char* caption, int type)
{
  AlertBoxW(StrW(text), StrW(caption), type);
}
void bss::AlertBoxW(const wchar_t* text, const wchar_t* caption, int type)
{
  MessageBoxW(0, text, caption, type);
}
#endif

void bss::bssDLLDeleteFunc(void* p) { ::operator delete(p); } // operator delete[] simply calls operator delete when its void*

#ifdef BSS_PLATFORM_WIN32
template<class _Fn>
inline int r_setregvalue(HKEY__*	hOpenKey, const wchar_t* szKey, _Fn fn)
{
  BOOL 	bRetVal = FALSE;
  DWORD	dwDisposition;
  HKEY  	hTempKey = (HKEY)0;

  // Open key of interest
  // Assume all access is okay and that all keys will be stored to file
  // Utilize the default security attributes
  //if( ERROR_SUCCESS == ::RegCreateKeyEx(hOpenKey, szKey, 0,(LPTSTR)0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0,&hTempKey, &dwDisposition))
  if(ERROR_SUCCESS == RegCreateKeyExW(hOpenKey, szKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0, &hTempKey, &dwDisposition))
  {
    if(fn(hTempKey) == ERROR_SUCCESS)
      bRetVal = TRUE;
  }

  // close opened key
  if(hTempKey)
    ::RegCloseKey(hTempKey);

  return bRetVal;
}

int bss::SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, const char* szData)
{
  return SetRegistryValueW(hOpenKey, StrW(szKey), StrW(szValue), szData);
}

int bss::SetRegistryValueW(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, const char* szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue || !szData) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input

  return r_setregvalue(hOpenKey, szKey, [&](HKEY& hTempKey) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, 0, REG_SZ, (LPBYTE)szData, ((DWORD)strlen(szData) + 1) * sizeof(wchar_t));
  });
}

int bss::SetRegistryValue(HKEY__*	hOpenKey, const char* szKey, const char* szValue, int32_t szData)
{
  return SetRegistryValueW(hOpenKey, StrW(szKey), StrW(szValue), szData);
}
int bss::SetRegistryValueW(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, int32_t szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input

  return r_setregvalue(hOpenKey, szKey, [&](HKEY& hTempKey) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, 0, REG_DWORD, (LPBYTE)&szData, sizeof(int32_t));
  });
}
int bss::SetRegistryValue64(HKEY__*	hOpenKey, const char* szKey, const char* szValue, int64_t szData)
{
  return SetRegistryValue64W(hOpenKey, StrW(szKey), StrW(szValue), szData);
}
int bss::SetRegistryValue64W(HKEY__*	hOpenKey, const wchar_t* szKey, const wchar_t* szValue, int64_t szData)
{
  if(!hOpenKey || !szKey || !szKey[0] || !szValue) { ::SetLastError((DWORD)E_INVALIDARG); return FALSE; } // validate input

  return r_setregvalue(hOpenKey, szKey, [&](HKEY& hTempKey) -> int {
    return RegSetValueExW(hTempKey, (LPWSTR)szValue, 0, REG_QWORD, (LPBYTE)&szData, sizeof(int64_t));
  });
}

int64_t bss::GetRegistryValueW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, unsigned char* data, unsigned long sz)
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
int64_t bss::GetRegistryValue(HKEY__* hKeyRoot, const char* szKey, const char* szValue, unsigned char* data, unsigned long sz)
{
  return GetRegistryValueW(hKeyRoot, StrW(szKey), StrW(szValue), data, sz);
}
int bss::GetRegistryValueDWORDW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, DWORD* data)
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
int bss::GetRegistryValueDWORD(HKEY__* hKeyRoot, const char* szKey, const char* szValue, DWORD* data)
{
  return GetRegistryValueDWORDW(hKeyRoot, StrW(szKey), StrW(szValue), data);
}
int bss::GetRegistryValueQWORDW(HKEY__* hKeyRoot, const wchar_t* szKey, const wchar_t* szValue, unsigned long long* data)
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
int bss::GetRegistryValueQWORD(HKEY__* hKeyRoot, const char* szKey, const char* szValue, unsigned long long* data)
{
  return GetRegistryValueQWORDW(hKeyRoot, StrW(szKey), StrW(szValue), data);
}

int r_delregnode(HKEY__* hKeyRoot, const wchar_t* lpSubKey)
{
  LONG lResult;
  DWORD dwSize;
  wchar_t szName[MAX_PATH];
  HKEY hKey;
  FILETIME ftWrite;

  bss::StrW lpEnd = lpSubKey;
  lResult = RegDeleteKeyW(hKeyRoot, lpEnd);

  if(lResult == ERROR_SUCCESS)
    return TRUE;

  lResult = RegOpenKeyExW(hKeyRoot, lpEnd, 0, KEY_READ, &hKey);

  if(lResult != ERROR_SUCCESS)
  {
    if(lResult == ERROR_FILE_NOT_FOUND)
    {
      OutputDebugStringW(L"Key not found.\n");
      return TRUE;
    }
    else
    {
      OutputDebugStringW(L"Error opening key.\n");
      return FALSE;
    }
  }
  // Check for an ending slash and add one if it is missing.
  if(lpEnd[lpEnd.length() - 1] != L'\\')
    lpEnd += L'\\';

  dwSize = MAX_PATH;
  lResult = RegEnumKeyExW(hKey, 0, szName, &dwSize, nullptr, nullptr, nullptr, &ftWrite);

  if(lResult == ERROR_SUCCESS)
  {
    do
    {
  //lpEnd = szName;
      if(!r_delregnode(hKeyRoot, lpEnd + szName))
        break;

      dwSize = MAX_PATH;
      lResult = RegEnumKeyExW(hKey, 0, szName, &dwSize, nullptr, nullptr, nullptr, &ftWrite);
    } while(lResult == ERROR_SUCCESS);
  }

  RegCloseKey(hKey);

  lResult = RegDeleteKeyW(hKeyRoot, lpEnd);

  return lResult == ERROR_SUCCESS ? TRUE : FALSE;
}

int bss::DelRegistryNode(HKEY__* hKeyRoot, const char* lpSubKey)
{
  return r_delregnode(hKeyRoot, StrW(lpSubKey).c_str());
}

int bss::DelRegistryNodeW(HKEY__* hKeyRoot, const wchar_t* lpSubKey)
{
  return r_delregnode(hKeyRoot, lpSubKey);
}

namespace bss {
  struct BSSFONT
  {
    BSSFONT(const ENUMLOGFONTEX* fontex, DWORD elftype) : weight((short)fontex->elfLogFont.lfWeight), italic(fontex->elfLogFont.lfItalic != 0), type((char)elftype)
    {
      memcpy_s(elfFullName, sizeof(wchar_t)*LF_FULLFACESIZE, fontex->elfFullName, sizeof(wchar_t)*LF_FULLFACESIZE);
    }

    static char Comp(const BSSFONT& l, const BSSFONT& r)
    {
      char ret = SGNCOMPARE(l.italic, r.italic);
      return !ret ? SGNCOMPARE(l.weight, r.weight) : ret;
    }

    typedef bss::ArraySort<BSSFONT, &BSSFONT::Comp> BSSFONTARRAY;

    WCHAR elfFullName[LF_FULLFACESIZE];
    short weight;
    char type;
    bool italic;
  };
}

int CALLBACK EnumFont_GetFontPath(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam)
{
  const ENUMLOGFONTEX* info = reinterpret_cast<const ENUMLOGFONTEX*>(lpelfe);
  bss::BSSFONT::BSSFONTARRAY* fonts = reinterpret_cast<bss::BSSFONT::BSSFONTARRAY*>(lParam);
  bss::BSSFONT font(info, FontType);
  if(fonts->Find(font) == (size_t)~0)
    fonts->Insert(font);
  return 1;
}

// This function tries to match the closest possible weight to the desired weight, using a hueristic based on Firefox's method: https://developer.mozilla.org/en-US/docs/Web/CSS/font-weight
bool weightbiascheck(short weight, short target, short cur)
{
  if(weight > 500) // bold bias
  {
    if(target < 500 && cur >= 500) // If the target is less than 500 and cur isn't, always choose cur
      return true;
    if(cur < 500) // If cur is less than 500, always use cur if it's greater than target
      return cur > target;
  }
  else // thin bias
  {
    if(target >= 500 && cur < 500) // If the target is greater than 500 and cur isn't, always choose cur
      return true;
    if(cur >= 500) // If cur is greater than or equals 500, always use cur if it's less than target
      return cur < target;
  }
  return abs(cur - weight) < abs(target - weight); // Otherwise, just minimize the difference to the desired weight
}

std::unique_ptr<char[], bss::bssDLLDelete<char[]>> bss::GetFontPath(const char* family, int weight, bool italic)
{
  HDC hdc = GetDC(0);
  LOGFONT font = { 0 };
  font.lfCharSet = DEFAULT_CHARSET;
  UTF8toUTF16(family, -1, font.lfFaceName, LF_FACESIZE - 1);
  bss::BSSFONT::BSSFONTARRAY fonts;
  EnumFontFamiliesExW(hdc, &font, &EnumFont_GetFontPath, reinterpret_cast<LPARAM>(&fonts), 0);

  size_t target = 0;
  std::unique_ptr<char[], bss::bssDLLDelete<char[]>> p(nullptr);
  if(fonts.Length() > 0) // Make sure we have at least one result
  {
    for(size_t i = 1; i < fonts.Length(); ++i)
    {
      if(fonts[i].italic == italic) // First check to match italic. This overrides everything else
      {
        if((fonts[target].italic != italic) || // If our current target doesn't match italic, immediately substitute this one.
          (fonts[i].weight == weight) || // Check for an exact weight match
          (weight == 500 && fonts[i].weight == 400 && fonts[target].weight != 500) || // If the requested weight is exactly 500 and we haven't matched it, use 400 instead
          (weight == 400 && fonts[i].weight == 500 && fonts[target].weight != 400) || // If the requested weight is exactly 400 and we haven't matched it, use 500 instead
          weightbiascheck(weight, fonts[target].weight, fonts[i].weight))
          target = i;
      }
    }
    wchar_t name[LF_FULLFACESIZE + 12] = { 0 };
    memcpy_s(name, sizeof(wchar_t)*LF_FULLFACESIZE, fonts[target].elfFullName, sizeof(wchar_t)*LF_FULLFACESIZE);
    if(fonts[target].type & DEVICE_FONTTYPE)
      wcscat_s(name, L" (OpenType)");
    else if(fonts[target].type & TRUETYPE_FONTTYPE)
      wcscat_s(name, L" (TrueType)");

    unsigned char path[MAX_PATH] = { 0 };
    if(GetRegistryValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", name, path, MAX_PATH) >= 0)
    {
      wchar_t buf[MAX_PATH] = { 0 };
      HRESULT res = SHGetFolderPathW(0, CSIDL_FONTS, 0, SHGFP_TYPE_CURRENT, buf);
      if(res != E_FAIL)
      {
        wcscat_s(buf, L"\\");
        wcscat_s(buf, (wchar_t*)path);
        p.reset(new char[MAX_PATH * 2]);
        UTF16toUTF8(buf, MAX_PATH, p.get(), MAX_PATH * 2);
      }
    }
  }
  return std::move(p);
}

#else

std::unique_ptr<char[], bss::bssDLLDelete<char[]>> bss::GetFontPath(const char* family, int weight, bool italic)
{
  FcConfig* config = FcInitLoadConfigAndFonts();
  std::unique_ptr<char[], bss::bssDLLDelete<char[]>> p(nullptr);

  FcPattern* pat = FcNameParse((const FcChar8*)family);
  FcConfigSubstitute(config, pat, FcMatchPattern);
  FcDefaultSubstitute(pat);

  // find the font
  FcResult result;
  FcPattern* font = FcFontMatch(config, pat, &result);
  if(font)
  {
    FcChar8* file = NULL;
    if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
    {
      size_t len = strlen((char*)file);
      p.reset(new char[len]);
      memcpy(p.get(), file, len);
    }
    FcPatternDestroy(font);
  }

  FcPatternDestroy(pat);
  return std::move(p);
}

#endif

#ifdef BSS_DEBUG
#include "bss-util/Delegate.h"
#include "bss-util/Hash.h"

bss::Hash<int, bss::Hash<int>, bss::ARRAY_SAFE> hashtest;
static bool testhash = !hashtest[2];
#endif