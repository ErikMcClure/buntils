// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_BLOCK_H__
#define __BSS_ALLOC_BLOCK_H__

#include "Alloc.h"
#include "bss_util.h"

namespace bss {
  class BSS_COMPILER_DLLEXPORT BlockAlloc
  {
  public:
    // Block Chunk Alloc
    struct Node
    {
      size_t size;
      Node* next;
    };

    inline BlockAlloc(BlockAlloc&& mov) : _root(mov._root), _freelist(mov._freelist), _sz(mov._sz), _align(mov._align), _alignsize(mov._alignsize)
    {
      mov._root = 0;
      mov._freelist = 0;
    }
    inline explicit BlockAlloc(size_t sz, size_t init = 8, size_t align = 1) : _root(0), _freelist(0), _sz(AlignSize(sz, align)), _align(align),
      _alignsize(AlignSize(sizeof(Node), align))
    {
      assert(sz >= sizeof(void*));
      _allocChunk(init*_sz);
    }
    inline ~BlockAlloc()
    {
      Node* hold;
      while(_root != nullptr)
      {
        hold = _root;
        _root = _root->next;
        ALIGNEDFREE(hold);
      }
    }
    inline const Node* GetRoot() const { return _root; }
    BSS_FORCEINLINE size_t GetSize() const { return _sz; }
    template<class T>
    BSS_FORCEINLINE T* AllocT(size_t num) noexcept { return reinterpret_cast<T*>(Alloc(num * sizeof(T), alignof(T))); }
    BSS_FORCEINLINE void* Alloc() noexcept { return Alloc(_sz, _align); }
    inline void* Alloc(size_t bytes, size_t align = 1) noexcept
    {
      assert(bytes <= _sz);
      assert(align <= _align);
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return ALIGNEDALLOC(bytes, align);
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
    inline void Dealloc(void* p) noexcept
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      delete p; return;
#endif
#ifdef BSS_DEBUG
      assert(_validPointer(p));
      memset(p, 0xfd, _sz);
#endif
      *((void**)p) = _freelist;
      _freelist = p;
    }
    void Clear()
    {
      size_t nsize = 0;
      Node* hold = _root;

      while((_root = hold) != 0)
      {
        nsize += hold->size;
        hold = _root->next;
        ALIGNEDFREE(_root);
      }

      _freelist = 0; // There's this funny story about a time where I forgot to put this in here and then wondered why everything blew up.
      _allocChunk(nsize); // Note that nsize is in bytes
    }

    inline BlockAlloc& operator=(BlockAlloc&& mov)
    {
      if(this != &mov)
      {
        this->~BlockAlloc(); // Only safe because there's no inheritance and no virtual functions
        new (this) BlockAlloc(std::move(mov));
      }
      return *this;
    }

  protected:
#ifdef BSS_DEBUG
    bool _validPointer(const void* p) const
    {
      const Node* hold = _root;
      while(hold)
      {
        if(p >= (reinterpret_cast<const uint8_t*>(hold) + _alignsize) && p < (reinterpret_cast<const uint8_t*>(hold) + _alignsize + hold->size))
          return ((((uint8_t*)p) - (reinterpret_cast<const uint8_t*>(hold) + _alignsize)) % _sz) == 0; //the pointer should be an exact multiple of _sz

        hold = hold->next;
      }
      return false;
    }
#endif
    inline void _allocChunk(size_t nsize) noexcept
    {
      Node* retval = reinterpret_cast<Node*>(ALIGNEDALLOC(_alignsize + nsize, _align));
      if(!retval)
        return;

      retval->next = _root;
      retval->size = nsize;
      //#pragma message(TODO "DEBUG REMOVE")
      //memset(retval->mem,0xff,retval->size);
      _initChunk(retval);
      _root = retval;
    }

    BSS_FORCEINLINE void _initChunk(Node* chunk) noexcept
    {
      uint8_t* memend = reinterpret_cast<uint8_t*>(chunk) + _alignsize + chunk->size;

      for(uint8_t* memref = reinterpret_cast<uint8_t*>(chunk) + _alignsize; memref < memend; memref += _sz)
      {
        *((void**)(memref)) = _freelist;
        _freelist = memref;
      }
    }

    Node* _root;
    void* _freelist;
    const size_t _sz;
    const size_t _align;
    const size_t _alignsize; // sizeof(Node) expanded to have alignment _align
  };

  template<size_t SIZE, size_t ALIGN>
  struct BSS_COMPILER_DLLEXPORT BlockPolicySize : protected BlockAlloc
  {
    inline BlockPolicySize(BlockPolicySize&& mov) = default;
    inline explicit BlockPolicySize(size_t init = 8) : BlockAlloc(SIZE, init, ALIGN) { static_assert((SIZE >= sizeof(void*)), "SIZE cannot be less than the size of a pointer"); }
    template<class T>
    BSS_FORCEINLINE T* allocate(size_t cnt, const T* p = 0)
    {
      assert(!p);
      static_assert((sizeof(T) <= SIZE), "sizeof(T) must be less than SIZE");
      static_assert((alignof(T) <= ALIGN) && !(ALIGN % alignof(T)), "alignof(T) must be less than ALIGN and be a multiple of it");
      return BlockAlloc::AllocT<T>(cnt);
    }
    template<class T>
    BSS_FORCEINLINE void deallocate(T* p, size_t num = 0) noexcept { BlockAlloc::Dealloc(p); }
    BSS_FORCEINLINE void Clear() { BlockAlloc::Clear(); }
    BlockPolicySize& operator=(BlockPolicySize&& mov) = default;
  };

  template<class T>
  struct BSS_COMPILER_DLLEXPORT BlockPolicy : protected BlockAlloc
  {
    inline BlockPolicy(BlockPolicy&& mov) : BlockAlloc(std::move(mov)) {}
    inline explicit BlockPolicy(size_t init = 8) : BlockAlloc(sizeof(T), init, alignof(T)) { static_assert((sizeof(T) >= sizeof(void*)), "T cannot be less than the size of a pointer"); }
    BSS_FORCEINLINE T* allocate(size_t cnt, const T* p = 0, size_t old = 0) noexcept { assert(!p); return BlockAlloc::AllocT<T>(cnt); }
    BSS_FORCEINLINE void deallocate(T* p, size_t num = 0) noexcept { BlockAlloc::Dealloc(p); }
    BSS_FORCEINLINE void Clear() { BlockAlloc::Clear(); }
    BlockPolicy& operator=(BlockPolicy&& mov) { BlockAlloc::operator=(std::move(mov)); return *this; }
  };
}

#endif
