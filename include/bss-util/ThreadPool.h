// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_THREAD_POOL_H__BSS__
#define __C_THREAD_POOL_H__BSS__

#include "bss-util/Thread.h"
#include "bss-util/LocklessQueue.h"
#include "bss-util/bss_alloc_ring.h"
#include "bss-util/Array.h"
#include "Delegate.h"

namespace bss {
  // Stores a pool of threads that execute tasks.
  class ThreadPool
  {
    typedef void(*FN)(void*);
    typedef std::pair<FN, void*> TASK;

    ThreadPool(const ThreadPool&) BSS_DELETEFUNC
      ThreadPool& operator=(const ThreadPool&) BSS_DELETEFUNCOP

  public:
    ThreadPool(ThreadPool&& mov) : _falloc(std::move(mov._falloc)), _run(mov._run.load(std::memory_order_relaxed)),
      _tasks(mov._tasks.load(std::memory_order_relaxed)), _inactive(mov._inactive.load(std::memory_order_relaxed)),
      _tasklist(std::move(mov._tasklist)), _threads(std::move(mov._threads))
    {
      mov._inactive.store(0, std::memory_order_release);
      mov._run.store(0, std::memory_order_release);
    }
    explicit ThreadPool(uint32_t count) : _falloc(sizeof(TASK) * 20), _run(0), _inactive(0), _tasks(0)
    {
      AddThreads(count);
    }
    ThreadPool() : _falloc(sizeof(TASK) * 20), _run(0), _inactive(0), _tasks(0)
    {
      AddThreads(IdealWorkerCount());
    }
    ~ThreadPool()
    {
      Wait();
      _run.store(-_run.load(std::memory_order_acquire), std::memory_order_release); // Negate the stop count, then wait for it to reach 0
      while(_run.load(std::memory_order_acquire) != 0) // While waiting, repeatedly signal threads to prevent race conditions
        _signalThreads(_threads.Capacity());
    }
    void AddTask(FN f, void* arg, uint32_t instances = 1)
    {
      if(!instances) instances = (uint32_t)_threads.Capacity();
      TASK task(f, arg);
      _tasks.fetch_add(instances, std::memory_order_release);
      for(uint32_t i = 0; i < instances; ++i)
        _tasklist.Push(task);

      _signalThreads(_tasklist.Length());
    }

#ifdef BSS_VARIADIC_TEMPLATES
    template<typename R, typename ...Args>
    void AddFunc(R(*f)(Args...), Args... args)
    {
      std::pair<StoreFunction<R, Args...>, RingAllocVoid*>* fn = _falloc.allocT<std::pair<StoreFunction<R, Args...>, RingAllocVoid*>>();
      new (&fn->first) StoreFunction<R, Args...>(f, std::forward<Args>(args)...);
      fn->second = &_falloc; // This could just be a pointer to this thread pool, but it's easier if it's a direct pointer to the allocator we need.
      AddTask(_callfn<R, Args...>, fn);
    }
#endif

    void AddThreads(uint32_t num = 1)
    {
      for(uint32_t i = 0; i < num; ++i)
      {
        _run.fetch_add(1, std::memory_order_release);
        std::unique_ptr<std::pair<Thread, std::atomic_bool>> t((std::pair<Thread, std::atomic_bool>*)calloc(1, sizeof(std::pair<Thread, std::atomic_bool>)));
        t->second.store(false, std::memory_order_release);
        new (&t->first) Thread(_worker, t.get(), std::ref(*this));
        _threads.Add(std::move(t));
      }
    }
    void Wait()
    {
      TASK task; // It is absolutely crucial that the main thread also process tasks to avoid the issue of orphaned tasks
      while(_run.load(std::memory_order_acquire) > 0 && _tasklist.Pop(task))
      {
        (*task.first)(task.second);
        _tasks.fetch_sub(1, std::memory_order_release);
      }
      while(_tasks.load(std::memory_order_relaxed) > 0); // Wait until all tasks actually stop processing
    }
    inline uint32_t Busy() const { return _tasks.load(std::memory_order_relaxed); }

    static unsigned int IdealWorkerCount()
    {
      unsigned int c = std::thread::hardware_concurrency();
      return c <= 1 ? 1 : (c - 1);
    }

  protected:
    void _signalThreads(uint32_t count)
    {
      if(_inactive.load(std::memory_order_acquire) > 0)
      {
        for(uint32_t i = 0; count > 0 && i < _threads.Capacity(); ++i)
        {
          if(_threads[i]->second.load(std::memory_order_acquire))
          {
            _threads[i]->first.Signal();
            --count;
          }
        }
      }
    }

    static void _worker(std::pair<Thread, std::atomic_bool>* job, ThreadPool& pool)
    {
      job->second.store(false, std::memory_order_release);

      while(pool._run.load(std::memory_order_acquire) > 0)
      {
        TASK task;
        while(pool._tasklist.Pop(task))
        {
          (*task.first)(task.second);
          pool._tasks.fetch_sub(1, std::memory_order_release);
        }

        job->second.store(true, std::memory_order_release);
        pool._inactive.fetch_add(1, std::memory_order_release);
        Thread::Wait();
        pool._inactive.fetch_sub(1, std::memory_order_release);
        job->second.store(false, std::memory_order_release);
      }

      pool._run.fetch_add(1, std::memory_order_release);
    }

#ifdef BSS_VARIADIC_TEMPLATES
    template<typename R, typename ...Args>
    static void _callfn(void* p)
    {
      std::pair<StoreFunction<R, Args...>, RingAllocVoid*>* fn = (std::pair<StoreFunction<R, Args...>, RingAllocVoid*>*)p;
      fn->first.Call();
      fn->first.~StoreFunction();
      fn->second->dealloc(fn);
    }
#endif

    MicroLockQueue<TASK, uint32_t> _tasklist;
    std::atomic<uint32_t> _tasks; // Count of tasks still being processed (this includes tasks that have been removed from the queue, but haven't finished yet)
    Array<std::unique_ptr<std::pair<Thread, std::atomic_bool>>, uint32_t, CARRAY_MOVE> _threads;
    std::atomic<uint32_t> _inactive;
    std::atomic<int32_t> _run;
    RingAllocVoid _falloc;
  };

  template<typename R, typename ...Args>
  class Future : StoreFunction<R, Args...>
  {
    inline Future(ThreadPool& pool, R(*f)(Args...), Args&&... args) : StoreFunction<R, Args...>(f, std::forward<Args>(args)...) { pool.AddTask(_eval, this); }
    ~Future() { assert(_finished.load(std::memory_order_acquire)); }
    R* Result() { return _finished.load(std::memory_order_acquire) ? &result : 0; }

  protected:
    R result;
    std::atomic_bool _finished;

    static void _eval(void* arg)
    {
      Future<R, Args...>* p = (Future<R, Args...>*)arg;
      p->result = p->Call();
      p->_finished.store(true, std::memory_order_release);
    }
  };
}

#endif