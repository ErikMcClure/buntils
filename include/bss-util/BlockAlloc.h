// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_BLOCK_H__
#define __BSS_ALLOC_BLOCK_H__

#include "Alloc.h"
#include "bss_util.h"

namespace bss {
  class BSS_COMPILER_DLLEXPORT BlockAllocVoid
  {
  public:
    // Block Chunk Alloc
    struct Node
    {
      size_t size;
      Node* next;
    };

    inline BlockAllocVoid(BlockAllocVoid&& mov) : _root(mov._root), _freelist(mov._freelist), _sz(mov._sz), _align(mov._align), _alignsize(mov._alignsize)
    {
      mov._root = 0;
      mov._freelist = 0;
    }
    inline explicit BlockAllocVoid(size_t sz, size_t init = 8, size_t align = 1) : _root(0), _freelist(0), _sz(AlignSize(sz, align)), _align(align),
      _alignsize(AlignSize(sizeof(Node), align))
    {
      assert(sz >= sizeof(void*));
      _allocChunk(init*_sz);
    }
    inline ~BlockAllocVoid()
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
    BSS_FORCEINLINE T* AllocT(size_t num) noexcept { return static_cast<T*>(Alloc(num * sizeof(T), alignof(T))); }
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

    inline BlockAllocVoid& operator=(BlockAllocVoid&& mov)
    {
      if(this != &mov)
      {
        this->~BlockAllocVoid(); // Only safe because there's no inheritance and no virtual functions
        new (this) BlockAllocVoid(std::move(mov));
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
  class BSS_COMPILER_DLLEXPORT BlockAllocSize : protected BlockAllocVoid
  {
  public:
    inline BlockAllocSize(BlockAllocSize&& mov) : BlockAllocVoid(std::move(mov)) {}
    inline explicit BlockAllocSize(size_t init = 8) : BlockAllocVoid(SIZE, init, ALIGN) { static_assert((SIZE >= sizeof(void*)), "SIZE cannot be less than the size of a pointer"); }
    template<class T>
    BSS_FORCEINLINE T* Alloc(size_t num = 1)
    {
      static_assert((sizeof(T) <= SIZE), "sizeof(T) must be less than SIZE");
      static_assert((alignof(T) <= ALIGN) && !(ALIGN % alignof(T)), "alignof(T) must be less than ALIGN and be a multiple of it");
      return BlockAllocVoid::AllocT<T>(num);
    }
    template<class T>
    BSS_FORCEINLINE void Dealloc(T* p) noexcept { static_assert((sizeof(T) <= SIZE), "sizeof(T) must be less than SIZE"); BlockAllocVoid::Dealloc(p); }
    BSS_FORCEINLINE void Clear() { BlockAllocVoid::Clear(); }
    BlockAllocSize& operator=(BlockAllocSize&& mov) { BlockAllocVoid::operator=(std::move(mov)); return *this; }
  };

  template<class T>
  class BSS_COMPILER_DLLEXPORT BlockAlloc : protected BlockAllocVoid
  {
  public:
    inline BlockAlloc(BlockAlloc&& mov) : BlockAllocVoid(std::move(mov)) {}
    inline explicit BlockAlloc(size_t init = 8) : BlockAllocVoid(sizeof(T), init, alignof(T)) { static_assert((sizeof(T) >= sizeof(void*)), "T cannot be less than the size of a pointer"); }
    BSS_FORCEINLINE T* Alloc(size_t num = 1) noexcept { return BlockAllocVoid::AllocT<T>(num); }
    BSS_FORCEINLINE void Dealloc(T* p) noexcept { BlockAllocVoid::Dealloc(p); }
    BSS_FORCEINLINE void Clear() { BlockAllocVoid::Clear(); }
    BlockAlloc& operator=(BlockAlloc&& mov) { BlockAllocVoid::operator=(std::move(mov)); return *this; }
  };

  template<typename T>
  class BSS_COMPILER_DLLEXPORT BlockPolicy : protected BlockAlloc<T>
  {
  public:
    typedef T* pointer;
    typedef T value_type;
    template<typename U>
    struct rebind { typedef BlockPolicy<U> other; };

    inline BlockPolicy(BlockPolicy&& mov) : BlockAlloc<T>(std::move(mov)) {}
    inline BlockPolicy() {}
    inline ~BlockPolicy() {}

    BlockPolicy& operator=(BlockPolicy&& mov) { BlockAlloc<T>::operator=(std::move(mov)); return *this; }

    BSS_FORCEINLINE pointer allocate(size_t cnt, const pointer = 0) noexcept { return BlockAlloc<T>::Alloc(cnt); }
    BSS_FORCEINLINE void deallocate(pointer p, size_t num = 0) noexcept { return BlockAlloc<T>::Dealloc(p); }
  };
}

#endif
