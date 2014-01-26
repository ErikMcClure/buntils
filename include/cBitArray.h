// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BITARRAY_H__BSS__ //These are used in case this header file is used by two different projects dependent on each other, resulting in duplicates which cannot be differentiated by #pragma once
#define __C_BITARRAY_H__BSS__

#include "cArraySimple.h"
#include "bss_util.h"
#include "cBitField.h"

namespace bss_util
{
  // Extremely fast bit array for compressed storage of any number of bools. O(1) speed for access regardless of size.
  template<typename StorageType=unsigned char, typename SizeType=unsigned int> //These should all be unsigned. If they aren't, things will explode and it will be YOUR FAULT
  class BSS_COMPILER_DLLEXPORT cBitArray
  {
  protected:
    typedef StorageType STORE_;
    typedef SizeType ST_;
    static const ST_ DIV_AMT=(sizeof(STORE_)<<3); 
    static const ST_ MOD_AMT=(sizeof(STORE_)<<3)-1;
    static_assert(std::is_unsigned<StorageType>::value, "StorageType must be an unsigned integral type.");    
    static_assert(std::is_unsigned<SizeType>::value, "SizeType must be an unsigned integral type.");    

  public:
    inline cBitArray(const cBitArray& copy) : _bits(copy), _numbits(copy._numbits){ } 
    inline cBitArray(cBitArray&& mov) : _bits(std::move(mov)), _numbits(mov._numbits) { mov._numbits=0; } 
    inline cBitArray(ST_ numbits=0) : _bits(_maxchunks(numbits)), _numbits(numbits) { Clear(); }
    inline void BSS_FASTCALL SetSize(ST_ numbits)
    { 
      ST_ last=_bits.Size();
      _bits.SetSize(_maxchunks(numbits));
      if(last<_bits.Size()) // If we got bigger, zero the new bytes
        memset(_bits+last,0,sizeof(STORE_)*(_bits.Size()-last));
      _numbits=numbits;
    }
    inline const STORE_* GetRaw() const { return _bits; }
    inline ST_ Length() const { return _numbits; }
    inline bool BSS_FASTCALL SetBit(ST_ bitindex, bool value)
    {
      if(bitindex>=_numbits) return false;
      ST_ realindex=(bitindex/DIV_AMT); //divide bitindex by 8
      STORE_ mask=(1<<(bitindex&MOD_AMT)); //assign bitindex the remainder
      _bits[realindex] = T_SETBIT(_bits[realindex],mask,value);
      return true;
    }    
    inline bool BSS_FASTCALL GetBit(ST_ bitindex) const 
    {
      assert(bitindex<_numbits);
      return (_bits[(bitindex/DIV_AMT)]&(((STORE_)1)<<(bitindex&MOD_AMT)))!=0;
    }
    /*inline bool BSS_FASTCALL SetBits(ST_ start, ST_ end, bool value)
    {
      length+=bitindex;
      ST_ start=bitindex/DIV_AMT;
      ST_ end=length/DIV_AMT;
      STORE_ smask = ~((((STORE_)1)<<(bitindex - (start*DIV_AMT)))-1);
      STORE_ emask = (~(STORE_)0)>>(length - (end*DIV_AMT));
      
      if(start==end)
        (smask&emask)
    }*/
    // Counts the bits in the given range
    ST_ BSS_FASTCALL GetBits(ST_ bitindex, ST_ length) const
    {
      length+=bitindex;
      ST_ start=bitindex/DIV_AMT;
      ST_ end=length/DIV_AMT;
      STORE_ smask = ~((((STORE_)1)<<(bitindex - (start*DIV_AMT)))-1);
      //STORE_ emask = (~(STORE_)0)>>(length - (end*DIV_AMT));
      char len = length - (end*DIV_AMT) - 1;
      STORE_ emask = T_GETBITRANGE(STORE_,0,len);
      
      if(start==end)
        return bitcount<STORE_>((_bits[start]&smask)&emask);
      
      ST_ c = bitcount<STORE_>(_bits[start]&smask);
      c += bitcount<STORE_>(_bits[end]&emask);

      for(ST_ i = start+1; i < end; ++i)
        c += bitcount<STORE_>(_bits[i]);

      return c;
    }
    inline void Clear() { memset(_bits,0,sizeof(STORE_)*_bits.Size()); }
    inline void Flip() { for(ST_ i = 0; i < _bits.Size(); ++i) _bits[i]=~_bits[i]; }
    inline cBitArray& operator=(const cBitArray& right) { _bits=right._bits; _numbits=right._numbits; }
    inline cBitArray& operator=(cBitArray&& mov) { _bits=std::move(mov._bits); _numbits=mov._numbits; mov._numbits=0; }
    BSS_FORCEINLINE bool operator[](ST_ index) const { return GetBit(index); }
    BSS_FORCEINLINE _cBIT_REF<STORE_> operator[](ST_ index) { return _cBIT_REF<STORE_>(((STORE_)1)<<(index&MOD_AMT),*(_bits+(index/DIV_AMT))); }
    BSS_FORCEINLINE cBitArray& operator+=(ST_ bitindex) { SetBit(bitindex,true); return *this; }
    BSS_FORCEINLINE cBitArray& operator-=(ST_ bitindex) { SetBit(bitindex,false); return *this; }

  protected:
    BSS_FORCEINLINE static ST_ _maxchunks(ST_ numbits) { return T_NEXTMULTIPLE(numbits,MOD_AMT); }

    typename WArray<STORE_,ST_>::t _bits;
    ST_ _numbits;
  };
}

#endif
