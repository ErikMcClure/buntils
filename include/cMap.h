// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_MAP_H__BSS__
#define __C_MAP_H__BSS__

#include "cArraySort.h"
#include <utility>

namespace bss_util {
  /* A map class implemented as an associative sorted array */
  template<class Key, class Data, char (*CFunc)(const Key&,const Key&)=CompT<Key>, typename _SizeType=unsigned int, typename ArrayType=cArraySimple<std::pair<Key,Data>,_SizeType>>
  class BSS_COMPILER_DLLEXPORT cMap : protected cArraySort<std::pair<Key,Data>, CompTFirst<std::pair<Key,Data>, CFunc>, _SizeType, ArrayType>
  {
  protected:
    typedef std::pair<Key,Data> pair_t;
    typedef cArraySort<pair_t, CompTFirst<pair_t, CFunc>, __ST, ArrayType> cArraySort_t;
    typedef const Data& constref;
    typedef Data&& moveref;
    typedef const Key& CKEYREF;

    inline static char mapComp(const Key& a, const pair_t& b) { return CFunc(a,b.first); }

  public:
    explicit cMap(_SizeType init=1) : cArraySort_t(init) {}
    cMap(const cMap& copy) : cArraySort_t(copy) {}
    cMap(cMap&& mov) : cArraySort_t(std::move(mov)) {}
    //~cMap() {}
    inline void BSS_FASTCALL Clear() { cArraySort_t::Clear(); }
    inline void BSS_FASTCALL Discard(unsigned int num) { cArraySort_t::Discard(num); }
    inline _SizeType BSS_FASTCALL Insert(CKEYREF key, constref data) { return cArraySort_t::Insert(pair_t(key,data)); }
    inline _SizeType BSS_FASTCALL Insert(CKEYREF key, moveref data) { return cArraySort_t::Insert(pair_t(key,std::move(data))); }
    inline _SizeType BSS_FASTCALL Get(CKEYREF key) const {  __ST retval=GetNear(key,true); return (retval!=(__ST)(-1)&&!CompT(_array[retval].first,key))?retval:(__ST)(-1); }
    inline constref BSS_FASTCALL GetData(CKEYREF key) const { return cArraySort_t::operator [](GetNear(key,true)).second; } //this has no checking
    inline _SizeType BSS_FASTCALL Remove(CKEYREF key) {  __ST retval=Get(key); cArraySort_t::Remove(retval); return retval; }
    inline _SizeType BSS_FASTCALL RemoveIndex(_SizeType index) { return cArraySort_t::Remove(index); }
    inline _SizeType BSS_FASTCALL Replace(__ST index, CKEYREF key, constref data) { return cArraySort_t::ReplaceData(index, pair_t(key,data)); }
    inline _SizeType BSS_FASTCALL Replace(__ST index, CKEYREF key, moveref data) { return cArraySort_t::ReplaceData(index, pair_t(key,std::move(data))); }
    inline _SizeType BSS_FASTCALL ReplaceKey(__ST index, CKEYREF key) { if(index<0 || index >= _length) return (__ST)(-1); return cArraySort_t::ReplaceData(index, pair_t(key,_array[index].second)); }
    inline CKEYREF BSS_FASTCALL KeyIndex(__ST index) const { return cArraySort_t::operator [](index).first; }
    inline Data& BSS_FASTCALL DataIndex(__ST index) const { return cArraySort_t::operator [](index).second; }
    inline _SizeType BSS_FASTCALL Length() const { return _length; }
    inline void BSS_FASTCALL Expand(__ST size) { cArraySort_t::Expand(size); }
    inline _SizeType BSS_FASTCALL Set(CKEYREF key, constref data)
    {
      __ST retval=GetNear(key,true);
      if(retval==(__ST)(-1) || CompT(_array[retval].first,key)!=0) return (__ST)(-1);

      _array[retval].second=data;
      return retval;
    }
    inline _SizeType BSS_FASTCALL GetNear(CKEYREF key, bool before) const {
      __ST retval=before?(binsearch_near<pair_t,Key,__ST,mapComp,CompT_NEQ<char>,-1>(_array,key,0,_length)-1):binsearch_near<pair_t,Key,__ST,mapComp,CompT_EQ<char>,1>(_array,key,0,_length);
      return (retval<_length)?retval:(__ST)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }

    inline cMap& operator =(const cMap& right) { cArraySort_t::operator =(right); /*_lastindex=right._lastindex;*/ return *this; }
    inline cMap& operator =(cMap&& right) { cArraySort_t::operator =(std::move(right)); /*_lastindex=right._lastindex;*/ return *this; }
    inline constref operator [](__ST index) const { return cArraySort_t::operator [](index).second; }
    inline Data& operator [](__ST index) { return cArraySort_t::operator [](index).second; }
  };
}

#endif

/*    {
      if(!_length) return (__ST)(-1);
      __ST last=_length;
      __ST first=0;
      __ST retval=last>>1;
      char compres;
      while((compres=CompT(key,_array[retval].first))!=0)
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
    }*/