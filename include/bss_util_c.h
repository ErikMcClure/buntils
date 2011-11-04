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

BSS_COMPILER_DLLEXPORT extern unsigned long BSS_FASTCALL strhex(const char* text);
BSS_COMPILER_DLLEXPORT extern unsigned long BSS_FASTCALL wcshex(const wchar_t* text);
BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL UTF8Decode2BytesUnicode(const char* input,wchar_t* output);
BSS_COMPILER_DLLEXPORT extern int BSS_FASTCALL UTF8Encode2BytesUnicode(const wchar_t* input, unsigned char* output);

#ifdef  __cplusplus
}
#endif

#endif