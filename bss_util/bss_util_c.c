// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_util_c.h"
#include <stdlib.h>
#include <string.h>
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#else

#endif

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
extern size_t BSS_FASTCALL UTF16toUTF8(const wchar_t*BSS_RESTRICT input, char*BSS_RESTRICT output, size_t buflen)
{
  return (size_t)WideCharToMultiByte(CP_UTF8, 0, input, -1, output, !output?0:buflen, NULL, NULL);
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