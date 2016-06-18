+++
title = "Array"
description = "cArray.h Reference"
tags = [
    "bssutil",
    "cArray.h",
    "ref",
]
date = "2016-04-02"
categories = [
    "Reference",
]
+++

The basis for all of the arrays in bss-util is in cArray.h, which contains two helper classes and two public classes. [cArray]({{< relref "#carray" >}}) is a standalone non-resizable array, whereas [cArrayBase]({{< relref "#carraybase" >}}) is intended to be inherited by a more complex data structure. Everything else is mostly for convenience.

The `ARRAY_TYPE` enumeration defines the four fundamental types of data management used by the underlying array implementation, which are consequently used in most of the data structures that [inherit cArrayInternal](#).

### enum ARRAY_TYPE
* `CARRAY_SIMPLE = 0`
<p>This is for **POD**, or Plain Old Data, which does not own any pointers or anything that needs special attention, so the underlying array can simply copy the bytes around without bothering with constructors or destructors. More complex data structures like the [dynamic array](#) can see significant performance boosts when moving large numbers of small objects around. It is crucial that the underlying type of the array **has no constructors or destructors** when using this management scheme. At the very least, the constructors should be for convenience only.</p>
* `CARRAY_CONSTRUCT = 1`
<p>This is for types that must be constructed or destructed, but **do not have any external references**. This means that nothing else in the program has a pointer to an element in the array, even indirectly. This is a situation that comes up fairly often, as many types will have internal pointers to manage, but since nothing cares about where those types are in memory, the bytes can be moved around freely after construction, so long as the destructor is eventually called on them.</p>
* `CARRAY_SAFE = 2`
<p>This behaves the same as an `std::vector` does, by carefully preserving the contents of the type by always calling the constructors and using move semantics where appropriate. While this management form can take a big hit on performance during resizing (because realloc cannot be called, and all the move constructors will have to be run), you can safely put any type into the array using this management scheme, provided it has a copy constructor.</p>
* `CARRAY_MOVE = 3`
<p>This is a special variant on the `SAFE` management scheme that removes the `copy` helper function from [cArrayInternal]({{< relref "#carrayinternal" >}}). While this prevents you from ever copying an array or data structure, it will still permit resizing the array using only move semantics. This management structure is crucial, because it allows for putting move-only types into arrays and data structures, such as `std::unique_ptr`.</p>

## cArray {{<badge code>}}cArray.h{{</badge>}} {{<badge green>}}14/14 Test Coverage{{</badge>}}
    template<class T,
      typename CType = size_t,
      ARRAY_TYPE ArrayType = CARRAY_SIMPLE,
      typename Alloc = StaticAllocPolicy<T>>
    class BSS_COMPILER_DLLEXPORT cArray :
      protected cArrayBase<T, CType, Alloc>,
      protected cArrayInternal<T, CType, ArrayType, Alloc>
Standalone array that is not resizable, which means that all elements are always constructed or destructed when the capacity changes. Useful for situations where the array size almost never changes. Subclasses **should not** inherit this class, instead they should inherit [cArrayBase]({{< relref "#carraybase" >}}) instead and use the [cArrayInternal]({{< relref "#carrayinternal" >}}) helper functions.

### Template Arguments
* **`class T`**: Determines the underlying type used for this array. No special handling is done for pointers, so if you have an array of `type*`, functions that return `T*` will return `type**`. Any type can be used here, but remember that you should use the correct management scheme for it.
* **`typename CType`**: Sets the *index type* of this array. Must be an integral type. This defaults to `size_t`, but can be a signed integer. Making this smaller than `size_t` is usually pointless, but certain data structures use a special index type, and ensuring the array uses the same type for its indexes helps cohesion and type safety.
* **`ARRAY_TYPE ArrayType`**: Determines the [management scheme]({{< relref "#enum-array_type" >}}) used by the array. Make sure this is appropriate for the type you're using for `T`!
* **`typename Alloc`**: Arrays accept purely static custom allocators that follow the [bss-alloc](#) standards. Stateful allocators are not supported. By default, the standard static allocator is used, which simply calls `realloc` and `free`.

    
## cArrayInternal {{<badge code>}}cArray.h{{</badge>}} {{<badge cyan>}}Indirect Coverage{{</badge>}}


## cArrayBase {{<badge code>}}cArray.h{{</badge>}} {{<badge cyan>}}Indirect Coverage{{</badge>}}
    template<class T,
      typename CType = size_t,
      typename Alloc = StaticAllocPolicy<T>>
    class BSS_COMPILER_DLLEXPORT cArrayBase
This class provides only the most basic array management operations and is intended to be inherited by more complex classes. Data structures that inherit this class should use [cArrayInternal]({{< relref "#carrayinternal" >}}) with the appropriate management scheme to manipulate the array. It's template arguments are the same as [cArray]({{< relref "#carray" >}}), except that it has no `ARRAY_TYPE` argument.
  
#### `cArrayBase(mov)`
    inline cArrayBase(cArrayBase&& mov)
    
#### `cArrayBase(CType capacity)`
    inline explicit cArrayBase(CT_ capacity = 0)
    
#### `~cArrayBase()`
    inline ~cArrayBase()
#### `Capacity()`
    inline CT_ Capacity() const noexcept
#### `SetCapacity(CT_ capacity)`
    BSS_FORCEINLINE void SetCapacity(CT_ capacity) noexcept
#### `SetCapacityDiscard(CT_ capacity)`
    BSS_FORCEINLINE void SetCapacityDiscard(CT_ capacity) noexcept
#### `operator=(mov)`
    cArrayBase& operator=(cArrayBase&& mov) noexcept
#### `GetSlice()`
    inline cArraySlice<T, CType> GetSlice()

## cArraySlice {{<badge code>}}cArray.h{{</badge>}} {{<badge red>}}0/6 Test Coverage{{</badge>}}


