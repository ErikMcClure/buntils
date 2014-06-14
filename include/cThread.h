// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_THREAD_H__BSS__
#define __C_THREAD_H__BSS__

#ifdef BSS_NOSTDTHREADS
#error "You can't use cThread with this compiler"
#endif

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
    inline cThread(const cThread&) BSS_DELETEFUNC
    cThread& operator=(const cThread&) BSS_DELETEFUNCOP
  public:
    template<class _Fn, class... _Args>
    explicit cThread(_Fn&& _Fx, _Args&&... _Ax) : std::thread(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...) {
#ifdef BSS_PLATFORM_POSIX
      struct sigaction action;

      action.sa_handler=___catcher;
      action.sa_flags=0;
      sigfillset(&action.sa_mask);
      sigaction(SIGUSR1, &action, NULL);
#endif
    }
    inline cThread(cThread&& mov) : std::thread(std::move((std::thread&&)mov)) {} // Move constructor only
    inline cThread() {}
    inline ~cThread() { if(joinable()) std::thread::join(); } // Ensures we don't crash if the thread hasn't been joined or detached yet.
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
        std::thread::join();
#else // BSS_PLATFORM_POSIX
        struct timespec ts;
        if(!mstimeout || clock_gettime(CLOCK_REALTIME, &ts) == -1) {
          if(pthread_tryjoin_np(native_handle(), (void**)&ret)!=0) // If failed, thread is either still busy or something blew up, so return -1
            return (size_t)-1;
        } else {
          ts.tv_sec += mstimeout/1000;
          ts.tv_nsec += (mstimeout%1000)*1000000;
          if(pthread_timedjoin_np(native_handle(), (void**)&ret, &ts)!=0) // size_t is defined as being big enough to hold a pointer
            return (size_t)-1;
        }
        *((std::thread::id*)(this))=std::thread::id();// Insanely horrible hack to manually make the ID not joinable
#endif
      }

      return ret;
    }
    BSS_FORCEINLINE void join() { std::thread::join(); }

    cThread& operator=(cThread&& mov) { std::thread::operator=(std::move((std::thread&&)mov)); return *this; }

#ifdef BSS_PLATFORM_WIN32
    BSS_FORCEINLINE void Signal() { DWORD r=QueueUserAPC(&cThread::_APCactivate, native_handle(), 0); assert(r); }
    BSS_FORCEINLINE static void Wait() { SleepEx(INFINITE, true); }

  protected:
    inline static void BSS_COMPILER_STDCALL _APCactivate(ULONG_PTR) {}
#else
    BSS_FORCEINLINE void Signal() { pthread_kill(native_handle(), SIGUSR1); }
    inline static int Wait() {
      sigset_t sigset;
      sigemptyset(&sigset);
      pselect(0, NULL, NULL, NULL, NULL, &sigset);
    }

  protected:
    static void ___catcher(int sig) {}
#endif
  };
}

#endif
