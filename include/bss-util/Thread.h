// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __THREAD_H__BSS__
#define __THREAD_H__BSS__

#include "defines.h"
#include <assert.h>
#ifndef BSS_COMPILER_MSC2010
#include <thread>
#else
#include <functional>
#endif
#ifdef BSS_PLATFORM_WIN32
#include "win32_includes.h"
#include <process.h>
#else // Assume BSS_PLATFORM_POSIX
#include <pthread.h>
#include <semaphore.h>
#endif

namespace bss {
#ifndef BSS_COMPILER_MSC2010
#pragma warning(push)
#pragma warning(disable:4275)
  // Cross-platform implementation of a semaphore, initialized to 0 (locked).
  class Semaphore
  {
  public:
#ifdef BSS_PLATFORM_WIN32
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
  class BSS_COMPILER_DLLEXPORT Thread : public std::thread
  {
#pragma warning(pop)
    inline Thread(const Thread&) BSS_DELETEFUNC
    Thread& operator=(const Thread&) BSS_DELETEFUNCOP
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
    BSS_FORCEINLINE size_t join(size_t mstimeout)
    {
      size_t ret = (size_t)-1;
      if(joinable())
      {
#ifdef BSS_PLATFORM_WIN32
        if(WaitForSingleObject((HANDLE)native_handle(), (DWORD)mstimeout) != 0)
          return (size_t)-1;
        GetExitCodeThread((HANDLE)native_handle(), (DWORD*)&ret); // size_t is gaurenteed to be big enough to hold DWORD
        std::thread::join();
#else // BSS_PLATFORM_POSIX
        struct timespec ts;

        if(!mstimeout || clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
          if(pthread_tryjoin_np(native_handle(), (void**)&ret) != 0) // If failed, thread is either still busy or something blew up, so return -1
            return (size_t)-1;
        }
        else
        {
          ts.tv_sec += mstimeout / 1000;
          ts.tv_nsec += (mstimeout % 1000) * 1000000;
          if(pthread_timedjoin_np(native_handle(), (void**)&ret, &ts) != 0) // size_t is defined as being big enough to hold a pointer
            return (size_t)-1;
        }

        *((std::thread::id*)(this)) = std::thread::id();// Insanely horrible hack to manually make the ID not joinable
#endif
      }

      return ret;
    }
    BSS_FORCEINLINE void join() { std::thread::join(); }

    Thread& operator=(Thread&& mov) { std::thread::operator=(std::move((std::thread&&)mov)); return *this; }
  };
#else // This is for VS2010 which means we only implement this manually for the win32 platform.
  class BSS_COMPILER_DLLEXPORT Thread
  {
    inline Thread(const Thread&) {}
    Thread& operator=(const Thread&) { return *this; }

  public:
    inline Thread(Thread&& mov) : _id(mov._id) { mov._id = (size_t)-1; }
    template<class _Fn> explicit Thread(_Fn&& _Fx) { std::function<void(void)> fn = [&]() { _started.clear(); _Fx(); }; _start(&fn); }
    template<class _Fn, class A1> Thread(_Fn&& _Fx, A1&& a1) { std::function<void(void)> fn = [&]() { _started.clear(); _Fx(std::forward<A1>(a1)); }; _start(&fn); }
    template<class _Fn, class A1, class A2> Thread(_Fn&& _Fx, A1&& a1, A2&& a2) { std::function<void(void)> fn = [&]() { _started.clear(); _Fx(std::forward<A1>(a1), std::forward<A2>(a2)); }; _start(&fn); }
    template<class _Fn, class A1, class A2, class A3> Thread(_Fn&& _Fx, A1&& a1, A2&& a2, A3&& a3) { std::function<void(void)> fn = [&]() { _started.clear(); _Fx(std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)); }; _start(&fn); }
    template<class _Fn, class A1, class A2, class A3, class A4> Thread(_Fn&& _Fx, A1&& a1, A2&& a2, A3&& a3, A4&& a4) { std::function<void(void)> fn = [&]() { _started.clear(); _Fx(std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4)); }; _start(&fn); }
    template<class _Fn, class A1, class A2, class A3, class A4, class A5> Thread(_Fn&& _Fx, A1&& a1, A2 &&a2, A3&& a3, A4&& a4, A5&& a5) { std::function<void(void)> fn = [&]() { _started.clear(); _Fx(std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5)); }; _start(&fn); }
    inline Thread() : _id((size_t)-1) {}
    inline ~Thread() { if(joinable()) join(); }
    inline bool joinable() const { return _id != (size_t)-1; }
    inline void detach() { _id = (size_t)-1; }
    inline void swap(Thread& other) { size_t i = _id; _id = other._id; other._id = i; }
    inline size_t native_handle() const { return _id; }

    // Blocks until this thread has terminated, and returns the threads exit code
    BSS_FORCEINLINE size_t join()
    {
      size_t ret = (size_t)-1;
      if(joinable())
      {
        if(WaitForSingleObject((HANDLE)native_handle(), INFINITE) != 0)
          return (size_t)-1;
        GetExitCodeThread((HANDLE)native_handle(), (DWORD*)&ret); // size_t is gaurenteed to be big enough to hold DWORD
      }

      _id = (size_t)-1;
      return ret;
    }
    // Blocks until either the thread has terminated, or 'timeout' milliseconds have elapsed. If a timeout occurs, returns -1.
    BSS_FORCEINLINE size_t join(size_t mstimeout)
    {
      size_t ret = (size_t)-1;
      if(joinable())
      {
        if(WaitForSingleObject((HANDLE)native_handle(), (DWORD)mstimeout) != 0)
          return (size_t)-1;
        GetExitCodeThread((HANDLE)native_handle(), (DWORD*)&ret); // size_t is gaurenteed to be big enough to hold DWORD
      }
      _id = (size_t)-1;
      return ret;
    }

    inline Thread& operator=(Thread&& mov) { _id = mov._id; mov._id = (size_t)-1; return *this; }

  protected:
    inline static size_t __stdcall _threadwrap(void* arg) { std::function<void(void)> fn = *(std::function<void(void)>*)arg; fn(); return 0; }
    BSS_FORCEINLINE void _start(void* arg)
    {
      _started.test_and_set();
      _id = _beginthreadex(0, 0, _threadwrap, arg, 0, 0);
      if(!_id) _id = (size_t)-1; // WHAT GODDAMN MANIAC LET _beginthreadex RETURN 0 ON HALF ITS ERRORS?! 0 ISN'T EVEN THE INVALID HANDLE VALUE!
      while(_started.test_and_set()); // Spin until we've actually copied the function in our wrapper thread so we don't nuke the stack.
    }

    size_t _id;
    std::atomic_flag _started;
  };
#endif
}

#endif
