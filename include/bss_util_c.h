// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_UTIL_C_H__
#define __BSS_UTIL_C_H__

#include "bss_defines.h"
#include <wchar.h>

#ifdef  __cplusplus
extern "C" {
#endif
  
struct VersionType
{
  union {
    struct {
      unsigned char Major;
      unsigned char Minor;
      unsigned short Revision;
    };
    unsigned __int32 Version;
  };
};

struct tm;

BSS_COMPILER_DLLEXPORT extern unsigned long BSS_FASTCALL strhex(const char* text);
#ifdef BSS_PLATFORM_WIN32
BSS_COMPILER_DLLEXPORT extern unsigned long BSS_FASTCALL wcshex(const wchar_t* text);
BSS_COMPILER_DLLEXPORT extern size_t BSS_FASTCALL UTF8toUTF16(const char*BSS_RESTRICT input,wchar_t*BSS_RESTRICT output, size_t buflen);
BSS_COMPILER_DLLEXPORT extern size_t BSS_FASTCALL UTF16toUTF8(const wchar_t*BSS_RESTRICT input, char*BSS_RESTRICT output, size_t buflen);
#endif
BSS_COMPILER_DLLEXPORT extern int itoa_r(int value, char* buffer, int size, unsigned int radix); // For various stupid reasons we must reimplement multiple threadsafe versions of various functions because MinGW doesn't have them.
//BSS_COMPILER_DLLEXPORT extern struct tm* gmtime64_r(const long long*BSS_RESTRICT clock, struct tm*BSS_RESTRICT result);
#ifdef BSS_PLATFORM_MINGW
BSS_COMPILER_DLLEXPORT extern char* strtok_r(char* s, const char* delim, char** lasts);
BSS_COMPILER_DLLEXPORT extern wchar_t* wcstok_r(wchar_t* s, const wchar_t* delim, wchar_t** lasts);
#endif

#ifdef  __cplusplus
}

// templatized implementation of itoa_r so it can work with stack allocated arrays
template<int SZ>
BSS_FORCEINLINE int BSS_FASTCALL _itoa_r(int value, char (&buffer)[SZ], unsigned int radix) { return itoa_r(value,buffer,SZ,radix); }
#endif

#endif
