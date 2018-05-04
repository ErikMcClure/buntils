// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __DYN_ARRAY_H__BSS__
#define __DYN_ARRAY_H__BSS__

#include "Array.h"
#include "BitField.h"
#include "bss_util.h"

namespace bss {
  // Dynamic array implemented using ArrayType (should only be used when constructors could potentially not be needed)
  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StandardAllocator<T>>
  class BSS_COMPILER_DLLEXPORT DynArray : protected ArrayBase<T, CType, ArrayType, Alloc>
  {
  protected:
    typedef ArrayBase<T, CType, ArrayType, Alloc> BASE;
    using BASE::_array;
    using BASE::_capacity;
    template<class U, typename C, ARRAY_TYPE AT, typename A>
    friend class StreamBufDynArray;

  public:
    typedef typename BASE::CT CT;
    typedef typename BASE::Ty Ty;

    inline DynArray(const DynArray& copy) : BASE(copy._capacity, copy), _length(copy._length) { BASE::_copy(_array, copy._array, _length); }
    inline DynArray(DynArray&& mov) : BASE(std::move(mov)), _length(mov._length) { mov._length = 0; }
    inline explicit DynArray(const Slice<const T, CT>& slice) : BASE(slice.length), _length(slice.length) { BASE::_copy(_array, slice.start, slice.length); }
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline DynArray(CT capacity, typename Alloc::policy_type* policy) : BASE(capacity, policy), _length(0) {}
    inline explicit DynArray(CT capacity) : BASE(capacity), _length(0) {}
    inline DynArray() : BASE(0), _length(0) {}
    inline DynArray(const std::initializer_list<T>& list) : BASE(list.size()), _length(0)
    {
      auto end = list.end();
      for(auto i = list.begin(); i != end && _length < _capacity; ++i)
        new(_array + (_length++)) T(*i);
    }
    inline ~DynArray() { BASE::_setLength(_array, _length, 0); }
    BSS_FORCEINLINE CT Add(const T& item) { _checkSize(); new(_array + _length) T(item); return _length++; }
    BSS_FORCEINLINE CT Add(T&& item) { _checkSize(); new(_array + _length) T(std::move(item)); return _length++; }
    template<typename... Args>
    BSS_FORCEINLINE CT AddConstruct(Args&&... args) { _checkSize(); new(_array + _length) T(std::forward<Args>(args)...); return _length++; }
    inline void Remove(CT index)
    {
      assert(index < _length);
      BASE::_remove(_array, _length--, index);
    }
    BSS_FORCEINLINE void RemoveLast() { Remove(_length - 1); }
    BSS_FORCEINLINE void Insert(const T& item, CT index = 0) { _insert(item, index); }
    BSS_FORCEINLINE void Insert(T&& item, CT index = 0) { _insert(std::move(item), index); }
    BSS_FORCEINLINE void Set(const Slice<const T, CT>& slice) { Set(slice.start, slice.length); }
    BSS_FORCEINLINE void Set(const T* p, CT n)
    {
      BASE::_setLength(_array, _length, 0);
      if(n > _capacity)
        BASE::_setCapacityDiscard(n);
      BASE::_copy(_array, p, n);
      _length = n;
    }
    BSS_FORCEINLINE bool Empty() const { return !_length; }
    BSS_FORCEINLINE void Clear() { SetLength(0); }
    inline void SetLength(CT length)
    {
      if(length < _length) BASE::_setLength(_array, _length, length);
      if(length > _capacity) BASE::_setCapacity(length, _length);
      if(length > _length) BASE::_setLength(_array, _length, length);
      _length = length;
    }
    inline void SetCapacity(CT capacity) { if(capacity > _capacity) BASE::_setCapacity(capacity, _length); }
    BSS_FORCEINLINE CT Length() const { return _length; }
    BSS_FORCEINLINE CT Capacity() const { return _capacity; }
    BSS_FORCEINLINE const T& Front() const { assert(_length > 0); return _array[0]; }
    BSS_FORCEINLINE const T& Back() const { assert(_length > 0); return _array[_length - 1]; }
    BSS_FORCEINLINE T& Front() { assert(_length > 0); return _array[0]; }
    BSS_FORCEINLINE T& Back() { assert(_length > 0); return _array[_length - 1]; }
    BSS_FORCEINLINE const T* begin() const noexcept { return _array; }
    BSS_FORCEINLINE const T* end() const noexcept { return _array + _length; }
    BSS_FORCEINLINE T* begin() noexcept { return _array; }
    BSS_FORCEINLINE T* end() noexcept { return _array + _length; }
    BSS_FORCEINLINE Slice<T, CT> GetSlice() const noexcept { return Slice<T, CT>(_array, _length); }
#if defined(BSS_64BIT) && defined(BSS_DEBUG) 
    BSS_FORCEINLINE T& operator [](uint64_t i) { assert(i < _length); return _array[i]; } // for some insane reason, this works on 64-bit, but not on 32-bit
    BSS_FORCEINLINE const T& operator [](uint64_t i) const { assert(i < _length); return _array[i]; }
#endif
    BSS_FORCEINLINE operator T*() { return _array; }
    BSS_FORCEINLINE operator const T*() const { return _array; }
    inline DynArray& operator=(const Array<T, CT, ArrayType, Alloc>& copy)
    {
      BASE::_setLength(_array, _length, 0);
      if(copy._capacity > _capacity)
        BASE::_setCapacityDiscard(copy._capacity);
      BASE::_copy(_array, copy._array, copy._capacity);
      _length = copy._capacity;
      return *this;
    }
    inline DynArray& operator=(Array<T, CT, ArrayType, Alloc>&& mov) { BASE::_setLength(_array, _length, 0); BASE::operator=(std::move(mov)); _length = mov._capacity; return *this; }
    inline DynArray& operator=(const DynArray& copy)
    {
      BASE::_setLength(_array, _length, 0);
      if(copy._length > _capacity)
        BASE::_setCapacityDiscard(copy._capacity);
      BASE::_copy(_array, copy._array, copy._length);
      _length = copy._length;
      return *this;
    }
    inline DynArray& operator=(DynArray&& mov) { BASE::_setLength(_array, _length, 0); BASE::operator=(std::move(mov)); _length = mov._length; mov._length = 0; return *this; }
    inline DynArray& operator=(const Slice<const T, CT>& copy) { Set(copy); return *this; }
    inline DynArray& operator +=(const DynArray& add)
    {
      BASE::_setCapacity(_length + add._length, _length);
      BASE::_copy(_array + _length, add._array, add._length);
      _length += add._length;
      return *this;
    }
    inline DynArray operator +(const DynArray& add) const
    {
      DynArray r(_length + add._length);
      BASE::_copy(r._array, _array, _length);
      BASE::_copy(r._array + _length, add._array, add._length);
      r._length = _length + add._length;
      return r;
    }
    typedef std::conditional_t<internal::is_pair_array<T>::value, void, T> SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id)
    {
      if constexpr(!internal::is_pair_array<T>::value)
        s.template EvaluateArray<DynArray, T, &_serializeAdd<Engine>, CT, &DynArray::SetLength>(*this, _length, id);
      else
        s.template EvaluateKeyValue<DynArray>(*this, [this](Serializer<Engine>& e, const char* name) { _serializeInsert(e, name); });
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
    inline static void _serializeAdd(Serializer<Engine>& e, DynArray& obj, int& n)
    {
      obj.AddConstruct();
      Serializer<Engine>::template ActionBind<T>::Parse(e, obj.Back(), 0);
    }
    template<typename U>
    inline void _insert(U && data, CT index)
    {
      _checkSize();
      BASE::_insert(_array, _length++, index, std::forward<U>(data));
    }
    BSS_FORCEINLINE void _checkSize()
    {
      if(_length >= _capacity)
        BASE::_setCapacity(T_FBNEXT(_capacity), _length);
      assert(_length < _capacity);
    }

    CT _length;
  };
  
  template<typename CType, ARRAY_TYPE ArrayType, typename Alloc>
  class BSS_COMPILER_DLLEXPORT DynArray<bool, CType, ArrayType, Alloc> : protected ArrayBase<uint8_t, CType, ArrayType, typename Alloc::template rebind<uint8_t>>
  {
  protected:
    typedef uint8_t STORE;
    typedef ArrayBase<STORE, CType, ArrayType, typename Alloc::template rebind<uint8_t>> BASE;
    typedef typename BASE::CT CT;
    typedef typename BASE::Ty Ty;
    typedef internal::_BIT_REF<STORE> BITREF;
    using BASE::_array;
    using BASE::_capacity;
    static const CT DIV_AMT = (sizeof(STORE) << 3);
    static const CT MOD_AMT = (sizeof(STORE) << 3) - 1;
    static_assert(std::is_unsigned<CType>::value, "CType must be an unsigned integral type.");

  public:
    inline DynArray(const DynArray& copy) : BASE(copy._capacity), _length(copy._length) { memcpy(_array, copy._array, copy._capacity * sizeof(STORE)); }
    inline DynArray(DynArray&& mov) : BASE(std::move(mov)), _length(mov._length) { mov._length = 0; }
    inline explicit DynArray(CT capacity) : BASE(_maxChunks(capacity) / DIV_AMT), _length(0) {}
    inline DynArray() : BASE(0), _length(0) {}
    inline DynArray(const std::initializer_list<bool>& list) : BASE(_maxChunks(list.size()) / DIV_AMT), _length(0)
    {
      auto end = list.end();
      for(auto i = list.begin(); i != end && _length < (_capacity*DIV_AMT); ++i)
        Add(*i);
    }
    inline ~DynArray() {}
    BSS_FORCEINLINE CT Add(bool item) { _checkSize(); GetBit(_length++) = item; return _length - 1; }
    BSS_FORCEINLINE CT AddConstruct() { _checkSize(); _array[_length] = false; return _length++; }
    inline void Remove(CT index)
    {
      assert(index < _length);
      _shiftDelete(index);
      --_length;
    }
    BSS_FORCEINLINE void RemoveLast() { --_length; }
    BSS_FORCEINLINE void Insert(bool item, CT index = 0)
    {
      _shiftInsert(index);
      ++_length;
      GetBit(index) = item;
    }
    BSS_FORCEINLINE bool Empty() const { return !_length; }
    BSS_FORCEINLINE void Clear() { SetLength(0); }
    inline void SetLength(CT length)
    {
      if(length > (_capacity*DIV_AMT)) BASE::_setCapacity(_maxChunks(length) / DIV_AMT);
      for(CT i = _length; i < length; ++i) GetBit(i) = false;
      _length = length;
    }
    inline void SetCapacity(CT capacity) { capacity = _maxChunks(capacity) / DIV_AMT; if(capacity > _capacity) SetCapacity(capacity); }
    inline void Flip() { for(CT i = 0; i < _capacity; ++i) _array[i] = ~_array[i]; }
    CT CountBits(CT bitindex, CT length) const
    {
      if(!length) return 0;
      length += bitindex;
      assert(length <= _length);
      CT start = bitindex / DIV_AMT;
      CT end = _maxChunks(length) / DIV_AMT;
      STORE smask = ~((((STORE)1) << (bitindex - (start*DIV_AMT))) - 1);
      STORE emask = (STORE)(~0) >> ((end*DIV_AMT) - length);

      if(start == (--end))
        return BitCount<STORE>((_array[start] & smask)&emask);

      CT c = BitCount<STORE>(_array[start] & smask);
      c += BitCount<STORE>(_array[end] & emask);

      for(CT i = start + 1; i < end; ++i)
        c += BitCount<STORE>(_array[i]);

      return c;
    }
    BSS_FORCEINLINE CT Length() const { return _length; }
    BSS_FORCEINLINE CT Capacity() const { return _capacity*DIV_AMT; }
    BSS_FORCEINLINE bool Front() const { assert(_length > 0); return _array[0]; }
    BSS_FORCEINLINE bool Back() const { assert(_length > 0); return _array[_length - 1]; }
    BSS_FORCEINLINE BITREF Front() { assert(_length > 0); return GetBit(0); }
    BSS_FORCEINLINE BITREF Back() { assert(_length > 0); return GetBit(_length - 1); }
    BSS_FORCEINLINE bool operator[](CT index) const { assert(index < _length); return (_array[(index / DIV_AMT)] & (((STORE)1) << (index&MOD_AMT))) != 0; }
    BSS_FORCEINLINE BITREF operator[](CT index) { assert(index < _length); return GetBit(index); }
    // This gets the raw storage byte in such a way that unused bits are always set to zero. Useful for debugging.
    inline STORE GetRawByte(CT index) const { assert(index < _capacity); return (index < (_capacity - 1)) ? _array[index] : (_array[index] & ((STORE)(~0) >> ((_capacity*DIV_AMT) - _length))); }

    BSS_FORCEINLINE internal::_BIT_ITER<const BITREF> begin() const { return GetBit(0); }
    BSS_FORCEINLINE internal::_BIT_ITER<const BITREF> end() const { return GetBit(_length); }
    BSS_FORCEINLINE internal::_BIT_ITER<BITREF> begin() { return GetBit(0); }
    BSS_FORCEINLINE internal::_BIT_ITER<BITREF> end() { return GetBit(_length); }

    inline DynArray& operator=(const DynArray& copy)
    {
      if(copy._length > (_capacity*DIV_AMT))
        BASE::_setCapacityDiscard(copy._capacity);
      memcpy(_array, copy._array, _capacity * sizeof(STORE));
      _length = copy._length;
      return *this;
    }
    inline DynArray& operator=(DynArray&& mov) { BASE::operator=(std::move(mov)); _length = mov._length; mov._length = 0; return *this; }
    /*inline DynArray& operator +=(const DynArray& add)
    {
      SetCapacity(*this, _length + add._length);
      memcpybits(_array + (_length/DIV_AMT), _length&MOD_AMT, add._array, 0, add._length);
      _length += add._length;
      return *this;
    }
    inline DynArray operator +(const DynArray& add) const
    {
      DynArray r(_length + add._length);
      memcpy(r._array, _array, (_maxChunks(_length) / DIV_AMT)*sizeof(STORE));
      memcpybits(r._array + (_length / DIV_AMT), _length&MOD_AMT, add._array, 0, add._length);
      r._length = _length + add._length;
      return r;
    }*/

    typedef Ty SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id)
    {
      s.template EvaluateArray<DynArray, bool, &_serializeAdd<Engine>, STORE, nullptr>(*this, _length, id);
    }

  protected:
    template<typename Engine>
    inline static void _serializeAdd(Serializer<Engine>& e, DynArray& obj, int& n)
    {
      bool b;
      Serializer<Engine>::template ActionBind<bool>::Parse(e, b, 0);
      obj.Add(b);
    }
    BSS_FORCEINLINE BITREF GetBit(CT bitindex) const { return BITREF(((STORE)1) << (bitindex&MOD_AMT), *(_array + (bitindex / DIV_AMT))); }
    BSS_FORCEINLINE static CT _maxChunks(CT numbits) { return T_NEXTMULTIPLE(numbits, MOD_AMT); }
    inline void _shiftDelete(CT index)
    {
      CT ind = index / DIV_AMT;
      CT off = index & MOD_AMT;
      CT len = _maxChunks(_length) / DIV_AMT;
      STORE mask = (STORE)(~0) << off;
      _array[ind] = ((_array[ind] >> 1)&mask) | (_array[ind] & (~mask));

      for(CT i = ind + 1; i < len; ++i)
      {
        _array[i - 1] = (_array[i - 1] & ((STORE)(~0) >> 1)) | ((_array[i] & 1) << MOD_AMT);
        _array[i] >>= 1;
      }
    }
    inline void _shiftInsert(CT index)
    {
      _checkSize();
      CT ind = index / DIV_AMT;
      CT off = index & MOD_AMT;
      CT len = _maxChunks(_length + 1) / DIV_AMT;
      STORE mask = (STORE)(~0) << off;
      STORE store = _array[ind];
      _array[ind] = ((_array[ind] & mask) << 1) | (_array[ind] & (~mask));
      STORE test = _array[ind];

      for(CT i = ind + 1; i < len; ++i)
      {
        _array[i] = (store >> MOD_AMT) | ((_array[i] << 1) & (STORE)(~1));
        store = _array[i];
      }
    }

    BSS_FORCEINLINE void _checkSize()
    {
      if(_length >= (_capacity*DIV_AMT))
        BASE::_setCapacity(T_FBNEXT(_capacity));
      assert(_length < (_capacity*DIV_AMT));
    }

    CT _length;
  };

  // A dynamic array that can dynamically adjust the size of each element
  template<typename CType, typename Alloc = StandardAllocator<uint8_t>>
  class BSS_COMPILER_DLLEXPORT ArbitraryArray : protected ArrayBase<uint8_t, CType, ARRAY_SIMPLE, Alloc>
  {
  protected:
    typedef ArrayBase<uint8_t, CType, ARRAY_SIMPLE, Alloc> BASE;
    typedef typename BASE::CT CT;
    typedef typename BASE::Ty Ty;
    using BASE::_array;
    using BASE::_capacity;

  public:
    inline ArbitraryArray(const ArbitraryArray& copy) : BASE(copy._capacity), _length(copy._length), _element(copy._element) { memcpy(_array, copy._array, _length*_element); }
    inline ArbitraryArray(ArbitraryArray&& mov) : BASE(std::move(mov)), _length(mov._length), _element(mov._element) {}
    inline ArbitraryArray(CT size = 0, CT element = 1) : BASE(size*element), _length(0), _element(element) {}
    template<typename T>
    inline CT Add(const T& item)
    {
      if((_length*_element) >= _capacity) BASE::_setCapacity(T_FBNEXT(_length)*_element);
      memcpy(_array + (_length*_element), &item, std::min<CT>(sizeof(T), _element));
      return _length++;
    }
    inline void Remove(CT index)
    {
      index *= _element;
      assert(_capacity > 0 && index < _capacity);
      memmove(_array + index, _array + index + _element, _capacity - index - _element);
      --_length;
    }
    inline void RemoveLast() { --_length; }
    inline void SetElement(const void* newarray, CT element, CT num) // num is a count of how many elements are in the array
    {
      if(((uint8_t*)newarray) == _array)
        return;

      _element = element;
      _length = num;

      if((_length*_element) > _capacity)
        BASE::_setCapacityDiscard(_length*_element);

      if(_array)
        memcpy(_array, newarray, element*num);
    }
    template<typename T> // num is a count of how many elements are in the array
    BSS_FORCEINLINE void SetElement(const T* newarray, CT num) { SetElement(newarray, sizeof(T), num); }
    template<typename T, int NUM>
    BSS_FORCEINLINE void SetElement(const T(&newarray)[NUM]) { SetElement(newarray, sizeof(T), NUM); }
    template<typename T, int NUM>
    BSS_FORCEINLINE void SetElement(const std::array<T, NUM>& newarray) { SetElement(newarray.data(), sizeof(T), NUM); }
    void SetElement(CT element)
    {
      if(element == _element)
        return;

      CT capacity = element*_length;
      uint8_t* narray = !_length ? 0 : (uint8_t*)Alloc::allocate(capacity);
      if(narray)
        memset(narray, 0, capacity);
      CT m = std::min<CT>(element, _element);

      for(CT i = 0; i < _length; ++i)
        memcpy(narray + (i*element), _array + (i*_element), m);

      BASE::_free(_array);
      _array = narray;
      _capacity = capacity;
      _element = element;
      assert(element > 0);
    }
    inline CT Element() const { return _element; }
    inline bool Empty() const { return !_length; }
    inline void Clear() { _length = 0; }
    inline void SetLength(CT length) { _length = length; length *= _element; if(length > _capacity) BASE::_setCapacity(length); }
    inline CT Length() const { return _length; }
    // These are templatized accessors, but we don't do an assertion on if the size of T is the same as _element, because there are many cases where this isn't actually true.
    template<typename T> inline const T& Front() const { assert(_length > 0); return _array[0]; }
    template<typename T> inline const T& Back() const { assert(_length > 0); return _array[(_length - 1)*_element]; }
    template<typename T> inline T& Front() { assert(_length > 0); return _array[0]; }
    template<typename T> inline T& Back() { assert(_length > 0); return _array[(_length - 1)*_element]; }
    template<typename T> inline const T* begin() const { return _array; }
    template<typename T> inline const T* end() const { return _array + (_length*_element); }
    template<typename T> inline T* begin() { return _array; }
    template<typename T> inline T* end() { return _array + (_length*_element); }
    template<typename T> inline const T& Get(CT index) const { return *(T*)Get(index); }
    template<typename T> inline T& Get(CT index) { return *(T*)Get(index); }
    BSS_FORCEINLINE const void* Get(CT index) const { assert(index < _length); return (_array + (index*_element)); }
    BSS_FORCEINLINE void* Get(CT index) { assert(index < _length); return (_array + (index*_element)); }

    inline const void* operator[](CT index) const { return Get(index); }
    inline void* operator[](CT index) { return Get(index); }
    inline ArbitraryArray& operator=(const ArbitraryArray& copy) 
    { 
      BASE::_setCapacityDiscard(copy._capacity);
      memcpy(_array, copy._array, _capacity);
      _length = copy._length; 
      _element = copy._element;
      return *this; 
    }
    inline ArbitraryArray& operator=(ArbitraryArray&& mov) 
    { 
      BASE::operator=(std::move(mov)); 
      _length = mov._length;
      _element = mov._element; 
      return *this; 
    }

  protected:
    CT _length; // Total length of the array in number of elements
    CT _element; // Size of each element
  };
}

#endif
