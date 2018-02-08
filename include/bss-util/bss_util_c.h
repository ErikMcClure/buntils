// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_UTIL_C_H__
#define __BSS_UTIL_C_H__

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

  struct bssCPUInfo {
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

  extern BSS_DLLEXPORT const bssVersionInfo bssVersion;
  struct tm;

  extern BSS_DLLEXPORT struct bssCPUInfo bssGetCPUInfo();
  extern BSS_DLLEXPORT int itoa_r(int value, char* buffer, int size, unsigned int radix); // For various stupid reasons we must reimplement multiple threadsafe versions of various functions because MinGW doesn't have them.
  extern BSS_DLLEXPORT const char* GetProgramPath();
  extern BSS_DLLEXPORT size_t GetWorkingSet();
  extern BSS_DLLEXPORT size_t GetPeakWorkingSet();
  extern BSS_DLLEXPORT void SetWorkDirToCur(); //Sets the working directory to the actual goddamn location of the EXE instead of the freaking start menu, or possibly the desktop. The possibilities are endless! Fuck you, windows.
  extern BSS_DLLEXPORT void ForceWin64Crash(); // I can't believe this function exists (forces 64-bit windows to not silently ignore fatal errors)
  extern BSS_DLLEXPORT unsigned long long bssFileSize(const char* path);
  extern BSS_DLLEXPORT unsigned long long bssFileSizeW(const wchar_t* path);
  extern BSS_DLLEXPORT long GetTimeZoneMinutes(); //Returns the current time zone difference from UTC in minutes
  //extern BSS_DLLEXPORT struct tm* gmtime64_r(const long long*BSS_RESTRICT clock, struct tm*BSS_RESTRICT result);
#ifdef BSS_PLATFORM_MINGW
  extern BSS_DLLEXPORT char* strtok_r(char* s, const char* delim, char** lasts);
  extern BSS_DLLEXPORT wchar_t* wcstok_r(wchar_t* s, const wchar_t* delim, wchar_t** lasts);
#endif

#ifdef  __cplusplus
}

// templatized implementation of itoa_r so it can work with stack allocated arrays
template<int SZ>
BSS_FORCEINLINE int _itoa_r(int value, char(&buffer)[SZ], unsigned int radix) { return itoa_r(value, buffer, SZ, radix); }
#endif

#endif
