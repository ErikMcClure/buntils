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
}

#endif
