// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ARRAY_H__BSS__
#define __ARRAY_H__BSS__

#include "Alloc.h"
#include "bss_util.h"
#include <string.h>
#include <initializer_list>

namespace bss {
  template<class Engine>
  class Serializer;

  // Represents an array slice, similar to span<T>.
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
    inline const Slice operator()(CType begin) const { return operator()(begin, length); }
    inline const Slice operator()(CType begin, CType end) const { return operator()(begin, length); }
    inline Slice operator()(CType begin) { return operator()(begin, length); }
    inline Slice operator()(CType begin, CType end)
    {
      assert(abs(begin) < length);
      assert(end <= length);
      begin = bssMod(begin, length);
      if(end <= 0) end = length - end;
      assert(end >= start);
      return Slice(start + begin, end - begin);
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

#ifdef BSS_DEBUG
#define BSS_DEBUGFILL(p, old, n, Ty) if(p) memset(p + std::min(n, old), 0xfd, (size_t)(((n > old)?(n - old):(old - n))*sizeof(Ty)))
#else
#define BSS_DEBUGFILL(p, old, n, Ty)  
#endif

  enum ARRAY_TYPE : uint8_t { ARRAY_SIMPLE = 0, ARRAY_CONSTRUCT = 1, ARRAY_SAFE = 2, ARRAY_MOVE = 3 };

  // Handles the very basic operations of an array. Constructor management is done by classes that inherit this class.
  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StandardAllocator<T>>
  class BSS_COMPILER_DLLEXPORT ArrayBase : protected Alloc
  {
  public:
    using CT = CType; // There are cases when you need access to these types even if you don't inherit (see RandomQueue in bss_algo.h)
    using Ty = T;
    static_assert(std::is_integral<CT>::value, "CType must be integral");

    inline ArrayBase(ArrayBase&& mov) : Alloc(std::move(mov)), _array(mov._array), _capacity(mov._capacity)
    {
      mov._array = 0;
      mov._capacity = 0;
    }
    inline ArrayBase(CT capacity, const Alloc& copy) : Alloc(copy), _array(0), _capacity(0) { _array = _getAlloc(capacity, 0, 0); _capacity = capacity; }
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline ArrayBase(CT capacity, typename Alloc::policy_type* policy) : Alloc(policy), _array(0), _capacity(0) { _array = _getAlloc(capacity, 0, 0); _capacity = capacity; }
    inline explicit ArrayBase(CT capacity) : _array(0), _capacity(0) { _array = _getAlloc(capacity, 0, 0); _capacity = capacity; }
    inline ArrayBase() : _array(0), _capacity(0) {}
    inline ~ArrayBase()
    {
      _free(_array);
    }
    ArrayBase& operator=(ArrayBase&& mov) noexcept
    {
      _free(_array);
      Alloc::operator=(std::move(mov));
      _array = mov._array;
      _capacity = mov._capacity;
      mov._array = 0;
      mov._capacity = 0;
      return *this;
    }

  protected:
    BSS_FORCEINLINE void _setCapacity(CT capacity) noexcept
    { 
      assert((_capacity != 0) == (_array != 0));
      _array = _getAlloc(capacity, _array, _capacity);
      _capacity = capacity;
    }
    BSS_FORCEINLINE void _setCapacityDiscard(CT capacity) noexcept
    {
      _free(_array);
      _array = _getAlloc(capacity, 0, 0);
      _capacity = capacity;
    }

    static void _setLength(T* dest, CType old, CType n) noexcept
    {
      if constexpr(ArrayType == ARRAY_SIMPLE)
      {
        BSS_DEBUGFILL(dest, old, n, T);
      }
      else
      {
        for(CType i = n; i < old; ++i)
          dest[i].~T();
        BSS_DEBUGFILL(dest, old, n, T);
        for(CType i = old; i < n; ++i)
          new(dest + i) T();
      }
    }

    inline void _setCapacity(CT capacity, CT length) noexcept
    {
      if constexpr(ArrayType == ARRAY_SIMPLE || ArrayType == ARRAY_CONSTRUCT)
        _setCapacity(capacity);
      else if constexpr(ArrayType == ARRAY_SAFE || ArrayType == ARRAY_MOVE)
      {
        T* n = _getAlloc(capacity, 0, 0);
        if(n != nullptr)
          _copyMove(n, _array, bssmin(length, capacity));
        _free(_array);
        _array = n;
        _capacity = capacity;
      }
    }

    inline T* _getAlloc(CT n, T* prev, CT old) noexcept
    {
      assert((_capacity != 0) == (_array != 0));
      if(!n)
      {
        _free(prev);
        return 0;
      }
      return (T*)Alloc::allocate((size_t)n, prev, old);
    }
    BSS_FORCEINLINE void _free(T* p) noexcept { if(p) Alloc::deallocate(p, _capacity); }
    // static_assert(!std::is_polymorphic<T>::value, "memcpy can't be used on type with vtable"); // Can't use this check because it becomes impossible to declare an array of the type the array itself is in.
    static void _copyMove(T* BSS_RESTRICT dest, T* BSS_RESTRICT src, CType n) noexcept
    {
      if(dest == nullptr || src == nullptr || !n)
        return;
      assert(dest != src);

      if constexpr(ArrayType == ARRAY_SIMPLE || ArrayType == ARRAY_CONSTRUCT)
        memcpy(dest, src, sizeof(T)*n);
      else if constexpr(ArrayType == ARRAY_SAFE || ArrayType == ARRAY_MOVE)
      {
        for(CType i = 0; i < n; ++i)
          new(dest + i) T(std::move(src[i]));
        _setLength(src, n, 0);
      }
    }
    static void _copy(T* BSS_RESTRICT dest, const T* BSS_RESTRICT src, CType n) noexcept
    {
      if(dest == nullptr || src == nullptr || !n)
        return;
      assert(dest != src);

      if constexpr(ArrayType == ARRAY_SIMPLE)
        memcpy(dest, src, sizeof(T)*n);
      else if constexpr(ArrayType == ARRAY_CONSTRUCT || ArrayType == ARRAY_SAFE)
      {
        for(CType i = 0; i < n; ++i)
          new(dest + i) T(src[i]);
      }
      else if constexpr(ArrayType == ARRAY_MOVE)
        assert(false);
    }
    template<typename U>
    static void _insert(T* dest, CType length, CType index, U && item) noexcept
    {
      assert(index >= 0 && length >= index && dest != 0);

      if constexpr(ArrayType == ARRAY_SIMPLE || ArrayType == ARRAY_CONSTRUCT)
      {
        memmove(dest + index + 1, dest + index, sizeof(T)*(length - index));
        new(dest + index) T(std::forward<U>(item));
      }
      else if constexpr(ArrayType == ARRAY_SAFE || ArrayType == ARRAY_MOVE)
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
    static void _remove(T* dest, CType length, CType index) noexcept 
    { 
      assert(index >= 0 && length > index);
      if constexpr(ArrayType == ARRAY_SAFE || ArrayType == ARRAY_MOVE)
      {
        std::move<T*, T*>(dest + index + 1, dest + length, dest + index);
        dest[length - 1].~T();
      }
      else
      {
        if constexpr(ArrayType == ARRAY_CONSTRUCT)
          dest[index].~T();
        memmove(dest + index, dest + index + 1, sizeof(T)*(length - index - 1));
      }
    }

    T* _array;
    CT _capacity;
  };

  // Wrapper for underlying arrays that expose the array, making them independently usable without blowing up everything that inherits them
  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StandardAllocator<T>>
  class BSS_COMPILER_DLLEXPORT Array : protected ArrayBase<T, CType, ArrayType, Alloc>
  {
  protected:
    using BASE = ArrayBase<T, CType, ArrayType, Alloc>;
    using CT = typename BASE::CT;
    using Ty = typename BASE::Ty;
    using BASE::_array;
    using BASE::_capacity;

  public:
    inline Array(const Array& copy) : BASE(copy._capacity, copy) { BASE::_copy(_array, copy._array, _capacity); } // We have to declare this because otherwise its interpreted as deleted
    inline Array(Array&& mov) : BASE(std::move(mov)) {}
    inline Array(const std::initializer_list<T>& list) : BASE(list.size())
    {
      auto end = list.end();
      CT c = 0;
      for(auto i = list.begin(); i != end && c < _capacity; ++i)
        new(_array + (c++)) T(*i);
      BASE::_setCapacity(c, _capacity);
    }
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline Array(CT capacity, typename Alloc::policy_type* policy) : BASE(capacity, policy) { BASE::_setLength(_array, 0, _capacity); }
    inline explicit Array(CT capacity = 0) : BASE(capacity) { BASE::_setLength(_array, 0, _capacity); }
    inline explicit Array(const Slice<const T, CT>& slice) : BASE(slice.length) { BASE::_copy(_array, slice.begin, slice.length); }
    inline ~Array() { BASE::_setLength(_array, _capacity, 0); }
    BSS_FORCEINLINE CT Add(const T& item) { _insert(item, _capacity); return _capacity - 1; }
    BSS_FORCEINLINE CT Add(T&& item) { _insert(std::move(item), _capacity); return _capacity - 1; }
    BSS_FORCEINLINE void Remove(CT index)
    {
      BASE::_remove(_array, _capacity, index); // we don't bother reallocating the array because it got smaller, so we just ignore part of it.
      BASE::_setCapacity(_capacity - 1, _capacity);
    }
    BSS_FORCEINLINE void RemoveLast() { Remove(_capacity - 1); }
    BSS_FORCEINLINE void Insert(const T& item, CT index = 0) { _insert(item, index); }
    BSS_FORCEINLINE void Insert(T&& item, CT index = 0) { _insert(std::move(item), index); }
    BSS_FORCEINLINE void Set(const Slice<const T, CT>& slice) { Set(slice.start, slice.length); }
    BSS_FORCEINLINE void Set(const T* p, CT n)
    {
      BASE::_setLength(_array, _capacity, 0);
      BASE::_setCapacityDiscard(n);
      BASE::_copy(_array, p, _capacity);
    }
    BSS_FORCEINLINE bool Empty() const noexcept { return !_capacity; }
    BSS_FORCEINLINE void Clear() noexcept { SetCapacity(0); }
    inline Slice<T, CT> GetSlice() const noexcept { return Slice<T, CT>(_array, _capacity); }
    BSS_FORCEINLINE void SetCapacity(CT capacity) noexcept
    {
      if(capacity == _capacity) return;
      CT old = _capacity;
      if(capacity < _capacity) BASE::_setLength(_array, _capacity, capacity);
      BASE::_setCapacity(capacity, _capacity);
      if(capacity > old) BASE::_setLength(_array, old, capacity);
    }
    BSS_FORCEINLINE void SetCapacityDiscard(CT capacity) noexcept
    {
      if(capacity == _capacity) return;
      BASE::_setLength(_array, _capacity, 0);
      BASE::_setCapacityDiscard(capacity);
      BASE::_setLength(_array, 0, _capacity);
    }
    BSS_FORCEINLINE CT Capacity() const noexcept { return _capacity; }
    inline const T& Front() const noexcept { assert(_capacity > 0); return _array[0]; }
    inline T& Front() noexcept { assert(_capacity > 0); return _array[0]; }
    inline const T& Back() const noexcept { assert(_capacity > 0); return _array[_capacity - 1]; }
    inline T& Back() noexcept { assert(_capacity > 0); return _array[_capacity - 1]; }
    BSS_FORCEINLINE operator T*() noexcept { return _array; }
    BSS_FORCEINLINE operator const T*() const noexcept { return _array; }
#if defined(BSS_64BIT) && defined(BSS_DEBUG) 
    BSS_FORCEINLINE T& operator [](uint64_t i) { assert(i < _capacity); return _array[i]; } // for some insane reason, this works on 64-bit, but not on 32-bit
    BSS_FORCEINLINE const T& operator [](uint64_t i) const { assert(i < _capacity); return _array[i]; }
#endif
    inline const T* begin() const noexcept { return _array; }
    inline const T* end() const noexcept { return _array + _capacity; }
    inline T* begin() noexcept { return _array; }
    inline T* end() noexcept { return _array + _capacity; }

    BSS_FORCEINLINE Array& operator=(const Array& copy) noexcept
    {
      BASE::_setLength(_array, _capacity, 0);
      BASE::_setCapacityDiscard(copy._capacity);
      BASE::_copy(_array, copy._array, _capacity);
      return *this;
    }
    BSS_FORCEINLINE Array& operator=(Array&& mov) noexcept
    { 
      BASE::_setLength(_array, _capacity, 0);
      BASE::operator=(std::move(mov));
      return *this;
    }
    BSS_FORCEINLINE Array& operator=(const Slice<const T, CT>& copy) noexcept
    { 
      Set(copy);
      return *this;
    }
    BSS_FORCEINLINE Array& operator +=(const Array& add) noexcept
    {
      CT old = _capacity;
      BASE::_setCapacity(_capacity + add._capacity, _capacity);
      BASE::_copy(_array + old, add._array, add._capacity);
      return *this;
    }
    BSS_FORCEINLINE Array operator +(const Array& add) const noexcept { Array r(*this); return (r += add); }

    using SerializerArray = std::conditional_t<internal::is_pair_array<T>::value, void, T>;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id)
    {
      if constexpr(!internal::is_pair_array<T>::value)
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
    inline void _insert(U && item, CT index)
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
      static const size_t HEAPFLAG = (size_t(1) << ((sizeof(size_t) << 3) - 1));
      static const size_t HEAPMASK = ~HEAPFLAG;
    public:
      VariableArray(VariableArray&&) = delete;
      VariableArray(const VariableArray&) = delete;
      VariableArray(size_t n, T* p) : _p(p ? p : (T*)malloc(n * sizeof(T))), _heap((size_t(!p) << ((sizeof(size_t) << 3) - 1)) | (n&HEAPMASK)) {
        if constexpr(!std::is_trivially_default_constructible<T>::value)
          for(size_t i = 0; i < n; ++i)
            new(_p + i) T();
      }
      ~VariableArray()
      {
        if constexpr(!std::is_trivially_destructible<T>::value)
        {
          size_t n = _heap&HEAPMASK;
          for(size_t i = 0; i < n; ++i)
            _p[i].~T();
        }
        if(_heap&HEAPFLAG)
          free(_p); 
      }

      BSS_FORCEINLINE size_t Length() const noexcept { return _heap&HEAPMASK; }
      BSS_FORCEINLINE const T* begin() const noexcept { return _p; }
      BSS_FORCEINLINE const T* end() const noexcept { return _p + Length(); }
      BSS_FORCEINLINE T* begin() noexcept { return _p; }
      BSS_FORCEINLINE T* end() noexcept { return _p + Length(); }
      BSS_FORCEINLINE const T& Front() const { assert(Length() > 0); return _p[0]; }
      BSS_FORCEINLINE const T& Back() const { assert(Length() > 0); return _p[Length() - 1]; }
      BSS_FORCEINLINE T& Front() { assert(Length() > 0); return _p[0]; }
      BSS_FORCEINLINE T& Back() { assert(Length() > 0); return _p[Length() - 1]; }
      inline Slice<T, size_t> GetSlice() const noexcept { return Slice<T, size_t>(_p, Length()); }
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
      size_t _heap;
    };
  }
}

#define VARARRAY(Type,Name,n) bss::internal::VariableArray<Type> Name((n), (((n) * sizeof(Type)) > 0xFFFF) ? nullptr : (Type*)ALLOCA((n)*sizeof(Type)));

#endif
