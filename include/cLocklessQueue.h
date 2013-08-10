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
    BSS_FORCEINLINE void Produce(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Produce(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Consume(T& result)
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
      result = std::move(cur.p->next->item); 	// try to use move semantics if possible
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

  // We use a partial specialization to move the single-producer, single-consumer case to a different, more efficient algorithm.
  /*template<typename T, typename LENGTH=void>
  class cLocklessQueue<T,true,true,LENGTH,void> : public cLocklessQueue<T,LENGTH>
  { // (move this down when you are actually going to use it)
  };

  // Lockless queue using a fixed lockless allocator for cheap memory allocations. Can be any combination of single/multi producer/consumer
  template<typename T, typename LENGTH=void, bool SINGLE_PRODUCER=true, bool SINGLE_CONSUMER=true>
  class cLocklessQueue : public i_LocklessQueue_Length<LENGTH>
  {
    struct cLQ_QNode<T> {
      inline cLQ_QNode<T>() : next(0) {} // This lets item have a proper default constructor
      template<typename U>
      inline cLQ_QNode<T>(U && item) : next(0), item(std::forward<U>(item)) {}
      cLQ_QNode<T>* next;
      T item;
    };

  public:
    inline cLocklessQueue() { _first=_last=_alloc.alloc(1); new((cLQ_QNode<T>*)_first) cLQ_QNode<T>(); }
    inline ~cLocklessQueue() { } // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Produce(const T& t) { return _produce<SINGLE_PRODUCER>(t); }
    BSS_FORCEINLINE bool Consume(T& result) { return _consume<SINGLE_CONSUMER>(result); }

  protected:
    template<bool SINGLE>
    inline void _produce(const T& t)
    {
      cLQ_QNode<T>* n=_alloc.alloc(1);
      new((cLQ_QNode<T>*)n) cLQ_QNode<T>();
      //n->item=t;
      _last->item=t;
      _last->next =n;  	// add the new item to _last rather than to the new node (which behaves instead as an empty sentinel node)
      atomic_xchg<volatile cLQ_QNode<T>*>(&_last,n);		// publish it
      _inclength(); // If we are tracking length, atomically increment it
    }
    template<>
    inline void _produce<false>(const T& t)
    {
      cLQ_QNode<T>* n=_alloc.alloc(1);
      new((cLQ_QNode<T>*)n) cLQ_QNode<T>(t);
      n->next=_last;
      _last->next =n;  	// add the new item to _last rather than to the new node (which behaves instead as an empty sentinel node)
      atomic_xchg<volatile cLQ_QNode<T>*>(&_last,n);		// publish it
      _inclength(); // If we are tracking length, atomically increment it
      /*cLQ_QNode<T>* n=_alloc.alloc(1);
      new((cLQ_QNode<T>*)n) cLQ_QNode<T>();
      //n->item=t;
      cLQ_QNode<T>* l=(cLQ_QNode<T>*)_last;
      assert(l!=0);
      cLQ_QNode<T>* ln=(cLQ_QNode<T>*)l->next;
      while(ln!=0 || !asmcas<volatile cLQ_QNode<T>*>(&l->next,n,ln)) // ABA problem
      {
        l=(cLQ_QNode<T>*)_last;
        ln=(cLQ_QNode<T>*)l->next;
      }
      _last->item=t; // Since nonzero l->next behaves as a lock, we can take advantage of this and put the item in _last, which makes Consume simpler.
      atomic_xchg<volatile cLQ_QNode<T>*>(&_last,n); // This assignment operation should end up being atomic anyway but this helps defend against CPU reordering
      _inclength(); // If we are tracking length, atomically increment it
    }
    
    template<bool SINGLE>
    inline bool _consume(T& result)
    {
      cLQ_QNode<T>* f=(cLQ_QNode<T>*)_first;
      if(f != _last)
      {
        atomic_xchg<volatile cLQ_QNode<T>*>(&_first,f->next);
        result = f->item;
        f->~cLQ_QNode<T>();
        _alloc.dealloc(f);
        _declength(); // If we're tracking length, atomically decrement it
        return true;
      }
      return false;
    }

    template<>
    inline bool _consume<false>(T& result)
    {
      cLQ_QNode<T>* f=(cLQ_QNode<T>*)_first;
      cLQ_QNode<T>* l;
      while(f!=(l=(cLQ_QNode<T>*)_last) && !asmcas<volatile cLQ_QNode<T>*>(&_first,f->next,f)) {
        f=(cLQ_QNode<T>*)_first;
      }
      if(f!=l) // We have to be careful not to read from _last again, lest it change and trick us into thinking that instead of hitting the end we had a valid hit.
      {
        result=f->item;
        f->~cLQ_QNode<T>();
        _alloc.dealloc(f);
        _declength(); // If we're tracking length, atomically decrement it
        return true;
      }
      return false;
    }
    
    BSS_ALIGN(64) volatile cLQ_QNode<T>* _first;
    BSS_ALIGN(64) volatile cLQ_QNode<T>* _last;
    cLocklessFixedAlloc<cLQ_QNode<T>> _alloc;
    //cFAKEALLOC<cLQ_QNode<T>> _alloc;
  };
  */
}

#endif
