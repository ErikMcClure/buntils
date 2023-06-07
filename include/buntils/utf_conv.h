// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __UTF_CONV_H__BUN__
#define __UTF_CONV_H__BUN__

#include "compiler.h"
#include <wchar.h>
#include <uchar.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif

  extern BUN_DLLEXPORT size_t UTF8toUTF16(const char*BUN_RESTRICT input, ptrdiff_t srclen, wchar_t*BUN_RESTRICT output, size_t buflen);
  extern BUN_DLLEXPORT size_t UTF16toUTF8(const wchar_t*BUN_RESTRICT input, ptrdiff_t srclen, char*BUN_RESTRICT output, size_t buflen);
  extern BUN_DLLEXPORT size_t UTF8toUTF32(const char*BUN_RESTRICT input, ptrdiff_t srclen, char32_t*BUN_RESTRICT output, size_t buflen);
  extern BUN_DLLEXPORT size_t UTF32toUTF8(const char32_t*BUN_RESTRICT input, ptrdiff_t srclen, char*BUN_RESTRICT output, size_t buflen);
  extern BUN_DLLEXPORT size_t UTF32toUTF16(const char32_t*BUN_RESTRICT input, ptrdiff_t srclen, wchar_t*BUN_RESTRICT output, size_t buflen);
  extern BUN_DLLEXPORT size_t UTF16toUTF32(const wchar_t*BUN_RESTRICT input, ptrdiff_t srclen, char32_t*BUN_RESTRICT output, size_t buflen);

#ifdef  __cplusplus
}
#endif

#endif