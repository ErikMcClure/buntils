// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_THREAD_POOL_H__BSS__
#define __C_THREAD_POOL_H__BSS__

#include "cThread.h"
#include "cLocklessQueue.h"
#include "cArrayCircular.h"

namespace bss_util
{
  // Manages a pool of worker threads
  template<int MAXTHREADS=32>
  class cThreadPool
  {
    typedef void(*FUNC)(void*);
    typedef void(*POOLFUNC)(cThreadPool&, std::atomic<unsigned int>&, unsigned int);
    typedef std::pair<FUNC, void*> PAIR;
    static const int NUMMAXCHARS = T_NEXTMULTIPLE(MAXTHREADS, 31)>>5;

    cThreadPool(const cThreadPool& copy) BSS_DELETEFUNC
    cThreadPool& operator=(const cThreadPool&)BSS_DELETEFUNCOP
  public:
    cThreadPool(cThreadPool&& mov) : _queue(std::move(mov._queue)), _pool(std::move(mov._pool)) {
      _numtasks.store(0, std::memory_order_relaxed);
      _sleepflag.store(mov._sleepflag.load(std::memory_order_relaxed), std::memory_order_relaxed);
      for(unsigned int i = 0; i < NUMMAXCHARS; ++i)
        _quitflags[i].store(mov._quitflags[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
    }
    explicit cThreadPool(size_t num=4) {
      _numtasks.store(0, std::memory_order_relaxed);
      for(unsigned int i = 0; i < NUMMAXCHARS; ++i)
        _quitflags[i].store(0, std::memory_order_relaxed);
      SetLength(num); 
    }
    ~cThreadPool() { _killall();  }
    inline void Push(FUNC f, void* a) { _numtasks.fetch_add(1, std::memory_order_acquire); _queue.Push(PAIR(f, a)); }
    inline void SetLength(size_t num)
    {
      for(unsigned int i = num; i < _pool.size(); ++i) // signal deleted threads to exit
        _quitflags[i>>5].store(_quitflags[i>>5].load(std::memory_order_relaxed)|(1<<(i%32)), std::memory_order_relaxed);
      Prime(); // prime in case the threads are asleep
      unsigned int oldsize = _pool.size();
      _pool.resize(num);
      for(unsigned int i = oldsize; i < _pool.size(); ++i) { // Initialize new threads
        _quitflags[i>>5].store(_quitflags[i>>5].load(std::memory_order_relaxed)&(~(1<<(i%32))), std::memory_order_relaxed);
        _pool[i] = cThread((POOLFUNC)&_threadpool_worker, std::ref(*this), std::ref(_quitflags[i>>5]), (1<<(i%32)));
      }
    }
    inline size_t Length() const { return _pool.size(); }
    inline size_t NumTasks() const { return _numtasks.load(std::memory_order_relaxed); }

    inline void Prime()
    {
      _sleepflag.store(0, std::memory_order_release);
      for(unsigned int i = 0; i < _pool.size(); ++i)
        _pool[i].Signal();
    }
    inline void Wait() { _sleepflag.store(1, std::memory_order_relaxed); }
    inline void Join() { while(NumTasks()>0); }

    cThreadPool& operator=(cThreadPool&& mov)
    {
      _killall();
      _queue=std::move(mov._queue);
      _pool=std::move(mov._pool);
      _sleepflag.store(mov._sleepflag.load(std::memory_order_relaxed), std::memory_order_relaxed);
      for(unsigned int i = 0; i < NUMMAXCHARS; ++i)
        _quitflags[i].store(mov._quitflags[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
      return *this;
    }

  protected:
    inline void _killall() {
      Prime();
      for(unsigned int i = 0; i < NUMMAXCHARS; ++i)
        _quitflags[i].store((unsigned int)-1, std::memory_order_relaxed);
      for(unsigned int i = 0; i < _pool.size(); ++i)
        _pool[i].join();
    }

    static void _threadpool_worker(cThreadPool& pool, std::atomic<unsigned int>& qflags, unsigned int qmask)
    {
      cMicroLockQueue<PAIR>& queue = pool._queue;
      PAIR cur;
      for(;;)
      {
        if(queue.Pop(cur)) {
          (*cur.first)(cur.second);
          pool._numtasks.fetch_add((size_t)-1, std::memory_order_release);
        }
        if(qflags.load(std::memory_order_relaxed)&qmask)
          break;
        if(pool._sleepflag.load(std::memory_order_relaxed)!=0)
          cThread::Wait();
      }
    }

    cMicroLockQueue<PAIR> _queue;
    std::vector<cThread> _pool;
    std::atomic<unsigned int> _quitflags[NUMMAXCHARS]; // each thread gets its own bit, which let's us resize the threadpool on the fly, even when things are queue up in it.
    std::atomic<unsigned char> _sleepflag;
    std::atomic<size_t> _numtasks; // We manually keep track of this so it is decremented AFTER a task is finished rather then relying on the lockless queue's internal counter.
    //std::atomic<unsigned int> _timeout;
  };

#ifdef BSS_VARIADIC_TEMPLATES
  template<typename R, typename ...Args>
  struct StoredFunction {
    typedef std::tuple<Args...> TUPLE;
    typedef R(*FUNC)(Args...);

    explicit inline StoredFunction(const StoredFunction& copy) : _f(copy._f), _args(copy._args) {}
    explicit inline StoredFunction(StoredFunction&& mov) : _f(mov._f), _args(std::move(mov._args)) {}
    explicit inline StoredFunction(FUNC f, Args... args) : _f(f), _args(args...) { }
    BSS_FORCEINLINE R operator()() const { return Call(); }
    BSS_FORCEINLINE R Call() const { return _unpack(typename bssSeq<sizeof...(Args)>::type()); }

    StoredFunction& operator=(const StoredFunction& copy) { _f=copy._f; _args=copy._args; return *this; }
    StoredFunction& operator=(StoredFunction&& mov) { _f=mov._f; _args=std::move(mov._args); return *this; }

  protected:
    template<int ...S>
    BSS_FORCEINLINE R _unpack(bssSeq_gen<S...>) const {
      (*_f)(std::get<S>(_args) ...);
    }
    TUPLE _args;
    FUNC _f;
  };

  template<int MAXTHREADS, typename ...Args>
  class cTaskPool
  {
    typedef StoredFunction<void, Args...> SFUNC;
    typedef void(*FUNC)(Args...);
    struct PAIR : SFUNC {
      PAIR(cLocklessFixedAlloc<PAIR>* p, FUNC f, Args... args) : SFUNC(f, args...), alloc(p) {}
      cLocklessFixedAlloc<PAIR>* alloc;
    };

  public:
    inline cTaskPool(const cTaskPool&) BSS_DELETEFUNC
    inline cTaskPool(cTaskPool&& mov) : _pool(mov._pool) {}
    explicit inline cTaskPool(cThreadPool<MAXTHREADS>& pool) : _pool(pool) {}
    inline void Push(FUNC f, Args... args) {
      _pool.Push(&cTaskPool::_helper, new(_alloc.alloc(1)) PAIR(&_alloc,f, args...));
    }
    inline const cThreadPool<MAXTHREADS>& GetPool() const { return _pool; }
    inline cThreadPool<MAXTHREADS>& GetPool() { return _pool; }

  protected:
    static inline void _helper(void* p) { ((PAIR*)p)->Call(); ((PAIR*)p)->alloc->dealloc(p); }

    cLocklessFixedAlloc<PAIR> _alloc;
    cThreadPool<MAXTHREADS>& _pool;
  };
#endif

  // Multi-producer Single-consumer function stack, useful for pushing updates to another thread (like an audio thread)
  /*class cFunctionStack : protected cArrayCircular<void*>
  {
    typedef void(*FUNC)(void*,std::atomic<size_t>*);
    typedef cArrayCircular<void*> BASE;

  public:
    template<typename ...Args>
    BSS_FORCEINLINE void BSS_FASTCALL Push(void(*f)(Args...), Args... args) 
    { 
      typedef StoredFunction<void, Args...> SFUNC;
      size_t len = 1+(T_NEXTMULTIPLE(sizeof(SFUNC),3)>>2);
      for(;;)
      {
        size_T index=_head.fetch_add(len);
        
        for(;;)
        {
        while(_resize.load()); // if this is true we're in a resize so wait
        _ref.fetch_add(1); // register a reference
        if(index+len>=_tail.load()+atomic_load(_size)) { // if we hit our tail, resize
          if(_resize.exchange(true)) { // attempt to grab the resize
            _ref.fetch_add(-1);
            continue; // someone else got it first, so we're punted back to the beginning
          }
          _ref.fetch_add(-1);
          while(_flag.test_and_set()); // Wait for any consume operation to terminate
          while(_ref.load()>0) // Wait for all pending references to finish
          if(index+len>=_tail.load()+atomic_load(_size)) { // verify we still actually need to resize to account for various race conditions
            _expand();
            // atomically set _size down here after all operations are complete
          }
          _resize.store(false);
        }
        break;
        }

        if(index+len>_size) { // If we run over the edge of the actual array, abandon our attempt by zeroing the function and restarting
          _array[index]=(FUNC)-1;
          continue;
        }
        new (_array+index+1) SFUNC(f, args...);
        atomic_store(&_array[index],&_helper<SFUNC>);
        break;
      }
    }
    BSS_FORCEINLINE bool BSS_FASTCALL Pop() {
      while(_flag.test_and_set()); // keeps us from doing anything while resizing the array
      if(!_array[_tail%_size])
        return false;
      if(_array[_tail]==-1) {// If the function is -1, it means we ran out of space at the end, so skip to the beginning
        _tail.fetch_add(); // add enough to equal the modulo
        if(!_array[_tail%_size]) // check once again to see if we've read everything
          return false;
      } // _Size must be a power of two for this to work
      size_t sz;
      (*(FUNC)_array[_tail%_size])(_array+((_tail+1)%_size), &sz);

      memset(_array+old%size,0,_tail-old); // set to 0 
      tail->fetch_add(sizeof(T)+sizeof(T*)); //increment tail
      _flag.clear();
      return true;
    }
    BSS_FORCEINLINE bool Empty() { return !_length; }
    //BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    //BSS_FORCEINLINE size_t Capacity() const { return BASE::Capacity(); }
    BSS_FORCEINLINE size_t Length() const { return _length; }

  protected:
    template<typename T>
    static inline void _helper(void* p, size_t* sz) { ((T*)p)->Call(); *sz=sizeof(T)+sizeof(T*); }
    
    BSS_ALIGN(64) std::atomic<size_t> _tail; // tail of the queue (read pointer)
    BSS_ALIGN(64) std::atomic<size_t> _head; // head of the queue (write pointer)
    BSS_ALIGN(64) std::atomic<size_t> _ref; // Number of actions pending before we can resize;
    std::atomic_flag _flag;
    std::atomic_bool _resize;
  };*/
}

#endif