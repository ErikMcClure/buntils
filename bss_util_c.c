// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_util_c.h"
#include <stdlib.h>
#include <string.h>

__declspec(dllexport)
extern unsigned long BSS_FASTCALL strhex(const char* text)
{
  return strtoul(text,0,16);
}

__declspec(dllexport)
extern unsigned long BSS_FASTCALL wcshex(const wchar_t* text)
{
  return wcstoul(text,0,16);
}

const unsigned short MASKBITS=0x3F;
const unsigned short MASKBYTE=0x80;
const unsigned short MASK2BYTES=0xC0;
const unsigned short MASK3BYTES=0xE0;
//const unsigned short MASK4BYTES=0xF0;
//const unsigned short MASK5BYTES=0xF8;
//const unsigned short MASK6BYTES=0xFC;

/* Decodes UTF8 byte stream into wchar_t string. Returns number of wide characters output */
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

/* Encodes wchar_t string into UTF8 byte stream. Returns number of bytes written */
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
}