// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __PRIORITY_QUEUE_H__BUN__
#define __PRIORITY_QUEUE_H__BUN__

#include "BinaryHeap.h"

namespace bun {
  // PriorityQueue that can be implemented as either a maxheap or a minheap
  template<typename K,
    typename D,
    char(*CFunc)(const K&, const K&) = CompT<K>,
    typename CT_ = size_t, 
    typename Alloc = StandardAllocator<std::pair<K, D>>>
  class BUN_COMPILER_DLLEXPORT PriorityQueue : protected BinaryHeap<std::pair<K, D>, CT_, CompTFirst<K, D, CFunc>, Alloc>
  {
    using PAIR = std::pair<K, D>;
    using BASE = BinaryHeap<std::pair<K, D>, CT_, CompTFirst<K, D, CFunc>, Alloc>;

  public:
    PriorityQueue(const PriorityQueue& copy) = default;
    PriorityQueue(PriorityQueue&& mov) = default;
    explicit PriorityQueue(const Alloc& alloc) : BASE(alloc) {}
    PriorityQueue() {}
    ~PriorityQueue() {}
    BUN_FORCEINLINE void Push(const K& key, D value) { BASE::Insert(PAIR(key, value)); }
    BUN_FORCEINLINE void Push(K&& key, D value) { BASE::Insert(PAIR(std::move(key), value)); }
    BUN_FORCEINLINE const PAIR& Peek() { return BASE::Peek(); }
    BUN_FORCEINLINE void Discard() { BASE::Remove(0); }
    BUN_FORCEINLINE PAIR Pop() { return std::move(BASE::Pop()); }
    BUN_FORCEINLINE bool Empty() { return BASE::Empty(); }
    BUN_FORCEINLINE const PAIR& Get(CT_ index) { return BASE::Get(index); }
    BUN_FORCEINLINE bool Remove(CT_ index) { return BASE::Remove(index); }
    BUN_FORCEINLINE void Clear() { BASE::Clear(); }
    inline CT_ Length() { return BASE::_length; }

    inline PriorityQueue& operator=(const PriorityQueue& copy) = default;
    inline PriorityQueue& operator=(PriorityQueue&& mov) = default;

    using BASE::SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { BASE::template Serialize<Engine>(s, id); }
  };

  namespace internal {
    template<class T, typename CT_>
    struct MFUNC_PRIORITY {
      BUN_FORCEINLINE static void MFunc(const T& item, CT_ i, MFUNC_PRIORITY* p) { p->_subarray[item.first] = i; }
      Array<CT_, CT_> _subarray;
    };
  }

  template<typename D,
    typename CT_ = size_t,
    char(*CFunc)(const D&, const D&) = CompT<D>,
    typename Alloc = StandardAllocator<std::pair<CT_, D>>>
  class BUN_COMPILER_DLLEXPORT PriorityHeap : protected BinaryHeap<std::pair<CT_, D>, CT_, CompTSecond<CT_, D, CFunc>, Alloc, internal::MFUNC_PRIORITY<std::pair<CT_, D>, CT_>>
  {
    using PAIR = std::pair<CT_, D>;
    using BASE = BinaryHeap<std::pair<CT_, D>, CT_, CompTSecond<CT_, D, CFunc>, Alloc, internal::MFUNC_PRIORITY<std::pair<CT_, D>, CT_>>;
    using BASE::_subarray;

  public:
    PriorityHeap(const PriorityHeap& copy) : BASE(copy), _freelist(copy._freelist) { _subarray = copy._subarray; }
    PriorityHeap(PriorityHeap&& mov) : BASE(std::move(mov)), _freelist(mov._freelist) { _subarray = std::move(mov._subarray); }
    explicit PriorityHeap(const Alloc& alloc) : BASE(alloc), _freelist((CT_)-1) {}
    PriorityHeap() : _freelist((CT_)-1) {}
    ~PriorityHeap() {}
    BUN_FORCEINLINE CT_ Push(const D& value) { CT_ k = _getNext(); BASE::Insert(PAIR(k, value)); return k; }
    BUN_FORCEINLINE CT_ Push(D&& value) { CT_ k = _getNext(); BASE::Insert(PAIR(k, std::move(value))); return k; }
    BUN_FORCEINLINE const D& Peek() { return BASE::Peek().second; }
    BUN_FORCEINLINE void Discard() { BASE::Remove(0); }
    BUN_FORCEINLINE D Pop() { return BASE::Pop().second; }
    BUN_FORCEINLINE bool Empty() { return BASE::Empty(); }
    BUN_FORCEINLINE const D& Get(CT_ key) { return BASE::Get(_subarray[key]).second; }
    BUN_FORCEINLINE bool Remove(CT_ key) { return BASE::Remove(_subarray[key]); }
    BUN_FORCEINLINE bool Set(CT_ key, const D& value) { return BASE::Set(_subarray[key], PAIR(key, value)); }
    BUN_FORCEINLINE bool Set(CT_ key, D&& value) { return BASE::Set(_subarray[key], PAIR(key, std::move(value))); }
    BUN_FORCEINLINE void Clear() { BASE::Clear(); }
    inline CT_ Length() { return BASE::_length; }

    inline PriorityHeap& operator=(const PriorityHeap& copy) 
    {
      BASE::operator=(copy); 
      _freelist = copy._freelist; 
      return *this; 
    }
    inline PriorityHeap& operator=(PriorityHeap&& mov) 
    { 
      BASE::operator=(std::move(mov)); 
      _freelist = mov._freelist; 
      return *this; 
    }

    using BASE::SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { BASE::template Serialize<Engine>(s, id); }

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
