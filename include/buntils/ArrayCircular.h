// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __ARRAY_CIRCULAR_H__BUN__
#define __ARRAY_CIRCULAR_H__BUN__

#include "Array.h"
#include "buntils.h"

namespace bun {
  // Simple circular array implementation. Unlike most data structures, CType must be signed instead of unsigned
  template<class T, typename CType = ptrdiff_t, typename Alloc = StandardAllocator<T>>  requires std::is_signed<CType>::value
  class BUN_COMPILER_DLLEXPORT ArrayCircular : protected ArrayBase<T, CType, Alloc>
  {
  protected:
    using BASE = ArrayBase<T, CType, Alloc>;
    using CT = typename BASE::CT;
    using Ty = typename BASE::Ty;
    using BASE::_array;
    using BASE::_capacity;

  public:
    // Constructors
    inline ArrayCircular(const ArrayCircular& copy) : BASE(0, copy), _cur(-1), _length(0) { operator=(copy); }
    inline ArrayCircular(ArrayCircular&& mov) : BASE(std::move(mov)), _cur(mov._cur), _length(mov._length)
    { 
      mov._cur = -1;
      mov._length = 0;
    }
    inline ArrayCircular(CT size, const Alloc& alloc) : BASE(size, alloc), _cur(-1), _length(0) {}
    inline explicit ArrayCircular(CT size) requires std::is_default_constructible_v<Alloc> : BASE(size), _cur(-1), _length(0) {}
    inline explicit ArrayCircular() requires std::is_default_constructible_v<Alloc> : BASE(0), _cur(-1), _length(0) {}
    inline ~ArrayCircular() { Clear(); }
    BUN_FORCEINLINE void Push(const T& item) { _push<const T&>(item); }
    BUN_FORCEINLINE void Push(T&& item) { _push<T&&>(std::move(item)); }
    inline T Pop()
    {
      assert(_length > 0);
      --_length;
      T r(std::move(_array[_cur]));
      _array[_cur].~T();
      _cur = bun_Mod<CT>(_cur - 1, _capacity);
      return r;
    }
    inline T PopBack()
    {
      assert(_length > 0);
      CT l = _modIndex(--_length);
      T r(std::move(_array[l]));
      _array[l].~T();
      return r;
    }
    inline const T& Front() const { assert(_length > 0); return _array[_cur]; }
    inline T& Front() { assert(_length > 0); return _array[_cur]; }
    inline const T& Back() const { assert(_length > 0); return _array[_modIndex(_length - 1)]; }
    inline T& Back() { assert(_length > 0); return _array[_modIndex(_length - 1)]; }
    inline CT Capacity() const { return _capacity; }
    inline CT Length() const { return _length; }
    inline void Clear()
    {
      if(_length == _capacity) // Dump the whole thing
        BASE::_setLength(_array, _length, 0);
      else if(_cur - _length >= -1) // The current used chunk is contiguous
        BASE::_setLength(_array + _cur - _length + 1, _length, 0);
      else // We have two seperate chunks that must be dealt with
      {
        CT i = _modIndex(_length - 1);
        BASE::_setLength(_array + i, _capacity - i, 0);
        BASE::_setLength(_array, _cur + 1, 0); // We can only have two seperate chunks if it crosses over the 0 mark at some point, so this always starts at 0
      }

      _length = 0;
      _cur = -1;
    }
    inline void SetCapacity(CT nsize) // Will preserve the array but only if it got larger
    {
      if(nsize < _capacity)
      {
        Clear(); // Getting the right destructors here is complicated and trying to preserve the array when it's shrinking is meaningless.
        BASE::_setCapacityDiscard(nsize);
      }
      else if(nsize > _capacity)
      {
        T* n = BASE::_getAlloc(nsize, 0, 0);
        if(_cur - _length >= -1) // If true the chunk is contiguous
          BASE::_copyMove(n + _cur - _length + 1, _array + _cur - _length + 1, _length);
        else
        {
          CT i = _modIndex(_length - 1);
          BASE::_copyMove(n + bun_Mod<CT>(_cur - _length + 1, nsize), _array + i, _capacity - i);
          BASE::_copyMove(n, _array, _cur + 1);
        }
        BASE::_dealloc(_array, _capacity);
        _array = n;
        _capacity = nsize;
      }
    }
    BUN_FORCEINLINE T& operator[](CT index) { return _array[_modIndex(index)]; } // an index of 0 is the most recent item pushed into the circular array.
    BUN_FORCEINLINE const T& operator[](CT index) const { return _array[_modIndex(index)]; }
    inline ArrayCircular& operator=(const ArrayCircular& right) requires is_copy_constructible_or_incomplete_v<T>
    {
      Clear();
      BASE::_setCapacityDiscard(right._capacity);
      _cur = right._cur;
      _length = right._length;

      if(_length == _capacity) // Copy the whole thing
        BASE::_copy(_array, right._array, _length);
      else if(_cur - _length >= -1) // The current used chunk is contiguous
        BASE::_copy(_array + _cur - _length + 1, right._array + _cur - _length + 1, _length);
      else // We have two seperate chunks that must be dealt with
      {
        CT i = _modIndex(_length - 1);
        BASE::_copy(_array + i, right._array + i, _capacity - i);
        BASE::_copy(_array, right._array, _cur + 1);
      }

      return *this;
    }
    inline ArrayCircular& operator=(ArrayCircular&& right)
    {
      Clear();
      BASE::operator=(std::move(right));
      _cur = right._cur;
      _length = right._length;
      right._length = 0;
      right._cur = -1;
      return *this;
    }

    template<bool CONST>
    struct BUN_TEMPLATE_DLLEXPORT CircularIterator
    {
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type = T;
      using difference_type = ptrdiff_t;
      using pointer = std::conditional_t<CONST, const T*, T*>;
      using reference = std::conditional_t<CONST, const T&, T&>;
      using ptr = std::conditional_t<CONST, const ArrayCircular*, ArrayCircular*>;

      inline CircularIterator(CT c, ptr s) : cur(c), src(s) { }
      inline reference operator*() { return (*src)[cur]; }
      inline CircularIterator& operator++() { ++cur; return *this; } //prefix
      inline CircularIterator operator++(int) { CircularIterator r(*this); ++*this; return r; } //postfix
      inline CircularIterator& operator--() { --cur; return *this; }
      inline CircularIterator operator--(int) { CircularIterator r(*this); --*this; return r; } //postfix
      inline bool operator==(const CircularIterator& _Right) const { return (cur == _Right.cur); }
      inline bool operator!=(const CircularIterator& _Right) const { return (cur != _Right.cur); }

      CT cur;
      ptr src;
    };

    BUN_FORCEINLINE CircularIterator<true> begin() const { return CircularIterator<true>(0, this); }
    BUN_FORCEINLINE CircularIterator<true> end() const { return CircularIterator<true>(_length, this); }
    BUN_FORCEINLINE CircularIterator<false> begin() { return CircularIterator<false>(0, this); }
    BUN_FORCEINLINE CircularIterator<false> end() { return CircularIterator<false>(_length, this); }

    using SerializerArray = std::conditional_t<internal::is_pair_array<T>::value, void, T>;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id)
    {
      if constexpr(!internal::is_pair_array<T>::value)
        s.template EvaluateArray<ArrayCircular, T, &_serializeAdd<Engine>, CT, nullptr>(*this, _length, id);
      else
        s.template EvaluateKeyValue<ArrayCircular>(*this, [this](Serializer<Engine>& e, const char* name) { _serializeInsert(e, name); });
    }

  protected:
    template<typename Engine, bool U = internal::is_pair_array<T>::value>
    inline std::enable_if_t<U> _serializeInsert(Serializer<Engine>& e, const char* name)
    {
      T pair;
      std::get<0>(pair) = name;
      Serializer<Engine>::template ActionBind<remove_cvref_t<std::tuple_element_t<1, T>>>::Parse(e, std::get<1>(pair), name);
      Push(pair);
    }
    template<typename Engine>
    inline static void _serializeAdd(Serializer<Engine>& e, ArrayCircular& obj, int& n)
    {
      T x;
      Serializer<Engine>::template ActionBind<T>::Parse(e, x, 0);
      Push(x);
    }

    template<typename U> // Note that _cur+1 can never be negative so we don't need to use bun_Mod
    inline void _push(U && item)
    {
      _cur = ((_cur + 1) % _capacity);

      if(_length < _capacity)
      {
        new(_array + _cur) T(std::forward<U>(item));
        ++_length;
      }
      else
        _array[_cur] = std::forward<U>(item);
    }
    BUN_FORCEINLINE CT _modIndex(CT index) const { return bun_Mod<CT>(_cur - index, _capacity); }

    CT _cur;
    CT _length;
  };
}

#endif
