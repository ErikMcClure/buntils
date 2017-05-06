// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LOCKLESS_QUEUE_H__
#define __C_LOCKLESS_QUEUE_H__

#include "bss-util/bss_alloc_block_MT.h"

namespace bss {
  template<typename T>
  struct LQ_QNode {
    inline LQ_QNode() : next(0) {} // This lets item have a proper default constructor
    template<typename U>
    inline LQ_QNode(U && item) : next(0), item(std::forward<U>(item)) {}
    LQ_QNode* next;
    T item;
  };

  // Internal class used to toggle whether or not the queue stores and updates a length value.
  template<typename CT_>
  struct i_LocklessQueue_Length
  {
    inline i_LocklessQueue_Length(const i_LocklessQueue_Length& copy) : _length(copy._length.load(std::memory_order_relaxed)) {}
    inline i_LocklessQueue_Length() : _length(0) {}
    inline CT_ Length() const { return _length; }

    i_LocklessQueue_Length& operator=(const i_LocklessQueue_Length& copy) { _length.store(copy._length.load(std::memory_order_relaxed), std::memory_order_relaxed); return *this; }

  protected:
    BSS_FORCEINLINE void _incLength() { _length.fetch_add(1); }
    BSS_FORCEINLINE void _decLength() { _length.fetch_add((CT_)-1); }
    std::atomic<CT_> _length;
  };
  template<>
  struct i_LocklessQueue_Length<void>
  {
    BSS_FORCEINLINE static void _incLength() {}
    BSS_FORCEINLINE static void _decLength() {}
  };

  // Single-producer single-consumer lockless queue implemented in such a way that it can use a normal single-threaded allocator
  template<typename T, typename LENGTH = void>
  class LocklessQueue : public i_LocklessQueue_Length<LENGTH>
  {
    LocklessQueue(const LocklessQueue&) BSS_DELETEFUNC
    LocklessQueue& operator=(const LocklessQueue&)BSS_DELETEFUNCOP

  public:
    LocklessQueue(LocklessQueue&& mov) : i_LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last), _first(mov._first), _alloc(std::move(mov._alloc)) { mov._div = mov._last = mov._first = 0; }
    inline LocklessQueue() { _div = _last = _first = _alloc.alloc(1); new((LQ_QNode<T>*)_first) LQ_QNode<T>(); /*assert(_last.is_lock_free()); assert(_div.is_lock_free());*/ } // bug in GCC doesn't define is_lock_free
    inline ~LocklessQueue() {} // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Push(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Pop(T& result)
    {
      LQ_QNode<T>* div = _div.load(std::memory_order_acquire);
      if(div != _last.load(std::memory_order_relaxed))
      {
        result = std::move(div->next->item); 	// try to use move semantics if possible
        _div.store(div->next, std::memory_order_release); // publish it
        i_LocklessQueue_Length<LENGTH>::_decLength(); // Decrement length if we're tracking it
        return true;
      }
      return false;
    }
    inline bool Peek() { return _div.load(std::memory_order_relaxed) != _last.load(std::memory_order_relaxed); }

    inline LocklessQueue& operator=(LocklessQueue&& mov)
    {
      _div = mov._div;
      _last = mov._last;
      _first = mov._first;
      _alloc = std::move(mov._alloc);
      mov._div = mov._last = mov._first = 0;
      i_LocklessQueue_Length<LENGTH>::operator=(std::move(mov));
      return *this;
    }

  protected:
    template<typename U>
    void _produce(U && t)
    {
      LQ_QNode<T>* last = _last.load(std::memory_order_acquire);
      last->next = _alloc.alloc(1);
      new((LQ_QNode<T>*)last->next) LQ_QNode<T>(std::forward<U>(t));
      _last.store(last->next, std::memory_order_release); // publish it
      i_LocklessQueue_Length<LENGTH>::_incLength(); // If we are tracking length, atomically increment it

      LQ_QNode<T>* tmp; // collect garbage
      while(_first != _div.load(std::memory_order_relaxed))
      {
        tmp = _first;
        _first = _first->next;
        tmp->~LQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(tmp);
      }
    }

    BlockAlloc<LQ_QNode<T>> _alloc;
    LQ_QNode<T>* _first;
    BSS_ALIGN(64) std::atomic<LQ_QNode<T>*> _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) std::atomic<LQ_QNode<T>*> _last;
  };

  // Multi-producer Multi-consumer microlock queue using a multithreaded allocator
  template<typename T, typename LENGTH = void>
  class MicroLockQueue : public i_LocklessQueue_Length<LENGTH>
  {
    MicroLockQueue(const MicroLockQueue&) BSS_DELETEFUNC
    MicroLockQueue& operator=(const MicroLockQueue&) BSS_DELETEFUNCOP

  public:
    MicroLockQueue(MicroLockQueue&& mov) : i_LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last), _alloc(std::move(mov._alloc)) { mov._div = mov._last = 0; _cflag.clear(std::memory_order_relaxed);  _pflag.clear(std::memory_order_relaxed); }
    inline MicroLockQueue() { _last = _div = _alloc.alloc(1); new(_div)LQ_QNode<T>(); _cflag.clear(std::memory_order_relaxed);  _pflag.clear(std::memory_order_relaxed); }
    inline ~MicroLockQueue() {} // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Push(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Pop(T& result)
    {
      if(!_div->next) return false; // Remove some contending pressure
      while(_cflag.test_and_set(std::memory_order_acquire));
      LQ_QNode<T>* ref = _div;
      LQ_QNode<T>* n = _div->next;
      if(n != 0)
      {
        result = std::move(n->item); 	// try to use move semantics if possible
        _div = n;
        _cflag.clear(std::memory_order_release);
        ref->~LQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(ref);
        i_LocklessQueue_Length<LENGTH>::_decLength(); // If we are tracking length, atomically decrement it
        return true;
      }
      _cflag.clear(std::memory_order_release);
      return false;
    }
    inline bool Peek() { return _div->next != 0; }
    inline MicroLockQueue& operator=(MicroLockQueue&& mov)
    {
      _div = mov._div;
      _last = mov._last;
      _alloc = std::move(mov._alloc);
      _cflag.clear(std::memory_order_release);
      mov._div = mov._last = 0;
      i_LocklessQueue_Length<LENGTH>::operator=(std::move(mov));
      return *this;
    }
  protected:
    template<typename U>
    void _produce(U && t)
    {
      LQ_QNode<T>* nval = _alloc.alloc(1);
      new(nval) LQ_QNode<T>(std::forward<U>(t));

      while(_pflag.test_and_set(std::memory_order_acquire));
      _last->next = nval;
      _last = nval; // This can happen before or after modifying _last->next because no other function uses _last
      _pflag.clear(std::memory_order_release);
      i_LocklessQueue_Length<LENGTH>::_incLength(); // If we are tracking length, atomically increment it
    }

    LocklessBlockAlloc<LQ_QNode<T>> _alloc;
    BSS_ALIGN(64) LQ_QNode<T>* _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) LQ_QNode<T>* _last;
    BSS_ALIGN(64) std::atomic_flag _cflag;
    BSS_ALIGN(64) std::atomic_flag _pflag;
  };

  // Multi-producer Multi-consumer lockless queue using a multithreaded allocator
  /*template<typename T, typename LENGTH = void>
  class MicroLockQueue : public i_LocklessQueue_Length<LENGTH>
  {
  public:
    MicroLockQueue(const MicroLockQueue&) BSS_DELETEFUNC
    MicroLockQueue(MicroLockQueue&& mov) : i_LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last), _alloc(std::move(mov._alloc)) { mov._div=mov._last=0; }
    inline MicroLockQueue() { _last.store(_div.p = _alloc.alloc(1), std::memory_order_relaxed); new(_div.p) LQ_QNode<T>(); assert(_last.is_lock_free());}
    inline ~MicroLockQueue() { } // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& t) { _produce<const T&>(t); }
    BSS_FORCEINLINE void Push(T&& t) { _produce<T&&>(std::move(t)); }
    inline bool Pop(T& result)
    {
      bss_PTag<LQ_QNode<T>> ref ={0, 0};
      bss_PTag<LQ_QNode<T>> nval;
      asmcasr<bss_PTag<LQ_QNode<T>>>(&_div, ref, ref, ref);
      while(ref.p != _last.load(std::memory_order_relaxed))
      {
        nval.p = ref.p->next; // This can fail because if there are two consumers ahead of this one, one can increment the pointer, then another can increment the pointer and is then legally allowed to destroy that pointer, leaving this thread with an invalid ref.p->next value.
        nval.tag = ref.tag+1;
        if(!asmcasr<bss_PTag<LQ_QNode<T>>>(&_div, nval, ref, ref)) continue;
        result = std::move(ref.p->item); 	// try to use move semantics if possible
        ref.p->~LQ_QNode<T>(); // We have to let item clean itself up
        _alloc.dealloc(ref.p);
        i_LocklessQueue_Length<LENGTH>::_decLength(); // Decrement length if we're tracking it
        return true;
      }
      return false;
    }

    MicroLockQueue& operator=(const MicroLockQueue&) BSS_DELETEFUNCOP
    inline MicroLockQueue& operator=(MicroLockQueue&& mov)
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
      LQ_QNode<T>* nval = _alloc.alloc(1);
      new(nval) LQ_QNode<T>();

      while(!asmcas<LQ_QNode<T>*>(&_last.load(std::memory_order_relaxed)->next, nval, 0)); //This doesn't work because of a race condition where this thread loads _last, then another producer pushes through a new _last and a consume thread then destroys the _last we just loaded, and we end up writing to an undefined location.
      _last.load(std::memory_order_relaxed)->item=std::forward<U>(t);
      _last.store(nval, std::memory_order_release);
      i_LocklessQueue_Length<LENGTH>::_incLength(); // If we are tracking length, atomically increment it
    }

    LocklessBlockAlloc<LQ_QNode<T>> _alloc;
    BSS_ALIGN(64) bss_PTag<LQ_QNode<T>> _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) std::atomic<LQ_QNode<T>*> _last;
  };*/
}

#endif
