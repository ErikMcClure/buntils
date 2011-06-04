// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_UTIL_C_H__
#define __BSS_UTIL_C_H__

#include "bss_defines.h"
#include "bss_call.h"
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
      unsigned short Patch;
    };
    unsigned __int32 Version;
  };
};

__declspec(dllexport) extern unsigned long BSS_FASTCALL strhex(const char* text);
__declspec(dllexport) extern unsigned long BSS_FASTCALL wcshex(const wchar_t* text);
extern int BSS_FASTCALL UTF8Decode2BytesUnicode(const char* input,wchar_t* output);
extern int BSS_FASTCALL UTF8Encode2BytesUnicode(const wchar_t* input, unsigned char* output);
//__declspec(dllexport) extern bool BSS_FASTCALL Util_Assert(const wchar_t* expression, const wchar_t* file, int line); //custom assert function so we can make it cross-platform and so the stupid gigantic windows headers don't have to be included

#ifdef  __cplusplus
}
#endif

#endif