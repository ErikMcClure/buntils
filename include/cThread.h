// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

/*#ifndef __C_THREAD_H__BSS__
#define __C_THREAD_H__BSS__

#include "cMutex.h"
#include "cBitField.h"
#include "functor.h"

namespace std { template<class _Ty1, class _Ty2> struct pair; }

namespace bss_util {
  // This is thread class designed for maximum efficiency
  class BSS_DLLEXPORT cThread : public cLockable, protected cBitField<unsigned int>
  {
  public:
    cThread(const cThread& copy);
    cThread(unsigned int (BSS_COMPILER_STDCALL *funcptr)(void*), bool sync=false, bool sleep=false, unsigned short sleepms=1);
    cThread(const Functor<unsigned int, void*>& funcptr, bool sync=false, bool sleep=false, unsigned short sleepms=1);
    ~cThread();
    bool Start(void* arg); //Starts the thread. Fails if the thread is already running
    unsigned int Join(unsigned int waitms=0); //Blocks until this thread has exited for waitms milliseconds (if this is 0 then it waits forever) (returns the thread's return value)
    unsigned int Stop(); //Tells the thread to stop (this only works if the thread is checking the stop value) - returns thread return value
    //void Sync(); //Blocks until this thread hits the sync section (Only works if utilized by the thread)
    bool ThreadStop(); //Call this from the thread to check if you need to stop
    void ThreadSync(); //Call this from the thread to signal that you have reached a syncronization section. This behaves in a very specific way: it will never block the executing thread, only one that is waiting for a sync to be hit.
    bool ThreadStopAndSync(); //Same as ThreadStop() except it acts as a sync point as well.
    void ThreadExit(); //Call this from the thread right before you exit

    cThread& operator =(const cThread& right);

    typedef std::pair<Functor<unsigned int, void*>*,void*> __DOUBLEARG;

  protected:
    void _sleepsync();
    void _blocksync();
    void _trylock();

    //cBitField<unsigned char> _bools; //Boolean values are compressed to a single integral value for memory efficiency (0 - stop, 1 - sync, 2 - stoppable, 3 - syncable, 4 - running, 5 - delegate used)
    unsigned short _sleepms;
    size_t _handle;
    union
    {
      unsigned int (BSS_COMPILER_STDCALL *_funcptr)(void*);
      Functor<unsigned int, void*>* _delegate;
    };
    unsigned int _threadret;
    void (cThread::*_syncptr)();
    __DOUBLEARG* _delegate_arg;
  };
}

#endif*/