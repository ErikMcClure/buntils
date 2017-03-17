// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_BLOCK_H__
#define __BSS_ALLOC_BLOCK_H__

#include "bss_alloc.h"
#include "bss_util.h"

namespace bss_util {
  // Block Chunk Alloc
  struct FIXEDLIST_NODE
  {
    size_t size;
    FIXEDLIST_NODE* next;
  };

  class BSS_COMPILER_DLLEXPORT cBlockAllocVoid
  {
#ifdef BSS_COMPILER_MSC2010
    cBlockAllocVoid(const cBlockAllocVoid& copy) : _freelist(0), _root(0), _sz(0) { assert(false); }
#else
    cBlockAllocVoid(const cBlockAllocVoid& copy) = delete;
#endif
    cBlockAllocVoid& operator=(const cBlockAllocVoid& copy) BSS_DELETEFUNCOP
  public:
    inline cBlockAllocVoid(cBlockAllocVoid&& mov) : _root(mov._root), _freelist(mov._freelist), _sz(mov._sz) { mov._root=0; mov._freelist=0; }
    inline explicit cBlockAllocVoid(size_t sz, size_t init=8) : _root(0), _freelist(0), _sz(sz)
    {
      assert(sz>=sizeof(void*));
      _allocchunk(init*_sz);
    }
    inline ~cBlockAllocVoid()
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
      assert(num==1);
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return malloc(num*_sz);
#endif
      if(!_freelist)
      {
        _allocchunk(fbnext(_root->size / _sz)*_sz);
        assert(_freelist != 0);
        if(!_freelist)
          return nullptr;
      }

      void* ret=_freelist;
      _freelist=*((void**)_freelist);
      assert(_validpointer(ret));
      return ret;
    }
    inline void dealloc(void* p) noexcept
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      delete p; return;
#endif
      assert(_validpointer(p));
#ifdef BSS_DEBUG
      memset(p, 0xfd, _sz);
#endif
      *((void**)p)=_freelist;
      _freelist=p;
    }
    void Clear()
    {
      size_t nsize=0;
      FIXEDLIST_NODE* hold=_root;
      while((_root=hold)!=0)
      {
        nsize+=hold->size;
        hold=_root->next;
        free(_root);
      }
      _freelist=0; // There's this funny story about a time where I forgot to put this in here and then wondered why everything blew up.
      _allocchunk(nsize); // Note that nsize is in bytes
    }

  protected:
#ifdef BSS_DEBUG
    bool _validpointer(const void* p) const
    {
      const FIXEDLIST_NODE* hold=_root;
      while(hold)
      {
        if(p>=(hold+1) && p<(((uint8_t*)(hold+1))+hold->size))
          return ((((uint8_t*)p)-((uint8_t*)(hold+1)))%_sz)==0; //the pointer should be an exact multiple of _sz

        hold=hold->next;
      }
      return false;
    }
#endif
    inline void _allocchunk(size_t nsize) noexcept
    {
      FIXEDLIST_NODE* retval=reinterpret_cast<FIXEDLIST_NODE*>(malloc(sizeof(FIXEDLIST_NODE)+nsize));
      if(!retval) return;
      retval->next=_root;
      retval->size=nsize;
      //#pragma message(TODO "DEBUG REMOVE")
      //memset(retval->mem,0xff,retval->size);
      _initchunk(retval);
      _root=retval;
    }

    BSS_FORCEINLINE void _initchunk(const FIXEDLIST_NODE* chunk) noexcept
    {
      uint8_t* memend=((uint8_t*)(chunk+1))+chunk->size;
      for(uint8_t* memref=(((uint8_t*)(chunk+1))); memref<memend; memref+=_sz)
      {
        *((void**)(memref))=_freelist;
        _freelist=memref;
      }
    }

    FIXEDLIST_NODE* _root;
    void* _freelist;
    const size_t _sz;
  };

  template<class T>
  class BSS_COMPILER_DLLEXPORT cBlockAlloc : public cBlockAllocVoid
  {
    cBlockAlloc(const cBlockAlloc& copy) BSS_DELETEFUNC
      cBlockAlloc& operator=(const cBlockAlloc& copy) BSS_DELETEFUNCOP
  public:
    inline cBlockAlloc(cBlockAlloc&& mov) : cBlockAllocVoid(std::move(mov)) {}
    inline explicit cBlockAlloc(size_t init=8) : cBlockAllocVoid(sizeof(T), init) { static_assert((sizeof(T)>=sizeof(void*)), "T cannot be less than the size of a pointer"); }
    inline T* alloc(size_t num) noexcept { return (T*)cBlockAllocVoid::alloc(num); }
  };

  template<typename T>
  class BSS_COMPILER_DLLEXPORT BlockPolicy : protected cBlockAlloc<T>
  {
    BlockPolicy(const BlockPolicy& copy) BSS_DELETEFUNC
      BlockPolicy& operator=(const BlockPolicy& copy) BSS_DELETEFUNCOP
  public:
    typedef T* pointer;
    typedef T value_type;
    template<typename U>
    struct rebind { typedef BlockPolicy<U> other; };

    inline BlockPolicy(BlockPolicy&& mov) : cBlockAlloc<T>(std::move(mov)) {}
    inline BlockPolicy() {}
    inline ~BlockPolicy() {}


    inline pointer allocate(size_t cnt, const pointer = 0) noexcept { return cBlockAlloc<T>::alloc(cnt); }
    inline void deallocate(pointer p, size_t num = 0) noexcept { return cBlockAlloc<T>::dealloc(p); }
  };
}

#endif
