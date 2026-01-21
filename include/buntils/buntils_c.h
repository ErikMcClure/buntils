// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_UTIL_C_H__
#define __BUN_UTIL_C_H__

#include "compiler.h"
#include <wchar.h>

#ifdef  __cplusplus
extern "C" {
#endif
  enum CPU_FLAGS {
    CPU_CMPXCHG8b = 1,
    CPU_CMPXCHG16b = 2,
    CPU_AMD_SSE4a = 4,
    CPU_CMOV = 8,
    CPU_CFLUSH = 16,
    CPU_POPCNT = 32,
  };

  struct bunCPUInfo {
    unsigned int cores; // Number of logical cores (not physical cores).
    enum {
      SSE_MMX = 1,
      SSE_SSE1,
      SSE_SSE2,
      SSE_SSE3,
      SSE_SSSE3,
      SSE_SSE4_1,
      SSE_SSE4_2,
      SSE_AVX,
      SSE_AVX2,
    } sse;
    unsigned int flags; // CPU_FLAGS
  };

  extern BUN_DLLEXPORT const bun_VersionInfo bun_Version;
  struct tm;

  extern BUN_DLLEXPORT struct bunCPUInfo bunGetCPUInfo();
  extern BUN_DLLEXPORT int itoa_r(int value, char* buffer, int size, unsigned int radix); // For various stupid reasons we must reimplement multiple threadsafe versions of various functions because MinGW doesn't have them.
  extern BUN_DLLEXPORT const char* GetProgramPath();
  extern BUN_DLLEXPORT size_t GetWorkingSet();
  extern BUN_DLLEXPORT size_t GetPeakWorkingSet();
  extern BUN_DLLEXPORT void SetWorkDirToCur(); //Sets the working directory to the actual goddamn location of the EXE instead of the freaking start menu, or possibly the desktop. The possibilities are endless! Fuck you, windows.
  extern BUN_DLLEXPORT void ForceWin64Crash(); // I can't believe this function exists (forces 64-bit windows to not silently ignore fatal errors)
  extern BUN_DLLEXPORT unsigned long long bun_FileSize(const char* path);
  extern BUN_DLLEXPORT unsigned long long bun_FileSizeW(const wchar_t* path);
  extern BUN_DLLEXPORT long GetTimeZoneMinutes(); //Returns the current time zone difference from UTC in minutes
  //extern BUN_DLLEXPORT struct tm* gmtime64_r(const long long*BUN_RESTRICT clock, struct tm*BUN_RESTRICT result);
#ifdef BUN_PLATFORM_MINGW
  extern BUN_DLLEXPORT char* strtok_r(char* s, const char* delim, char** lasts);
  extern BUN_DLLEXPORT wchar_t* wcstok_r(wchar_t* s, const wchar_t* delim, wchar_t** lasts);
#endif

#ifdef  __cplusplus
}

// templatized implementation of itoa_r so it can work with stack allocated arrays
template<int SZ>
BUN_FORCEINLINE int _itoa_r(int value, char(&buffer)[SZ], unsigned int radix) { return itoa_r(value, buffer, SZ, radix); }
#endif

#endif
