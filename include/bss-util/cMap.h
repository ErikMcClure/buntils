// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_MAP_H__BSS__
#define __C_MAP_H__BSS__

#include "cArraySort.h"
#include <utility>

namespace bss_util {
  // A map class implemented as an associative sorted array
  template<class Key, class Data, char(*CFunc)(const Key&, const Key&)=CompT<Key>, typename CType = size_t, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<std::pair<Key, Data>>>
  class BSS_COMPILER_DLLEXPORT cMap : protected cArraySort<std::pair<Key, Data>, CompTFirst<std::pair<Key, Data>, CFunc>, CType, ArrayType, Alloc>
  {
  protected:
    typedef CType CT_;
    typedef std::pair<Key, Data> pair_t;
    typedef cArraySort<pair_t, CompTFirst<pair_t, CFunc>, CT_, ArrayType, Alloc> cArraySort_t;
    typedef const Data& constref;
    typedef const Key& CKEYREF;
    using cArraySort_t::_array;

  public:
    explicit cMap(CT_ init=1) : cArraySort_t(init) {}
    cMap(const cMap& copy) : cArraySort_t(copy) {}
    cMap(cMap&& mov) : cArraySort_t(std::move(mov)) {}
    //~cMap() {}
    BSS_FORCEINLINE void Clear() { cArraySort_t::Clear(); }
    BSS_FORCEINLINE void Discard(uint32_t num) { cArraySort_t::Discard(num); }
    BSS_FORCEINLINE CT_ Insert(CKEYREF key, constref data) { return cArraySort_t::Insert(pair_t(key, data)); }
    BSS_FORCEINLINE CT_ Insert(CKEYREF key, Data&& data) { return cArraySort_t::Insert(pair_t(key, std::move(data))); }
    inline CT_ Get(CKEYREF key) const { CT_ retval=GetNear(key, true); return (retval!=(CT_)(-1)&&!CompT(_array[retval].first, key))?retval:(CT_)(-1); }
    inline constref GetData(CKEYREF key) const { return cArraySort_t::operator [](GetNear(key, true)).second; } //this has no checking
    inline CT_ Remove(CKEYREF key) { CT_ retval=Get(key); cArraySort_t::Remove(retval); return retval; }
    BSS_FORCEINLINE CT_ RemoveIndex(CT_ index) { return cArraySort_t::Remove(index); }
    BSS_FORCEINLINE CT_ Replace(CT_ index, CKEYREF key, constref data) { return cArraySort_t::ReplaceData(index, pair_t(key, data)); }
    BSS_FORCEINLINE CT_ Replace(CT_ index, CKEYREF key, Data&& data) { return cArraySort_t::ReplaceData(index, pair_t(key, std::move(data))); }
    inline CT_ ReplaceKey(CT_ index, CKEYREF key) { if(index<0 || index >= Length()) return (CT_)(-1); return cArraySort_t::ReplaceData(index, pair_t(key, _array[index].second)); }
    BSS_FORCEINLINE CKEYREF KeyIndex(CT_ index) const { return cArraySort_t::operator [](index).first; }
    BSS_FORCEINLINE const Data& DataIndex(CT_ index) const { return cArraySort_t::operator [](index).second; }
    BSS_FORCEINLINE CT_ Length() const { return cArraySort_t::Length(); }
    BSS_FORCEINLINE void Expand(CT_ size) { cArraySort_t::Expand(size); }
    inline CT_ Set(CKEYREF key, constref data)
    {
      CT_ retval=GetNear(key, true);
      if(retval==(CT_)(-1) || CompT(_array[retval].first, key)!=0) return (CT_)(-1);

      _array[retval].second=data;
      return retval;
    }
    BSS_FORCEINLINE static char mapComp(const Key& a, const pair_t& b) { return CFunc(a, b.first); }
    CT_ GetNear(CKEYREF key, bool before) const {
      CT_ retval=before?(binsearch_near<pair_t, Key, CT_, mapComp, CompT_NEQ<char>, -1>(_array.begin(), key, 0, Length())-1):binsearch_near<pair_t, Key, CT_, mapComp, CompT_EQ<char>, 1>(_array.begin(), key, 0, Length());
      return (retval<Length())?retval:(CT_)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }
    inline const std::pair<Key, Data>* begin() const { return _array.begin(); }
    inline const std::pair<Key, Data>* end() const { return _array.end(); }
    inline std::pair<Key, Data>* begin() { return _array.begin(); }
    inline std::pair<Key, Data>* end() { return _array.end(); }
    inline const std::pair<Key, Data>& Front() const { return _array.Front(); }
    inline std::pair<Key, Data>& Front() { return _array.Front(); }
    inline const std::pair<Key, Data>& Back() const { return _array.Back(); }
    inline std::pair<Key, Data>& Back() { return _array.Back(); }

    inline cMap& operator =(const cMap& right) { cArraySort_t::operator =(right); return *this; }
    inline cMap& operator =(cMap&& right) { cArraySort_t::operator =(std::move(right)); return *this; }
    inline constref operator [](CT_ index) const { return cArraySort_t::operator [](index).second; }
    inline Data& operator [](CT_ index) { return cArraySort_t::operator [](index).second; }
  };
}

#endif