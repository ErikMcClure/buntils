// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_QUEUE_H__
#define __BUN_QUEUE_H__

#include "ArrayCircular.h"

namespace bun {
  // Fast, tiny circular array-based queue. Pop and Peek are only valid if there is an item in the stack; this check must be done by the user.
  template<class T, typename CType = ptrdiff_t, typename Alloc = StandardAllocator<T>>
  class BUN_COMPILER_DLLEXPORT Queue : protected ArrayCircular<T, CType, Alloc>
  {
  protected:
    using BASE = ArrayCircular<T, CType, Alloc>;
    using BASE::_length;

  public:
    inline Queue(const Queue& copy) = default;
    inline Queue(Queue&& mov) = default;
    inline Queue(CType init, const Alloc& alloc) : BASE(init, alloc) {}
    inline explicit Queue(CType init = 0) : BASE(init) {}
    inline ~Queue() {}
    // Pushes a value into the queue in FIFO order.
    BUN_FORCEINLINE void Push(const T& value) { _push<const T&>(value); }
    BUN_FORCEINLINE void Push(T&& value) { _push<T&&>(std::move(value)); }
    // Pops a value out of the queue in FIFO order. If there is no value to pop, calling this is illegal and will crash.
    BUN_FORCEINLINE T Pop() { assert(_length != 0); return BASE::PopBack(); }
    // Peeks at the value at the top of the queue but does not remove it. If there is no value, this is an illegal call.
    BUN_FORCEINLINE T& Peek() { assert(_length != 0); return BASE::Back(); }
    BUN_FORCEINLINE const T& Peek() const { assert(_length != 0); return BASE::Back(); }
    BUN_FORCEINLINE void Discard() { --_length; }
    // Returns true if there are no values in the queue (indicating that Pop() and Peek() cannot be called)
    BUN_FORCEINLINE bool Empty() { return !_length; }
    BUN_FORCEINLINE void Clear() { BASE::Clear(); }
    // Returns the underlying capacity of the circular array
    BUN_FORCEINLINE CType Capacity() const { return BASE::Capacity(); }
    // Returns how many items are currently in the queue. Calling Pop or Peek when this is 0 is illegal.
    BUN_FORCEINLINE CType size() const { return _length; }

    inline Queue& operator=(const Queue& copy) = default;
    inline Queue& operator=(Queue&& mov) = default;

    using BASE::SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { BASE::template Serialize<Engine>(s, id); }

  protected:
    template<typename U>
    void _push(U && value) 
    { 
      if(_length >= Capacity())
        BASE::SetCapacity(T_FBNEXT(_length));
      BASE::_push(std::forward<U>(value)); 
    }
  };
}

#endif
