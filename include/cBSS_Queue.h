// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BSS_QUEUE_H__
#define __C_BSS_QUEUE_H__

#include "cArrayCircular.h"

namespace bss_util {
  // Fast, tiny circular array-based queue. Pop and Peek are only valid if there is an item in the stack; this check must be done by the user.
  template<class T, typename SizeType=int, typename ArrayType=cArraySimple<T,SizeType>>
  class BSS_COMPILER_DLLEXPORT cBSS_Queue : protected cArrayCircular<T,SizeType,ArrayType>
  {
  protected:
    typedef cArrayCircular<T,SizeType,ArrayType> BASE;
    using BASE::_length;

  public:
    inline cBSS_Queue(const cBSS_Queue& copy) : BASE(copy) {}
    inline cBSS_Queue(cBSS_Queue&& mov) : BASE(std::move(mov)) {}
    inline explicit cBSS_Queue(SizeType init=8) : BASE(0) {}
    inline ~cBSS_Queue() {}
    BSS_FORCEINLINE void BSS_FASTCALL Push(const T& value) { _push<const T&>(value); }
    BSS_FORCEINLINE void BSS_FASTCALL Push(T&& value) { _push<T&&>(std::move(value)); }
    BSS_FORCEINLINE T BSS_FASTCALL Pop() { assert(_length!=0); return BASE::PopBack(); }
    BSS_FORCEINLINE T& BSS_FASTCALL Peek() { assert(_length!=0); return BASE::Back(); }
    BSS_FORCEINLINE const T& BSS_FASTCALL Peek() const { assert(_length!=0); return BASE::Back(); }
    BSS_FORCEINLINE void Discard() { --_length; }
    BSS_FORCEINLINE bool Empty() { return !_length; }
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    BSS_FORCEINLINE SizeType Capacity() const { return BASE::Capacity(); }
    BSS_FORCEINLINE SizeType Length() const { return _length; }

    inline cBSS_Queue& operator=(const cBSS_Queue& copy) { BASE::operator=(copy); return *this; }
    inline cBSS_Queue& operator=(cBSS_Queue&& mov) { BASE::operator=(std::move(mov)); return *this; }

  protected:
    template<typename U>
    void BSS_FASTCALL _push(U && value) { if(_length>=BASE::_size) BASE::SetSize(fbnext(_length)); BASE::_push<U>(std::forward<U>(value)); }
  };

}

#endif
