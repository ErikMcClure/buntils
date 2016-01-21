// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_GREEDY_H__
#define __BSS_ALLOC_GREEDY_H__

#include "bss_alloc.h"
#include "bss_util.h"

namespace bss_util {
  struct AFLISTITEM
  {
    AFLISTITEM* next;
    size_t size;
  };

  // Dynamic greedy allocator that can allocate any number of bytes
  class BSS_COMPILER_DLLEXPORT cGreedyAlloc
  {
    cGreedyAlloc(const cGreedyAlloc& copy) BSS_DELETEFUNC
      cGreedyAlloc& operator=(const cGreedyAlloc& copy) BSS_DELETEFUNCOP
  public:
    inline cGreedyAlloc(cGreedyAlloc&& mov) : _root(mov._root), _curpos(mov._curpos) { mov._root=0; mov._curpos=0; }
    inline explicit cGreedyAlloc(size_t init=64) : _root(0), _curpos(0)
    {
      _allocchunk(init);
    }
    inline ~cGreedyAlloc()
    {
      AFLISTITEM* hold=_root;
      while(_root=hold)
      {
        hold=_root->next;
        free(_root);
      }
    }
    template<typename T>
    inline T* BSS_FASTCALL allocT(size_t num)
    {
      return (T*)alloc(num*sizeof(T));
    }
    inline void* BSS_FASTCALL alloc(size_t _sz)
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return malloc(_sz);
#endif
      if((_curpos+_sz)>=_root->size) { _allocchunk(fbnext(bssmax(_root->size, _sz))); _curpos=0; }

      void* retval= ((char*)(_root+1))+_curpos;
      _curpos+=_sz;
      return retval;
    }
    void BSS_FASTCALL dealloc(void* p)
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      delete p; return;
#endif
#ifdef BSS_DEBUG
      AFLISTITEM* cur=_root;
      bool found=false;
      while(cur)
      {
        if(p>=(cur+1) && p<(((char*)(cur+1))+cur->size)) { found=true; break; }
        cur=cur->next;
      }
      assert(found);
      //memset(p,0xFEEEFEEE,sizeof(T)); //No way to know how big this is
#endif
    }
    void Clear()
    {
      _curpos=0;
      if(!_root->next) return;
      size_t nsize=0;
      AFLISTITEM* hold;
      while(_root)
      {
        hold=_root->next;
        nsize+=_root->size;
        free(_root);
        _root=hold;
      }
      _allocchunk(nsize); //consolidates all memory into one chunk to try and take advantage of data locality
    }
  protected:
    BSS_FORCEINLINE void _allocchunk(size_t nsize)
    {
      AFLISTITEM* retval=(AFLISTITEM*)malloc(sizeof(AFLISTITEM)+nsize);
      retval->next=_root;
      retval->size=nsize;
      _root=retval;
      assert(_prepDEBUG());
    }
    BSS_FORCEINLINE bool _prepDEBUG()
    {
      if(!_root) return false;
      memset(_root+1, 0xcdcdcdcd, _root->size);
      return true;
    }

    AFLISTITEM* _root;
    size_t _curpos;
  };

  template<typename T>
  class BSS_COMPILER_DLLEXPORT GreedyPolicy : protected cGreedyAlloc {
  public:
    typedef T* pointer;
    typedef T value_type;
    template<typename U>
    struct rebind { typedef GreedyPolicy<U> other; };

    inline GreedyPolicy() {}
    inline ~GreedyPolicy() {}

    inline pointer allocate(std::size_t cnt, const pointer = 0) { return cGreedyAlloc::allocT<T>(cnt); }
    inline void deallocate(pointer p, std::size_t num = 0) { }
    inline void BSS_FASTCALL clear() { cGreedyAlloc::Clear(); } //done for functor reasons, BSS_FASTCALL has no effect here
  };
}


#endif