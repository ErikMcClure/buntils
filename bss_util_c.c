// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_util_c.h"
#include "bss_win32_includes.h"
#include <stdlib.h>
#include <string.h>

BSS_COMPILER_DLLEXPORT
extern unsigned long BSS_FASTCALL strhex(const char* text)
{
  return strtoul(text,0,16);
}

BSS_COMPILER_DLLEXPORT
extern unsigned long BSS_FASTCALL wcshex(const wchar_t* text)
{
  return wcstoul(text,0,16);
}

//Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
//
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//documentation files (the "Software"), to deal in the Software without restriction, including without
//limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
//conditions:
//
//The above copyright notice and this permission notice shall be included in all copies or substantial
//portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
//CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//DEALINGS IN THE SOFTWARE.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

static const unsigned char utf8d[] = {
  // The first part of the table maps bytes to character classes that
  // to reduce the size of the transition table and create bitmasks.
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

  // The second part is a transition table that maps a combination
  // of a state of the automaton and a character class to a state.
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12, 
};

unsigned __int32 decode(unsigned __int32* state, unsigned __int32* codep, unsigned __int32 byte)
{
  unsigned __int32 type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state + type];
  return *state;
}

size_t countutf16(const unsigned char* s)
{
  size_t count;
  unsigned __int32 codepoint;
  unsigned __int32 state = 0;

  for (count = 1; *s; ++s) //One for null terminator
    if(!decode(&state, &codepoint, *s))
      count += (1+(codepoint > 0xffff)); //If the codepoint is above 0xffff, two utf16 characters will be printed.

  return (state == UTF8_ACCEPT)?count:(size_t)-1;
}

BSS_COMPILER_DLLEXPORT
extern size_t BSS_FASTCALL UTF8toUTF16(const char* input,wchar_t* output, size_t buflen)
{
  return (size_t)MultiByteToWideChar(CP_UTF8, 0, input, -1, output, !output?0:buflen);
  /*const unsigned char* s = src;
  wchar_t* d = dst;
  unsigned __int32 codepoint;
  unsigned __int32 state = 0;

  if(!dst) return countutf16(src);

  while (*s)
  {
    if(decode(&state, &codepoint, *s++))
      continue;

    if(codepoint > 0xffff) {
      *d++ = (wchar_t)(0xD7C0 + (codepoint >> 10));
      *d++ = (wchar_t)(0xDC00 + (codepoint & 0x3FF));
    } else
      *d++ = (wchar_t)codepoint;
  }

  *d++ = 0; // We do this because the loop terminates on a NULL terminator so that won't have been copied over.

  if (state != UTF8_ACCEPT)
    return (size_t)-1;
  
  return (size_t)(d - dst);*/
}

BSS_COMPILER_DLLEXPORT
extern size_t BSS_FASTCALL UTF16toUTF8(const wchar_t* input, char* output, size_t buflen)
{
  return (size_t)WideCharToMultiByte(CP_UTF8, 0, input, -1, output, !output?0:buflen, NULL, NULL);
}