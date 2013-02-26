// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_DEPRECATED_H__
#define __BSS_DEPRECATED_H__

#include "bss_compiler.h"
#include <time.h> //Done so we can use time_t, and as a way to get VC++ to include crtdefs.h without ruining compatability with other compilers

#ifdef BSS_COMPILER_MSC
#define TIME64(ptime) _time64(ptime)
#define GMTIMEFUNC(time, tm) _gmtime64_s(tm, time)
#define VSNPRINTF(dst,size,format,list) _vsnprintf_s(dst,size,size,format,list)
#define VSNWPRINTF(dst,size,format,list) _vsnwprintf_s(dst,size,size,format,list)
#define VSCPRINTF(format,args) _vscprintf(format,args)
#define VSCWPRINTF(format,args) _vscwprintf(format,args)
#define FOPEN(f, path, mode) fopen_s(&f, path, mode)
#define WFOPEN(f, path, mode) _wfopen_s(&f, path, mode)
#define MEMCPY(dst,size,src,count) memcpy_s(dst,size,src,count)
#define STRNCPY(dst,size,src,count) strncpy_s(dst,size,src,count)
#define WCSNCPY(dst,size,src,count) wcsncpy_s(dst,size,src,count)
#define STRCPY(dst,size,src) strcpy_s(dst,size,src)
#define WCSCPY(dst,size,src) wcscpy_s(dst,size,src)
#define STRCPYx0(dst,src) strcpy_s(dst,src)
#define WCSCPYx0(dst,src) wcscpy_s(dst,src)
#define STRICMP(a,b) _stricmp(a,b)
#define WCSICMP(a,b) _wcsicmp(a,b)
#define STRNICMP(a,b,n) _strnicmp(a,b,n)
#define WCSNICMP(a,b,n) _wcsnicmp(a,b,n)
#define STRTOK(str,delim,context) strtok_s(str,delim,context)
#define WCSTOK(str,delim,context) wcstok_s(str,delim,context)
#define SSCANF sscanf_s
#define ITOAx0(v,buf,r) _itoa_s(v,buf,r)
#define ITOA(v,buf,bufsize,r) _itoa_s(v,buf,bufsize,r)
#define ATOLL(s) _atoi64(s)
#define STRTOULL(s,e,r) _strtoui64(s,e,r)
#define SLEEP(s) _sleep(s)
#else
#ifdef BSS_PLATFORM_MINGW
#define TIME64(ptime) _time64(ptime)
#define GMTIMEFUNC(time, tm) _gmtime64_s(tm, time)
#define VSNWPRINTF(dst,size,format,list) vsnwprintf(dst,size,format,list)
#else
#define TIME64(ptime) time(ptime) // Linux does not have explicit 64-bit time functions, time_t will simply be 64-bit if the OS is 64-bit.
#define GMTIMEFUNC(time, tm) (gmtime_r(time, tm)==0) // This makes it so it will return 1 on error and 0 otherwise, matching _gmtime64_s
#define VSNWPRINTF(dst,size,format,list) vswprintf(dst,size,format,list) //vswprintf is exactly what vsnwprintf should be, for some reason.
#endif

#define VSNPRINTF(dst,size,format,list) vsnprintf(dst,size,format,list)
#define VSCPRINTF(format,args) vsnprintf(0,0,format,args)
//#define VSCWPRINTF(format,args) _vscwprintf(format,args) //no way to implement this
#define FOPEN(f, path, mode) f = fopen(path, mode)
#define WFOPEN(f, path, mode) f = fopen(path, mode)
#define MEMCPY(dst,size,src,count) memcpy(dst,src,((size)<(count))?(size):(count))
#define STRNCPY(dst,size,src,count) strncpy(dst,src,((size)<(count))?(size):(count))
#define WCSNCPY(dst,size,src,count) wcsncpy(dst,src,((size)<(count))?(size):(count))
#define STRCPY(dst,size,src) strncpy(dst,src,size-1)
#define WCSCPY(dst,size,src) wcsncpy(dst,src,size-1)
#define STRCPYx0(dst,src) strcpyx0(dst,src)
//#define WCSCPYx0(dst,src) wcscpyx0(dst,src)
#define STRICMP(a,b) strcasecmp(a,b)
#define WCSICMP(a,b) wcscasecmp(a,b)
#define STRNICMP(a,b,n) strncasecmp(a,b,n)
#define WCSNICMP(a,b,n) wcsncasecmp(a,b,n)
#define STRTOK(str,delim,context) strtok_r(str,delim,context)
#define WCSTOK(str,delim,context) wcstok(str,delim,context) // For some reason, in linux, wcstok *IS* the threadsafe version
#define SSCANF sscanf
#define ITOAx0(v,buf,r) _itoa_r(v,buf,r)
#define ITOA(v,buf,bufsize,r) itoa_r(v,buf,bufsize,r)
#define ATOLL(s) atoll(s)
#define STRTOULL(s,e,r) strtoull(s,e,r)
#define SLEEP(s) sleep(s)
#endif

#endif
