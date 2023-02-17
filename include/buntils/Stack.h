// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_STACK_H__
#define __BUN_STACK_H__

#include "DynArray.h"

namespace bun {
  // Fast, tiny array-based stack. Pop and Top are only valid if there is an item in the stack; this check must be done by the user.
  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StandardAllocator<T>>
  class BUN_COMPILER_DLLEXPORT Stack
  {
  public:
    inline Stack(const Stack& copy) : _array(copy) {}
    inline Stack(Stack&& mov) : _array(std::move(mov)) {}
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline Stack(CType init, typename Alloc::policy_type* policy) : _array(init, policy) {}
    inline explicit Stack(CType init = 0) : _array(init) {}
    inline ~Stack() {}
    // Pushes an item on to the stack in LIFO order.
    BUN_FORCEINLINE void Push(const T& value) { _array.Add(value); }
    BUN_FORCEINLINE void Push(T&& value) { _array.Add(std::move(value)); }
    // Pops an item off the stack in LIFO order. If there is no item to pop, this call is illegal and will crash.
    BUN_FORCEINLINE T Pop() { T r(std::move(_array.Back())); _array.RemoveLast(); return std::move(r); }
    // Peeks at the item on top of the stack, but does not remove it. If there is no item to look at, this is illegal.
    BUN_FORCEINLINE T& Peek() { return _array.Back(); }
    BUN_FORCEINLINE const T& Peek() const { return _array.Back(); }
    // Throws away the item on top of the stack.
    BUN_FORCEINLINE void Discard() { _array.RemoveLast(); }
    BUN_FORCEINLINE void Clear() { _array.Clear(); }
    // Gets how many items are on the stack. If this is 0, Push and Pop cannot be called.
    BUN_FORCEINLINE CType Length() const { return _array.Length(); }
    // Sets how many items are on the stack. Is usually used to truncate the stack, but can also be used to extend it. The stack
    // will follow the same rules that govern extending it's base array, so in a simple array, the values will be uninitialized.
    BUN_FORCEINLINE void SetLength(CType length) { _array.SetLength(length); }
    // Gets the capacity of the underlying array.
    BUN_FORCEINLINE CType Capacity() const { return _array.Capacity(); }
    inline Stack& operator=(const Stack& copy) { _array = copy._array; return *this; }
    inline Stack& operator=(Stack&& mov) { _array = std::move(mov._array); return *this; }
    // Gets an item at an arbitrary point on the stack. Valid only if i < _length
    inline const T& operator[](CType i) const { return _array[i]; }
    inline T& operator[](CType i) { return _array[i]; }

    typedef typename DynArray<T, CType, ArrayType, Alloc>::SerializerArray SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { _array.template Serialize<Engine>(s, id); }

  protected:
    DynArray<T, CType, ArrayType, Alloc> _array;
  };
}

#endif
