// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_GREEDY_BLOCK_H__
#define __BSS_ALLOC_GREEDY_BLOCK_H__

#include "Alloc.h"
#include "bss_util.h"

namespace bss {
  // A simplified single-threaded greedy allocator specifically designed for high-speed block allocations.
  template<class T>
  class BSS_COMPILER_DLLEXPORT GreedyBlockAlloc
  {
  public:
    // Block Chunk Alloc
    BSS_ALIGN(alignof(T)) union Node
    {
      struct {
        size_t size;
        Node* next;
      };
      char align[alignof(T)];
    };

    inline GreedyBlockAlloc(GreedyBlockAlloc&& mov) : _root(mov._root), _curpos(mov._curpos)
    {
      mov._root = 0;
      mov._curpos = 0;
    }
    inline explicit GreedyBlockAlloc(size_t init = 8) : _root(0), _curpos(0)
    {
      _allocChunk(init);
    }

    inline ~GreedyBlockAlloc()
    {
      Node* hold = _root;

      while((_root = hold))
      {
        hold = hold->next;
        ALIGNEDFREE(_root);
      }
    }
    inline T* Alloc(size_t sz = 1) noexcept
    {
      sz = AlignSize(sz * sizeof(T), alignof(T));
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      return ALIGNEDALLOC(sz * sizeof(T), alignof(T));
#endif
      size_t r = _curpos;
      _curpos += sz;
      if(_curpos > _root->size)
      {
        _allocChunk(fbnext(_curpos));
        r = 0;
        _curpos = sz;
      }
      T* p = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(_root + 1) + r);
      assert(!(reinterpret_cast<size_t>(p) % alignof(T)));
      return p;
    }
    void Dealloc(void* p) noexcept
    {
#ifdef BSS_DISABLE_CUSTOM_ALLOCATORS
      ALIGNEDFREE(p);
      return;
#endif
#ifdef BSS_DEBUG
      Node* cur = _root;
      bool found = false;

      while(cur)
      {
        if(p >= reinterpret_cast<uint8_t*>(cur + 1) && p < (reinterpret_cast<uint8_t*>(cur + 1) + cur->size)) { found = true; break; }
        cur = cur->next;
      }

      assert(found);
#endif
    }
    void Clear() noexcept
    {
      _curpos = 0;

      if(!_root->next)
        return;

      size_t nsize = 0;
      Node* hold;

      while(_root)
      {
        hold = _root->next;
        nsize += _root->size;
        ALIGNEDFREE(_root);
        _root = hold;
      }

      _allocChunk(nsize); //consolidates all memory into one chunk to try and take advantage of data locality
    }
  protected:
    BSS_FORCEINLINE void _allocChunk(size_t nsize) noexcept
    {
      nsize = AlignSize(nsize, sizeof(T));
      nsize = AlignSize(nsize, alignof(T));
      Node* retval = reinterpret_cast<Node*>(ALIGNEDALLOC(sizeof(Node) + nsize, alignof(T)));
      assert(retval != 0);
      retval->next = _root;
      retval->size = nsize;
      assert(_prepDEBUG(retval));
      _root = retval;
    }
#ifdef BSS_DEBUG
    BSS_FORCEINLINE static bool _prepDEBUG(Node* root) noexcept
    {
      if(!root) return false;
      memset(reinterpret_cast<uint8_t*>(root + 1), 0xfd, root->size);
      return true;
    }
#endif

    Node* _root;
    size_t _curpos;
  };
}

#endif