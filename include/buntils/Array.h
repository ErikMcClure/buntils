// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __ARRAY_H__BUN__
#define __ARRAY_H__BUN__

#include "Alloc.h"
#include "buntils.h"
#include <string.h>
#include <initializer_list>
#include <span>

namespace bun {
  template<class Engine> class Serializer;

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

  // Handles the very basic operations of an array. Constructor management is done by classes that inherit this class.
  template<class T, typename Alloc = StandardAllocator<T>, typename RECURSIVE = T>
  class BUN_COMPILER_DLLEXPORT ArrayBase : protected Alloc
  {
  public:
    using Ty = T;

    inline ArrayBase(ArrayBase&& mov) : Alloc(std::move(mov)), _array(mov._array) { mov._array = std::span<T>(); }
    inline ArrayBase(size_t capacity, const Alloc& alloc) : Alloc(alloc) { _array = _getAlloc(capacity, _array); }
    inline explicit ArrayBase(size_t capacity)
      requires std::is_default_constructible_v<Alloc>
    {
      _array = _getAlloc(capacity, _array);
    }
    inline ArrayBase()
      requires std::is_default_constructible_v<Alloc>
    {}
    inline ~ArrayBase() { _dealloc(_array); }
    ArrayBase& operator=(ArrayBase&& mov) noexcept
    {
      _dealloc(_array);
      Alloc::operator=(std::move(mov));
      _array     = mov._array;
      mov._array = std::span<T>();
      return *this;
    }

  protected:
    BUN_FORCEINLINE void _setCapacity(size_t capacity) noexcept { _array = _getAlloc(capacity, _array); }
    BUN_FORCEINLINE void _setCapacityDiscard(size_t capacity) noexcept
    {
      _dealloc(_array);
      _array = _getAlloc(capacity, std::span<T>());
    }
    template<typename CType>
      requires std::is_integral<CType>::value
    static void _setLength(T* dest, CType old, CType n) noexcept
    {
      if constexpr(std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>)
      {
        for(CType i = n; i < old; ++i)
          dest[i].~T();
      }
#ifdef BUN_DEBUG
      if(dest)
        memset(dest + std::min(n, old), 0xfd, (size_t)(((n > old) ? (n - old) : (old - n)) * sizeof(T)));
#endif
      if constexpr(std::is_default_constructible_v<T> && !std::is_trivially_default_constructible_v<T>)
      {
        for(CType i = old; i < n; ++i)
          new(dest + i) T();
      }
    }

    inline void _setCapacity(size_t capacity, size_t length) noexcept
    {
      if constexpr(std::is_trivially_copyable_v<T>)
        _setCapacity(capacity);
      else
      {
        auto n = _getAlloc(capacity, std::span<T>());
        if(!n.empty())
          _copyMove(n.data(), _array.data(), bun_min(length, n.size()));
        _dealloc(_array);
        _array = n;
      }
    }

    inline std::span<T> _getAlloc(size_t n, std::span<T> prev) noexcept
    {
      if(!n)
      {
        _dealloc(prev);
        return std::span<T>();
      }
      if constexpr(std::is_trivially_copyable_v<T>)
        return standard_realloc<Alloc>(*this, n, prev);
      else
      {
        auto old = prev.size();
        auto p   = std::allocator_traits<Alloc>::allocate(*this, n);
        if(!prev.empty() && old > 0)
        {
          auto count = std::min(old, n);
          for(size_t i = 0; i < count; ++i)
            new(p + i) T(std::move(prev[i]));
          _dealloc(prev);
        }
        return std::span<T>(p, n);
      }
    }
    BUN_FORCEINLINE void _dealloc(std::span<T> p) noexcept
    {
      if(!p.empty())
        std::allocator_traits<Alloc>::deallocate(*this, p.data(), p.size());
    }

    template<typename CType>
      requires std::is_integral<CType>::value
    static void _copyMove(T* BUN_RESTRICT dest, T* BUN_RESTRICT src, CType n) noexcept
    {
      if(dest == nullptr || src == nullptr || !n)
        return;
      assert(dest != src);

      if constexpr(std::is_trivially_copyable_v<T>)
        memcpy(dest, src, sizeof(T) * n);
      else
      {
        for(CType i = 0; i < n; ++i)
          new(dest + i) T(std::move(src[i]));
        _setLength<CType>(src, n, 0);
      }
    }

    template<typename CType>
      requires std::is_integral<CType>::value
    static void _copy(T* BUN_RESTRICT dest, const T* BUN_RESTRICT src, CType n)
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      if(dest == nullptr || src == nullptr || !n)
        return;
      assert(dest != src);

      if constexpr(std::is_trivially_copyable_v<T>)
        memcpy(dest, src, sizeof(T) * n);
      else if constexpr(std::is_copy_constructible_v<RECURSIVE>)
      {
        for(CType i = 0; i < n; ++i)
          new(dest + i) T(src[i]);
      }
      else
        static_assert(std::is_void_v<T>, "_copy shouldn't exist");
    }

    template<typename U, typename CType>
      requires std::is_integral<CType>::value
    static void _insert(T* dest, CType length, CType index, U&& item) noexcept
    {
      assert(index >= 0 && length >= index && dest != 0);

      if constexpr(std::is_trivially_copyable_v<T>)
      {
        memmove(dest + index + 1, dest + index, sizeof(T) * (length - index));
        new(dest + index) T(std::forward<U>(item));
      }
      else
      {
        if(index < length)
        {
          new(dest + length) T(std::move(dest[length - 1]));
          std::move_backward<T*, T*>(dest + index, dest + length - 1, dest + length);
          dest[index] = std::forward<U>(item);
        }
        else
          new(dest + index) T(std::forward<U>(item));
      }
    }

    template<typename CType>
      requires std::is_integral<CType>::value
    static void _remove(T* dest, CType length, CType index) noexcept
    {
      assert(index >= 0 && length > index);
      if constexpr(!std::is_trivially_copyable_v<T>)
      {
        std::move<T*, T*>(dest + index + 1, dest + length, dest + index);
        dest[length - 1].~T();
      }
      else
      {
        if constexpr(std::is_destructible_v<T>)
          dest[index].~T();
        memmove(dest + index, dest + index + 1, sizeof(T) * (length - index - 1));
      }
    }
#pragma warning(push)
#pragma warning(disable : 4251)
    std::span<T> _array;
#pragma warning(pop)
  };

  // Wrapper for underlying arrays that expose the array, making them independently usable without blowing up everything
  // that inherits them
  template<class T, typename CType = size_t, typename Alloc = StandardAllocator<T>, typename RECURSIVE = T>
    requires std::is_unsigned<CType>::value
  class BUN_COMPILER_DLLEXPORT Array : protected ArrayBase<T, Alloc, RECURSIVE>
  {
  protected:
    using BASE = ArrayBase<T, Alloc, RECURSIVE>;
    using CT   = CType;
    using Ty   = typename BASE::Ty;
    using BASE::_array;

  public:
    inline Array(const Array& copy)
      requires is_copy_constructible_or_incomplete_v<RECURSIVE>
      : BASE(copy._array.size(), copy)
    {
      BASE::_copy(_array.data(), copy._array.data(), _array.size());
    }
    inline Array(Array&& mov) : BASE(std::move(mov)) {}
    inline Array(const std::initializer_list<T>& list) : BASE(list.size())
    {
      auto end = list.end();
      size_t c = 0;
      for(auto i = list.begin(); i != end && c < _array.size(); ++i)
        new(_array.data() + (c++)) T(*i);
      BASE::_setCapacity(c, _array.size());
    }
    inline Array(CT capacity, const Alloc& alloc) : BASE(capacity, alloc)
    {
      BASE::template _setLength<size_t>(_array.data(), 0, _array.size());
    }
    inline explicit Array(CT capacity)
      requires std::is_default_constructible_v<Alloc>
      : BASE(capacity)
    {
      BASE::template _setLength<size_t>(_array.data(), 0, _array.size());
    }
    inline explicit Array()
      requires std::is_default_constructible_v<Alloc>
      : BASE(0)
    {
      BASE::template _setLength<size_t>(_array.data(), 0, _array.size());
    }
    inline explicit Array(std::span<const T> s) : BASE(s.size()) { BASE::_copy(_array.data(), s.data(), s.size()); }
    inline ~Array() { BASE::template _setLength<size_t>(_array.data(), _array.size(), 0); }
    BUN_FORCEINLINE CT Add(const T& item)
    {
      _insert(item, _array.size());
      return _array.size() - 1;
    }
    BUN_FORCEINLINE CT Add(T&& item)
    {
      _insert(std::move(item), _array.size());
      return _array.size() - 1;
    }
    BUN_FORCEINLINE void Remove(CT index)
    {
      BASE::_remove(_array.data(), static_cast<CT>(_array.size()),
                    index); // we don't bother reallocating the array because it got smaller, so we just ignore part of it.
      BASE::_setCapacity(_array.size() - 1, _array.size());
    }
    BUN_FORCEINLINE void RemoveLast() { Remove(_array.size() - 1); }
    BUN_FORCEINLINE void Insert(const T& item, CT index = 0) { _insert(item, index); }
    BUN_FORCEINLINE void Insert(T&& item, CT index = 0) { _insert(std::move(item), index); }
    BUN_FORCEINLINE void Set(std::span<const T> s)
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      BASE::template _setLength<size_t>(_array.data(), _array.size(), 0);
      BASE::_setCapacityDiscard(s.size());
      BASE::_copy(_array.data(), s.data(), _array.size());
    }
    BUN_FORCEINLINE bool Empty() const noexcept { return !_array.size(); }
    BUN_FORCEINLINE void Clear() noexcept { SetCapacity(0); }
    BUN_FORCEINLINE void SetCapacity(size_t capacity) noexcept
    {
      if(capacity == _array.size())
        return;
      auto old = _array.size();
      if(capacity < _array.size())
        BASE::template _setLength<size_t>(_array.data(), _array.size(), capacity);
      BASE::_setCapacity(capacity, _array.size());
      if(capacity > old)
        BASE::template _setLength<size_t>(_array.data(), old, capacity);
    }
    BUN_FORCEINLINE void SetCapacityDiscard(size_t capacity) noexcept
    {
      if(capacity == _array.size())
        return;
      BASE::template _setLength<size_t>(_array.data(), _array.size(), 0);
      BASE::_setCapacityDiscard(capacity);
      BASE::template _setLength<size_t>(_array.data(), 0, _array.size());
    }
    BUN_FORCEINLINE size_t Capacity() const noexcept { return _array.size(); }
    inline const T& Front() const noexcept
    {
      assert(_array.size() > 0);
      return _array[0];
    }
    inline T& Front() noexcept
    {
      assert(_array.size() > 0);
      return _array[0];
    }
    inline const T& Back() const noexcept
    {
      assert(_array.size() > 0);
      return _array[_array.size() - 1];
    }
    inline T& Back() noexcept
    {
      assert(_array.size() > 0);
      return _array[_array.size() - 1];
    }
    BUN_FORCEINLINE const T& operator[](CType i) const
    {
      assert(i < size());
      return _array[i];
    }
    BUN_FORCEINLINE T& operator[](CType i)
    {
      assert(i < size());
      return _array[i];
    }
    inline const T* begin() const noexcept { return _array.data(); }
    inline const T* end() const noexcept { return _array.data() + _array.size(); }
    inline T* begin() noexcept { return _array.data(); }
    inline T* end() noexcept { return _array.data() + _array.size(); }
    inline CT size() const noexcept { return static_cast<CT>(_array.size()); }
    inline T* data() noexcept { return _array.data(); }
    inline const T* data() const noexcept { return _array.data(); }

    BUN_FORCEINLINE Array& operator=(const Array& copy) noexcept
      requires is_copy_constructible_or_incomplete_v<RECURSIVE>
    {
      BASE::template _setLength<size_t>(_array.data(), _array.size(), 0);
      BASE::_setCapacityDiscard(copy._array.size());
      BASE::_copy(_array.data(), copy._array.data(), _array.size());
      return *this;
    }
    BUN_FORCEINLINE Array& operator=(Array&& mov) noexcept
    {
      BASE::template _setLength<size_t>(_array.data(), _array.size(), 0);
      BASE::operator=(std::move(mov));
      return *this;
    }
    BUN_FORCEINLINE Array& operator=(std::span<const T> copy) noexcept
      requires is_copy_constructible_or_incomplete_v<RECURSIVE>
    {
      Set(copy);
      return *this;
    }
    BUN_FORCEINLINE Array& operator+=(const Array& add) noexcept
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      auto old = _array.size();
      BASE::_setCapacity(_array.size() + add._array.size(), _array.size());
      BASE::_copy(_array.data() + old, add._array.data(), add._array.size());
      return *this;
    }
    BUN_FORCEINLINE Array operator+(const Array& add) const noexcept
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      Array r(*this);
      return (r += add);
    }

    using SerializerArray = std::conditional_t<internal::is_pair_array<T>::value, void, T>;
    template<typename Engine> void Serialize(Serializer<Engine>& s, const char* id)
    {
      if constexpr(!internal::is_pair_array<T>::value)
        s.template EvaluateArray<Array, T, &_serializeAdd, CT, &Array::SetCapacity>(*this, _array.size(), id);
      else
        s.template EvaluateKeyValue<Array>(*this,
                                           [this](Serializer<Engine>& e, const char* name) { _serializeInsert(e, name); });
    }

  protected:
    template<typename Engine, bool U = internal::is_pair_array<T>::value>
    inline std::enable_if_t<U> _serializeInsert(Serializer<Engine>& e, const char* name)
    {
      T pair;
      std::get<0>(pair) = name;
      Serializer<Engine>::template ActionBind<std::remove_cvref_t<std::tuple_element_t<1, T>>>::Parse(e, std::get<1>(pair),
                                                                                                      name);
      Add(pair);
    }
    template<typename Engine> inline static void _serializeAdd(Serializer<Engine>& e, Array& obj, int& n)
    {
      obj.SetCapacity(obj.Capacity() + 1);
      Serializer<Engine>::template ActionBind<T>::Parse(e, obj.Back(), 0);
    }
    template<typename U> inline void _insert(U&& item, CT index)
    {
      CT length = static_cast<CT>(_array.size());
      BASE::_setCapacity(_array.size() + 1, _array.size());
      BASE::_insert(_array.data(), length, index, std::forward<U>(item));
    }
  };

  template<typename T, typename... S> // You should specify the axes in the order Z, Y, X, since the LAST one is contiguous.
  class ArrayMultiRef
  {
    template<int N, typename Arg, typename... Args>
    static void _getIndice(ptrdiff_t& i, std::tuple<S...>& t, Arg arg, Args... args)
    {
      if constexpr(N == 0)
      {
        i = arg;
        _getIndice<N + 1, Args...>(i, t, args...);
      }
      else if constexpr(sizeof...(Args) > 0)
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
    BUN_FORCEINLINE ptrdiff_t GetIndice(S... s)
    {
      ptrdiff_t i;
      _getIndice<0, S...>(i, _indices, s...);
      return i;
    }

    inline bool operator!() const noexcept { return !_p; }
    inline bool operator==(const T* right) const noexcept { return _p == right; }
    inline bool operator!=(const T* right) const noexcept { return _p != right; }
    inline T& operator()(S... s) noexcept { return _p[GetIndice(s...)]; }
    inline const T& operator()(S... s) const noexcept { return _p[GetIndice(s...)]; }

  protected:
    T* _p;
    std::tuple<S...> _indices;
  };

  namespace internal {
    // A stack-based variable array
    template<typename T> class VariableArray
    {
      static const size_t HEAPFLAG = (size_t(1) << ((sizeof(size_t) << 3) - 1));
      static const size_t HEAPMASK = ~HEAPFLAG;

    public:
      VariableArray(VariableArray&&)      = delete;
      VariableArray(const VariableArray&) = delete;
      VariableArray(size_t n, T* p) :
        _p(p ? p : (T*)malloc(n * sizeof(T))), _heap((size_t(!p) << ((sizeof(size_t) << 3) - 1)) | (n & HEAPMASK))
      {
        if constexpr(!std::is_trivially_default_constructible<T>::value)
          for(size_t i = 0; i < n; ++i)
            new(_p + i) T();
      }
      ~VariableArray()
      {
        if constexpr(!std::is_trivially_destructible<T>::value)
        {
          size_t n = _heap & HEAPMASK;
          for(size_t i = 0; i < n; ++i)
            _p[i].~T();
        }
        if(_heap & HEAPFLAG)
          free(_p);
      }

      BUN_FORCEINLINE const T* begin() const noexcept { return _p; }
      BUN_FORCEINLINE const T* end() const noexcept { return _p + size(); }
      BUN_FORCEINLINE T* begin() noexcept { return _p; }
      BUN_FORCEINLINE T* end() noexcept { return _p + size(); }
      BUN_FORCEINLINE const T& Front() const
      {
        assert(size() > 0);
        return _p[0];
      }
      BUN_FORCEINLINE const T& Back() const
      {
        assert(size() > 0);
        return _p[size() - 1];
      }
      BUN_FORCEINLINE T& Front()
      {
        assert(size() > 0);
        return _p[0];
      }
      BUN_FORCEINLINE T& Back()
      {
        assert(size() > 0);
        return _p[size() - 1];
      }
      inline size_t size() const noexcept { return _heap & HEAPMASK; }
      inline T* data() noexcept { return _p; }
      inline const T* data() const noexcept { return _p; }
      inline bool operator!() const noexcept { return !_p; }
      inline bool operator==(const T* right) const noexcept { return _p == right; }
      inline bool operator!=(const T* right) const noexcept { return _p != right; }
      BUN_FORCEINLINE const T& operator[](size_t i) const
      {
        assert(i < size());
        return _p[i];
      }
      BUN_FORCEINLINE T& operator[](size_t i)
      {
        assert(i < size());
        return _p[i];
      }
      inline T& operator*() noexcept { return *_p; }
      inline const T& operator*() const noexcept { return *_p; }

      VariableArray& operator=(VariableArray&&)      = delete;
      VariableArray& operator=(const VariableArray&) = delete;

    protected:
      T* _p;
      size_t _heap;
    };
  }
}

#define VARARRAY(Type, Name, n)                \
  bun::internal::VariableArray<Type> Name((n), \
                                          (((n) * sizeof(Type)) > 0xFFFF) ? nullptr : (Type*)ALLOCA((n) * sizeof(Type)));

#endif
