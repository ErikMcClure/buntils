// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_THREAD_POOL_H__BSS__
#define __C_THREAD_POOL_H__BSS__

#include "cThread.h"
#include "delegate.h"
#include "cLocklessQueue.h"

namespace bss_util
{
  // Manages a pool of worker threads
  template<unsigned int MAXTHREADS=32>
  class cThreadPool
  {
    typedef void(*FUNC)(void*);
    typedef std::pair<FUNC, void*> PAIR;

  public:
    cThreadPool(const cThreadPool& copy) = delete;
    cThreadPool(cThreadPool&& mov) : _queue(std::move(mov._queue)), _pool(std::move(mov._pool)) {
      _sleepflag.store(mov._sleepflag.load(std::memory_order_relaxed), std::memory_order_relaxed);
      for(unsigned int i = 0; i < NUMMAXCHARS; ++i)
        _quitflags[i].store(mov._quitflags[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
    }
    explicit cThreadPool(size_t num=4) { SetLength(num);  }
    ~cThreadPool() { _killall();  }
    inline void QueueTask(FUNC f, void* a) { _queue.Push(PAIR(f, a)); }
    inline void SetLength(size_t num)
    {
      for(int i = num; i < _pool.size(); ++i) // signal deleted threads to exit
        _quitflags[i>>3].store(_quitflags.load(std::memory_order_relaxed)|(1<<(i%8)), std::memory_order_relaxed);
      Prime(); // prime in case the threads are asleep
      int oldsize = _pool.size();
      _pool.resize(num);
      for(int i = oldsize; i < _pool.size(); ++i) { // Initialize new threads
        _quitflags[i>>3].store(_quitflags.load(std::memory_order_relaxed)&(~(1<<(i%8))), std::memory_order_relaxed);
        _pool[i] = cThread(_threadpool_worker, std::ref(*this), std::ref(_quitflags[i>>3]), (1<<(i%8)));
      }
    }
    inline size_t Length() const { return _pool.size(); }
    inline size_t NumTasks() const { return _queue.Length(); }

    static const int NUMMAXCHARS = T_NEXTMULTIPLE(MAXTHREADS, 31)>>3;
    inline void Prime()
    {
      _sleepflag.store(0, std::memory_order_release);
      for(ST i = 0; i < _pool.size(); ++i)
        _pool[i].Signal();
    }
    inline void Wait() { _sleepflag.store(1, std::memory_order_release); }

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
        _quitflags.store((unsigned int)-1, std::memory_order_relaxed);
      for(ST i = 0; i < _pool.size(); ++i)
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

      return 0;
    }

    cMicroLockQueue<PAIR, char> _queue;
    std::vector<cThread> _pool;
    std::atomic<unsigned int> _quitflags[NUMMAXCHARS]; // each thread gets its own bit, which let's us resize the threadpool on the fly, even when things are queue up in it.
    std::atomic<unsigned char> _sleepflag;
    std::atomic<unsigned int> _timeout;
  };
}

#endif