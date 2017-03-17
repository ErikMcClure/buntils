// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BSS_QUEUE_H__
#define __C_BSS_QUEUE_H__

#include "cArrayCircular.h"

namespace bss_util {
  // Fast, tiny circular array-based queue. Pop and Peek are only valid if there is an item in the stack; this check must be done by the user.
  template<class T, typename CType = ptrdiff_t, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cQueue : protected cArrayCircular<T, CType, ArrayType, Alloc>
  {
  protected:
    typedef cArrayCircular<T, CType, ArrayType, Alloc> BASE;
    using BASE::_length;

  public:
    inline cQueue(const cQueue& copy) : BASE(copy) {}
    inline cQueue(cQueue&& mov) : BASE(std::move(mov)) {}
    inline explicit cQueue(CType init=0) : BASE(init) {}
    inline ~cQueue() {}
    // Pushes a value into the queue in FIFO order.
    BSS_FORCEINLINE void Push(const T& value) { _push<const T&>(value); }
    BSS_FORCEINLINE void Push(T&& value) { _push<T&&>(std::move(value)); }
    // Pops a value out of the queue in FIFO order. If there is no value to pop, calling this is illegal and will crash.
    BSS_FORCEINLINE T Pop() { assert(_length!=0); return BASE::PopBack(); }
    // Peeks at the value at the top of the queue but does not remove it. If there is no value, this is an illegal call.
    BSS_FORCEINLINE T& Peek() { assert(_length!=0); return BASE::Back(); }
    BSS_FORCEINLINE const T& Peek() const { assert(_length!=0); return BASE::Back(); }
    BSS_FORCEINLINE void Discard() { --_length; }
    // Returns true if there are no values in the queue (indicating that Pop() and Peek() cannot be called)
    BSS_FORCEINLINE bool Empty() { return !_length; }
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    // Returns the underlying capacity of the circular array
    BSS_FORCEINLINE CType Capacity() const { return BASE::Capacity(); }
    // Returns how many items are currently in the queue. Calling Pop or Peek when this is 0 is illegal.
    BSS_FORCEINLINE CType Length() const { return _length; }

    inline cQueue& operator=(const cQueue& copy) { BASE::operator=(copy); return *this; }
    inline cQueue& operator=(cQueue&& mov) { BASE::operator=(std::move(mov)); return *this; }

  protected:
    template<typename U>
    void _push(U && value) { if(_length>=BASE::_capacity) BASE::SetCapacity(T_FBNEXT(_length)); BASE::_push(std::forward<U>(value)); }
  };
}

#endif
