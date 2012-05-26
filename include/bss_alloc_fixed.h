// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_FIXED_H__
#define __BSS_ALLOC_FIXED_H__

#include "bss_alloc.h"
#include "bss_util.h"

#define DECL_FIXEDPOLICY(T) bss_util::Allocator<T,bss_util::FixedChunkPolicy<T>>

namespace bss_util {
  struct fsaBlock
  {
	  fsaBlock* next;
  };
  struct fsaChunk
  {
	  __int32 blockSize;
	  fsaBlock* blocks;
  };

  /* This is a fixed size block allocator adapted from the box2D heap allocator */
  template<class T, __int32 chunkIncrement=128, __int32 __chunkSize=4096>
  class BSS_COMPILER_DLLEXPORT cFixedSizeAllocator
  {
  public:
	  inline cFixedSizeAllocator()
    {
	    _chunkspace = chunkIncrement;
	    _chunknum = 0;
	    _chunks = (fsaChunk*)malloc(_chunkspace * sizeof(fsaChunk));
    	
	    memset(_chunks, 0, _chunkspace * sizeof(fsaChunk));
	   _freelist=0;
    }
	  inline ~cFixedSizeAllocator()
    {
	    for (__int32 i = 0; i < _chunknum; ++i)
	    {
		    free(_chunks[i].blocks);
	    }

	    free(_chunks);
    }

	  inline T* BSS_FASTCALL alloc(__int32 num)
    {
	    if (num == 0)
		    return NULL;
      else if(num > 1)
        return (T*)malloc(num*sizeof(T));

	    if (_freelist)
	    {
		    fsaBlock* block = _freelist;
		    _freelist = block->next;
		    return (T*)block;
	    }
	    else
	    {
		    if (_chunknum == _chunkspace)
		    {
			    fsaChunk* oldChunks = _chunks;
			    _chunkspace += chunkIncrement;
			    _chunks = (fsaChunk*)malloc(_chunkspace * sizeof(fsaChunk));
			    memcpy(_chunks, oldChunks, _chunknum * sizeof(fsaChunk));
			    memset(_chunks + _chunknum, 0, chunkIncrement * sizeof(fsaChunk));
			    free(oldChunks);
		    }

		    fsaChunk* chunk = _chunks + _chunknum;
		    chunk->blocks = (fsaBlock*)malloc(__chunkSize);
    #if defined(_DEBUG)
		    memset(chunk->blocks, 0xcd, __chunkSize);
    #endif
		    const __int32 blockSize = sizeof(T)*32;
		    chunk->blockSize = blockSize;
		    __int32 blockCount = __chunkSize / blockSize;
		    assert(blockCount * blockSize <= __chunkSize);
		    for (__int32 i = 0; i < blockCount - 1; ++i)
		    {
			    fsaBlock* block = (fsaBlock*)((__int8*)chunk->blocks + blockSize * i);
			    fsaBlock* next = (fsaBlock*)((__int8*)chunk->blocks + blockSize * (i + 1));
			    block->next = next;
		    }
		    fsaBlock* last = (fsaBlock*)((__int8*)chunk->blocks + blockSize * (blockCount - 1));
		    last->next = NULL;

		    _freelist = chunk->blocks->next;
		    ++_chunknum;

		    return (T*)chunk->blocks;
	    }
    }
	  inline void BSS_FASTCALL dealloc(void* p)
    {
      assert(p!=0);

    #ifdef _DEBUG
	    // Verify the memory address and size is valid.
	    const __int32 blockSize = sizeof(T)*32;
	    bool found = false;
	    __int32 gap = (__int32)((__int8*)&_chunks->blocks - (__int8*)_chunks);
	    for (__int32 i = 0; i < _chunknum; ++i)
	    {
		    fsaChunk* chunk = _chunks + i;
		    if (chunk->blockSize != blockSize)
		    {
			    assert(	(__int8*)p + blockSize <= (__int8*)chunk->blocks ||
						    (__int8*)chunk->blocks + __chunkSize + gap <= (__int8*)p);
		    }
		    else
		    {
			    if ((__int8*)chunk->blocks <= (__int8*)p && (__int8*)p + blockSize <= (__int8*)chunk->blocks + __chunkSize)
			    {
				    found = true;
			    }
		    }
	    }

	    assert(found);
	    memset(p, 0xfd, blockSize);
    #endif

	    fsaBlock* block = (fsaBlock*)p;
	    block->next = _freelist;
	    _freelist = block;
    }

	  inline void Clear()
    {
	    for (int32 i = 0; i < _chunknum; ++i)
	    {
		    free(_chunks[i].blocks);
	    }

	    _chunknum = 0;
	    memset(_chunks, 0, _chunkspace * sizeof(fsaChunk));
	    _freelist=0;
    }

  protected:
	  fsaChunk* _chunks;
	  __int32 _chunknum;
	  __int32 _chunkspace;
    fsaBlock* _freelist;
  };

	template<typename T>
  class BSS_COMPILER_DLLEXPORT FixedSizeAllocPolicy : public AllocPolicySize<T>, protected cFixedSizeAllocator<T> {
	public:
    template<typename U>
    struct rebind { typedef FixedSizeAllocPolicy<U> other; };

    inline explicit FixedSizeAllocPolicy() {}
    inline ~FixedSizeAllocPolicy() {}
    inline explicit FixedSizeAllocPolicy(FixedSizeAllocPolicy const&) {}
    template <typename U>
    inline explicit FixedSizeAllocPolicy(FixedSizeAllocPolicy<U> const&) {}

    inline pointer allocate(std::size_t cnt, 
      typename std::allocator<void>::const_pointer = 0) {
        //return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T))); 
        return alloc(cnt);
    }
    //inline pointer allocatebytes(std::size_t bytes) { //We need a direct bytes allocation count because in some cases operator[] will request 4 additional bytes on top of the array
    //    //return reinterpret_cast<pointer>(::operator new(bytes)); 
    //    return allocbytes(bytes);
    //}
    inline void deallocate(pointer p, std::size_t num = 0) { 
      return dealloc(p);
      //::operator delete(p);
      //if(num <= 1) dealloc(p);
      //else dealloc_multi(p);
    }
	};

  /* Fixed Chunk Alloc */
  struct FIXEDCHUNKLIST
  {
    void* mem;
    size_t size;
    FIXEDCHUNKLIST* next;
  };

  template<class T, size_t init=8>
  class BSS_COMPILER_DLLEXPORT cFixedChunkAlloc
  {
  public:
    cFixedChunkAlloc() : _freelist(0), _root(0)
    {
		  static_assert((sizeof(T)>=sizeof(void*)),"T cannot be less than the size of a pointer");
      _allocchunk(init*sizeof(T));
    }
    ~cFixedChunkAlloc()
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
      if(!_freelist) _allocchunk(fbnext(_root->size/sizeof(T))*sizeof(T));
      assert(_freelist!=0);

      void* ret=_freelist;
      _freelist=*((void**)_freelist);
      //assert(_validpointer(ret));
      return (T*)ret;
    }
#if defined(DEBUG) || defined(_DEBUG)
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
#if defined(DEBUG) || defined(_DEBUG)
      memset(p,0xDEADBEEF,sizeof(T));
#endif
      *((void**)p)=_freelist;
      _freelist=p;
    }

  protected:
    inline void _allocchunk(size_t nsize)
    {
      FIXEDCHUNKLIST* retval=(FIXEDCHUNKLIST*)malloc(sizeof(FIXEDCHUNKLIST));
      retval->next=_root;
      retval->size=nsize;
      retval->mem=malloc(retval->size);
//#pragma message(TODO "DEBUG REMOVE")
      //memset(retval->mem,0xff,retval->size);
      _initchunk(*retval);
      _root=retval;
    }

    inline void _initchunk(const FIXEDCHUNKLIST& chunk)
    {
      unsigned char* memend=((unsigned char*)chunk.mem)+chunk.size;
      for(unsigned char* memref=(unsigned char*)chunk.mem; memref<memend; memref+=sizeof(T))
      {
        *((void**)(memref))=_freelist;
        _freelist=memref;
      }
    }

    FIXEDCHUNKLIST* _root;
    void* _freelist;
  };
  
	template<typename T>
  class BSS_COMPILER_DLLEXPORT FixedChunkPolicy : public AllocPolicySize<T>, protected cFixedChunkAlloc<T> {
	public:
    template<typename U>
    struct rebind { typedef FixedChunkPolicy<U> other; };

    inline explicit FixedChunkPolicy() {}
    inline ~FixedChunkPolicy() {}
    inline explicit FixedChunkPolicy(FixedChunkPolicy const&) {}
    template <typename U>
    inline explicit FixedChunkPolicy(FixedChunkPolicy<U> const&) {}

    inline pointer allocate(std::size_t cnt, 
      typename std::allocator<void>::const_pointer = 0) {
        return alloc(cnt);
    }
    inline void deallocate(pointer p, std::size_t num = 0) { 
      return dealloc(p);
    }
	};

  BSSBUILD_STATIC_POLICY(StaticFixedChunk,FixedChunkPolicy);

  /* Generic static allocator for a class. T is the type of the class implementing this allocator. Every single subclass has to implement this if its to be taken advantage of */
  template<typename T>
  class BSS_COMPILER_DLLEXPORT cClassAllocator //: protected cSubClassAlloc<sizeof(T)>
  {
  public:
    //cClassAllocator(size_t indexID=0) {}
    
    void *operator new(size_t size) { assert(size==sizeof(T)); return (void*)_alloc.allocate(1); }
    void operator delete(void *p) { _alloc.deallocate((T*)p,1); }
    //void *operator new[](size_t size) { assert(!(size%sizeof(T))); return (void*)_alloc.allocate(size/sizeof(T)); }
    //void *operator new[](size_t size) { return (void*)_alloc.allocatebytes(size); }
    //void *operator new[](size_t size) { return malloc(size); }
    //void operator delete[](void *p) { _alloc.deallocate((T*)p,2); }

  protected:
    static Allocator<T, FixedChunkPolicy<T>> _alloc;
  };

  template<typename T>
  Allocator<T, FixedChunkPolicy<T>> cClassAllocator<T>::_alloc = Allocator<T, FixedChunkPolicy<T>>();
}

#endif