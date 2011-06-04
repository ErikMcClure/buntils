// Copyright �2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_DEPRECATED_H__
#define __BSS_DEPRECATED_H__

#if __STDC_WANT_SECURE_LIB__
typedef __int64 TIMEVALUSED;
#define FTIME(ptime) _time64(ptime)
#define GMTIMEFUNC(raw, assign) if(_gmtime64_s(assign, raw)) return false
#define VSPRINTF(dest,length,format,list) _vsnprintf_s(dest,length,length,format,list)
#define VSWPRINTF(dest,length,format,list) _vsnwprintf_s(dest,length,length,format,list)
#define FOPEN(f, path, mode) fopen_s(&f, path, mode)
#define WFOPEN(f, path, mode) _wfopen_s(&f, path, mode)
#define MEMCPY(dest,length,source,count) memcpy_s(dest,length,source,count)
#define STRNCPY(dst,size,src,count) strncpy_s(dst,size,src,count)
#define WCSNCPY(dst,size,src,count) wcsncpy_s(dst,size,src,count)
#define STRCPY(dst,size,src) strcpy_s(dst,size,src)
#define WCSCPY(dst,size,src) wcscpy_s(dst,size,src)
#define STRICMP(a,b) _stricmp(a,b)
#define WCSICMP(a,b) _wcsicmp(a,b)
#else
typedef time_t TIMEVALUSED;
#define FTIME(ptime) time(ptime)
#define GMTIMEFUNC(raw, assign) assign = gmtime(raw); if(!assign) return false
#define VSPRINTF(dest,length,format,list) vsprintf(dest,format,list)
#define VSWPRINTF(dest,length,format,list) vswprintf(dest,length,length,format,list)
#define FOPEN(f, path, mode) f = fopen(path, mode)
#define WFOPEN(f, path, mode) f = _wfopen(path, mode)
#define MEMCPY(dest,length,source,count) memcpy(dest,source,count)
#define STRNCPY(dst,size,src,count) strncpy(dst,src,count)
#define WCSNCPY(dst,size,src,count) wcsncpy(dst,src,count)
#define STRCPY(dst,size,src) strcpy(dst,src)
#define WCSCPY(dst,size,src) wcscpy(dst,src)
#define STRICMP(a,b) stricmp(a,b)
#define WCSICMP(a,b) wcsicmp(a,b)
#endif

#endif