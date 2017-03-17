// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_RING_H__
#define __BSS_ALLOC_RING_H__

#include "lockless.h"
#include "LLBase.h"
#include "rwlock.h"

namespace bss_util {
  // Primary implementation of a multi-consumer multi-producer lockless ring allocator
  class BSS_COMPILER_DLLEXPORT cRingAllocVoid
  {
    struct Bucket
    {
      Bucket* next; // next free bucket
      size_t sz;
      std::atomic<size_t> reserved; // How much was attempted to be used (includes allocations over the limit)
      LLBase<Bucket> list; // position on permanent doubly linked list
      RWLock lock;
    };

    struct Node
    {
      size_t sz;
      Bucket* p;
    };

  public:
    cRingAllocVoid(cRingAllocVoid&& mov) : _gc(mov._gc), _lastsize(mov._lastsize), _list(mov._list)
    {
      _lock.clear(std::memory_order_release);
      _cur.store(mov._cur.load(std::memory_order_acquire), std::memory_order_release);
      mov._cur.store(0, std::memory_order_release);
      mov._list = 0;
    }
    explicit cRingAllocVoid(size_t sz) : _lastsize(sz), _list(0)
    {
      _lock.clear(std::memory_order_release);
      _gc.p = 0;
      _gc.tag = 0;
      _cur.store(_genbucket(sz), std::memory_order_release);
    }
    ~cRingAllocVoid() { _clear(); }

    inline void* alloc(size_t num) noexcept
    {
      size_t n = num + sizeof(Node);
      Bucket* cur;
      size_t r, rend;
      for(;;)
      {
        cur = _cur.load(std::memory_order_acquire);
        if(!cur->lock.AttemptRLock()) // If this fails we probably got a bucket that was in the middle of being recycled
          continue;
        r = cur->reserved.fetch_add(n, std::memory_order_acq_rel);
        rend = r + n;
        if(rend > cur->sz) // If we went over the limit, we'll have to grab another bucket
        {
          cur->reserved.fetch_sub(n, std::memory_order_release);
          cur->lock.RUnlock();
          if(!r) // If r was zero, it's theoretically possible for this bucket to get orphaned, so we send it into the _checkrecycle function
            _checkrecycle(cur); // Even if someone else acquires the lock before this runs, either the allocation will succeed or this check will
          if(!_lock.test_and_set(std::memory_order_acq_rel))
          {
            _cur.store(_genbucket(n), std::memory_order_release);
            _lock.clear(std::memory_order_release);
          }
        }
        else
          break;
      }

      Node* ret = (Node*)(((char*)(cur + 1)) + r);
#ifdef BSS_DEBUG
      uint8_t* check = (uint8_t*)ret;
      for(uint32_t i = 0; i < n; ++i) assert(check[i] == 0xfc);
#endif
      ret->sz = n;
      ret->p = cur;
      return ret + 1;
    }
    template<class T>
    inline T* allocT(size_t count = 1) noexcept { return (T*)alloc(sizeof(T)*count); }

    inline void dealloc(void* p) noexcept
    {
      Node* n = ((Node*)p)-1;
      Bucket* b = n->p; // grab bucket pointer before we annihilate the node
#ifdef BSS_DEBUG
      memset(n, 0xfc, n->sz); // n->sz is the entire size of the node, including the node itself.
#endif
      b->lock.RUnlock();
      _checkrecycle(b);
    }
    inline void Clear()
    {
      _clear();
      _cur.store(_genbucket(_lastsize), std::memory_order_release);
    }

    cRingAllocVoid& operator=(cRingAllocVoid&& mov) noexcept
    {
      _clear();
      _gc = mov._gc;
      _lastsize = mov._lastsize;
      _list = mov._list;
      _cur.store(mov._cur.load(std::memory_order_acquire), std::memory_order_release);
      mov._cur.store(0, std::memory_order_release);
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
        assert(!hold->lock.ReaderCount()); // verify no memory is being held
        free(hold);
      }
      _gc.p = 0;
      _gc.tag = 0;
      _cur.store(0, std::memory_order_release);
    }
    // It is crucial that only allocation sizes are passed into this, or the ring allocator will simply keep allocating larger and larger buckets forever
    Bucket* _genbucket(size_t num) noexcept
    {
      if(_lastsize < num) _lastsize = num;

      // If there are buckets in _gc, see if they are big enough. If they aren't, throw them out
      bss_PTag<Bucket> prev ={ 0, 0 };
      bss_PTag<Bucket> nval ={ 0, 0 };
      asmcasr<bss_PTag<Bucket>>(&_gc, prev, prev, prev);
      Bucket* hold;
      while((hold = prev.p) != 0)
      {
        nval.p = prev.p->next;
        nval.tag = prev.tag + 1;
        if(!asmcasr<bss_PTag<Bucket>>(&_gc, nval, prev, prev)) // Loop until we atomically extract the first bucket from GC
          continue;
        if(hold->sz < num) // If a bucket is too small for this allocation, destroy it
        {
          AltLLRemove<Bucket, &_getbucket>(hold, _list);
          assert(!hold->lock.ReaderCount()); // verify no memory is being held
          free(hold);
        }
        else // Otherwise, break out of the loop using this bucket
          break;
      }

      if(!hold) // If hold is nullptr we need a new bucket
      {
        _lastsize = T_FBNEXT(_lastsize);
        hold = (Bucket*)calloc(1, sizeof(Bucket)+_lastsize);
        new (&hold->lock) RWLock();
        hold->sz = _lastsize;
        AltLLAdd<Bucket, &_getbucket>(hold, _list);
#ifdef BSS_DEBUG
        memset(hold + 1, 0xfc, hold->sz);
#endif
      }
      else
      {
#ifdef BSS_DEBUG
        memset(hold + 1, 0xfc, hold->sz);
#endif
        hold->reserved.store(0, std::memory_order_relaxed);
        hold->lock.Unlock();
      }

      return hold;
    }

    void _checkrecycle(Bucket* b) noexcept
    {
      if(b->lock.AttemptStrictLock())
      {
        if(b == _cur.load(std::memory_order_acquire)) // If this appears to be the current bucket, we can't recycle it
        {
          if(!_lock.test_and_set(std::memory_order_acq_rel)) // So we attempt to acquire the lock for the _cur object
          {
            if(b == _cur.load(std::memory_order_acquire)) // If we get the lock, verify we are still the current bucket
            {
              b->reserved.store(0, std::memory_order_release); // If we are, we can "recycle" this bucket by just resetting the reserved to 0.
              b->lock.Unlock(); // Now we can unlock because we have been "recycled" into the current bucket.
              _lock.clear(std::memory_order_release);
              return;
            }
            _lock.clear(std::memory_order_release); // Otherwise, drop the lock and recycle as normal
          }
        }
        _recycle(b);
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

    BSS_ALIGN(16) bss_PTag<Bucket> _gc; // Contains a list of empty buckets that can be used to replace _root
#pragma warning(push)
#pragma warning(disable:4251)
    BSS_ALIGN(16) std::atomic<Bucket*> _cur; // Current bucket
    BSS_ALIGN(64) std::atomic_flag _lock; // GC lock
#pragma warning(pop)
    size_t _lastsize; // Last size used for a bucket.
    Bucket* _list; // root of permanent list of all buckets.
  };

  template<class T>
  class BSS_COMPILER_DLLEXPORT cRingAlloc : public cRingAllocVoid
  {
    cRingAlloc(const cRingAlloc& copy) BSS_DELETEFUNC
      cRingAlloc& operator=(const cRingAlloc& copy) BSS_DELETEFUNCOP

  public:
    inline cRingAlloc(cRingAlloc&& mov) : cRingAllocVoid(std::move(mov)) {}
    inline explicit cRingAlloc(size_t init=8) : cRingAllocVoid(init) { }
    inline T* alloc(size_t num) noexcept { return (T*)cRingAllocVoid::alloc(num*sizeof(T)); }
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