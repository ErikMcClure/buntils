// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_DYN_ARRAY_H__BSS__
#define __C_DYN_ARRAY_H__BSS__

#include "cArraySimple.h"

namespace bss_util {
  // Dynamic array implemented using ArrayType (should only be used when std::vector won't work, for example, if constructors aren't needed)
  template<class ArrayType>
  class BSS_COMPILER_DLLEXPORT cDynArray : protected ArrayType
  {
  protected:
    typedef typename ArrayType::ST_ ST_;
    typedef typename ArrayType::T_ T_;
    typedef ArrayType AT_;
    using ArrayType::_array;
    using ArrayType::_size;

  public:
    //inline cDynArray(const cDynArray& copy) : AT_(copy), _length(copy._length) {}
    inline cDynArray(cDynArray&& mov) : AT_(std::move(mov)), _length(mov._length) {}
    inline explicit cDynArray(ST_ size=0): AT_(size), _length(0) {}
    inline ST_ Add(const T_& t) { return _add(t); }
    inline ST_ Add(T_&& t) { return _add(std::move(t)); }
    inline void Remove(ST_ index) { AT_::RemoveInternal(index); --_length; }
    inline void RemoveLast() { --_length; }
    inline void Insert(const T_& t, ST_ index=0) { return _insert(t,index); }
    inline void Insert(T_&& t, ST_ index=0) { return _insert(std::move(t),index); }
    inline ST_ Length() const { return _length; }
    inline bool IsEmpty() const { return !_length; }
    inline void Clear() { _length=0; }
    inline void SetLength(ST_ length) { if(length>_size) AT_::SetSize(length); _length=length; }
    inline const T_& Front() const { assert(_length>0); return _array[0]; }
    inline const T_& Back() const { assert(_length>0); return _array[_length-1]; }
    inline T_& Front() { assert(_length>0); return _array[0]; }
    inline T_& Back() { assert(_length>0); return _array[_length-1]; }
    inline const T_* begin() const { return _array; }
    inline const T_* end() const { return _array+_length; }
    inline T_* begin() { return _array; }
    inline T_* end() { return _array+_length; }

    inline operator T_*() { return _array; }
    inline operator const T_*() const { return _array; }
    inline cDynArray& operator=(const AT_& copy) { AT_::operator=(copy); _length=_size; return *this; }
    inline cDynArray& operator=(AT_&& mov) { AT_::operator=(std::move(mov)); _length=_size; return *this; }
    inline cDynArray& operator=(const cDynArray& copy) { AT_::operator=(copy); _length=copy._length; return *this; }
    inline cDynArray& operator=(cDynArray&& mov) { AT_::operator=(std::move(mov)); _length=mov._length; return *this; }
    inline cDynArray& operator +=(const cDynArray& add) { SetSize(_length); AT_::operator+=(add); _length+=add._length; return *this; }
    inline const cDynArray operator +(const cDynArray& add) const { cDynArray r(*this); return (r+=add); }

  protected:
    template<typename U>
    inline ST_ _add(U && t) { _checksize(); _array[_length]=std::forward<U>(t); return _length++; }
    template<typename U>
    inline void _insert(U && t, ST_ index=0) { _checksize(); AT_::_pushback(index,(_length++)-index,std::forward<U>(t)); assert(_length<=_size); }
    inline void _checksize()
    {
      if(_length>=_size) AT_::SetSize(fbnext(_size));
      assert(_length<_size);
    }

    ST_ _length;
  };
}

#endif
