// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ARRAY_H__BSS__
#define __ARRAY_H__BSS__

#include "Alloc.h"
#include <string.h>
#include <initializer_list>

namespace bss {
  template<class T, typename CType = size_t>
  struct BSS_COMPILER_DLLEXPORT Slice
  {
    inline Slice(const Slice& copy) : start(copy.start), length(copy.length) {}
    inline Slice(T* s, CType l) : start(s), length(l) {}
    template<int N>
    inline explicit Slice(T (&s)[N]) : start(s), length(N) {}
    inline Slice() {}
    inline Slice& operator=(const Slice& right)
    { 
      start = right.start;
      length = right.length;
      return *this;
    }
    inline Slice& operator++() // prefix
    { 
      assert(length > 0);
      ++start;
      --length;
      return *this;
    }
    inline Slice operator++(int) //postfix
    { 
      assert(length > 0); 
      Slice r = *this;
      ++start;
      --length;
      return r; 
    }
    inline const Slice operator()(CType start) const { return operator()(start, length); }
    inline const Slice operator()(CType start, CType end) const { return operator()(start, length); }
    inline Slice operator()(CType start) { return operator()(start, length); }
    inline Slice operator()(CType start, CType end)
    {
      assert(abs(start) < length);
      assert(end <= length);
      start = bssMod(start, length);
      if(end <= 0) end = length - end;
      assert(end >= start);
      return Slice(start + start, end - start);
    }
    inline bool operator!() const noexcept { return !start || !length; }
    inline operator Slice<const T>() const noexcept { return Slice<const T>(start, length); }

    BSS_FORCEINLINE bool Valid() const noexcept { return start != 0 && length != 0; }
    BSS_FORCEINLINE const T& Front() const { assert(length > 0); return start[0]; }
    BSS_FORCEINLINE const T& Back() const { assert(length > 0); return start[length - 1]; }
    BSS_FORCEINLINE T& Front() { assert(length > 0); return start[0]; }
    BSS_FORCEINLINE T& Back() { assert(length > 0); return start[length - 1]; }
    BSS_FORCEINLINE const T* begin() const noexcept { return start; }
    BSS_FORCEINLINE const T* end() const noexcept { return start + length; }
    BSS_FORCEINLINE T* begin() noexcept { return start; }
    BSS_FORCEINLINE T* end() noexcept { return start + length; }
    BSS_FORCEINLINE T& operator [](CType i) noexcept { assert(i < length); return start[i]; }
    BSS_FORCEINLINE const T& operator [](CType i) const noexcept { assert(i < length); return start[i]; }

    T* start;
    CType length;
  };

  // Helper function for inserting a range into a simple array
  template<class T, typename CType = size_t>
  inline void InsertRangeSimple(T* dest, CType length, CType index, const T* src, CType srcsize) noexcept
  {
    assert(index >= 0 && length >= index && dest != 0);
    memmove(dest + index + srcsize, dest + index, sizeof(T)*(length - index));
    memcpy(dest + index, src, sizeof(T)*srcsize);
  }

  template<class T, typename CType = size_t>
  inline void RemoveRangeSimple(T* dest, CType length, CType index, CType range) noexcept
  {
    assert(index >= 0 && length > index && dest != 0);
    memmove(dest + index, dest + index + range, sizeof(T)*(length - index - range));
  }

  enum ARRAY_TYPE : uint8_t { ARRAY_SIMPLE = 0, ARRAY_CONSTRUCT = 1, ARRAY_SAFE = 2, ARRAY_MOVE = 3 };

  namespace internal {
    template<class U, typename UType, ARRAY_TYPE ArrayType, typename V>
    struct ArrayInternal;
  }

  // Handles the very basic operations of an array. Constructor management is done by classes that inherit this class.
  template<class T, typename CType = size_t, typename Alloc = StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT ArrayBase
  {
  public:
    typedef CType CT_; // There are cases when you need access to these types even if you don't inherit (see RandomQueue in bss_algo.h)
    typedef T T_;
    static_assert(std::is_integral<CType>::value, "CType must be integral");

    inline ArrayBase(ArrayBase&& mov) : _array(mov._array), _capacity(mov._capacity)
    {
      mov._array = 0;
      mov._capacity = 0;
    }
    inline explicit ArrayBase(CT_ capacity = 0) : _array(_getAlloc(capacity)), _capacity(capacity) {}
    inline ~ArrayBase()
    {
      _free(_array);
    }
    inline CT_ Capacity() const noexcept { return _capacity; }
    BSS_FORCEINLINE void SetCapacity(CT_ capacity) noexcept { _capacity = capacity; _array = _getAlloc(_capacity, _array); }
    BSS_FORCEINLINE void SetCapacityDiscard(CT_ capacity) noexcept
    {
      _free(_array);
      _capacity = capacity;
      _array = _getAlloc(_capacity, 0);
    }
    ArrayBase& operator=(ArrayBase&& mov) noexcept
    {
      _free(_array);
      _array = mov._array;
      _capacity = mov._capacity;
      mov._array = 0;
      mov._capacity = 0;
      return *this;
    }

  protected:
    static inline T* _getAlloc(CT_ n, T* prev = 0) noexcept
    {
      if(!n)
      {
        _free(prev);
        return 0;
      }
      return (T*)Alloc::allocate((size_t)n, prev);
    }
    static inline void _free(T* p) noexcept { if(p) Alloc::deallocate(p); }
    template<class U, typename UType, ARRAY_TYPE ArrayType, typename V>
    friend struct internal::ArrayInternal;

    T* _array;
    CType _capacity;
  };

#ifdef BSS_DEBUG
#define BSS_DEBUGFILL(p, old, n, Ty) if(p) memset(p + std::min(n, old), 0xfd, (size_t)(((n > old)?(n - old):(old - n))*sizeof(Ty)))
#else
#define BSS_DEBUGFILL(p, old, n, Ty)  
#endif

  namespace internal {
    template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<T>>
    struct BSS_COMPILER_DLLEXPORT ArrayInternal
    {
      static void _copyMove(T* BSS_RESTRICT dest, T* BSS_RESTRICT src, CType n) noexcept
      { 
        if(dest == nullptr)
          return;
        assert(dest != src);
        _copy(dest, src, n);
      }
      static void _copy(T* BSS_RESTRICT dest, const T* BSS_RESTRICT src, CType n) noexcept
      { 
        if(dest == nullptr || src == nullptr || !n)
          return;
        assert(dest != src);
        memcpy(dest, src, sizeof(T)*n); 
      }
      template<typename U>
      static void _insert(T* dest, CType length, CType index, U && item) noexcept
      {
        assert(index >= 0 && length >= index && dest != 0);
        memmove(dest + index + 1, dest + index, sizeof(T)*(length - index));
        new(dest + index) T(std::forward<U>(item));
      }
      static void _remove(T* dest, CType length, CType index) noexcept { assert(index >= 0 && length > index); memmove(dest + index, dest + index + 1, sizeof(T)*(length - index - 1)); }
      static void _setLength(T* dest, CType old, CType n) noexcept { BSS_DEBUGFILL(dest, old, n, T); }
      static void _setCapacity(ArrayBase<T, CType, Alloc>& dest, CType length, CType capacity) noexcept
      {
        if(capacity <= dest._capacity)
          dest._capacity = capacity;
        else
          dest.SetCapacity(capacity);
      }
    };

    template<class T, typename CType, typename Alloc>
    struct BSS_COMPILER_DLLEXPORT ArrayInternal<T, CType, ARRAY_CONSTRUCT, Alloc>
    {
      static void _copyMove(T* BSS_RESTRICT dest, T* BSS_RESTRICT src, CType num) noexcept
      { 
        if(dest == nullptr)
          return; 
        assert(dest != src);
        memcpy(dest, src, sizeof(T)*num);
      }
      static void _copy(T* BSS_RESTRICT dest, const T* BSS_RESTRICT src, CType num) noexcept
      {
        if(dest == nullptr || src == nullptr || !num) return;
        assert(dest != src);
        for(CType i = 0; i < num; ++i)
          new(dest + i) T(src[i]);
      }
      template<typename U>
      static void _insert(T* dest, CType length, CType index, U && item) noexcept { ArrayInternal<T, CType, ARRAY_SIMPLE, Alloc>::_insert(dest, length, index, std::forward<U>(item)); }
      static void _remove(T* dest, CType length, CType index) noexcept
      {
        dest[index].~T();
        assert(index >= 0 && length > index);
        memmove(dest + index, dest + index + 1, sizeof(T)*(length - index - 1));
      }
      static void _setLength(T* dest, CType old, CType n) noexcept
      {
        for(CType i = n; i < old; ++i)
          dest[i].~T();
        BSS_DEBUGFILL(dest, old, n, T);
        for(CType i = old; i < n; ++i)
          new(dest + i) T();
      }
      static void _setCapacity(ArrayBase<T, CType, Alloc>& dest, CType length, CType capacity) noexcept
      {
        dest.SetCapacity(capacity);
      }
    };

    template<class T, typename CType, typename Alloc>
    struct BSS_COMPILER_DLLEXPORT ArrayInternal<T, CType, ARRAY_SAFE, Alloc>
    {
      static void _copyMove(T* BSS_RESTRICT dest, T* BSS_RESTRICT src, CType n) noexcept
      {
        if(dest == nullptr || src == nullptr || !n) return;
        assert(dest != src);
        for(CType i = 0; i < n; ++i)
          new(dest + i) T(std::move(src[i]));
        _setLength(src, n, 0);
      }
      static void _copy(T* BSS_RESTRICT dest, const T* BSS_RESTRICT src, CType n) noexcept
      {
        if(dest == nullptr || src == nullptr || !n) return;
        assert(dest != src);
        for(CType i = 0; i < n; ++i)
          new(dest + i) T(src[i]);
      }
      template<typename U>
      static void _insert(T* dest, CType length, CType index, U && item) noexcept
      {
        assert(index >= 0 && length >= index);
        if(index < length)
        {
          new(dest + length) T(std::move(dest[length - 1]));
          std::move_backward<T*, T*>(dest + index, dest + length - 1, dest + length);
          dest[index] = std::forward<U>(item);
        }
        else
          new(dest + index) T(std::forward<U>(item));
      }
      static void _remove(T* dest, CType length, CType index) noexcept
      {
        assert(index >= 0 && length > index);
        std::move<T*, T*>(dest + index + 1, dest + length, dest + index);
        dest[length - 1].~T();
      }
      static void _setLength(T* dest, CType old, CType n) noexcept { ArrayInternal<T, CType, ARRAY_CONSTRUCT, Alloc>::_setLength(dest, old, n); }
      static void _setCapacity(ArrayBase<T, CType, Alloc>& dest, CType length, CType capacity) noexcept
      {
        T* n = ArrayBase<T, CType, Alloc>::_getAlloc(capacity, 0);
        if(n != nullptr) _copyMove(n, dest._array, bssmin(length, capacity));
        ArrayBase<T, CType, Alloc>::_free(dest._array);
        dest._array = n;
        dest._capacity = capacity;
      }
    };

    template<class T, typename CType, typename Alloc>
    struct BSS_COMPILER_DLLEXPORT ArrayInternal<T, CType, ARRAY_MOVE, Alloc>
    {
      typedef ArrayInternal<T, CType, ARRAY_SAFE, Alloc> SAFE;
      static void _copyMove(T* BSS_RESTRICT dest, T* BSS_RESTRICT src, CType n) noexcept { if(dest == nullptr) return; assert(dest != src); SAFE::_copyMove(dest, src, n); }
      static void _copy(T* BSS_RESTRICT dest, const T* BSS_RESTRICT src, CType n) noexcept { assert(false); }
      template<typename U>
      static void _insert(T* dest, CType length, CType index, U && item) noexcept { SAFE::_insert(dest, length, index, std::forward<U>(item)); }
      static void _remove(T* dest, CType length, CType index) noexcept { SAFE::_remove(dest, length, index); }
      static void _setLength(T* dest, CType old, CType n) noexcept { SAFE::_setLength(dest, old, n); }
      static void _setCapacity(ArrayBase<T, CType, Alloc>& dest, CType length, CType capacity) noexcept { SAFE::_setCapacity(dest, length, capacity); }
    };
  }

  // Wrapper for underlying arrays that expose the array, making them independently usable without blowing up everything that inherits them
  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT Array : protected ArrayBase<T, CType, Alloc>, protected internal::ArrayInternal<T, CType, ArrayType, Alloc>
  {
  protected:
    typedef internal::ArrayInternal<T, CType, ArrayType, Alloc> BASE;
    typedef ArrayBase<T, CType, Alloc> AT_;
    typedef typename AT_::CT_ CT_;
    typedef typename AT_::T_ T_;
    using AT_::_array;
    using AT_::_capacity;

  public:
    inline Array(const Array& copy) : AT_(copy._capacity) { BASE::_copy(_array, copy._array, _capacity); } // We have to declare this because otherwise its interpreted as deleted
    inline Array(Array&& mov) : AT_(std::move(mov)) {}
    inline Array(const std::initializer_list<T>& list) : AT_(list.size())
    {
      auto end = list.end();
      CType c = 0;
      for(auto i = list.begin(); i != end && c < _capacity; ++i)
        new(_array + (c++)) T(*i);
      _capacity = c;
    }
    inline explicit Array(CT_ capacity = 0) : AT_(capacity) { BASE::_setLength(_array, 0, _capacity); }
    inline explicit Array(const Slice<const T, CType>& slice) : AT_(slice.length) { BASE::_copy(_array, slice.begin, slice.length); }
    inline ~Array() { BASE::_setLength(_array, _capacity, 0); }
    BSS_FORCEINLINE CT_ Add(const T_& item) { _insert(item, _capacity); return _capacity - 1; }
    BSS_FORCEINLINE CT_ Add(T_&& item) { _insert(std::move(item), _capacity); return _capacity - 1; }
    BSS_FORCEINLINE void Remove(CT_ index)
    {
      BASE::_remove(_array, _capacity--, index); // we don't bother reallocating the array because it got smaller, so we just ignore part of it.
    }
    BSS_FORCEINLINE void RemoveLast() { Remove(_capacity - 1); }
    BSS_FORCEINLINE void Insert(const T_& item, CT_ index = 0) { _insert(item, index); }
    BSS_FORCEINLINE void Insert(T_&& item, CT_ index = 0) { _insert(std::move(item), index); }
    BSS_FORCEINLINE void Set(const Slice<const T, CType>& slice) { Set(slice.start, slice.length); }
    BSS_FORCEINLINE void Set(const T_* p, CT_ n)
    {
      BASE::_setLength(_array, _capacity, 0);
      AT_::SetCapacityDiscard(n);
      BASE::_copy(_array, p, _capacity);
    }
    BSS_FORCEINLINE bool Empty() const noexcept { return !_capacity; }
    BSS_FORCEINLINE void Clear() noexcept { SetCapacity(0); }
    inline Slice<T, CType> GetSlice() const noexcept { return Slice<T, CType>(_array, _capacity); }
    BSS_FORCEINLINE void SetCapacity(CT_ capacity) noexcept
    {
      if(capacity == _capacity) return;
      CT_ old = _capacity;
      if(capacity < _capacity) BASE::_setLength(_array, _capacity, capacity);
      BASE::_setCapacity(*this, _capacity, capacity);
      if(capacity > old) BASE::_setLength(_array, old, capacity);
    }
    BSS_FORCEINLINE void SetCapacityDiscard(CT_ capacity) noexcept
    {
      if(capacity == _capacity) return;
      BASE::_setLength(_array, _capacity, 0);
      AT_::SetCapacityDiscard(capacity);
      BASE::_setLength(_array, 0, _capacity);
    }
    BSS_FORCEINLINE CT_ Capacity() const noexcept { return _capacity; }
    inline const T_& Front() const noexcept { assert(_capacity > 0); return _array[0]; }
    inline T_& Front() noexcept { assert(_capacity > 0); return _array[0]; }
    inline const T_& Back() const noexcept { assert(_capacity > 0); return _array[_capacity - 1]; }
    inline T_& Back() noexcept { assert(_capacity > 0); return _array[_capacity - 1]; }
    BSS_FORCEINLINE operator T_*() noexcept { return _array; }
    BSS_FORCEINLINE operator const T_*() const noexcept { return _array; }
#if defined(BSS_64BIT) && defined(BSS_DEBUG) 
    BSS_FORCEINLINE T_& operator [](uint64_t i) { assert(i < _capacity); return _array[i]; } // for some insane reason, this works on 64-bit, but not on 32-bit
    BSS_FORCEINLINE const T_& operator [](uint64_t i) const { assert(i < _capacity); return _array[i]; }
#endif
    inline const T_* begin() const noexcept { return _array; }
    inline const T_* end() const noexcept { return _array + _capacity; }
    inline T_* begin() noexcept { return _array; }
    inline T_* end() noexcept { return _array + _capacity; }

    BSS_FORCEINLINE Array& operator=(const Array& copy) noexcept
    {
      BASE::_setLength(_array, _capacity, 0);
      AT_::SetCapacityDiscard(copy._capacity);
      BASE::_copy(_array, copy._array, _capacity);
      return *this;
    }
    BSS_FORCEINLINE Array& operator=(Array&& mov) noexcept
    { 
      BASE::_setLength(_array, _capacity, 0);
      AT_::operator=(std::move(mov));
      return *this;
    }
    BSS_FORCEINLINE Array& operator=(const Slice<const T, CType>& copy) noexcept
    { 
      Set(copy);
      return *this;
    }
    BSS_FORCEINLINE Array& operator +=(const Array& add) noexcept
    {
      CType old = _capacity;
      BASE::_setCapacity(*this, _capacity, _capacity + add._capacity);
      BASE::_copy(_array + old, add._array, add._capacity);
      return *this;
    }
    BSS_FORCEINLINE Array operator +(const Array& add) const noexcept { Array r(*this); return (r += add); }

  protected:
    template<typename U>
    inline void _insert(U && item, CType index)
    {
      CType length = _capacity;
      BASE::_setCapacity(*this, _capacity, _capacity + 1);
      BASE::_insert(_array, length, index, std::forward<U>(item));
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
        i = arg + std::get<N>(t)*i;
        _getIndice<N + 1, Args...>(i, t, args...);
      }
      else
        i = arg + std::get<N>(t)*i;
    }

  public:
    ArrayMultiRef(const ArrayMultiRef&) = default;
    ArrayMultiRef(T* p, S... s) : _indices(s...), _p(p) {}
    BSS_FORCEINLINE ptrdiff_t GetIndice(S... s) { ptrdiff_t i; _getIndice<0, S...>(i, _indices, s...); return i; }

    inline bool operator !() const noexcept { return !_p; }
    inline bool operator ==(const T* right) const noexcept { return _p == right; }
    inline bool operator !=(const T* right) const noexcept { return _p != right; }
    inline operator T*() noexcept { return _p; }
    inline operator const T*() const noexcept { return _p; }
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
    public:
      VariableArray(VariableArray&&) = delete;
      VariableArray(const VariableArray&) = delete;
      VariableArray(size_t n, T* p) : _p(p ? p : (T*)malloc(n * sizeof(T))), _heap(!p) {}
      ~VariableArray() { if(_heap) free(_p); }

      inline bool operator !() const noexcept { return !_p; }
      inline bool operator ==(const T* right) const noexcept { return _p == right; }
      inline bool operator !=(const T* right) const noexcept { return _p != right; }
      inline operator T*() noexcept { return _p; }
      inline operator const T*() const noexcept { return _p; }
      inline T* operator->() noexcept { return _p; }
      inline const T* operator->() const noexcept { return _p; }
      inline T& operator*() noexcept { return *_p; }
      inline const T& operator*() const noexcept { return *_p; }

      VariableArray& operator=(VariableArray&&) = delete;
      VariableArray& operator=(const VariableArray&) = delete;

    protected:
      T* _p;
      bool _heap;
    };
  }
}

#define VARARRAY(Type,Name,n) bss::internal::VariableArray<Type> Name((n), (((n) * sizeof(Type)) > 0xFFFF) ? nullptr : (Type*)ALLOCA((n)*sizeof(Type)));

#endif
