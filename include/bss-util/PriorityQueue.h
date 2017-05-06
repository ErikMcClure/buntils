// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __PRIORITY_QUEUE_H__BSS__
#define __PRIORITY_QUEUE_H__BSS__

#include "BinaryHeap.h"

namespace bss {
  // PriorityQueue that can be implemented as either a maxheap or a minheap
  template<typename K, typename D, char(*CFunc)(const K&, const K&) = CompT<K>, typename CT_ = uint32_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<std::pair<K, D>>>
  class BSS_COMPILER_DLLEXPORT PriorityQueue : protected BinaryHeap<std::pair<K, D>, CT_, CompTFirst<std::pair<K, D>, CFunc>, ArrayType, Alloc>
  {
    typedef std::pair<K, D> PAIR;
    typedef BinaryHeap<std::pair<K, D>, CT_, CompTFirst<std::pair<K, D>, CFunc>, ArrayType, Alloc> BASE;

  public:
    PriorityQueue(const PriorityQueue& copy) : BASE(copy) {}
    PriorityQueue(PriorityQueue&& mov) : BASE(std::move(mov)) {}
    PriorityQueue() {}
    ~PriorityQueue() {}
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

    inline PriorityQueue& operator=(const PriorityQueue& copy) { BASE::operator=(copy); return *this; }
    inline PriorityQueue& operator=(PriorityQueue&& mov) { BASE::operator=(std::move(mov)); return *this; }
  };

  template<class T, typename CT_>
  struct MFUNC_PRIORITY {
    BSS_FORCEINLINE static void MFunc(const T& item, CT_ i, MFUNC_PRIORITY* p) { p->_subarray[item.first] = i; }
    Array<CT_, CT_> _subarray;
  };

  template<typename D, typename CT_ = uint32_t, char(*CFunc)(const D&, const D&) = CompT<D>, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<std::pair<CT_, D>>>
  class BSS_COMPILER_DLLEXPORT PriorityHeap : protected BinaryHeap<std::pair<CT_, D>, CT_, CompTSecond<std::pair<CT_, D>, CFunc>, ArrayType, Alloc, MFUNC_PRIORITY<std::pair<CT_, D>, CT_>>
  {
    typedef std::pair<CT_, D> PAIR;
    typedef BinaryHeap<std::pair<CT_, D>, CT_, CompTSecond<std::pair<CT_, D>, CFunc>, ArrayType, Alloc, MFUNC_PRIORITY<std::pair<CT_, D>, CT_>> BASE;
    using BASE::_subarray;
  public:
    PriorityHeap(const PriorityHeap& copy) : BASE(copy), _freelist(copy._freelist) { _subarray = copy._subarray; }
    PriorityHeap(PriorityHeap&& mov) : BASE(std::move(mov)), _freelist(mov._freelist) { _subarray = std::move(mov._subarray); }
    PriorityHeap() : _freelist((CT_)-1) {}
    ~PriorityHeap() {}
    BSS_FORCEINLINE CT_ Push(const D& value) { CT_ k = _getNext(); BASE::Insert(PAIR(k, value)); return k; }
    BSS_FORCEINLINE CT_ Push(D&& value) { CT_ k = _getNext(); BASE::Insert(PAIR(k, std::move(value))); return k; }
    BSS_FORCEINLINE const D& Peek() { return BASE::Peek().second; }
    BSS_FORCEINLINE void Discard() { BASE::Remove(0); }
    BSS_FORCEINLINE D Pop() { return BASE::Pop().second; }
    BSS_FORCEINLINE bool Empty() { return BASE::Empty(); }
    BSS_FORCEINLINE const D& Get(CT_ key) { return BASE::Get(_subarray[key]).second; }
    BSS_FORCEINLINE bool Remove(CT_ key) { return BASE::Remove(_subarray[key]); }
    BSS_FORCEINLINE bool Set(CT_ key, const D& value) { return BASE::Set(_subarray[key], PAIR(key, value)); }
    BSS_FORCEINLINE bool Set(CT_ key, D&& value) { return BASE::Set(_subarray[key], PAIR(key, std::move(value))); }
    BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    inline CT_ Length() { return BASE::_length; }

    inline PriorityHeap& operator=(const PriorityHeap& copy) { BASE::operator=(copy); _freelist = copy._freelist; return *this; }
    inline PriorityHeap& operator=(PriorityHeap&& mov) { BASE::operator=(std::move(mov)); _freelist = mov._freelist; return *this; }

  protected:
    CT_ _getNext()
    {
      if(_freelist == (CT_)-1)
      {
        CT_ size = _subarray.Capacity();
        _subarray.SetCapacity(fbnext(size));
        _initFreeList(size);
      }
      CT_ r = _freelist;
      _freelist = _subarray[r];
      return r;
    }
    void _initFreeList(CT_ start)
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
