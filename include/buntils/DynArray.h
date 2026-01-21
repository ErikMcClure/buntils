// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __DYN_ARRAY_H__BUN__
#define __DYN_ARRAY_H__BUN__

#include "Array.h"
#include "BitField.h"
#include "buntils.h"

namespace bun {
  // Dynamic array
  template<class T, typename CType = size_t, typename Alloc = StandardAllocator<T>, typename RECURSIVE = T>
  class BUN_COMPILER_DLLEXPORT DynArray : protected ArrayBase<T, Alloc, RECURSIVE>
  {
  protected:
    using BASE = ArrayBase<T, Alloc, RECURSIVE>;
    using BASE::_array;
    template<class U, typename C, typename A> friend class StreamBufDynArray;

  public:
    using CT = CType;
    using Ty = typename BASE::Ty;

    inline DynArray(const DynArray& copy)
      requires std::is_copy_constructible_v<RECURSIVE>
      : BASE(copy._array.size(), copy), _length(copy._length)
    {
      BASE::_copy(_array.data(), copy._array.data(), _length);
    }
    inline DynArray(DynArray&& mov) : BASE(std::move(mov)), _length(mov._length) { mov._length = 0; }
    inline explicit DynArray(std::span<const T> s)
      requires(std::is_copy_constructible_v<RECURSIVE> && std::is_default_constructible_v<Alloc>)
      : BASE(s.size()), _length(s.size())
    {
      BASE::_copy(_array.data(), s.data(), s.size());
    }
    inline DynArray(CT capacity, const Alloc& alloc) : BASE(capacity, alloc), _length(0) {}
    inline explicit DynArray(CT capacity)
      requires std::is_default_constructible_v<Alloc>
      : BASE(capacity), _length(0)
    {}
    inline DynArray()
      requires std::is_default_constructible_v<Alloc>
      : BASE(0), _length(0)
    {}
    inline DynArray(const std::initializer_list<T>& list)
      requires std::is_default_constructible_v<Alloc>
      : BASE(list.size()), _length(0)
    {
      auto end = list.end();
      for(auto i = list.begin(); i != end && _length < _array.size(); ++i)
        new(_array.data() + (_length++)) T(*i);
    }
    inline ~DynArray() { BASE::template _setLength<CType>(_array.data(), _length, 0); }
    BUN_FORCEINLINE CT Add(const T& item)
    {
      _checkSize();
      new(_array.data() + _length) T(item);
      return _length++;
    }
    BUN_FORCEINLINE CT Add(T&& item)
    {
      _checkSize();
      new(_array.data() + _length) T(std::move(item));
      return _length++;
    }
    template<typename... Args> BUN_FORCEINLINE CT AddConstruct(Args&&... args)
    {
      _checkSize();
      new(_array.data() + _length) T(std::forward<Args>(args)...);
      return _length++;
    }
    inline void Remove(CT index)
    {
      assert(index < _length);
      BASE::_remove(_array.data(), _length--, index);
    }
    BUN_FORCEINLINE void RemoveLast() { Remove(_length - 1); }
    BUN_FORCEINLINE void Insert(const T& item, CT index = 0) { _insert(item, index); }
    BUN_FORCEINLINE void Insert(T&& item, CT index = 0) { _insert(std::move(item), index); }
    BUN_FORCEINLINE void Set(std::span<const T> slice)
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      BASE::template _setLength<CType>(_array.data(), _length, 0);
      if(slice.size() > _array.size())
        BASE::_setCapacityDiscard(slice.size());
      BASE::_copy(_array.data(), slice.data(), slice.size());
      _length = slice.size();
    }
    BUN_FORCEINLINE bool Empty() const { return !_length; }
    BUN_FORCEINLINE void Clear() { SetLength(0); }
    inline void SetLength(CT length)
    {
      if(length < _length)
        BASE::template _setLength<CType>(_array.data(), _length, length);
      if(length > _array.size())
        BASE::_setCapacity(static_cast<size_t>(length), static_cast<size_t>(_length));
      if(length > _length)
        BASE::template _setLength<CType>(_array.data(), _length, length);
      _length = length;
    }
    inline void SetCapacity(size_t capacity)
    {
      if(capacity > _array.size())
        BASE::_setCapacity(capacity, static_cast<size_t>(_length));
    }
    BUN_FORCEINLINE size_t Capacity() const { return _array.size(); }
    BUN_FORCEINLINE const T& Front() const
    {
      assert(_length > 0);
      return _array[0];
    }
    BUN_FORCEINLINE const T& Back() const
    {
      assert(_length > 0);
      return _array[_length - 1];
    }
    BUN_FORCEINLINE T& Front()
    {
      assert(_length > 0);
      return _array[0];
    }
    BUN_FORCEINLINE T& Back()
    {
      assert(_length > 0);
      return _array[_length - 1];
    }
    BUN_FORCEINLINE const T* begin() const noexcept { return _array.data(); }
    BUN_FORCEINLINE const T* end() const noexcept { return _array.data() + _length; }
    BUN_FORCEINLINE T* begin() noexcept { return _array.data(); }
    BUN_FORCEINLINE T* end() noexcept { return _array.data() + _length; }
    inline CT size() const noexcept { return _length; }
    inline T* data() noexcept { return _array.data(); }
    inline const T* data() const noexcept { return _array.data(); }
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
    inline DynArray& operator=(const Array<T, CT, Alloc>& copy)
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      BASE::template _setLength<CType>(_array.data(), _length, 0);
      if(copy.size() > _array.size())
        BASE::_setCapacityDiscard(copy.size());
      BASE::_copy(_array.data(), copy.data(), copy.size());
      _length = copy.size();
      return *this;
    }
    inline DynArray& operator=(Array<T, CT, Alloc>&& mov)
    {
      BASE::template _setLength<CType>(_array.data(), _length, 0);
      BASE::operator=(std::move(mov));
      _length = mov._array.size();
      return *this;
    }
    inline DynArray& operator=(const DynArray& copy)
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      BASE::template _setLength<CType>(_array.data(), _length, 0);
      if(copy._length > _array.size())
        BASE::_setCapacityDiscard(copy._array.size());
      BASE::_copy(_array.data(), copy._array.data(), copy._length);
      _length = copy._length;
      return *this;
    }
    inline DynArray& operator=(DynArray&& mov)
    {
      BASE::template _setLength<CType>(_array.data(), _length, 0);
      BASE::operator=(std::move(mov));
      _length     = mov._length;
      mov._length = 0;
      return *this;
    }
    inline DynArray& operator=(std::span<const T> copy)
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      Set(copy);
      return *this;
    }
    inline DynArray& operator+=(const DynArray& add)
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      BASE::_setCapacity(static_cast<size_t>(_length + add._length), static_cast<size_t>(_length));
      BASE::_copy(_array.data() + _length, add._array.data(), add._length);
      _length += add._length;
      return *this;
    }
    inline DynArray operator+(const DynArray& add) const
      requires std::is_copy_constructible_v<RECURSIVE>
    {
      DynArray r(_length + add._length);
      BASE::_copy(r._array.data(), _array.data(), _length);
      BASE::_copy(r._array.data() + _length, add._array.data(), add._length);
      r._length = _length + add._length;
      return r;
    }
    using SerializerArray = std::conditional_t<internal::is_pair_array<T>::value, void, T>;
    template<typename Engine> void Serialize(Serializer<Engine>& s, const char* id)
    {
      if constexpr(!internal::is_pair_array<T>::value)
        s.template EvaluateArray<DynArray, T, &_serializeAdd<Engine>, CT, &DynArray::SetLength>(*this, _length, id);
      else
        s.template EvaluateKeyValue<DynArray>(*this, [this](Serializer<Engine>& e, const char* name) {
          _serializeInsert(e, name);
        });
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
    template<typename Engine> inline static void _serializeAdd(Serializer<Engine>& e, DynArray& obj, int& n)
    {
      obj.AddConstruct();
      Serializer<Engine>::template ActionBind<T>::Parse(e, obj.Back(), 0);
    }
    template<typename U> inline void _insert(U&& data, CT index)
    {
      _checkSize();
      BASE::_insert(_array.data(), _length++, index, std::forward<U>(data));
    }
    BUN_FORCEINLINE void _checkSize()
    {
      if(_length >= _array.size())
        BASE::_setCapacity(T_FBNEXT(_array.size()), static_cast<size_t>(_length));
      assert(_length < _array.size());
    }

    CT _length;
  };

  template<typename CType, typename Alloc>
    requires std::is_unsigned<CType>::value
  class BUN_COMPILER_DLLEXPORT DynArray<bool, CType, Alloc> :
    protected ArrayBase<uint8_t, typename std::allocator_traits<Alloc>::template rebind_alloc<uint8_t>>
  {
  protected:
    using STORE  = uint8_t;
    using BASE   = ArrayBase<STORE, typename std::allocator_traits<Alloc>::template rebind_alloc<uint8_t>>;
    using CT     = CType;
    using Ty     = typename BASE::Ty;
    using BITREF = internal::_BIT_REF<STORE>;
    using BASE::_array;
    static const CT DIV_AMT = (sizeof(STORE) << 3);
    static const CT MOD_AMT = (sizeof(STORE) << 3) - 1;

  public:
    inline DynArray(const DynArray& copy) : BASE(copy._array.size()), _length(copy._length)
    {
      memcpy(_array.data(), copy._array.data(), copy._array.size() * sizeof(STORE));
    }
    inline DynArray(DynArray&& mov) : BASE(std::move(mov)), _length(mov._length) { mov._length = 0; }
    inline explicit DynArray(CT capacity) : BASE(_maxChunks(capacity) / DIV_AMT), _length(0) {}
    inline DynArray() : BASE(0), _length(0) {}
    inline DynArray(const std::initializer_list<bool>& list) : BASE(_maxChunks(list.size()) / DIV_AMT), _length(0)
    {
      auto end = list.end();
      for(auto i = list.begin(); i != end && _length < (_array.size() * DIV_AMT); ++i)
        Add(*i);
    }
    inline ~DynArray() {}
    BUN_FORCEINLINE CT Add(bool item)
    {
      _checkSize();
      GetBit(_length++) = item;
      return _length - 1;
    }
    BUN_FORCEINLINE CT AddConstruct()
    {
      _checkSize();
      _array[_length] = false;
      return _length++;
    }
    inline void Remove(CT index)
    {
      assert(index < _length);
      _shiftDelete(index);
      --_length;
    }
    BUN_FORCEINLINE void RemoveLast() { --_length; }
    BUN_FORCEINLINE void Insert(bool item, CT index = 0)
    {
      _shiftInsert(index);
      ++_length;
      GetBit(index) = item;
    }
    BUN_FORCEINLINE bool Empty() const { return !_length; }
    BUN_FORCEINLINE void Clear() { SetLength(0); }
    inline void SetLength(CT length)
    {
      if(length > (_array.size() * DIV_AMT))
        BASE::_setCapacity(static_cast<size_t>(_maxChunks(length) / DIV_AMT));
      for(CT i = _length; i < length; ++i)
        GetBit(i) = false;
      _length = length;
    }
    inline void SetCapacity(size_t capacity)
    {
      capacity = _maxChunks(capacity) / DIV_AMT;
      if(capacity > _array.size())
        SetCapacity(capacity);
    }
    inline void Flip()
    {
      for(CT i = 0; i < _length; ++i)
        _array[i] = ~_array[i];
    }
    CT CountBits(CT bitindex, CT length) const
    {
      if(!length)
        return 0;
      length += bitindex;
      assert(length <= _length);
      CT start    = bitindex / DIV_AMT;
      CT end      = _maxChunks(length) / DIV_AMT;
      STORE smask = ~((((STORE)1) << (bitindex - (start * DIV_AMT))) - 1);
      STORE emask = (STORE)(~0) >> ((end * DIV_AMT) - length);

      if(start == (--end))
        return BitCount<STORE>((_array[start] & smask) & emask);

      CT c = BitCount<STORE>(_array[start] & smask);
      c += BitCount<STORE>(_array[end] & emask);

      for(CT i = start + 1; i < end; ++i)
        c += BitCount<STORE>(_array[i]);

      return c;
    }
    BUN_FORCEINLINE size_t Capacity() const { return _array.size() * DIV_AMT; }
    BUN_FORCEINLINE bool Front() const
    {
      assert(_length > 0);
      return _array[0];
    }
    BUN_FORCEINLINE bool Back() const
    {
      assert(_length > 0);
      return _array[_length - 1];
    }
    BUN_FORCEINLINE BITREF Front()
    {
      assert(_length > 0);
      return GetBit(0);
    }
    BUN_FORCEINLINE BITREF Back()
    {
      assert(_length > 0);
      return GetBit(_length - 1);
    }
    BUN_FORCEINLINE bool operator[](CT index) const
    {
      assert(index < _length);
      return (_array[(index / DIV_AMT)] & (((STORE)1) << (index & MOD_AMT))) != (STORE)0;
    }
    BUN_FORCEINLINE BITREF operator[](CT index)
    {
      assert(index < _length);
      return GetBit(index);
    }
    // This gets the raw storage byte in such a way that unused bits are always set to zero. Useful for debugging.
    inline STORE GetRawByte(CT index) const
    {
      assert(index < _array.size());
      return (index < (_array.size() - 1)) ? _array[index] :
                                             (_array[index] & ((STORE)(~0) >> ((_array.size() * DIV_AMT) - _length)));
    }

    BUN_FORCEINLINE internal::_BIT_ITER<const BITREF> begin() const { return GetBit(0); }
    BUN_FORCEINLINE internal::_BIT_ITER<const BITREF> end() const { return GetBit(_length); }
    BUN_FORCEINLINE internal::_BIT_ITER<BITREF> begin() { return GetBit(0); }
    BUN_FORCEINLINE internal::_BIT_ITER<BITREF> end() { return GetBit(_length); }
    inline CT size() const noexcept { return _length; }

    inline DynArray& operator=(const DynArray& copy)
    {
      if(copy._length > (_array.size() * DIV_AMT))
        BASE::_setCapacityDiscard(copy._array.size());
      memcpy(_array.data(), copy._array.data(), _array.size() * sizeof(STORE));
      _length = copy._length;
      return *this;
    }
    inline DynArray& operator=(DynArray&& mov)
    {
      BASE::operator=(std::move(mov));
      _length     = mov._length;
      mov._length = 0;
      return *this;
    }

    using SerializerArray = Ty;
    template<typename Engine> void Serialize(Serializer<Engine>& s, const char* id)
    {
      s.template EvaluateArray<DynArray, bool, &_serializeAdd<Engine>, STORE, nullptr>(*this, _length, id);
    }

  protected:
    template<typename Engine> inline static void _serializeAdd(Serializer<Engine>& e, DynArray& obj, int& n)
    {
      bool b;
      Serializer<Engine>::template ActionBind<bool>::Parse(e, b, 0);
      obj.Add(b);
    }
    BUN_FORCEINLINE BITREF GetBit(CT bitindex) const
    {
      return BITREF(((STORE)1) << (bitindex & MOD_AMT), *(_array.data() + (bitindex / DIV_AMT)));
    }
    BUN_FORCEINLINE static CT _maxChunks(CT numbits) { return T_NEXTMULTIPLE(numbits, MOD_AMT); }
    inline void _shiftDelete(CT index)
    {
      CT ind      = index / DIV_AMT;
      CT off      = index & MOD_AMT;
      CT len      = _maxChunks(_length) / DIV_AMT;
      STORE mask  = (STORE)(~0) << off;
      _array[ind] = ((_array[ind] >> 1) & mask) | (_array[ind] & (~mask));

      for(CT i = ind + 1; i < len; ++i)
      {
        _array[i - 1] = (_array[i - 1] & ((STORE)(~0) >> 1)) | ((_array[i] & 1) << MOD_AMT);
        _array[i] >>= 1;
      }
    }
    inline void _shiftInsert(CT index)
    {
      _checkSize();
      CT ind      = index / DIV_AMT;
      CT off      = index & MOD_AMT;
      CT len      = _maxChunks(_length + 1) / DIV_AMT;
      STORE mask  = (STORE)(~0) << off;
      STORE store = _array[ind];
      _array[ind] = ((_array[ind] & mask) << 1) | (_array[ind] & (~mask));
      STORE test  = _array[ind];

      for(CT i = ind + 1; i < len; ++i)
      {
        _array[i] = (store >> MOD_AMT) | ((_array[i] << 1) & (STORE)(~1));
        store     = _array[i];
      }
    }

    BUN_FORCEINLINE void _checkSize()
    {
      if(_length >= (_array.size() * DIV_AMT))
        BASE::_setCapacity(T_FBNEXT(_array.size()));
      assert(_length < (_array.size() * DIV_AMT));
    }

    CT _length;
  };

  // A dynamic array that can dynamically adjust the size of each element
  template<typename CType, typename Alloc = StandardAllocator<std::byte>>
  class BUN_COMPILER_DLLEXPORT ArbitraryArray : protected ArrayBase<std::byte, Alloc>
  {
  protected:
    using BASE = ArrayBase<std::byte, Alloc>;
    using CT   = typename CType;
    using Ty   = typename BASE::Ty;
    using BASE::_array;

  public:
    inline ArbitraryArray(const ArbitraryArray& copy) :
      BASE(copy._array.size()), _length(copy._length), _element(copy._element)
    {
      memcpy(_array, copy._array, _length * _element);
    }
    inline ArbitraryArray(ArbitraryArray&& mov) : BASE(std::move(mov)), _length(mov._length), _element(mov._element) {}
    inline ArbitraryArray(CT size = 0, CT element = 1) : BASE(size * element), _length(0), _element(element) {}
    template<typename T> inline CT Add(const T& item)
    {
      if((_length * _element) >= _array.size())
        BASE::_setCapacity(T_FBNEXT(_length) * _element);
      memcpy(_array.data() + (_length * _element), &item, std::min<CT>(sizeof(T), _element));
      return _length++;
    }
    inline void Remove(CT index)
    {
      index *= _element;
      assert(_array.size() > 0 && index < _array.size());
      memmove(_array.data() + index, _array.data() + index + _element, _array.size() - index - _element);
      --_length;
    }
    inline void RemoveLast() { --_length; }
    inline void SetElement(const void* newarray, CT element, CT num) // num is a count of how many elements are in the array
    {
      if(newarray == (const void*)_array.data())
        return;

      _element = element;
      _length  = num;

      if((_length * _element) > _array.size())
        BASE::_setCapacityDiscard(static_cast<size_t>(_length * _element));

      if(!_array.empty())
        memcpy(_array.data(), newarray, element * num);
    }
    template<typename T> // num is a count of how many elements are in the array
    BUN_FORCEINLINE void SetElement(const T* newarray, CT num)
    {
      SetElement(newarray, sizeof(T), num);
    }
    template<typename T, int NUM> BUN_FORCEINLINE void SetElement(const T (&newarray)[NUM])
    {
      SetElement(newarray, sizeof(T), NUM);
    }
    template<typename T, int NUM> BUN_FORCEINLINE void SetElement(const std::array<T, NUM>& newarray)
    {
      SetElement(newarray.data(), sizeof(T), NUM);
    }
    void SetElement(CT element)
    {
      if(element == _element)
        return;

      CT capacity       = element * _length;
      std::byte* narray = !_length ? nullptr : (std::byte*)std::allocator_traits<Alloc>::allocate(*this, capacity);
      if(narray)
        memset(narray, 0, capacity);
      CT m = std::min<CT>(element, _element);

      for(CT i = 0; i < _length; ++i)
        memcpy(narray + (i * element), _array.data() + (i * _element), m);

      BASE::_dealloc(_array);
      _array   = std::span(narray, capacity);
      _element = element;
      assert(element > 0);
    }
    inline CT Element() const { return _element; }
    inline bool Empty() const { return !_length; }
    inline void Clear() { _length = 0; }
    inline void SetLength(CT length)
    {
      _length = length;
      length *= _element;
      if(length > _array.size())
        BASE::_setCapacity(static_cast<size_t>(length));
    }
    inline CT size() const { return _length; }
    // These are templatized accessors, but we don't do an assertion on if the size of T is the same as _element, because
    // there are many cases where this isn't actually true.
    template<typename T> inline const T& Front() const
    {
      assert(_length > 0);
      return _array[0];
    }
    template<typename T> inline const T& Back() const
    {
      assert(_length > 0);
      return _array[(_length - 1) * _element];
    }
    template<typename T> inline T& Front()
    {
      assert(_length > 0);
      return _array[0];
    }
    template<typename T> inline T& Back()
    {
      assert(_length > 0);
      return _array[(_length - 1) * _element];
    }
    template<typename T> inline const T* begin() const { return _array.data(); }
    template<typename T> inline const T* end() const { return _array.data() + (_length * _element); }
    template<typename T> inline T* begin() { return _array.data(); }
    template<typename T> inline T* end() { return _array.data() + (_length * _element); }
    template<typename T> inline const T& Get(CT index) const { return *reinterpret_cast<T*>(Get(index)); }
    template<typename T> inline T& Get(CT index) { return *reinterpret_cast<T*>(Get(index)); }
    BUN_FORCEINLINE const void* Get(CT index) const
    {
      assert(index < _length);
      return (_array.data() + (index * _element));
    }
    BUN_FORCEINLINE void* Get(CT index)
    {
      assert(index < _length);
      return (_array.data() + (index * _element));
    }

    inline const void* operator[](CT index) const { return Get(index); }
    inline void* operator[](CT index) { return Get(index); }
    inline ArbitraryArray& operator=(const ArbitraryArray& copy)
    {
      BASE::_setCapacityDiscard(copy._array.size());
      memcpy(_array.data(), copy._array.data(), _array.size());
      _length  = copy._length;
      _element = copy._element;
      return *this;
    }
    inline ArbitraryArray& operator=(ArbitraryArray&& mov)
    {
      BASE::operator=(std::move(mov));
      _length  = mov._length;
      _element = mov._element;
      return *this;
    }

  protected:
    CT _length;  // Total length of the array in number of elements
    CT _element; // Size of each element
  };
}

#endif
