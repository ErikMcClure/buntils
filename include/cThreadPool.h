// Copyright ©2013 Black Sphere Studios
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
    typedef std::pair<FUNC, void*> PAIR;
    static const int NUMMAXCHARS = T_NEXTMULTIPLE(MAXTHREADS, 31)>>5;

  public:
    cThreadPool(const cThreadPool& copy) = delete;
    cThreadPool(cThreadPool&& mov) : _queue(std::move(mov._queue)), _pool(std::move(mov._pool)) {
      _sleepflag.store(mov._sleepflag.load(std::memory_order_relaxed), std::memory_order_relaxed);
      for(unsigned int i = 0; i < NUMMAXCHARS; ++i)
        _quitflags[i].store(mov._quitflags[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
    }
    explicit cThreadPool(size_t num=4) {
      for(unsigned int i = 0; i < NUMMAXCHARS; ++i)
        _quitflags[i].store(0, std::memory_order_relaxed);
      SetLength(num); 
    }
    ~cThreadPool() { _killall();  }
    inline void Push(FUNC f, void* a) { _queue.Push(PAIR(f, a)); }
    inline void SetLength(size_t num)
    {
      for(int i = num; i < _pool.size(); ++i) // signal deleted threads to exit
        _quitflags[i>>5].store(_quitflags[i>>5].load(std::memory_order_relaxed)|(1<<(i%32)), std::memory_order_relaxed);
      Prime(); // prime in case the threads are asleep
      unsigned int oldsize = _pool.size();
      _pool.resize(num);
      for(unsigned int i = oldsize; i < _pool.size(); ++i) { // Initialize new threads
        _quitflags[i>>5].store(_quitflags[i>>5].load(std::memory_order_relaxed)&(~(1<<(i%32))), std::memory_order_relaxed);
        _pool[i] = cThread(_threadpool_worker, std::ref(*this), std::ref(_quitflags[i>>5]), (1<<(i%32)));
      }
    }
    inline size_t Length() const { return _pool.size(); }
    inline size_t NumTasks() const { return _queue.Length(); }

    inline void Prime()
    {
      _sleepflag.store(0, std::memory_order_release);
      for(unsigned int i = 0; i < _pool.size(); ++i)
        _pool[i].Signal();
    }
    inline void Wait() { _sleepflag.store(1, std::memory_order_release); }
    inline void Join() { while(NumTasks()>0); }

    cThreadPool& operator=(const cThreadPool&) = delete;
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
      cMicroLockQueue<PAIR, char>& queue = pool._queue;
      PAIR cur;
      for(;;)
      {
        if(queue.Pop(cur))
          (*cur.first)(cur.second);
        if(qflags.load(std::memory_order_relaxed)&qmask)
          break;
        if(pool._sleepflag.load(std::memory_order_relaxed)!=0)
          SleepEx(INFINITE, true);
      }
    }

    cMicroLockQueue<PAIR, char> _queue;
    std::vector<cThread> _pool;
    std::atomic<unsigned int> _quitflags[NUMMAXCHARS]; // each thread gets its own bit, which let's us resize the threadpool on the fly, even when things are queue up in it.
    std::atomic<unsigned char> _sleepflag;
    //std::atomic<unsigned int> _timeout;
  };

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
    inline cTaskPool(const cTaskPool&) = delete;
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

  /*class cFunctionStack : protected cArrayCircular<void(*)(void*,int*)>
  {
    typedef void(*FUNC)(void*);
    typedef cArrayCircular<void*> BASE;

  public:
    template<typename ...Args>
    BSS_FORCEINLINE void BSS_FASTCALL Push(void(*f)(Args...), Args... args) 
    { 
      typedef StoredFunction<void, Args...> SFUNC;
      if(_length+1+sizeof(SFUNC)) {
        while(_flag.test_and_set());
        _expand();
        _flag.clear();
      }
      _array[_length]=_helper<SFUNC>;
      new (_array+_length+1) SFUNC(f, args...); 
      _length+=1+sizeof(SFUNC)
    }
    BSS_FORCEINLINE void BSS_FASTCALL Pop() {
      while(_flag.test_and_set()); // keeps us from doing anything while resizing the array
    //if(); // If the function is null, it means we ran out of space at the end, so skip to the beginning
      (*_array[_cur])(_array+_cur+1,&_cur);
      _flag.clear();
    }
    BSS_FORCEINLINE bool Empty() { return !_length; }
    //BSS_FORCEINLINE void Clear() { BASE::Clear(); }
    //BSS_FORCEINLINE size_t Capacity() const { return BASE::Capacity(); }
    BSS_FORCEINLINE size_t Length() const { return _length; }

  protected:
    template<typename T>
    static inline void _helper(void* p, int* cur) { ((T*)p)->Call(); cur+=sizeof(T); }

    std::atomic_flag _flag;
  };*/
}

#endif