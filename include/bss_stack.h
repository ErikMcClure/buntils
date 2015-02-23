// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BSS_STACK_H__
#define __C_BSS_STACK_H__

#include "cArray.h"

namespace bss_util {
  // Fast, tiny array-based stack. Pop and Top are only valid if there is an item in the stack; this check must be done by the user.
  template<class T, typename SizeType=unsigned int, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cStack : protected cArrayInternal<T, SizeType, ArrayType, Alloc>
  {
  protected:
    typedef cArrayInternal<T, SizeType, ArrayType, Alloc> AT_;
    using AT_::_array;
    using AT_::_size;

  public:
    inline cStack(const cStack& copy) : AT_(copy), _length(copy._length) {}
    inline cStack(cStack&& mov) : AT_(std::move(mov)), _length(mov._length) {}
    inline explicit cStack(SizeType init=0) : AT_(init), _length(0) {}
    inline ~cStack() {}
    BSS_FORCEINLINE void BSS_FASTCALL Push(const T& value) { _push(value); }
    BSS_FORCEINLINE void BSS_FASTCALL Push(T&& value) { _push(std::move(value)); }
    BSS_FORCEINLINE T BSS_FASTCALL Pop() { assert(_length!=0); return std::move(_array[--_length]); }
    BSS_FORCEINLINE T& BSS_FASTCALL Peek() { assert(_length!=0); return _array[_length-1]; }
    BSS_FORCEINLINE const T& BSS_FASTCALL Peek() const { assert(_length!=0); return _array[_length-1]; }
    BSS_FORCEINLINE void Discard() { --_length; }
    BSS_FORCEINLINE void Clear() { _length=0; }
    BSS_FORCEINLINE SizeType Length() const { return _length; }
    BSS_FORCEINLINE void SetLength(SizeType length) { _length = length; }
    BSS_FORCEINLINE SizeType Capacity() const { return _size; }
    inline cStack& operator=(const cStack& copy) { AT_::operator=(copy); _length=copy._length; return *this; }
    inline cStack& operator=(cStack&& mov) { AT_::operator=(std::move(mov)); _length=mov._length; return *this; }
    inline const T& operator[](SizeType i) const { return _array[i]; }
    inline T& operator[](SizeType i) { return _array[i]; }

  protected:
    template<typename U>
    void BSS_FASTCALL _push(U && value)
    {
      if(++_length>_size)
        AT_::SetSize(T_FBNEXT(_size));
      assert(_length-1<_size);
      _array[_length-1]=std::forward<U>(value);
    }

    SizeType _length;
  };
}

#endif
