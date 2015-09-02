// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_PRIORITY_QUEUE_H__BSS__
#define __C_PRIORITY_QUEUE_H__BSS__

#include "cBinaryHeap.h"

namespace bss_util {
  // PriorityQueue that can be implemented as either a maxheap or a minheap
  template<typename K, typename D, char(*CFunc)(const K&, const K&)=CompT<K>, typename ST_=unsigned int, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<std::pair<K, D>>>
  class BSS_COMPILER_DLLEXPORT cPriorityQueue : protected cBinaryHeap<std::pair<K, D>, ST_, CompTFirst<std::pair<K, D>, CFunc>, ArrayType, Alloc>
  {
    typedef std::pair<K, D> PAIR;
    typedef cBinaryHeap<std::pair<K, D>, ST_, CompTFirst<std::pair<K, D>, CFunc>, ArrayType, Alloc> BASE;

  public:
    cPriorityQueue(const cPriorityQueue& copy) : BASE(copy) {}
    cPriorityQueue(cPriorityQueue&& mov) : BASE(std::move(mov)) {}
    cPriorityQueue() {}
    ~cPriorityQueue() {}
    BSS_FORCEINLINE void Push(const K& key, D value) { BASE::Insert(PAIR(key, value)); }
    BSS_FORCEINLINE void Push(K&& key, D value) { BASE::Insert(PAIR(std::move(key), value)); }
    BSS_FORCEINLINE const PAIR& Peek() { return BASE::Peek(); }
    BSS_FORCEINLINE void Discard() { BASE::Remove(0); }
    BSS_FORCEINLINE PAIR Pop() { return std::move(BASE::Pop()); }
    BSS_FORCEINLINE bool Empty() { return BASE::Empty(); }
    BSS_FORCEINLINE const PAIR& Get(ST_ index) { return BASE::Get(index); }
    BSS_FORCEINLINE bool Remove(ST_ index) { return BASE::Remove(index); }
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    inline ST_ Length() { return BASE::_length; }

    inline cPriorityQueue& operator=(const cPriorityQueue& copy) { BASE::operator=(copy); return *this; }
    inline cPriorityQueue& operator=(cPriorityQueue&& mov) { BASE::operator=(std::move(mov)); return *this; }
  };

  template<class T, typename ST_>
  struct MFUNC_PRIORITY {
    BSS_FORCEINLINE static void BSS_FASTCALL MFunc(const T& item, ST_ i, MFUNC_PRIORITY* p) { p->_subarray[item.first] = i; }
    cArray<ST_, ST_> _subarray;
  };

  template<typename D, typename ST_ = unsigned int, char(*CFunc)(const D&, const D&) = CompT<D>,  ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<std::pair<ST_, D>>>
  class BSS_COMPILER_DLLEXPORT cPriorityHeap : protected cBinaryHeap<std::pair<ST_, D>, ST_, CompTSecond<std::pair<ST_, D>, CFunc>, ArrayType, Alloc, MFUNC_PRIORITY<std::pair<ST_, D>, ST_>>
  {
    typedef std::pair<ST_, D> PAIR;
    typedef cBinaryHeap<std::pair<ST_, D>, ST_, CompTSecond<std::pair<ST_, D>, CFunc>, ArrayType, Alloc, MFUNC_PRIORITY<std::pair<ST_, D>, ST_>> BASE;
    using BASE::_subarray;
  public:
    cPriorityHeap(const cPriorityHeap& copy) : BASE(copy), _freelist(copy._freelist) { _subarray = copy._subarray; }
    cPriorityHeap(cPriorityHeap&& mov) : BASE(std::move(mov)), _freelist(mov._freelist) { _subarray = std::move(mov._subarray); }
    cPriorityHeap() : _freelist((ST_)-1) {}
    ~cPriorityHeap() {}
    BSS_FORCEINLINE ST_ Push(const D& value) { ST_ k = _getnext(); BASE::Insert(PAIR(k, value)); return k; }
    BSS_FORCEINLINE ST_ Push(D&& value) { ST_ k = _getnext(); BASE::Insert(PAIR(k, std::move(value))); return k; }
    BSS_FORCEINLINE const D& Peek() { return BASE::Peek().second; }
    BSS_FORCEINLINE void Discard() { BASE::Remove(0); }
    BSS_FORCEINLINE D Pop() { return BASE::Pop().second; }
    BSS_FORCEINLINE bool Empty() { return BASE::Empty(); }
    BSS_FORCEINLINE const D& Get(ST_ key) { return BASE::Get(_subarray[key]).second; }
    BSS_FORCEINLINE bool Remove(ST_ key) { return BASE::Remove(_subarray[key]); }
    BSS_FORCEINLINE bool Set(ST_ key, const D& value) { return BASE::Set(_subarray[key], PAIR(key,value)); }
    BSS_FORCEINLINE bool Set(ST_ key, D&& value) { return BASE::Set(_subarray[key], PAIR(key, std::move(value))); }
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    inline ST_ Length() { return BASE::_length; }

    inline cPriorityHeap& operator=(const cPriorityHeap& copy) { BASE::operator=(copy); _freelist = copy._freelist; return *this; }
    inline cPriorityHeap& operator=(cPriorityHeap&& mov) { BASE::operator=(std::move(mov)); _freelist = mov._freelist; return *this; }

  protected:
    ST_ _getnext()
    {
      if(_freelist == (ST_)-1)
      {
        ST_ size = _subarray.Size();
        _subarray.SetSize(fbnext(size));
        _initfreelist(size);
      }
      ST_ r = _freelist;
      _freelist = _subarray[r];
      return r;
    }
    void _initfreelist(ST_ start)
    {
      for(ST_ i = start; i < _subarray.Size(); ++i)
      {
        _subarray[i] = _freelist;
        _freelist = i;
      }
    }

    ST_ _freelist;
  };
}

#endif
