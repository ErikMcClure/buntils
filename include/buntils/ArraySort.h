// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __ARRAY_SORT_H__BUN__
#define __ARRAY_SORT_H__BUN__

#include "algo.h"
#include "compare.h"
#include "DynArray.h"

namespace bun {
  // A dynamic array that keeps its contents sorted using insertion sort and uses a binary search to retrieve items.
  template<typename T, char(*CFunc)(const T&, const T&) = &CompT<T>, typename CType = size_t, typename Alloc = StandardAllocator<T>>
  class BUN_COMPILER_DLLEXPORT ArraySort
  {
  protected:
    using Ty = T;
    using CT = CType;
    using constref = const T&;
    using moveref = T&&;

  public:
    inline ArraySort(const ArraySort& copy) : _array(copy._array) {}
    inline ArraySort(ArraySort&& mov) : _array(std::move(mov._array)) {}
    inline explicit ArraySort(const std::span<const T>& slice) requires std::is_default_constructible_v<Alloc> : _array(slice) {}
    inline ArraySort(CT size, const Alloc& alloc) : _array(size, alloc) {}
    inline explicit ArraySort(CT size) requires std::is_default_constructible_v<Alloc> : _array(size) {}
    inline explicit ArraySort() requires std::is_default_constructible_v<Alloc> : _array(0) {}
    inline ~ArraySort() {}
    BUN_FORCEINLINE CT Insert(constref item) { CT loc = _insert(item); _array.Insert(item, loc); return loc; }
    BUN_FORCEINLINE CT Insert(moveref item)
    { 
      CT loc = _insert(std::move(item));
      _array.Insert(std::move(item), loc);
      return loc;
    }
    inline void Clear() { _array.Clear(); }
    inline void Discard(CType num) { _array.SetLength((num > _array.Length()) ? 0 : (_array.Length() - num)); }
    BUN_FORCEINLINE bool Empty() const { return _array.Empty(); }
    BUN_FORCEINLINE void SetCapacity(CT capacity) { _array.SetCapacity(capacity); }
    BUN_FORCEINLINE CT Length() const { return _array.Length(); }
    BUN_FORCEINLINE CT Capacity() const { return _array.Capacity(); }
    inline const T& Front() const { return _array.Front(); }
    inline T& Front() { return _array.Front(); }
    inline const T& Back() const { return _array.Back(); }
    inline T& Back() { return _array.Back(); }
    inline const T* begin() const noexcept { return _array.begin(); }
    inline const T* end() const noexcept { return _array.end(); }
    inline T* begin() noexcept { return _array.begin(); }
    inline T* end() noexcept { return _array.end(); }
    inline CT size() const noexcept { return _array.size(); }
    inline T* data() noexcept { return _array.data(); }
    inline const T* data() const noexcept { return _array.data(); }

    CT ReplaceData(CT index, constref item)
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

    inline bool Remove(CT index)
    {
      if(index >= _array.Length()) return false;
      _array.Remove(index);
      return true;
    }

    BUN_FORCEINLINE CT Find(constref item) const
    {
      return BinarySearchExact<T, T, CT, CFunc>(_array, item, 0, _array.Length());
    }

    // Can actually return -1 if there isn't anything in the array
    inline CT FindNear(constref item, bool before = true) const
    {
      CT retval = before ? BinarySearchBefore<T, CT, CFunc>(_array, _array.Length(), item) : BinarySearchAfter<T, CT, CFunc>(_array, _array.Length(), item);
      return (retval < _array.Length()) ? retval : (CT)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }

    BUN_FORCEINLINE constref operator [](CT index) const { return _array[index]; }
    BUN_FORCEINLINE T& operator [](CT index) { return _array[index]; }
    inline ArraySort& operator=(const ArraySort& right) { _array = right._array; return *this; }
    inline ArraySort& operator=(ArraySort&& mov) { _array = std::move(mov._array); return *this; }
    inline ArraySort& operator=(std::span<const T> copy) { _array = copy; return *this; }

    using SerializerArray = std::conditional_t<internal::is_pair_array<T>::value, void, T>;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id)
    {
      if constexpr(!internal::is_pair_array<T>::value)
        s.template EvaluateArray<ArraySort, T, &_serializeAdd<Engine>, CT, nullptr>(*this, Length(), id);
      else
        s.template EvaluateKeyValue<ArraySort>(*this, [this](Serializer<Engine>& e, const char* name) { _serializeInsert(e, name); });
    }

  protected:
    template<typename Engine, bool U = internal::is_pair_array<T>::value>
    inline std::enable_if_t<U> _serializeInsert(Serializer<Engine>& e, const char* name)
    {
      T pair;
      std::get<0>(pair) = name;
      Serializer<Engine>::template ActionBind<std::remove_cvref_t<std::tuple_element_t<1, T>>>::Parse(e, std::get<1>(pair), name);
      Insert(pair);
    }
    template<typename Engine>
    inline static void _serializeAdd(Serializer<Engine>& e, ArraySort& obj, int& n)
    {
      T x;
      Serializer<Engine>::template ActionBind<T>::Parse(e, x, 0);
      Insert(x);
    }
    template<typename U>
    CT _insert(U && item)
    {
      if(_array.Empty())
        return 0;
      else
      {
        CT loc;

        if((*CFunc)(item, _array[0]) < 0)
          loc = 0;
        else if((*CFunc)(item, _array.Back()) >= 0)
          loc = _array.Length();
        else
          loc = BinarySearchAfter<T, CT, CFunc>(_array, _array.Length(), std::forward<U>(item));

        return loc;
      }
    }

    DynArray<T, CType, Alloc> _array;
  };
}

#endif
