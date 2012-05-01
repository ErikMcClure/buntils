// Copyright ©2011 Black Sphere Studios
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
/*
const unsigned short MASKBITS=0x3F;
const unsigned short MASKBYTE=0x80;
const unsigned short MASK2BYTES=0xC0;
const unsigned short MASK3BYTES=0xE0;
//const unsigned short MASK4BYTES=0xF0;
//const unsigned short MASK5BYTES=0xF8;
//const unsigned short MASK6BYTES=0xFC;

// Decodes UTF8 byte stream into wchar_t string. Returns number of wide characters output
int BSS_FASTCALL UTF8Decode2BytesUnicode(const char* input,wchar_t* output)
{
  int count=-1;
  wchar_t ch=0;
  const char* pos;

  if(!output) //count mode
  {
    for(pos=input; *pos;)
    {
      if(((*pos) & MASK3BYTES) == MASK3BYTES) pos+=3;
      else if(((*pos) & MASK2BYTES) == MASK2BYTES) pos+=2;
      else if((*pos) < MASKBYTE) ++pos;
      ++count;
    }
    return ++count;
  }

  for(pos=input; *pos;)
  {

    if(((*pos) & MASK3BYTES) == MASK3BYTES) // 1110xxxx 10xxxxxx 10xxxxxx
    {
      ch = (((*pos) & 0x0F) << 12) | (
          ((*(pos+1)) & MASKBITS) << 6)
        | ((*(pos+2)) & MASKBITS);
      pos += 3;
    } else if(((*pos) & MASK2BYTES) == MASK2BYTES) {// 110xxxxx 10xxxxxx
      ch = (((*pos) & 0x1F) << 6) | ((*(pos+1)) & MASKBITS);
      pos += 2;
    } else if((*pos) < MASKBYTE) {// 0xxxxxxx
      ch = (*pos);
      ++pos;
    }

    output[++count]=ch;
  }
  return ++count;
}

// Encodes wchar_t string into UTF8 byte stream. Returns number of bytes written
int BSS_FASTCALL UTF8Encode2BytesUnicode(const wchar_t* input, unsigned char* output)
{
  int count=-1;
  const wchar_t* pos;
  if(!output) //count mode
  {
    for(pos=input; *pos; ++pos)
    {
      if((*pos) < 0x80) ++count;
      else if((*pos) < 0x800) count+=2;
      else if((*pos) < 0x10000) count+=3;
    }
    return ++count;
  }

  for(pos=input; *pos; ++pos)
  {
    if((*pos) < 0x80) { // 0xxxxxxx
        output[++count]=(unsigned char)(*pos);
    } else if((*pos) < 0x800) {// 110xxxxx 10xxxxxx
      output[++count]=(unsigned char)(MASK2BYTES | (*pos) >> 6);
      output[++count]=(unsigned char)(MASKBYTE | (*pos) & MASKBITS);
    } else if((*pos) < 0x10000) {// 1110xxxx 10xxxxxx 10xxxxxx
      output[++count]=(unsigned char)(MASK3BYTES | (*pos) >> 12);
      output[++count]=(unsigned char)(MASKBYTE | (*pos) >> 6 & MASKBITS);
      output[++count]=(unsigned char)(MASKBYTE | (*pos) & MASKBITS);
    }
  }
    return ++count;
}*/