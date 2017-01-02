// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_RING_H__
#define __BSS_ALLOC_RING_H__

#include "lockless.h"
#include "LLBase.h"

namespace bss_util {
  // Primary implementation of a multi-consumer multi-producer lockless ring allocator
  class BSS_COMPILER_DLLEXPORT cRingAllocVoid
  {
    struct Bucket
    {
      Bucket* next;
      size_t sz;
      std::atomic<size_t> used; // How much is actually used (allocations that went through)
      std::atomic<size_t> free; // How much is currently freed
      std::atomic<size_t> reserved; // How much was attempted to be used (includes allocations over the limit)
      LLBase<Bucket> list; // position on permanent doubly linked list
      std::atomic_flag recycling; // flag used to ensure only one thread attempts to recycle at a time.
    };

    struct Node
    {
      size_t sz;
      Bucket* root;
    };

  public:
    cRingAllocVoid(cRingAllocVoid&& mov) : _gc(mov._gc), _lastsize(mov._lastsize), _list(mov._list)
    {
      _root.store(mov._root.load(std::memory_order_relaxed), std::memory_order_relaxed);
      _readers.store(mov._readers.load(std::memory_order_relaxed), std::memory_order_relaxed);
      _flag.store(mov._flag.load(std::memory_order_relaxed), std::memory_order_relaxed);
      mov._root = 0;
      mov._list = 0;
    }
    explicit cRingAllocVoid(size_t sz) : _lastsize(sz), _list(0)
    {
      _gc.p = 0;
      _gc.tag = 0;
      _root.store(_genbucket(sz, 0), std::memory_order_relaxed);
      _readers.store(0, std::memory_order_relaxed);
      _flag.store(false, std::memory_order_relaxed);
    }
    ~cRingAllocVoid() { _clear(); }

    inline void* BSS_FASTCALL alloc(size_t num) noexcept
    {
ALLOC_BEGIN:
      for(;;)
      {
        while(_flag.load(std::memory_order_acquire));
        _readers.fetch_add(1, std::memory_order_release);
        if(!_flag.load(std::memory_order_acquire))
          break;
        _readers.fetch_sub(1, std::memory_order_release);
        continue;
      }

      size_t n = num + sizeof(Node);
      Bucket* root = _root.load(std::memory_order_acquire);
      size_t r = root->reserved.fetch_add(n, std::memory_order_acquire) + n; // Add num here because fetch_add returns the PREVIOUS value
      if(r > root->sz) // If we went over the limit, we'll have to grab another bucket
      {
        _readers.fetch_sub(1, std::memory_order_release);
        if(!_flag.exchange(true, std::memory_order_acq_rel))
        {
          while(_readers.load(std::memory_order_acquire)>0);
          root = _root.load(std::memory_order_acquire);
          if(root->reserved.load(std::memory_order_relaxed) > root->sz) // Ensure the root actually still needs to be swapped out to avoid race conditions.
          { 
            Bucket* oldroot = root;
            // If the next bucket doesn't exist or isn't big enough, allocate a new one.
            if(root->next != 0 && root->next->sz >= n)
              root = root->next;
            else
              root = _genbucket(n, root->next);
            _root.store(root, std::memory_order_release);
            _checkrecycle(oldroot);
          }
          _flag.store(false, std::memory_order_release);
        }
        goto ALLOC_BEGIN; // Restart the entire function (a goto is used here instead of a break out of an infinite for loop because that's arguably even harder to understand)
      }

      // If we get here, we know our last addition didn't go over the limit, so we can add our amount to used.
      size_t pos = root->used.fetch_add(n, std::memory_order_acq_rel); // The position returned here is our unique allocation.
      Node* ret = (Node*)(((char*)(root + 1)) + pos);
#ifdef BSS_DEBUG
      uint8_t* check = (uint8_t*)ret;
      for(uint32_t i = 0; i < n; ++i) assert(check[i] == 0xfc);
#endif
      ret->sz = n;
      ret->root = root;
      _readers.fetch_sub(1, std::memory_order_release);
      return ret+1;
    }
    inline void BSS_FASTCALL dealloc(void* p) noexcept
    {
      Node* n = ((Node*)p)-1;
      n->root->free.fetch_add(n->sz, std::memory_order_release);
      _checkrecycle(n->root);
#ifdef BSS_DEBUG
      memset(n, 0xfc, n->sz); // n->sz is the entire size of the node, including the node itself.
#endif
    }
    inline void Clear()
    {
      _clear();
      _root.store(_genbucket(_lastsize, 0), std::memory_order_relaxed);
    }

    cRingAllocVoid& operator=(cRingAllocVoid&& mov) noexcept
    {
      _clear();
      _gc = mov._gc;
      _lastsize = mov._lastsize;
      _list = mov._list;
      _root.store(mov._root.load(std::memory_order_relaxed), std::memory_order_relaxed);
      _readers.store(mov._readers.load(std::memory_order_relaxed), std::memory_order_relaxed);
      _flag.store(mov._flag.load(std::memory_order_relaxed), std::memory_order_relaxed);
      mov._root = 0;
      mov._list = 0;
      return *this;
    }

  protected:
    BSS_FORCEINLINE static LLBase<Bucket>& _getbucket(Bucket* b) noexcept { return b->list; }

    void _clear() 
    {
      Bucket* hold;
      while(_list != 0)
      {
        hold = _list;
        _list = _list->list.next;
        free(hold);
      }
      _gc.p = 0;
      _gc.tag = 0;
      _root.store(0, std::memory_order_relaxed);
    }
    Bucket* _genbucket(size_t num, Bucket* next) noexcept
    {
      if(_lastsize < num) _lastsize = num;

      // If there are buckets in _gc, see if they are big enough. If they aren't, throw them out
      bss_PTag<Bucket> prev ={ 0, 0 };
      bss_PTag<Bucket> nval ={ 0, 0 };
      while(!asmcasr<bss_PTag<Bucket>>(&_gc, nval, prev, prev)); // set _gc to null

      Bucket* hold = prev.p;
      while(hold != 0 && hold->sz < num) // throw any buckets that are too small back into _gc
      {
        prev.p = hold;
        hold = hold->next;
        _recycle(prev.p);
      }

      if(!hold) // If hold is nullptr we need a new bucket
      {
        _lastsize = T_FBNEXT(_lastsize);
        hold = (Bucket*)malloc(sizeof(Bucket)+_lastsize);
        hold->sz = _lastsize;
        hold->next = 0;
        hold->list.next = hold->list.prev = 0;
        hold->recycling.clear();
        AltLLAdd<Bucket, &_getbucket>(hold, _list);
      }

      if(hold->next) // If there are trailing buckets we need to put them back in the GC
        _recycle(hold->next);
      hold->next = next;
      hold->used.store(0, std::memory_order_relaxed);
      hold->reserved.store(0, std::memory_order_relaxed);
      hold->free.store(0, std::memory_order_relaxed);

#ifdef BSS_DEBUG
      memset(hold+1, 0xfc, hold->sz);
#endif

      return hold;
    }

    void _checkrecycle(Bucket* b) noexcept
    {
      if(b->used.load(std::memory_order_acquire) == b->free.load(std::memory_order_acquire)
        && b->reserved.load(std::memory_order_acquire) > b->sz
        && b != _root.load(std::memory_order_acquire))
      {
        if(!b->recycling.test_and_set(std::memory_order_acq_rel))
        {
          _recycle(b);
          b->recycling.clear();
        }
      }
    }

    void _recycle(Bucket* b) noexcept
    {
      bss_PTag<Bucket> prev ={ 0, 0 };
      bss_PTag<Bucket> nval ={ b, 0 };
      asmcasr<bss_PTag<Bucket>>(&_gc, prev, prev, prev); // This will almost always fail but it only serves to get the value of _gc
      do
      {
        b->next = prev.p;
        nval.tag = prev.tag+1;
      } while(!asmcasr<bss_PTag<Bucket>>(&_gc, nval, prev, prev));
    }

    BSS_ALIGN(16) bss_PTag<Bucket> _gc; // Contains a list of discarded buckets
#pragma warning(push)
#pragma warning(disable:4251)
    BSS_ALIGN(16) std::atomic<Bucket*> _root;
    BSS_ALIGN(64) std::atomic<size_t> _readers; // How many threads are currently depending on _root to not change
    BSS_ALIGN(64) std::atomic_bool _flag; // Blocking flag for when _root needs to change
#pragma warning(pop)
    size_t _lastsize; // Last size used for a bucket.
    Bucket* _list; // root of permanent list of all active allocated buckets.
  };

  template<class T>
  class BSS_COMPILER_DLLEXPORT cRingAlloc : public cRingAllocVoid
  {
    cRingAlloc(const cRingAlloc& copy) BSS_DELETEFUNC
      cRingAlloc& operator=(const cRingAlloc& copy) BSS_DELETEFUNCOP

  public:
    inline cRingAlloc(cRingAlloc&& mov) : cRingAllocVoid(std::move(mov)) {}
    inline explicit cRingAlloc(size_t init=8) : cRingAllocVoid(init) { }
    inline T* BSS_FASTCALL alloc(size_t num) noexcept { return (T*)cRingAllocVoid::alloc(num*sizeof(T)); }
    inline cRingAlloc& operator=(cRingAlloc&& mov) noexcept { cRingAllocVoid::operator=(std::move(mov)); return *this; }
  };

  template<typename T>
  class BSS_COMPILER_DLLEXPORT RingPolicy : protected cRingAlloc<T>
  {
    RingPolicy(const RingPolicy& copy) BSS_DELETEFUNC
      RingPolicy& operator=(const RingPolicy& copy) BSS_DELETEFUNCOP

  public:
    typedef T* pointer;
    typedef T value_type;
    template<typename U>
    struct rebind { typedef RingPolicy<U> other; };

    inline RingPolicy(RingPolicy&& mov) : cRingAlloc<T>(std::move(mov)) {}
    inline RingPolicy() {}
    inline explicit RingPolicy(size_t init) : cRingAlloc<T>(init) {}
    inline ~RingPolicy() {}
    inline RingPolicy& operator=(RingPolicy&& mov) noexcept { cRingAlloc<T>::operator=(std::move(mov)); return *this; }

    inline pointer allocate(size_t cnt, const pointer = 0) noexcept { return cRingAlloc<T>::alloc(cnt); }
    inline void deallocate(pointer p, size_t num = 0) noexcept { return cRingAlloc<T>::dealloc(p); }
  };
}

#endif