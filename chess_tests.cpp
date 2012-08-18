// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "bss_alloc_fixed_MT.h"

/*
  template<class T, size_t init=8>
  class BSS_COMPILER_DLLEXPORT cLocklessFixedAlloc
  {
  public:
    cLocklessFixedAlloc() : _freelist(0), _root(0), _spin(0)
    {
		  static_assert((sizeof(T)>=sizeof(void*)),"T cannot be less than the size of a pointer");
		  static_assert((sizeof(ABAPointer<void>)==(sizeof(void*)*2)),"ABAPointer isn't twice the size of a pointer!");
      _allocchunk(init*sizeof(T));
    }
    ~cLocklessFixedAlloc()
    {
      FIXEDCHUNKLIST* hold=_root;
      while(_root=hold)
      {
        hold=_root->next;
        free(_root->mem);
        free(_root);
      }
    }
	  inline T* BSS_FASTCALL alloc(size_t num)
    {
      assert(num<=1);
      void* ret;
      do
      {
        ret=_freelist;
        while(!ret)
        {
          if(!_spin && !atomic_xadd<unsigned int>(&_spin,_spin+1)) {
            // We must to have a check for _freelist here (not ret), after the atomic lock operation, to ensure it doesn't get reordered
            // and race conditions don't cause unnecessary resizing (it is possible for the code to check that _spin is nonzero, then for
            // _spin to get set to zero and _freelist to be set to nonzero by the other thread, before this thread then attempts the
            // atomic increment, which will then succeed even though _freelist is actually nonzero).
            if(!_freelist)
              _allocchunk(fbnext(_root->size/sizeof(T))*sizeof(T));
            atomic_xchg<unsigned int>(&_spin,0);
          }
          ret=_freelist;
        }
        assert(ret!=0);

      } while(!asmcas(&_freelist,*((void**)ret),ret)); //ABA problem

      //assert(_validpointer(ret));
      return (T*)ret;
    }
#ifdef BSS_DEBUG
    inline bool _validpointer(const void* p) const
    {
      const FIXEDCHUNKLIST* hold=_root;
      while(hold)
      {
        if(p>=hold->mem && p<(((unsigned char*)hold->mem)+hold->size))
          return ((((char*)p)-((char*)hold->mem))%sizeof(T))==0; //the pointer should be an exact multiple of sizeof(T)
        
        hold=hold->next;
      }
      return false;
    }
#endif
	  inline void BSS_FASTCALL dealloc(void* p)
    {
      assert(_validpointer(p));
#ifdef BSS_DEBUG
      memset(p,0xDEADBEEF,sizeof(T));
#endif
      *((void**)p)=_freelist;
      while(!asmcas(&_freelist,p,*((void**)p))) //ABA problem
        *((void**)p)=_freelist;
    }

  protected:
    inline void _allocchunk(size_t nsize)
    {
      FIXEDCHUNKLIST* retval=(FIXEDCHUNKLIST*)malloc(sizeof(FIXEDCHUNKLIST));
      retval->next=_root;
      retval->size=nsize;
      retval->mem=malloc(retval->size);
      _initchunk(*retval);
      _root=retval;
    }

    inline void _initchunk(const FIXEDCHUNKLIST& chunk)
    {
      void* hold=0;
      unsigned char* memend=((unsigned char*)chunk.mem)+chunk.size;
      for(unsigned char* memref=(unsigned char*)chunk.mem; memref<memend; memref+=sizeof(T))
      {
        *((void**)(memref))=hold;
        hold=memref;
      }
      *((void**)(chunk.mem))=_freelist; //We have to make sure we don't lose any existing values.
      while(!asmcas(&_freelist,hold,*((void**)(chunk.mem))))
        *((void**)(chunk.mem))=_freelist; // Keep assigning until it works. This is safe because no one has access to this chunk of memory until it succeeds.
    }

    FIXEDCHUNKLIST* _root;
    //ABAPointer<void> _freelist;
    void* _freelist;
    unsigned int _spin;
  };
*/

/*
extern "C" 
BSS_COMPILER_DLLEXPORT int ChessTestRun()
{
  cLocklessByteQueue qtest(512);
  bool expResult = true;

  cThread crapthread(&dorandomcrap);
  crapthread.Start(&qtest);
  
  std::pair<void*, unsigned int> hold;
  bool br=true;
  int num=-1;
  for(int i = 0; i < 2; ++i)
    while((hold=qtest.ReadNextChunk()).first!=0)
    {
      if(hold.second==MAX_ALLOC+1) { ++num; break; }
      else if(hold.second!=(((++num)<<3)%MAX_ALLOC))
        expResult=false;
    }
  
  crapthread.Join();
    while((hold=qtest.ReadNextChunk()).first!=0)
    {
      if(hold.second==MAX_ALLOC+1) { ++num; break; }
      else if(hold.second!=(((++num)<<3)%MAX_ALLOC))
        expResult=false;
    }
  //if(qtest.ReadNextChunk().first!=0) expResult=false; //This should have been done by now
  //if(expResult)
  //  printf("True\n");
  //else
  //  printf("**** False ****\n");

  return expResult ? 0 : -1;
}*/