// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __LOCKLESS_QUEUE_H__
#define __LOCKLESS_QUEUE_H__

#include "BlockAllocMT.h"

namespace bss {
  namespace internal {
    template<typename T>
    struct LQ_QNode {
      inline LQ_QNode() : next(0) {} // This lets item have a proper default constructor
      template<typename U>
      inline LQ_QNode(U && Item) : next(0), item(std::forward<U>(Item)) {}
      LQ_QNode* next;
      T item;
    };

    // Internal class used to toggle whether or not the queue stores and updates a length value.
    template<typename CT_>
    struct LocklessQueue_Length
    {
      inline LocklessQueue_Length(const LocklessQueue_Length& copy) : _length(copy._length.load(std::memory_order_relaxed)) {}
      inline LocklessQueue_Length() : _length(0) {}
      inline CT_ Length() const { return _length; }

      LocklessQueue_Length& operator=(const LocklessQueue_Length& copy) { _length.store(copy._length.load(std::memory_order_relaxed), std::memory_order_relaxed); return *this; }

    protected:
      BSS_FORCEINLINE void _incLength() { _length.fetch_add(1); }
      BSS_FORCEINLINE void _decLength() { _length.fetch_add((CT_)-1); }
      std::atomic<CT_> _length;
    };
    template<>
    struct LocklessQueue_Length<void>
    {
      BSS_FORCEINLINE static void _incLength() {}
      BSS_FORCEINLINE static void _decLength() {}
    };
  }

  // Single-producer single-consumer lockless queue implemented in such a way that it can use a normal single-threaded allocator
  template<typename T, typename LENGTH = void, typename Alloc = PolymorphicAllocator<internal::LQ_QNode<T>, BlockPolicy>>
  class LocklessQueue : public internal::LocklessQueue_Length<LENGTH>, public Alloc
  {
    typedef internal::LQ_QNode<T> QNODE;
    LocklessQueue(const LocklessQueue&) = delete;
    LocklessQueue& operator=(const LocklessQueue&) = delete;

  public:
    LocklessQueue(LocklessQueue&& mov) : Alloc(std::move(mov)), internal::LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last), _first(mov._first) { mov._div = mov._last = mov._first = 0; }
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline LocklessQueue(typename Alloc::policy_type* policy) : Alloc(policy)
    {
      _div = _last = _first = Alloc::allocate(1);
      new((QNODE*)_first) QNODE();
    }
    inline LocklessQueue()
    {
      _div = _last = _first = Alloc::allocate(1);
      new((QNODE*)_first) QNODE();
      /*assert(_last.is_lock_free()); assert(_div.is_lock_free());*/   // bug in GCC doesn't define is_lock_free
    }
    inline ~LocklessQueue()
    {
      while(QNODE* tmp = _first)
      {
        _first = _first->next;
        tmp->~QNODE();
        Alloc::deallocate(tmp, 1);
      }
    }
    BSS_FORCEINLINE void Push(const T& item) { _produce<const T&>(item); }
    BSS_FORCEINLINE void Push(T&& item) { _produce<T&&>(std::move(item)); }
    inline bool Pop(T& result)
    {
      QNODE* div = _div.load(std::memory_order_acquire);

      if(div != _last.load(std::memory_order_relaxed))
      {
        result = std::move(div->next->item); 	// try to use move semantics if possible
        _div.store(div->next, std::memory_order_release); // publish it
        internal::LocklessQueue_Length<LENGTH>::_decLength(); // Decrement length if we're tracking it
        return true;
      }

      return false;
    }
    inline bool Peek() { return _div.load(std::memory_order_relaxed) != _last.load(std::memory_order_relaxed); }

    inline LocklessQueue& operator=(LocklessQueue&& mov)
    {
      Alloc::operator=(std::move(mov));
      _div = mov._div;
      _last = mov._last;
      _first = mov._first;
      mov._div = mov._last = mov._first = 0;
      internal::LocklessQueue_Length<LENGTH>::operator=(std::move(mov));
      return *this;
    }

  protected:
    template<typename U>
    void _produce(U && item)
    {
      QNODE* last = _last.load(std::memory_order_acquire);
      last->next = Alloc::allocate(1);
      new((QNODE*)last->next) QNODE(std::forward<U>(item));
      _last.store(last->next, std::memory_order_release); // publish it
      internal::LocklessQueue_Length<LENGTH>::_incLength(); // If we are tracking length, atomically increment it

      QNODE* tmp; // collect garbage
      while(_first != _div.load(std::memory_order_relaxed))
      {
        tmp = _first;
        _first = _first->next;
        tmp->~QNODE(); // We have to let item clean itself up
        Alloc::deallocate(tmp, 1);
      }
    }

    QNODE* _first;
    BSS_ALIGN(64) std::atomic<QNODE*> _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) std::atomic<QNODE*> _last;
  };

  // Multi-producer Multi-consumer microlock queue using a multithreaded allocator
  template<typename T, typename LENGTH = void, typename Alloc = PolymorphicAllocator<internal::LQ_QNode<T>, LocklessBlockPolicy>>
  class MicroLockQueue : public internal::LocklessQueue_Length<LENGTH>, Alloc
  {
    typedef internal::LQ_QNode<T> QNODE;
    MicroLockQueue(const MicroLockQueue&) = delete;
    MicroLockQueue& operator=(const MicroLockQueue&) = delete;

  public:
    MicroLockQueue(MicroLockQueue&& mov) : Alloc(std::move(mov)), internal::LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last)
    {
      mov._div = mov._last = 0;
      _cflag.clear(std::memory_order_relaxed);
      _pflag.clear(std::memory_order_relaxed);
    }
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline explicit MicroLockQueue(typename Alloc::policy_type* policy) : Alloc(policy)
    {
      _last = _div = Alloc::allocate(1);
      new(_div)QNODE();
      _cflag.clear(std::memory_order_relaxed);
      _pflag.clear(std::memory_order_relaxed);
    }
    inline MicroLockQueue()
    {
      _last = _div = Alloc::allocate(1);
      new(_div)QNODE();
      _cflag.clear(std::memory_order_relaxed);
      _pflag.clear(std::memory_order_relaxed);
    }
    inline ~MicroLockQueue()
    {
      while(QNODE* tmp = _div)
      {
        _div = _div->next;
        tmp->~QNODE();
        Alloc::deallocate(tmp, 1);
      }
    }
    BSS_FORCEINLINE void Push(const T& item) { _produce<const T&>(item); }
    BSS_FORCEINLINE void Push(T&& item) { _produce<T&&>(std::move(item)); }
    inline bool Pop(T& result)
    {
      if(!_div->next) return false; // Remove some contending pressure

      while(_cflag.test_and_set(std::memory_order_acquire));
      QNODE* ref = _div;
      QNODE* n = _div->next;

      if(n != 0)
      {
        result = std::move(n->item); 	// try to use move semantics if possible
        _div = n;
        _cflag.clear(std::memory_order_release);
        ref->~QNODE(); // We have to let item clean itself up
        Alloc::deallocate(ref, 1);
        internal::LocklessQueue_Length<LENGTH>::_decLength(); // If we are tracking length, atomically decrement it
        return true;
      }

      _cflag.clear(std::memory_order_release);
      return false;
    }
    inline bool Peek() { return _div->next != 0; }
    inline MicroLockQueue& operator=(MicroLockQueue&& mov)
    {
      Alloc::operator=(std::move(mov));
      _div = mov._div;
      _last = mov._last;
      _cflag.clear(std::memory_order_release);
      mov._div = mov._last = 0;
      internal::LocklessQueue_Length<LENGTH>::operator=(std::move(mov));
      return *this;
    }
  protected:
    template<typename U>
    void _produce(U && item)
    {
      QNODE* nval = Alloc::allocate(1);
      new(nval) QNODE(std::forward<U>(item));

      while(_pflag.test_and_set(std::memory_order_acquire));
      _last->next = nval;
      _last = nval; // This can happen before or after modifying _last->next because no other function uses _last
      _pflag.clear(std::memory_order_release);
      internal::LocklessQueue_Length<LENGTH>::_incLength(); // If we are tracking length, atomically increment it
    }

    BSS_ALIGN(64) QNODE* _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) QNODE* _last;
    BSS_ALIGN(64) std::atomic_flag _cflag;
    BSS_ALIGN(64) std::atomic_flag _pflag;
  };

  // Multi-producer Multi-consumer lockless queue using a multithreaded allocator
  /*template<typename T, typename LENGTH = void>
  class MicroLockQueue : public internal::LocklessQueue_Length<LENGTH>
  {
  public:
    MicroLockQueue(const MicroLockQueue&) = delete;
    MicroLockQueue(MicroLockQueue&& mov) : internal::LocklessQueue_Length<LENGTH>(std::move(mov)), _div(mov._div), _last(mov._last), _alloc(std::move(mov._alloc)) { mov._div=mov._last=0; }
    inline MicroLockQueue() { _last.store(_div.p = _alloc.Alloc(1), std::memory_order_relaxed); new(_div.p) QNODE(); assert(_last.is_lock_free());}
    inline ~MicroLockQueue() { } // Don't need to clean up because the allocator will destroy everything by itself
    BSS_FORCEINLINE void Push(const T& item) { _produce<const T&>(item); }
    BSS_FORCEINLINE void Push(T&& item) { _produce<T&&>(std::move(item)); }
    inline bool Pop(T& result)
    {
      bss_PTag<QNODE> ref ={0, 0};
      bss_PTag<QNODE> nval;
      asmcasr<bss_PTag<QNODE>>(&_div, ref, ref, ref);
      while(ref.p != _last.load(std::memory_order_relaxed))
      {
        nval.p = ref.p->next; // This can fail because if there are two consumers ahead of this one, one can increment the pointer, then another can increment the pointer and is then legally allowed to destroy that pointer, leaving this thread with an invalid ref.p->next value.
        nval.tag = ref.tag+1;
        if(!asmcasr<bss_PTag<QNODE>>(&_div, nval, ref, ref)) continue;
        result = std::move(ref.p->item); 	// try to use move semantics if possible
        ref.p->~QNODE(); // We have to let item clean itself up
        _alloc.Dealloc(ref.p);
        internal::LocklessQueue_Length<LENGTH>::_decLength(); // Decrement length if we're tracking it
        return true;
      }
      return false;
    }

    MicroLockQueue& operator=(const MicroLockQueue&) = delete;
    inline MicroLockQueue& operator=(MicroLockQueue&& mov)
    {
        _div=mov._div;
        _last=mov._last;
        _alloc=std::move(mov._alloc);
        mov._div=mov._last=0;
        internal::LocklessQueue_Length<LENGTH>::operator=(std::move(mov));
    }
  protected:
    template<typename U>
    void _produce(U && t)
    {
      QNODE* nval = _alloc.Alloc(1);
      new(nval) QNODE();

      while(!asmcas<QNODE*>(&_last.load(std::memory_order_relaxed)->next, nval, 0)); //This doesn't work because of a race condition where this thread loads _last, then another producer pushes through a new _last and a consume thread then destroys the _last we just loaded, and we end up writing to an undefined location.
      _last.load(std::memory_order_relaxed)->item=std::forward<U>(t);
      _last.store(nval, std::memory_order_release);
      internal::LocklessQueue_Length<LENGTH>::_incLength(); // If we are tracking length, atomically increment it
    }

    LocklessBlockAlloc<QNODE> _alloc;
    BSS_ALIGN(64) bss_PTag<QNODE> _div; // Align to try and get them on different cache lines
    BSS_ALIGN(64) std::atomic<QNODE*> _last;
  };*/
}

#endif
