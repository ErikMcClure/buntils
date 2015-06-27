// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_DYN_ARRAY_H__BSS__
#define __C_DYN_ARRAY_H__BSS__

#include "cArray.h"

namespace bss_util {
  // Dynamic array implemented using ArrayType (should only be used when constructors could potentially not be needed)
  template<class T, typename SizeType=unsigned int, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cDynArray : protected cArrayBase<T, SizeType, ArrayType, Alloc>
  {
  protected:
    typedef cArrayBase<T, SizeType, ArrayType, Alloc> AT_;
    typedef typename AT_::ST_ ST_;
    typedef typename AT_::T_ T_;
    using AT_::_array;
    using AT_::_size;

  public:
    inline cDynArray(const cDynArray& copy) : AT_(copy), _length(copy._length) {}
    inline cDynArray(cDynArray&& mov) : AT_(std::move(mov)), _length(mov._length) {}
    inline explicit cDynArray(ST_ size=0) : AT_(size), _length(0) {}
    inline cDynArray(const std::initializer_list<T> list) : AT_(list), _length(list.size()) {}
    BSS_FORCEINLINE ST_ Add(const T_& t) { return _add(t); }
    BSS_FORCEINLINE ST_ Add(T_&& t) { return _add(std::move(t)); }
    BSS_FORCEINLINE void Remove(ST_ index) { AT_::RemoveInternal(index); --_length; }
    BSS_FORCEINLINE void RemoveLast() { --_length; }
    BSS_FORCEINLINE void Insert(const T_& t, ST_ index=0) { return _insert(t, index); }
    BSS_FORCEINLINE void Insert(T_&& t, ST_ index=0) { return _insert(std::move(t), index); }
    BSS_FORCEINLINE bool Empty() const { return !_length; }
    BSS_FORCEINLINE void Clear() { _length=0; }
    BSS_FORCEINLINE void SetLength(ST_ length) { if(length>_size) AT_::SetSize(length); _length=length; }
    BSS_FORCEINLINE ST_ Length() const { return _length; }
    BSS_FORCEINLINE ST_ Capacity() const { return _size; }
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
    inline cDynArray& operator +=(const cDynArray& add) { AT_::SetSize(_length); AT_::operator+=(add); _length+=add._length; return *this; }
    inline cDynArray operator +(const cDynArray& add) const { cDynArray r(*this); return (r+=add); }

  protected:
    template<typename U>
    ST_ _add(U && t) { _checksize(); _array[_length]=std::forward<U>(t); return _length++; }
    template<typename U>
    void _insert(U && t, ST_ index=0) { _checksize(); AT_::_pushback(index, (_length++)-index, std::forward<U>(t)); assert(_length<=_size); }
    BSS_FORCEINLINE void _checksize()
    {
      if(_length>=_size) AT_::SetSize(T_FBNEXT(_size));
      assert(_length<_size);
    }

    ST_ _length;
  };

  // A dynamic array that can dynamically adjust the size of each element
  template<typename ST_, typename Alloc=StaticAllocPolicy<unsigned char>>
  class BSS_COMPILER_DLLEXPORT cArbitraryArray : protected cArrayBase<unsigned char, ST_, CARRAY_SIMPLE, Alloc>
  {
  protected:
    typedef cArrayBase<unsigned char, ST_, CARRAY_SIMPLE, Alloc> AT_;
    using AT_::_array;
    using AT_::_size;

  public:
    inline cArbitraryArray(const cArbitraryArray& copy) : AT_(copy), _length(copy._length), _element(copy._element) {}
    inline cArbitraryArray(cArbitraryArray&& mov) : AT_(std::move(mov)), _length(mov._length), _element(mov._element) {}
    inline cArbitraryArray(ST_ size=0, ST_ element=1): AT_(size*element), _length(0), _element(element) {}
    template<typename T>
    inline ST_ Add(const T& t)
    {
      if((_length*_element)>=_size) AT_::SetSize(T_FBNEXT(_length)*_element);
      memcpy(_array+(_length*_element), &t, bssmin(sizeof(T), _element));
      return _length++;
    }
    inline void Remove(ST_ index)
    {
      index *= _element;
      assert(_size>0 && index<_size);
      memmove(_array+index, _array+index+_element, _size-index-_element);
      --_length;
    }
    inline void RemoveLast() { --_length; }
    inline void SetElement(const void* newarray, ST_ element, ST_ num) // num is a count of how many elements are in the array
    {
      if(((unsigned char*)newarray)==_array) return;
      _element=element;
      _length=num;
      if((_length*_element)>_size) AT_::SetSizeDiscard(_length*_element);
      if(_array)
        memcpy(_array, newarray, element*num);
    }
    template<typename T> // num is a count of how many elements are in the array
    BSS_FORCEINLINE void SetElement(const T* newarray, ST_ num) { SetElement(newarray, sizeof(T), num); }
    template<typename T, unsigned int NUM>
    BSS_FORCEINLINE void SetElement(const T(&newarray)[NUM]) { SetElement(newarray, sizeof(T), NUM); }
    void SetElement(ST_ element)
    {
      if(element==_element) return;
      _size=element*_length;
      unsigned char* narray = !_length?0:(unsigned char*)Alloc::allocate(_size);
      memset(narray, 0, _size);
      ST_ m=bssmin(element, _element);
      for(unsigned int i = 0; i < _length; ++i)
        memcpy(narray+(i*element), _array+(i*_element), m);
      if(_array)
        Alloc::deallocate(_array);
      _array=narray;
      _element=element;
      assert(element>0);
    }
    inline ST_ Element() const { return _element; }
    inline bool Empty() const { return !_length; }
    inline void Clear() { _length=0; }
    inline void SetLength(ST_ length) { _length=length; length*=_element; if(length>_size) AT_::SetSize(length); }
    inline ST_ Length() const { return _length; }
    // These are templatized accessors, but we don't do an assertion on if the size of T is the same as _element, because there are many cases where this isn't actually true.
    template<typename T> inline const T& Front() const { assert(_length>0); return _array[0]; }
    template<typename T> inline const T& Back() const { assert(_length>0); return _array[(_length-1)*_element]; }
    template<typename T> inline T& Front() { assert(_length>0); return _array[0]; }
    template<typename T> inline T& Back() { assert(_length>0); return _array[(_length-1)*_element]; }
    template<typename T> inline const T* begin() const { return _array; }
    template<typename T> inline const T* end() const { return _array+(_length*_element); }
    template<typename T> inline T* begin() { return _array; }
    template<typename T> inline T* end() { return _array+(_length*_element); }
    template<typename T> inline const T& Get(ST_ index) const { return *(T*)Get(index); }
    template<typename T> inline T& Get(ST_ index) { return *(T*)Get(index); }
    BSS_FORCEINLINE const void* Get(ST_ index) const { assert(index<_length); return (_array+(index*_element)); }
    BSS_FORCEINLINE void* Get(ST_ index) { assert(index<_length); return (_array+(index*_element)); }

    inline const void* operator[](ST_ index) const { return Get(index); }
    inline void* operator[](ST_ index) { return Get(index); }
    inline cArbitraryArray& operator=(const cArbitraryArray& copy) { AT_::operator=(copy); _length=copy._length; _element=copy._element; return *this; }
    inline cArbitraryArray& operator=(cArbitraryArray&& mov) { AT_::operator=(std::move(mov)); _length=mov._length; _element=mov._element; return *this; }

  protected:
    ST_ _length; // Total length of the array in number of elements
    ST_ _element; // Size of each element
  };
}

#endif
