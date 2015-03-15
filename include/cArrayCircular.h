// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_CIRCULAR_H__BSS__
#define __C_ARRAY_CIRCULAR_H__BSS__

#include "cArray.h"
#include "bss_util.h"

namespace bss_util {
  // Simple circular array implementation. Unlike most data structures, SizeType must be signed instead of unsigned
  template<class T, typename SizeType=int, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cArrayCircular : protected cArrayBase<T, SizeType, ArrayType, Alloc>
  {
  protected:
    typedef SizeType ST_;
    typedef cArrayBase<T, SizeType, ArrayType, Alloc> AT_;
    using AT_::_size;
    using AT_::_array;
    static_assert(std::is_signed<SizeType>::value, "SizeType must be signed");

  public:
    // Constructors
    inline cArrayCircular(const cArrayCircular& copy) : AT_(copy), _cur(copy._cur), _length(copy._length) {}
    inline cArrayCircular(cArrayCircular&& mov) : AT_(std::move(mov)), _cur(mov._cur), _length(mov._length) {}
    inline explicit cArrayCircular(ST_ size=0) : AT_(size), _cur(-1), _length(0) {}
    inline ~cArrayCircular() {}
    BSS_FORCEINLINE void Push(const T& item) { _push<const T&>(item); }
    BSS_FORCEINLINE void Push(T&& item) { _push<T&&>(std::move(item)); }
    inline T Pop() { assert(_length>0); --_length; ST_ prev=_cur; _cur=bssmod<ST_>(_cur-1, _size); return std::move(_array[prev]); }
    inline T PopBack() { assert(_length>0); return std::move(_array[_modindex(--_length)]); }
    inline const T& Front() const { assert(_length>0); return _array[_cur]; }
    inline T& Front() { assert(_length>0); return _array[_cur]; }
    inline const T& Back() const { assert(_length>0); return _array[_modindex(_length-1)]; }
    inline T& Back() { assert(_length>0); return _array[_modindex(_length-1)]; }
    inline ST_ Capacity() const { return _size; }
    inline ST_ Length() const { return _length; }
    inline void Clear() { _length=0; _cur=-1; }
    inline void SetSize(ST_ nsize) // Will preserve the array but only if it got larger
    {
      ST_ sz=_size;
      AT_::SetSize(nsize);
      ST_ c=_cur+1; // We don't want to normalize this becuase its important it equals sz if _cur=sz-1
      if(sz<nsize)
        AT_::_mvarray(c, c+_size-sz, sz-c);
    }
    BSS_FORCEINLINE T& operator[](ST_ index) { return _array[_modindex(index)]; } // an index of 0 is the most recent item pushed into the circular array.
    BSS_FORCEINLINE const T& operator[](ST_ index) const { return _array[_modindex(index)]; }
    inline cArrayCircular& operator=(const cArrayCircular& right) { AT_::operator=(right); _cur=right._cur; _length=right._length; return *this; }
    inline cArrayCircular& operator=(cArrayCircular&& right) { AT_::operator=(std::move(right)); _cur=right._cur; _length=right._length; right._length=0; right._cur=0; return *this; }

  protected:
    template<typename U> // Note that _cur+1 can never be negative so we don't need to use bssmod there
    inline void _push(U && item) { _array[_cur=((_cur+1)%_size)]=std::forward<U>(item); _length+=(_length<_size); }
    BSS_FORCEINLINE ST_ _modindex(ST_ index) { return bssmod<ST_>(_cur-index, _size); }

    ST_ _cur;
    ST_ _length;
  };
}

#endif
