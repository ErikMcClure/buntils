// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_THREAD_H__BSS__
#define __C_THREAD_H__BSS__

#include "bss_dlldef.h"
#ifdef BSS_PLATFORM_WIN32
#include "bss_win32_includes.h"
#include <process.h>
#else // Assume BSS_PLATFORM_POSIX
#include <pthread.h>
#include <signal.h>
#endif

namespace bss_util {
  // This wraps the creation of a thread and stores a handle to it. Everything must be inlined so the thread is created in the correct dll
  class BSS_COMPILER_DLLEXPORT cThread
  {
#ifdef BSS_PLATFORM_WIN32
    typedef unsigned int (__stdcall *FUNC)(void*);
#else // Assume BSS_PLATFORM_POSIX
    typedef void* (*FUNC)(void*);
#endif

  public:
    // Move constructor only
    inline cThread(cThread&& mov) : _id(mov._id) { mov._id=(size_t)-1; }
    // Create thread
    explicit inline cThread(FUNC f, void* arg=0) { _start(f,arg); }
    inline cThread() : _id((size_t)-1) {}
    inline ~cThread() { Join(); }
    // Blocks until this thread has terminated, and returns the threads exit code
    BSS_FORCEINLINE size_t Join()
    {
      size_t ret=(size_t)-1;
      if(_id!=(size_t)-1)
      {
#ifdef BSS_PLATFORM_WIN32
        if(WaitForSingleObject((HANDLE)_id,INFINITE)!=0)
          return (size_t)-1;
        GetExitCodeThread((HANDLE)_id,(DWORD*)&ret); // size_t is gaurenteed to be big enough to hold DWORD
#else // BSS_PLATFORM_POSIX
        if(pthread_join(_id,(void**)&ret)!=0) // size_t is gaurenteed to be big enough to hold a pointer
          return (size_t)-1;
#endif
      }

      _id=(size_t)-1;
      return ret;
    }
    // Blocks until either the thread has terminated, or 'timeout' milliseconds have elapsed. If a timeout occurs, returns -1.
    BSS_FORCEINLINE size_t Join(size_t mstimeout)
    {
      size_t ret=(size_t)-1;
      if(_id!=(size_t)-1)
      {
#ifdef BSS_PLATFORM_WIN32
        if(WaitForSingleObject((HANDLE)_id,mstimeout)!=0)
          return (size_t)-1;
        GetExitCodeThread((HANDLE)_id,(DWORD*)&ret); // size_t is gaurenteed to be big enough to hold DWORD
#else // BSS_PLATFORM_POSIX
        struct timespec ts;
        if(!mstimeout || clock_gettime(CLOCK_REALTIME, &ts) == -1) {
          if(pthread_tryjoin_np(_id,(void**)&ret)!=0) // If failed, thread is either still busy or something blew up, so return -1
            return (size_t)-1;
        } else {
          ts.tv_sec += mstimeout/1000;
          ts.tv_nsec += (mstimeout%1000)*1000000;
          if(pthread_timedjoin_np(_id,(void**)&ret, &ts)!=0) // size_t is gaurenteed to be big enough to hold a pointer
            return (size_t)-1;
        }
#endif
      }

      _id=(size_t)-1;
      return ret;
    }

    // If this thread object does not currently have a valid thread, attempts to start one. Returns 0 on success, -1 if thread creation
    // failed, or -2 if an existing thread is still running.
    inline char BSS_FASTCALL Start(FUNC f, void* arg=0)
    {
      if(_id!=(size_t)-1) return -2;
      _start(f,arg);
      return (_id==(size_t)-1)?-1:0;
    }
    // Gets the thread's id
#ifdef BSS_PLATFORM_WIN32
    inline HANDLE GetID() const { return (HANDLE)_id; }
    BSS_FORCEINLINE void SendSignal() const { DWORD r=QueueUserAPC(&cThread::_APCactivate,GetID(),0); assert(r); }
#else
    inline pthread_t GetID() const { return _id; }
    BSS_FORCEINLINE void SendSignal() const { raise(SIGUSR2); }
#endif

    inline cThread& operator=(cThread&& mov) { _id=mov._id; mov._id=(size_t)-1; return *this; }
#ifdef BSS_PLATFORM_WIN32
    inline static void BSS_COMPILER_STDCALL _APCactivate(unsigned long) {}
    BSS_FORCEINLINE static void SignalWait() { SleepEx(INFINITE,true); }
#else
    inline static int SignalWait() {
      struct timespec ts;
      sigset_t mask;
		  ts.tv_sec=1;
		  ts.tv_nsec=0;
		  sigemptyset(&set);
		  return pselect(0,NULL,NULL,NULL,&ts,&set);
    }
#endif

  protected:
    inline cThread(const cThread&) {} // No copying
    inline cThread& operator=(const cThread&) { return *this; } // No copy assignment either
    BSS_FORCEINLINE void BSS_FASTCALL _start(FUNC f, void* arg=0)
    {
#ifdef BSS_PLATFORM_WIN32
      _id=_beginthreadex(0,0,f,arg,0,0);
#else // BSS_PLATFORM_POSIX
      if(pthread_create(&_id,0,f,arg)!=0)
        _id=(size_t)-1;
#endif
    }

    size_t _id;
  };
}

#endif
