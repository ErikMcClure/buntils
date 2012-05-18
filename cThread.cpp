// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#include "cThread.h"
#include "bss_util.h"
#include "bss_win32_includes.h"
#include <process.h>
#include <utility>
#include "lockless.h"

using namespace bss_util;

cThread::cThread(const cThread& copy) : _funcptr(copy._funcptr), _sleepms(copy._sleepms), cBitField<unsigned int>((copy./*_bools.*/GetBits()&(~(1 << 4)))),
_threadret(copy._threadret), cLockable(copy), _handle(-1L), _syncptr(copy._syncptr), _delegate_arg(new __DOUBLEARG((Functor<unsigned int, void*>*)0,(void*)0))
{
}

cThread::cThread(unsigned int (BSS_COMPILER_STDCALL *funcptr)(void*), bool sync, bool sleep, unsigned short sleepms, unsigned long flips) :
  _funcptr(funcptr), _sleepms(sleepms), cBitField<unsigned int>((sync?(1 << 3):0)), _threadret(0),
    _handle(-1L), _syncptr(sleep?&cThread::_sleepsync:&cThread::_blocksync), _delegate_arg(new __DOUBLEARG())
{
}
cThread::cThread(const Functor<unsigned int, void*>& funcptr, bool sync, bool sleep, unsigned short sleepms, unsigned long flips) : 
_delegate(funcptr.Clone()), _sleepms(sleepms), cBitField<unsigned int>((sync?(1 << 3):0)|(1 << 5)), _threadret(0),
  _handle(-1L), _syncptr(sleep?&cThread::_sleepsync:&cThread::_blocksync), _delegate_arg(new __DOUBLEARG())
{
}

cThread::~cThread()
{
  Stop();
  delete _delegate_arg;
  if(/*_bools.*/GetBit(1<<5))
    delete _delegate;
}

unsigned int BSS_COMPILER_STDCALL __threadstartdelegate(void* ptr)
{
  return ((cThread::__DOUBLEARG*)ptr)->first->Call(((cThread::__DOUBLEARG*)ptr)->second);
}

bool cThread::Start(void* arg)
{
  if(/*_bools.*/GetBit((1<<4)))
    return false;

  /*_bools.*/RemoveBit((1<<0)); //Ensure that these don't carry over from something else
  /*_bools.*/AddBit((1<<4)); //this is done up here so we don't accidentally create thread sync issues

  if(/*_bools.*/GetBit(1<<5)) { //if this is true its a functor instead of a function pointer
    _delegate_arg->first=_delegate;
    _delegate_arg->second=arg;
    _handle=_beginthreadex(0,0, &__threadstartdelegate, _delegate_arg, 0, &_threadret);
  } else
    _handle=_beginthreadex(0,0, _funcptr, arg, 0, &_threadret);

  if(_handle != -1L)
    return true;

  /*_bools.*/RemoveBit((1<<4));
  return false;
}
unsigned int cThread::Join(unsigned int waitms)
{
  //Unlock(); //If the thread is locked and we try to join it, DEADLOCK!
  _syncobj.Unravel();
  if(!/*_bools.*/GetBit((1<<4)) || WaitForSingleObject((void*)_handle, !waitms?INFINITE:waitms)==WAIT_FAILED)
    return (unsigned int)-1;
  /*_bools.*/RemoveBit((1<<4));
  return _threadret;
}
unsigned int cThread::Stop()
{
  if(/*!GetBit((1<<2)) ||*/ !/*_bools.*/GetBit((1<<4)))
    return (unsigned int)-1;

  //Unlock(); //Ensure that we unlock the thread before re-locking it or we'll deadlock.
  //_syncobj.Lock();
  //AddBit((1<<0)); //this is done via a lockless technique
  asmcas<unsigned int>(&_bitfield,_bitfield|(1<<0),_bitfield);
  //_syncobj.Unlock();
  unsigned int retval= Join(0);
  if(retval!=-1)
    /*_bools.*/RemoveBit((1<<0));
  return retval;
}
void cThread::_blocksync()
{
  _syncobj.Lock();
}
void cThread::_sleepsync()
{
  while(!_syncobj.TryLock())
    Sleep(_sleepms);
}

bool cThread::ThreadStop()
{
  return /*_bools.*/GetBit((1<<0));
  //if(_syncobj.TryLock())
  //{
  //  bool retval = /*_bools.*/GetBit((1<<0));
  //  _syncobj.Unlock();
  //  return retval;
  //}
  //return false;
}

bool cThread::ThreadStopAndSync()
{
  (this->*_syncptr)();
  bool retval = /*_bools.*/GetBit((1<<0));
  _syncobj.Unlock();
  return retval;
}

void cThread::ThreadSync()
{
  (this->*_syncptr)();
  _syncobj.Unlock();
}
void cThread::ThreadExit()
{
  _syncobj.Lock();
  /*_bools.*/RemoveBit((1<<4));
  _syncobj.Unlock();
}

cThread& cThread::operator =(const bss_util::cThread &right)
{
  Stop();
  //_bools = (right./*_bools.*/GetBits()&(~(1 << 4)));
  _bitfield=right._bitfield;
  _handle=-1L;
  _funcptr=right._funcptr;
  _sleepms=right._sleepms;
  _syncptr=right._syncptr;
  return *this;
}