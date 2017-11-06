// Copyright ©2017 Black Sphere Studios
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
    typename Alloc = StaticAllocPolicy<std::tuple<Key, Data>>>
  class BSS_COMPILER_DLLEXPORT Map : protected ArraySort<std::tuple<Key, Data>, &CompTuple<std::tuple<Key, Data>, 0, CFunc>, CType, ArrayType, Alloc>
  {
  protected:
    typedef CType CT_;
    typedef std::tuple<Key, Data> pair_t;
    typedef ArraySort<pair_t, &CompTuple<pair_t, 0, CFunc>, CT_, ArrayType, Alloc> BASE;
    typedef const Data& constref;
    typedef const Key& CKEYREF;
    using BASE::_array;

  public:
    explicit Map(CT_ init = 1) : BASE(init) {}
    Map(const Map& copy) = default;
    Map(Map&& mov) = default;
    //~Map() {}
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    BSS_FORCEINLINE void Discard(CType num) { BASE::Discard(num); }
    BSS_FORCEINLINE CT_ Insert(CKEYREF key, constref data) { return BASE::Insert(pair_t(key, data)); }
    BSS_FORCEINLINE CT_ Insert(CKEYREF key, Data&& data) { return BASE::Insert(pair_t(key, std::move(data))); }
    inline CT_ Get(CKEYREF key) const 
    {
      CT_ retval = GetNear(key, true);
      return (retval != (CT_)(-1) && !CFunc(std::get<0>(_array[retval]), key)) ? retval : (CT_)(-1);
    }
    inline constref GetData(CKEYREF key) const { return DataIndex(GetNear(key, true)); } //this has no checking
    inline constref DataIndex(CT_ index) const { return std::get<1>(_array[index]); }
    inline CKEYREF KeyIndex(CT_ index) const { return std::get<0>(_array[index]); }
    inline CT_ Remove(CKEYREF key) { CT_ retval = Get(key); BASE::Remove(retval); return retval; }
    BSS_FORCEINLINE CT_ RemoveIndex(CT_ index) { return BASE::Remove(index); }
    BSS_FORCEINLINE CT_ Replace(CT_ index, CKEYREF key, constref data) { return BASE::ReplaceData(index, pair_t(key, data)); }
    BSS_FORCEINLINE CT_ Replace(CT_ index, CKEYREF key, Data&& data) { return BASE::ReplaceData(index, pair_t(key, std::move(data))); }
    inline CT_ ReplaceKey(CT_ index, CKEYREF key) 
    {
      if(index < 0 || index >= Length())
        return (CT_)(-1); 
      return BASE::ReplaceData(index, pair_t(key, std::get<1>(_array[index])));
    }
    BSS_FORCEINLINE CT_ Length() const { return BASE::Length(); }
    BSS_FORCEINLINE void Expand(CT_ size) { BASE::Expand(size); }
    inline CT_ Set(CKEYREF key, constref data)
    {
      CT_ retval = GetNear(key, true);
      if(retval != (CT_)(-1))
      {
        auto&[k, v] = _array[retval];
        if(!CFunc(k, key))
          v = data;
      }
      return retval;
    }
    BSS_FORCEINLINE static char mapComp(const Key& a, const pair_t& b) { return CFunc(a, std::get<0>(b)); }
    CT_ GetNear(CKEYREF key, bool before) const
    {
      CT_ retval = before ? (BinarySearchNear<pair_t, Key, CT_, mapComp, CompT_NEQ<char>, -1>(_array.begin(), key, 0, Length()) - 1) : BinarySearchNear<pair_t, Key, CT_, mapComp, CompT_EQ<char>, 1>(_array.begin(), key, 0, Length());
      return (retval < Length()) ? retval : (CT_)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }
    inline const pair_t* begin() const { return _array.begin(); }
    inline const pair_t* end() const { return _array.end(); }
    inline pair_t* begin() { return _array.begin(); }
    inline pair_t* end() { return _array.end(); }
    inline const pair_t& Front() const { return _array.Front(); }
    inline pair_t& Front() { return _array.Front(); }
    inline const pair_t& Back() const { return _array.Back(); }
    inline pair_t& Back() { return _array.Back(); }
    BSS_FORCEINLINE Slice<pair_t, CT_> GetSlice() const noexcept { return _array.GetSlice(); }

    inline Map& operator =(const Map& right) = default;
    inline Map& operator =(Map&& right) = default;
    inline const pair_t& operator [](CT_ index) const { return _array[index]; }
    inline pair_t& operator [](CT_ index) { return _array[index]; }
    inline constref operator ()(CT_ index) const { return std::get<1>(_array[index]); }
    inline Data& operator ()(CT_ index) { return std::get<1>(_array[index]); }
  };
}

#endif