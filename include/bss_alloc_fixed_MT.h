// Copyright ©2012 Black Sphere Studios
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
    explicit cLocklessFixedAlloc(size_t init=8) : _root(0), _spin(0)
    {
      //contention=0;
      _freelist.p=0;
      _freelist.tag=0;
		  static_assert((sizeof(T)>=sizeof(void*)),"T cannot be less than the size of a pointer");
		  static_assert((sizeof(bss_PTag<void>)==(sizeof(void*)*2)),"ABAPointer isn't twice the size of a pointer!");
      _allocchunk(init*sizeof(T));
    }
    ~cLocklessFixedAlloc()
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
        ret.p=_freelist.p;
        ret.tag=_freelist.tag;
        if(!ret.p)
        {
          while(atomic_xadd<unsigned int>(&_spin,1)!=0) { // Attempt to acquire lock.
            atomic_xadd<unsigned int>(&_spin,-1); // Remove our attempt at hitting the lock.
            while(_spin!=0); // Wait until spin is 0 again.
            //atomic_xadd<size_t>(&contention); //DEBUG
          }
          if(!_freelist.p)
            _allocchunk(fbnext(_root->size/sizeof(T))*sizeof(T));
          atomic_xadd<unsigned int>(&_spin,-1);
          continue; // _freelist.p can be NULL here due to a race condition where _freelist becomes non-null, and then another thread uses the entire freelist, followed by this thread executing
        }
        assert(ret.p!=0);
        nval.p = *((void**)ret.p);
        nval.tag= ret.tag+1;
        if(asmcas<bss_PTag<void>>(&_freelist,nval,ret)) break; // Have to do this here so continue; works the way we want it to.
      }

      //assert(_validpointer(ret));
      return (T*)ret.p;
    }
	  inline void BSS_FASTCALL dealloc(void* p)
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      delete p; return;
#endif
      assert(_validpointer(p));
#ifdef BSS_DEBUG
      memset(p,0xDEADBEEF,sizeof(T));
#endif
      bss_PTag<void> prev={_freelist.p,_freelist.tag};
      *((void**)p)=(void*)prev.p;
      bss_PTag<void> nval = { p,prev.tag+1 };
      //size_t count=0;
      while(!asmcas<bss_PTag<void>>(&_freelist,nval,prev)) {
        prev.p=_freelist.p;
        prev.tag=_freelist.tag;
        *((void**)p)=(void*)prev.p;
        nval.tag=prev.tag+1;
        //atomic_xadd<size_t>(&contention); //DEBUG
        //++count;
      }
      //if(count>contention)
      //  while(atomic_xchg(&contention,count)>contention);

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
    inline void _allocchunk(size_t nsize)
    {
      FIXEDLIST_NODE* retval=(FIXEDLIST_NODE*)malloc(sizeof(FIXEDLIST_NODE)+nsize);
      retval->next=_root;
      retval->size=nsize;
      _initchunk(retval);
      _root=retval;
    }

    inline void _initchunk(const FIXEDLIST_NODE* chunk)
    {
      void* hold=0;
      unsigned char* memend=((unsigned char*)(chunk+1))+chunk->size;
      for(unsigned char* memref=(unsigned char*)(chunk+1); memref<memend; memref+=sizeof(T))
      {
        *((void**)(memref))=hold;
        hold=memref;
      }
      bss_PTag<void> prev = { _freelist.p,_freelist.tag };
      *((void**)(chunk+1))=(void*)prev.p;
      bss_PTag<void> nval = { hold,prev.tag+1 };
      while(!asmcas<bss_PTag<void>>(&_freelist,nval,prev)) {
        prev.p=_freelist.p;
        prev.tag=_freelist.tag;
        *((void**)(chunk+1))=(void*)prev.p;
        nval.tag=prev.tag+1;
        //atomic_xadd<size_t>(&contention); //DEBUG
      }
      //*((void**)(chunk+1))=(void*)_freelist; //We have to make sure we don't lose any existing values.
      //while(!asmcas<volatile bss_PTag<void>>(&_freelist,hold,*((void**)(chunk+1))))
      //  *((void**)(chunk+1))=(void*)_freelist; // Keep assigning until it works. This is safe because no one has access to this chunk of memory until it succeeds.
    }

    FIXEDLIST_NODE* _root;
    BSS_ALIGN(16) volatile bss_PTag<void> _freelist;
    //volatile void* _freelist;
    BSS_ALIGN(4) volatile unsigned int _spin;
  };
}

#endif