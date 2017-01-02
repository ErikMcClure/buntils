// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_THREAD_POOL_H__BSS__
#define __C_THREAD_POOL_H__BSS__

#include "cThread.h"
#include "cLocklessQueue.h"
#include "bss_alloc_ring.h"

namespace bss_util
{
  // Stores a pool of threads that execute tasks.
  class cThreadPool
  {
    typedef void(*FUNC)(void*);
    typedef std::pair<FUNC, void*> TASK;
    typedef cMicroLockQueue<cThread*, uint32_t> THREADQUEUE;
    typedef cMicroLockQueue<TASK, uint32_t> TASKQUEUE;

    cThreadPool(const cThreadPool&) BSS_DELETEFUNC
    cThreadPool& operator=(const cThreadPool&) BSS_DELETEFUNCOP

  public:
    cThreadPool(cThreadPool&& mov) : _length(mov._length.load(std::memory_order_relaxed)), _threads(std::move(mov._threads)), _tasklist(std::move(mov._tasklist)),
      _flag(mov._flag.load(std::memory_order_relaxed)), _alloc(std::move(mov._alloc)), _circ(std::move(mov._circ))
    {
      mov._length.store(0, std::memory_order_relaxed);
    }
    explicit cThreadPool(uint32_t count = 4)
    { 
      _length.store(0, std::memory_order_relaxed);
      _flag.store(false, std::memory_order_relaxed);
      AddThreads(count);
    }

    ~cThreadPool()
    {
      Wait();
      _flag.store(true, std::memory_order_relaxed);
      cThread* t = nullptr;
      while(_threads.Length())
      {
        if(_threads.Pop(t))
        {
          t->Signal();
          t->join();
          t->~cThread();
          _alloc.dealloc(t);
        }
      }
    }

    cThreadPool& operator=(cThreadPool&& mov)
    {
      _length.store(mov._length.load(std::memory_order_relaxed), std::memory_order_relaxed);
      _threads = std::move(mov._threads);
      _tasklist = std::move(mov._tasklist);
      _flag.store(mov._flag.load(std::memory_order_relaxed), std::memory_order_relaxed);
      _alloc = std::move(mov._alloc);
      _circ = std::move(mov._circ);
      mov._length.store(0, std::memory_order_relaxed);
      return *this;
    }
    
#ifdef BSS_VARIADIC_TEMPLATES
    template<typename R, typename ...Args>
    void AddFunc(R(*f)(Args...), Args... args)
    {
      STOREFUNC<R, Args...>* sf = (STOREFUNC<R, Args...>*)_circ.alloc(sizeof(STOREFUNC<R, Args...>));
      new (sf) STOREFUNC<R, Args...>(&_circ, f, args...);
      AddTask(_funcunpack<R, Args...>, sf);
    }
#endif

    void AddTask(FUNC f, void* args, uint32_t instances = 1)
    {
      if(!instances) instances = _threads.Length();
      TASK task(f, args);
      for(uint32_t i = 0; i < instances; ++i)
        _tasklist.Push(task);
      
      _signalthreads(instances);
    }
    void AddThreads(uint32_t num=1) { for(uint32_t i = 0; i < num; ++i) _addthread(); }
    void Wait() { while((int)_threads.Length() < _length.load(std::memory_order_relaxed)); }

  protected:
    cThread* _addthread()
    {
      cThread* t = _alloc.alloc(1);
      new (t) cThread(_worker, t, std::ref(*this));
      _length.fetch_add(1, std::memory_order_relaxed);
      return t;
    }
    void _signalthreads(uint32_t instances)
    {
      cThread* t;
      uint32_t i;
      for(i = 0; i < instances; ++i)
      {
        if(!_threads.Pop(t))
          break;
        t->Signal();
      }
    }
#ifdef BSS_VARIADIC_TEMPLATES
    template<typename R, typename ...Args>
    struct STOREFUNC
    {
      inline STOREFUNC(cRingAlloc<char>* _alloc, R(*_f)(Args...), Args... _args) : alloc(_alloc), f(_f), args(_args...) {}
      std::tuple<Args...> args;
      R(*f)(Args...);
      cRingAlloc<char>* alloc;

      inline R call() { return _unpack(typename bssSeq_gens<sizeof...(Args)>::type()); }
      template<int ...S> inline R _unpack(bssSeq<S...>) { return f(std::get<S>(args) ...); }
    };

    template<typename R, typename ...Args>
    static void _funcunpack(void* p)
    {
      STOREFUNC<R, Args...>* sf = (STOREFUNC<R, Args...>*)p;
      sf->call();
      sf->alloc->dealloc(sf); // Destroy storage
    }
#endif

    static void _worker(cThread* job, cThreadPool& pool)
    {
      THREADQUEUE& queue = pool._threads;
      TASKQUEUE& tasklist = pool._tasklist;
      
      while(!pool._flag.load(std::memory_order_relaxed))
      {
        TASK task;
        while(tasklist.Pop(task))
          (*task.first)(task.second);
        queue.Push(job);
        if(tasklist.Peek()) // This addresses a race condition where a task was added before this thread could signal that it was inactive.
          pool._signalthreads(1); // If this happens, signal another thread. If we were the only thread we will signal ourselves which will cause us to wake up immediately.
        cThread::Wait(); // TODO: The if statement here does not have a race condition in windows because of how Signal() works, but it might in Linux.
      }
    }

    std::atomic_int _length;
    THREADQUEUE _threads;
    TASKQUEUE _tasklist;
    std::atomic_bool _flag; // Master shutoff flag
    cLocklessBlockAlloc<cThread> _alloc; // Allocator for thread objects
    cRingAlloc<char> _circ; // Allocator for functions.
  };

  /*
  template<typename T>
  class cThreadJob
  {
    typedef void(*FUNC)(const T&);
  public:
    cThreadJob(cThreadPool& p, FUNC f, cMicroLockQueue<T>& q, uint32_t instances = 1) : _f(f), _queue(q), _flag(false)
    {
      p.AddTask(&_func, this, instances);
      while(_active.load(std::memory_order_relaxed) < instances); // If instances equals 0 we can't gaurantee that we'll get anything so this won't block.
    }
    ~cThreadJob() { Finish(); }
    void Finish()
    {
      _flag.store(true, std::memory_order_relaxed);
      while(_active.load(std::memory_order_relaxed));
    }
  protected:
    static void _func(void* p)
    {
      cThreadJob<T>* job = (cThreadJob<T>*)p;
      job->_active.fetch_add(1, std::memory_order_relaxed);
      FUNC f = job->_f;
      cMicroLockQueue<T>& q = job->_queue;

      T value;
      while(!job->_flag.load(std::memory_order_relaxed))
      {
        while(q.Pop(value))
          (*f)(value);
      }

      job->_active.fetch_add(-1, std::memory_order_relaxed);
    }

    FUNC _f;
    cMicroLockQueue<T>& _queue;
    std::atomic_bool _flag; // Master shutoff flag
    std::atomic_int _active; // Number of active threads.
  };*/
}

#endif