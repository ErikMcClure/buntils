// Copyright �2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __THREAD_POOL_H__BSS__
#define __THREAD_POOL_H__BSS__

#include "Thread.h"
#include "LocklessQueue.h"
#include "RingAlloc.h"
#include "DynArray.h"
#include "Delegate.h"
#include <condition_variable>
#include <mutex>

namespace bss {
  // Stores a pool of threads that execute tasks.
  class ThreadPool
  {
    typedef void(*FN)(void*);
    using TASK = std::pair<FN, void*>;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

  public:
    ThreadPool(ThreadPool&& mov) : _falloc(std::move(mov._falloc)), _run(mov._run.load(std::memory_order_relaxed)),
      _tasks(mov._tasks.load(std::memory_order_relaxed)),
      _tasklist(std::move(mov._tasklist)), _threads(std::move(mov._threads))
    {
      mov._run.store(0, std::memory_order_release);
    }
    explicit ThreadPool(size_t count) : _falloc(sizeof(TASK) * 20), _run(0), _tasks(0)
    {
      AddThreads(count);
    }
    ThreadPool() : _falloc(sizeof(TASK) * 20), _run(0), _tasks(0)
    {
      AddThreads(IdealWorkerCount());
    }
    ~ThreadPool()
    {
      Wait();
      _run.store(-_run.load(std::memory_order_acquire), std::memory_order_release); // Negate the stop count, then wait for it to reach 0
      _lock.Notify(_threads.Length());
      while(_run.load(std::memory_order_acquire) < 0);
    }
    void AddTask(FN f, void* arg, size_t instances = 1)
    {
      if(!instances)
        instances = (size_t)_threads.Length();

      TASK task(f, arg);
      _tasks.fetch_add(instances, std::memory_order_release);

      for(size_t i = 0; i < instances; ++i)
        _tasklist.Push(task);

      _lock.Notify(instances);
    }

    template<typename R, typename ...Args>
    void AddFunc(R(*f)(Args...), Args... args)
    {
      std::pair<StoreFunction<R, Args...>, RingAllocVoid*>* fn = _falloc.AllocT<std::pair<StoreFunction<R, Args...>, RingAllocVoid*>>();
      new (&fn->first) StoreFunction<R, Args...>(f, std::forward<Args>(args)...);
      fn->second = &_falloc; // This could just be a pointer to this thread pool, but it's easier if it's a direct pointer to the allocator we need.
      AddTask(_callfn<R, Args...>, fn);
    }

    void AddThreads(size_t num = 1)
    {
      for(size_t i = 0; i < num; ++i)
      {
        _run.fetch_add(1, std::memory_order_release);
        _threads.AddConstruct(_worker, std::ref(*this));
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
    inline size_t Busy() const { return _tasks.load(std::memory_order_relaxed); }

    static size_t IdealWorkerCount()
    {
      size_t c = std::thread::hardware_concurrency();
      return c <= 1 ? 1 : (c - 1);
    }

  protected:
    static void _worker(ThreadPool& pool)
    {
      while(pool._run.load(std::memory_order_acquire) > 0)
      {
        pool._lock.Wait();
        TASK task;
        while(pool._tasklist.Pop(task))
        {
          (*task.first)(task.second);
          pool._tasks.fetch_sub(1, std::memory_order_release);
        }
      }

      pool._run.fetch_add(1, std::memory_order_release);
    }

    template<typename R, typename ...Args>
    static void _callfn(void* p)
    {
      std::pair<StoreFunction<R, Args...>, RingAllocVoid*>* fn = (std::pair<StoreFunction<R, Args...>, RingAllocVoid*>*)p;
      fn->first.Call();
      fn->first.~StoreFunction();
      fn->second->Dealloc(fn);
    }

    MicroLockQueue<TASK, size_t> _tasklist;
    std::atomic<size_t> _tasks; // Count of tasks still being processed (this includes tasks that have been removed from the queue, but haven't finished yet)
    DynArray<Thread, size_t, ARRAY_MOVE> _threads;
    std::atomic<int32_t> _run;
    Semaphore _lock;
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