// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_CIRCULAR_H__BSS__
#define __C_ARRAY_CIRCULAR_H__BSS__

#include "cArray.h"
#include "bss_util.h"

namespace bss_util {
  // Simple circular array implementation. Unlike most data structures, CType must be signed instead of unsigned
  template<class T, typename CType = ptrdiff_t, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cArrayCircular : protected cArrayBase<T, CType, Alloc>, protected cArrayInternal<T, CType, ArrayType, Alloc>
  {
  protected:
    typedef CType CT_;
    typedef cArrayInternal<T, CType, ArrayType, Alloc> BASE;
    typedef cArrayBase<T, CType, Alloc> AT_;
    using AT_::_capacity;
    using AT_::_array;
    static_assert(std::is_signed<CType>::value, "CType must be signed");

  public:
    // Constructors
    inline cArrayCircular(const cArrayCircular& copy) : AT_(0), _cur(-1), _length(0) { operator=(copy); }
    inline cArrayCircular(cArrayCircular&& mov) : AT_(std::move(mov)), _cur(mov._cur), _length(mov._length) { mov._cur = -1; mov._length = 0; }
    inline explicit cArrayCircular(CType size=0) : AT_(size), _cur(-1), _length(0) {}
    inline ~cArrayCircular() { Clear(); }
    BSS_FORCEINLINE void Push(const T& item) { _push<const T&>(item); }
    BSS_FORCEINLINE void Push(T&& item) { _push<T&&>(std::move(item)); }
    inline T Pop()
    { 
      assert(_length>0);
      --_length;
      T r(std::move(_array[_cur]));
      _array[_cur].~T();
      _cur=bssmod<CType>(_cur-1, _capacity);
      return r;
    }
    inline T PopBack()
    { 
      assert(_length>0);
      CType l = _modindex(--_length);
      T r(std::move(_array[l]));
      _array[l].~T();
      return r;
    }
    inline const T& Front() const { assert(_length>0); return _array[_cur]; }
    inline T& Front() { assert(_length>0); return _array[_cur]; }
    inline const T& Back() const { assert(_length>0); return _array[_modindex(_length-1)]; }
    inline T& Back() { assert(_length>0); return _array[_modindex(_length-1)]; }
    inline CType Capacity() const { return _capacity; }
    inline CType Length() const { return _length; }
    inline void Clear()
    { 
      if(_length == _capacity) // Dump the whole thing
        BASE::_setlength(_array, _length, 0);
      else if(_cur - _length >= -1) // The current used chunk is contiguous
        BASE::_setlength(_array + _cur - _length + 1, _length, 0);
      else // We have two seperate chunks that must be dealt with
      {
        CType i = _modindex(_length - 1);
        BASE::_setlength(_array + i, _capacity - i, 0);
        BASE::_setlength(_array, _cur + 1, 0); // We can only have two seperate chunks if it crosses over the 0 mark at some point, so this always starts at 0
      }

      _length=0;
      _cur=-1;
    }
    inline void SetCapacity(CType nsize) // Will preserve the array but only if it got larger
    {
      if(nsize < _capacity)
      {
        Clear(); // Getting the right destructors here is complicated and trying to preserve the array when it's shrinking is meaningless.
        AT_::SetCapacityDiscard(nsize);
      }
      else if(nsize > _capacity)
      {
        T* n = AT_::_getalloc(nsize);
        if(_cur - _length >= -1) // If true the chunk is contiguous
          BASE::_copymove(n + _cur - _length + 1, _array + _cur - _length + 1, _length);
        else
        {
          CType i = _modindex(_length - 1);
          BASE::_copymove(n + bssmod<CType>(_cur - _length + 1, nsize), _array + i, _capacity - i);
          BASE::_copymove(n, _array, _cur + 1);
        }
        AT_::_free(_array);
        _array = n;
        _capacity = nsize;
      }
    }
    BSS_FORCEINLINE T& operator[](CType index) { return _array[_modindex(index)]; } // an index of 0 is the most recent item pushed into the circular array.
    BSS_FORCEINLINE const T& operator[](CType index) const { return _array[_modindex(index)]; }
    inline cArrayCircular& operator=(const cArrayCircular& right)
    {
      Clear();
      AT_::SetCapacityDiscard(right._capacity);
      _cur = right._cur;
      _length = right._length;

      if(_length == _capacity) // Copy the whole thing
        BASE::_copy(_array, right._array, _length);
      else if(_cur - _length >= -1) // The current used chunk is contiguous
        BASE::_copy(_array + _cur - _length + 1, right._array + _cur - _length + 1, _length);
      else // We have two seperate chunks that must be dealt with
      {
        CType i = _modindex(_length - 1);
        BASE::_copy(_array + i, right._array + i, _capacity - i);
        BASE::_copy(_array, right._array, _cur + 1);
      }

      return *this;
    }
    inline cArrayCircular& operator=(cArrayCircular&& right)
    { 
      Clear();
      AT_::operator=(std::move(right));
      _cur = right._cur;
      _length = right._length;
      right._length = 0;
      right._cur = -1;
      return *this; 
    }

  protected:
    template<typename U> // Note that _cur+1 can never be negative so we don't need to use bssmod
    inline void _push(U && item)
    { 
      _cur = ((_cur + 1) % _capacity);
      if(_length < _capacity)
      {
        new(_array + _cur) T(std::forward<U>(item));
        ++_length;
      }
      else
        _array[_cur] = std::forward<U>(item);
    }
    BSS_FORCEINLINE CType _modindex(CType index) { return bssmod<CType>(_cur-index, _capacity); }

    CType _cur;
    CType _length;
  };
}

#endif
