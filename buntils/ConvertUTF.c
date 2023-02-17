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

/* ---------------------------------------------------------------------

Conversions between UTF32, UTF-16, and UTF-8. Source code file.
Author: Mark E. Davis, 1994.
Rev History: Rick McGowan, fixes & updates May 2001.
Sept 2001: fixed const & error conditions per
mods suggested by S. Parent & A. Lillich.
June 2002: Tim Dodd added detection and handling of incomplete
source sequences, enhanced error detection, added casts
to eliminate compiler warnings.
July 2003: slight mods to back out aggressive FFFE detection.
Jan 2004: updated switches in from-UTF8 conversions.
Oct 2004: updated to use UNI_MAX_LEGAL_UTF32 in UTF-32 conversions.

See the header file "ConvertUTF.h" for complete documentation.

------------------------------------------------------------------------ */


#include "buntils/utf_conv.h"
#include <stdint.h>

static const int halfShift = 10; /* used for shifting by 10 bits */

typedef unsigned int UTF32;
typedef unsigned short UTF16;
typedef unsigned char UTF8;

static const UTF32 halfBase = 0x0010000UL;
static const UTF32 halfMask = 0x3FFUL;

#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF
#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF
#define false      0
#define true        1

/* --------------------------------------------------------------------- */

/*
* Index into the table below with the first byte of a UTF-8 sequence to
* get the number of trailing bytes that are supposed to follow it.
* Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
* left as-is for anyone who may want to do such conversion, which was
* allowed in earlier algorithms.
*/
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

/*
* Magic values subtracted from a buffer value during UTF8 conversion.
* This table contains as many values as there might be trailing bytes
* in a UTF-8 sequence.
*/
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
* Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
* into the first byte, depending on how many bytes follow.  There are
* as many entries in this table as there are UTF-8 sequence types.
* (I.e., one byte sequence, two byte... etc.). Remember that sequencs
* for *legal* UTF-8 will be 4 or fewer bytes total.
*/
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
* Constants have been gathered. Loops & conditionals have been removed as
* much as possible for efficiency, in favor of drop-through switches.
* (See "Note A" at the bottom of the file for equivalent code.)
* If your compiler supports it, the "isLegalUTF8" call can be turned
* into an inline function.
*/

BUN_DLLEXPORT size_t UTF32toUTF16(const char32_t*BUN_RESTRICT input, ptrdiff_t srclen, wchar_t*BUN_RESTRICT output, size_t buflen)
{
  if(!srclen) return 0;
  const UTF32* source = (UTF32*)input;
  UTF16* target = (UTF16*)output;
  UTF16* targetEnd = target + buflen;
  if(srclen < 0) srclen = PTRDIFF_MAX;
  buflen = 1;
  while(*input && (input - (const char32_t*)source) < srclen)
  {
    buflen += 1 + (*input > UNI_MAX_BMP);
    ++input;
  }
  if(!output) return buflen;
  const UTF32* sourceEnd = ((input - (const char32_t*)source) < srclen) ? sourceEnd = ++input : source + srclen;

  while(source < sourceEnd)
  {
    UTF32 ch;
    if(target >= targetEnd)
    {
      break;
    }
    ch = *source++;
    if(ch <= UNI_MAX_BMP)
    { /* Target is a character <= 0xFFFF */
      /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
      if(ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)
      {
        //if(flags == strictConversion)
        //{
        //  --source; /* return to the illegal value itself */
        //  result = -1;
        //  break;
        //}
        //else
        //{
        *target++ = UNI_REPLACEMENT_CHAR;
      //}
      }
      else
      {
        *target++ = (UTF16)ch; /* normal case */
      }
    }
    else if(ch > UNI_MAX_LEGAL_UTF32)
    {
      //if(flags == strictConversion)
      //{
      //  result = -1;
      //}
      //else
      //{
      *target++ = UNI_REPLACEMENT_CHAR;
    //}
    }
    else
    {
      /* target is a character in range 0xFFFF - 0x10FFFF. */
      if(target + 1 >= targetEnd)
      {
        --source; /* Back up source pointer! */
        break;
      }
      ch -= halfBase;
      *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_HIGH_START);
      *target++ = (UTF16)((ch & halfMask) + UNI_SUR_LOW_START);
    }
  }
  return (size_t)(((wchar_t*)target) - output);
}

BUN_DLLEXPORT size_t UTF16toUTF32(const wchar_t*BUN_RESTRICT input, ptrdiff_t srclen, char32_t*BUN_RESTRICT output, size_t buflen)
{
  if(!srclen) return 0;
  const UTF16* source = (UTF16*)input;
  const UTF16* sourceEnd = source;
  UTF32* target = (UTF32*)output;
  UTF32* targetEnd = target + buflen;
  if(srclen < 0) srclen = wcslen(input) + 1;
  if(!output) return srclen;
  sourceEnd += srclen;
  UTF32 ch, ch2;
  while(source < sourceEnd)
  {
    const UTF16* oldSource = source; /*  In case we have to back up because of target overflow. */
    ch = *source++;
    /* If we have a surrogate pair, convert to UTF32 first. */
    if(ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END)
    {
      /* If the 16 bits following the high surrogate are in the source buffer... */
      if(source < sourceEnd)
      {
        ch2 = *source;
        /* If it's a low surrogate, convert to UTF32. */
        if(ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END)
        {
          ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
            + (ch2 - UNI_SUR_LOW_START) + halfBase;
          ++source;
        }
        //else if(flags == strictConversion)
        //{ /* it's an unpaired high surrogate */
        //  --source; /* return to the illegal value itself */
        //  result = -1;
        //  break;
        //}
      }
      else
      { /* We don't have the 16 bits following the high surrogate. */
        --source; /* return to the high surrogate */
        break;
      }
    }
    //else if(flags == strictConversion)
    //{
    //  /* UTF-16 surrogate values are illegal in UTF-32 */
    //  if(ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END)
    //  {
    //    --source; /* return to the illegal value itself */
    //    result = -1;
    //    break;
    //  }
    //}
    if(target >= targetEnd)
    {
      source = oldSource; /* Back up source pointer! */
      break;
    }
    *target++ = ch;
  }
  return (size_t)(((char32_t*)target) - output);
}

/* --------------------------------------------------------------------- */

BUN_DLLEXPORT size_t UTF32toUTF8(const char32_t*BUN_RESTRICT input, ptrdiff_t srclen, char*BUN_RESTRICT output, size_t buflen)
{
  if(!srclen) return 0;
  const UTF32* source = (UTF32*)input;
  const UTF32* sourceEnd = source + srclen;
  UTF8* target = (UTF8*)output;
  UTF8* targetEnd = target + buflen;
  if(srclen < 0) srclen = PTRDIFF_MAX;
  buflen = 1;
  while(*input && (input - (const char32_t*)source) < srclen)
  {
    if(*input < (UTF32)0x80)
      buflen += 1;
    else if(*input < (UTF32)0x800)
      buflen += 2;
    else if(*input < (UTF32)0x10000)
      buflen += 3;
    else if(*input <= UNI_MAX_LEGAL_UTF32)
      buflen += 4;
    else
      buflen += 3;
    ++input;
  }
  if(!output) return buflen;
  if((input - (const char32_t*)source) < srclen)
    sourceEnd = ++input; // increment to include null terminator if we hit one

  while(source < sourceEnd)
  {
    UTF32 ch;
    unsigned short bytesToWrite = 0;
    const UTF32 byteMask = 0xBF;
    const UTF32 byteMark = 0x80;
    ch = *source++;
    //if(flags == strictConversion)
    //{
    //  /* UTF-16 surrogate values are illegal in UTF-32 */
    //  if(ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)
    //  {
    //    --source; /* return to the illegal value itself */
    //    result = -1;
    //    break;
    //  }
    //}
    /*
    * Figure out how many bytes the result will require. Turn any
    * illegally large UTF32 things (> Plane 17) into replacement chars.
    */
    if(ch < (UTF32)0x80)
    {
      bytesToWrite = 1;
    }
    else if(ch < (UTF32)0x800)
    {
      bytesToWrite = 2;
    }
    else if(ch < (UTF32)0x10000)
    {
      bytesToWrite = 3;
    }
    else if(ch <= UNI_MAX_LEGAL_UTF32)
    {
      bytesToWrite = 4;
    }
    else
    {
      bytesToWrite = 3;
      ch = UNI_REPLACEMENT_CHAR;
    }

    target += bytesToWrite;
    if(target > targetEnd)
    {
      --source; /* Back up source pointer! */
      target -= bytesToWrite; break;
    }
    switch(bytesToWrite)
    { /* note: everything falls through. */
    case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
    case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
    case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
    case 1: *--target = (UTF8)(ch | firstByteMark[bytesToWrite]);
    }
    target += bytesToWrite;
  }
  return (size_t)(((char*)target) - output);
}

/* --------------------------------------------------------------------- */

/*
* Utility routine to tell whether a sequence of bytes is legal UTF-8.
* This must be called with the length pre-determined by the first byte.
* If not calling this from ConvertUTF8to*, then the length can be set by:
*  length = trailingBytesForUTF8[*source]+1;
* and the sequence is illegal right away if there aren't that many bytes
* available.
* If presented with a length > 4, this returns false.  The Unicode
* definition of UTF-8 goes up to 4-byte sequences.
*/

char isLegalUTF8(const UTF8 *source, int length)
{
  UTF8 a;
  const UTF8 *srcptr = source + length;
  switch(length)
  {
  default: return false;
    /* Everything else falls through when "true"... */
  case 4: if((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
  case 3: if((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
  case 2: if((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;

    switch(*source)
    {
      /* no fall-through in this inner switch */
    case 0xE0: if(a < 0xA0) return false; break;
    case 0xED: if(a > 0x9F) return false; break;
    case 0xF0: if(a < 0x90) return false; break;
    case 0xF4: if(a > 0x8F) return false; break;
    default:   if(a < 0x80) return false;
    }

  case 1: if(*source >= 0x80 && *source < 0xC2) return false;
  }
  if(*source > 0xF4) return false;
  return true;
}

/* --------------------------------------------------------------------- */

/*
* Exported function to return whether a UTF-8 sequence is legal or not.
* This is not used here; it's just exported.
*/
char isLegalUTF8Sequence(const UTF8 *source, const UTF8 *sourceEnd)
{
  int length = trailingBytesForUTF8[*source] + 1;
  if(length > sourceEnd - source)
  {
    return false;
  }
  return isLegalUTF8(source, length);
}

/* --------------------------------------------------------------------- */
BUN_DLLEXPORT size_t UTF8toUTF32(const char*BUN_RESTRICT input, ptrdiff_t srclen, char32_t*BUN_RESTRICT output, size_t buflen)
{
  if(!srclen) return 0;
  const UTF8* source = (UTF8*)input;
  UTF32* target = (UTF32*)output;
  UTF32* targetEnd = target + buflen;
  if(srclen < 0) srclen = PTRDIFF_MAX;
  buflen = 1;
  while(*input && (input - (const char*)source) < srclen)
  {
    buflen += (((*input) & 0b11000000) != 0b10000000);
    ++input;
  }
  if(!output) return buflen;
  const UTF8* sourceEnd = ((input - (const char*)source) < srclen) ? (UTF8*)(++input) : (source + srclen);

  while(source < sourceEnd)
  {
    UTF32 ch = 0;
    unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
    if(extraBytesToRead >= sourceEnd - source)
    {
      break;
    }
    /* Do this check whether lenient or strict */
    if(!isLegalUTF8(source, extraBytesToRead + 1))
    {
      break;
    }
    /*
    * The cases all fall through. See "Note A" below.
    */
    switch(extraBytesToRead)
    {
    case 5: ch += *source++; ch <<= 6;
    case 4: ch += *source++; ch <<= 6;
    case 3: ch += *source++; ch <<= 6;
    case 2: ch += *source++; ch <<= 6;
    case 1: ch += *source++; ch <<= 6;
    case 0: ch += *source++;
    }
    ch -= offsetsFromUTF8[extraBytesToRead];

    if(target >= targetEnd)
    {
      source -= (extraBytesToRead + 1); /* Back up the source pointer! */
      break;
    }
    if(ch <= UNI_MAX_LEGAL_UTF32)
    {
      /*
      * UTF-16 surrogate values are illegal in UTF-32, and anything
      * over Plane 17 (> 0x10FFFF) is illegal.
      */
      if(ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)
      {
        //if(flags == strictConversion)
        //{
        //  source -= (extraBytesToRead + 1); /* return to the illegal value itself */
        //  result = -1;
        //  break;
        //}
        //else
        //{
        *target++ = UNI_REPLACEMENT_CHAR;
        //}
      }
      else
      {
        *target++ = ch;
      }
    }
    else
    { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
      *target++ = UNI_REPLACEMENT_CHAR;
    }
  }
  return (size_t)(((char32_t*)target) - output);
}

/* ---------------------------------------------------------------------

Note A.
The fall-through switches in UTF-8 reading code save a
temp variable, some decrements & conditionals.  The switches
are equivalent to the following loop:
{
int tmpBytesToRead = extraBytesToRead+1;
do {
ch += *source++;
--tmpBytesToRead;
if (tmpBytesToRead) ch <<= 6;
} while (tmpBytesToRead > 0);
}
In UTF-8 writing code, the switches on "bytesToWrite" are
similarly unrolled loops.

--------------------------------------------------------------------- */