// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_PRIORITY_QUEUE_H__BSS__
#define __C_PRIORITY_QUEUE_H__BSS__

#include "cBinaryHeap.h"

namespace bss_util {
  /* PriorityQueue that can be implemented as either a maxheap or a minheap */
  template<class K, class D, char (*CFunc)(const K&, const K&)=CompT<K>, typename __ST=unsigned int>
  class BSS_COMPILER_DLLEXPORT cPriorityQueue : private cBinaryHeap<std::pair<K,D>,__ST,CompTFirst<std::pair<K,D>,K,CFunc>>
  {
    typedef std::pair<K,D> PAIR;

  public:
    inline cPriorityQueue(const cPriorityQueue& copy) : cBinaryHeap(copy) {}
    inline cPriorityQueue(cPriorityQueue&& mov) : cBinaryHeap(std::move(mov)) {}
    inline cPriorityQueue() {}
    inline ~cPriorityQueue() {}
    inline void Insert(const K& key, D value) { cBinaryHeap<K,D,CFunc>::Insert(PAIR(key,value)); }
    inline void Insert(K&& key, D value) { cBinaryHeap<K,D,CFunc>::Insert(PAIR(std::move(key),value)); }
    inline const PAIR& PeekRoot() { return cBinaryHeap<K,D,CFunc>::GetRoot(); }
    inline void RemoveRoot() { cBinaryHeap<K,D,CFunc>::Remove(0); }
    inline PAIR PopRoot()
    {
      PAIR retval = _array[0];
      RemoveRoot();
      return retval;
    }
    inline bool Empty() { return cBinaryHeap<K,D,CFunc>::Empty(); }

    inline cPriorityQueue& operator=(const cPriorityQueue& copy) { cBinaryHeap::operator=(copy); return *this; }
    inline cPriorityQueue& operator=(cPriorityQueue&& mov) { cBinaryHeap::operator=(std::move(mov)); return *this; }
  };
}

#endif