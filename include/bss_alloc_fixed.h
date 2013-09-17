// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_FIXED_H__
#define __BSS_ALLOC_FIXED_H__

#include "bss_alloc.h"
#include "bss_util.h"

#define DECL_FIXEDPOLICY(T) bss_util::Allocator<T,bss_util::FixedPolicy<T>>

namespace bss_util {
  // Fixed Chunk Alloc
  struct FIXEDLIST_NODE
  {
    size_t size;
    FIXEDLIST_NODE* next;
  };

  class BSS_COMPILER_DLLEXPORT cFixedAllocVoid
  {
  public:
    inline cFixedAllocVoid(size_t sz, size_t init=8) : _freelist(0), _root(0), _sz(sz)
    {
      assert(sz>=sizeof(void*));
      _allocchunk(init*_sz);
    }
    inline ~cFixedAllocVoid()
    {
      FIXEDLIST_NODE* hold=_root;
      while(_root=hold)
      {
        hold=_root->next;
        free(_root);
      }
    }
	  inline void* BSS_FASTCALL alloc(size_t num)
    {
      assert(num==1);
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return malloc(num*_sz);
#endif
      if(!_freelist) _allocchunk(fbnext(_root->size/_sz)*_sz);
      assert(_freelist!=0);

      void* ret=_freelist;
      _freelist=*((void**)_freelist);
      assert(_validpointer(ret));
      return ret;
    }
	  inline void BSS_FASTCALL dealloc(void* p)
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      delete p; return;
#endif
      assert(_validpointer(p));
#ifdef BSS_DEBUG
      memset(p,0xDEADBEEF,_sz);
#endif
      *((void**)p)=_freelist;
      _freelist=p;
    }
    void Clear()
    {
      size_t nsize=0;
      FIXEDLIST_NODE* hold=_root;
      while(_root=hold)
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
        if(p>=(hold+1) && p<(((unsigned char*)(hold+1))+hold->size))
          return ((((unsigned char*)p)-((unsigned char*)(hold+1)))%_sz)==0; //the pointer should be an exact multiple of _sz
        
        hold=hold->next;
      }
      return false;
    }
#endif
    inline void BSS_FASTCALL _allocchunk(size_t nsize)
    {
      FIXEDLIST_NODE* retval=(FIXEDLIST_NODE*)malloc(sizeof(FIXEDLIST_NODE)+nsize);
      retval->next=_root;
      retval->size=nsize;
//#pragma message(TODO "DEBUG REMOVE")
      //memset(retval->mem,0xff,retval->size);
      _initchunk(retval);
      _root=retval;
    }

    BSS_FORCEINLINE void BSS_FASTCALL _initchunk(const FIXEDLIST_NODE* chunk)
    {
      unsigned char* memend=((unsigned char*)(chunk+1))+chunk->size;
      for(unsigned char* memref=(((unsigned char*)(chunk+1))); memref<memend; memref+=_sz)
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
  class BSS_COMPILER_DLLEXPORT cFixedAlloc : public cFixedAllocVoid
  {
  public:
    inline explicit cFixedAlloc(size_t init=8) : cFixedAllocVoid(sizeof(T),init) { static_assert((sizeof(T)>=sizeof(void*)),"T cannot be less than the size of a pointer"); }
    inline T* BSS_FASTCALL alloc(size_t num) { return (T*)cFixedAllocVoid::alloc(num); }
  };
  
	template<typename T>
  class BSS_COMPILER_DLLEXPORT FixedPolicy : public AllocPolicySize<T>, protected cFixedAlloc<T> {
	public:
    typedef typename AllocPolicySize<T>::pointer pointer;
    template<typename U>
    struct rebind { typedef FixedPolicy<U> other; };

    inline explicit FixedPolicy() {}
    inline ~FixedPolicy() {}
    inline explicit FixedPolicy(FixedPolicy const&) {}
    template <typename U>
    inline explicit FixedPolicy(FixedPolicy<U> const&) {}

    inline pointer allocate(size_t cnt, typename std::allocator<void>::const_pointer = 0) {
        return cFixedAlloc<T>::alloc(cnt);
    }
    inline void deallocate(pointer p, size_t num = 0) { 
      return cFixedAlloc<T>::dealloc(p);
    }
	};
}

#endif
