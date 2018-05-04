// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __UTF_CONV_H__BSS__
#define __UTF_CONV_H__BSS__

#include "compiler.h"
#include <wchar.h>
#include <uchar.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif

  extern BSS_DLLEXPORT size_t UTF8toUTF16(const char*BSS_RESTRICT input, ptrdiff_t srclen, wchar_t*BSS_RESTRICT output, size_t buflen);
  extern BSS_DLLEXPORT size_t UTF16toUTF8(const wchar_t*BSS_RESTRICT input, ptrdiff_t srclen, char*BSS_RESTRICT output, size_t buflen);
  extern BSS_DLLEXPORT size_t UTF8toUTF32(const char*BSS_RESTRICT input, ptrdiff_t srclen, char32_t*BSS_RESTRICT output, size_t buflen);
  extern BSS_DLLEXPORT size_t UTF32toUTF8(const char32_t*BSS_RESTRICT input, ptrdiff_t srclen, char*BSS_RESTRICT output, size_t buflen);
  extern BSS_DLLEXPORT size_t UTF32toUTF16(const char32_t*BSS_RESTRICT input, ptrdiff_t srclen, wchar_t*BSS_RESTRICT output, size_t buflen);
  extern BSS_DLLEXPORT size_t UTF16toUTF32(const wchar_t*BSS_RESTRICT input, ptrdiff_t srclen, char32_t*BSS_RESTRICT output, size_t buflen);

#ifdef  __cplusplus
}
#endif

#endif