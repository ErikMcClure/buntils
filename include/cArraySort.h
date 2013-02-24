// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_SORT_H__BSS__
#define __C_ARRAY_SORT_H__BSS__

#include "bss_algo.h"
#include "bss_compare.h"
#include "cArraySimple.h"

namespace bss_util {
  // Sorted dynamic array
  template<typename T, char (*CFunc)(const T&,const T&)=CompT<T>, typename SizeType=unsigned int, class ArrayType=cArraySimple<T,SizeType>>
  class BSS_COMPILER_DLLEXPORT cArraySort : protected ArrayType
  {
  public:
    typedef SizeType ST_;
    typedef const T& constref;
    typedef T&& moveref;
    using ArrayType::_array;
    using ArrayType::_size;

    inline cArraySort(const cArraySort& copy) : _length(copy._length), ArrayType(copy) {} 
    inline cArraySort(cArraySort&& mov) : _length(mov._length), ArrayType(std::move(mov)) {} 
    inline explicit cArraySort(ST_ size=0) : _length(0), ArrayType(size) {}
    inline ~cArraySort() { }
    inline BSS_FORCEINLINE ST_ BSS_FASTCALL Insert(constref data) { return _insert(data); }
    inline BSS_FORCEINLINE ST_ BSS_FASTCALL Insert(moveref data) { return _insert(std::move(data)); }
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
    inline ST_ BSS_FASTCALL ReplaceData(ST_ index, constref data)
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
      ArrayType::RemoveInternal(index);
      --_length;
      return true;
    }
    inline BSS_FORCEINLINE void BSS_FASTCALL Expand(ST_ newsize)
    {
      ArrayType::SetSize(newsize);
    }
    inline ST_ BSS_FASTCALL Find(constref data) const
    {
      return binsearch_exact<T,T,ST_,CFunc>(_array,data,0,_length);
      //ST_ retval=_findnear(data,true);
      //return ((retval!=(ST_)(-1))&&(!CFunc(_array[retval],data)))?retval:(ST_)(-1);
    }
    // Can actually return -1 if there isn't anything in the array
    inline ST_ BSS_FASTCALL FindNear(constref data, bool before=true) const
    {
      ST_ retval=before?binsearch_before<T,ST_,CFunc>(_array,_length,data):binsearch_after<T,ST_,CFunc>(_array,_length,data);
      return (retval<_length)?retval:(ST_)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }
    inline bool IsEmpty() const { return !_length; }
    inline ST_ Length() const { return _length; }
    inline constref operator [](ST_ index) const { return _array[index]; }
    inline T& operator [](ST_ index) { return _array[index]; }
    inline cArraySort& operator=(const cArraySort& right)
    { 
      ArrayType::operator=(right);
      _length=right._length;
      return *this;
    }
    inline cArraySort& operator=(cArraySort&& mov)
    {
      ArrayType::operator=(std::move(mov));
      _length=mov._length;
      return *this;
    }
  protected:
    template<typename U>
    inline ST_ BSS_FASTCALL _insert(U && data)
    {
      if(_length>=_size) Expand(fbnext(_size));
      if(!_length) _array[_length++]=std::forward<U>(data);
      else
      {
        ST_ loc = binsearch_after<T,ST_,CFunc>(_array,_length,std::forward<U>(data));
        ArrayType::_pushback(loc,(_length++)-loc,std::forward<U>(data));
        return loc;
      }
      return 0;
    }

    ST_ _length; //How many slots are used
  };
}

#endif
