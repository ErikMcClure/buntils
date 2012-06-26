// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_MUTEX_H__BSS__
#define __C_MUTEX_H__BSS__

#include "bss_dlldef.h"

struct _RTL_CRITICAL_SECTION; //there's no way in hell I'm including the entire windows header in this

namespace bss_util {
  class BSS_DLLEXPORT cThreadSync
  {
  public:
    virtual ~cThreadSync() {}
    virtual void Lock() const=0;
    virtual bool TryLock() const=0;
    virtual void Unlock() const=0;
    virtual void Unravel() const=0; //Completely unlocks the semaphore
    virtual long GetLockCount() const=0;

    inline virtual cThreadSync* Clone() const=0;
  };

  class BSS_DLLEXPORT cMutex : public cThreadSync
  {
  public:
    cMutex(const char* name, unsigned long delay = 0xFFFFFFFF); //0xFFFFFFFF = INFINITE (wait forever)
    virtual ~cMutex();
    void Lock(unsigned long delay) const;
    virtual void Lock() const;
    virtual bool TryLock() const;
    virtual void Unlock() const;
    virtual long GetLockCount() const;
    virtual void Unravel() const; //Completely unlocks the semaphore

    inline virtual cMutex* Clone() const { return new cMutex(*this); };

  private:
    mutable long _nlock;
    unsigned long _default;
    void* _handle;
  };

  class BSS_DLLEXPORT cCritSection : public cThreadSync
  {
  public:
    cCritSection(const cCritSection& copy);
    cCritSection(unsigned long flip=0);
    virtual ~cCritSection();
    virtual void Lock() const;
    virtual void Unlock() const;
    virtual bool TryLock() const;
    virtual long GetLockCount() const;
    virtual void Unravel() const; //Completely unlocks the semaphore

    inline virtual cCritSection* Clone() const { return new cCritSection(*this); };
    cCritSection& operator =(const cCritSection& right);

  private:
    _RTL_CRITICAL_SECTION* _critsec;
  };

  /* Defines a class that has a semaphore */
  class BSS_COMPILER_DLLEXPORT cLockable
  {
  public:
    inline cLockable() : _syncobj(4) {}
    inline const cThreadSync& GetSyncObj() const { return _syncobj; }
    inline void Lock() const { _syncobj.Lock(); }
    inline void Unlock() const { _syncobj.Unlock(); }

  protected:
    cCritSection _syncobj;
  };

  /* Preforms a lock within a defined scope */
  struct BSS_COMPILER_DLLEXPORT cScopeLock
  {
    inline cScopeLock(const cLockable& lockable) : _syncobj(lockable.GetSyncObj()) { _syncobj.Lock(); }
    inline cScopeLock(const cThreadSync& syncobj) : _syncobj(syncobj) { _syncobj.Lock(); }
    inline ~cScopeLock() { _syncobj.Unlock(); }
    inline cScopeLock& operator=(const cScopeLock& right) { return *this; } // Disallow assignment because it makes no sense

  protected:
    const cThreadSync& _syncobj;
  };

#define SCOPELOCK(p) cScopeLock __scopelocker__(p)
#define LOCKME() SCOPELOCK(*this)
}

#endif