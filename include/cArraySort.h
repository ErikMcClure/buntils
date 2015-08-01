// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_SORT_H__BSS__
#define __C_ARRAY_SORT_H__BSS__

#include "bss_algo.h"
#include "bss_compare.h"
#include "cArray.h"

namespace bss_util {
  // A dynamic array that keeps its contents sorted using insertion sort and uses a binary search to retrieve items.
  template<typename T, char(*CFunc)(const T&, const T&)=CompT<T>, typename SizeType = unsigned int, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cArraySort : protected cArrayBase<T, SizeType, ArrayType, Alloc>
  {
  public:
    typedef SizeType ST_;
    typedef const T& constref;
    typedef T&& moveref;
    typedef cArrayBase<T, SizeType, ArrayType, Alloc> AT_;
    using AT_::_array;
    using AT_::_size;

    inline cArraySort(const cArraySort& copy) : _length(copy._length), AT_(copy) {}
    inline cArraySort(cArraySort&& mov) : _length(mov._length), AT_(std::move(mov)) {}
    inline explicit cArraySort(ST_ size=0) : _length(0), AT_(size) {}
    inline ~cArraySort() { }
    BSS_FORCEINLINE ST_ BSS_FASTCALL Insert(constref data) { return _insert(data); }
    BSS_FORCEINLINE ST_ BSS_FASTCALL Insert(moveref data) { return _insert(std::move(data)); }
    inline void Clear() { _length=0; }
    inline void BSS_FASTCALL Discard(unsigned int num) { _length-=((num>_length)?_length:num); }
    inline const T& Front() const { assert(_length>0); return _array[0]; }
    inline T& Front() { assert(_length>0); return _array[0]; }
    inline const T& Back() const { assert(_length>0); return _array[_length-1]; }
    inline T& Back() { assert(_length>0); return _array[_length-1]; }
    inline const T* begin() const { return _array; }
    inline const T* end() const { return _array+_length; }
    inline T* begin() { return _array; }
    inline T* end() { return _array+_length; }
    ST_ BSS_FASTCALL ReplaceData(ST_ index, constref data)
    {
      T swap;
      _array[index]=data;
      while(index>0 && CFunc(_array[index-1], _array[index])>0)
      { //we do a swap here and hope that the compiler is smart enough to optimize it
        swap=_array[index];
        _array[index]=_array[index-1];
        _array[--index]=swap;
      }
      while(index<(_length-1) && CFunc(_array[index+1], _array[index])<0)
      { //we do a swap here and hope that the compiler is smart enough to optimize it
        swap=_array[index];
        _array[index]=_array[index+1];
        _array[++index]=swap;
      }
      return index;
    }
    inline bool BSS_FASTCALL Remove(ST_ index)
    {
      if(index<0||index>=_length) return false;
      AT_::RemoveInternal(index);
      --_length;
      return true;
    }
    BSS_FORCEINLINE void BSS_FASTCALL Expand(ST_ newsize)
    {
      AT_::SetSize(newsize);
    }
    BSS_FORCEINLINE ST_ BSS_FASTCALL Find(constref data) const
    {
      return binsearch_exact<T, T, ST_, CFunc>(_array, data, 0, _length);
    }
    // Can actually return -1 if there isn't anything in the array
    inline ST_ BSS_FASTCALL FindNear(constref data, bool before=true) const
    {
      ST_ retval=before?binsearch_before<T, ST_, CFunc>(_array, _length, data):binsearch_after<T, ST_, CFunc>(_array, _length, data);
      return (retval<_length)?retval:(ST_)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }
    BSS_FORCEINLINE bool Empty() const { return !_length; }
    BSS_FORCEINLINE ST_ Length() const { return _length; }
    BSS_FORCEINLINE constref operator [](ST_ index) const { return _array[index]; }
    BSS_FORCEINLINE T& operator [](ST_ index) { return _array[index]; }
    inline cArraySort& operator=(const cArraySort& right)
    {
      AT_::operator=(right);
      _length=right._length;
      return *this;
    }
    inline cArraySort& operator=(cArraySort&& mov)
    {
      AT_::operator=(std::move(mov));
      _length=mov._length;
      return *this;
    }
  protected:
    template<typename U>
    ST_ BSS_FASTCALL _insert(U && data)
    {
      if(_length>=_size) Expand(fbnext(_size));
      if(!_length) _array[_length++]=std::forward<U>(data);
      else
      {
        ST_ loc;
        if((*CFunc)(data, _array[0]) < 0) loc = 0;
        else if((*CFunc)(data, _array[_length - 1]) >= 0) loc = _length;
        else loc = binsearch_after<T, ST_, CFunc>(_array, _length, std::forward<U>(data));
        AT_::_pushback(loc, (_length++)-loc, std::forward<U>(data));
        return loc;
      }
      return 0;
    }

    ST_ _length; //How many slots are used
  };
}

#endif
