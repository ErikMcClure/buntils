// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_ALLOC_REUSE_H__
#define __BUN_ALLOC_REUSE_H__

#include "GreedyAlloc.h"
#include "Hash.h"

namespace bun {
  // Multithreaded allocator that matches exact allocation sizes with a hash, backed with a greedy allocator.
  class BUN_COMPILER_DLLEXPORT CacheAlloc : protected GreedyAlloc
  {
    CacheAlloc(const CacheAlloc& copy) = delete;
    CacheAlloc& operator=(const CacheAlloc& copy) = delete;
    
  public:
    inline CacheAlloc(CacheAlloc&& mov) : GreedyAlloc(std::move(mov)), _maxsize(mov._maxsize), _debugalign(mov._debugalign) {}
    inline explicit CacheAlloc(size_t maxsize = 8192, size_t init = 64, size_t align = 1) : GreedyAlloc(init, align), _maxsize(maxsize), _debugalign(bun_max(align, sizeof(size_t))) {}
    inline ~CacheAlloc() {}

    template<typename T>
    inline T* AllocT(size_t num) noexcept
    {
      return (T*)Alloc(num * sizeof(T));
    }
    inline void* Alloc(size_t sz) noexcept
    {
      char* p;
      if(sz > _maxsize)
#ifdef BUN_DEBUG
        p = (char*)malloc(sz + _debugalign);
#else
        p = (char*)malloc(sz);
#endif
      else
      {
        _cachelock.RLock();
        khiter_t i = _cache.Iterator(sz);
        if(!_cache.ExistsIter(i))
        {
          _cachelock.Upgrade();
          i = _cache.Insert(sz, { 0, 0 });
          _cachelock.Downgrade();
        }
        assert(_cache.ExistsIter(i));
        bun_PTag<void>& freelist = _cache.Value(i);
        bun_PTag<void> ret = { 0, 0 };
        bun_PTag<void> nval;
        asmcasr<bun_PTag<void>>(&freelist, ret, ret, ret);
        assert(!ret.p || _verifyDEBUG(ret.p));

        for(;;)
        {
          if(!ret.p)
          {
            size_t realsz = bun_max(sz, sizeof(void*));
#ifdef BUN_DEBUG
            ret.p = GreedyAlloc::Alloc(realsz + _debugalign);
#else
            ret.p = GreedyAlloc::Alloc(realsz);
#endif
            break;
          }

          nval.p = *((void**)ret.p);
          nval.tag = ret.tag + 1;

          if(asmcasr<bun_PTag<void>>(&freelist, nval, ret, ret))
            break;
        }

        _cachelock.RUnlock();
        p = (char*)ret.p;
      }

#ifdef BUN_DEBUG
      *reinterpret_cast<size_t*>(p) = sz;
      return p + _debugalign;
#else
      return p;
#endif
    }
    inline void Dealloc(void* p, size_t sz) noexcept
    {
#ifdef BUN_DEBUG
      void* r = p;
      p = reinterpret_cast<char*>(p) - _debugalign;
      assert(*reinterpret_cast<size_t*>(p) == sz);
#endif
      if(sz > _maxsize)
        free(p);
      else
      {
        _cachelock.RLock();
#ifdef BUN_DEBUG
        memset(r, 0xfd, sz);
#endif
        khiter_t i = _cache.Iterator(sz);
        assert(_cache.ExistsIter(i));
        _setFreeList(&_cache.Value(i), p, p);
        _cachelock.RUnlock();
      }
    }

    CacheAlloc& operator=(CacheAlloc&& mov) noexcept
    {
      GreedyAlloc::operator=(std::move(mov));
      _cache = std::move(mov._cache);
      assert(_maxsize == mov._maxsize);
      assert(_debugalign == mov._debugalign);
      return *this;
    }

  protected:
    inline static void _setFreeList(bun_PTag<void>* freelist, void* p, void* target) noexcept
    {
      bun_PTag<void> prev = { 0, 0 };
      bun_PTag<void> nval = { p, 0 };
      asmcasr<bun_PTag<void>>(freelist, prev, prev, prev);

      do
      {
        nval.tag = prev.tag + 1;
        *((void**)(target)) = (void*)prev.p;
      } while(!asmcasr<bun_PTag<void>>(freelist, nval, prev, prev));
    }

    alignas(16) RWLock _cachelock;
    const size_t _maxsize;
    const size_t _debugalign;
    HashIns<size_t, bun_PTag<void>, StandardAllocator<std::byte, 16>> _cache; // Each size in the hash points to a freelist for allocations of that size
  };


  template<typename T>
  struct BUN_COMPILER_DLLEXPORT CachePolicy : protected CacheAlloc
  {
    CachePolicy() = default;
    inline explicit CachePolicy(size_t maxsize, size_t init = 64, size_t align = 1) : CacheAlloc(maxsize, init, align) {}

    inline T* allocate(std::size_t cnt, T* p = nullptr, size_t old = 0) noexcept
    {
      T* n = CacheAlloc::AllocT<T>(cnt);
      if(p)
      {
        assert(old > 0);
        if(cnt < old)
          old = cnt;
        MEMCPY(n, cnt * sizeof(T), p, old * sizeof(T));
        CacheAlloc::Dealloc(p, old * sizeof(T));
      }
      return n;
    }
    inline void deallocate(T* p, std::size_t num = 0) noexcept { CacheAlloc::Dealloc(p, num * sizeof(T)); }
    inline void Clear() noexcept { }
    void VERIFY() {
      _cachelock.RLock();
      for(khiter_t i : _cache)
      {
        void* p = _cache.Value(i).p;
        while(p)
        {
          if(!_verifyDEBUG(p))
          {
            printf("%p: %zu", p, _cache.GetKey(i));
            //DUMP();
            abort();
          }
          p = *((void**)p);
        }
      }
      _cachelock.RUnlock();
    }
  };
}

#endif
