// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LOCKLESS_QUEUE_H__
#define __C_LOCKLESS_QUEUE_H__

#include "bss_alloc_fixed_MT.h"

namespace bss_util {
  template<typename T>
  struct cLQ_QNode {
    inline cLQ_QNode() : next(0) {} // This lets item have a proper default constructor
    template<typename U>
    inline cLQ_QNode(U && item) : next(0), item(std::forward<U>(item)) {}
    cLQ_QNode* next;
    T item;
  };

  // Internal class used to toggle whether or not the queue stores and updates a length value.
  template<typename ST_>
  struct i_LocklessQueue_Length
  { 
    inline i_LocklessQueue_Length() : _length(0) {}
    inline ST_ Length() const { return _length; }

  protected:
    BSS_FORCEINLINE void _inclength() { atomic_xadd<ST_>(&_length); }
    BSS_FORCEINLINE void _declength() { atomic_xadd<ST_>(&_length,-1); }
    ST_ _length; 
  };
  template<>
  struct i_LocklessQueue_Length<void>
  { 
    BSS_FORCEINLINE static void _inclength() { }
    BSS_FORCEINLINE static void _declength() { }
  };
  
  // Single-producer single-consumer lockless queue implemented in such a way that it can use a normal single-threaded allocator
  template<typename T, typename LENGTH=void>
  class cLocklessQueue : public i_LocklessQueue_Length<LENGTH>
  {
  public:
    inline cLocklessQueue() { _div=_last=_first=_alloc.alloc(1); new((cLQ_QNode<T>*)_first) cLQ_QNode<T>(); }
    inline ~cLocklessQueue() { } // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Push(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Pop(T& result)
    { 
      if( _div != _last )
      {
        result = std::move(_div->next->item); 	// try to use move semantics if possible
        atomic_xchg<volatile cLQ_QNode<T>*>(&_div,_div->next);		// publish it
        i_LocklessQueue_Length<LENGTH>::_declength(); // Decrement length if we're tracking it
        return true;
      }
      return false;
    }

  protected:
    template<typename U>
    void _produce(U && t)
    {
      _last->next=_alloc.alloc(1);
      new((cLQ_QNode<T>*)_last->next) cLQ_QNode<T>(std::forward<U>(t));
      atomic_xchg<volatile cLQ_QNode<T>*>(&_last,_last->next);		// publish it
      i_LocklessQueue_Length<LENGTH>::_inclength(); // If we are tracking length, atomically increment it
      
      cLQ_QNode<T>* tmp; // collect garbage
      while( _first != _div ) {	
        tmp = _first;
        _first = _first->next;
        tmp->~cLQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(tmp); 
      }
    }

    cFixedAlloc<cLQ_QNode<T>> _alloc;
    cLQ_QNode<T>* _first; 
    BSS_ALIGN(64) volatile cLQ_QNode<T>* _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) volatile cLQ_QNode<T>* _last;
  };

  // Multi-producer Multi-consumer lockless queue using a multithreaded allocator
  template<typename T, typename LENGTH = void>
  class cLocklessQueueMM : public i_LocklessQueue_Length<LENGTH>
  {
  public:
    inline cLocklessQueueMM() : _spin(0) { _last = _div.p = _alloc.alloc(1); new(_last) cLQ_QNode<T>(); }
    inline ~cLocklessQueueMM() { } // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Push(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Pop(T& result)
    {
      bss_PTag<cLQ_QNode<T>> ref;
      bss_PTag<cLQ_QNode<T>> nval;
      while((ref = (bss_PTag<cLQ_QNode<T>>&)_div).p != _last)
      {
        nval.p = ref.p->next;
        nval.tag = ref.tag+1;
        if(!asmcas<bss_PTag<cLQ_QNode<T>>>(&_div, nval, ref)) continue;
        result = std::move(ref.p->item); 	// try to use move semantics if possible
        ref.p->~cLQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(ref.p);
        i_LocklessQueue_Length<LENGTH>::_declength(); // Decrement length if we're tracking it
        return true;
      }
      return false;
    }

  protected:
    template<typename U>
    void _produce(U && t)
    {
      cLQ_QNode<T>* nval = _alloc.alloc(1);
      new(nval) cLQ_QNode<T>();

      while(!asmcas<cLQ_QNode<T>*>(&_last->next, nval, 0)); // So long as last->next is zero, this will succeed and then block all other threads until the below atomic exchange is completed, at which point _last->next will be 0 again.
      _last->item=std::forward<U>(t);
      atomic_xchg<cLQ_QNode<T>*>(&_last, nval);
      i_LocklessQueue_Length<LENGTH>::_inclength(); // If we are tracking length, atomically increment it
    }

    cLocklessFixedAlloc<cLQ_QNode<T>> _alloc;
    BSS_ALIGN(64) volatile bss_PTag<cLQ_QNode<T>> _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) cLQ_QNode<T>* _last;
    BSS_ALIGN(4) volatile size_t _spin;
  };

  // Single-producer multi-consumer lockless queue implemented in such a way that it can use a normal single-threaded allocator
  /*template<typename T, typename LENGTH=void>
  class cLocklessQueueSM : public i_LocklessQueue_Length<LENGTH>
  {
  public:
    inline cLocklessQueueSM() { _div.p=_last=_first=_alloc.alloc(1); _div.tag=0; new((cLQ_QNode<T>*)_first) cLQ_QNode<T>(); }
    inline ~cLocklessQueueSM() { } // Don't need to clean up because the allocator will destroy everything by itself
    inline void Produce(const T& t) { _produce<const T&>(t); }
    inline void Produce(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Consume(T& result)
    { 
      bss_PTag<cLQ_QNode<T>> cur = { _div.p,_div.tag };
      bss_PTag<cLQ_QNode<T>> ndiv = { cur.p->next,cur.tag+1 };
      result = std::move(cur.p->next->item); 	// THIS DOESNT WORK, cur.p->next could be invalid due to race conditions and you can't touch it ever
      while(cur.p != _last && asmcas<bss_PTag<cLQ_QNode<T>>>(&_div,ndiv,_div))
      {
        atomic_xchg<volatile cLQ_QNode<T>*>(&_div,_div->next);		// publish it
        i_LocklessQueue_Length<LENGTH>::_declength(); // Decrement length if we're tracking it
        return true;
      }
      return false;
    }

  protected:
    template<typename U>
    inline void _produce(U && t)
    {
      _last->next=_alloc.alloc(1);
      new((cLQ_QNode<T>*)_last->next) cLQ_QNode<T>(std::forward<U>(t));
      atomic_xchg<volatile cLQ_QNode<T>*>(&_last,_last->next);		// publish it
      i_LocklessQueue_Length<LENGTH>::_inclength(); // If we are tracking length, atomically increment it
      
      cLQ_QNode<T>* tmp; // collect garbage
      while( _first != _div.p ) {	
        tmp = _first;
        _first = _first->next;
        tmp->~cLQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(tmp); 
      }
    }

    cFixedAlloc<cLQ_QNode<T>> _alloc;
    cLQ_QNode<T>* _first; 
    BSS_ALIGN(64) volatile bss_PTag<cLQ_QNode<T>> _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) volatile cLQ_QNode<T>* _last;
  };*/

}

#endif
