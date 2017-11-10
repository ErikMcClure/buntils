// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_QUEUE_H__
#define __BSS_QUEUE_H__

#include "ArrayCircular.h"

namespace bss {
  // Fast, tiny circular array-based queue. Pop and Peek are only valid if there is an item in the stack; this check must be done by the user.
  template<class T, typename CType = ptrdiff_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StandardAllocator<T>>
  class BSS_COMPILER_DLLEXPORT Queue : protected ArrayCircular<T, CType, ArrayType, Alloc>
  {
  protected:
    typedef ArrayCircular<T, CType, ArrayType, Alloc> BASE;
    using BASE::_length;

  public:
    inline Queue(const Queue& copy) = default;
    inline Queue(Queue&& mov) = default;
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline Queue(CType init, typename Alloc::policy_type* policy) : BASE(init, policy) {}
    inline explicit Queue(CType init = 0) : BASE(init) {}
    inline ~Queue() {}
    // Pushes a value into the queue in FIFO order.
    BSS_FORCEINLINE void Push(const T& value) { _push<const T&>(value); }
    BSS_FORCEINLINE void Push(T&& value) { _push<T&&>(std::move(value)); }
    // Pops a value out of the queue in FIFO order. If there is no value to pop, calling this is illegal and will crash.
    BSS_FORCEINLINE T Pop() { assert(_length != 0); return BASE::PopBack(); }
    // Peeks at the value at the top of the queue but does not remove it. If there is no value, this is an illegal call.
    BSS_FORCEINLINE T& Peek() { assert(_length != 0); return BASE::Back(); }
    BSS_FORCEINLINE const T& Peek() const { assert(_length != 0); return BASE::Back(); }
    BSS_FORCEINLINE void Discard() { --_length; }
    // Returns true if there are no values in the queue (indicating that Pop() and Peek() cannot be called)
    BSS_FORCEINLINE bool Empty() { return !_length; }
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    // Returns the underlying capacity of the circular array
    BSS_FORCEINLINE CType Capacity() const { return BASE::Capacity(); }
    // Returns how many items are currently in the queue. Calling Pop or Peek when this is 0 is illegal.
    BSS_FORCEINLINE CType Length() const { return _length; }

    inline Queue& operator=(const Queue& copy) = default;
    inline Queue& operator=(Queue&& mov) = default;

  protected:
    template<typename U>
    void _push(U && value) 
    { 
      if(_length >= BASE::_capacity)
        BASE::SetCapacity(T_FBNEXT(_length));
      BASE::_push(std::forward<U>(value)); 
    }
  };
}

#endif
