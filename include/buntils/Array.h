// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __ARRAY_H__BUN__
#define __ARRAY_H__BUN__

#include "Alloc.h"
#include "buntils.h"
#include <string.h>
#include <initializer_list>
#include <span>

namespace bun {
  template<class Engine>
  class Serializer;

  // Helper function for inserting a range into a simple array
  template<class T, typename CType = size_t>
  inline void InsertRangeSimple(T* dest, CType length, CType index, const T* src, CType srcsize) noexcept
  {
    assert(index >= 0 && length >= index && dest != 0);
    memmove(dest + index + srcsize, dest + index, sizeof(T) * (length - index));
    memcpy(dest + index, src, sizeof(T) * srcsize);
  }

  template<class T, typename CType = size_t>
  inline void RemoveRangeSimple(T* dest, CType length, CType index, CType range) noexcept
  {
    assert(index >= 0 && length > index && dest != 0);
    memmove(dest + index, dest + index + range, sizeof(T) * (length - index - range));
  }

#ifdef BUN_DEBUG
#define BUN_DEBUGFILL(p, old, n, Ty) if(p) memset(p + std::min(n, old), 0xfd, (size_t)(((n > old)?(n - old):(old - n))*sizeof(Ty)))
#else
#define BUN_DEBUGFILL(p, old, n, Ty)  
#endif

  // Handles the very basic operations of an array. Constructor management is done by classes that inherit this class.
  template<class T, typename CType = size_t, typename Alloc = StandardAllocator<T>> requires std::is_integral<CType>::value
  class BUN_COMPILER_DLLEXPORT ArrayBase : protected Alloc
  {
  public:
    using CT = CType; // There are cases when you need access to these types even if you don't inherit (see RandomQueue in bun_algo.h)
    using Ty = T;

    inline ArrayBase(ArrayBase&& mov) : Alloc(std::move(mov)), _array(mov._array), _capacity(mov._capacity)
    {
      mov._array = 0;
      mov._capacity = 0;
    }
    inline ArrayBase(CT capacity, const Alloc& alloc) : Alloc(alloc), _array(0), _capacity(0) { _array = _getAlloc(capacity, 0, 0); _capacity = capacity; }
    inline explicit ArrayBase(CT capacity) requires std::is_default_constructible_v<Alloc> : _array(0), _capacity(0) { _array = _getAlloc(capacity, 0, 0); _capacity = capacity; }
    inline ArrayBase() requires std::is_default_constructible_v<Alloc> : _array(0), _capacity(0) {}
    inline ~ArrayBase()
    {
      _dealloc(_array, _capacity);
    }
    ArrayBase& operator=(ArrayBase&& mov) noexcept
    {
      _dealloc(_array, _capacity);
      Alloc::operator=(std::move(mov));
      _array = mov._array;
      _capacity = mov._capacity;
      mov._array = 0;
      mov._capacity = 0;
      return *this;
    }

  protected:
    BUN_FORCEINLINE void _setCapacity(CT capacity) noexcept
    {
      assert((_capacity != 0) == (_array != 0));
      _array = _getAlloc(capacity, _array, _capacity);
      _capacity = capacity;
    }
    BUN_FORCEINLINE void _setCapacityDiscard(CT capacity) noexcept
    {
      _dealloc(_array, _capacity);
      _array = _getAlloc(capacity, 0, 0);
      _capacity = capacity;
    }

    static void _setLength(T* dest, CType old, CType n) noexcept
    {
      if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>)
      {
        for (CType i = n; i < old; ++i)
          dest[i].~T();
      }
      BUN_DEBUGFILL(dest, old, n, T);
      if constexpr (std::is_default_constructible_v<T> && !std::is_trivially_default_constructible_v<T>)
      {
        for (CType i = old; i < n; ++i)
          new(dest + i) T();
      }
    }

    inline void _setCapacity(CT capacity, CT length) noexcept
    {
      if constexpr (std::is_trivially_copyable_v<T>)
        _setCapacity(capacity);
      else
      {
        T* n = _getAlloc(capacity, 0, 0);
        if (n != nullptr)
          _copyMove(n, _array, bun_min(length, capacity));
        _dealloc(_array, _capacity);
        _array = n;
        _capacity = capacity;
      }
    }

    inline T* _getAlloc(CT n, T* prev, CT old) noexcept
    {
      assert((_capacity != 0) == (_array != 0));
      if (!n)
      {
        _dealloc(prev, old);
        return 0;
      }
      if constexpr (std::is_trivially_copyable_v<T>)
        return standard_realloc<Alloc>(*this, (size_t)n, prev, old);
      else
      {
        auto p = std::allocator_traits<Alloc>::allocate(*this, n);
        if (prev && old > 0)
        {
          auto count = std::min(old, n);
          for (CType i = 0; i < count; ++i)
            new(p + i) T(std::move(prev[i]));
          _dealloc(prev, old);
        }
        return p;
      }
    }
    BUN_FORCEINLINE void _dealloc(T* p, size_t old) noexcept { if (p) std::allocator_traits<Alloc>::deallocate(*this, p, old); }
    static void _copyMove(T* BUN_RESTRICT dest, T* BUN_RESTRICT src, CType n) noexcept
    {
      if (dest == nullptr || src == nullptr || !n)
        return;
      assert(dest != src);

      if constexpr (std::is_trivially_copyable_v<T>)
        memcpy(dest, src, sizeof(T) * n);
      else
      {
        for (CType i = 0; i < n; ++i)
          new(dest + i) T(std::move(src[i]));
        _setLength(src, n, 0);
      }
    }
    static void _copy(T* BUN_RESTRICT dest, const T* BUN_RESTRICT src, CType n) requires is_copy_constructible_or_incomplete_v<T>
    {
      if (dest == nullptr || src == nullptr || !n)
        return;
      assert(dest != src);

      if constexpr (std::is_trivially_copyable_v<T>)
        memcpy(dest, src, sizeof(T) * n);
      else if constexpr (is_copy_constructible_or_incomplete_v<T>)
      {
        for (CType i = 0; i < n; ++i)
          new(dest + i) T(src[i]);
      }
      else
        static_assert(std::is_void_v<T>, "_copy shouldn't exist");
    }
    template<typename U>
    static void _insert(T* dest, CType length, CType index, U&& item) noexcept
    {
      assert(index >= 0 && length >= index && dest != 0);

      if constexpr (std::is_trivially_copyable_v<T>)
      {
        memmove(dest + index + 1, dest + index, sizeof(T) * (length - index));
        new(dest + index) T(std::forward<U>(item));
      }
      else
      {
        if (index < length)
        {
          new(dest + length) T(std::move(dest[length - 1]));
          std::move_backward<T*, T*>(dest + index, dest + length - 1, dest + length);
          dest[index] = std::forward<U>(item);
        }
        else
          new(dest + index) T(std::forward<U>(item));
      }
    }
    static void _remove(T* dest, CType length, CType index) noexcept
    {
      assert(index >= 0 && length > index);
      if constexpr (!std::is_trivially_copyable_v<T>)
      {
        std::move<T*, T*>(dest + index + 1, dest + length, dest + index);
        dest[length - 1].~T();
      }
      else
      {
        if constexpr (std::is_destructible_v<T>)
          dest[index].~T();
        memmove(dest + index, dest + index + 1, sizeof(T) * (length - index - 1));
      }
    }

    T* _array;
    CT _capacity;
  };

  // Wrapper for underlying arrays that expose the array, making them independently usable without blowing up everything that inherits them
  template<class T, typename CType = size_t, typename Alloc = StandardAllocator<T>>
  class BUN_COMPILER_DLLEXPORT Array : protected ArrayBase<T, CType, Alloc>
  {
  protected:
    using BASE = ArrayBase<T, CType, Alloc>;
    using CT = typename BASE::CT;
    using Ty = typename BASE::Ty;
    using BASE::_array;
    using BASE::_capacity;

  public:
    inline Array(const Array& copy) requires is_copy_constructible_or_incomplete_v<T> : BASE(copy._capacity, copy) { BASE::_copy(_array, copy._array, _capacity); }
    inline Array(Array&& mov) : BASE(std::move(mov)) {}
    inline Array(const std::initializer_list<T>& list) : BASE(list.size())
    {
      auto end = list.end();
      CT c = 0;
      for (auto i = list.begin(); i != end && c < _capacity; ++i)
        new(_array + (c++)) T(*i);
      BASE::_setCapacity(c, _capacity);
    }
    inline Array(CT capacity, const Alloc& alloc) : BASE(capacity, alloc) { BASE::_setLength(_array, 0, _capacity); }
    inline explicit Array(CT capacity) requires std::is_default_constructible_v<Alloc> : BASE(capacity) { BASE::_setLength(_array, 0, _capacity); }
    inline explicit Array() requires std::is_default_constructible_v<Alloc> : BASE(0) { BASE::_setLength(_array, 0, _capacity); }
    inline explicit Array(std::span<const T> s) : BASE(s.size()) { BASE::_copy(_array, s.data(), s.size()); }
    inline ~Array() { BASE::_setLength(_array, _capacity, 0); }
    BUN_FORCEINLINE CT Add(const T& item) { _insert(item, _capacity); return _capacity - 1; }
    BUN_FORCEINLINE CT Add(T&& item) { _insert(std::move(item), _capacity); return _capacity - 1; }
    BUN_FORCEINLINE void Remove(CT index)
    {
      BASE::_remove(_array, _capacity, index); // we don't bother reallocating the array because it got smaller, so we just ignore part of it.
      BASE::_setCapacity(_capacity - 1, _capacity);
    }
    BUN_FORCEINLINE void RemoveLast() { Remove(_capacity - 1); }
    BUN_FORCEINLINE void Insert(const T& item, CT index = 0) { _insert(item, index); }
    BUN_FORCEINLINE void Insert(T&& item, CT index = 0) { _insert(std::move(item), index); }
    BUN_FORCEINLINE void Set(std::span<const T> s) requires is_copy_constructible_or_incomplete_v<T> { Set(s.data(), s.size()); }
    BUN_FORCEINLINE void Set(const T* p, CT n) requires is_copy_constructible_or_incomplete_v<T>
    {
      BASE::_setLength(_array, _capacity, 0);
      BASE::_setCapacityDiscard(n);
      BASE::_copy(_array, p, _capacity);
    }
    BUN_FORCEINLINE bool Empty() const noexcept { return !_capacity; }
    BUN_FORCEINLINE void Clear() noexcept { SetCapacity(0); }
    BUN_FORCEINLINE void SetCapacity(CT capacity) noexcept
    {
      if (capacity == _capacity) return;
      CT old = _capacity;
      if (capacity < _capacity) BASE::_setLength(_array, _capacity, capacity);
      BASE::_setCapacity(capacity, _capacity);
      if (capacity > old) BASE::_setLength(_array, old, capacity);
    }
    BUN_FORCEINLINE void SetCapacityDiscard(CT capacity) noexcept
    {
      if (capacity == _capacity) return;
      BASE::_setLength(_array, _capacity, 0);
      BASE::_setCapacityDiscard(capacity);
      BASE::_setLength(_array, 0, _capacity);
    }
    BUN_FORCEINLINE CT Capacity() const noexcept { return _capacity; }
    inline const T& Front() const noexcept { assert(_capacity > 0); return _array[0]; }
    inline T& Front() noexcept { assert(_capacity > 0); return _array[0]; }
    inline const T& Back() const noexcept { assert(_capacity > 0); return _array[_capacity - 1]; }
    inline T& Back() noexcept { assert(_capacity > 0); return _array[_capacity - 1]; }
    BUN_FORCEINLINE operator T* () noexcept { return _array; }
    BUN_FORCEINLINE operator const T* () const noexcept { return _array; }
#if defined(BUN_64BIT) && defined(BUN_DEBUG) 
    BUN_FORCEINLINE T& operator [](uint64_t i) { assert(i < _capacity); return _array[i]; } // for some insane reason, this works on 64-bit, but not on 32-bit
    BUN_FORCEINLINE const T& operator [](uint64_t i) const { assert(i < _capacity); return _array[i]; }
#endif
    inline const T* begin() const noexcept { return _array; }
    inline const T* end() const noexcept { return _array + _capacity; }
    inline T* begin() noexcept { return _array; }
    inline T* end() noexcept { return _array + _capacity; }
    inline CT size() const noexcept { return _capacity; }
    inline T* data() noexcept { return _array; }
    inline const T* data() const noexcept { return _array; }

    BUN_FORCEINLINE Array& operator=(const Array& copy) noexcept requires is_copy_constructible_or_incomplete_v<T>
    {
      BASE::_setLength(_array, _capacity, 0);
      BASE::_setCapacityDiscard(copy._capacity);
      BASE::_copy(_array, copy._array, _capacity);
      return *this;
    }
    BUN_FORCEINLINE Array& operator=(Array&& mov) noexcept
    {
      BASE::_setLength(_array, _capacity, 0);
      BASE::operator=(std::move(mov));
      return *this;
    }
    BUN_FORCEINLINE Array& operator=(std::span<const T> copy) noexcept requires is_copy_constructible_or_incomplete_v<T>
    {
      Set(copy);
      return *this;
    }
    BUN_FORCEINLINE Array& operator +=(const Array& add) noexcept requires is_copy_constructible_or_incomplete_v<T>
    {
      CT old = _capacity;
      BASE::_setCapacity(_capacity + add._capacity, _capacity);
      BASE::_copy(_array + old, add._array, add._capacity);
      return *this;
    }
    BUN_FORCEINLINE Array operator +(const Array& add) const noexcept requires is_copy_constructible_or_incomplete_v<T> { Array r(*this); return (r += add); }

    using SerializerArray = std::conditional_t<internal::is_pair_array<T>::value, void, T>;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id)
    {
      if constexpr (!internal::is_pair_array<T>::value)
        s.template EvaluateArray<Array, T, &_serializeAdd, CT, &Array::SetCapacity>(*this, _capacity, id);
      else
        s.template EvaluateKeyValue<Array>(*this, [this](Serializer<Engine>& e, const char* name) { _serializeInsert(e, name); });
    }

  protected:
    template<typename Engine, bool U = internal::is_pair_array<T>::value>
    inline std::enable_if_t<U> _serializeInsert(Serializer<Engine>& e, const char* name)
    {
      T pair;
      std::get<0>(pair) = name;
      Serializer<Engine>::template ActionBind<remove_cvref_t<std::tuple_element_t<1, T>>>::Parse(e, std::get<1>(pair), name);
      Add(pair);
    }
    template<typename Engine>
    inline static void _serializeAdd(Serializer<Engine>& e, Array& obj, int& n)
    {
      obj.SetCapacity(obj.Capacity() + 1);
      Serializer<Engine>::template ActionBind<T>::Parse(e, obj.Back(), 0);
    }
    template<typename U>
    inline void _insert(U&& item, CT index)
    {
      CT length = _capacity;
      BASE::_setCapacity(_capacity + 1, _capacity);
      BASE::_insert(_array, length, index, std::forward<U>(item));
    }
  };

  template<typename T, typename... S> // You should specify the axes in the order Z, Y, X, since the LAST one is contiguous.
  class ArrayMultiRef
  {
    template<int N, typename Arg, typename... Args>
    static void _getIndice(ptrdiff_t& i, std::tuple<S...>& t, Arg arg, Args... args)
    {
      if constexpr (N == 0)
      {
        i = arg;
        _getIndice<N + 1, Args...>(i, t, args...);
      }
      else if constexpr (sizeof...(Args) > 0)
      {
        i = arg + std::get<N>(t) * i;
        _getIndice<N + 1, Args...>(i, t, args...);
      }
      else
        i = arg + std::get<N>(t) * i;
    }

  public:
    ArrayMultiRef(const ArrayMultiRef&) = default;
    ArrayMultiRef(T* p, S... s) : _indices(s...), _p(p) {}
    BUN_FORCEINLINE ptrdiff_t GetIndice(S... s) { ptrdiff_t i; _getIndice<0, S...>(i, _indices, s...); return i; }

    inline bool operator !() const noexcept { return !_p; }
    inline bool operator ==(const T* right) const noexcept { return _p == right; }
    inline bool operator !=(const T* right) const noexcept { return _p != right; }
    inline operator T* () noexcept { return _p; }
    inline operator const T* () const noexcept { return _p; }
    inline T* operator->() noexcept { return _p; }
    inline const T* operator->() const noexcept { return _p; }
    inline T& operator()(S... s) noexcept { return _p[GetIndice(s...)]; }
    inline const T& operator()(S... s) const noexcept { return _p[GetIndice(s...)]; }

  protected:
    T* _p;
    std::tuple<S...> _indices;
  };

  namespace internal {
    // A stack-based variable array
    template<typename T>
    class VariableArray
    {
      static const size_t HEAPFLAG = (size_t(1) << ((sizeof(size_t) << 3) - 1));
      static const size_t HEAPMASK = ~HEAPFLAG;
    public:
      VariableArray(VariableArray&&) = delete;
      VariableArray(const VariableArray&) = delete;
      VariableArray(size_t n, T* p) : _p(p ? p : (T*)malloc(n * sizeof(T))), _heap((size_t(!p) << ((sizeof(size_t) << 3) - 1)) | (n & HEAPMASK)) {
        if constexpr (!std::is_trivially_default_constructible<T>::value)
          for (size_t i = 0; i < n; ++i)
            new(_p + i) T();
      }
      ~VariableArray()
      {
        if constexpr (!std::is_trivially_destructible<T>::value)
        {
          size_t n = _heap & HEAPMASK;
          for (size_t i = 0; i < n; ++i)
            _p[i].~T();
        }
        if (_heap & HEAPFLAG)
          free(_p);
      }

      BUN_FORCEINLINE size_t Length() const noexcept { return _heap & HEAPMASK; }
      BUN_FORCEINLINE const T* begin() const noexcept { return _p; }
      BUN_FORCEINLINE const T* end() const noexcept { return _p + Length(); }
      BUN_FORCEINLINE T* begin() noexcept { return _p; }
      BUN_FORCEINLINE T* end() noexcept { return _p + Length(); }
      BUN_FORCEINLINE const T& Front() const { assert(Length() > 0); return _p[0]; }
      BUN_FORCEINLINE const T& Back() const { assert(Length() > 0); return _p[Length() - 1]; }
      BUN_FORCEINLINE T& Front() { assert(Length() > 0); return _p[0]; }
      BUN_FORCEINLINE T& Back() { assert(Length() > 0); return _p[Length() - 1]; }
      inline size_t size() const noexcept { return Length(); }
      inline T* data() noexcept { return _p; }
      inline const T* data() const noexcept { return _p; }
      inline bool operator !() const noexcept { return !_p; }
      inline bool operator ==(const T* right) const noexcept { return _p == right; }
      inline bool operator !=(const T* right) const noexcept { return _p != right; }
      inline operator T* () noexcept { return _p; }
      inline operator const T* () const noexcept { return _p; }
      inline T* operator->() noexcept { return _p; }
      inline const T* operator->() const noexcept { return _p; }
      inline T& operator*() noexcept { return *_p; }
      inline const T& operator*() const noexcept { return *_p; }

      VariableArray& operator=(VariableArray&&) = delete;
      VariableArray& operator=(const VariableArray&) = delete;

    protected:
      T* _p;
      size_t _heap;
    };
  }
}

#define VARARRAY(Type,Name,n) bun::internal::VariableArray<Type> Name((n), (((n) * sizeof(Type)) > 0xFFFF) ? nullptr : (Type*)ALLOCA((n)*sizeof(Type)));

#endif
