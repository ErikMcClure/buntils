// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BITARRAY_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_BITARRAY_H__BSS__

#include <memory.h>
#include "bss_deprecated.h"
#include "bss_util.h"

namespace bss_util
{
  /* Extremely fast bit array for compressed storage of any number of bools. O(1) speed for access regardless of size. */
  template<typename _StorageType=unsigned char, typename _SizeType=unsigned int> //These should all be unsigned. If they aren't, things will explode and it will be YOUR FAULT
  class __declspec(dllexport) cBitArray
  {
  protected:
    typedef _StorageType __STORE;
    typedef _SizeType __ST;
    const __ST DIV_AMT; //this is of type __ST becuase that's what integral types these will be applied to, so anything else would just be converted anyway
    const __ST MOD_AMT;
  
  public:
    inline cBitArray(const cBitArray& copy) : _bits(0), DIV_AMT(log2_p2(sizeof(_StorageType)<<3)), MOD_AMT((sizeof(_StorageType)<<3)-1) { operator=(copy); } 
    inline cBitArray(cBitArray&& mov) : _bits(0), DIV_AMT(log2_p2(sizeof(_StorageType)<<3)), MOD_AMT((sizeof(_StorageType)<<3)-1) { operator=(std::move(mov)); } 
    inline cBitArray(__ST numbits=0) : _bits(0), _numbits(0), _reservedbytes(0), DIV_AMT(log2_p2(sizeof(_StorageType)<<3)), MOD_AMT((sizeof(_StorageType)<<3)-1) { SetSize(numbits); }
    inline ~cBitArray() { if(_bits) delete [] _bits; }
    inline void BSS_FASTCALL SetSize(__ST numbits) { _resize((numbits&MOD_AMT)==0?(numbits>>DIV_AMT):((numbits>>DIV_AMT)+1)); _numbits=numbits; }
    inline __ST Size() const { return _numbits; }
    inline bool BSS_FASTCALL SetBit(__ST bitindex, bool value)
    {
      if(bitindex>=_numbits) return false;
      __ST realindex=(bitindex>>DIV_AMT); //divide bitindex by 8
      bitindex=(bitindex&MOD_AMT); //assign bitindex the remainder
      _bits[realindex] = value?(_bits[realindex]|(1<<bitindex)):(_bits[realindex]&(~(1<<bitindex)));
      return true;
    }    
    inline char BSS_FASTCALL GetBit(__ST bitindex) const //returns -1 on failure, 0 for false and 1 for true
    {
      if(bitindex>=_numbits) return -1;
      __ST realindex=(bitindex>>DIV_AMT); //divide bitindex by the size of our integral type (8, 16, 32, 64 etc.)
      bitindex=(bitindex&MOD_AMT); //assign bitindex the remainder (we do this by using the bits in our integral minus one, (7, 15, etc.) because it sets all the relevent bits to 1)
      return (_bits[realindex]&(1<<bitindex))==0?0:1;
    }
    //inline bool BSS_FASTCALL SetBits(__ST start, __ST end, bool value);
    //inline __ST BSS_FASTCALL GetBits(__ST bitindex, __ST length) const; //returns -1 on failure, 0 if all are false or simply returns the number of bits that are true
    inline void Clear() { memset(_bits,0,sizeof(__STORE)*_reservedbytes); }

    inline cBitArray& operator=(const cBitArray& right)
    { 
      if(this == &right) return *this;
      if(right._reservedbytes>_reservedbytes)
      { 
        if(_bits) delete [] _bits;
        _bits=new __STORE[right._reservedbytes*sizeof(__STORE)];
        _reservedbytes=right._reservedbytes;
      } 
      _numbits=right._numbits;
      memset(_bits, 0, _reservedbytes*sizeof(__STORE));
      memcpy(_bits, right._bits,_reservedbytes*sizeof(__STORE));
      return *this;
    }
    inline cBitArray& operator=(cBitArray&& mov)
    {
      if(this == &mov) return *this;
      if(_bits) delete [] _bits;
      _bits=mov._bits;
      _numbits=mov._numbits;
      _reservedbytes=mov._reservedbytes;
      mov._bits=0;
      mov._reservedbytes=0;
      mov._numbits=0;
    }
    inline char operator[](__ST index) const { return GetBit(index); } //returns -1 on failure, 0 for false and 1 for true

  protected:
    inline void BSS_FASTCALL _resize(__ST numbytes)
    {
      if(numbytes>_reservedbytes)
      {
        __STORE* nbits = new __STORE[numbytes*sizeof(__STORE)];

        if(_bits)
        {
          MEMCPY(nbits,numbytes,_bits,_reservedbytes*sizeof(__STORE));
          delete [] _bits;
        }

        _bits = nbits;
        memset(_bits+_reservedbytes,0,(numbytes-_reservedbytes)*sizeof(__STORE)); //Sets difference to 0
        _reservedbytes = numbytes;
      }
    }

    __STORE* _bits;

  private:
    __ST _numbits;
    __ST _reservedbytes; //"bytes" here is a misnomer - its really the size of the integral array
  };
}

#endif