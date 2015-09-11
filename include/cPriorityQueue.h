// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_PRIORITY_QUEUE_H__BSS__
#define __C_PRIORITY_QUEUE_H__BSS__

#include "cBinaryHeap.h"

namespace bss_util {
  // PriorityQueue that can be implemented as either a maxheap or a minheap
  template<typename K, typename D, char(*CFunc)(const K&, const K&)=CompT<K>, typename CT_=unsigned int, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<std::pair<K, D>>>
  class BSS_COMPILER_DLLEXPORT cPriorityQueue : protected cBinaryHeap<std::pair<K, D>, CT_, CompTFirst<std::pair<K, D>, CFunc>, ArrayType, Alloc>
  {
    typedef std::pair<K, D> PAIR;
    typedef cBinaryHeap<std::pair<K, D>, CT_, CompTFirst<std::pair<K, D>, CFunc>, ArrayType, Alloc> BASE;

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
    BSS_FORCEINLINE const PAIR& Get(CT_ index) { return BASE::Get(index); }
    BSS_FORCEINLINE bool Remove(CT_ index) { return BASE::Remove(index); }
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    inline CT_ Length() { return BASE::_length; }

    inline cPriorityQueue& operator=(const cPriorityQueue& copy) { BASE::operator=(copy); return *this; }
    inline cPriorityQueue& operator=(cPriorityQueue&& mov) { BASE::operator=(std::move(mov)); return *this; }
  };

  template<class T, typename CT_>
  struct MFUNC_PRIORITY {
    BSS_FORCEINLINE static void BSS_FASTCALL MFunc(const T& item, CT_ i, MFUNC_PRIORITY* p) { p->_subarray[item.first] = i; }
    cArray<CT_, CT_> _subarray;
  };

  template<typename D, typename CT_ = unsigned int, char(*CFunc)(const D&, const D&) = CompT<D>,  ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<std::pair<CT_, D>>>
  class BSS_COMPILER_DLLEXPORT cPriorityHeap : protected cBinaryHeap<std::pair<CT_, D>, CT_, CompTSecond<std::pair<CT_, D>, CFunc>, ArrayType, Alloc, MFUNC_PRIORITY<std::pair<CT_, D>, CT_>>
  {
    typedef std::pair<CT_, D> PAIR;
    typedef cBinaryHeap<std::pair<CT_, D>, CT_, CompTSecond<std::pair<CT_, D>, CFunc>, ArrayType, Alloc, MFUNC_PRIORITY<std::pair<CT_, D>, CT_>> BASE;
    using BASE::_subarray;
  public:
    cPriorityHeap(const cPriorityHeap& copy) : BASE(copy), _freelist(copy._freelist) { _subarray = copy._subarray; }
    cPriorityHeap(cPriorityHeap&& mov) : BASE(std::move(mov)), _freelist(mov._freelist) { _subarray = std::move(mov._subarray); }
    cPriorityHeap() : _freelist((CT_)-1) {}
    ~cPriorityHeap() {}
    BSS_FORCEINLINE CT_ Push(const D& value) { CT_ k = _getnext(); BASE::Insert(PAIR(k, value)); return k; }
    BSS_FORCEINLINE CT_ Push(D&& value) { CT_ k = _getnext(); BASE::Insert(PAIR(k, std::move(value))); return k; }
    BSS_FORCEINLINE const D& Peek() { return BASE::Peek().second; }
    BSS_FORCEINLINE void Discard() { BASE::Remove(0); }
    BSS_FORCEINLINE D Pop() { return BASE::Pop().second; }
    BSS_FORCEINLINE bool Empty() { return BASE::Empty(); }
    BSS_FORCEINLINE const D& Get(CT_ key) { return BASE::Get(_subarray[key]).second; }
    BSS_FORCEINLINE bool Remove(CT_ key) { return BASE::Remove(_subarray[key]); }
    BSS_FORCEINLINE bool Set(CT_ key, const D& value) { return BASE::Set(_subarray[key], PAIR(key,value)); }
    BSS_FORCEINLINE bool Set(CT_ key, D&& value) { return BASE::Set(_subarray[key], PAIR(key, std::move(value))); }
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    inline CT_ Length() { return BASE::_length; }

    inline cPriorityHeap& operator=(const cPriorityHeap& copy) { BASE::operator=(copy); _freelist = copy._freelist; return *this; }
    inline cPriorityHeap& operator=(cPriorityHeap&& mov) { BASE::operator=(std::move(mov)); _freelist = mov._freelist; return *this; }

  protected:
    CT_ _getnext()
    {
      if(_freelist == (CT_)-1)
      {
        CT_ size = _subarray.Capacity();
        _subarray.SetCapacity(fbnext(size));
        _initfreelist(size);
      }
      CT_ r = _freelist;
      _freelist = _subarray[r];
      return r;
    }
    void _initfreelist(CT_ start)
    {
      for(CT_ i = start; i < _subarray.Capacity(); ++i)
      {
        _subarray[i] = _freelist;
        _freelist = i;
      }
    }

    CT_ _freelist;
  };
}

#endif
