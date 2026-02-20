// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_ALLOC_BLOCK_LOCKLESS_H__
#define __BUN_ALLOC_BLOCK_LOCKLESS_H__

#include "BlockAlloc.h"
#include "lockless.h"

namespace bun {
  /* Multi-producer multi-consumer lockless fixed size allocator */
  class BUN_COMPILER_DLLEXPORT LocklessBlockAllocator
  {
    using Node = BlockAlloc::Node;

  public:
    inline LocklessBlockAllocator(LocklessBlockAllocator&& mov) : _root(mov._root), _blocksize(mov._blocksize)
    {
      static_assert((sizeof(bun_PTag<void>) == (sizeof(void*) * 2)), "ABAPointer isn't twice the size of a pointer!");
      _freelist.p     = mov._freelist.p;
      _freelist.tag   = mov._freelist.tag;
      mov._freelist.p = mov._root = nullptr;
      _flag.clear(std::memory_order_relaxed);
    }
    inline LocklessBlockAllocator(size_t blocksize, size_t init) : _root(nullptr), _blocksize(blocksize)
    {
      _flag.clear(std::memory_order_relaxed);
      _freelist.p   = nullptr;
      _freelist.tag = 0;
      _allocChunk(init * _blocksize);
    }
    inline LocklessBlockAllocator(size_t blocksize) : _root(nullptr), _blocksize(blocksize)
    {
      _flag.clear(std::memory_order_relaxed);
      _freelist.p   = nullptr;
      _freelist.tag = 0;
      _allocChunk(8 * _blocksize);
    }
    inline LocklessBlockAllocator() : _root(nullptr), _blocksize(sizeof(void*))
    {
      _flag.clear(std::memory_order_relaxed);
      _freelist.p   = nullptr;
      _freelist.tag = 0;
      _allocChunk(8 * _blocksize);
    }
    inline ~LocklessBlockAllocator() { _destroy(); }
    inline void Clear() noexcept
    {
      if(_root)
      {
        while(_flag.test_and_set(std::memory_order_acquire))
          ;
        size_t init = _root->size;
        _destroy();
        _flag.clear(std::memory_order_relaxed);
        _freelist.p   = nullptr;
        _freelist.tag = 0;
        _allocChunk(init);
        _flag.clear(std::memory_order_release);
      }
    }

    inline void* alloc(size_t blocks, void* p = nullptr, [[maybe_unused]] size_t old = 0) noexcept
    {
      assert(blocks == 1 && !p);
#ifdef BUN_DISABLE_CUSTOM_ALLOCATORS
      return bun_Malloc(blocks * _blocksize);
#endif
      bun_PTag<void> ret = { 0, 0 };
      bun_PTag<void> nval;
      asmcasr<bun_PTag<void>>(&_freelist, ret, ret, ret);

      for(;;)
      {
        if(!ret.p)
        {
          if(!_flag.test_and_set(std::memory_order_acquire))
          {                  // If we get the lock, do a new allocation
            if(!_freelist.p) // Check this due to race condition where someone finishes a new allocation and unlocks while
                             // we were testing that flag.
              _allocChunk(fbnext(_root->size / _blocksize) * _blocksize);
            _flag.clear(std::memory_order_release);
          }
          asmcasr<bun_PTag<void>>(
            &_freelist, ret, ret,
            ret); // we could put this in the while loop but then you have to set nval to ret and it's just as messy
          continue;
        }

        nval.p   = *((void**)ret.p);
        nval.tag = ret.tag + 1;

        if(asmcasr<bun_PTag<void>>(&_freelist, nval, ret, ret))
          break;
      }

      // assert(_validPointer(ret));
      return ret.p;
    }
    inline void dealloc(void* p, [[maybe_unused]] size_t num = 0) noexcept
    {
#ifdef BUN_DISABLE_CUSTOM_ALLOCATORS
      free(p);
      return;
#endif
      assert(_validPointer(p));
#ifdef BUN_DEBUG
      memset(p, 0xfd, _blocksize);
#endif
      _setFreeList(p, p);
      //*((void**)p)=(void*)_freelist;
      // while(!asmcas<void*>(&_freelist,p,*((void**)p))) //ABA problem
      //  *((void**)p)=(void*)_freelist;
    }

    LocklessBlockAllocator& operator=(LocklessBlockAllocator&& mov) noexcept
    {
      _root           = mov._root;
      _blocksize      = mov._blocksize;
      _freelist.p     = mov._freelist.p;
      _freelist.tag   = mov._freelist.tag;
      mov._freelist.p = mov._root = 0;
      _flag.clear(std::memory_order_relaxed);
      return *this;
    }
    size_t GetBlockSize() const noexcept { return _blocksize; }

  protected:
    inline void _destroy() noexcept
    {
      Node* hold = _root;

      while((_root = hold))
      {
        hold = _root->next;
        free(_root);
      }
    }

#ifdef BUN_DEBUG
    inline bool _validPointer(const void* p) const
    {
      const Node* hold = _root;
      while(hold)
      {
        if(p >= (hold + 1) && p < (((std::byte*)(hold + 1)) + hold->size))
          return ((((std::byte*)p) - ((std::byte*)(hold + 1))) % _blocksize) ==
                 0; // the pointer should be an exact multiple of sizeof(T)

        hold = hold->next;
      }
      return false;
    }
#endif
    inline void _allocChunk(size_t nsize) noexcept
    {
      Node* retval = reinterpret_cast<Node*>(malloc(sizeof(Node) + nsize));
      assert(retval != 0);
      retval->next = _root;
      retval->size = nsize;
      _root = retval; // There's a potential race condition on DEBUG mode only where failing to set this first would allow a
                      // thread to allocate a new pointer and then delete it before _root got changed, which would then be
                      // mistaken for an invalid pointer
      _initChunk(retval);
    }

    inline void _initChunk(const Node* chunk) noexcept
    {
      void* hold        = 0;
      std::byte* memend = ((std::byte*)(chunk + 1)) + chunk->size;

      for(std::byte* memref = (std::byte*)(chunk + 1); memref < memend; memref += _blocksize)
      {
        *((void**)(memref)) = hold;
        hold                = memref;
      }

      _setFreeList(hold,
                   (void*)(chunk +
                           1)); // The target here is different because normally, the first block (at chunk+1) would point
                                // to whatever _freelist used to be. However, since we are lockless, _freelist could not be
                                // 0 at the time we insert this, so we have to essentially go backwards and set the first
                                // one to whatever freelist is NOW, before setting freelist to the one on the end.
    }

    inline void _setFreeList(void* p, void* target) noexcept
    {
      bun_PTag<void> prev = { 0, 0 };
      bun_PTag<void> nval = { p, 0 };
      asmcasr<bun_PTag<void>>(&_freelist, prev, prev, prev);

      do
      {
        nval.tag            = prev.tag + 1;
        *((void**)(target)) = (void*)prev.p;
        // contention.fetch_add(1); //DEBUG
      } while(!asmcasr<bun_PTag<void>>(&_freelist, nval, prev, prev));
    }

#pragma warning(push)
#pragma warning(disable : 4251)
    alignas(16) volatile bun_PTag<void> _freelist;
    alignas(4) std::atomic_flag _flag;
#pragma warning(pop)
    Node* _root;
    size_t _blocksize;
  };

  /* Multi-producer multi-consumer lockless fixed size allocator */
  template<class T>
    requires((sizeof(T) >= sizeof(void*)) && (sizeof(bun_PTag<void>) == (sizeof(void*) * 2)))
  class BUN_COMPILER_DLLEXPORT LocklessBlockPolicy : public LocklessBlockAllocator
  {
  public:
    inline LocklessBlockPolicy(LocklessBlockPolicy&&) = default;
    inline explicit LocklessBlockPolicy(size_t init = 8) : LocklessBlockAllocator(sizeof(T), init) {}
    inline ~LocklessBlockPolicy() {}

    inline T* allocate(size_t num, T* p = nullptr, size_t old = 0) noexcept
    {
      return reinterpret_cast<T*>(LocklessBlockAllocator::alloc(num, p, old));
    }
    inline void deallocate(T* p, size_t num = 0) noexcept { LocklessBlockAllocator::dealloc(p, num); }

    LocklessBlockPolicy& operator=(LocklessBlockPolicy&&) = default;
  };

  template<size_t MAXSIZE = sizeof(void*) * 32, class = std::make_index_sequence<bun_Log2(MAXSIZE / sizeof(void*)) + 1>>
  class BUN_COMPILER_DLLEXPORT LocklessBlockCollection;

  /* A collection of block allocators up to a given size */
  template<size_t MAXSIZE, size_t... Is>
  class BUN_COMPILER_DLLEXPORT LocklessBlockCollection<MAXSIZE, std::index_sequence<Is...>>
  {
    static constexpr size_t COUNT = sizeof...(Is);

    inline static constexpr size_t _getindex(size_t n) { return bun_Log2(NextPow2(n) >> 3); }

  public:
    inline LocklessBlockCollection(LocklessBlockCollection&&) = default;
    LocklessBlockCollection() : _allocators{ (sizeof(void*) << Is)... } {}
    inline void* allocate(size_t num, void* p = nullptr, size_t old = 0) noexcept
    {
      auto idx = _getindex(num);
      if(p)
      {
        assert(old != 0);
        auto prev = _getindex(old);
        if(prev == idx)
        {
          if(idx < COUNT)
            return p;
        }
        else
          deallocate(p, old);
      }

      if(idx >= COUNT)
        return realloc(p, num);
      else
      {
        assert(num <= _allocators[idx].GetBlockSize());
        return _allocators[idx].alloc(1, nullptr, 0);
      }
    }
    template<class T> inline T* allocT(size_t num) { return reinterpret_cast<T*>(allocate(num * sizeof(T))); }
    inline void deallocate(void* p, size_t num) noexcept
    {
      assert(num != 0);
      auto idx = _getindex(num);
      if(idx >= COUNT)
        free(p);
      else
        _allocators[idx].dealloc(p, 1);
    }
    template<class T> inline void deallocT(T* p, size_t num) { deallocate(p, num * sizeof(T)); }
    inline void Clear() noexcept
    {
      for(auto& alloc : _allocators)
      {
        alloc.Clear();
      }
    }

    LocklessBlockCollection& operator=(LocklessBlockCollection&&) = default;

  protected:
    LocklessBlockAllocator _allocators[COUNT];
  };
}

#endif