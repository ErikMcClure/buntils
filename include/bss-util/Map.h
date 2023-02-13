// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __MAP_H__BSS__
#define __MAP_H__BSS__

#include "ArraySort.h"
#include <tuple>

namespace bss {
  // A map class implemented as an associative sorted array
  template<class Key,
    class Data,
    char(*CFunc)(const Key&, const Key&) = CompT<Key>,
    typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE,
    typename Alloc = StandardAllocator<std::tuple<Key, Data>>>
  class BSS_COMPILER_DLLEXPORT Map : protected ArraySort<std::tuple<Key, Data>, &CompTuple<std::tuple<Key, Data>, 0, CFunc>, CType, ArrayType, Alloc>
  {
  protected:
    using pair_t = std::tuple<Key, Data>;
    using BASE = ArraySort<pair_t, &CompTuple<pair_t, 0, CFunc>, CType, ArrayType, Alloc>;
    using constref = const Data&;
    using CKEYREF = const Key&;
    using BASE::_array;
    using CT = typename BASE::CT;
    using Ty = typename BASE::Ty;

  public:
    explicit Map(CT init = 1) : BASE(init) {}
    Map(const Map& copy) = default;
    Map(Map&& mov) = default;
    //~Map() {}
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    BSS_FORCEINLINE void Discard(CT num) { BASE::Discard(num); }
    BSS_FORCEINLINE CT Insert(CKEYREF key, constref data) { return BASE::Insert(pair_t(key, data)); }
    BSS_FORCEINLINE CT Insert(CKEYREF key, Data&& data) { return BASE::Insert(pair_t(key, std::move(data))); }
    inline CT Get(CKEYREF key) const 
    {
      CT retval = GetNear(key, true);
      return (retval != (CT)(-1) && !CFunc(key, std::get<0>(_array[retval]))) ? retval : (CT)(-1);
    }
    inline constref GetData(CKEYREF key) const { return DataIndex(GetNear(key, true)); } //this has no checking
    inline constref DataIndex(CT index) const { return std::get<1>(_array[index]); }
    inline CKEYREF KeyIndex(CT index) const { return std::get<0>(_array[index]); }
    inline CT Remove(CKEYREF key) { CT retval = Get(key); BASE::Remove(retval); return retval; }
    BSS_FORCEINLINE CT RemoveIndex(CT index) { return BASE::Remove(index); }
    BSS_FORCEINLINE CT Replace(CT index, CKEYREF key, constref data) { return BASE::ReplaceData(index, pair_t(key, data)); }
    BSS_FORCEINLINE CT Replace(CT index, CKEYREF key, Data&& data) { return BASE::ReplaceData(index, pair_t(key, std::move(data))); }
    inline CT ReplaceKey(CT index, CKEYREF key) 
    {
      if(index < 0 || index >= Length())
        return (CT)(-1); 
      return BASE::ReplaceData(index, pair_t(key, std::get<1>(_array[index])));
    }
    BSS_FORCEINLINE CT Length() const { return BASE::Length(); }
    BSS_FORCEINLINE void Expand(CT size) { BASE::Expand(size); }
    inline CT Set(CKEYREF key, constref data)
    {
      CT retval = GetNear(key, true);
      if(retval != (CT)(-1))
      {
        auto&[k, v] = _array[retval];
        if(!CFunc(key, k))
          v = data;
      }
      return retval;
    }
    BSS_FORCEINLINE static char mapComp(const Key& a, const pair_t& b) { return CFunc(a, std::get<0>(b)); }
    CT GetNear(CKEYREF key, bool before) const
    {
      CT retval = before ? (BinarySearchNear<pair_t, Key, CT, mapComp, CompT_NEQ<char>, -1>(_array.begin(), key, 0, Length()) - 1) : BinarySearchNear<pair_t, Key, CT, mapComp, CompT_EQ<char>, 1>(_array.begin(), key, 0, Length());
      return (retval < Length()) ? retval : (CT)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }
    inline const pair_t* begin() const { return _array.begin(); }
    inline const pair_t* end() const { return _array.end(); }
    inline pair_t* begin() { return _array.begin(); }
    inline pair_t* end() { return _array.end(); }
    inline const pair_t& Front() const { return _array.Front(); }
    inline pair_t& Front() { return _array.Front(); }
    inline const pair_t& Back() const { return _array.Back(); }
    inline pair_t& Back() { return _array.Back(); }
    BSS_FORCEINLINE Slice<pair_t, CT> GetSlice() const noexcept { return _array.GetSlice(); }

    inline Map& operator =(const Map& right) = default;
    inline Map& operator =(Map&& right) = default;
    inline const pair_t& operator [](CT index) const { return _array[index]; }
    inline pair_t& operator [](CT index) { return _array[index]; }
    inline constref operator ()(CT index) const { return std::get<1>(_array[index]); }
    inline Data& operator ()(CT index) { return std::get<1>(_array[index]); }

    using BASE::SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { BASE::template Serialize<Engine>(s, id); }
  };
}

#endif