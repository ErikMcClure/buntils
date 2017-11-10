// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ARRAY_SORT_H__BSS__
#define __ARRAY_SORT_H__BSS__

#include "algo.h"
#include "compare.h"
#include "DynArray.h"

namespace bss {
  // A dynamic array that keeps its contents sorted using insertion sort and uses a binary search to retrieve items.
  template<typename T, char(*CFunc)(const T&, const T&) = &CompT<T>, typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StandardAllocator<T>>
  class BSS_COMPILER_DLLEXPORT ArraySort
  {
  public:
    typedef CType CT_;
    typedef const T& constref;
    typedef T&& moveref;

    inline ArraySort(const ArraySort& copy) : _array(copy._array) {}
    inline ArraySort(ArraySort&& mov) : _array(std::move(mov._array)) {}
    inline explicit ArraySort(const Slice<const T, CType>& slice) : _array(slice) {}
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline ArraySort(CT_ size, typename Alloc::policy_type* policy) : _array(size, policy) {}
    inline explicit ArraySort(CT_ size = 0) : _array(size) {}
    inline ~ArraySort() {}
    BSS_FORCEINLINE CT_ Insert(constref item) { CT_ loc = _insert(item); _array.Insert(item, loc); return loc; }
    BSS_FORCEINLINE CT_ Insert(moveref item)
    { 
      CT_ loc = _insert(std::move(item));
      _array.Insert(std::move(item), loc);
      return loc;
    }
    inline void Clear() { _array.Clear(); }
    inline void Discard(CType num) { _array.SetLength((num > _array.Length()) ? 0 : (_array.Length() - num)); }
    BSS_FORCEINLINE bool Empty() const { return _array.Empty(); }
    BSS_FORCEINLINE void SetCapacity(CT_ capacity) { _array.SetCapacity(capacity); }
    BSS_FORCEINLINE CT_ Length() const { return _array.Length(); }
    BSS_FORCEINLINE CT_ Capacity() const { return _array.Capacity(); }
    inline const T& Front() const { return _array.Front(); }
    inline T& Front() { return _array.Front(); }
    inline const T& Back() const { return _array.Back(); }
    inline T& Back() { return _array.Back(); }
    inline const T* begin() const noexcept { return _array.begin(); }
    inline const T* end() const noexcept { return _array.end(); }
    inline T* begin() noexcept { return _array.begin(); }
    inline T* end() noexcept { return _array.end(); }
    BSS_FORCEINLINE Slice<T, CT_> GetSlice() const noexcept { return _array.GetSlice(); }

    CT_ ReplaceData(CT_ index, constref item)
    {
      T swap;
      _array[index] = item;
      while(index > 0 && CFunc(_array[index - 1], _array[index]) > 0)
      { //we do a swap here and hope that the compiler is smart enough to optimize it
        swap = _array[index];
        _array[index] = _array[index - 1];
        _array[--index] = swap;
      }
      while(index < (_array.Length() - 1) && CFunc(_array[index + 1], _array[index]) < 0)
      { //we do a swap here and hope that the compiler is smart enough to optimize it
        swap = _array[index];
        _array[index] = _array[index + 1];
        _array[++index] = swap;
      }
      return index;
    }

    inline bool Remove(CT_ index)
    {
      if(index >= _array.Length()) return false;
      _array.Remove(index);
      return true;
    }

    BSS_FORCEINLINE CT_ Find(constref item) const
    {
      return BinarySearchExact<T, T, CT_, CFunc>(_array, item, 0, _array.Length());
    }

    // Can actually return -1 if there isn't anything in the array
    inline CT_ FindNear(constref item, bool before = true) const
    {
      CT_ retval = before ? BinarySearchBefore<T, CT_, CFunc>(_array, _array.Length(), item) : BinarySearchAfter<T, CT_, CFunc>(_array, _array.Length(), item);
      return (retval < _array.Length()) ? retval : (CT_)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }

    BSS_FORCEINLINE constref operator [](CT_ index) const { return _array[index]; }
    BSS_FORCEINLINE T& operator [](CT_ index) { return _array[index]; }
    inline ArraySort& operator=(const ArraySort& right) { _array = right._array; return *this; }
    inline ArraySort& operator=(ArraySort&& mov) { _array = std::move(mov._array); return *this; }
    inline ArraySort& operator=(const Slice<const T, CType>& copy) { _array = copy; return *this; }

  protected:
    template<typename U>
    CT_ _insert(U && item)
    {
      if(_array.Empty())
        return 0;
      else
      {
        CT_ loc;

        if((*CFunc)(item, _array[0]) < 0)
          loc = 0;
        else if((*CFunc)(item, _array.Back()) >= 0)
          loc = _array.Length();
        else
          loc = BinarySearchAfter<T, CT_, CFunc>(_array, _array.Length(), std::forward<U>(item));

        return loc;
      }
    }

    DynArray<T, CType, ArrayType, Alloc> _array;
  };
}

#endif
