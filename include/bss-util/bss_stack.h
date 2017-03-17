// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_BSS_STACK_H__
#define __C_BSS_STACK_H__

#include "cDynArray.h"

namespace bss_util {
  // Fast, tiny array-based stack. Pop and Top are only valid if there is an item in the stack; this check must be done by the user.
  template<class T, typename CType=uint32_t, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cStack
  {
  public:
    inline cStack(const cStack& copy) : _array(copy) {}
    inline cStack(cStack&& mov) : _array(std::move(mov)) {}
    inline explicit cStack(CType init=0) : _array(init) {}
    inline ~cStack() {}
    // Pushes an item on to the stack in LIFO order.
    BSS_FORCEINLINE void Push(const T& value) { _array.Add(value); }
    BSS_FORCEINLINE void Push(T&& value) { _array.Add(std::move(value)); }
    // Pops an item off the stack in LIFO order. If there is no item to pop, this call is illegal and will crash.
    BSS_FORCEINLINE T Pop() { T r(std::move(_array.Back())); _array.RemoveLast(); return std::move(r); }
    // Peeks at the item on top of the stack, but does not remove it. If there is no item to look at, this is illegal.
    BSS_FORCEINLINE T& Peek() { return _array.Back(); }
    BSS_FORCEINLINE const T& Peek() const { return _array.Back(); }
    // Throws away the item on top of the stack.
    BSS_FORCEINLINE void Discard() { _array.RemoveLast(); }
    BSS_FORCEINLINE void Clear() { _array.Clear(); }
    // Gets how many items are on the stack. If this is 0, Push and Pop cannot be called.
    BSS_FORCEINLINE CType Length() const { return _array.Length(); }
    // Sets how many items are on the stack. Is usually used to truncate the stack, but can also be used to extend it. The stack
    // will follow the same rules that govern extending it's base array, so in a simple array, the values will be uninitialized.
    BSS_FORCEINLINE void SetLength(CType length) { _array.SetLength(length); }
    // Gets the capacity of the underlying array.
    BSS_FORCEINLINE CType Capacity() const { return _array.Capacity(); }
    inline cStack& operator=(const cStack& copy) { _array = copy._array; return *this; }
    inline cStack& operator=(cStack&& mov) { _array = std::move(mov._array); return *this; }
    // Gets an item at an arbitrary point on the stack. Valid only if i < _length
    inline const T& operator[](CType i) const { return _array[i]; }
    inline T& operator[](CType i) { return _array[i]; }

  protected:
    cDynArray<T, CType, ArrayType, Alloc> _array;
  };
}

#endif
