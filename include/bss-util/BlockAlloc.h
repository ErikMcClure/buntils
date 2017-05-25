// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_BLOCK_H__
#define __BSS_ALLOC_BLOCK_H__

#include "Alloc.h"
#include "bss_util.h"

namespace bss {
  class BSS_COMPILER_DLLEXPORT BlockAllocVoid
  {
#ifdef BSS_COMPILER_MSC2010
    BlockAllocVoid(const BlockAllocVoid& copy) : _freelist(0), _root(0), _sz(0) { assert(false); }
#else
    BlockAllocVoid(const BlockAllocVoid& copy) = delete;
#endif
    BlockAllocVoid& operator=(const BlockAllocVoid& copy) BSS_DELETEFUNCOP
  public:
    // Block Chunk Alloc
    struct FIXEDLIST_NODE
    {
      size_t size;
      FIXEDLIST_NODE* next;
    };

    inline BlockAllocVoid(BlockAllocVoid&& mov) : _root(mov._root), _freelist(mov._freelist), _sz(mov._sz) { mov._root = 0; mov._freelist = 0; }
    inline explicit BlockAllocVoid(size_t sz, size_t init = 8) : _root(0), _freelist(0), _sz(sz)
    {
      assert(sz >= sizeof(void*));
      _allocChunk(init*_sz);
    }
    inline ~BlockAllocVoid()
    {
      FIXEDLIST_NODE* hold;
      while(_root != nullptr)
      {
        hold = _root;
        _root = _root->next;
        free(hold);
      }
    }
    inline void* alloc(size_t num) noexcept
    {
      assert(num == 1);
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return malloc(num*_sz);
#endif
      if(!_freelist)
      {
        _allocChunk(fbnext(_root->size / _sz)*_sz);
        assert(_freelist != 0);
        if(!_freelist)
          return nullptr;
      }

      void* ret = _freelist;
      _freelist = *((void**)_freelist);
      assert(_validPointer(ret));
      return ret;
    }
    inline void dealloc(void* p) noexcept
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      delete p; return;
#endif
      assert(_validPointer(p));
#ifdef BSS_DEBUG
      memset(p, 0xfd, _sz);
#endif
      *((void**)p) = _freelist;
      _freelist = p;
    }
    void Clear()
    {
      size_t nsize = 0;
      FIXEDLIST_NODE* hold = _root;
      while((_root = hold) != 0)
      {
        nsize += hold->size;
        hold = _root->next;
        free(_root);
      }
      _freelist = 0; // There's this funny story about a time where I forgot to put this in here and then wondered why everything blew up.
      _allocChunk(nsize); // Note that nsize is in bytes
    }

  protected:
#ifdef BSS_DEBUG
    bool _validPointer(const void* p) const
    {
      const FIXEDLIST_NODE* hold = _root;
      while(hold)
      {
        if(p >= (hold + 1) && p < (((uint8_t*)(hold + 1)) + hold->size))
          return ((((uint8_t*)p) - ((uint8_t*)(hold + 1))) % _sz) == 0; //the pointer should be an exact multiple of _sz

        hold = hold->next;
      }
      return false;
    }
#endif
    inline void _allocChunk(size_t nsize) noexcept
    {
      FIXEDLIST_NODE* retval = reinterpret_cast<FIXEDLIST_NODE*>(malloc(sizeof(FIXEDLIST_NODE) + nsize));
      if(!retval) return;
      retval->next = _root;
      retval->size = nsize;
      //#pragma message(TODO "DEBUG REMOVE")
      //memset(retval->mem,0xff,retval->size);
      _initChunk(retval);
      _root = retval;
    }

    BSS_FORCEINLINE void _initChunk(const FIXEDLIST_NODE* chunk) noexcept
    {
      uint8_t* memend = ((uint8_t*)(chunk + 1)) + chunk->size;
      for(uint8_t* memref = (((uint8_t*)(chunk + 1))); memref < memend; memref += _sz)
      {
        *((void**)(memref)) = _freelist;
        _freelist = memref;
      }
    }

    FIXEDLIST_NODE* _root;
    void* _freelist;
    const size_t _sz;
  };

  template<class T>
  class BSS_COMPILER_DLLEXPORT BlockAlloc : public BlockAllocVoid
  {
    BlockAlloc(const BlockAlloc& copy) BSS_DELETEFUNC
      BlockAlloc& operator=(const BlockAlloc& copy) BSS_DELETEFUNCOP
  public:
    inline BlockAlloc(BlockAlloc&& mov) : BlockAllocVoid(std::move(mov)) {}
    inline explicit BlockAlloc(size_t init = 8) : BlockAllocVoid(sizeof(T), init) { static_assert((sizeof(T) >= sizeof(void*)), "T cannot be less than the size of a pointer"); }
    inline T* alloc(size_t num) noexcept { return (T*)BlockAllocVoid::alloc(num); }
  };

  template<typename T>
  class BSS_COMPILER_DLLEXPORT BlockPolicy : protected BlockAlloc<T>
  {
    BlockPolicy(const BlockPolicy& copy) BSS_DELETEFUNC
      BlockPolicy& operator=(const BlockPolicy& copy) BSS_DELETEFUNCOP
  public:
    typedef T* pointer;
    typedef T value_type;
    template<typename U>
    struct rebind { typedef BlockPolicy<U> other; };

    inline BlockPolicy(BlockPolicy&& mov) : BlockAlloc<T>(std::move(mov)) {}
    inline BlockPolicy() {}
    inline ~BlockPolicy() {}


    inline pointer allocate(size_t cnt, const pointer = 0) noexcept { return BlockAlloc<T>::alloc(cnt); }
    inline void deallocate(pointer p, size_t num = 0) noexcept { return BlockAlloc<T>::dealloc(p); }
  };
}

#endif
