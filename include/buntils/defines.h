// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_DEFINES_H__
#define __BUN_DEFINES_H__

#include "compiler.h"

// Version numbers
#define BUN_VERSION_MAJOR 0
#define BUN_VERSION_MINOR 6
#define BUN_VERSION_REVISION 0

//sometimes the std versions of these are a bit overboard, so this redefines the MS version, except it will no longer cause conflicts everywhere
#define bun_max(a,b)            (((a) > (b)) ? (a) : (b))
#define bun_min(a,b)            (((a) < (b)) ? (a) : (b))
#define bun_clamp(var,min,max) (((var)>(max))?(max):(((var)<(min))?(min):(var)))

// Use the following defines to force TODO items as warnings in the compile. VC++ currently does not support "messages" despite having
// them as a category in the error list.
// Usage: #pragma message(TODO "Clean up code here")
#define __TXT(x) x
#define TXT(x) __TXT(#x)
#define TODO __FILE__ "(" TXT(__LINE__) ") : warning (TODO): "
#define NOTICE __FILE__ "(" TXT(__LINE__) ") : warning (NOTICE): "
#define WIDEN2(x) L ## x 
#define WIDEN(x) WIDEN2(x) 
#ifndef __WFILE__
#define __WFILE__ WIDEN(__FILE__) 
#endif
#define CONCAT(...) __VA_ARGS__

#ifdef __cpp_exceptions
  #define THROW_OR_ABORT(msg) throw std::runtime_error(msg)
#else
  #define THROW_OR_ABORT(msg) { assert(false); abort(); }
#endif

    // These yield float/int ranges using rand(). However, rand() is a horrible RNG, so unless you have a good reason, use bun_rand() instead.
#ifndef RANDFLOATGEN
#define RANDFLOATGEN(min,max) (((max) - (min)) * (rand()/(RAND_MAX+1.0)) + (min))
#endif
#ifndef RANDINTGEN
#define RANDINTGEN(min,max) ((min)+(int)((rand()/(RAND_MAX + 1.0))*((max)-(min))))
#endif
#ifndef RANDBOOLGEN
#define RANDBOOLGEN() (rand()>(RAND_MAX>>1))
#endif

#define SAFESHIFT(v,s) (((s)>0)?((v)<<(s)):((v)>>(-(s)))) //positive number shifts left, negative number shifts right, prevents undefined behavior.
#define T_GETBIT(type, bit) ((type)(((type)1)<<(((type)bit)%(sizeof(type)<<3)))) // Gets a bitmask using its 0-based index.
//#define T_GETBITRANGE(type, low, high) ((type)((((2<<((high)-(low)))-1)<<(low))|(((2<<((high)-(low)))-1)>>(-(low)))|(((2<<((high)-(low)))-1)<<((low)%(sizeof(type)<<3)))))
// Gets a bitmask for a range of bits. This range can be negative to start from the left end instead of the right.
#define T_GETBITRANGE(type, low, high) ((type)(SAFESHIFT(((2<<((high)-(low)))-1),low)|(((2<<((high)-(low)))-1)<<((low)%(sizeof(type)<<3))))) 
#define MAKEVERSIONINT(major,minor,revision) ((major) + ((minor)<<8) + ((revision)<<16)) // Packs version information into a 32-bit integer
// This calculates the most significant bit in an 8-bit integer (CHAR) at compile time. It's really just a basterdized version of
// i |= i >> 1; i |= i >> 2; i |= i >> 4; i -= i >> 1; so that it happens during compile time.
#define T_CHARGETMSB(i) ((((i | (i>>1)) | ((i | (i>>1))>>2)) | (((i | (i>>1)) | ((i | (i>>1))>>2))>>4)) - ((((i | (i>>1)) | ((i | (i>>1))>>2)) | (((i | (i>>1)) | ((i | (i>>1))>>2))>>4))>>1)) // I CAN'T BELIEVE THIS WORKS（ ﾟДﾟ）
// Round x up to next highest multiple of (t+1), which must be a multiple of 2. For example, to get the next multiple of 8: T_NEXTMULTIPLE(x,7)
#define T_NEXTMULTIPLE(x,t) (((x)+(t))&(~(t)))
#define T_SETBIT(w,b,f,T) (((w) & (~(b))) | ((-(T)f) & (b)))
#define T_FBNEXT(x) (x + 1 + (x>>1) + (x>>3) - (x>>7))
#define ISPOW2(x) (x && !( (x-1) & x ))

#if defined(BUN_PLATFORM_POSIX) || defined(BUN_PLATFORM_MINGW)
#define BUNPOSIX_WCHAR(s) s
#define BUN__L(x)      x
#elif defined(BUN_PLATFORM_WIN32)
#define BUNPOSIX_WCHAR(s) bun::StrW(s).c_str()
#define BUN__L(x)      L ## x
#endif

#define GEN_FUNCTION_CHECK(FUNC)   \
template<typename T, typename R, typename ... Args> \
class has_##FUNC \
{ \
  template <typename U, R(U::*)(Args...)> struct Check; \
  template <typename U> static char func(Check<U, &U::FUNC> *); \
  template <typename U> static int func(...); \
public: \
  typedef bun_has_function_##FUNC type; \
  enum { value = sizeof(func<T>(0)) == sizeof(char) };\
};

// This is used to implement a check to see if a given function exists in a class
#define DEFINE_MEMBER_CHECKER(Member)   \
  template<class T> class has_##Member \
{ \
struct big { char a[2]; }; \
  template<class C> static big  probe(decltype(&C::Member)); \
  template<class C> static char probe(...); \
public: \
  static const bool value = sizeof(probe<T>(nullptr)) > 1; \
}

#ifdef BUN_PLATFORM_WIN32
#define TIME64(ptime) _time64(ptime)
#define GMTIMEFUNC(time, tm) _gmtime64_s(tm, time)
#define VSNPRINTF(dst,size,format,list) _vsnprintf_s(dst,size,size,format,list)
#define VSNWPRINTF(dst,size,format,list) _vsnwprintf_s(dst,size,size,format,list)
#define VSCPRINTF(format,args) _vscprintf(format,args)
#define VSCWPRINTF(format,args) _vscwprintf(format,args)
#define FOPEN(f, path, mode) fopen_s(&f, path, mode)
#define WFOPEN(f, path, mode) _wfopen_s(&f, bun::StrW(path).c_str(), bun::StrW(mode).c_str())
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
#define STRLWR(a) _strlwr(a)
#define WCSLWR(a) _wcslwr(a)
#define STRUPR(a) _strupr(a)
#define SSCANF sscanf_s
#define ITOAx0(v,buf,r) _itoa_s(v,buf,r)
#define ITOA(v,buf,bufsize,r) _itoa_s(v,buf,bufsize,r)
#define ATOLL(s) _atoi64(s)
#define STRTOULL(s,e,r) _strtoui64(s,e,r)
#define ALLOCA(x) _alloca(x) // _malloca requires using _freea so we can't use it
#define LOADDYNLIB(s) LoadLibraryA(s)
#define GETDYNFUNC(p,s) GetProcAddress((HMODULE)p, TXT(s))
#define FREEDYNLIB(p) FreeLibrary((HMODULE)p)
#define ALIGNEDALLOC(size, align) _aligned_malloc(size, align)
#define ALIGNEDFREE(p) _aligned_free(p)
#else
#ifdef BUN_PLATFORM_MINGW
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
#define STRLWR(a) strlwr(a)
#define WCSLWR(a) wcslwr(a)
#define STRUPR(a) strupr(a)
#define SSCANF sscanf
#define ITOAx0(v,buf,r) _itoa_r(v,buf,r)
#define ITOA(v,buf,bufsize,r) itoa_r(v,buf,bufsize,r)
#define ATOLL(s) atoll(s)
#define STRTOULL(s,e,r) strtoull(s,e,r)
#define ALLOCA(x) alloca(x)
#define LOADDYNLIB(s) dlopen(s, RTLD_LAZY)
#define GETDYNFUNC(p,s) dlsym(p, TXT(s))
#define FREEDYNLIB(p) dlclose(p)
#define ALIGNEDALLOC(size, align) aligned_alloc(align, size)
#define ALIGNEDFREE(p) free(p)
#endif

#endif
