// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_MAP_H__BSS__
#define __C_MAP_H__BSS__

#include "cArraySort.h"
#include <utility>

namespace bss_util {
  /* A map class implemented as an associative sorted array */
  template<class Key, class Data, typename CompareTraits=CompareKeysTraits<Key>, typename DataTraits=ValueTraits<Data>, typename _SizeType=unsigned int, typename ArrayType=cArraySimple<std::pair<Key,Data>,_SizeType>>
  class BSS_COMPILER_DLLEXPORT cMap : protected cArraySort<std::pair<Key,Data>, ComparePairTraits_first<std::pair<Key,Data>, RefTraits<std::pair<Key,Data>>, CompareTraits>, _SizeType, ArrayType>, DataTraits
  {
  protected:
    typedef std::pair<Key,Data> pair_t;
    typedef cMap<Key,Data,CompareTraits,_SizeType> cMap_t;
    typedef cArraySort<pair_t, ComparePairTraits_first<pair_t, RefTraits<pair_t>, CompareTraits>, __ST, ArrayType> cArraySort_t;
    typedef typename DataTraits::const_reference constref;
    typedef typename DataTraits::reference reference;
    typedef typename CompareTraits::const_reference CKEYREF;
    typedef typename CompareTraits::reference KEYREF;

  public:
    explicit cMap(_SizeType init=1) : cArraySort_t(init) {}
    cMap(const cMap& copy) : cArraySort_t(copy) {}
    cMap(cMap&& mov) : cArraySort_t(std::move(mov)) {}
    //~cMap() {}
    inline void BSS_FASTCALL Clear() { cArraySort_t::Clear(); }
    inline void BSS_FASTCALL Discard(unsigned int num) { cArraySort_t::Discard(num); }
    inline _SizeType BSS_FASTCALL Insert(CKEYREF key, constref data) { return cArraySort_t::Insert(pair_t(key,data)); }
    inline _SizeType BSS_FASTCALL Get(CKEYREF key) const {  __ST retval=GetNear(key,true); return (retval!=(__ST)(-1)&&!CompareTraits::Compare(_array[retval].first,key))?retval:(__ST)(-1); }
    inline constref BSS_FASTCALL GetData(CKEYREF key) const { return cArraySort_t::operator [](GetNear(key,true)).second; } //this has no checking
    inline _SizeType BSS_FASTCALL Remove(CKEYREF key) {  __ST retval=Get(key); cArraySort_t::Remove(retval); return retval; }
    inline _SizeType BSS_FASTCALL RemoveIndex(_SizeType index) { return cArraySort_t::Remove(index); }
    inline _SizeType BSS_FASTCALL Replace(__ST index, CKEYREF key, constref data) { return cArraySort_t::ReplaceData(index, pair_t(key,data)); }
    inline _SizeType BSS_FASTCALL ReplaceKey(__ST index, CKEYREF key) { if(index<0 || index >= _length) return (__ST)(-1); return cArraySort_t::ReplaceData(index, pair_t(key,_array[index].second)); }
    inline KEYREF BSS_FASTCALL KeyIndex(__ST index) const { return cArraySort_t::operator [](index).first; }
    inline reference BSS_FASTCALL DataIndex(__ST index) const { return cArraySort_t::operator [](index).second; }
    inline _SizeType BSS_FASTCALL Length() const { return _length; }
    inline void BSS_FASTCALL Expand(__ST size) { cArraySort_t::Expand(size); }
    inline _SizeType BSS_FASTCALL Set(CKEYREF key, constref data)
    {
      __ST retval=GetNear(key,true);
      if(retval==(__ST)(-1) || CompareTraits::Compare(_array[retval].first,key)!=0) return (__ST)(-1);

      _array[retval].second=data;
      return retval;
    }
    inline _SizeType BSS_FASTCALL GetNear(CKEYREF key, bool before) const
    {
      if(!_length) return (__ST)(-1);
      __ST last=_length;
      __ST first=0;
      __ST retval=last>>1;
      char compres;
      while((compres=CompareTraits::Compare(key,_array[retval].first))!=0)
      {
        if(compres<0)
        {
          last=retval;
          retval=first+((last-first)>>1); //R = F+((L-F)/2)
          if(last==retval)
            return before?--retval:retval; //interestingly, if retval is 0, we end up with -1 which is exactly what we'd want anyway
        }
        else
        {
          first=retval;
          retval+=(last-first)>>1;
          if(first==retval)
            return before?retval:(((++retval)<_length)?retval:(__ST)(-1));
        }
      }
      return retval;
    }

    inline cMap& operator =(const cMap& right) { cArraySort_t::operator =(right); /*_lastindex=right._lastindex;*/ return *this; }
    inline cMap& operator =(cMap&& right) { cArraySort_t::operator =(std::move(right)); /*_lastindex=right._lastindex;*/ return *this; }
    inline constref operator [](__ST index) const { return cArraySort_t::operator [](index).second; }
    inline Data& operator [](__ST index) { return cArraySort_t::operator [](index).second; }
  };
}

#endif