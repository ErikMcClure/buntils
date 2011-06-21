// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_MAP_H__BSS__
#define __C_MAP_H__BSS__

#include "cArraySort.h"
#include <utility>

namespace bss_util {
  /* A map class implemented as an associative sorted array */
  template<class Key, class Data, typename CompareTraits=CompareKeysTraits<Key>, typename DataTraits=ValueTraits<Data>, typename _SizeType=unsigned int>
  class __declspec(dllexport) cMap : protected cArraySort<std::pair<Key,Data>, ComparePairTraits_first<std::pair<Key,Data>, RefTraits<std::pair<Key,Data>>, CompareTraits>, _SizeType>, DataTraits
  {
  protected:
    typedef std::pair<Key,Data> pair_t;
    typedef cMap<Key,Data,CompareTraits,_SizeType> cMap_t;
    typedef cArraySort<pair_t, ComparePairTraits_first<pair_t, RefTraits<pair_t>, CompareTraits>, __ST> cArraySort_t;
    typedef typename DataTraits::const_reference constref;
    typedef typename DataTraits::reference reference;

  public:
    cMap(_SizeType init=1) : cArraySort_t(init) {}
    cMap(const cMap& copy) : cArraySort_t(copy) {}
    //~cMap() {}
    inline void BSS_FASTCALL Clear() { cArraySort_t::Clear(); }
    inline void BSS_FASTCALL Discard(unsigned int num) { cArraySort_t::Discard(num); }
    inline _SizeType BSS_FASTCALL Insert(Key key, constref data) { return cArraySort_t::Insert(pair_t(key,data)); }
    inline _SizeType BSS_FASTCALL Get(Key key) const {  __ST retval=GetNear(key,true); return (retval!=(__ST)(-1)&&!CompareTraits::Compare(_array[retval].first,key))?retval:(__ST)(-1); }
    inline constref BSS_FASTCALL GetData(Key key) const { return cArraySort_t::operator [](GetNear(key,true)).second; } //this has no checking
    inline _SizeType BSS_FASTCALL Remove(Key key) {  __ST retval=Get(key); cArraySort_t::Remove(retval); return retval; }
    inline _SizeType BSS_FASTCALL RemoveIndex(_SizeType index) { return cArraySort_t::Remove(index); }
    inline _SizeType BSS_FASTCALL Replace(__ST index, Key key, constref data) { return cArraySort_t::ReplaceData(index, pair_t(key,data)); }
    inline _SizeType BSS_FASTCALL ReplaceKey(__ST index, Key key) { if(index<0 || index >= _length) return (__ST)(-1); return cArraySort_t::ReplaceData(index, pair_t(key,_array[index].second)); }
    inline Key BSS_FASTCALL KeyIndex(__ST index) const { return cArraySort_t::operator [](index).first; }
    inline Data BSS_FASTCALL DataIndex(__ST index) const { return cArraySort_t::operator [](index).second; }
    inline _SizeType BSS_FASTCALL GetLength() const { return _length; }
    inline void BSS_FASTCALL Expand(__ST size) { cArraySort_t::Expand(size); }
    inline _SizeType BSS_FASTCALL Set(Key key, constref data)
    {
      __ST retval=GetNear(key,true);
      if(retval==(__ST)(-1) || CompareTraits::Compare(_array[retval].first,key)!=0) return (__ST)(-1);

      _array[retval].second=data;
      return retval;
    }
    inline _SizeType BSS_FASTCALL GetNear(Key key, bool before) const
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
            return before?retval:((++retval<_length)?retval:(__ST)(-1));
        }
      }
      return retval;
    }

    inline cMap& operator =(const cMap& right) { cArraySort_t::operator =(right); /*_lastindex=right._lastindex;*/ return *this; }
    inline constref operator [](__ST index) const { return cArraySort_t::operator [](index).second; }
    inline Data& operator [](__ST index) { return cArraySort_t::operator [](index).second; }
  };
}

#endif











  ///* A map class designed to be really really really fast, and is pointer-based. It uses a recursive search algorythm*/
  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)=CompareKeys<Key>>
  //class cMap //: cAVLtree<Key,Data,Compare>
  //{
  //public:
  //  cMap();
  //  cMap(const cMap& copy);
  //  ~cMap();
  //  bool Insert(Key* key, Data* data); //this function returns false if key is not unique
  //  Data* Get(Key* key);
  //  bool Set(Key* key, Data* data);
  //  bool Remove(Key* key);
  //  void Clear();
  //  size_t CopyTo(Key*** keys, Data*** datas) const; //Triple pointers. Yes, I'm dead serious. Because we have to store arrays of pointers to the class object, we have to send a pointer to our array of pointers. o.O;
  //  Key* GetKeyIndex(size_t index) const; //gets a key by index
  //  Data* GetDataIndex(size_t index) const; //gets a data node by index
  //  size_t GetTotal() const; //gets number of keys
  //  bool Resize(size_t size);

  //  cMap<Key,Data,Compare>& operator =(const cMap<Key,Data,Compare>& right) { right.CopyTo(&_keys,&_datas); return (*this); }

  //private:
  //  size_t _findkey(Key* key); //Finds the location within the internal array where a given key is
  //  size_t _findindex(size_t min, size_t max, Key* key); //Recursive algorythm

  //  Key** _keys;
  //  Data** _datas; //Yes this is bad grammer. Shut up.
  //  size_t _total; // How many elements are in there
  //  size_t _size; //Actual size of the arrays for optimization (Avoidance of unnecessary memory reallocation)
  //  Key* _lastsearch;
  //  size_t _lastindex;
  //};

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //cMap<Key,Data,Compare>::cMap()
  //{
  //  _keys = 0;
  //  _datas = 0;
  //  _size = _total = 0;
  //  _lastsearch=0;
  //  _lastindex = 0;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //cMap<Key,Data,Compare>::cMap(const cMap<Key,Data,Compare> &copy)
  //{
  //  _size = _total = copy.CopyTo(&_keys,&_datas);
  //  _lastindex = 0;
  //  _lastsearch=0;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //cMap<Key,Data,Compare>::~cMap()
  //{
  //  if(_keys) delete [] _keys;
  //  if(_datas) delete [] _datas;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //bool cMap<Key,Data,Compare>::Insert(Key *key, Data *data)
  //{
  //  if(!key) return false;

  //  size_t oldtotal = _total++;
  //  if(_size < _total) //If this is true we need to allocate more memory
  //  {
  //    Key** nkeys = new Key*[++_size];
  //    if(_keys)
  //    {
  //      memcpy(nkeys,_keys,sizeof(Key*)*oldtotal);
  //      delete [] _keys;
  //    }
  //    nkeys[oldtotal] = 0; //the last entry should be zero'd or it'll be nonsense
  //    _keys = nkeys;

  //    //do exact same thing for data pointers
  //    Data** ndatas = new Data*[_size];
  //    if(_datas)
  //    {
  //      memcpy(ndatas,_datas,sizeof(Data*)*oldtotal);
  //      delete [] _datas;
  //    }
  //    ndatas[oldtotal] = 0; //the last entry should be zero'd or it'll be nonsense
  //    _datas = ndatas;
  //  }

  //  _lastsearch = 0; //this is now invalid

  //  if(oldtotal == 0 || Compare(*key, *_keys[oldtotal-1]) > 0) //check to see if our item is bigger then all the rest, which is always true but not testable when oldtotal = 0
  //  {
  //    _keys[oldtotal] = key;
  //    _datas[oldtotal] = data;
  //  }
  //  else
  //    for(unsigned i = 0; i < oldtotal; ++i)
  //    {
  //      char result = Compare(*key, *_keys[i]);
  //      if(result == 0) { _total = oldtotal; return false; } //This key isn't unique! return to our old total and return false

  //      if(result < 0) //if this is true, key is "less than" the key at index i, so we insert it here
  //      {
  //        Key* prevk = key;
  //        Data* prevd = data;
  //        for(unsigned k = i; k < _total; ++k) //pushes the rest of the array up
  //        {
  //          //Set key
  //          Key* hold = _keys[k];
  //          _keys[k] = prevk;
  //          prevk = hold;
  //          //Set data
  //          Data* hold2 = _datas[k];
  //          _datas[k] = prevd;
  //          prevd = hold2;
  //        }
  //        break;
  //      }
  //    }

  //  return true;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //bool cMap<Key,Data,Compare>::Resize(size_t size)
  //{
  //  if(size < _total) return false; //We can't resize to something smaller then required
  //  _size = size;
  //  
  //  Key** nkeys = new Key*[_size];
  //  if(_keys)
  //  {
  //    memcpy(nkeys,_keys,sizeof(Key*)*_total);
  //    delete [] _keys;
  //  }
  //  if(_total != _size) memset(&nkeys[_total],0,sizeof(Key*)*(_size-_total));
  //  //nkeys[oldtotal] = 0; //the last entry should be zero'd or it'll be nonsense
  //  _keys = nkeys;

  //  //do exact same thing for data pointers
  //  Data** ndatas = new Data*[_size];
  //  if(_datas)
  //  {
  //    memcpy(ndatas,_datas,sizeof(Data*)*_total);
  //    delete [] _datas;
  //  }
  //  if(_total != _size) memset(&ndatas[_total],0,sizeof(Data*)*(_size-_total));
  //  _datas = ndatas;
  //  return true;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //Data* cMap<Key,Data,Compare>::Get(Key *key)
  //{
  //  size_t loc = _findkey(key);
  //  if(loc < _total) return _datas[loc];
  //  return 0;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //bool cMap<Key,Data,Compare>::Set(Key* key, Data* data)
  //{
  //  size_t loc = _findkey(key);
  //  if(loc < _total)
  //  {
  //    _datas[loc] = data;
  //    return true;
  //  }
  //  return false;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //bool cMap<Key,Data,Compare>::Remove(Key* key)
  //{
  //  size_t loc = _findkey(key);
  //  if(loc < _total)
  //  {
  //    --_total;
  //    for(unsigned i = loc; i < _total; ++i)
  //    {
  //      _keys[i] = _keys[i+1];
  //      _datas[i] = _datas[i+1];
  //    }
  //    _datas[_total] = 0;
  //    _keys[_total] = 0;
  //    _lastsearch = 0; //this is now invalid
  //    return true;
  //  }
  //  return false;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //void cMap<Key,Data,Compare>::Clear()
  //{
  //  if(_keys) delete [] _keys;
  //  if(_datas) delete [] _datas;
  //  _keys = 0;
  //  _datas = 0;
  //  _size = _total = 0;
  //  _lastsearch=0;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //size_t cMap<Key,Data,Compare>::CopyTo(Key*** keys, Data*** datas) const
  //{
  //  *keys = new Key*[_total];
  //  memcpy(*keys, _keys, sizeof(Key*)*_total);
  //  *datas = new Data*[_total];
  //  memcpy(*datas, _datas, sizeof(Data*)*_total);
  //  return _total;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //size_t cMap<Key,Data,Compare>::_findkey(Key *key)
  //{
  //  if(!key) return -1;
  //  if(_lastsearch != 0 && _lastindex != -1 && Compare(*_lastsearch, *key) == 0) return _lastindex;
  //  //_lastsearch = key; //Well if it wasn't the last search before, it is now
  //  size_t retval = !_total?-1:_lastindex = _findindex(0, _total-1, key); //total must be greater then 0
  //  if(retval < _total) _lastsearch = _keys[retval]; //we have to do this because if we assign lastsearch to key, key could get deleted and reassigned to something else
  //  return retval;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //size_t cMap<Key,Data,Compare>::_findindex(size_t min, size_t max, Key *key)
  //{
  //  if(min == max) //If this is true we've landed on the only possible match
  //    return !Compare(*key, *_keys[min])?min:-1; //If this isn't a match, the key doesn't exist in the map!
  //  size_t mid = (min+max)/(size_t)2;
  //  char result = Compare(*key, *_keys[mid]);
  //  if(result == 0) return mid; //By chance we hit the right one. Return!
  //  if(result < 0) return _findindex(min, !mid?0:mid-1,key);
  //  return _findindex(mid+1,max,key);
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //Key* cMap<Key,Data,Compare>::GetKeyIndex(size_t index) const
  //{
  //  if(index < _total)
  //    return _keys[index];
  //  return 0;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //Data* cMap<Key,Data,Compare>::GetDataIndex(size_t index) const
  //{
  //  if(index < _total)
  //    return _datas[index];
  //  return 0;
  //}

  //template<class Key, class Data, char (*Compare)(const Key& keyleft, const Key& keyright)>
  //size_t cMap<Key,Data,Compare>::GetTotal() const
  //{
  //  return _total;
  //}