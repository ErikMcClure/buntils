// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_PRIORITY_QUEUE_H__BSS__
#define __C_PRIORITY_QUEUE_H__BSS__

#include "cBinaryHeap.h"

namespace bss_util {
  // PriorityQueue that can be implemented as either a maxheap or a minheap
  template<typename K, typename D, char (*CFunc)(const K&, const K&)=CompT<K>, typename ST_=unsigned int, typename ARRAYTYPE=cArraySimple<std::pair<K,D>,ST_>>
  class BSS_COMPILER_DLLEXPORT cPriorityQueue : private cBinaryHeap<std::pair<K,D>,ST_,CompTFirst<std::pair<K,D>,CFunc>,ARRAYTYPE>
  {
    typedef std::pair<K,D> PAIR;
    typedef cBinaryHeap<std::pair<K,D>,ST_,CompTFirst<std::pair<K,D>,CFunc>,ARRAYTYPE> BASE;

  public:
    inline cPriorityQueue(const cPriorityQueue& copy) : BASE(copy) {}
    inline cPriorityQueue(cPriorityQueue&& mov) : BASE(std::move(mov)) {}
    inline cPriorityQueue() {}
    inline ~cPriorityQueue() {}
    inline BSS_FORCEINLINE void Push(const K& key, D value) { BASE::Insert(PAIR(key,value)); }
    inline BSS_FORCEINLINE void Push(K&& key, D value) { BASE::Insert(PAIR(std::move(key),value)); }
    inline BSS_FORCEINLINE const PAIR& Peek() { return BASE::GetRoot(); }
    inline BSS_FORCEINLINE void Discard() { BASE::Remove(0); }
    inline BSS_FORCEINLINE PAIR Pop() { return std::move(BASE::PopRoot()); }
    inline BSS_FORCEINLINE bool Empty() { return BASE::Empty(); }
    inline BSS_FORCEINLINE const PAIR& Get(ST_ index) { return BASE::Get(index); }
    inline BSS_FORCEINLINE bool Remove(ST_ index) { return BASE::Remove(index); }
    inline ST_ Length() { return BASE::_length; }

    inline cPriorityQueue& operator=(const cPriorityQueue& copy) { BASE::operator=(copy); return *this; }
    inline cPriorityQueue& operator=(cPriorityQueue&& mov) { BASE::operator=(std::move(mov)); return *this; }
  };
}

#endif
