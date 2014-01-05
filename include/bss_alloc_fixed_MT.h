
// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_FIXED_LOCKLESS_H__
#define __BSS_ALLOC_FIXED_LOCKLESS_H__

#include "bss_alloc_fixed.h"
#include "lockless.h"

namespace bss_util {
  /* Multi-producer multi-consumer lockless fixed size allocator */
  template<class T>
  class BSS_COMPILER_DLLEXPORT cLocklessFixedAlloc
  {
  public:
    inline explicit cLocklessFixedAlloc(size_t init=8) : _root(0), _spin(0)
    {
      //contention=0;
      _freelist.p=0;
      _freelist.tag=0;
		  static_assert((sizeof(T)>=sizeof(void*)),"T cannot be less than the size of a pointer");
		  static_assert((sizeof(bss_PTag<void>)==(sizeof(void*)*2)),"ABAPointer isn't twice the size of a pointer!");
      _allocchunk(init*sizeof(T));
    }
    inline ~cLocklessFixedAlloc()
    {
      FIXEDLIST_NODE* hold=_root;
      while(_root=hold)
      {
        hold=_root->next;
        free(_root);
      }
    }
	  inline T* BSS_FASTCALL alloc(size_t num)
    {
      assert(num==1);
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return (T*)malloc(num*sizeof(T));
#endif
      bss_PTag<void> ret;
      bss_PTag<void> nval;

      for(;;)
      {
        ret = (bss_PTag<void>&)_freelist;
        if(!ret.p)
        {
          if(atomic_xchg<size_t>(&_spin, 1)==1) { // If we get the lock, do a new allocation
            if(!_freelist.p) // Check this due to race condition
              _allocchunk(fbnext(_root->size/sizeof(T))*sizeof(T));
            atomic_xchg<size_t>(&_spin, 0);
          }
          continue;
        }
        nval.p = *((void**)ret.p);
        nval.tag = ret.tag+1;
        if(asmcas<bss_PTag<void>>(&_freelist, nval, ret)) break;
      }

      //assert(_validpointer(ret));
      return (T*)ret.p;
    }
	  inline void BSS_FASTCALL dealloc(void* p)
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      free(p); return;
#endif
      assert(_validpointer(p));
#ifdef BSS_DEBUG
      memset(p,0xDEADBEEF,sizeof(T));
#endif
      _setfreelist(p, p);
      //*((void**)p)=(void*)_freelist;
      //while(!asmcas<volatile void*>(&_freelist,p,*((void**)p))) //ABA problem
      //  *((void**)p)=(void*)_freelist;
    }
    
    //size_t contention;

  protected:
#ifdef BSS_DEBUG
    inline bool _validpointer(const void* p) const
    {
      const FIXEDLIST_NODE* hold=_root;
      while(hold)
      {
        if(p>=(hold+1) && p<(((unsigned char*)(hold+1))+hold->size))
          return ((((unsigned char*)p)-((unsigned char*)(hold+1)))%sizeof(T))==0; //the pointer should be an exact multiple of sizeof(T)
        
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
      _root = retval; // There's a potential race condition on DEBUG mode only where failing to set this first would allow a thread to allocate a new pointer and then delete it before _root got changed, which would then be mistaken for an invalid pointer
      _initchunk(retval);
    }

    inline void BSS_FASTCALL _initchunk(const FIXEDLIST_NODE* chunk)
    {
      void* hold=0;
      unsigned char* memend=((unsigned char*)(chunk+1))+chunk->size;
      for(unsigned char* memref=(unsigned char*)(chunk+1); memref<memend; memref+=sizeof(T))
      {
        *((void**)(memref))=hold;
        hold=memref;
      }
      _setfreelist(hold, (void*)(chunk+1)); // The target here is different because normally, the first block (at chunk+1) would point to whatever _freelist used to be. However, since we are lockless, _freelist could not be 0 at the time we insert this, so we have to essentially go backwards and set the first one to whatever freelist is NOW, before setting freelist to the one on the end.
    }

    inline void BSS_FASTCALL _setfreelist(void* p, void* target) {
      bss_PTag<void> prev;
      bss_PTag<void> nval = { p, 0 };
      do
      {
        prev = (bss_PTag<void>&)_freelist;
        nval.tag = prev.tag+1;
        *((void**)(target)) = (void*)prev.p;
        //atomic_xadd<size_t>(&contention); //DEBUG
      } while(!asmcas<bss_PTag<void>>(&_freelist, nval, prev));
    }

    FIXEDLIST_NODE* _root;
    BSS_ALIGN(16) volatile bss_PTag<void> _freelist;
    BSS_ALIGN(4) volatile size_t _spin;
  };
}

#endif