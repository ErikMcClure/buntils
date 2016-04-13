// Copyright �2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ID_HASH_H__BSS__
#define __C_ID_HASH_H__BSS__

#include "cArray.h"
#include "bss_util.h"
#include "cHash.h"
#include "delegate.h"

namespace bss_util {
  // This is a simple ID hash intended for pointers, which can optionally be compressed to eliminate holes.
  template<typename T, typename ST = uint32_t, typename Alloc=StaticAllocPolicy<T>, T INVALID=0>
  class cIDHash : protected cArrayBase<T, ST, Alloc>
  {
    static_assert(sizeof(ST) <= sizeof(T), "T must not be smaller than ST");

  protected:
    using cArrayBase<T, ST, Alloc>::_array;
    using cArrayBase<T, ST, Alloc>::_capacity;

  public:
    cIDHash(const cIDHash& copy) : cArrayBase<T, ST, Alloc>(copy._capacity), _max(copy._max), _length(copy._length), _freelist(copy._freelist) { memcpy(_array, copy._array, _capacity*sizeof(T)); }
    cIDHash(cIDHash&& mov) : cArrayBase<T, ST, Alloc>(std::move(mov)), _max(mov._max), _length(mov._length), _freelist(mov._freelist) { mov._max = 0; mov._length = 0; mov._freelist = (ST)-1; }
    explicit cIDHash(ST reserve = 0) : cArrayBase<T, ST, Alloc>(reserve), _max(0), _length(0), _freelist((ST)-1) { _fixfreelist(0); }
    virtual ~cIDHash() {}
    virtual ST Add(T item)
    {
      if(_freelist==(ST)-1)
        _grow();
      assert(_freelist!=(ST)-1);
      ST r = _freelist;
      _freelist=*((ST*)&_array[_freelist]);
      _array[r]=item;
      if(r>_max) _max=r;
      ++_length;
      return r;
    }
    virtual T Remove(ST id)
    {
      T r=_array[id];
      *((ST*)&_array[id])=_freelist;
      _freelist=id;
      --_length;
      return r;
    }
    inline T Get(ST id) const { return _array[id]; }
    inline ST Length() const { return _length; }
    inline ST MaxID() const { return _max; }
    // Compresses the IDs by eliminating holes. F is called on any ID before it's changed so you can adjust it.
    template<typename F>
    void Compress(F f) {
      ST t;
      while((t=_freelist)!=(ST)-1) // Run through the freelist, setting everything in it to INVALID
      {
        _freelist=*((ST*)&_array[_freelist]);
        _array[t]=INVALID;
      }

      t=_capacity; // Run forwards until we hit a hole, then run backwards until we don't hit a hole, and swap the two
      for(ST i = 0; i < _length; ++i)
      {
        if(_array[i]!=INVALID) continue;
        while(_array[--t]==INVALID);
        f(t, i);
        _array[i]=_array[t];
      }
      _fixfreelist(_length); // Reset the _freelist 
      _max=_length-1; // Reset max value to one less than _length
    }

    inline cIDHash& operator=(const cIDHash& copy) { cArrayBase<T, ST, Alloc>::operator=(copy); _max=copy._max; _length=copy._length; _freelist=copy._freelist; return *this; }
    inline cIDHash& operator=(cIDHash&& mov) { cArrayBase<T, ST, Alloc>::operator=(std::move(mov)); _max=mov._max; _length=mov._length; _freelist=mov._freelist; return *this; }
    inline T& operator[](ST id) { return _array[id]; }
    inline const T& operator[](ST id) const { return _array[id]; }

  protected:
    BSS_FORCEINLINE void _grow()
    {
      ST oldsize=_capacity;
      cArrayBase<T, ST, Alloc>::SetCapacity(fbnext(_capacity));
      _fixfreelist(oldsize);
    }
    inline void _fixfreelist(ST start)
    {
      for(ST i = _capacity; (i--) > start;) // Add all the new items to the freelist
      {
        *((ST*)&_array[i])=_freelist;
        _freelist=i;
      }
    }

    ST _max;
    ST _length;
    ST _freelist;
  };

  // This is a reversible wrapper around cIDhash that allows for two-way lookups.
  template<typename T, typename ST = uint32_t, typename Alloc = StaticAllocPolicy<T>, T INVALID = 0, khint_t(*__hash_func)(T) = &CHASH_HELPER<T, false>::hash, bool(*__hash_equal)(T, T) = &CHASH_HELPER<T, false>::equal>
  class cIDReverse : protected cIDHash<T, ST, Alloc, INVALID>
  {
  protected:
    typedef cIDHash<T, ST, Alloc, INVALID> BASE;
    using BASE::_array;
    using BASE::_capacity;

  public:
    cIDReverse(const cIDReverse& copy) : BASE(copy), _hash(copy._freelist) {}
    cIDReverse(cIDReverse&& mov) : BASE(std::move(mov)), _hash(std::move(mov._hash)) {}
    explicit cIDReverse(ST reserve=0) : BASE(reserve) { }
    ~cIDReverse() {}
    virtual ST Add(T item)
    {
      ST r = BASE::Add(item);
      _hash.Insert(item, r);
      return r;
    }
    virtual T Remove(ST id)
    {
      _hash.Remove(_array[id]);
      return BASE::Remove(id);
    }
    inline T Get(ST id) const { return _array[id]; }
    inline ST Lookup(T item) { return _hash[item]; }
    inline ST Length() const { return BASE::_length; }
    inline ST MaxID() const { return BASE::_max; }

    // Compresses the IDs by eliminating holes. F is called on any ID before its changed so you can adjust it.
    void Compress() { // GCC is broken and won't compile ST at all, so we just replace it with size_t.
      BASE::Compress(delegate<void, size_t, size_t>::From<cIDReverse, &cIDReverse::_flip>(this)); 
    } 

    inline cIDReverse& operator=(const cIDReverse& copy) { BASE::operator=(copy); _hash=copy._hash; return *this; }
    inline cIDReverse& operator=(cIDReverse&& mov) { BASE::operator=(std::move(mov)); _hash=mov._hash; return *this; }
    inline T& operator[](ST id) { return _array[id]; }
    inline const T& operator[](ST id) const { return _array[id]; }

  protected:
    inline void BSS_FASTCALL _flip(size_t id, size_t nid) { _hash.Set(_array[id], nid); }

    cKhash<T, ST, true, __hash_func, __hash_equal, ST, (ST)-1> _hash;
  };
}

#endif
