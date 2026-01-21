// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __THREAD_H__BUN__
#define __THREAD_H__BUN__

#include "defines.h"
#include <assert.h>
#include <thread>
#ifdef BUN_PLATFORM_WIN32
#include "win32_includes.h"
#include <process.h>
#else // Assume BUN_PLATFORM_POSIX
#include <pthread.h>
#include <semaphore.h>
#endif

namespace bun {
#pragma warning(push)
#pragma warning(disable:4275)
  // Cross-platform implementation of a semaphore, initialized to 0 (locked).
  class Semaphore
  {
  public:
#ifdef BUN_PLATFORM_WIN32
    inline Semaphore(Semaphore&& mov) : _sem(mov._sem) { mov._sem = NULL; }
    inline Semaphore() : _sem(CreateSemaphore(NULL, 0, 65535, NULL)) {}
    inline ~Semaphore() { if(_sem != NULL) CloseHandle(_sem); }
    // Unlocks the semaphore by incrementing the current value
    inline bool Notify(size_t count = 1)
    { 
      LONG prev;
      return ReleaseSemaphore(_sem, (LONG)count, &prev) != 0;
    }
    // Waits on the semaphore by waiting until it can decrement the value by 1 (which requires the semaphore is greater than zero).
    inline bool Wait() { return WaitForSingleObject(_sem, INFINITE) != WAIT_FAILED; }

  protected:
    HANDLE _sem;
#else
    inline Semaphore(Semaphore&& mov) : _sem(mov._sem) { sem_init(&mov._sem, 0, 0); }
    inline Semaphore() { sem_init(&_sem, 0, 0); }
    inline ~Semaphore() { }
    inline bool Notify(size_t count = 1)
    { 
      for(size_t i = 0; i < count; ++i)
        if(sem_post(&_sem))
          return false;
      return true; 
    }
    inline bool Wait() { return !sem_wait(&_sem); }

  protected:
    sem_t _sem;
#endif
  };
  // This extends std::thread and adds support for joining with a timeout
  class BUN_COMPILER_DLLEXPORT Thread : public std::thread
  {
#pragma warning(pop)
    inline Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
  public:
    template<class _Fn, class... _Args>
    explicit Thread(_Fn&& _Fx, _Args&&... _Ax) : std::thread(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...) {}
    inline Thread(Thread&& mov) : std::thread(std::move((std::thread&&)mov)) { } // Move constructor only
    inline Thread() { }
    inline ~Thread()
    { 
      if(joinable()) // Ensures we don't crash if the thread hasn't been joined or detached yet.
        std::thread::join();
    } 
    // Blocks until either the thread has terminated, or 'timeout' milliseconds have elapsed. If a timeout occurs, returns -1.
    BUN_FORCEINLINE size_t join(size_t mstimeout)
    {
      size_t ret = (size_t)~0;
      if(joinable())
      {
#ifdef BUN_PLATFORM_WIN32
        if(WaitForSingleObject((HANDLE)native_handle(), (DWORD)mstimeout) != 0)
          return (size_t)~0;
        GetExitCodeThread((HANDLE)native_handle(), (DWORD*)&ret); // size_t is gaurenteed to be big enough to hold DWORD
        std::thread::join();
#else // BUN_PLATFORM_POSIX
        struct timespec ts;

        if(!mstimeout || clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
          if(pthread_tryjoin_np(native_handle(), (void**)&ret) != 0) // If failed, thread is either still busy or something blew up, so return -1
            return (size_t)~0;
        }
        else
        {
          ts.tv_sec += mstimeout / 1000;
          ts.tv_nsec += (mstimeout % 1000) * 1000000;
          if(pthread_timedjoin_np(native_handle(), (void**)&ret, &ts) != 0) // size_t is defined as being big enough to hold a pointer
            return (size_t)~0;
        }

        *((std::thread::id*)(this)) = std::thread::id();// Insanely horrible hack to manually make the ID not joinable
#endif
      }

      return ret;
    }
    BUN_FORCEINLINE void join() { std::thread::join(); }

    Thread& operator=(Thread&& mov) { std::thread::operator=(std::move((std::thread&&)mov)); return *this; }
  };
}

#endif
