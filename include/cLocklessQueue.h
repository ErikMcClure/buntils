// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LOCKLESS_QUEUE_H__
#define __C_LOCKLESS_QUEUE_H__

#include "bss_alloc_block_MT.h"

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
  template<typename CT_>
  struct i_LocklessQueue_Length
  { 
    inline i_LocklessQueue_Length(const i_LocklessQueue_Length& copy) : _length(copy._length.load(std::memory_order_relaxed)) { }
    inline i_LocklessQueue_Length() : _length(0) { }
    inline CT_ Length() const { return _length; }

    i_LocklessQueue_Length& operator=(const i_LocklessQueue_Length& copy) { _length.store(copy._length.load(std::memory_order_relaxed), std::memory_order_relaxed); return *this; }

  protected:
    BSS_FORCEINLINE void _inclength() { _length.fetch_add(1); }
    BSS_FORCEINLINE void _declength() { _length.fetch_add((CT_)-1); }
    std::atomic<CT_> _length; 
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
    cLocklessQueue(const cLocklessQueue&) BSS_DELETEFUNC
    cLocklessQueue& operator=(const cLocklessQueue&)BSS_DELETEFUNCOP
  public:
    cLocklessQueue(cLocklessQueue&& mov) : i_LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last), _first(mov._first), _alloc(std::move(mov._alloc)) { mov._div=mov._last=mov._first=0; }
    inline cLocklessQueue() { _div=_last=_first=_alloc.alloc(1); new((cLQ_QNode<T>*)_first) cLQ_QNode<T>(); /*assert(_last.is_lock_free()); assert(_div.is_lock_free());*/ } // bug in GCC doesn't define is_lock_free
    inline ~cLocklessQueue() { } // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Push(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Pop(T& result)
    { 
      cLQ_QNode<T>* div=_div.load(std::memory_order_acquire);
      if(div != _last.load(std::memory_order_relaxed))
      {
        result = std::move(div->next->item); 	// try to use move semantics if possible
        _div.store(div->next, std::memory_order_release); // publish it
        i_LocklessQueue_Length<LENGTH>::_declength(); // Decrement length if we're tracking it
        return true;
      }
      return false;
    }
    inline bool Peek() { return _div.load(std::memory_order_relaxed) != _last.load(std::memory_order_relaxed); }

    inline cLocklessQueue& operator=(cLocklessQueue&& mov)
    {
      _div=mov._div;
      _last=mov._last;
      _first=mov._first;
      _alloc=std::move(mov._alloc);
      mov._div=mov._last=mov._first=0;
      i_LocklessQueue_Length<LENGTH>::operator=(std::move(mov));
      return *this;
    }

  protected:
    template<typename U>
    void _produce(U && t)
    {
      cLQ_QNode<T>* last=_last.load(std::memory_order_acquire);
      last->next=_alloc.alloc(1);
      new((cLQ_QNode<T>*)last->next) cLQ_QNode<T>(std::forward<U>(t));
      _last.store(last->next, std::memory_order_release); // publish it
      i_LocklessQueue_Length<LENGTH>::_inclength(); // If we are tracking length, atomically increment it
      
      cLQ_QNode<T>* tmp; // collect garbage
      while(_first != _div.load(std::memory_order_relaxed)) {
        tmp = _first;
        _first = _first->next;
        tmp->~cLQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(tmp); 
      }
    }

    cBlockAlloc<cLQ_QNode<T>> _alloc;
    cLQ_QNode<T>* _first;
    BSS_ALIGN(64) std::atomic<cLQ_QNode<T>*> _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) std::atomic<cLQ_QNode<T>*> _last;
  };

  // Multi-producer Multi-consumer microlock queue using a multithreaded allocator
  template<typename T, typename LENGTH = void>
  class cMicroLockQueue : public i_LocklessQueue_Length<LENGTH>
  {
    cMicroLockQueue(const cMicroLockQueue&) BSS_DELETEFUNC
    cMicroLockQueue& operator=(const cMicroLockQueue&) BSS_DELETEFUNCOP
  public:
    cMicroLockQueue(cMicroLockQueue&& mov) : i_LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last), _alloc(std::move(mov._alloc)) { mov._div=mov._last=0; _cflag.clear(std::memory_order_relaxed);  _pflag.clear(std::memory_order_relaxed); }
    inline cMicroLockQueue() { _last = _div = _alloc.alloc(1); new(_div)cLQ_QNode<T>(); _cflag.clear(std::memory_order_relaxed);  _pflag.clear(std::memory_order_relaxed); }
    inline ~cMicroLockQueue() { } // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Push(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Pop(T& result)
    {
      if(!_div->next) return false; // Remove some contending pressure
      while(_cflag.test_and_set(std::memory_order_acquire));
      cLQ_QNode<T>* ref = _div;
      cLQ_QNode<T>* n = _div->next;
      if(n != 0) {
        result = std::move(n->item); 	// try to use move semantics if possible
        _div = n;
        _cflag.clear(std::memory_order_release);
        ref->~cLQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(ref);
        i_LocklessQueue_Length<LENGTH>::_declength(); // If we are tracking length, atomically decrement it
        return true;
      }
      _cflag.clear(std::memory_order_release);
      return false; 
    }
    inline bool Peek() { return _div->next != 0; }
    inline cMicroLockQueue& operator=(cMicroLockQueue&& mov)
    {
      _div=mov._div;
      _last=mov._last;
      _alloc=std::move(mov._alloc);
      _cflag.clear(std::memory_order_release);
      mov._div=mov._last=0;
      i_LocklessQueue_Length<LENGTH>::operator=(std::move(mov));
      return *this;
    }
  protected:
    template<typename U>
    void _produce(U && t)
    {
      cLQ_QNode<T>* nval = _alloc.alloc(1);
      new(nval) cLQ_QNode<T>(std::forward<U>(t));

      while(_pflag.test_and_set(std::memory_order_acquire));
      _last->next=nval;
      _last=nval; // This can happen before or after modifying _last->next because no other function uses _last
      _pflag.clear(std::memory_order_release);
      i_LocklessQueue_Length<LENGTH>::_inclength(); // If we are tracking length, atomically increment it
    }

    cLocklessBlockAlloc<cLQ_QNode<T>> _alloc;
    BSS_ALIGN(64) cLQ_QNode<T>* _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) cLQ_QNode<T>* _last;
    BSS_ALIGN(64) std::atomic_flag _cflag;
    BSS_ALIGN(64) std::atomic_flag _pflag;
  };
  
  // Multi-producer Multi-consumer lockless queue using a multithreaded allocator
  /*template<typename T, typename LENGTH = void>
  class cMicroLockQueue : public i_LocklessQueue_Length<LENGTH>
  {
  public:
    cMicroLockQueue(const cMicroLockQueue&) BSS_DELETEFUNC
    cMicroLockQueue(cMicroLockQueue&& mov) : i_LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last), _alloc(std::move(mov._alloc)) { mov._div=mov._last=0; }
    inline cMicroLockQueue() { _last.store(_div.p = _alloc.alloc(1), std::memory_order_relaxed); new(_div.p) cLQ_QNode<T>(); assert(_last.is_lock_free());}
    inline ~cMicroLockQueue() { } // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Push(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Pop(T& result)
    {
      bss_PTag<cLQ_QNode<T>> ref ={0, 0};
      bss_PTag<cLQ_QNode<T>> nval;
      asmcasr<bss_PTag<cLQ_QNode<T>>>(&_div, ref, ref, ref);
      while(ref.p != _last.load(std::memory_order_relaxed))
      {
        nval.p = ref.p->next; // This can fail because if there are two consumers ahead of this one, one can increment the pointer, then another can increment the pointer and is then legally allowed to destroy that pointer, leaving this thread with an invalid ref.p->next value.
        nval.tag = ref.tag+1;
        if(!asmcasr<bss_PTag<cLQ_QNode<T>>>(&_div, nval, ref, ref)) continue;
        result = std::move(ref.p->item); 	// try to use move semantics if possible
        ref.p->~cLQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(ref.p);
        i_LocklessQueue_Length<LENGTH>::_declength(); // Decrement length if we're tracking it
        return true;
      }
      return false;
    }

    cMicroLockQueue& operator=(const cMicroLockQueue&) BSS_DELETEFUNCOP
    inline cMicroLockQueue& operator=(cMicroLockQueue&& mov)
    {
        _div=mov._div;
        _last=mov._last;
        _alloc=std::move(mov._alloc);
        mov._div=mov._last=0;
        i_LocklessQueue_Length<LENGTH>::operator=(std::move(mov));
    }
  protected:
    template<typename U>
    void _produce(U && t)
    {
      cLQ_QNode<T>* nval = _alloc.alloc(1);
      new(nval) cLQ_QNode<T>();

      while(!asmcas<cLQ_QNode<T>*>(&_last.load(std::memory_order_relaxed)->next, nval, 0)); //This doesn't work because of a race condition where this thread loads _last, then another producer pushes through a new _last and a consume thread then destroys the _last we just loaded, and we end up writing to an undefined location.
      _last.load(std::memory_order_relaxed)->item=std::forward<U>(t);
      _last.store(nval, std::memory_order_release);
      i_LocklessQueue_Length<LENGTH>::_inclength(); // If we are tracking length, atomically increment it
    }

    cLocklessBlockAlloc<cLQ_QNode<T>> _alloc;
    BSS_ALIGN(64) bss_PTag<cLQ_QNode<T>> _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) std::atomic<cLQ_QNode<T>*> _last;
  };*/
}

#endif
