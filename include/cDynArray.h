// Copyright �2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_DYN_ARRAY_H__BSS__
#define __C_DYN_ARRAY_H__BSS__

#include "cArraySimple.h"

namespace bss_util {
  /* Dynamic array implemented using ARRAYTYPE (should only be used in situations where std::vector has problems) */
  template<class ARRAYTYPE>
  class BSS_COMPILER_DLLEXPORT cDynArray : public ARRAYTYPE
  {
    typedef typename ARRAYTYPE::__ST __ST;
    typedef typename ARRAYTYPE::__T __T;
    typedef ARRAYTYPE __AT;

  public:
    //inline cDynArray(const cDynArray& copy) : __AT(copy), _length(copy._length) {}
    inline cDynArray(cDynArray&& mov) : __AT(std::move(mov)), _length(mov._length) {}
    inline explicit cDynArray(__ST size=1): __AT(size), _length(0) {}
    inline __ST Add(const __T& t) { _checksize(); _array[_length]=t; return _length++; }
    inline void Remove(__ST index) { __AT::Remove(index); --_length; }
    inline void RemoveLast() { --_length; }
    inline __ST Insert(const __T& t, __ST index=0)
    {
      _checksize();
      __AT::_pushback(index,(_length++)-index,t);
      return index;
    }
    inline __ST Length() const { return _length; }
    inline bool IsEmpty() const { return !_length; }
    inline void Clear() { _length=0; }
    inline void SetLength(__ST length) { if(length>_size) SetSize(length); _length=length; }

    inline operator __T*() { return _array; }
    inline operator const __T*() const { return _array; }
    inline cDynArray& operator=(const __AT& copy) { __AT::operator=(copy); _length=_size; return *this; }
    inline cDynArray& operator=(__AT&& mov) { __AT::operator=(std::move(mov)); _length=_size; return *this; }
    inline cDynArray& operator=(const cDynArray& copy) { __AT::operator=(copy); _length=copy._length; return *this; }
    inline cDynArray& operator=(cDynArray&& mov) { __AT::operator=(std::move(mov)); _length=mov._length; return *this; }
    //inline cDynArray& operator +=(const cDynArray& add) { __AT::operator+=(add); return *this; }
    //inline const cDynArray operator +(const cDynArray& add) { cArrayWrap r(*this); return (r+=add); }

  protected:
    inline void _checksize()
    {
      if(_length>=_size) SetSize(fbnext(_size));
      assert(_length<_size);
    }

    __ST _length;
  };
}

#endif