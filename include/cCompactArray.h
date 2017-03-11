// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_COMPACT_ARRAY_H__BSS__
#define __C_COMPACT_ARRAY_H__BSS__

#include "cArray.h"
#include "bss_util.h"

namespace bss_util {
  // Array that uses a inline stack for smaller arrays before allocating something on the heap. Can only be used with simple data.
  template<class T, int I = 2, class CT = size_t, typename Alloc = StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cCompactArray
  {
    static_assert(std::is_unsigned<CT>::value, "CT must be unsigned");
    static const CT COMPACTMASK = (((CT)(~0)) >> 1);
    static const CT COMPACTFLAG = ~(((CT)(~0)) >> 1);

  public:
    inline cCompactArray(const cCompactArray& copy) : _capacity(0), _length(COMPACTFLAG)
    {
      SetLength(copy._length&COMPACTMASK);
      MEMCPY(begin(), (_length&COMPACTMASK) * sizeof(T), copy.begin(), (copy._length&COMPACTMASK)*sizeof(T));
    }
    inline cCompactArray(cCompactArray&& mov) : _array(0), _capacity(mov._capacity), _length(mov._length)
    {
      if(_length&COMPACTFLAG)
        MEMCPY(_internal, I * sizeof(T), mov._internal, (mov._length&COMPACTMASK) * sizeof(T));
      else
        _array = mov._array;

      mov._length = COMPACTFLAG;
    }
    inline cCompactArray(const cArraySlice<const T, CT>& slice) : _capacity(0), _length(COMPACTFLAG)
    {
      SetLength(slice.length);
      MEMCPY(begin(), (_length&COMPACTMASK) * sizeof(T), slice.start, slice.length * sizeof(T));
    }
    inline cCompactArray() : _array(0), _capacity(0), _length(COMPACTFLAG) {}
    inline cCompactArray(const std::initializer_list<T>& list) : _capacity(0), _length(COMPACTFLAG)
    {
      SetLength(list.size());
      auto end = list.end();
      CT j = 0;
      CT l = Length();
      for(auto i = list.begin(); i != end && j < l; ++i)
        new(_array + (j++)) T(*i);
    }
    inline ~cCompactArray() { if(!(_length&COMPACTFLAG) && _array != 0) Alloc::deallocate(_array); }
    inline CT Add(T t) { Reserve((_length&COMPACTMASK) + 1); new(end()) T(t); return (_length++)&COMPACTMASK; }
    inline void Remove(CT index) { RemoveRangeSimple<T, CT>(begin(), (_length--)&COMPACTMASK, index, 1); }
    BSS_FORCEINLINE void RemoveLast() { Remove(Length() - 1); }
    BSS_FORCEINLINE void Insert(T t, CT index = 0)
    {
      Reserve((_length&COMPACTMASK) + 1);
      InsertRangeSimple<T, CT>(begin(), (_length++)&COMPACTMASK, index, &t, 1);
    }
    BSS_FORCEINLINE void Set(const cArraySlice<const T, CT>& slice) { Set(slice.start, slice.length); }
    BSS_FORCEINLINE void Set(const T* p, CT n)
    {
      SetLength(n);
      MEMCPY(begin(), (_length&COMPACTMASK) * sizeof(T), p, n*sizeof(T));
    }
    BSS_FORCEINLINE bool Empty() const { return !(_length&COMPACTMASK); }
    BSS_FORCEINLINE void Clear() { _length = (_length&COMPACTFLAG); }
    inline void SetLength(CT length)
    {
      Reserve(length);
#ifdef BSS_DEBUG
      if(length > Length())
        memset(end(), 0xfd, (length - Length()) * sizeof(T));
#endif
      _length = (length | (_length&COMPACTFLAG));
    }
    inline void Reserve(CT capacity)
    {
      if(capacity > I)
      {
        capacity = T_FBNEXT(capacity);
        if(_length&COMPACTFLAG)
        {
          T* a = Alloc::allocate(capacity, 0);
          _length = (_length&COMPACTMASK);
          if(_length)
            MEMCPY(a, capacity * sizeof(T), _internal, _length*sizeof(T));
          _array = a;
          _capacity = capacity;
        }
        else
        {
          _array = Alloc::allocate(capacity, _array);
          _capacity = capacity;
        }
      }
    }
    BSS_FORCEINLINE CT Length() const { return _length&COMPACTMASK; }
    BSS_FORCEINLINE CT Capacity() const { return (_length&COMPACTFLAG)?I:_capacity; }
    BSS_FORCEINLINE const T& Front() const { assert(Length()>0); return begin()[0]; }
    BSS_FORCEINLINE const T& Back() const { assert(Length()>0); return begin()[Length() - 1]; }
    BSS_FORCEINLINE T& Front() { assert(Length()>0); return begin()[0]; }
    BSS_FORCEINLINE T& Back() { assert(Length()>0); return begin()[Length() - 1]; }
    BSS_FORCEINLINE const T* begin() const noexcept { return (_length&COMPACTFLAG) ? _internal : _array; }
    BSS_FORCEINLINE const T* end() const noexcept { return begin() + Length(); }
    BSS_FORCEINLINE T* begin() noexcept { return (_length&COMPACTFLAG) ? _internal : _array; }
    BSS_FORCEINLINE T* end() noexcept { return begin() + Length(); }
    BSS_FORCEINLINE cArraySlice<T, CT> GetSlice() const noexcept { return cArraySlice<T, CT>(begin(), Length()); }
    BSS_FORCEINLINE operator T*() { return begin(); }
    BSS_FORCEINLINE operator const T*() const { return begin(); }
    inline cCompactArray& operator=(const cCompactArray& copy)
    {
      SetLength(copy._length&COMPACTMASK);
      MEMCPY(begin(), (_length&COMPACTMASK) * sizeof(T), copy.begin(), (copy._length&COMPACTMASK) * sizeof(T));
      return *this;
    }
    inline cCompactArray& operator=(cCompactArray&& mov)
    {
      if(_length&COMPACTFLAG)
        MEMCPY(_internal, I * sizeof(T), mov._internal, (mov._length&COMPACTMASK) * sizeof(T));
      else
        _array = mov._array;

      mov._length = COMPACTFLAG;
      return *this;
    }
    inline cCompactArray& operator=(const cArraySlice<const T, CT>& copy) { Set(copy); return *this; }

  protected:
    union
    {
      struct {
        T* _array;
        CT _capacity;
      };
      struct {
        T _internal[I];
      };
    };
    
    CT _length;
  };
}

#endif
