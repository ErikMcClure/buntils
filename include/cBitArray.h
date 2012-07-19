// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BITARRAY_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_BITARRAY_H__BSS__

#include "cArraySimple.h"
#include "bss_deprecated.h"
#include "bss_util.h"

namespace bss_util
{
  template<class __ST>
	struct _BIT_REF // reference to a bit within a base word
	{	
    inline _BIT_REF(__ST* p, __ST off) : _p(p), _m(1<<off) {}
    inline void flip() { *_p ^= _m; }

    inline bool operator~() const { return operator!(); }
    inline bool operator!() const { return !((*_p)&_m); }
	  inline operator bool() const {	return ((*_p)&_m) != 0; }
    inline _BIT_REF& operator=(const _BIT_REF& r) { return operator=((bool)r); }
    inline _BIT_REF& operator=(bool r) { *_p = r?((*_p)|_m):((*_p)&(~_m));  return *this; }
	};

  /* Extremely fast bit array for compressed storage of any number of bools. O(1) speed for access regardless of size. */
  template<typename StorageType=unsigned char, typename SizeType=unsigned int> //These should all be unsigned. If they aren't, things will explode and it will be YOUR FAULT
  class BSS_COMPILER_DLLEXPORT cBitArray
  {
  protected:
    typedef StorageType __STORE;
    typedef SizeType __ST;
    static const __ST DIV_AMT=(sizeof(__STORE)<<3); 
    static const __ST MOD_AMT=(sizeof(__STORE)<<3)-1;
    
  public:
    inline cBitArray(const cBitArray& copy) : _bits(copy), _numbits(copy._numbits){ } 
    inline cBitArray(cBitArray&& mov) : _bits(std::move(mov)), _numbits(mov._numbits) { mov._numbits=0; } 
    inline cBitArray(__ST numbits=0) : _bits(_maxchunks(numbits)), _numbits(numbits) { Clear(); }
    inline ~cBitArray() { if(_bits) delete [] _bits; }
    inline void BSS_FASTCALL SetSize(__ST numbits)
    { 
      __ST last=_bits.Size();
      _bits.SetSize(_maxchunks(numbits));
      if(last<_bits.Size()) // If we got bigger, zero the new bytes
        memset(_bits+last,0,sizeof(__STORE)*(_bits.Size()-last));
      _numbits=numbits;
    }
    inline const __STORE* GetRaw() const { return _bits; }
    inline __ST Length() const { return _numbits; }
    inline bool BSS_FASTCALL SetBit(__ST bitindex, bool value)
    {
      if(bitindex>=_numbits) return false;
      __ST realindex=(bitindex/DIV_AMT); //divide bitindex by 8
      __STORE mask=(1<<(bitindex&MOD_AMT)); //assign bitindex the remainder
      _bits[realindex] = value?(_bits[realindex]|mask):(_bits[realindex]&(~mask));
      return true;
    }    
    inline bool BSS_FASTCALL GetBit(__ST bitindex) const 
    {
      assert(bitindex<_numbits);
      return (_bits[(bitindex/DIV_AMT)]&(((__STORE)1)<<(bitindex&MOD_AMT)))!=0;
    }
    /*inline bool BSS_FASTCALL SetBits(__ST start, __ST end, bool value)
    {
      length+=bitindex;
      __ST start=bitindex/DIV_AMT;
      __ST end=length/DIV_AMT;
      __STORE smask = ~((((__STORE)1)<<(bitindex - (start*DIV_AMT)))-1);
      __STORE emask = (~(__STORE)0)>>(length - (end*DIV_AMT));
      
      if(start==end)
        (smask&emask)
    }*/
    /* Counts the bits in the given range */
    inline __ST BSS_FASTCALL GetBits(__ST bitindex, __ST length) const
    {
      length+=bitindex;
      __ST start=bitindex/DIV_AMT;
      __ST end=length/DIV_AMT;
      __STORE smask = ~((((__STORE)1)<<(bitindex - (start*DIV_AMT)))-1);
      __STORE emask = (~(__STORE)0)>>(length - (end*DIV_AMT));
      if(start==end)
        return bitcount<__STORE>((_bits[start]&smask)&emask);
      
      __ST c = bitcount<__STORE>(_bits[start]&smask);
      c += bitcount<__STORE>(_bits[end]&emask);

      for(__ST i = start+1; i < end; ++i)
        c += bitcount<__STORE>(_bits[i]);

      return c;
    }
    inline void Clear() { memset(_bits,0,sizeof(__STORE)*_bits.Size()); }
    inline void Flip() { for(__ST i = 0; i < _bits.Size(); ++i) _bits[i]=~_bits[i]; }
    inline cBitArray& operator=(const cBitArray& right) { _bits=right._bits; _numbits=right._numbits; }
    inline cBitArray& operator=(cBitArray&& mov) { _bits=std::move(mov._bits); _numbits=mov._numbits; mov._numbits=0; }
    inline bool operator[](__ST index) const { return GetBit(index); }
    inline _BIT_REF<__ST> operator[](__ST index) { return _BIT_REF<__ST>(_bits+(bitindex/DIV_AMT),(bitindex&MOD_AMT)); }

  protected:
    inline __ST _maxchunks(__ST numbits) { return (numbits/DIV_AMT) + ((numbits&MOD_AMT)!=0) }

    cArraySimple<__STORE,__ST> _bits;
    __ST _numbits;
  };
}

#endif