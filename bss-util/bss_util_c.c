// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_util_c.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#include <intrin.h>
#include <psapi.h>
#else
#include <iconv.h>
#include <unistd.h> // for sysconf
#include <cpuid.h>
#include <stdio.h>
#include <sys/resource.h>
#endif

BSS_COMPILER_DLLEXPORT 
extern union bssCPUInfo BSS_FASTCALL bssGetCPUInfo()
{
  union bssCPUInfo r={0};
  unsigned int info[4];
#ifdef BSS_COMPILER_MSC
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  r.cores=(unsigned short)sysinfo.dwNumberOfProcessors;
  __cpuid(info, 1);
#elif defined(BSS_COMPILER_GCC)
  r.cores=sysconf(_SC_NPROCESSORS_ONLN);
  __get_cpuid(1,info+0,info+1,info+2,info+3);
    //asm volatile ("cpuid" : "=a" (info[0]), [ebx] "=r" (info[1]), "=c" (info[2]), "=d" (info[3]) : "a" (1), "c" (0));
#endif
    
  if(info[3]&T_GETBIT(unsigned int, 23)) // MMX
    r.SSE=1;
  if(info[3]&T_GETBIT(unsigned int, 25)) {// SSE
    assert(r.SSE==1); // Ensure that our assumption that no processor skips any of these is correct, otherwise explode.
    r.SSE=2;
  } if(info[3]&T_GETBIT(unsigned int, 26)) {// SSE2
    assert(r.SSE==2);
    r.SSE=3;
  } if(info[2]&T_GETBIT(unsigned int, 0)) {// SSE3
    assert(r.SSE==3);
    r.SSE=4;
  } if(info[2]&T_GETBIT(unsigned int, 9)) {// SSSE3
    assert(r.SSE==4);
    r.SSE=5;
  } if(info[2]&T_GETBIT(unsigned int, 19)) {// SSE4.1
    assert(r.SSE==5);
    r.SSE=6;
  } if(info[2]&T_GETBIT(unsigned int, 20)) {// SSE4.2
    assert(r.SSE==6);
    r.SSE=7;
  } if(info[2]&T_GETBIT(unsigned int, 28)) {// AVX
    assert(r.SSE==7);
    r.SSE=8;
  }
  r.flags|=(((info[3]&T_GETBIT(unsigned int, 8))!=0)<<0); // cmpxchg8b support
  r.flags|=(((info[2]&T_GETBIT(unsigned int, 13))!=0)<<1); // cmpxchg16b support
  r.flags|=(((info[2]&T_GETBIT(unsigned int, 6))!=0)<<2); // SSE4a support
  r.flags|=(((info[3]&T_GETBIT(unsigned int, 15))!=0)<<3); // cmov support
  r.flags|=(((info[3]&T_GETBIT(unsigned int, 19))!=0)<<4); // CFLUSH support
  r.flags|=(((info[2]&T_GETBIT(unsigned int, 23))!=0)<<5); // POPCNT support
  return r;
}

BSS_COMPILER_DLLEXPORT 
extern const char* GetProgramPath()
{
#ifdef BSS_PLATFORM_WIN32
  static char buf[MAX_PATH*2];
  wchar_t wbuf[MAX_PATH];
  GetModuleFileNameExW(GetCurrentProcess(), 0, wbuf, MAX_PATH);
  wbuf[MAX_PATH-1]=0; //XP doesn't ensure this is null terminated
  UTF16toUTF8(wbuf, buf, MAX_PATH);
  return buf;
#else
  return 0;
#endif
}

BSS_COMPILER_DLLEXPORT
extern size_t GetWorkingSet()
{
#ifdef BSS_PLATFORM_WIN32
  PROCESS_MEMORY_COUNTERS counter;
  GetProcessMemoryInfo(GetCurrentProcess(), &counter, sizeof(PROCESS_MEMORY_COUNTERS));
  return counter.WorkingSetSize;
#else
  long rss = 0L;
  FILE* fp = fopen("/proc/self/statm", "r");
  if(!fp) return (size_t)0L;
  if(fscanf(fp, "%*s%ld", &rss) != 1)
    rss=0;
  fclose(fp);
  return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);
#endif
}

BSS_COMPILER_DLLEXPORT
extern size_t GetPeakWorkingSet()
{
#ifdef BSS_PLATFORM_WIN32
  PROCESS_MEMORY_COUNTERS info;
  GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
  return (size_t)info.PeakWorkingSetSize;
#else
  struct rusage rusage;
  getrusage(RUSAGE_SELF, &rusage);
#if defined(__APPLE__) && defined(__MACH__)
  return (size_t)rusage.ru_maxrss;
#else
  return (size_t)(rusage.ru_maxrss * 1024L);
#endif
#endif
}
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
extern size_t BSS_FASTCALL UTF8toUTF16(const char*BSS_RESTRICT input,wchar_t*BSS_RESTRICT output, size_t buflen)
{
#ifdef BSS_PLATFORM_WIN32
  return (size_t)MultiByteToWideChar(CP_UTF8, 0, input, -1, output, !output?0:buflen);
#else
  static iconv_t iconv_utf8to16=0;
  size_t len = strlen(input);
  char* out = (char*)output;
  if(!output) return (len*4) + 1;
  len+=1; // include null terminator
  if(!iconv_utf8to16) iconv_utf8to16=iconv_open("UTF-8", "UTF-16");
  char* in = (char*)input; // Linux is stupid
  return iconv(iconv_utf8to16, &in, &len, &out, &buflen);
#endif

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

/*
* Copyright 2001-2004 Unicode, Inc.
* 
* Disclaimer
* 
* This source code is provided as is by Unicode, Inc. No claims are
* made as to fitness for any particular purpose. No warranties of any
* kind are expressed or implied. The recipient agrees to determine
* applicability of information provided. If this file has been
* purchased on magnetic or optical media from Unicode, Inc., the
* sole remedy for any claim will be exchange of defective media
* within 90 days of receipt.
* 
* Limitations on Rights to Redistribute This Code
* 
* Unicode, Inc. hereby grants the right to freely use the information
* supplied in this file in the creation of products supporting the
* Unicode Standard, and to make copies of this file in any form
* for internal or external distribution as long as this notice
* remains attached.
*/

#define UNI_REPLACEMENT_CHAR (unsigned int)0x0000FFFD
#define UNI_MAX_BMP (unsigned int)0x0000FFFF
#define UNI_MAX_UTF16 (unsigned int)0x0010FFFF
#define UNI_MAX_UTF32 (unsigned int)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (unsigned int)0x0010FFFF
#define UNI_SUR_HIGH_START  (unsigned int)0xD800
#define UNI_SUR_HIGH_END    (unsigned int)0xDBFF
#define UNI_SUR_LOW_START   (unsigned int)0xDC00
#define UNI_SUR_LOW_END     (unsigned int)0xDFFF

static const char trailingBytesForUTF8[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

static const unsigned int offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

static char isLegalUTF8(const unsigned char *source, int length) {
  unsigned char a;
  const unsigned char *srcptr = source+length;
  switch (length) {
  default: return 0;
      /* Everything else falls through when "true"... */
  case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return 0;
  case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return 0;
  case 2: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return 0;

      switch (*source) {
          /* no fall-through in this inner switch */
          case 0xE0: if (a < 0xA0) return 0; break;
          case 0xED: if (a > 0x9F) return 0; break;
          case 0xF0: if (a < 0x90) return 0; break;
          case 0xF4: if (a > 0x8F) return 0; break;
          default:   if (a < 0x80) return 0;
      }

  case 1: if (*source >= 0x80 && *source < 0xC2) return 0;
  }
  if (*source > 0xF4) return 0;
  return 1;
}

BSS_COMPILER_DLLEXPORT
extern size_t BSS_FASTCALL UTF8toUTF32(const char*BSS_RESTRICT input, int*BSS_RESTRICT output, size_t buflen) 
{
  char result = 0;
  const unsigned char* source = (unsigned char*)input;
  const unsigned char* sourceEnd = source;
  unsigned int* target = (unsigned int*)output;
  unsigned int* targetEnd = target+buflen;
  size_t srclen=strlen(input)+1;
  if(!output) return srclen;
  sourceEnd+=srclen;

  while(source < sourceEnd) {
      unsigned int ch = 0;
      unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
      if (extraBytesToRead >= sourceEnd - source) {
          result = -2; break;
      }
      /* Do this check whether lenient or strict */
      if (!isLegalUTF8(source, extraBytesToRead+1)) {
          result = -1;
          break;
      }
      /*
        * The cases all fall through. See "Note A" below.
        */
      switch (extraBytesToRead) {
          case 5: ch += *source++; ch <<= 6;
          case 4: ch += *source++; ch <<= 6;
          case 3: ch += *source++; ch <<= 6;
          case 2: ch += *source++; ch <<= 6;
          case 1: ch += *source++; ch <<= 6;
          case 0: ch += *source++;
      }
      ch -= offsetsFromUTF8[extraBytesToRead];

      if (target >= targetEnd) {
          source -= (extraBytesToRead+1); /* Back up the source pointer! */
          result = -3; break;
      }
      if (ch <= UNI_MAX_LEGAL_UTF32) {
          /*
            * UTF-16 surrogate values are illegal in UTF-32, and anything
            * over Plane 17 (> 0x10FFFF) is illegal.
            */
          if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
              //if (flags == strictConversion) {
              //    source -= (extraBytesToRead+1); /* return to the illegal value itself */
              //    result = sourceIllegal;
              //    break;
              //} else {
                  *target++ = UNI_REPLACEMENT_CHAR;
              //}
          } else {
              *target++ = ch;
          }
      } else { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
          result = -1;
          *target++ = UNI_REPLACEMENT_CHAR;
      }
  }
  return (size_t)(((int*)target)-output);
}

BSS_COMPILER_DLLEXPORT
extern size_t BSS_FASTCALL UTF16toUTF8(const wchar_t*BSS_RESTRICT input, char*BSS_RESTRICT output, size_t buflen)
{
#ifdef BSS_PLATFORM_WIN32
  return (size_t)WideCharToMultiByte(CP_UTF8, 0, input, -1, output, !output?0:buflen, NULL, NULL);
#else
  static iconv_t iconv_utf16to8=0;
  size_t len = wcslen(input)*2;
  char* in = (char*)input;
  if(!output) return (len*2) + 1;
  len+=2; // include null terminator (which is 2 bytes wide here)
  if(!iconv_utf16to8) iconv_utf16to8=iconv_open("UTF-16", "UTF-8");
  return iconv(iconv_utf16to8, &in, &len, &output, &buflen);
#endif
}

BSS_COMPILER_DLLEXPORT
extern int itoa_r(int val, char* buf, int size, unsigned int radix)
{
  int t;
  char tmp;
  char* cur=buf;
  char* p;
  if(!buf || radix<2 || size<=1) return 22; //22 is EINVAL error code from CRT. We use this because what we return doesn't matter so long as its not 0.

  if(val<0)
  {
    *cur++='-'; // This is safe because we've already checked to ensure size > 0, so we have at least one character to work with.
    val=-val;
  }

  p=cur;
  --size; //Leave room for null terminator

  while((cur-buf)<size)
  {
    t=(val%radix);
    *cur++ = (t>9)?(t+'7'):(t+'0'); //This allows radix 16 to work
    val/=radix;

    if(val<=0) break;
  }
  
  *cur--=0;

  while(p<cur)
  {
    tmp=*p;
    *p=*cur;
    *cur=tmp;
    --cur;
    ++p;
  };

  return 0;
}

#ifdef BSS_PLATFORM_MINGW

/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

BSS_COMPILER_DLLEXPORT
extern char* strtok_r(char* s, const char* delim, char** lasts)
{
	register char *spanp;
	register int c, sc;
	char *tok;

	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

cont: /* Skip (span) leading delimiters (s += strspn(s, delim), sort of). */
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	for (;;) { /* Scan token (scan for delimiters: s += strcspn(s, delim), sort of). Note that delim must have one NUL; we stop if we see that, too. */
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
}

// type-swapped version of above function
BSS_COMPILER_DLLEXPORT
extern wchar_t* wcstok_r(wchar_t* s, const wchar_t* delim, wchar_t** lasts)
{
	register wchar_t *spanp;
	register int c, sc;
	wchar_t *tok;

	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

cont: /* Skip (span) leading delimiters (s += strspn(s, delim), sort of). */
	c = *s++;
	for (spanp = (wchar_t *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	for (;;) { /* Scan token (scan for delimiters: s += strcspn(s, delim), sort of). Note that delim must have one NUL; we stop if we see that, too. */
		c = *s++;
		spanp = (wchar_t *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
}

#endif
