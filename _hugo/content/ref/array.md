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

#### `InsertRangeSimple(T* a, CType length, CType index, const T* t, CType tsize)`
    template<class T, typename CType = size_t>
    static inline void InsertRangeSimple(T* a, CType length, CType index, const T* t, CType tsize) noexcept
    
**Preconditions:** The capacity of `a` must be equal to or greater than `length` + `tsize`.

This is a low level helper function for inserting ranges of objects using `memmove()` and `memcpy()`. It takes a target array, which must be large enough to hold all the data, the current length of the target array, the index in the target array to insert the data into, the source array to insert, and the size of the source array.

#### `RemoveRangeSimple(T* a, CType length, CType index, CType range)`
    template<class T, typename CType = size_t>
    static inline void RemoveRangeSimple(T* a, CType length, CType index, CType range) noexcept

Similar to [InsertRangeSimple]({{< relref "#insertrangesimple" >}}), this _removes_ a range of data from the given array using `memmove()`. The first three arguments are the same as [InsertRangeSimple]({{< relref "#insertrangesimple" >}}), but the fourth argument is instead the length of data that should be removed. `index` + `range` should not exceed `length`.

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
This is an internal class used by classes that inherit [cArrayBase]({{< relref "#carraybase" >}}). It has the same template arguments as [cArray]({{< relref "#carray" >}}), but is specialized on the `ARRAY_TYPE` template argument.

#### `_copymove(T* dest, T* src, CType n)`
    static void _copymove(T* BSS_RESTRICT dest, T* BSS_RESTRICT src, CType n) noexcept
**Preconditions**: `dest` should not have had any constructors called on it. `src` must point to valid memory. Both `dest` and `src` must be at least `n` long.

*Moves* the contents of `src` into `dest` using the appropriate [management scheme]({{< relref "#enum-array-type" >}}). Both `SIMPLE` and `CONSTRUCT will just use `memcpy`. `SAFE` and `MOVE` go through `dest` and call the move constructor, passing the corresponding element from `src` in using `std::move`. It then calls the destructors on all elements in `src` to deal with any remaining cleanup.
    
#### `_copy(T* BSS_RESTRICT dest, const T* BSS_RESTRICT src, CType n)`
    static void _copy(T* BSS_RESTRICT dest, const T* BSS_RESTRICT src, CType n) noexcept
**Preconditions**: `dest` should not have had any constructors called on it. `src` must point to valid memory. Both `dest` and `src` must be at least `n` long.
    
*Copies* the contents of `src` into `dest`. Both `SIMPLE` and `CONSTRUCT` will just use `memcpy`. `SAFE` will call the copy constructor on each element of `dest`, passing in a constant reference to the corresponding `src` element. It does **not** call any destructors. `MOVE` will throw an assertion failure if this function is called.

#### `_insert(T* a, CType length, CType index, U && t)`
    template<typename U> static void _insert(T* a, CType length, CType index, U && t) noexcept
**Preconditions**: `a` must have a capacity of at least `length + 1`.

Inserts the element `t` into `a` using `std::forward` to enforce perfect forwarding. While `SIMPLE` and `CONSTRUCT` both use memmove to shift elements around, **all** management schemes will call the constructor on the item to enforce perfect forwarding. This should be optimized into a memory copy by the compiler if there is no constructor provided. 
    
#### `_remove(T* a, CType length, CType index)`
    static void _remove(T* a, CType length, CType index) noexcept
Removes an element from the specified index. `SIMPLE` uses `memmove` to accomplish this, but `CONSTRUCT` calls the destructor on the item to be removed *first*, then uses `memmove` to shift the other elements over by one. `SAFE` (and `MOVE`) calls `operator=` using move semantics, setting each item past the index equal to the contents of the item to its right, then calls the destructor of the *last* item, which is the opposite of what `CONSTRUCT` does.

#### `_setlength(T* a, CType old, CType n)`
    static void _setlength(T* a, CType old, CType n) noexcept
**Preconditions**: T must have a default constructor. `a` must be as large or larger than **both** `old` and `n`.

This deals with a change in length in `a`. If the new length `n` is greater than `old`, constructors will be called on the new elements. If `n` is less than `old`, all elements above `n` will have their destructors called, up to `old`. This function does nothing for `SIMPLE`.
    
#### `_setcapacity(cArrayBase<T, CType, Alloc>& a, CType length, CType capacity)`
    static void _setcapacity(cArrayBase<T, CType, Alloc>& a, CType length, CType capacity) noexcept

## cArrayBase {{<badge code>}}cArray.h{{</badge>}} {{<badge cyan>}}Indirect Coverage{{</badge>}}
    template<class T,
      typename CType = size_t,
      typename Alloc = StaticAllocPolicy<T>>
    class BSS_COMPILER_DLLEXPORT cArrayBase
This class provides only the most basic array management operations and is intended to be inherited by more complex classes. Data structures that inherit this class should use [cArrayInternal]({{< relref "#carrayinternal" >}}) with the appropriate management scheme to manipulate the array. It's template arguments are the same as [cArray]({{< relref "#carray" >}}), except that it has no `ARRAY_TYPE` argument.
  
#### `cArrayBase(mov)`
    inline cArrayBase(cArrayBase&& mov)

Steals the array pointer from `mov` and sets `mov`'s array pointer and capacity to 0.

#### `cArrayBase(CType capacity)`
    inline explicit cArrayBase(CT_ capacity = 0)
Constructs a new array with the given capacity. If the capacity is zero, the array pointer is set to `null` and no allocation is attempted.

#### `~cArrayBase()`
    inline ~cArrayBase()
Frees the array pointer, if it exists.

#### `Capacity()`
    inline CT_ Capacity() const noexcept
Gets the capacity of the array.

#### `SetCapacity(CT_ capacity)`
    BSS_FORCEINLINE void SetCapacity(CT_ capacity) noexcept
Sets the capacity and uses realloc to preserve the existing contents.    

#### `SetCapacityDiscard(CT_ capacity)`
    BSS_FORCEINLINE void SetCapacityDiscard(CT_ capacity) noexcept
Sets the capacity, but simply allocates an entirely new block of memory, discarding any contents currently in the array.

#### `operator=(mov)`
    cArrayBase& operator=(cArrayBase&& mov) noexcept
Destroys the array currently being held, if it exists, and then steals the array pointer from `mov` and sets `mov`'s array pointer and capacity to 0.

#### `GetSlice()`
    inline cArraySlice<T, CType> GetSlice()
Returns an [cArraySlice]({{< relref "#carrayslice" >}}) that covers this entire array, starting at the first element and setting the length equal to the current capacity.

## cArraySlice {{<badge code>}}cArray.h{{</badge>}} {{<badge red>}}0/6 Test Coverage{{</badge>}}


