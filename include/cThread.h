// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_THREAD_H__BSS__
#define __C_THREAD_H__BSS__

#include "bss_defines.h"
#include <assert.h>
#include <thread>
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#include <process.h>
#else // Assume BSS_PLATFORM_POSIX
#include <pthread.h>
#include <signal.h>
#endif

namespace bss_util {
  // This extends std::thread and adds support for joining with a timeout and signaled state
  class BSS_COMPILER_DLLEXPORT cThread : public std::thread
  {
  public:
    template<class _Fn, class... _Args>
    explicit cThread(_Fn&& _Fx, _Args&&... _Ax) : std::thread(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...) {}
    inline cThread(cThread&& mov) : std::thread(std::move((std::thread&&)mov)) {} // Move constructor only
    inline cThread(const cThread&) = delete;
    inline cThread() {}
    inline ~cThread() { }
    // Blocks until either the thread has terminated, or 'timeout' milliseconds have elapsed. If a timeout occurs, returns -1.
    BSS_FORCEINLINE size_t join(size_t mstimeout)
    {
      size_t ret=(size_t)-1;
      if(joinable())
      {
#ifdef BSS_PLATFORM_WIN32
        if(WaitForSingleObject((HANDLE)native_handle(), (DWORD)mstimeout)!=0)
          return (size_t)-1;
        GetExitCodeThread((HANDLE)native_handle(), (DWORD*)&ret); // size_t is gaurenteed to be big enough to hold DWORD
#else // BSS_PLATFORM_POSIX
        struct timespec ts;
        if(!mstimeout || clock_gettime(CLOCK_REALTIME, &ts) == -1) {
          if(pthread_tryjoin_np(native_handle(), (void**)&ret)!=0) // If failed, thread is either still busy or something blew up, so return -1
            return (size_t)-1;
        } else {
          ts.tv_sec += mstimeout/1000;
          ts.tv_nsec += (mstimeout%1000)*1000000;
          if(pthread_timedjoin_np(native_handle(), (void**)&ret, &ts)!=0) // size_t is gaurenteed to be big enough to hold a pointer
            return (size_t)-1;
        }
#endif
      }
      
      std::thread::join();
      return ret;
    }
    BSS_FORCEINLINE void join() { std::thread::join(); }

    cThread& operator=(const cThread&) = delete;
    cThread& operator=(cThread&& mov) { std::thread::operator=(std::move((std::thread&&)mov)); return *this; }

#ifdef BSS_PLATFORM_WIN32
    BSS_FORCEINLINE void Signal() { DWORD r=QueueUserAPC(&cThread::_APCactivate, native_handle(), 0); assert(r); }
    BSS_FORCEINLINE static void Wait() { SleepEx(INFINITE, true); }

  protected:
    inline static void BSS_COMPILER_STDCALL _APCactivate(ULONG_PTR) {}
#else
    BSS_FORCEINLINE void Signal() { raise(SIGUSR2); }
    inline static int Wait() {
      struct timespec ts;
      sigset_t set;
		  ts.tv_sec=1;
		  ts.tv_nsec=0;
		  sigemptyset(&set);
		  return pselect(0,NULL,NULL,NULL,&ts,&set);
    }
#endif
  };
}

#endif
