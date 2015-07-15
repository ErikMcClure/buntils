// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_H__BSS__
#define __C_ARRAY_H__BSS__

#include "bss_alloc.h"
#include <string.h>
#include <initializer_list>

namespace bss_util {
  enum ARRAY_TYPE : unsigned char { CARRAY_SIMPLE=0, CARRAY_CONSTRUCT=1, CARRAY_SAFE=2 };

  // cArrayBase class that can be specialized as a simple, constructor-only, or fully safe array for performance reasons.
  // Defaults to a simple array used to avoid constructors and assignment operator bottlenecks for simple data types.
  template<class T, typename SizeType=unsigned int, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cArrayBase
  {
  public:
    typedef SizeType ST_; // There are cases when you need access to these types even if you don't inherit (see cRandomQueue in bss_algo.h)
    typedef T T_;
    static_assert(std::is_integral<SizeType>::value, "SizeType must be integral");

    inline cArrayBase<T, ST_, ArrayType, Alloc>(const cArrayBase<T, ST_, ArrayType, Alloc>& copy) : _array(!copy._size?(T*)0:(T*)Alloc::allocate(copy._size)), _size(copy._size)
    {
      memcpy(_array, copy._array, _size*sizeof(T));
    }
    inline cArrayBase<T, ST_, ArrayType, Alloc>(cArrayBase<T, ST_, ArrayType, Alloc>&& mov) : _array(mov._array), _size(mov._size)
    {
      mov._array=0;
      mov._size=0;
    }
    inline explicit cArrayBase<T, ST_, ArrayType, Alloc>(ST_ size) : _array(!size?(T*)0:(T*)Alloc::allocate(size)), _size(size) {}
    inline cArrayBase<T, ST_, ArrayType, Alloc>(const std::initializer_list<T> list) : _array(!list.size()?(T*)0:(T*)Alloc::allocate(list.size())), _size(list.size())
    {
      auto end = list.end();
      int c = 0;
      for(auto i = list.begin(); i != end && c < _size; ++i) _array[c++] = *i;
    }
    inline ~cArrayBase<T, ST_, ArrayType, Alloc>()
    {
      if(_array!=0)
        Alloc::deallocate(_array);
    }
    inline ST_ Size() const { return _size; }
    void BSS_FASTCALL SetSizeDiscard(ST_ nsize)
    {
      if(nsize<=_size) { _size=nsize; return; }
      if(_array!=0) Alloc::deallocate(_array);
      //_array = (T*)malloc(nsize*sizeof(T));
      _array = (T*)Alloc::allocate(nsize);
      _size=!_array?0:nsize;
    }
    void BSS_FASTCALL SetSize(ST_ nsize)
    {
      if(nsize<=_size) { _size=nsize; return; }
      //T* narray = (T*)realloc(_array,nsize*sizeof(T));
      T* narray = (T*)Alloc::allocate(nsize, _array);
      if(!narray && nsize>0) Alloc::deallocate(_array); // realloc won't free _array if it fails
      _array=!nsize?0:narray; // If nsize is 0 realloc will just deallocate _array.
      _size=!_array?0:nsize;
    }
    inline void BSS_FASTCALL RemoveInternal(ST_ index)
    {
      assert(_size>0 && index<_size);
      memmove(_array+index, _array+index+1, (_size-index-1)*sizeof(T));
      //--_size;
    }
    void BSS_FASTCALL Insert(T item, ST_ location)
    {
      ST_ nsize=_size+1;
      if(location==_size)
      {
        SetSize(nsize);
        assert(_array!=0);
        _array[location]=item;
        return;
      }
      assert(location<_size);

      //T* narray = (T*)malloc(nsize*sizeof(T)); // nsize can't be 0
      T* narray = (T*)Alloc::allocate(nsize);
      assert(narray!=0);
      memcpy(narray, _array, location*sizeof(T)); // array will never be zero here, because if _size was 0, the only valid location is also 0, which triggers an add.
      narray[location]=item;
      memcpy(narray+location+1, _array+location, (_size-location)*sizeof(T));
      if(_array!=0) Alloc::deallocate(_array);
      _array=narray;
      _size=nsize;
    }
    //inline operator T*() { return _array; }
    //inline operator const T*() const { return _array; }
    cArrayBase<T, ST_, ArrayType, Alloc>& operator=(const cArrayBase<T, ST_, ArrayType, Alloc>& copy)
    {
      if(this == &copy) return *this;
      if(_array!=0) Alloc::deallocate(_array);
      _size=copy._size;
      //_array=!_size?0:(T*)malloc(_size*sizeof(T));
      _array=!_size?0:(T*)Alloc::allocate(_size*sizeof(T));
      if(_array)
        memcpy(_array, copy._array, _size*sizeof(T));
      return *this;
    }
    cArrayBase<T, ST_, ArrayType, Alloc>& operator=(cArrayBase<T, ST_, ArrayType, Alloc>&& mov)
    {
      if(this == &mov) return *this;
      if(_array!=0) Alloc::deallocate(_array);
      _array=mov._array;
      _size=mov._size;
      mov._array=0;
      mov._size=0;
      return *this;
    }
    cArrayBase<T, ST_, ArrayType, Alloc>& operator +=(const cArrayBase<T, ST_, ArrayType, Alloc>& add)
    {
      assert(this!=&add);
      ST_ oldsize=_size;
      SetSize(_size+add._size);
      if(add._size>0)
        memcpy(_array+oldsize, add._array, add._size*sizeof(T));
      return *this;
    }
    BSS_FORCEINLINE cArrayBase<T, ST_, ArrayType, Alloc> operator +(const cArrayBase<T, ST_, ArrayType, Alloc>& add) const
    {
      cArrayBase<T, ST_, ArrayType, Alloc> retval(*this);
      retval+=add;
      return retval;
    }
    inline void Scrub(int val)
    {
      if(_array!=0)
        memset(_array, val, _size*sizeof(T));
    }
    inline void SetArray(const T* a, ST_ n, ST_ s=0) { SetSize(n+s); memcpy(_array+s, a, n*sizeof(T)); }

  protected:
    //BSS_FORCEINLINE static void* _minmalloc(size_t n) { return malloc((n<1)?1:n); } //Malloc can legally return NULL if it tries to allocate 0 bytes
    template<typename U>
    inline void _pushback(ST_ index, ST_ length, U && data)
    {
      _mvarray(index+1, index, length);
      _array[index]=std::forward<U>(data);
    }
    BSS_FORCEINLINE void _mvarray(ST_ begin, ST_ end, ST_ length)
    {
      memmove(_array+begin, _array+end, length*sizeof(T));
    }
    //inline void _setsize(SizeType nsize, int val)
    //{
    //  SizeType last=_size;
    //  SetSize(nsize);
    //  if(_size>last)
    //    memset(_array+last,val,(_size-last)*sizeof(T));
    //}

    T* _array;
    SizeType _size;
  };

  // Very simple "dynamic" array that calls the constructor and destructor
  template<class T, typename SizeType, typename Alloc>
  class BSS_COMPILER_DLLEXPORT cArrayBase<T, SizeType, CARRAY_CONSTRUCT, Alloc>
  {
    static_assert(std::is_integral<SizeType>::value, "SizeType must be integral");
  public:
    inline cArrayBase(const cArrayBase& copy) : _array(!copy._size?0:Alloc::allocate(copy._size)), _size(copy._size)
    {
      //memcpy(_array,copy._array,_size*sizeof(T)); // Can't use memcpy on an external source because you could end up copying a pointer that would later be destroyed
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T(copy._array[i]);
    }
    inline cArrayBase(cArrayBase&& mov) : _array(mov._array), _size(mov._size)
    {
      mov._array=0;
      mov._size=0;
    }
    inline explicit cArrayBase(SizeType size) : _array(!size?0:Alloc::allocate(size)), _size(size)
    {
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T();
    }
    inline cArrayBase(const std::initializer_list<T> list) : _array((T*)Alloc::allocate(list.size())), _size(list.size())
    {
      auto end = list.end();
      int c = 0;
      for(auto i = list.begin(); i != end && c < _size; ++i) new (_array+(c++)) T(*i); // Use copy constructors to initialize array elements
      while(c < _size) new (_array+(c++)) T(); // If you didn't pass in enough parameters, ensure the array is still initialized
    }
    inline ~cArrayBase()
    {
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      if(_array!=0)
        Alloc::deallocate(_array);
    }
    inline SizeType Size() const { return _size; }
    void BSS_FASTCALL SetSize(SizeType nsize)
    {
      if(nsize==_size) return;
      T* narray = !nsize?0:Alloc::allocate(nsize); // can't use realloc because we have to destruct ones first.
      memcpy(narray, _array, bssmin(nsize, _size)*sizeof(T)); // We can do this because these aren't external sources.

      if(nsize<_size) { //we removed some so we need to destroy them
        for(SizeType i = _size; i > nsize;)
          (_array+(--i))->~T();
      } else { //we created some so we need to construct them
        for(SizeType i = _size; i < nsize; ++i)
          new(narray+i) T();
      }

      if(_array!=0) Alloc::deallocate(_array);
      _array=narray;
      _size=nsize;
    }
    void BSS_FASTCALL RemoveInternal(SizeType index)
    {
      assert(_size>0 && index<_size);
      _array[index].~T();
      memmove(_array+index, _array+index+1, (_size-index-1)*sizeof(T));
      new(_array+(_size-1)) T();
    }
    cArrayBase& operator=(const cArrayBase& copy)
    {
      if(this == &copy) return *this;
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      if(_array!=0) Alloc::deallocate(_array);
      _size=copy._size;
      _array=Alloc::allocate(_size);
      //memcpy(_array,copy._array,_size*sizeof(T));
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T(copy._array[i]);
      return *this;
    }
    cArrayBase& operator=(cArrayBase&& mov)
    {
      if(this == &mov) return *this;
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      if(_array!=0) Alloc::deallocate(_array);
      _array=mov._array;
      _size=mov._size;
      mov._array=0;
      mov._size=0;
      return *this;
    }
    cArrayBase& operator +=(const cArrayBase& add)
    {
      SizeType nsize=_size+add._size;
      T* narray = Alloc::allocate(nsize);
      memcpy(narray, _array, _size*sizeof(T));
      //memcpy(narray+_size,add._array,add._size*sizeof(T));

      for(SizeType i = _size; i < nsize; ++i)
        new (narray+i) T(add._array[i-_size]);

      if(_array!=0) Alloc::deallocate(_array); // we do this down here so doing += with yourself doesn't break.
      _array=narray;
      _size=nsize;
      return *this;
    }
    BSS_FORCEINLINE cArrayBase operator +(const cArrayBase& add) const
    {
      cArrayBase retval(*this);
      retval+=add;
      return retval;
    }
    void BSS_FASTCALL Insert(T item, SizeType location)
    {
      SizeType nsize=_size+1;
      if(location==_size)
      {
        _array = (T*)Alloc::allocate(_size = nsize, _array);
        assert(_array!=0); // Don't bother checking for null case becuase we'd have to explode anyway.
        new (_array+location) T(item);
        return;
      }
      assert(location<_size);

      T* narray = (T*)Alloc::allocate(nsize);
      assert(narray!=0);
      memcpy(narray, _array, location*sizeof(T)); // array will never be zero here, because if _size was 0, the only valid location is also 0, which triggers an add.
      new (narray+location) T(item);
      memcpy(narray+location+1, _array+location, (_size-location)*sizeof(T));
      if(_array!=0) Alloc::deallocate(_array);
      _array=narray;
      _size=nsize;
    }
    inline void SetArray(const T* a, SizeType n, SizeType s=0) { SetSize(n+s); memcpy(_array+s, a, n*sizeof(T)); }

  protected:
    template<typename U>
    inline void _pushback(SizeType index, SizeType length, U && data)
    {
      (_array+(index+length))->~T();
      memmove(_array+(index+1), _array+index, length*sizeof(T));
      new (_array+index) T(std::forward<U>(data));
    }
    BSS_FORCEINLINE void _mvarray(SizeType begin, SizeType end, SizeType length)
    {
      memmove(_array+begin, _array+end, length*sizeof(T));
    }

    T* _array;
    SizeType _size;

    typedef SizeType ST_;
    typedef T T_;
  };

  // Typesafe array that reconstructs everything properly, without any memory moving tricks
  template<class T, typename SizeType, typename Alloc>
  class BSS_COMPILER_DLLEXPORT cArrayBase<T, SizeType, CARRAY_SAFE, Alloc>
  {
    static_assert(std::is_integral<SizeType>::value, "SizeType must be integral");
  public:
    inline cArrayBase(const cArrayBase& copy) : _array(Alloc::allocate(copy._size)), _size(copy._size)
    {
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T(copy._array[i]);
    }
    inline cArrayBase(cArrayBase&& mov) : _array(mov._array), _size(mov._size)
    {
      mov._array=0;
      mov._size=0;
    }
    inline explicit cArrayBase(SizeType size) : _array(Alloc::allocate(size)), _size(size)
    {
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T();
    }
    inline cArrayBase(const std::initializer_list<T> list) : _array((T*)Alloc::allocate(list.size())), _size(list.size())
    {
      auto end = list.end();
      int c = 0;
      for(auto i = list.begin(); i != end && c < _size; ++i) new (_array+(c++)) T(*i); // Use copy constructors to initialize array elements
      while(c < _size) new (_array+(c++)) T(); // If you didn't pass in enough parameters, ensure the array is still initialized
    }
    inline ~cArrayBase()
    {
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      if(_array!=0)
        Alloc::deallocate(_array);
    }
    inline SizeType Size() const { return _size; }
    void BSS_FASTCALL SetSize(SizeType nsize)
    {
      if(nsize==_size) return;
      T* narray = Alloc::allocate(nsize);

      SizeType smax = _size<nsize?_size:nsize;
      for(SizeType i = 0; i < smax; ++i) //copy over any we aren't discarding
        new (narray+i) T(std::move(_array[i])); //We're going to be deleting the old ones so use move semantics if possible
      for(SizeType i = smax; i < nsize; ++i) //Initialize any newcomers
        new (narray+i) T();
      for(SizeType i = 0; i < _size; ++i) //Demolish the old ones
        (_array+i)->~T();

      if(_array!=0) Alloc::deallocate(_array);
      _array=narray;
      _size=nsize;
    }
    void BSS_FASTCALL RemoveInternal(SizeType index)
    {
      --_size; // Note that this _size decrease is reversed at the end of this function, so _size doesn't actually change, matching the behavior of cArrayBase
      for(SizeType i=index; i<_size; ++i)
        _array[i]=std::move(_array[i+1]);
      _array[_size].~T();
      new(_array+(_size++)) T();
    }
    //inline operator T*() { return _array; }
    //inline operator const T*() const { return _array; }
    cArrayBase& operator=(const cArrayBase& copy)
    {
      if(this == &copy) return *this;
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      if(_array!=0) Alloc::deallocate(_array);
      _size=copy._size;
      _array=Alloc::allocate(_size);
      for(SizeType i = 0; i < _size; ++i)
        new (_array+i) T(copy._array[i]);
      return *this;
    }
    cArrayBase& operator=(cArrayBase&& mov)
    {
      if(this == &mov) return *this;
      for(SizeType i = 0; i < _size; ++i)
        (_array+i)->~T();
      if(_array!=0) Alloc::deallocate(_array);
      _array=mov._array;
      _size=mov._size;
      mov._array=0;
      mov._size=0;
      return *this;
    }
    cArrayBase& operator +=(const cArrayBase& add)
    {
      SizeType nsize=_size+add._size;
      T* narray = Alloc::allocate(nsize);
      assert(narray!=0);

      for(SizeType i = 0; i < _size; ++i) //copy over old ones
        new (narray+i) T(std::move(_array[i])); //We're going to delete the old ones so use move semantics if possible
      for(SizeType i = _size; i < nsize; ++i) //Copy over newcomers
        new (narray+i) T(add._array[i-_size]);
      for(SizeType i = 0; i < _size; ++i) //Demolish the old ones
        (_array+i)->~T();

      if(_array!=0) Alloc::deallocate(_array);
      _array=narray;
      _size=nsize;
      return *this;
    }
    BSS_FORCEINLINE cArrayBase operator +(const cArrayBase& add) const
    {
      cArrayBase retval(*this);
      retval+=add;
      return retval;
    }
    inline void BSS_FASTCALL Insert(T item, SizeType location)
    {
      SetSize(_size+1);
      _pushback(location, _size-location-1, item);
    }
    inline void SetArray(const T* a, SizeType n, SizeType s=0) { SetSize(n+s); for(SizeType i=s; i<n; ++i) _array[i]=a[i]; }

  protected:
    template<typename U>
    inline void _pushback(SizeType index, SizeType length, U && data)
    {
      for(SizeType i=index+length; i>index; --i)
        _array[i]=std::move(_array[i-1]);
      _array[index] = std::forward<U>(data);
    }
    inline void _mvarray(SizeType begin, SizeType end, SizeType length)
    {
      if(begin>end)
      {
        for(SizeType i=0; i<length; ++i)
          _array[end+i]=std::move(_array[begin+i]);
      } else
      {
        for(SizeType i=length; i-->0;)
          _array[end+i]=std::move(_array[begin+i]);
      }
    }

    T* _array;
    SizeType _size;

    typedef SizeType ST_;
    typedef T T_;
  };

  // Wrapper for underlying arrays that expose the array, making them independently usable without blowing up everything that inherits them
  template<class T, typename SizeType=unsigned int, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cArray : public cArrayBase<T, SizeType, ArrayType, Alloc>
  {
  protected:
    typedef cArrayBase<T, SizeType, ArrayType, Alloc> AT_;
    typedef typename AT_::ST_ ST_;
    typedef typename AT_::T_ T_;
    using AT_::_array;
    using AT_::_size;

  public:
    inline cArray(const cArray& copy) : AT_(copy) {} // We have to declare this because otherwise its interpreted as deleted
    inline cArray(cArray&& mov) : AT_(std::move(mov)) {}
    inline cArray(const std::initializer_list<T> list) : AT_(list) {}
    inline explicit cArray(ST_ size=0) : AT_(size) {}

    //inline void Add(T item) { AT_::Insert(item,_size); } // Not all cArrays implement Insert
    //Implementation of RemoveInternal that adjusts the size of the array.
    BSS_FORCEINLINE void Remove(ST_ index) { AT_::RemoveInternal(index); AT_::SetSize(_size-1); }
    inline const T_& Front() const { assert(_size>0); return _array[0]; }
    inline T_& Front() { assert(_size>0); return _array[0]; }
    inline const T_& Back() const { assert(_size>0); return _array[_size-1]; }
    inline T_& Back() { assert(_size>0); return _array[_size-1]; }
    BSS_FORCEINLINE operator T_*() { return _array; }
    BSS_FORCEINLINE operator const T_*() const { return _array; }
    inline const T_* begin() const { return _array; }
    inline const T_* end() const { return _array+_size; }
    inline T_* begin() { return _array; }
    inline T_* end() { return _array+_size; }

    BSS_FORCEINLINE cArray& operator=(const cArray& copy) { AT_::operator=(copy); return *this; }
    BSS_FORCEINLINE cArray& operator=(const AT_& copy) { AT_::operator=(copy); return *this; }
    BSS_FORCEINLINE cArray& operator=(AT_&& mov) { AT_::operator=(std::move(mov)); return *this; }
    BSS_FORCEINLINE cArray& operator +=(const AT_& add) { AT_::operator+=(add); return *this; }
    BSS_FORCEINLINE cArray operator +(const AT_& add) const { cArray r(*this); return (r+=add); }
  };
}

#endif
