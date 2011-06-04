// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// WINDOWS ONLY

#include "cMutex.h"
#include "bss_win32_includes.h"

using namespace bss_util;

cMutex::cMutex(const char* name, unsigned long delay)
{
  _handle = OpenMutex(0, FALSE, name);
  _default = delay;
}

cMutex::~cMutex()
{
  CloseHandle(_handle);
}

void cMutex::Lock(unsigned long delay) const
{
  if(WaitForSingleObject(_handle, delay)==WAIT_OBJECT_0)
    ++_nlock;
}

bool cMutex::TryLock() const
{
  if(WaitForSingleObject(_handle, 0)==WAIT_OBJECT_0) //If this returns anything else, the lock failed. We don't care if its an error or not
    ++_nlock;
  else
    return false;
  return true;
}

void cMutex::Lock() const
{
  if(WaitForSingleObject(_handle, _default)==WAIT_OBJECT_0)
    ++_nlock;
}

void cMutex::Unlock() const
{
  --_nlock;
  ReleaseMutex(_handle);
}

long cMutex::GetLockCount() const
{
  return _nlock;
}

void cMutex::Unravel() const
{
  while(_nlock>0)
    Unlock();
}

cCritSection::cCritSection(const cCritSection& copy)
{
  _critsec = new CRITICAL_SECTION();
  if(!copy._critsec || !InitializeCriticalSectionAndSpinCount(_critsec, copy._critsec->SpinCount))
  {
    delete _critsec;
    _critsec = 0;
  }
}

cCritSection::cCritSection(unsigned long flip)
{
  _critsec = new CRITICAL_SECTION();
  if(!InitializeCriticalSectionAndSpinCount(_critsec, flip))
  {
    delete _critsec;
    _critsec = 0;
  }
}

cCritSection::~cCritSection()
{
  if(_critsec != 0)
  {
    DeleteCriticalSection(_critsec);
    delete _critsec;
  }
}

void cCritSection::Lock() const
{
  EnterCriticalSection(_critsec);
}
bool cCritSection::TryLock() const
{
  return TryEnterCriticalSection(_critsec)!=0;
}
void cCritSection::Unlock() const
{
  LeaveCriticalSection(_critsec);
}

long cCritSection::GetLockCount() const
{
  return _critsec->RecursionCount;
}

void cCritSection::Unravel() const
{
  while(_critsec->RecursionCount>0)
    Unlock();
}

cCritSection& cCritSection::operator =(const cCritSection& right)
{
  if(!right._critsec || !InitializeCriticalSectionAndSpinCount(_critsec, right._critsec->SpinCount))
  {
    delete _critsec;
    _critsec = 0;
  }
  return *this;
}
